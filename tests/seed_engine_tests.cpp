#include "seed_engine.hpp"
#include "seed_session.hpp"

#include <chrono>
#include <cstdlib>
#include <filesystem>
#include <iostream>
#include <thread>

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
    CHECK(SeedEngine::seedSpaceSize() == 1785793904896ULL);
    CHECK(SeedEngine::isValidSeed("AAAAAAAA"));
    CHECK(SeedEngine::isValidSeed("p1234xyz"));
    CHECK(!SeedEngine::isValidSeed("OAAAAAAA"));
    CHECK(!SeedEngine::isValidSeed("0AAAAAAA"));
    CHECK(!SeedEngine::isValidSeed("TOO_SHORT"));
    CHECK(SeedEngine::seedToNumber("AAAAAAAA") == 0);
    CHECK(SeedEngine::numberToSeed(0) == "AAAAAAAA");
    CHECK(SeedEngine::numberToSeed(SeedEngine::seedSpaceSize() - 1) == "99999999");

    const SeedEngine::Rules rules;
    const SeedEngine::Analysis first = SeedEngine::analyze(
        "AAAAAAA1", "Red Deck", "White Stake", 1, 6, 4, rules);
    const SeedEngine::Analysis second = SeedEngine::analyze(
        "aaaaaaa1", "Red Deck", "White Stake", 1, 6, 4, rules);
    CHECK(first.valid);
    CHECK(second.valid);
    CHECK(first.seed == "AAAAAAA1");
    CHECK(first.boss == second.boss);
    CHECK(first.voucher == second.voucher);
    CHECK(first.tags == second.tags);
    CHECK(first.shopItems.size() == 6);
    CHECK(first.packs.size() == 4);
    CHECK(first.shopItems.front().name == second.shopItems.front().name);

    const SeedEngine::Analysis shifted = SeedEngine::analyze(
        "AAAAAAA1", "Red Deck", "White Stake", 1, 2, 2, 0, rules);
    CHECK(shifted.valid);
    CHECK(shifted.shopOffset == 2);
    CHECK(shifted.shopItems.size() == 2);
    CHECK(shifted.shopItems[0].kind == first.shopItems[2].kind);
    CHECK(shifted.shopItems[0].name == first.shopItems[2].name);
    CHECK(shifted.shopItems[1].kind == first.shopItems[3].kind);
    CHECK(shifted.shopItems[1].name == first.shopItems[3].name);

    SeedEngine::SearchQuery emptyQuery;
    emptyQuery.seedCount = 1;
    SeedEngine::SearchSession emptySearch;
    std::string emptyError;
    CHECK(!emptySearch.start(emptyQuery, &emptyError));
    CHECK(!emptyError.empty());

    SeedEngine::SearchQuery query;
    query.startSeed = "AAAAAAA1";
    query.seedCount = 1;
    query.threadCount = 1;
    query.resultLimit = 1;
    const SeedEngine::Analysis known = SeedEngine::analyze(
        query.startSeed, query.deck, query.stake, 1, 4, 0, query.rules);
    SeedEngine::SearchObservation firstObservation;
    firstObservation.ante = 1;
    firstObservation.boss = known.boss;
    firstObservation.voucher = known.voucher;
    firstObservation.tags[0] = known.tags[0];
    firstObservation.shopSlots[0] = SeedEngine::ShopCriterion{
        true, known.shopItems[0].kind, known.shopItems[0].name};
    SeedEngine::SearchObservation secondObservation;
    secondObservation.ante = 1;
    secondObservation.shopOffset = 2;
    secondObservation.shopSlots[0] = SeedEngine::ShopCriterion{
        true, known.shopItems[2].kind, known.shopItems[2].name};
    query.observations = {firstObservation, secondObservation};

    SeedEngine::SearchSession search;
    std::string error;
    CHECK(search.start(query, &error));
    CHECK(error.empty());
    for (int attempt = 0; attempt < 200 && search.snapshot().running; ++attempt) {
        std::this_thread::sleep_for(std::chrono::milliseconds(5));
    }
    const SeedEngine::SearchSnapshot snapshot = search.snapshot();
    CHECK(snapshot.finished);
    CHECK(snapshot.processed == 1);
    CHECK(snapshot.matches.size() == 1);
    CHECK(snapshot.matches[0] == "AAAAAAA1");

    const auto suffix = std::chrono::high_resolution_clock::now().time_since_epoch().count();
    const std::filesystem::path sessionPath = std::filesystem::temp_directory_path() /
        ("balatrauto_seed_session_" + std::to_string(suffix) + ".json");
    SeedInvestigation investigation;
    investigation.startSeed = "aaaaaaa1";
    investigation.seedCount = 123456;
    investigation.threadCount = 3;
    investigation.resultLimit = 17;
    investigation.deck = "Ghost Deck";
    investigation.stake = "Purple Stake";
    investigation.draft = secondObservation;
    investigation.observations = {firstObservation, secondObservation};
    investigation.shopSize = 3;
    investigation.actions = {
        SeedRunAction{SeedRunActionKind::RerollShop, 1, 0, 1, 3, 3},
        SeedRunAction{SeedRunActionKind::AdvanceAnte, 1, 3, 2, 0, 0}
    };
    SeedRunBranch branch;
    branch.name = "Before reroll";
    branch.draft = firstObservation;
    branch.observations = {firstObservation};
    branch.shopSize = 2;
    branch.actions = {SeedRunAction{SeedRunActionKind::NextShop, 1, 0, 1, 2, 2}};
    investigation.branches = {branch};
    investigation.selectedBranch = 0;
    CHECK(investigation.save(sessionPath.string(), &error));
    SeedInvestigation restored;
    CHECK(restored.load(sessionPath.string(), &error));
    CHECK(restored.startSeed == "AAAAAAA1");
    CHECK(restored.seedCount == 123456);
    CHECK(restored.threadCount == 3);
    CHECK(restored.resultLimit == 17);
    CHECK(restored.deck == "Ghost Deck");
    CHECK(restored.stake == "Purple Stake");
    CHECK(restored.draft.shopOffset == 2);
    CHECK(restored.observations.size() == 2);
    CHECK(restored.observations[1].shopSlots[0].enabled);
    CHECK(restored.observations[1].shopSlots[0].name == known.shopItems[2].name);
    CHECK(restored.shopSize == 3);
    CHECK(restored.actions.size() == 2);
    CHECK(restored.actions[0].kind == SeedRunActionKind::RerollShop);
    CHECK(restored.actions[0].shopOffsetAfter == 3);
    CHECK(restored.actions[1].kind == SeedRunActionKind::AdvanceAnte);
    CHECK(restored.actions[1].anteAfter == 2);
    CHECK(restored.branches.size() == 1);
    CHECK(restored.branches[0].name == "Before reroll");
    CHECK(restored.branches[0].draft.shopOffset == 0);
    CHECK(restored.branches[0].observations.size() == 1);
    CHECK(restored.branches[0].actions.size() == 1);
    CHECK(restored.branches[0].actions[0].shopOffsetAfter == 2);
    CHECK(restored.selectedBranch == 0);
    std::filesystem::remove(sessionPath);
    return 0;
}
