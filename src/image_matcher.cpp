#include "image_matcher.hpp"

#include "stb_image.h"

#include <algorithm>
#include <cmath>
#include <fstream>
#include <limits>
#include <nlohmann/json.hpp>
#include <stdexcept>

namespace {
float sampleChannel(
    const unsigned char* pixels,
    int width,
    int height,
    float x,
    float y,
    int channel) {
    const float clampedX = std::clamp(x, 0.0f, static_cast<float>(width - 1));
    const float clampedY = std::clamp(y, 0.0f, static_cast<float>(height - 1));
    const int x0 = static_cast<int>(clampedX);
    const int y0 = static_cast<int>(clampedY);
    const int x1 = std::min(x0 + 1, width - 1);
    const int y1 = std::min(y0 + 1, height - 1);
    const float fx = clampedX - static_cast<float>(x0);
    const float fy = clampedY - static_cast<float>(y0);

    const auto value = [pixels, width, channel](int px, int py) {
        const std::size_t offset = (static_cast<std::size_t>(py) * static_cast<std::size_t>(width) +
            static_cast<std::size_t>(px)) * 4U;
        const float alpha = static_cast<float>(pixels[offset + 3]) / 255.0f;
        const float color = static_cast<float>(pixels[offset + static_cast<std::size_t>(channel)]) / 255.0f;
        return color * alpha + 0.035f * (1.0f - alpha);
    };

    const float top = value(x0, y0) * (1.0f - fx) + value(x1, y0) * fx;
    const float bottom = value(x0, y1) * (1.0f - fx) + value(x1, y1) * fx;
    return top * (1.0f - fy) + bottom * fy;
}

float luma(float red, float green, float blue) {
    return red * 0.299f + green * 0.587f + blue * 0.114f;
}

int bitCount(std::uint64_t value) {
    int count = 0;
    while (value != 0) {
        value &= value - 1;
        ++count;
    }
    return count;
}
}

bool ImageMatcher::load(
    const std::filesystem::path& manifestPath,
    const std::filesystem::path& texturePrefix,
    std::string* error) {
    loaded_ = false;
    references_.clear();

    std::ifstream input(manifestPath);
    if (!input.is_open()) {
        if (error) *error = "Game reference manifest not found";
        return false;
    }

    try {
        nlohmann::json document;
        input >> document;
        const nlohmann::json& assets = document.at("assets");
        if (!assets.is_array()) throw std::runtime_error("assets must be an array");

        references_.reserve(assets.size());
        for (const nlohmann::json& value : assets) {
            const std::filesystem::path relativePath = value.at("path").get<std::string>();
            Reference reference;
            reference.asset.key = value.at("key").get<std::string>();
            reference.asset.name = value.at("name").get<std::string>();
            reference.asset.set = value.at("set").get<std::string>();
            reference.asset.path = texturePrefix / relativePath;
            if (!descriptorFromFile(manifestPath.parent_path() / relativePath, reference.descriptor, nullptr)) {
                continue;
            }
            references_.push_back(std::move(reference));
        }
    }
    catch (const std::exception& exception) {
        references_.clear();
        if (error) *error = std::string("Could not build image index: ") + exception.what();
        return false;
    }

    loaded_ = !references_.empty();
    if (!loaded_) {
        if (error) *error = "No readable game reference images were found";
        return false;
    }
    if (error) error->clear();
    return true;
}

std::vector<ImageMatch> ImageMatcher::matchFile(
    const std::filesystem::path& imagePath,
    std::size_t limit,
    const std::string& setFilter,
    std::string* error) const {
    Descriptor query;
    if (!descriptorFromFile(imagePath, query, error)) return {};

    std::vector<ImageMatch> matches;
    matches.reserve(references_.size());
    for (const Reference& reference : references_) {
        if (!setFilter.empty() && reference.asset.set != setFilter) continue;
        ImageMatch match = reference.asset;
        match.confidence = compare(query, reference.descriptor);
        matches.push_back(std::move(match));
    }
    std::stable_sort(matches.begin(), matches.end(), [](const ImageMatch& left, const ImageMatch& right) {
        return left.confidence > right.confidence;
    });
    if (matches.size() > limit) matches.resize(limit);
    if (error) error->clear();
    return matches;
}

std::vector<ImageMatch> ImageMatcher::matchFileRegion(
    const std::filesystem::path& imagePath,
    float left,
    float top,
    float right,
    float bottom,
    std::size_t limit,
    const std::string& setFilter,
    std::string* error) const {
    const std::string filename = imagePath.string();
    int width = 0;
    int height = 0;
    int channels = 0;
    unsigned char* pixels = stbi_load(filename.c_str(), &width, &height, &channels, STBI_rgb_alpha);
    if (!pixels) {
        if (error) {
            *error = "Could not load image " + imagePath.filename().string() + ": " + stbi_failure_reason();
        }
        return {};
    }
    std::vector<ImageMatch> matches = matchRgbaRegion(
        pixels,
        width,
        height,
        left,
        top,
        right,
        bottom,
        limit,
        setFilter);
    stbi_image_free(pixels);
    if (matches.empty() && error) *error = "The selected image region is empty";
    else if (error) error->clear();
    return matches;
}

std::vector<ImageMatch> ImageMatcher::matchRgba(
    const unsigned char* pixels,
    int width,
    int height,
    std::size_t limit,
    const std::string& setFilter) const {
    Descriptor query;
    if (!descriptorFromRgba(pixels, width, height, query)) return {};

    std::vector<ImageMatch> matches;
    matches.reserve(references_.size());
    for (const Reference& reference : references_) {
        if (!setFilter.empty() && reference.asset.set != setFilter) continue;
        ImageMatch match = reference.asset;
        match.confidence = compare(query, reference.descriptor);
        matches.push_back(std::move(match));
    }
    std::stable_sort(matches.begin(), matches.end(), [](const ImageMatch& left, const ImageMatch& right) {
        return left.confidence > right.confidence;
    });
    if (matches.size() > limit) matches.resize(limit);
    return matches;
}

std::vector<ImageMatch> ImageMatcher::matchRgbaRegion(
    const unsigned char* pixels,
    int width,
    int height,
    float left,
    float top,
    float right,
    float bottom,
    std::size_t limit,
    const std::string& setFilter) const {
    if (!pixels || width <= 0 || height <= 0) return {};
    const float minX = std::clamp(std::min(left, right), 0.0f, 1.0f);
    const float minY = std::clamp(std::min(top, bottom), 0.0f, 1.0f);
    const float maxX = std::clamp(std::max(left, right), 0.0f, 1.0f);
    const float maxY = std::clamp(std::max(top, bottom), 0.0f, 1.0f);
    const int x0 = std::clamp(static_cast<int>(std::floor(minX * width)), 0, width - 1);
    const int y0 = std::clamp(static_cast<int>(std::floor(minY * height)), 0, height - 1);
    const int x1 = std::clamp(static_cast<int>(std::ceil(maxX * width)), x0 + 1, width);
    const int y1 = std::clamp(static_cast<int>(std::ceil(maxY * height)), y0 + 1, height);
    const int cropWidth = x1 - x0;
    const int cropHeight = y1 - y0;
    if (cropWidth < 2 || cropHeight < 2) return {};

    std::vector<unsigned char> crop(
        static_cast<std::size_t>(cropWidth) * static_cast<std::size_t>(cropHeight) * 4U);
    for (int row = 0; row < cropHeight; ++row) {
        const std::size_t sourceOffset =
            (static_cast<std::size_t>(y0 + row) * static_cast<std::size_t>(width) +
                static_cast<std::size_t>(x0)) * 4U;
        const std::size_t destinationOffset =
            static_cast<std::size_t>(row) * static_cast<std::size_t>(cropWidth) * 4U;
        std::copy_n(
            pixels + sourceOffset,
            static_cast<std::size_t>(cropWidth) * 4U,
            crop.data() + destinationOffset);
    }
    return matchRgba(crop.data(), cropWidth, cropHeight, limit, setFilter);
}

bool ImageMatcher::loaded() const {
    return loaded_;
}

std::size_t ImageMatcher::size() const {
    return references_.size();
}

bool ImageMatcher::descriptorFromFile(
    const std::filesystem::path& path,
    Descriptor& descriptor,
    std::string* error) {
    const std::string filename = path.string();
    int width = 0;
    int height = 0;
    int channels = 0;
    unsigned char* pixels = stbi_load(filename.c_str(), &width, &height, &channels, STBI_rgb_alpha);
    if (!pixels) {
        if (error) {
            *error = "Could not load image " + path.filename().string() + ": " + stbi_failure_reason();
        }
        return false;
    }
    const bool success = descriptorFromRgba(pixels, width, height, descriptor);
    stbi_image_free(pixels);
    if (!success && error) *error = "Image has invalid dimensions";
    return success;
}

bool ImageMatcher::descriptorFromRgba(
    const unsigned char* pixels,
    int width,
    int height,
    Descriptor& descriptor) {
    if (!pixels || width <= 0 || height <= 0) return false;

    descriptor = {};
    descriptor.aspectRatio = static_cast<float>(width) / static_cast<float>(height);
    float mean = 0.0f;
    for (int row = 0; row < GridHeight; ++row) {
        for (int column = 0; column < GridWidth; ++column) {
            const float x = (static_cast<float>(column) + 0.5f) * static_cast<float>(width) /
                static_cast<float>(GridWidth) - 0.5f;
            const float y = (static_cast<float>(row) + 0.5f) * static_cast<float>(height) /
                static_cast<float>(GridHeight) - 0.5f;
            const float red = sampleChannel(pixels, width, height, x, y, 0);
            const float green = sampleChannel(pixels, width, height, x, y, 1);
            const float blue = sampleChannel(pixels, width, height, x, y, 2);
            const std::size_t index = static_cast<std::size_t>(row * GridWidth + column);
            descriptor.luma[index] = ::luma(red, green, blue);
            descriptor.chromaRed[index] = red - green;
            descriptor.chromaBlue[index] = blue - green;
            mean += descriptor.luma[index];
        }
    }
    mean /= static_cast<float>(GridSize);

    float variance = 0.0f;
    for (float value : descriptor.luma) {
        const float difference = value - mean;
        variance += difference * difference;
    }
    const float deviation = std::max(0.06f, std::sqrt(variance / static_cast<float>(GridSize)));
    for (std::size_t index = 0; index < descriptor.luma.size(); ++index) {
        descriptor.normalizedLuma[index] = (descriptor.luma[index] - mean) / deviation;
    }

    std::uint64_t hash = 0;
    int bit = 0;
    for (int row = 0; row < 8; ++row) {
        const float y = (static_cast<float>(row) + 0.5f) * static_cast<float>(height) / 8.0f - 0.5f;
        for (int column = 0; column < 8; ++column) {
            const float leftX = (static_cast<float>(column) + 0.5f) * static_cast<float>(width) / 9.0f - 0.5f;
            const float rightX = (static_cast<float>(column) + 1.5f) * static_cast<float>(width) / 9.0f - 0.5f;
            const float left = ::luma(
                sampleChannel(pixels, width, height, leftX, y, 0),
                sampleChannel(pixels, width, height, leftX, y, 1),
                sampleChannel(pixels, width, height, leftX, y, 2));
            const float right = ::luma(
                sampleChannel(pixels, width, height, rightX, y, 0),
                sampleChannel(pixels, width, height, rightX, y, 1),
                sampleChannel(pixels, width, height, rightX, y, 2));
            if (left > right) hash |= std::uint64_t{1} << bit;
            ++bit;
        }
    }
    descriptor.differenceHash = hash;
    return true;
}

float ImageMatcher::compare(const Descriptor& query, const Descriptor& reference) {
    float lumaDistance = 0.0f;
    float patternDistance = 0.0f;
    float chromaDistance = 0.0f;
    for (std::size_t index = 0; index < query.luma.size(); ++index) {
        lumaDistance += std::abs(query.luma[index] - reference.luma[index]);
        patternDistance += std::min(
            3.0f,
            std::abs(query.normalizedLuma[index] - reference.normalizedLuma[index])) / 3.0f;
        chromaDistance += (
            std::abs(query.chromaRed[index] - reference.chromaRed[index]) +
            std::abs(query.chromaBlue[index] - reference.chromaBlue[index])) * 0.5f;
    }
    const float count = static_cast<float>(query.luma.size());
    lumaDistance /= count;
    patternDistance /= count;
    chromaDistance /= count;

    const float hashDistance = static_cast<float>(bitCount(
        query.differenceHash ^ reference.differenceHash)) / 64.0f;
    const float aspectDistance = std::min(
        1.0f,
        std::abs(std::log(std::max(0.001f, query.aspectRatio) /
            std::max(0.001f, reference.aspectRatio))) / 0.45f);
    const float distance =
        patternDistance * 0.40f +
        chromaDistance * 0.25f +
        lumaDistance * 0.15f +
        hashDistance * 0.15f +
        aspectDistance * 0.05f;
    return std::clamp((1.0f - distance) * 100.0f, 0.0f, 100.0f);
}
