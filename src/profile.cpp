#include "profile.hpp"

#include "data.hpp"

#include <algorithm>
#include <filesystem>
#include <fstream>
#include <system_error>

PlayerProfile playerProfile;

namespace {
void setError(std::string* error, const std::string& message) {
    if (error) {
        *error = message;
    }
}

bool isKnownStake(const std::string& stake) {
    return stake == NO_STICKER || std::find(STAKE_NAMES.begin(), STAKE_NAMES.end(), stake) != STAKE_NAMES.end();
}
}

void PlayerProfile::reset() {
    jokers.clear();
    decks.clear();
    stakes.clear();

    for (const auto& name : JOKER_NAMES) {
        jokers.emplace(name, JokerStatus{});
    }

    for (const auto& name : DECK_NAMES) {
        decks.emplace(name, name == "Red Deck");
        stakes.emplace(name, NO_STICKER);
    }
}

bool PlayerProfile::load(const std::string& filepath, std::string* error) {
    reset();

    std::ifstream input(filepath);
    if (!input.is_open()) {
        setError(error, "No profile found. A new profile is ready to save.");
        return false;
    }

    json document;
    try {
        input >> document;
    }
    catch (const std::exception& exception) {
        setError(error, std::string("Could not read the profile: ") + exception.what());
        return false;
    }

    const json& jokerValues = document.value("jokers", json::object());
    for (const auto& name : JOKER_NAMES) {
        if (!jokerValues.contains(name)) {
            continue;
        }

        const json& value = jokerValues[name];
        if (value.is_boolean()) {
            const bool unlocked = value.get<bool>();
            jokers[name] = JokerStatus{unlocked, unlocked};
        }
        else if (value.is_object()) {
            JokerStatus status;
            status.unlocked = value.value("unlocked", false);
            status.discovered = status.unlocked && value.value("discovered", status.unlocked);
            jokers[name] = status;
        }
    }

    const json& deckValues = document.value("decks", json::object());
    for (const auto& name : DECK_NAMES) {
        if (deckValues.contains(name) && deckValues[name].is_boolean()) {
            decks[name] = deckValues[name].get<bool>();
        }
    }
    decks["Red Deck"] = true;

    const json& stakeValues = document.value("stakes", json::object());
    for (const auto& name : DECK_NAMES) {
        if (!stakeValues.contains(name) || !stakeValues[name].is_string()) {
            continue;
        }

        const std::string stake = stakeValues[name].get<std::string>();
        if (isKnownStake(stake)) {
            stakes[name] = stake;
        }
    }

    if (error) {
        error->clear();
    }
    return true;
}

bool PlayerProfile::save(const std::string& filepath, std::string* error) const {
    nlohmann::ordered_json document = nlohmann::ordered_json::object();
    nlohmann::ordered_json jokerValues = nlohmann::ordered_json::object();
    nlohmann::ordered_json deckValues = nlohmann::ordered_json::object();
    nlohmann::ordered_json stakeValues = nlohmann::ordered_json::object();

    for (const auto& name : JOKER_NAMES) {
        const auto found = jokers.find(name);
        const JokerStatus status = found != jokers.end() ? found->second : JokerStatus{};
        jokerValues[name] = {
            {"unlocked", status.unlocked},
            {"discovered", status.discovered}
        };
    }

    for (const auto& name : DECK_NAMES) {
        const auto deck = decks.find(name);
        deckValues[name] = name == "Red Deck" || (deck != decks.end() && deck->second);

        const auto stake = stakes.find(name);
        stakeValues[name] = stake != stakes.end() && isKnownStake(stake->second)
            ? stake->second
            : std::string(NO_STICKER);
    }

    document["jokers"] = std::move(jokerValues);
    document["decks"] = std::move(deckValues);
    document["stakes"] = std::move(stakeValues);

    const std::filesystem::path destination(filepath);
    const std::filesystem::path temporary = destination.string() + ".tmp";
    const std::filesystem::path backup = destination.string() + ".bak";

    std::error_code filesystemError;
    if (!destination.parent_path().empty()) {
        std::filesystem::create_directories(destination.parent_path(), filesystemError);
        if (filesystemError) {
            setError(error, "Could not create the profile folder: " + filesystemError.message());
            return false;
        }
    }

    std::ofstream output(temporary, std::ios::trunc);
    if (!output.is_open()) {
        setError(error, "Could not open the profile for writing.");
        return false;
    }

    output << document.dump(2) << '\n';
    output.close();
    if (!output) {
        setError(error, "Could not finish writing the profile.");
        return false;
    }

    if (std::filesystem::exists(destination)) {
        std::filesystem::copy_file(destination, backup, std::filesystem::copy_options::overwrite_existing, filesystemError);
        filesystemError.clear();
    }

    std::filesystem::copy_file(temporary, destination, std::filesystem::copy_options::overwrite_existing, filesystemError);
    std::filesystem::remove(temporary);
    if (filesystemError) {
        setError(error, "Could not replace the profile: " + filesystemError.message());
        return false;
    }

    if (error) {
        error->clear();
    }
    return true;
}
