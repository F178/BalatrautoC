#pragma once

#include "seed_engine.hpp"

#include <cstdint>
#include <string>
#include <vector>

enum class SeedRunActionKind {
    NextShop,
    RerollShop,
    Purchase,
    OpenPack,
    SkipBlind,
    AdvanceAnte
};

struct SeedRunAction {
    SeedRunActionKind kind = SeedRunActionKind::NextShop;
    int anteBefore = 1;
    std::size_t shopOffsetBefore = 0;
    int anteAfter = 1;
    std::size_t shopOffsetAfter = 0;
    int quantity = 0;
};

const char* seedRunActionName(SeedRunActionKind kind);

struct SeedRunBranch {
    std::string name;
    SeedEngine::SearchObservation draft;
    std::vector<SeedEngine::SearchObservation> observations;
    int shopSize = 2;
    std::vector<SeedRunAction> actions;
};

struct SeedInvestigation {
    std::string startSeed = "AAAAAAAA";
    std::uint64_t seedCount = 250000;
    unsigned int threadCount = 0;
    std::size_t resultLimit = 25;
    std::string deck = "Red Deck";
    std::string stake = "White Stake";
    bool useProfileUnlocks = true;
    bool freshRun = true;
    SeedEngine::SearchObservation draft;
    std::vector<SeedEngine::SearchObservation> observations;
    int shopSize = 2;
    std::vector<SeedRunAction> actions;
    std::vector<SeedRunBranch> branches;
    int selectedBranch = -1;

    void reset();
    bool load(const std::string& filepath, std::string* error = nullptr);
    bool save(const std::string& filepath, std::string* error = nullptr) const;
};
