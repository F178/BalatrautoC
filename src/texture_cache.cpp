#include "texture_cache.hpp"

#include "stb_image.h"

#include <cstring>

TextureCache::TextureCache(SDL_Renderer* renderer, std::filesystem::path assetRoot)
    : renderer_(renderer), assetRoot_(std::move(assetRoot)) {
}

TextureCache::~TextureCache() {
    clear();
}

const TextureAsset& TextureCache::get(const std::filesystem::path& relativePath) {
    const std::string key = relativePath.generic_string();
    const auto found = assets_.find(key);
    if (found != assets_.end()) {
        return found->second;
    }

    return assets_.emplace(key, load(relativePath)).first->second;
}

void TextureCache::invalidate(const std::filesystem::path& relativePath) {
    const std::string key = relativePath.generic_string();
    const auto found = assets_.find(key);
    if (found == assets_.end()) return;
    if (found->second.texture) SDL_DestroyTexture(found->second.texture);
    assets_.erase(found);
}

void TextureCache::clear() {
    for (auto& [key, asset] : assets_) {
        if (asset.texture) {
            SDL_DestroyTexture(asset.texture);
            asset.texture = nullptr;
        }
    }
    assets_.clear();
}

TextureAsset TextureCache::load(const std::filesystem::path& relativePath) const {
    const std::filesystem::path fullPath = relativePath.is_absolute()
        ? relativePath
        : assetRoot_ / relativePath;
    const std::string filename = fullPath.string();

    int width = 0;
    int height = 0;
    int channels = 0;
    unsigned char* pixels = stbi_load(filename.c_str(), &width, &height, &channels, STBI_rgb_alpha);
    if (!pixels) {
        SDL_Log("Could not load image %s: %s", filename.c_str(), stbi_failure_reason());
        return {};
    }

    SDL_Surface* surface = SDL_CreateSurface(width, height, SDL_PIXELFORMAT_RGBA32);
    if (!surface) {
        SDL_Log("Could not create image surface for %s: %s", filename.c_str(), SDL_GetError());
        stbi_image_free(pixels);
        return {};
    }

    const size_t sourcePitch = static_cast<size_t>(width) * 4U;
    auto* destination = static_cast<unsigned char*>(surface->pixels);
    for (int row = 0; row < height; ++row) {
        std::memcpy(
            destination + static_cast<size_t>(row) * static_cast<size_t>(surface->pitch),
            pixels + static_cast<size_t>(row) * sourcePitch,
            sourcePitch);
    }

    SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer_, surface);
    SDL_DestroySurface(surface);
    stbi_image_free(pixels);

    if (!texture) {
        SDL_Log("Could not create texture for %s: %s", filename.c_str(), SDL_GetError());
        return {};
    }

    SDL_SetTextureScaleMode(texture, SDL_SCALEMODE_NEAREST);
    SDL_SetTextureBlendMode(texture, SDL_BLENDMODE_BLEND);
    return TextureAsset{texture, width, height};
}
