#pragma once

#include <filesystem>
#include <string>

namespace WindowCapture {
bool captureBalatro(const std::filesystem::path& outputPath, std::string* error = nullptr);
}
