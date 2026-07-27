#include "tracker_app.hpp"

#include "data.hpp"
#include "ui_helpers.hpp"

#include <algorithm>
#include <cstdio>

namespace {
ImTextureRef textureRef(const TextureAsset* asset) {
    return asset && asset->texture
        ? ImTextureRef(reinterpret_cast<ImTextureID>(asset->texture))
        : ImTextureRef();
}

std::string stickerForStake(const std::string& stake) {
    const auto found = std::find_if(
        STAKE_STICKERS.begin(),
        STAKE_STICKERS.end(),
        [&stake](const auto& pair) { return pair.first == stake; });
    return found != STAKE_STICKERS.end() ? found->second : std::string(NO_STICKER);
}

void centerText(const std::string& text) {
    const float width = ImGui::CalcTextSize(text.c_str()).x;
    const float cursor = ImGui::GetCursorPosX();
    ImGui::SetCursorPosX(cursor + std::max(0.0f, (ImGui::GetContentRegionAvail().x - width) * 0.5f));
    ImGui::TextUnformatted(text.c_str());
}
}

TrackerApp::TrackerApp(
    TextureCache& textures,
    PlayerProfile& profile,
    std::filesystem::path profilePath,
    bool dirty,
    std::string statusMessage,
    bool statusIsError,
    std::function<void()> requestProfileImport,
    std::function<void()> requestImageImport)
    : textures_(textures),
      profile_(profile),
      profilePath_(std::move(profilePath)),
      dirty_(dirty),
      statusMessage_(std::move(statusMessage)),
      statusIsError_(statusIsError),
      requestProfileImport_(std::move(requestProfileImport)),
      seedLab_(
          textures_,
          profilePath_.parent_path() / "seed_investigation.json",
          profilePath_.parent_path() / "assets" / "game_reference" / "asset_manifest.json",
          std::move(requestImageImport)) {
}

bool TrackerApp::draw(float timeSeconds) {
    ImGuiIO& io = ImGui::GetIO();
    ImGui::SetNextWindowPos(ImVec2(0.0f, 0.0f), ImGuiCond_Always);
    ImGui::SetNextWindowSize(io.DisplaySize, ImGuiCond_Always);
    ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.0f, 0.0f, 0.0f, 0.0f));
    ImGui::Begin(
        "##balatrauto_root",
        nullptr,
        ImGuiWindowFlags_NoDecoration |
        ImGuiWindowFlags_NoMove |
        ImGuiWindowFlags_NoResize |
        ImGuiWindowFlags_NoSavedSettings |
        ImGuiWindowFlags_NoBringToFrontOnFocus);

    Ui::drawAnimatedBackground(timeSeconds);
    drawHeader();
    ImGui::Separator();

    const float backAreaHeight = 48.0f;
    const float workspaceHeight = std::max(0.0f, ImGui::GetContentRegionAvail().y - backAreaHeight);
    const ImGuiWindowFlags workspaceFlags = section_ == Section::SeedLab
        ? ImGuiWindowFlags_AlwaysVerticalScrollbar
        : ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse;
    ImGui::BeginChild(
        "workspace",
        ImVec2(0.0f, workspaceHeight),
        false,
        workspaceFlags);

    if (section_ == Section::Jokers) {
        drawJokers();
    }
    else if (section_ == Section::Decks) {
        drawDecks();
    }
    else {
        seedLab_.draw(profile_);
    }

    drawStakePopup();
    ImGui::EndChild();

    bool keepRunning = true;
    if (Ui::backButton(std::min(340.0f, ImGui::GetContentRegionAvail().x))) {
        keepRunning = false;
    }

    ImGui::End();
    ImGui::PopStyleColor();
    return keepRunning;
}

bool TrackerApp::save() {
    std::string error;
    if (!profile_.save(profilePath_.string(), &error)) {
        statusMessage_ = error;
        statusIsError_ = true;
        return false;
    }

    dirty_ = false;
    statusMessage_ = "Profile saved";
    statusIsError_ = false;
    return true;
}

bool TrackerApp::importProfile(const std::filesystem::path& sourcePath) {
    PlayerProfile imported;
    std::string error;
    if (!imported.load(sourcePath.string(), &error)) {
        statusMessage_ = error.empty() ? "Profile import failed" : error;
        statusIsError_ = true;
        return false;
    }

    profile_ = std::move(imported);
    selectedJoker_ = -1;
    selectedDeck_ = -1;
    jokerPage_ = 0;
    dirty_ = true;
    statusMessage_ = "Imported " + sourcePath.filename().string();
    statusIsError_ = false;
    return true;
}

void TrackerApp::importRecognitionImage(const std::filesystem::path& sourcePath) {
    section_ = Section::SeedLab;
    seedLab_.importRecognitionImage(sourcePath);
}

bool TrackerApp::dirty() const {
    return dirty_;
}

void TrackerApp::drawHeader() {
    ImGui::BeginChild(
        "header",
        ImVec2(0.0f, 82.0f),
        false,
        ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);

    ImGui::SetCursorPosY(7.0f);
    ImGui::TextUnformatted("BALATRAUTO");
    ImGui::SameLine(0.0f, 28.0f);

    if (Ui::sectionTab("Jokers", section_ == Section::Jokers, ImVec2(128.0f, 40.0f))) {
        section_ = Section::Jokers;
    }
    ImGui::SameLine();
    if (Ui::sectionTab("Decks", section_ == Section::Decks, ImVec2(128.0f, 40.0f))) {
        section_ = Section::Decks;
    }
    ImGui::SameLine();
    if (Ui::sectionTab("Seed Lab", section_ == Section::SeedLab, ImVec2(128.0f, 40.0f))) {
        section_ = Section::SeedLab;
    }

    const float buttonWidth = 106.0f;
    const float buttonGap = 10.0f;
    const float buttonGroupWidth = buttonWidth * 2.0f + buttonGap;
    ImGui::SameLine();
    ImGui::SetCursorPosX(std::max(ImGui::GetCursorPosX(), ImGui::GetWindowWidth() - buttonGroupWidth - 8.0f));
    if (ImGui::Button("Import", ImVec2(buttonWidth, 40.0f)) && requestProfileImport_) {
        requestProfileImport_();
    }
    ImGui::SameLine(0.0f, buttonGap);
    if (Ui::saveButton(dirty_, ImVec2(buttonWidth, 40.0f))) {
        save();
    }

    ImGui::SetCursorPosY(57.0f);
    if (section_ == Section::Jokers) {
        ImGui::TextDisabled(
            "%d unlocked  |  %d discovered  |  %d total",
            unlockedJokerCount(),
            discoveredJokerCount(),
            static_cast<int>(JOKER_NAMES.size()));
    }
    else if (section_ == Section::Decks) {
        ImGui::TextDisabled(
            "%d unlocked  |  %d total",
            unlockedDeckCount(),
            static_cast<int>(DECK_NAMES.size()));
    }
    else {
        ImGui::TextDisabled("Seed analyzer  |  Reverse search");
    }

    if (!statusMessage_.empty()) {
        ImGui::SameLine(0.0f, 24.0f);
        ImGui::PushStyleColor(
            ImGuiCol_Text,
            statusIsError_
                ? ImVec4(1.0f, 0.42f, 0.38f, 1.0f)
                : dirty_
                    ? ImVec4(1.0f, 0.72f, 0.18f, 1.0f)
                    : ImVec4(0.45f, 0.92f, 0.58f, 1.0f));
        ImGui::TextUnformatted(statusMessage_.c_str());
        ImGui::PopStyleColor();
    }

    ImGui::EndChild();
}

void TrackerApp::drawJokers() {
    constexpr int cardsPerPage = 15;
    constexpr int columns = 5;
    constexpr int rows = 3;

    const int totalPages = (static_cast<int>(JOKER_NAMES.size()) + cardsPerPage - 1) / cardsPerPage;
    const int oldPage = jokerPage_;

    ImGui::TextUnformatted("JOKER COLLECTION");
    ImGui::Spacing();

    const float detailHeight = 62.0f;
    const float navHeight = 48.0f;
    const float gridHeight = std::max(330.0f, ImGui::GetContentRegionAvail().y - detailHeight - navHeight - 18.0f);
    const float rowHeight = gridHeight / static_cast<float>(rows);
    const float imageHeight = std::clamp(rowHeight - 16.0f, 96.0f, 158.0f);
    const ImVec2 imageSize(imageHeight * 0.747f, imageHeight);

    const int start = jokerPage_ * cardsPerPage;
    const int end = std::min(start + cardsPerPage, static_cast<int>(JOKER_NAMES.size()));

    if (ImGui::BeginTable("joker_grid", columns, ImGuiTableFlags_SizingStretchSame, ImVec2(0.0f, gridHeight))) {
        for (int row = 0; row < rows; ++row) {
            ImGui::TableNextRow(ImGuiTableRowFlags_None, rowHeight);
            for (int column = 0; column < columns; ++column) {
                ImGui::TableSetColumnIndex(column);
                const int index = start + row * columns + column;
                if (index >= end) {
                    continue;
                }

                const std::string& name = JOKER_NAMES[index];
                const JokerStatus& status = profile_.jokers[name];
                const TextureAsset& texture = jokerTexture(name);
                const std::string state = !status.unlocked
                    ? "Locked"
                    : status.discovered
                        ? "Discovered"
                        : "Undiscovered";
                const std::string tooltip = name + "\n" + state;
                const Ui::CardButtonResult result = Ui::cardButton(
                    ("##joker_" + std::to_string(index)).c_str(),
                    textureRef(&texture),
                    ImTextureRef(),
                    imageSize,
                    selectedJoker_ == index,
                    !status.unlocked,
                    tooltip.c_str());
                if (result.clicked) {
                    selectedJoker_ = index;
                }
            }
        }
        ImGui::EndTable();
    }

    jokerPage_ = Ui::pageNav(jokerPage_, totalPages);
    if (jokerPage_ != oldPage) {
        selectedJoker_ = -1;
    }
    drawJokerDetails();
}

void TrackerApp::drawDecks() {
    constexpr int columns = 5;
    constexpr int rows = 3;

    ImGui::TextUnformatted("DECK COLLECTION");
    ImGui::SameLine(0.0f, 24.0f);
    if (Ui::choiceButton("deck_unlock_mode", "Unlocks", deckMode_ == DeckMode::Unlocks, Ui::Red, ImVec2(120.0f, 34.0f))) {
        deckMode_ = DeckMode::Unlocks;
    }
    ImGui::SameLine();
    if (Ui::choiceButton("deck_stake_mode", "Stake stickers", deckMode_ == DeckMode::Stakes, Ui::Orange, ImVec2(150.0f, 34.0f))) {
        deckMode_ = DeckMode::Stakes;
    }
    ImGui::Spacing();

    const float detailHeight = 66.0f;
    const float gridHeight = std::max(330.0f, ImGui::GetContentRegionAvail().y - detailHeight - 12.0f);
    const float rowHeight = gridHeight / static_cast<float>(rows);
    const float imageHeight = std::clamp(rowHeight - 16.0f, 96.0f, 158.0f);
    const ImVec2 imageSize(imageHeight * 0.747f, imageHeight);

    if (ImGui::BeginTable("deck_grid", columns, ImGuiTableFlags_SizingStretchSame, ImVec2(0.0f, gridHeight))) {
        for (int row = 0; row < rows; ++row) {
            ImGui::TableNextRow(ImGuiTableRowFlags_None, rowHeight);
            for (int column = 0; column < columns; ++column) {
                ImGui::TableSetColumnIndex(column);
                const int index = row * columns + column;
                if (index >= static_cast<int>(DECK_NAMES.size())) {
                    continue;
                }

                const std::string& name = DECK_NAMES[index];
                const bool unlocked = profile_.decks[name];
                const std::string stake = profile_.stakes[name];
                const TextureAsset& texture = deckTexture(name);
                const TextureAsset* sticker = stickerTexture(name);
                const std::string tooltip = name + "\n" + (unlocked ? "Unlocked" : "Locked") + "\n" + stake;
                const Ui::CardButtonResult result = Ui::cardButton(
                    ("##deck_" + std::to_string(index)).c_str(),
                    textureRef(&texture),
                    textureRef(sticker),
                    imageSize,
                    selectedDeck_ == index,
                    !unlocked,
                    tooltip.c_str());

                if (!result.clicked) {
                    continue;
                }

                selectedDeck_ = index;
                if (deckMode_ == DeckMode::Unlocks) {
                    if (name != "Red Deck") {
                        profile_.decks[name] = !unlocked;
                        markDirty();
                    }
                }
                else if (unlocked) {
                    pendingStakeDeck_ = name;
                    stakePopupRequested_ = true;
                }
            }
        }
        ImGui::EndTable();
    }

    drawDeckDetails();
}

void TrackerApp::drawJokerDetails() {
    ImGui::BeginChild(
        "joker_details",
        ImVec2(0.0f, 60.0f),
        false,
        ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);
    ImGui::SetCursorPosY(10.0f);

    if (selectedJoker_ < 0 || selectedJoker_ >= static_cast<int>(JOKER_NAMES.size())) {
        ImGui::TextDisabled("%d of %d Jokers discovered", discoveredJokerCount(), static_cast<int>(JOKER_NAMES.size()));
        ImGui::EndChild();
        return;
    }

    const std::string& name = JOKER_NAMES[selectedJoker_];
    JokerStatus& status = profile_.jokers[name];
    ImGui::TextUnformatted(name.c_str());
    ImGui::SameLine(0.0f, 24.0f);

    if (Ui::choiceButton("joker_locked", "Locked", !status.unlocked, Ui::Dark, ImVec2(112.0f, 34.0f))) {
        status = JokerStatus{};
        markDirty();
    }
    ImGui::SameLine();
    if (Ui::choiceButton("joker_unlocked", "Unlocked", status.unlocked && !status.discovered, Ui::Red, ImVec2(112.0f, 34.0f))) {
        status = JokerStatus{true, false};
        markDirty();
    }
    ImGui::SameLine();
    if (Ui::choiceButton("joker_discovered", "Discovered", status.discovered, Ui::Orange, ImVec2(126.0f, 34.0f))) {
        status = JokerStatus{true, true};
        markDirty();
    }

    ImGui::EndChild();
}

void TrackerApp::drawDeckDetails() {
    ImGui::BeginChild(
        "deck_details",
        ImVec2(0.0f, 62.0f),
        false,
        ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);
    ImGui::SetCursorPosY(11.0f);

    if (selectedDeck_ < 0 || selectedDeck_ >= static_cast<int>(DECK_NAMES.size())) {
        ImGui::TextDisabled("%d of %d decks unlocked", unlockedDeckCount(), static_cast<int>(DECK_NAMES.size()));
        ImGui::EndChild();
        return;
    }

    const std::string& name = DECK_NAMES[selectedDeck_];
    bool& unlocked = profile_.decks[name];
    ImGui::TextUnformatted(name.c_str());
    ImGui::SameLine(0.0f, 24.0f);

    if (deckMode_ == DeckMode::Unlocks) {
        if (name == "Red Deck") {
            Ui::badge("Always unlocked", Ui::Red);
        }
        else {
            if (Ui::choiceButton("deck_locked", "Locked", !unlocked, Ui::Dark, ImVec2(120.0f, 34.0f))) {
                unlocked = false;
                markDirty();
            }
            ImGui::SameLine();
            if (Ui::choiceButton("deck_unlocked", "Unlocked", unlocked, Ui::Red, ImVec2(120.0f, 34.0f))) {
                unlocked = true;
                markDirty();
            }
        }
    }
    else {
        const std::string stake = profile_.stakes[name];
        Ui::badge(stake.c_str(), stake == NO_STICKER ? Ui::Dark : Ui::Orange);
        ImGui::SameLine();
        if (!unlocked) {
            ImGui::BeginDisabled();
        }
        if (ImGui::Button("Choose stake", ImVec2(150.0f, 34.0f))) {
            pendingStakeDeck_ = name;
            stakePopupRequested_ = true;
        }
        if (!unlocked) {
            ImGui::EndDisabled();
        }
    }

    ImGui::EndChild();
}

void TrackerApp::drawStakePopup() {
    if (stakePopupRequested_) {
        ImGui::OpenPopup("Select stake");
        stakePopupRequested_ = false;
    }

    ImGui::SetNextWindowSize(ImVec2(590.0f, 420.0f), ImGuiCond_Appearing);
    if (!ImGui::BeginPopupModal("Select stake", nullptr, ImGuiWindowFlags_NoResize)) {
        return;
    }

    if (pendingStakeDeck_.empty()) {
        ImGui::CloseCurrentPopup();
        ImGui::EndPopup();
        return;
    }

    ImGui::Text("%s", pendingStakeDeck_.c_str());
    ImGui::Separator();
    ImGui::Spacing();

    if (ImGui::BeginTable("stake_grid", 4, ImGuiTableFlags_SizingStretchSame)) {
        for (int index = 0; index < static_cast<int>(STAKE_NAMES.size()); ++index) {
            ImGui::TableNextColumn();
            const std::string& stake = STAKE_NAMES[index];
            const TextureAsset& texture = textures_.get(std::filesystem::path("Icons") / "Stakes" / (stake + ".png"));
            const Ui::CardButtonResult result = Ui::cardButton(
                ("##stake_" + std::to_string(index)).c_str(),
                textureRef(&texture),
                ImTextureRef(),
                ImVec2(58.0f, 58.0f),
                profile_.stakes[pendingStakeDeck_] == stake,
                false,
                stake.c_str());
            centerText(stake);
            if (result.clicked) {
                profile_.stakes[pendingStakeDeck_] = stake;
                markDirty();
                ImGui::CloseCurrentPopup();
            }
        }
        ImGui::EndTable();
    }

    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();
    if (Ui::choiceButton("remove_sticker", "Remove sticker", false, Ui::Red, ImVec2(170.0f, 38.0f))) {
        profile_.stakes[pendingStakeDeck_] = NO_STICKER;
        markDirty();
        ImGui::CloseCurrentPopup();
    }
    ImGui::SameLine();
    if (ImGui::Button("Cancel", ImVec2(120.0f, 38.0f))) {
        ImGui::CloseCurrentPopup();
    }

    ImGui::EndPopup();
}

void TrackerApp::markDirty() {
    dirty_ = true;
    statusMessage_ = "Unsaved changes";
    statusIsError_ = false;
}

const TextureAsset& TrackerApp::jokerTexture(const std::string& name) const {
    const JokerStatus& status = profile_.jokers.at(name);
    const std::string filename = !status.unlocked
        ? "Locked_Joker"
        : status.discovered
            ? name
            : "Undiscovered";
    return textures_.get(std::filesystem::path("Icons") / "Jokers" / (filename + ".png"));
}

const TextureAsset& TrackerApp::deckTexture(const std::string& name) const {
    const std::string filename = profile_.decks.at(name) ? name : "LockedDeck";
    return textures_.get(std::filesystem::path("Icons") / "Decks" / (filename + ".png"));
}

const TextureAsset* TrackerApp::stickerTexture(const std::string& deckName) const {
    if (!profile_.decks.at(deckName)) {
        return nullptr;
    }

    const auto stake = profile_.stakes.find(deckName);
    if (stake == profile_.stakes.end() || stake->second == NO_STICKER) {
        return nullptr;
    }

    const std::string sticker = stickerForStake(stake->second);
    if (sticker == NO_STICKER) {
        return nullptr;
    }
    return &textures_.get(std::filesystem::path("Icons") / "Stickers" / (sticker + ".png"));
}

int TrackerApp::discoveredJokerCount() const {
    return static_cast<int>(std::count_if(
        JOKER_NAMES.begin(),
        JOKER_NAMES.end(),
        [this](const std::string& name) { return profile_.jokers.at(name).discovered; }));
}

int TrackerApp::unlockedJokerCount() const {
    return static_cast<int>(std::count_if(
        JOKER_NAMES.begin(),
        JOKER_NAMES.end(),
        [this](const std::string& name) { return profile_.jokers.at(name).unlocked; }));
}

int TrackerApp::unlockedDeckCount() const {
    return static_cast<int>(std::count_if(
        DECK_NAMES.begin(),
        DECK_NAMES.end(),
        [this](const std::string& name) { return profile_.decks.at(name); }));
}
