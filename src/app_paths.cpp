#include "app_paths.hpp"

#include <SDL3/SDL.h>

#include <system_error>

AppPaths::AppPaths(std::filesystem::path basePath)
    : basePath_(std::move(basePath)) {
}

AppPaths AppPaths::discover() {
    const char* sdlBasePath = SDL_GetBasePath();
    std::filesystem::path basePath = sdlBasePath && *sdlBasePath
        ? std::filesystem::path(sdlBasePath)
        : std::filesystem::current_path();

    std::error_code error;
    basePath = std::filesystem::weakly_canonical(basePath, error);
    if (error) {
        basePath = std::filesystem::absolute(basePath);
    }

    return AppPaths(std::move(basePath));
}

const std::filesystem::path& AppPaths::base() const {
    return basePath_;
}

std::filesystem::path AppPaths::asset(const std::filesystem::path& relative) const {
    return basePath_ / relative;
}

std::filesystem::path AppPaths::profile() const {
    return basePath_ / "player_profile.json";
}
