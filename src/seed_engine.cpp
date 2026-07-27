#include "seed_engine.hpp"

#include "instance.hpp"
#include "items_to_string.hpp"

#include <algorithm>
#include <cctype>
#include <iterator>
#include <limits>
#include <sstream>

namespace {
constexpr char Alphabet[] = "ABCDEFGHIJKLMNPQRSTUVWXYZ123456789";
constexpr std::uint64_t AlphabetSize = sizeof(Alphabet) - 1;

std::string profileJokerName(std::string name) {
    if (name == "Wily Joker") return "Willy Joker";
    if (name == "Mail In Rebate") return "Mail-In Rebate";
    if (name == "To the Moon") return "To The Moon";
    if (name == "Drivers License") return "Driver's License";
    if (name == "Canio") return "Caino";
    return name;
}

void applyRules(Instance::Instance& instance, const SeedEngine::Rules& rules) {
    if (!rules.useProfileUnlocks) {
        return;
    }

    for (std::size_t index = 0; index < rules.jokerUnlocked.size(); ++index) {
        if (!rules.jokerUnlocked[index]) {
            instance.setJokerUnlocked(static_cast<Items::Joker>(index), false);
        }
    }
}

SeedEngine::ShopItem convertShopItem(const Items::OptimizedShopItem& item) {
    SeedEngine::ShopItem result;
    switch (item.type) {
        case Items::OptimizedShopItem::Type::JOKER:
            result.kind = SeedEngine::ShopItemKind::Joker;
            result.name = profileJokerName(Items::toString(item.item.joker));
            result.edition = Items::toString(item.joker_data.edition);
            result.eternal = item.joker_data.eternal;
            result.perishable = item.joker_data.perishable;
            result.rental = item.joker_data.rental;
            break;
        case Items::OptimizedShopItem::Type::TAROT:
            result.kind = SeedEngine::ShopItemKind::Tarot;
            result.name = Items::toString(item.item.tarot);
            break;
        case Items::OptimizedShopItem::Type::PLANET:
            result.kind = SeedEngine::ShopItemKind::Planet;
            result.name = Items::toString(item.item.planet);
            break;
        case Items::OptimizedShopItem::Type::SPECTRAL:
            result.kind = SeedEngine::ShopItemKind::Spectral;
            result.name = Items::toString(item.item.spectral);
            break;
        case Items::OptimizedShopItem::Type::PLAYING_CARD:
            result.kind = SeedEngine::ShopItemKind::PlayingCard;
            result.name = "Playing Card";
            break;
    }
    return result;
}

std::string cardName(const Items::CardEnum& card) {
    static constexpr const char* suits[] = {"Clubs", "Diamonds", "Hearts", "Spades"};
    static constexpr const char* ranks[] = {
        "2", "3", "4", "5", "6", "7", "8", "9", "Ace", "Jack", "King", "Queen", "10"
    };
    if (card.base.size() != 3 || card.base[1] != '_') {
        return card.base;
    }

    const std::string suitCodes = "CDHS";
    const std::string rankCodes = "23456789AJKQT";
    const std::size_t suit = suitCodes.find(card.base[0]);
    const std::size_t rank = rankCodes.find(card.base[2]);
    if (suit == std::string::npos || rank == std::string::npos) {
        return card.base;
    }

    std::string result = std::string(ranks[rank]) + " of " + suits[suit];
    const std::string enhancement = Items::toString(card.enhancement);
    const std::string edition = Items::toString(card.edition);
    const std::string seal = Items::toString(card.seal);
    if (enhancement != "No Enhancement") result += " | " + enhancement;
    if (edition != "No Edition") result += " | " + edition;
    if (seal != "No Seal") result += " | " + seal;
    return result;
}

SeedEngine::Pack generatePack(Instance::Instance& instance, int ante) {
    SeedEngine::Pack result;
    const Items::Pack pack = instance.nextPack_enum(ante);
    result.name = Items::toString(pack);
    const Items::NextPackData data = Items::convertPackData(pack);

    switch (data.type) {
        case Items::Pack::ARCANA_PACK: {
            const auto contents = instance.nextArcanaPack_enum(data.size, ante);
            for (std::size_t index = 0; index < contents.isSpectral.size(); ++index) {
                result.contents.emplace_back(contents.isSpectral[index]
                    ? Items::toString(contents.spectrals[index])
                    : Items::toString(contents.tarots[index]));
            }
            break;
        }
        case Items::Pack::CELESTIAL_PACK: {
            for (const Items::Planet item : instance.nextCelestialPack_enum(data.size, ante)) {
                result.contents.emplace_back(Items::toString(item));
            }
            break;
        }
        case Items::Pack::STANDARD_PACK: {
            for (const Items::CardEnum& item : instance.nextStandardPack_enum(data.size, ante)) {
                result.contents.push_back(cardName(item));
            }
            break;
        }
        case Items::Pack::BUFFOON_PACK: {
            for (const Items::OptimizedJokerData& item : instance.nextBuffoonPack_enum(data.size, ante)) {
                std::string label = profileJokerName(Items::toString(item.joker));
                const std::string edition = Items::toString(item.edition);
                if (edition != "No Edition") label = edition + " " + label;
                if (item.eternal) label += " | Eternal";
                if (item.perishable) label += " | Perishable";
                if (item.rental) label += " | Rental";
                result.contents.push_back(std::move(label));
            }
            break;
        }
        case Items::Pack::SPECTRAL_PACK: {
            for (const Items::Spectral item : instance.nextSpectralPack_enum(data.size, ante)) {
                result.contents.emplace_back(Items::toString(item));
            }
            break;
        }
        default:
            break;
    }
    return result;
}

bool hasCriterion(const SeedEngine::SearchObservation& observation) {
    if (!observation.boss.empty() || !observation.voucher.empty()) {
        return true;
    }
    if (std::any_of(observation.tags.begin(), observation.tags.end(), [](const std::string& value) { return !value.empty(); })) {
        return true;
    }
    return std::any_of(
        observation.shopSlots.begin(),
        observation.shopSlots.end(),
        [](const SeedEngine::ShopCriterion& value) { return value.enabled; });
}

bool matchesObservation(
    const std::string& seed,
    const SeedEngine::SearchQuery& query,
    const SeedEngine::SearchObservation& observation) {
    Instance::Instance instance(seed);
    instance.setDeck(query.deck);
    instance.setStake(query.stake);
    instance.initLocks(1, false, query.rules.freshRun);
    applyRules(instance, query.rules);

    const int ante = std::clamp(observation.ante, 1, 8);
    if (ante > 1) {
        generatePack(instance, 1);
    }

    for (int currentAnte = 1; currentAnte <= ante; ++currentAnte) {
        instance.initUnlocks(currentAnte, false);
        const std::string boss = Items::toString(instance.nextBoss_enum(currentAnte));
        if (currentAnte != ante) {
            continue;
        }

        const std::string voucher = Items::toString(instance.nextVoucher_enum(currentAnte));
        const std::array<std::string, 2> tags = {
            Items::toString(instance.nextTag_enum(currentAnte)),
            Items::toString(instance.nextTag_enum(currentAnte))
        };
        if (!observation.boss.empty() && boss != observation.boss) {
            return false;
        }
        if (!observation.voucher.empty() && voucher != observation.voucher) {
            return false;
        }
        for (std::size_t index = 0; index < observation.tags.size(); ++index) {
            if (!observation.tags[index].empty() && tags[index] != observation.tags[index]) {
                return false;
            }
        }

        const std::size_t offset = std::min<std::size_t>(observation.shopOffset, 200);
        for (std::size_t index = 0; index < offset; ++index) {
            instance.nextShopItem_enum(currentAnte);
        }
        for (const SeedEngine::ShopCriterion& expected : observation.shopSlots) {
            const SeedEngine::ShopItem actual = convertShopItem(instance.nextShopItem_enum(currentAnte));
            if (expected.enabled && (actual.kind != expected.kind || actual.name != expected.name)) {
                return false;
            }
        }
        return true;
    }
    return false;
}

bool matchesQuery(const std::string& seed, const SeedEngine::SearchQuery& query) {
    return std::all_of(
        query.observations.begin(),
        query.observations.end(),
        [&seed, &query](const SeedEngine::SearchObservation& observation) {
            return matchesObservation(seed, query, observation);
        });
}
}

namespace SeedEngine {
bool isValidSeed(const std::string& seed) {
    return seed.size() == 8 && std::all_of(seed.begin(), seed.end(), [](char value) {
        const char upper = static_cast<char>(std::toupper(static_cast<unsigned char>(value)));
        return std::find(std::begin(Alphabet), std::end(Alphabet) - 1, upper) != std::end(Alphabet) - 1;
    });
}

std::string normalizeSeed(std::string seed) {
    std::transform(seed.begin(), seed.end(), seed.begin(), [](char value) {
        return static_cast<char>(std::toupper(static_cast<unsigned char>(value)));
    });
    return seed;
}

std::uint64_t seedToNumber(const std::string& seed) {
    const std::string normalized = normalizeSeed(seed);
    if (!isValidSeed(normalized)) {
        return std::numeric_limits<std::uint64_t>::max();
    }

    std::uint64_t result = 0;
    for (const char value : normalized) {
        const char* found = std::find(std::begin(Alphabet), std::end(Alphabet) - 1, value);
        result = result * AlphabetSize + static_cast<std::uint64_t>(found - std::begin(Alphabet));
    }
    return result;
}

std::string numberToSeed(std::uint64_t number) {
    number %= seedSpaceSize();
    std::string result(8, Alphabet[0]);
    for (int index = 7; index >= 0; --index) {
        result[static_cast<std::size_t>(index)] = Alphabet[number % AlphabetSize];
        number /= AlphabetSize;
    }
    return result;
}

std::uint64_t seedSpaceSize() {
    std::uint64_t result = 1;
    for (int index = 0; index < 8; ++index) {
        result *= AlphabetSize;
    }
    return result;
}

const std::vector<std::string>& bossNames() {
    static const std::vector<std::string> names(Items::BOSS_NAMES.begin(), Items::BOSS_NAMES.end());
    return names;
}

const std::vector<std::string>& voucherNames() {
    static const std::vector<std::string> names(Items::VOUCHER_NAMES.begin(), Items::VOUCHER_NAMES.end());
    return names;
}

const std::vector<std::string>& tagNames() {
    static const std::vector<std::string> names(Items::TAG_NAMES.begin(), Items::TAG_NAMES.end());
    return names;
}

const std::vector<std::string>& tarotNames() {
    static const std::vector<std::string> names(Items::TAROT_NAMES.begin(), Items::TAROT_NAMES.end());
    return names;
}

const std::vector<std::string>& planetNames() {
    static const std::vector<std::string> names(Items::PLANET_NAMES.begin(), Items::PLANET_NAMES.end());
    return names;
}

const std::vector<std::string>& spectralNames() {
    static const std::vector<std::string> names(Items::SPECTRAL_NAMES.begin(), Items::SPECTRAL_NAMES.end());
    return names;
}

Rules rulesFromUnlockedJokers(const std::vector<std::string>& names) {
    Rules rules;
    rules.useProfileUnlocks = true;
    for (std::size_t index = 0; index < JokerCount; ++index) {
        const std::string engineName = profileJokerName(Items::toString(static_cast<Items::Joker>(index)));
        rules.jokerUnlocked[index] = std::find(names.begin(), names.end(), engineName) != names.end();
    }
    return rules;
}

Analysis analyze(
    const std::string& seed,
    const std::string& deck,
    const std::string& stake,
    int ante,
    int shopSlots,
    int packCount,
    const Rules& rules) {
    return analyze(seed, deck, stake, ante, 0, shopSlots, packCount, rules);
}

Analysis analyze(
    const std::string& seed,
    const std::string& deck,
    const std::string& stake,
    int ante,
    int shopOffset,
    int shopSlots,
    int packCount,
    const Rules& rules) {
    Analysis result;
    result.seed = normalizeSeed(seed);
    result.deck = deck;
    result.stake = stake;
    result.ante = std::clamp(ante, 1, 8);
    result.shopOffset = std::clamp(shopOffset, 0, 200);
    if (!isValidSeed(result.seed)) {
        result.error = "Seed must be 8 characters using A-N, P-Z, or 1-9";
        return result;
    }

    Instance::Instance instance(result.seed);
    instance.setDeck(deck);
    instance.setStake(stake);
    instance.initLocks(1, false, rules.freshRun);
    applyRules(instance, rules);

    if (result.ante > 1) {
        generatePack(instance, 1);
    }

    for (int currentAnte = 1; currentAnte <= result.ante; ++currentAnte) {
        instance.initUnlocks(currentAnte, false);
        const Items::Boss boss = instance.nextBoss_enum(currentAnte);
        if (currentAnte != result.ante) {
            continue;
        }

        result.boss = Items::toString(boss);
        result.voucher = Items::toString(instance.nextVoucher_enum(currentAnte));
        result.tags[0] = Items::toString(instance.nextTag_enum(currentAnte));
        result.tags[1] = Items::toString(instance.nextTag_enum(currentAnte));

        for (int index = 0; index < result.shopOffset; ++index) {
            instance.nextShopItem_enum(currentAnte);
        }
        const int limitedShopSlots = std::clamp(shopSlots, 0, 20);
        result.shopItems.reserve(static_cast<std::size_t>(limitedShopSlots));
        for (int index = 0; index < limitedShopSlots; ++index) {
            result.shopItems.push_back(convertShopItem(instance.nextShopItem_enum(currentAnte)));
        }

        const int limitedPackCount = std::clamp(packCount, 0, 6);
        result.packs.reserve(static_cast<std::size_t>(limitedPackCount));
        for (int index = 0; index < limitedPackCount; ++index) {
            result.packs.push_back(generatePack(instance, currentAnte));
        }
    }

    result.valid = true;
    return result;
}

SearchSession::~SearchSession() {
    cancel();
    joinWorkers();
}

bool SearchSession::start(const SearchQuery& query, std::string* error) {
    cancel();
    joinWorkers();

    SearchQuery normalized = query;
    normalized.startSeed = normalizeSeed(normalized.startSeed);
    if (!isValidSeed(normalized.startSeed)) {
        if (error) *error = "Start seed must be 8 characters using A-N, P-Z, or 1-9";
        return false;
    }
    normalized.observations.erase(
        std::remove_if(
            normalized.observations.begin(),
            normalized.observations.end(),
            [](const SearchObservation& observation) { return !hasCriterion(observation); }),
        normalized.observations.end());
    if (normalized.observations.empty()) {
        if (error) *error = "Choose at least one observed clue";
        return false;
    }
    if (normalized.seedCount == 0) {
        if (error) *error = "Search size must be greater than zero";
        return false;
    }

    const std::uint64_t startNumber = seedToNumber(normalized.startSeed);
    const std::uint64_t available = seedSpaceSize() - startNumber;
    total_ = std::min(normalized.seedCount, available);
    resultLimit_ = std::clamp<std::size_t>(normalized.resultLimit, 1, 500);
    normalized.threadCount = normalized.threadCount == 0
        ? std::max(1u, std::thread::hardware_concurrency())
        : std::clamp(normalized.threadCount, 1u, 64u);

    {
        std::lock_guard<std::mutex> lock(mutex_);
        matches_.clear();
        error_.clear();
    }
    stopRequested_ = false;
    running_ = true;
    finished_ = false;
    cancelled_ = false;
    nextOffset_ = 0;
    processed_ = 0;
    activeWorkers_ = normalized.threadCount;
    startedAt_ = std::chrono::steady_clock::now();
    finishedAt_ = {};

    try {
        workers_.reserve(normalized.threadCount);
        for (unsigned int index = 0; index < normalized.threadCount; ++index) {
            workers_.emplace_back(&SearchSession::worker, this, normalized, startNumber, total_);
        }
    }
    catch (const std::exception& exception) {
        stopRequested_ = true;
        joinWorkers();
        running_ = false;
        std::lock_guard<std::mutex> lock(mutex_);
        error_ = exception.what();
        if (error) *error = error_;
        return false;
    }

    if (error) error->clear();
    return true;
}

void SearchSession::cancel() {
    if (running_) {
        cancelled_ = true;
    }
    stopRequested_ = true;
}

SearchSnapshot SearchSession::snapshot() const {
    SearchSnapshot result;
    result.running = running_;
    result.finished = finished_;
    result.cancelled = cancelled_;
    result.processed = processed_;
    result.total = total_;
    {
        std::lock_guard<std::mutex> lock(mutex_);
        result.matches = matches_;
        result.error = error_;
    }

    const auto end = running_ ? std::chrono::steady_clock::now() : finishedAt_;
    if (startedAt_.time_since_epoch().count() != 0 && end.time_since_epoch().count() != 0) {
        result.elapsedSeconds = std::chrono::duration<double>(end - startedAt_).count();
    }
    if (result.elapsedSeconds > 0.0) {
        result.seedsPerSecond = static_cast<double>(result.processed) / result.elapsedSeconds;
    }
    return result;
}

void SearchSession::joinWorkers() {
    for (std::thread& workerThread : workers_) {
        if (workerThread.joinable()) {
            workerThread.join();
        }
    }
    workers_.clear();
}

void SearchSession::worker(SearchQuery query, std::uint64_t startNumber, std::uint64_t total) {
    while (!stopRequested_) {
        const std::uint64_t offset = nextOffset_.fetch_add(1);
        if (offset >= total) {
            break;
        }

        const std::string seed = numberToSeed(startNumber + offset);
        if (matchesQuery(seed, query)) {
            std::lock_guard<std::mutex> lock(mutex_);
            if (matches_.size() < resultLimit_) {
                matches_.push_back(seed);
            }
            if (matches_.size() >= resultLimit_) {
                stopRequested_ = true;
            }
        }
        processed_.fetch_add(1);
    }

    if (activeWorkers_.fetch_sub(1) == 1) {
        finishedAt_ = std::chrono::steady_clock::now();
        running_ = false;
        finished_ = true;
    }
}
}
