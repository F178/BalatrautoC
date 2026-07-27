#include "data.hpp"
#include "profile.hpp"

#include <chrono>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>

namespace {
void check(bool condition, const char* expression, int line) {
    if (condition) {
        return;
    }
    std::cerr << "Check failed at line " << line << ": " << expression << '\n';
    std::exit(EXIT_FAILURE);
}
}

#define CHECK(expression) check(static_cast<bool>(expression), #expression, __LINE__)

int main() {
    const auto suffix = std::chrono::high_resolution_clock::now().time_since_epoch().count();
    const std::filesystem::path testDirectory = std::filesystem::temp_directory_path() / ("balatrauto_profile_tests_" + std::to_string(suffix));
    std::filesystem::create_directories(testDirectory);

    const std::filesystem::path missingProfile = testDirectory / "missing.json";
    PlayerProfile defaults;
    std::string error;
    CHECK(!defaults.load(missingProfile.string(), &error));
    CHECK(defaults.jokers.size() == JOKER_NAMES.size());
    CHECK(defaults.decks.size() == DECK_NAMES.size());
    CHECK(defaults.stakes.size() == DECK_NAMES.size());
    CHECK(defaults.decks.at("Red Deck"));
    CHECK(defaults.stakes.at("Red Deck") == NO_STICKER);

    const std::filesystem::path legacyProfile = testDirectory / "legacy.json";
    {
        std::ofstream output(legacyProfile);
        output << R"({
  "jokers": {
    "Joker": true,
    "Hack": false
  },
  "decks": {
    "Red Deck": false,
    "Blue Deck": true
  },
  "stakes": {
    "Blue Deck": "Red Stake"
  }
})";
    }

    PlayerProfile imported;
    CHECK(imported.load(legacyProfile.string(), &error));
    CHECK(error.empty());
    CHECK(imported.jokers.at("Joker").unlocked);
    CHECK(imported.jokers.at("Joker").discovered);
    CHECK(!imported.jokers.at("Hack").unlocked);
    CHECK(imported.decks.at("Red Deck"));
    CHECK(imported.decks.at("Blue Deck"));
    CHECK(imported.stakes.at("Blue Deck") == "Red Stake");
    CHECK(imported.stakes.at("Plasma Deck") == NO_STICKER);

    imported.jokers.at("Greedy Joker") = JokerStatus{true, false};
    imported.stakes.at("Blue Deck") = "Gold Stake";
    const std::filesystem::path savedProfile = testDirectory / "saved.json";
    CHECK(imported.save(savedProfile.string(), &error));
    CHECK(error.empty());

    nlohmann::ordered_json saved;
    {
        std::ifstream input(savedProfile);
        input >> saved;
    }
    CHECK(saved.at("jokers").size() == JOKER_NAMES.size());
    CHECK(saved.at("decks").size() == DECK_NAMES.size());
    CHECK(saved.at("stakes").size() == DECK_NAMES.size());
    CHECK(saved.at("jokers").begin().key() == JOKER_NAMES.front());
    CHECK(saved.at("decks").begin().key() == DECK_NAMES.front());
    CHECK(saved.at("jokers").at("Greedy Joker").at("unlocked").get<bool>());
    CHECK(!saved.at("jokers").at("Greedy Joker").at("discovered").get<bool>());
    CHECK(saved.at("stakes").at("Blue Deck").get<std::string>() == "Gold Stake");

    PlayerProfile roundTrip;
    CHECK(roundTrip.load(savedProfile.string(), &error));
    CHECK(roundTrip.jokers.at("Greedy Joker").unlocked);
    CHECK(!roundTrip.jokers.at("Greedy Joker").discovered);
    CHECK(roundTrip.stakes.at("Blue Deck") == "Gold Stake");

    std::filesystem::remove_all(testDirectory);
    return 0;
}
