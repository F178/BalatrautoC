#pragma once

#include <filesystem>

class AppPaths {
public:
    static AppPaths discover();

    const std::filesystem::path& base() const;
    std::filesystem::path asset(const std::filesystem::path& relative) const;
    std::filesystem::path profile() const;

private:
    explicit AppPaths(std::filesystem::path basePath);

    std::filesystem::path basePath_;
};
