#pragma once

#include <cstddef>
#include <filesystem>
#include <string>
#include <vector>

struct ImageRegion {
    float left = 0.0f;
    float top = 0.0f;
    float right = 1.0f;
    float bottom = 1.0f;
    float score = 0.0f;
};

class ImageRegionDetector {
public:
    static std::vector<ImageRegion> detectFile(
        const std::filesystem::path& imagePath,
        const std::vector<float>& aspectRatios,
        std::size_t limit,
        std::string* error = nullptr);

    static std::vector<ImageRegion> detectRgba(
        const unsigned char* pixels,
        int width,
        int height,
        const std::vector<float>& aspectRatios,
        std::size_t limit);
};
