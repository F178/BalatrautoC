#pragma once

#include "profile.hpp"
#include "seed_lab.hpp"
#include "texture_cache.hpp"

#include <filesystem>
#include <functional>
#include <string>

class TrackerApp {
public:
    TrackerApp(
        TextureCache& textures,
        PlayerProfile& profile,
        std::filesystem::path profilePath,
        bool dirty,
        std::string statusMessage,
        bool statusIsError,
        std::function<void()> requestProfileImport,
        std::function<void()> requestImageImport);

    bool draw(float timeSeconds);
    bool save();
    bool importProfile(const std::filesystem::path& sourcePath);
    void importRecognitionImage(const std::filesystem::path& sourcePath);
    bool dirty() const;

private:
    enum class Section {
        Jokers,
        Decks,
        SeedLab
    };

    enum class DeckMode {
        Unlocks,
        Stakes
    };

    void drawHeader();
    void drawJokers();
    void drawDecks();
    void drawJokerDetails();
    void drawDeckDetails();
    void drawStakePopup();
    void markDirty();

    const TextureAsset& jokerTexture(const std::string& name) const;
    const TextureAsset& deckTexture(const std::string& name) const;
    const TextureAsset* stickerTexture(const std::string& deckName) const;

    int discoveredJokerCount() const;
    int unlockedJokerCount() const;
    int unlockedDeckCount() const;

    TextureCache& textures_;
    PlayerProfile& profile_;
    std::filesystem::path profilePath_;
    Section section_ = Section::Jokers;
    DeckMode deckMode_ = DeckMode::Unlocks;
    int jokerPage_ = 0;
    int selectedJoker_ = -1;
    int selectedDeck_ = -1;
    std::string pendingStakeDeck_;
    bool stakePopupRequested_ = false;
    bool dirty_ = false;
    std::string statusMessage_;
    bool statusIsError_ = false;
    std::function<void()> requestProfileImport_;
    SeedLab seedLab_;
};
