// profile.hpp
#pragma once

#include <unordered_map>
#include <string>
#include <fstream>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

// Holds the unlock state of everything
struct JokerStatus {
    bool unlocked = false;
    bool discovered = false;
};

// JSON (de)serialization for JokerStatus
inline void to_json(json& j, const JokerStatus& s) {
    j = json{ {"unlocked", s.unlocked}, {"discovered", s.discovered} };
}

inline void from_json(const json& j, JokerStatus& s) {
    j.at("unlocked").get_to(s.unlocked);
    j.at("discovered").get_to(s.discovered);
}

inline std::unordered_map<std::string, JokerStatus> jokers;

struct PlayerProfile {
    std::unordered_map<std::string, JokerStatus> jokers;
    std::unordered_map<std::string, bool> decks;
    std::unordered_map<std::string, std::string> stakes;

    bool load(const std::string& filepath);
    bool save(const std::string& filepath);
};

extern PlayerProfile playerProfile;
