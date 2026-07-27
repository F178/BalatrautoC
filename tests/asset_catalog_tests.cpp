#include "game_asset_catalog.hpp"

#include <cstdlib>
#include <filesystem>
#include <iostream>

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
    GameAssetCatalog catalog;
    std::string error;
    CHECK(catalog.load(
        root / "assets" / "game_reference" / "asset_manifest.json",
        std::filesystem::path("assets") / "game_reference",
        &error));
    CHECK(error.empty());
    CHECK(catalog.loaded());
    CHECK(catalog.size() >= 300);

    const GameAssetEntry* joker = catalog.find("Joker", "Blueprint");
    CHECK(joker != nullptr);
    CHECK(joker->key == "j_blueprint");
    CHECK(joker->width == 142);
    CHECK(joker->height == 190);
    CHECK(joker->path == std::filesystem::path("assets") / "game_reference" / "Joker" /
        "123_Blueprint__j_blueprint.png");

    const GameAssetEntry* blind = catalog.find("Blind", "The Arm");
    CHECK(blind != nullptr);
    CHECK(blind->width == 68);
    CHECK(blind->height == 68);

    const GameAssetEntry* pack = catalog.find("Booster", "Arcana Pack");
    CHECK(pack != nullptr);
    CHECK(pack->key == "p_arcana_normal_1");
    return 0;
}
