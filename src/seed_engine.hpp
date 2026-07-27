#pragma once

#include <array>
#include <atomic>
#include <chrono>
#include <cstdint>
#include <mutex>
#include <string>
#include <thread>
#include <vector>

namespace SeedEngine {
inline constexpr std::size_t JokerCount = 150;
inline constexpr std::size_t SearchShopSlots = 4;

enum class ShopItemKind {
    Joker,
    Tarot,
    Planet,
    Spectral,
    PlayingCard
};

struct ShopItem {
    ShopItemKind kind = ShopItemKind::Joker;
    std::string name;
    std::string edition;
    bool eternal = false;
    bool perishable = false;
    bool rental = false;
};

struct ShopCriterion {
    bool enabled = false;
    ShopItemKind kind = ShopItemKind::Joker;
    std::string name;
};

struct Pack {
    std::string name;
    std::vector<std::string> contents;
};

struct Rules {
    std::array<bool, JokerCount> jokerUnlocked{};
    bool useProfileUnlocks = false;
    bool freshRun = false;
};

struct Analysis {
    bool valid = false;
    std::string error;
    std::string seed;
    std::string deck;
    std::string stake;
    int ante = 1;
    int shopOffset = 0;
    std::string boss;
    std::string voucher;
    std::array<std::string, 2> tags;
    std::vector<ShopItem> shopItems;
    std::vector<Pack> packs;
};

struct SearchObservation {
    int ante = 1;
    std::size_t shopOffset = 0;
    std::string boss;
    std::string voucher;
    std::array<std::string, 2> tags;
    std::array<ShopCriterion, SearchShopSlots> shopSlots;
};

struct SearchQuery {
    std::string startSeed = "AAAAAAAA";
    std::uint64_t seedCount = 250000;
    unsigned int threadCount = 0;
    std::size_t resultLimit = 50;
    std::string deck = "Red Deck";
    std::string stake = "White Stake";
    std::vector<SearchObservation> observations;
    Rules rules;
};

struct SearchSnapshot {
    bool running = false;
    bool finished = false;
    bool cancelled = false;
    std::string error;
    std::uint64_t processed = 0;
    std::uint64_t total = 0;
    double elapsedSeconds = 0.0;
    double seedsPerSecond = 0.0;
    std::vector<std::string> matches;
};

bool isValidSeed(const std::string& seed);
std::string normalizeSeed(std::string seed);
std::uint64_t seedToNumber(const std::string& seed);
std::string numberToSeed(std::uint64_t number);
std::uint64_t seedSpaceSize();
const std::vector<std::string>& bossNames();
const std::vector<std::string>& voucherNames();
const std::vector<std::string>& tagNames();
const std::vector<std::string>& tarotNames();
const std::vector<std::string>& planetNames();
const std::vector<std::string>& spectralNames();
Rules rulesFromUnlockedJokers(const std::vector<std::string>& names);
Analysis analyze(
    const std::string& seed,
    const std::string& deck,
    const std::string& stake,
    int ante,
    int shopSlots,
    int packCount,
    const Rules& rules);
Analysis analyze(
    const std::string& seed,
    const std::string& deck,
    const std::string& stake,
    int ante,
    int shopOffset,
    int shopSlots,
    int packCount,
    const Rules& rules);

class SearchSession {
public:
    SearchSession() = default;
    ~SearchSession();

    SearchSession(const SearchSession&) = delete;
    SearchSession& operator=(const SearchSession&) = delete;

    bool start(const SearchQuery& query, std::string* error = nullptr);
    void cancel();
    SearchSnapshot snapshot() const;

private:
    void joinWorkers();
    void worker(SearchQuery query, std::uint64_t startNumber, std::uint64_t total);

    mutable std::mutex mutex_;
    std::vector<std::thread> workers_;
    std::vector<std::string> matches_;
    std::string error_;
    std::atomic<bool> stopRequested_{false};
    std::atomic<bool> running_{false};
    std::atomic<bool> finished_{false};
    std::atomic<bool> cancelled_{false};
    std::atomic<std::uint64_t> nextOffset_{0};
    std::atomic<std::uint64_t> processed_{0};
    std::atomic<unsigned int> activeWorkers_{0};
    std::uint64_t total_ = 0;
    std::size_t resultLimit_ = 0;
    std::chrono::steady_clock::time_point startedAt_{};
    std::chrono::steady_clock::time_point finishedAt_{};
};
}
