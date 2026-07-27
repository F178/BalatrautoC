#pragma once

#include <filesystem>
#include <string>
#include <unordered_map>

struct GameAssetEntry {
    std::string key;
    std::string name;
    std::string set;
    std::filesystem::path path;
    int width = 0;
    int height = 0;
};

class GameAssetCatalog {
public:
    bool load(
        const std::filesystem::path& manifestPath,
        const std::filesystem::path& texturePrefix,
        std::string* error = nullptr);

    const GameAssetEntry* find(const std::string& set, const std::string& name) const;
    bool loaded() const;
    std::size_t size() const;

private:
    static std::string lookupKey(const std::string& set, const std::string& name);

    bool loaded_ = false;
    std::unordered_map<std::string, GameAssetEntry> entries_;
};
