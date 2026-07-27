#include "game_asset_catalog.hpp"

#include <fstream>
#include <nlohmann/json.hpp>
#include <stdexcept>
#include <utility>

bool GameAssetCatalog::load(
    const std::filesystem::path& manifestPath,
    const std::filesystem::path& texturePrefix,
    std::string* error) {
    loaded_ = false;
    entries_.clear();

    std::ifstream input(manifestPath);
    if (!input.is_open()) {
        if (error) *error = "Game reference manifest not found";
        return false;
    }

    try {
        nlohmann::json document;
        input >> document;
        const nlohmann::json& assets = document.at("assets");
        if (!assets.is_array()) throw std::runtime_error("assets must be an array");

        for (const nlohmann::json& value : assets) {
            GameAssetEntry entry;
            entry.key = value.at("key").get<std::string>();
            entry.name = value.at("name").get<std::string>();
            entry.set = value.at("set").get<std::string>();
            entry.path = texturePrefix / value.at("path").get<std::string>();
            entry.width = value.value("width", 0);
            entry.height = value.value("height", 0);
            if (entry.name.empty() || entry.set.empty() || entry.path.empty()) continue;
            entries_.try_emplace(lookupKey(entry.set, entry.name), std::move(entry));
        }
    }
    catch (const std::exception& exception) {
        entries_.clear();
        if (error) *error = std::string("Could not read game reference manifest: ") + exception.what();
        return false;
    }

    loaded_ = !entries_.empty();
    if (!loaded_) {
        if (error) *error = "Game reference manifest contains no assets";
        return false;
    }
    if (error) error->clear();
    return true;
}

const GameAssetEntry* GameAssetCatalog::find(const std::string& set, const std::string& name) const {
    const auto found = entries_.find(lookupKey(set, name));
    return found == entries_.end() ? nullptr : &found->second;
}

bool GameAssetCatalog::loaded() const {
    return loaded_;
}

std::size_t GameAssetCatalog::size() const {
    return entries_.size();
}

std::string GameAssetCatalog::lookupKey(const std::string& set, const std::string& name) {
    return set + '\x1f' + name;
}
