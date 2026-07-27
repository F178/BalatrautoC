#include "image_region_detector.hpp"

#include "stb_image.h"

#include <algorithm>
#include <cmath>
#include <limits>
#include <utility>

namespace {
struct IntegralImage {
    int width = 0;
    int height = 0;
    std::vector<float> values;

    float sum(int left, int top, int right, int bottom) const {
        left = std::clamp(left, 0, width);
        right = std::clamp(right, 0, width);
        top = std::clamp(top, 0, height);
        bottom = std::clamp(bottom, 0, height);
        if (left >= right || top >= bottom) return 0.0f;
        const int stride = width + 1;
        return values[static_cast<std::size_t>(bottom * stride + right)] -
            values[static_cast<std::size_t>(top * stride + right)] -
            values[static_cast<std::size_t>(bottom * stride + left)] +
            values[static_cast<std::size_t>(top * stride + left)];
    }
};

IntegralImage makeIntegral(const std::vector<float>& source, int width, int height) {
    IntegralImage result;
    result.width = width;
    result.height = height;
    result.values.assign(
        static_cast<std::size_t>(width + 1) * static_cast<std::size_t>(height + 1),
        0.0f);
    const int stride = width + 1;
    for (int y = 0; y < height; ++y) {
        float row = 0.0f;
        for (int x = 0; x < width; ++x) {
            row += source[static_cast<std::size_t>(y * width + x)];
            result.values[static_cast<std::size_t>((y + 1) * stride + x + 1)] =
                result.values[static_cast<std::size_t>(y * stride + x + 1)] + row;
        }
    }
    return result;
}

float intersectionOverUnion(const ImageRegion& left, const ImageRegion& right) {
    const float intersectionWidth = std::max(
        0.0f,
        std::min(left.right, right.right) - std::max(left.left, right.left));
    const float intersectionHeight = std::max(
        0.0f,
        std::min(left.bottom, right.bottom) - std::max(left.top, right.top));
    const float intersection = intersectionWidth * intersectionHeight;
    const float leftArea = (left.right - left.left) * (left.bottom - left.top);
    const float rightArea = (right.right - right.left) * (right.bottom - right.top);
    const float combined = leftArea + rightArea - intersection;
    return combined > 0.0f ? intersection / combined : 0.0f;
}

float channel(const unsigned char* pixels, int width, int x, int y, int component) {
    const std::size_t offset =
        (static_cast<std::size_t>(y) * static_cast<std::size_t>(width) +
            static_cast<std::size_t>(x)) * 4U;
    return static_cast<float>(pixels[offset + static_cast<std::size_t>(component)]) / 255.0f;
}
}

std::vector<ImageRegion> ImageRegionDetector::detectFile(
    const std::filesystem::path& imagePath,
    const std::vector<float>& aspectRatios,
    std::size_t limit,
    std::string* error) {
    const std::string filename = imagePath.string();
    int width = 0;
    int height = 0;
    int channels = 0;
    unsigned char* pixels = stbi_load(filename.c_str(), &width, &height, &channels, STBI_rgb_alpha);
    if (!pixels) {
        if (error) {
            *error = "Could not load " + imagePath.filename().string() + ": " + stbi_failure_reason();
        }
        return {};
    }
    std::vector<ImageRegion> regions = detectRgba(pixels, width, height, aspectRatios, limit);
    stbi_image_free(pixels);
    if (error) error->clear();
    return regions;
}

std::vector<ImageRegion> ImageRegionDetector::detectRgba(
    const unsigned char* pixels,
    int width,
    int height,
    const std::vector<float>& aspectRatios,
    std::size_t limit) {
    if (!pixels || width < 24 || height < 24 || aspectRatios.empty() || limit == 0) return {};

    const float scale = std::min(
        1.0f,
        std::min(480.0f / static_cast<float>(width), 320.0f / static_cast<float>(height)));
    const int sampleWidth = std::max(1, static_cast<int>(std::round(width * scale)));
    const int sampleHeight = std::max(1, static_cast<int>(std::round(height * scale)));
    const std::size_t sampleCount =
        static_cast<std::size_t>(sampleWidth) * static_cast<std::size_t>(sampleHeight);
    std::vector<float> red(sampleCount);
    std::vector<float> green(sampleCount);
    std::vector<float> blue(sampleCount);
    std::vector<float> luminance(sampleCount);
    std::vector<float> luminanceSquared(sampleCount);
    std::vector<float> saturation(sampleCount);
    std::vector<float> edge(sampleCount, 0.0f);

    for (int y = 0; y < sampleHeight; ++y) {
        const int sourceY = std::min(
            height - 1,
            static_cast<int>((static_cast<float>(y) + 0.5f) / scale));
        for (int x = 0; x < sampleWidth; ++x) {
            const int sourceX = std::min(
                width - 1,
                static_cast<int>((static_cast<float>(x) + 0.5f) / scale));
            const std::size_t index = static_cast<std::size_t>(y * sampleWidth + x);
            red[index] = channel(pixels, width, sourceX, sourceY, 0);
            green[index] = channel(pixels, width, sourceX, sourceY, 1);
            blue[index] = channel(pixels, width, sourceX, sourceY, 2);
            luminance[index] = red[index] * 0.299f + green[index] * 0.587f + blue[index] * 0.114f;
            luminanceSquared[index] = luminance[index] * luminance[index];
            const float maximum = std::max({red[index], green[index], blue[index]});
            const float minimum = std::min({red[index], green[index], blue[index]});
            saturation[index] = maximum - minimum;
        }
    }

    for (int y = 1; y < sampleHeight - 1; ++y) {
        for (int x = 1; x < sampleWidth - 1; ++x) {
            const std::size_t index = static_cast<std::size_t>(y * sampleWidth + x);
            const std::size_t left = index - 1;
            const std::size_t right = index + 1;
            const std::size_t top = index - static_cast<std::size_t>(sampleWidth);
            const std::size_t bottom = index + static_cast<std::size_t>(sampleWidth);
            const float lumaGradient =
                std::abs(luminance[right] - luminance[left]) +
                std::abs(luminance[bottom] - luminance[top]);
            const float colorGradient = (
                std::abs(red[right] - red[left]) +
                std::abs(green[right] - green[left]) +
                std::abs(blue[right] - blue[left]) +
                std::abs(red[bottom] - red[top]) +
                std::abs(green[bottom] - green[top]) +
                std::abs(blue[bottom] - blue[top])) / 3.0f;
            edge[index] = std::clamp(lumaGradient * 0.62f + colorGradient * 0.38f, 0.0f, 1.0f);
        }
    }

    const IntegralImage lumaIntegral = makeIntegral(luminance, sampleWidth, sampleHeight);
    const IntegralImage squaredIntegral = makeIntegral(luminanceSquared, sampleWidth, sampleHeight);
    const IntegralImage saturationIntegral = makeIntegral(saturation, sampleWidth, sampleHeight);
    const IntegralImage edgeIntegral = makeIntegral(edge, sampleWidth, sampleHeight);

    std::vector<ImageRegion> candidates;
    candidates.reserve(2200);
    const int minimumHeight = std::max(18, static_cast<int>(std::round(sampleHeight * 0.055f)));
    const int maximumHeight = std::max(
        minimumHeight,
        static_cast<int>(std::round(sampleHeight * 0.58f)));

    for (float aspectRatio : aspectRatios) {
        if (aspectRatio < 0.3f || aspectRatio > 2.0f) continue;
        for (float proposedHeight = static_cast<float>(minimumHeight);
             proposedHeight <= static_cast<float>(maximumHeight) + 0.5f;
             proposedHeight *= 1.22f) {
            const int windowHeight = std::max(12, static_cast<int>(std::round(proposedHeight)));
            const int windowWidth = std::max(10, static_cast<int>(std::round(windowHeight * aspectRatio)));
            if (windowWidth >= sampleWidth || windowHeight >= sampleHeight) continue;
            const int stride = std::max(4, std::min(windowWidth, windowHeight) / 5);
            for (int top = 0; top + windowHeight <= sampleHeight; top += stride) {
                for (int left = 0; left + windowWidth <= sampleWidth; left += stride) {
                    const int right = left + windowWidth;
                    const int bottom = top + windowHeight;
                    const float area = static_cast<float>(windowWidth * windowHeight);
                    const float mean = lumaIntegral.sum(left, top, right, bottom) / area;
                    const float secondMoment = squaredIntegral.sum(left, top, right, bottom) / area;
                    const float deviation = std::sqrt(std::max(0.0f, secondMoment - mean * mean));
                    const float edgeMean = edgeIntegral.sum(left, top, right, bottom) / area;
                    const float saturationMean = saturationIntegral.sum(left, top, right, bottom) / area;

                    const int padding = std::max(3, windowHeight / 10);
                    const int outerLeft = std::max(0, left - padding);
                    const int outerTop = std::max(0, top - padding);
                    const int outerRight = std::min(sampleWidth, right + padding);
                    const int outerBottom = std::min(sampleHeight, bottom + padding);
                    const float outerArea = static_cast<float>(
                        (outerRight - outerLeft) * (outerBottom - outerTop));
                    const float ringArea = outerArea - area;
                    float contrast = 0.0f;
                    if (ringArea > 1.0f) {
                        const float outerSum = lumaIntegral.sum(
                            outerLeft,
                            outerTop,
                            outerRight,
                            outerBottom);
                        const float ringMean = (outerSum - mean * area) / ringArea;
                        contrast = std::abs(mean - ringMean);
                    }

                    const float score =
                        std::min(1.0f, edgeMean * 4.2f) * 0.48f +
                        std::min(1.0f, deviation * 3.5f) * 0.30f +
                        std::min(1.0f, contrast * 4.0f) * 0.17f +
                        std::min(1.0f, saturationMean * 1.8f) * 0.05f;
                    if (score < 0.20f) continue;
                    candidates.push_back(ImageRegion{
                        static_cast<float>(left) / static_cast<float>(sampleWidth),
                        static_cast<float>(top) / static_cast<float>(sampleHeight),
                        static_cast<float>(right) / static_cast<float>(sampleWidth),
                        static_cast<float>(bottom) / static_cast<float>(sampleHeight),
                        score});
                }
            }
        }
    }

    std::stable_sort(candidates.begin(), candidates.end(), [](const ImageRegion& left, const ImageRegion& right) {
        return left.score > right.score;
    });

    std::vector<ImageRegion> result;
    result.reserve(limit);
    const std::size_t perScale = std::max<std::size_t>(2, (limit + 3U) / 4U);
    for (int scaleBand = 0; scaleBand < 4 && result.size() < limit; ++scaleBand) {
        std::size_t acceptedInBand = 0;
        for (const ImageRegion& candidate : candidates) {
            const float relativeHeight = candidate.bottom - candidate.top;
            const int candidateBand = relativeHeight < 0.12f
                ? 0
                : relativeHeight < 0.24f
                    ? 1
                    : relativeHeight < 0.40f ? 2 : 3;
            if (candidateBand != scaleBand) continue;
            const bool overlaps = std::any_of(result.begin(), result.end(), [&candidate](const ImageRegion& accepted) {
                return intersectionOverUnion(candidate, accepted) > 0.42f;
            });
            if (overlaps) continue;
            result.push_back(candidate);
            ++acceptedInBand;
            if (acceptedInBand >= perScale || result.size() >= limit) break;
        }
    }
    for (const ImageRegion& candidate : candidates) {
        if (result.size() >= limit) break;
        const bool overlaps = std::any_of(result.begin(), result.end(), [&candidate](const ImageRegion& accepted) {
            return intersectionOverUnion(candidate, accepted) > 0.42f;
        });
        if (!overlaps) result.push_back(candidate);
    }
    return result;
}
