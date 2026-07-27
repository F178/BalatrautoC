#pragma once

#include "profile.hpp"
#include "game_asset_catalog.hpp"
#include "image_matcher.hpp"
#include "image_region_detector.hpp"
#include "seed_engine.hpp"
#include "seed_session.hpp"
#include "texture_cache.hpp"
#include "imgui.h"

#include <array>
#include <cstdint>
#include <filesystem>
#include <functional>
#include <string>
#include <vector>

class SeedLab {
public:
    SeedLab(
        TextureCache& textures,
        std::filesystem::path investigationPath = {},
        std::filesystem::path assetManifestPath = {},
        std::function<void()> requestImageImport = {});
    ~SeedLab();

    void draw(const PlayerProfile& profile);
    void importRecognitionImage(const std::filesystem::path& imagePath);

private:
    enum class Mode {
        Analyze,
        Search,
        Recognize
    };

    void drawAnalyzer(const PlayerProfile& profile);
    void drawSearch(const PlayerProfile& profile);
    void drawRecognition();
    void runRecognition();
    void detectRecognitionRegions();
    void selectRecognitionRegion(int index);
    void captureBalatroWindow();
    bool applyRecognitionMatch(const ImageMatch& match);
    void runAnalysis(const PlayerProfile& profile);
    void startSearch(const PlayerProfile& profile);
    SeedEngine::SearchObservation currentObservation() const;
    void applyObservation(const SeedEngine::SearchObservation& observation);
    bool hasCurrentObservation() const;
    void clearCurrentObservation();
    SeedInvestigation investigation() const;
    void applyInvestigation(const SeedInvestigation& investigation);
    bool saveInvestigation(bool showSuccess);
    void loadInvestigation();
    void recordAction(SeedRunActionKind kind, int nextAnte, int nextOffset, int quantity = 0);
    void undoLastAction();
    SeedRunBranch branchSnapshot(const std::string& name) const;
    void applyBranch(const SeedRunBranch& branch);
    bool createBranch();
    bool updateBranch();
    bool restoreBranch();
    bool deleteBranch();
    const TextureAsset* gameAsset(const std::string& set, const std::string& name);
    bool drawGameAsset(const std::string& set, const std::string& name, float height);
    void drawAssetMetric(const char* label, const std::string& set, const std::string& name, ImU32 color);
    SeedEngine::Rules rulesFor(const PlayerProfile& profile) const;

    Mode mode_ = Mode::Analyze;
    std::array<char, 9> seed_ = {'A', 'A', 'A', 'A', 'A', 'A', 'A', '1', '\0'};
    std::array<char, 9> searchStart_ = {'A', 'A', 'A', 'A', 'A', 'A', 'A', 'A', '\0'};
    int deckIndex_ = 0;
    int stakeIndex_ = 0;
    int ante_ = 1;
    int shopOffset_ = 0;
    int shopSlots_ = 8;
    int packCount_ = 4;
    bool useProfileUnlocks_ = true;
    bool freshRun_ = true;
    SeedEngine::Analysis analysis_;
    bool hasAnalysis_ = false;
    std::string analysisMessage_;
    int observedBoss_ = -1;
    int observedVoucher_ = -1;
    std::array<int, 2> observedTags_ = {-1, -1};
    std::array<int, SeedEngine::SearchShopSlots> observedShopItems_ = {-1, -1, -1, -1};
    std::array<std::array<char, 64>, SeedEngine::SearchShopSlots> shopFilters_{};
    std::vector<SeedEngine::SearchObservation> timeline_;
    int shopSize_ = 2;
    std::vector<SeedRunAction> actions_;
    std::array<char, 64> branchName_{};
    std::vector<SeedRunBranch> branches_;
    int selectedBranch_ = -1;
    std::uint64_t searchCount_ = 250000;
    int threadCount_ = 0;
    int resultLimit_ = 25;
    std::string searchMessage_;
    bool searchMessageIsError_ = false;
    TextureCache& textures_;
    GameAssetCatalog gameAssets_;
    ImageMatcher imageMatcher_;
    std::filesystem::path investigationPath_;
    std::filesystem::path assetManifestPath_;
    std::filesystem::path recognitionImagePath_;
    std::function<void()> requestImageImport_;
    std::vector<ImageMatch> recognitionMatches_;
    struct RecognitionRegion {
        ImageRegion region;
        ImageMatch match;
        float score = 0.0f;
    };
    std::vector<RecognitionRegion> recognitionRegions_;
    int recognitionSetIndex_ = 0;
    int selectedRecognition_ = -1;
    int selectedRecognitionRegion_ = -1;
    int recognitionShopSlot_ = 1;
    int recognitionTagSlot_ = 1;
    ImVec2 recognitionCropStart_ = ImVec2(0.0f, 0.0f);
    ImVec2 recognitionCropEnd_ = ImVec2(1.0f, 1.0f);
    ImVec2 recognitionDragAnchor_ = ImVec2(0.0f, 0.0f);
    bool recognitionDragging_ = false;
    bool resetScroll_ = false;
    std::string recognitionMessage_;
    bool recognitionMessageIsError_ = false;
    SeedEngine::SearchSession search_;
};
