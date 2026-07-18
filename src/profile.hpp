#pragma once

#include <string>
#include <unordered_map>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

struct JokerStatus {
    bool unlocked = false;
    bool discovered = false;
};

inline void to_json(json& value, const JokerStatus& status) {
    value = json{
        {"unlocked", status.unlocked},
        {"discovered", status.discovered}
    };
}

inline void from_json(const json& value, JokerStatus& status) {
    status.unlocked = value.value("unlocked", false);
    status.discovered = value.value("discovered", false);
}

struct PlayerProfile {
    std::unordered_map<std::string, JokerStatus> jokers;
    std::unordered_map<std::string, bool> decks;
    std::unordered_map<std::string, std::string> stakes;

    bool load(const std::string& filepath);
    bool save(const std::string& filepath) const;
};

extern PlayerProfile playerProfile;
