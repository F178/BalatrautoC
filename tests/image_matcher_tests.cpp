#include "image_matcher.hpp"

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
}

#define CHECK(expression) require(static_cast<bool>(expression), #expression, __LINE__)

int main() {
    const std::filesystem::path root(BALATRAUTO_SOURCE_DIR);
    const std::filesystem::path manifest = root / "assets" / "game_reference" / "asset_manifest.json";
    const std::filesystem::path blueprint = root / "assets" / "game_reference" / "Joker" /
        "123_Blueprint__j_blueprint.png";

    ImageMatcher matcher;
    std::string error;
    CHECK(matcher.load(manifest, std::filesystem::path("assets") / "game_reference", &error));
    CHECK(error.empty());
    CHECK(matcher.loaded());
    CHECK(matcher.size() >= 400);

    std::vector<ImageMatch> exact = matcher.matchFile(blueprint, 5, "Joker", &error);
    CHECK(error.empty());
    CHECK(exact.size() == 5);
    CHECK(exact[0].name == "Blueprint");
    CHECK(exact[0].confidence > 99.0f);

    int width = 0;
    int height = 0;
    int channels = 0;
    unsigned char* source = stbi_load(blueprint.string().c_str(), &width, &height, &channels, STBI_rgb_alpha);
    CHECK(source != nullptr);
    const int reducedWidth = width / 2;
    const int reducedHeight = height / 2;
    std::vector<unsigned char> reduced(
        static_cast<std::size_t>(reducedWidth) * static_cast<std::size_t>(reducedHeight) * 4U);
    for (int y = 0; y < reducedHeight; ++y) {
        for (int x = 0; x < reducedWidth; ++x) {
            const std::size_t sourceOffset =
                (static_cast<std::size_t>(y * 2) * static_cast<std::size_t>(width) +
                    static_cast<std::size_t>(x * 2)) * 4U;
            const std::size_t targetOffset =
                (static_cast<std::size_t>(y) * static_cast<std::size_t>(reducedWidth) +
                    static_cast<std::size_t>(x)) * 4U;
            for (int channel = 0; channel < 3; ++channel) {
                reduced[targetOffset + static_cast<std::size_t>(channel)] = static_cast<unsigned char>(
                    static_cast<float>(source[sourceOffset + static_cast<std::size_t>(channel)]) * 0.82f);
            }
            reduced[targetOffset + 3] = source[sourceOffset + 3];
        }
    }
    stbi_image_free(source);

    std::vector<ImageMatch> transformed = matcher.matchRgba(
        reduced.data(),
        reducedWidth,
        reducedHeight,
        5,
        "Joker");
    CHECK(transformed.size() == 5);
    CHECK(transformed[0].name == "Blueprint");
    CHECK(transformed[0].confidence > 80.0f);

    const int canvasWidth = reducedWidth + 80;
    const int canvasHeight = reducedHeight + 70;
    std::vector<unsigned char> canvas(
        static_cast<std::size_t>(canvasWidth) * static_cast<std::size_t>(canvasHeight) * 4U,
        18);
    for (std::size_t index = 3; index < canvas.size(); index += 4) canvas[index] = 255;
    constexpr int pasteX = 31;
    constexpr int pasteY = 23;
    for (int y = 0; y < reducedHeight; ++y) {
        for (int x = 0; x < reducedWidth; ++x) {
            const std::size_t sourceOffset =
                (static_cast<std::size_t>(y) * static_cast<std::size_t>(reducedWidth) +
                    static_cast<std::size_t>(x)) * 4U;
            const std::size_t targetOffset =
                (static_cast<std::size_t>(pasteY + y) * static_cast<std::size_t>(canvasWidth) +
                    static_cast<std::size_t>(pasteX + x)) * 4U;
            std::copy_n(reduced.data() + sourceOffset, 4, canvas.data() + targetOffset);
        }
    }
    std::vector<ImageMatch> regionMatches = matcher.matchRgbaRegion(
        canvas.data(),
        canvasWidth,
        canvasHeight,
        static_cast<float>(pasteX) / static_cast<float>(canvasWidth),
        static_cast<float>(pasteY) / static_cast<float>(canvasHeight),
        static_cast<float>(pasteX + reducedWidth) / static_cast<float>(canvasWidth),
        static_cast<float>(pasteY + reducedHeight) / static_cast<float>(canvasHeight),
        5,
        "Joker");
    CHECK(regionMatches.size() == 5);
    CHECK(regionMatches[0].name == "Blueprint");

    const std::filesystem::path tag = root / "assets" / "game_reference" / "Tag" /
        "007_Investment Tag__tag_investment.png";
    std::vector<ImageMatch> tagMatches = matcher.matchFile(tag, 3, "Tag", &error);
    CHECK(tagMatches.size() == 3);
    CHECK(tagMatches[0].name == "Investment Tag");
    return 0;
}
