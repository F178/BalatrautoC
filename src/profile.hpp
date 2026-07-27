#pragma once

#include <unordered_map>
#include <string>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

struct JokerStatus {
    bool unlocked = false;
    bool discovered = false;
};

inline void to_json(json& j, const JokerStatus& s) {
    j = json{ {"unlocked", s.unlocked}, {"discovered", s.discovered} };
}

inline void from_json(const json& j, JokerStatus& s) {
    j.at("unlocked").get_to(s.unlocked);
    j.at("discovered").get_to(s.discovered);
}

struct PlayerProfile {
    std::unordered_map<std::string, JokerStatus> jokers;
    std::unordered_map<std::string, bool> decks;
    std::unordered_map<std::string, std::string> stakes;

    void reset();
    bool load(const std::string& filepath, std::string* error = nullptr);
    bool save(const std::string& filepath, std::string* error = nullptr) const;
};

extern PlayerProfile playerProfile;
