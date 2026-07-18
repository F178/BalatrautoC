#include "profile.hpp"

#include <fstream>
#include <utility>

PlayerProfile playerProfile;

bool PlayerProfile::load(const std::string& filepath) {
    std::ifstream input(filepath);
    if (!input.is_open()) {
        return false;
    }

    try {
        json root;
        input >> root;

        PlayerProfile loaded;

        if (root.contains("jokers") && root["jokers"].is_object()) {
            for (const auto& [name, value] : root["jokers"].items()) {
                if (value.is_boolean()) {
                    const bool state = value.get<bool>();
                    loaded.jokers[name] = JokerStatus{state, state};
                } else if (value.is_object()) {
                    loaded.jokers[name] = value.get<JokerStatus>();
                }
            }
        }

        if (root.contains("decks") && root["decks"].is_object()) {
            loaded.decks = root["decks"].get<decltype(loaded.decks)>();
        }

        if (root.contains("stakes") && root["stakes"].is_object()) {
            loaded.stakes = root["stakes"].get<decltype(loaded.stakes)>();
        }

        *this = std::move(loaded);
        return true;
    } catch (const json::exception&) {
        return false;
    }
}

bool PlayerProfile::save(const std::string& filepath) const {
    std::ofstream output(filepath);
    if (!output.is_open()) {
        return false;
    }

    try {
        const json root = {
            {"jokers", jokers},
            {"decks", decks},
            {"stakes", stakes}
        };
        output << root.dump(4);
        return output.good();
    } catch (const json::exception&) {
        return false;
    }
}
