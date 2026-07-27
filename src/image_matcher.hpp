#pragma once

#include <array>
#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <string>
#include <vector>

struct ImageMatch {
    std::string key;
    std::string name;
    std::string set;
    std::filesystem::path path;
    float confidence = 0.0f;
};

class ImageMatcher {
public:
    bool load(
        const std::filesystem::path& manifestPath,
        const std::filesystem::path& texturePrefix,
        std::string* error = nullptr);

    std::vector<ImageMatch> matchFile(
        const std::filesystem::path& imagePath,
        std::size_t limit,
        const std::string& setFilter = {},
        std::string* error = nullptr) const;

    std::vector<ImageMatch> matchFileRegion(
        const std::filesystem::path& imagePath,
        float left,
        float top,
        float right,
        float bottom,
        std::size_t limit,
        const std::string& setFilter = {},
        std::string* error = nullptr) const;

    std::vector<ImageMatch> matchRgba(
        const unsigned char* pixels,
        int width,
        int height,
        std::size_t limit,
        const std::string& setFilter = {}) const;

    std::vector<ImageMatch> matchRgbaRegion(
        const unsigned char* pixels,
        int width,
        int height,
        float left,
        float top,
        float right,
        float bottom,
        std::size_t limit,
        const std::string& setFilter = {}) const;

    bool loaded() const;
    std::size_t size() const;

private:
    static constexpr int GridWidth = 16;
    static constexpr int GridHeight = 20;
    static constexpr int GridSize = GridWidth * GridHeight;

    struct Descriptor {
        std::array<float, GridSize> luma{};
        std::array<float, GridSize> normalizedLuma{};
        std::array<float, GridSize> chromaRed{};
        std::array<float, GridSize> chromaBlue{};
        std::uint64_t differenceHash = 0;
        float aspectRatio = 1.0f;
    };

    struct Reference {
        ImageMatch asset;
        Descriptor descriptor;
    };

    static bool descriptorFromFile(
        const std::filesystem::path& path,
        Descriptor& descriptor,
        std::string* error);
    static bool descriptorFromRgba(
        const unsigned char* pixels,
        int width,
        int height,
        Descriptor& descriptor);
    static float compare(const Descriptor& query, const Descriptor& reference);

    bool loaded_ = false;
    std::vector<Reference> references_;
};
