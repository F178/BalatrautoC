// profile.cpp
#include "profile.hpp"

PlayerProfile playerProfile;

bool PlayerProfile::load(const std::string& filepath) {
    std::ifstream in(filepath);
    if (!in.is_open()) return false;

    json j;
    in >> j;
    jokers = j["jokers"].get<decltype(jokers)>();
    decks = j["decks"].get<decltype(decks)>();
    stakes = j["stakes"].get<decltype(stakes)>();
    return true;
}

bool PlayerProfile::save(const std::string& filepath) {
    json j;
    j["jokers"] = jokers;
    j["decks"] = decks;
    j["stakes"] = stakes;

    std::ofstream out(filepath);
    if (!out.is_open()) return false;

    out << j.dump(4);
    return true;
}
