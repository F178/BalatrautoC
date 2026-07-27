#include "image_region_detector.hpp"

#include "stb_image.h"

#include <algorithm>
#include <cstdlib>
#include <filesystem>
#include <iostream>
#include <vector>

namespace {
void require(bool condition, const char* expression, int line) {
    if (condition) return;
    std::cerr << "CHECK failed at line " << line << ": " << expression << '\n';
    std::exit(1);
}

float intersectionOverUnion(const ImageRegion& region, float left, float top, float right, float bottom) {
    const float intersectionWidth = std::max(0.0f, std::min(region.right, right) - std::max(region.left, left));
    const float intersectionHeight = std::max(0.0f, std::min(region.bottom, bottom) - std::max(region.top, top));
    const float intersection = intersectionWidth * intersectionHeight;
    const float regionArea = (region.right - region.left) * (region.bottom - region.top);
    const float targetArea = (right - left) * (bottom - top);
    return intersection / (regionArea + targetArea - intersection);
}
}

#define CHECK(expression) require(static_cast<bool>(expression), #expression, __LINE__)

int main() {
    const std::filesystem::path root(BALATRAUTO_SOURCE_DIR);
    const std::filesystem::path blueprint = root / "assets" / "game_reference" / "Joker" /
        "123_Blueprint__j_blueprint.png";
    int cardWidth = 0;
    int cardHeight = 0;
    int channels = 0;
    unsigned char* card = stbi_load(
        blueprint.string().c_str(),
        &cardWidth,
        &cardHeight,
        &channels,
        STBI_rgb_alpha);
    CHECK(card != nullptr);

    constexpr int canvasWidth = 720;
    constexpr int canvasHeight = 440;
    constexpr int pasteX = 94;
    constexpr int pasteY = 78;
    std::vector<unsigned char> canvas(
        static_cast<std::size_t>(canvasWidth) * static_cast<std::size_t>(canvasHeight) * 4U,
        20);
    for (std::size_t index = 3; index < canvas.size(); index += 4U) canvas[index] = 255;
    for (int y = 0; y < cardHeight; ++y) {
        for (int x = 0; x < cardWidth; ++x) {
            const std::size_t source =
                (static_cast<std::size_t>(y) * static_cast<std::size_t>(cardWidth) +
                    static_cast<std::size_t>(x)) * 4U;
            const std::size_t target =
                (static_cast<std::size_t>(pasteY + y) * static_cast<std::size_t>(canvasWidth) +
                    static_cast<std::size_t>(pasteX + x)) * 4U;
            const float alpha = static_cast<float>(card[source + 3U]) / 255.0f;
            for (std::size_t component = 0; component < 3U; ++component) {
                canvas[target + component] = static_cast<unsigned char>(
                    static_cast<float>(card[source + component]) * alpha +
                    static_cast<float>(canvas[target + component]) * (1.0f - alpha));
            }
            canvas[target + 3U] = 255;
        }
    }
    stbi_image_free(card);

    const std::vector<ImageRegion> regions = ImageRegionDetector::detectRgba(
        canvas.data(),
        canvasWidth,
        canvasHeight,
        {142.0f / 190.0f},
        24);
    CHECK(!regions.empty());

    const float left = static_cast<float>(pasteX) / static_cast<float>(canvasWidth);
    const float top = static_cast<float>(pasteY) / static_cast<float>(canvasHeight);
    const float right = static_cast<float>(pasteX + cardWidth) / static_cast<float>(canvasWidth);
    const float bottom = static_cast<float>(pasteY + cardHeight) / static_cast<float>(canvasHeight);
    float bestOverlap = 0.0f;
    for (const ImageRegion& region : regions) {
        const float overlap = intersectionOverUnion(region, left, top, right, bottom);
        bestOverlap = std::max(bestOverlap, overlap);
    }
    const bool found = bestOverlap > 0.40f;
    CHECK(found);
    return 0;
}
