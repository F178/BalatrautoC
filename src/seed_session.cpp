#include "seed_session.hpp"

#include <algorithm>
#include <filesystem>
#include <fstream>
#include <nlohmann/json.hpp>
#include <system_error>
#include <utility>

namespace {
using Json = nlohmann::json;
using OrderedJson = nlohmann::ordered_json;

void setError(std::string* error, const std::string& message) {
    if (error) *error = message;
}

const char* kindKey(SeedEngine::ShopItemKind kind) {
    switch (kind) {
        case SeedEngine::ShopItemKind::Joker: return "joker";
        case SeedEngine::ShopItemKind::Tarot: return "tarot";
        case SeedEngine::ShopItemKind::Planet: return "planet";
        case SeedEngine::ShopItemKind::Spectral: return "spectral";
        case SeedEngine::ShopItemKind::PlayingCard: return "playing_card";
    }
    return "joker";
}

bool parseKind(const std::string& value, SeedEngine::ShopItemKind& kind) {
    if (value == "joker") kind = SeedEngine::ShopItemKind::Joker;
    else if (value == "tarot") kind = SeedEngine::ShopItemKind::Tarot;
    else if (value == "planet") kind = SeedEngine::ShopItemKind::Planet;
    else if (value == "spectral") kind = SeedEngine::ShopItemKind::Spectral;
    else if (value == "playing_card") kind = SeedEngine::ShopItemKind::PlayingCard;
    else return false;
    return true;
}

const char* actionKey(SeedRunActionKind kind) {
    switch (kind) {
        case SeedRunActionKind::NextShop: return "next_shop";
        case SeedRunActionKind::RerollShop: return "reroll_shop";
        case SeedRunActionKind::Purchase: return "purchase";
        case SeedRunActionKind::OpenPack: return "open_pack";
        case SeedRunActionKind::SkipBlind: return "skip_blind";
        case SeedRunActionKind::AdvanceAnte: return "advance_ante";
    }
    return "next_shop";
}

bool parseActionKind(const std::string& value, SeedRunActionKind& kind) {
    if (value == "next_shop") kind = SeedRunActionKind::NextShop;
    else if (value == "reroll_shop") kind = SeedRunActionKind::RerollShop;
    else if (value == "purchase") kind = SeedRunActionKind::Purchase;
    else if (value == "open_pack") kind = SeedRunActionKind::OpenPack;
    else if (value == "skip_blind") kind = SeedRunActionKind::SkipBlind;
    else if (value == "advance_ante") kind = SeedRunActionKind::AdvanceAnte;
    else return false;
    return true;
}

OrderedJson observationToJson(const SeedEngine::SearchObservation& observation) {
    OrderedJson result = OrderedJson::object();
    result["ante"] = observation.ante;
    result["shop_offset"] = observation.shopOffset;
    result["boss"] = observation.boss;
    result["voucher"] = observation.voucher;
    result["tags"] = observation.tags;
    result["shop_slots"] = OrderedJson::array();
    for (const SeedEngine::ShopCriterion& slot : observation.shopSlots) {
        if (!slot.enabled) {
            result["shop_slots"].push_back(nullptr);
            continue;
        }
        result["shop_slots"].push_back({{"kind", kindKey(slot.kind)}, {"name", slot.name}});
    }
    return result;
}

SeedEngine::SearchObservation observationFromJson(const Json& value) {
    SeedEngine::SearchObservation result;
    if (!value.is_object()) return result;

    result.ante = std::clamp(value.value("ante", 1), 1, 8);
    result.shopOffset = std::min<std::size_t>(value.value("shop_offset", 0u), 200);
    result.boss = value.value("boss", std::string());
    result.voucher = value.value("voucher", std::string());

    const Json& tags = value.value("tags", Json::array());
    for (std::size_t index = 0; index < result.tags.size() && index < tags.size(); ++index) {
        if (tags[index].is_string()) result.tags[index] = tags[index].get<std::string>();
    }

    const Json& slots = value.value("shop_slots", Json::array());
    for (std::size_t index = 0; index < result.shopSlots.size() && index < slots.size(); ++index) {
        if (!slots[index].is_object()) continue;
        SeedEngine::ShopItemKind kind;
        const std::string kindValue = slots[index].value("kind", std::string());
        const std::string name = slots[index].value("name", std::string());
        if (name.empty() || !parseKind(kindValue, kind)) continue;
        result.shopSlots[index] = SeedEngine::ShopCriterion{true, kind, name};
    }
    return result;
}

OrderedJson actionToJson(const SeedRunAction& action) {
    return {
        {"kind", actionKey(action.kind)},
        {"ante_before", action.anteBefore},
        {"shop_offset_before", action.shopOffsetBefore},
        {"ante_after", action.anteAfter},
        {"shop_offset_after", action.shopOffsetAfter},
        {"quantity", action.quantity}
    };
}

bool actionFromJson(const Json& value, SeedRunAction& action) {
    if (!value.is_object() || !parseActionKind(value.value("kind", std::string()), action.kind)) {
        return false;
    }
    action.anteBefore = std::clamp(value.value("ante_before", 1), 1, 8);
    action.shopOffsetBefore = std::min<std::size_t>(value.value("shop_offset_before", 0u), 200);
    action.anteAfter = std::clamp(value.value("ante_after", action.anteBefore), 1, 8);
    action.shopOffsetAfter = std::min<std::size_t>(value.value("shop_offset_after", action.shopOffsetBefore), 200);
    action.quantity = std::clamp(value.value("quantity", 0), 0, 20);
    return true;
}

OrderedJson branchToJson(const SeedRunBranch& branch) {
    OrderedJson result = OrderedJson::object();
    result["name"] = branch.name;
    result["draft"] = observationToJson(branch.draft);
    result["observations"] = OrderedJson::array();
    for (const SeedEngine::SearchObservation& observation : branch.observations) {
        result["observations"].push_back(observationToJson(observation));
    }
    result["shop_size"] = std::clamp(branch.shopSize, 1, 6);
    result["actions"] = OrderedJson::array();
    for (const SeedRunAction& action : branch.actions) {
        result["actions"].push_back(actionToJson(action));
    }
    return result;
}

bool branchFromJson(const Json& value, SeedRunBranch& branch) {
    if (!value.is_object()) return false;
    branch.name = value.value("name", std::string());
    if (branch.name.empty()) return false;
    if (branch.name.size() > 63) branch.name.resize(63);
    branch.draft = observationFromJson(value.value("draft", Json::object()));
    const Json& observations = value.value("observations", Json::array());
    for (std::size_t index = 0; index < observations.size() && index < 64; ++index) {
        branch.observations.push_back(observationFromJson(observations[index]));
    }
    branch.shopSize = std::clamp(value.value("shop_size", branch.shopSize), 1, 6);
    const Json& actions = value.value("actions", Json::array());
    for (std::size_t index = 0; index < actions.size() && index < 500; ++index) {
        SeedRunAction action;
        if (actionFromJson(actions[index], action)) branch.actions.push_back(action);
    }
    return true;
}
}

const char* seedRunActionName(SeedRunActionKind kind) {
    switch (kind) {
        case SeedRunActionKind::NextShop: return "Next shop";
        case SeedRunActionKind::RerollShop: return "Reroll shop";
        case SeedRunActionKind::Purchase: return "Purchase";
        case SeedRunActionKind::OpenPack: return "Open pack";
        case SeedRunActionKind::SkipBlind: return "Skip blind";
        case SeedRunActionKind::AdvanceAnte: return "Next ante";
    }
    return "Action";
}

void SeedInvestigation::reset() {
    *this = SeedInvestigation{};
}

bool SeedInvestigation::load(const std::string& filepath, std::string* error) {
    reset();
    std::ifstream input(filepath);
    if (!input.is_open()) {
        setError(error, "No saved seed investigation found");
        return false;
    }

    Json document;
    try {
        input >> document;
        startSeed = SeedEngine::normalizeSeed(document.value("start_seed", startSeed));
        if (!SeedEngine::isValidSeed(startSeed)) startSeed = "AAAAAAAA";
        seedCount = std::max<std::uint64_t>(1, document.value("seed_count", seedCount));
        threadCount = std::clamp(document.value("thread_count", threadCount), 0u, 64u);
        resultLimit = std::clamp<std::size_t>(document.value("result_limit", resultLimit), 1, 500);
        deck = document.value("deck", deck);
        stake = document.value("stake", stake);
        useProfileUnlocks = document.value("use_profile_unlocks", useProfileUnlocks);
        freshRun = document.value("fresh_run", freshRun);
        draft = observationFromJson(document.value("draft", Json::object()));
        const Json& savedObservations = document.value("observations", Json::array());
        for (std::size_t index = 0; index < savedObservations.size() && index < 64; ++index) {
            observations.push_back(observationFromJson(savedObservations[index]));
        }
        shopSize = std::clamp(document.value("shop_size", shopSize), 1, 6);
        const Json& savedActions = document.value("actions", Json::array());
        for (std::size_t index = 0; index < savedActions.size() && index < 500; ++index) {
            SeedRunAction action;
            if (actionFromJson(savedActions[index], action)) actions.push_back(action);
        }
        const Json& savedBranches = document.value("branches", Json::array());
        for (std::size_t index = 0; index < savedBranches.size() && index < 50; ++index) {
            SeedRunBranch branch;
            if (branchFromJson(savedBranches[index], branch)) branches.push_back(std::move(branch));
        }
        selectedBranch = std::clamp(document.value("selected_branch", selectedBranch), -1,
            static_cast<int>(branches.size()) - 1);
    }
    catch (const std::exception& exception) {
        reset();
        setError(error, std::string("Could not read the seed investigation: ") + exception.what());
        return false;
    }

    if (error) error->clear();
    return true;
}

bool SeedInvestigation::save(const std::string& filepath, std::string* error) const {
    OrderedJson document = OrderedJson::object();
    document["version"] = 2;
    document["start_seed"] = SeedEngine::isValidSeed(SeedEngine::normalizeSeed(startSeed))
        ? SeedEngine::normalizeSeed(startSeed)
        : std::string("AAAAAAAA");
    document["seed_count"] = std::max<std::uint64_t>(1, seedCount);
    document["thread_count"] = std::min(threadCount, 64u);
    document["result_limit"] = std::clamp<std::size_t>(resultLimit, 1, 500);
    document["deck"] = deck;
    document["stake"] = stake;
    document["use_profile_unlocks"] = useProfileUnlocks;
    document["fresh_run"] = freshRun;
    document["draft"] = observationToJson(draft);
    document["observations"] = OrderedJson::array();
    for (const SeedEngine::SearchObservation& observation : observations) {
        document["observations"].push_back(observationToJson(observation));
    }
    document["shop_size"] = std::clamp(shopSize, 1, 6);
    document["actions"] = OrderedJson::array();
    for (const SeedRunAction& action : actions) {
        document["actions"].push_back(actionToJson(action));
    }
    document["branches"] = OrderedJson::array();
    for (const SeedRunBranch& branch : branches) {
        document["branches"].push_back(branchToJson(branch));
    }
    document["selected_branch"] = std::clamp(selectedBranch, -1, static_cast<int>(branches.size()) - 1);

    const std::filesystem::path destination(filepath);
    const std::filesystem::path temporary = destination.string() + ".tmp";
    const std::filesystem::path backup = destination.string() + ".bak";
    std::error_code filesystemError;
    if (!destination.parent_path().empty()) {
        std::filesystem::create_directories(destination.parent_path(), filesystemError);
        if (filesystemError) {
            setError(error, "Could not create the investigation folder: " + filesystemError.message());
            return false;
        }
    }

    std::ofstream output(temporary, std::ios::trunc);
    if (!output.is_open()) {
        setError(error, "Could not open the seed investigation for writing");
        return false;
    }
    output << document.dump(2) << '\n';
    output.close();
    if (!output) {
        setError(error, "Could not finish writing the seed investigation");
        return false;
    }

    if (std::filesystem::exists(destination)) {
        std::filesystem::copy_file(destination, backup, std::filesystem::copy_options::overwrite_existing, filesystemError);
        filesystemError.clear();
    }
    std::filesystem::copy_file(temporary, destination, std::filesystem::copy_options::overwrite_existing, filesystemError);
    std::filesystem::remove(temporary);
    if (filesystemError) {
        setError(error, "Could not replace the seed investigation: " + filesystemError.message());
        return false;
    }
    if (error) error->clear();
    return true;
}
