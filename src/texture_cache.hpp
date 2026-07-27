#pragma once

#include <SDL3/SDL.h>

#include <filesystem>
#include <string>
#include <unordered_map>

struct TextureAsset {
    SDL_Texture* texture = nullptr;
    int width = 0;
    int height = 0;

    explicit operator bool() const {
        return texture != nullptr;
    }
};

class TextureCache {
public:
    TextureCache(SDL_Renderer* renderer, std::filesystem::path assetRoot);
    ~TextureCache();

    TextureCache(const TextureCache&) = delete;
    TextureCache& operator=(const TextureCache&) = delete;

    const TextureAsset& get(const std::filesystem::path& relativePath);
    void invalidate(const std::filesystem::path& relativePath);
    void clear();

private:
    TextureAsset load(const std::filesystem::path& relativePath) const;

    SDL_Renderer* renderer_ = nullptr;
    std::filesystem::path assetRoot_;
    std::unordered_map<std::string, TextureAsset> assets_;
};
