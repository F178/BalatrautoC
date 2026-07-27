#include "seed_lab.hpp"

#include "data.hpp"
#include "ui_helpers.hpp"
#include "window_capture.hpp"
#include "stb_image.h"

#include <algorithm>
#include <cctype>
#include <cstdio>
#include <optional>
#include <vector>

namespace {
std::string trim(const char* value) {
    std::string result = value ? value : "";
    const auto first = std::find_if_not(result.begin(), result.end(), [](unsigned char character) {
        return std::isspace(character) != 0;
    });
    const auto last = std::find_if_not(result.rbegin(), result.rend(), [](unsigned char character) {
        return std::isspace(character) != 0;
    }).base();
    if (first >= last) return {};
    return std::string(first, last);
}

const char* shopKindName(SeedEngine::ShopItemKind kind) {
    switch (kind) {
        case SeedEngine::ShopItemKind::Joker: return "Joker";
        case SeedEngine::ShopItemKind::Tarot: return "Tarot";
        case SeedEngine::ShopItemKind::Planet: return "Planet";
        case SeedEngine::ShopItemKind::Spectral: return "Spectral";
        case SeedEngine::ShopItemKind::PlayingCard: return "Playing Card";
    }
    return "Item";
}

const char* shopAssetSet(SeedEngine::ShopItemKind kind) {
    switch (kind) {
        case SeedEngine::ShopItemKind::Joker: return "Joker";
        case SeedEngine::ShopItemKind::Tarot: return "Tarot";
        case SeedEngine::ShopItemKind::Planet: return "Planet";
        case SeedEngine::ShopItemKind::Spectral: return "Spectral";
        case SeedEngine::ShopItemKind::PlayingCard: return "Playing Card";
    }
    return "";
}

const std::vector<std::string>& recognitionSets() {
    static const std::vector<std::string> sets = {
        "Any",
        "Joker",
        "Tarot",
        "Planet",
        "Spectral",
        "Voucher",
        "Tag",
        "Blind",
        "Booster",
        "Back",
        "Stake",
        "Playing Card"
    };
    return sets;
}

bool isRecognizableSet(const std::string& set) {
    const std::vector<std::string>& sets = recognitionSets();
    return std::find(sets.begin() + 1, sets.end(), set) != sets.end();
}

std::optional<SeedEngine::ShopItemKind> shopKindForSet(const std::string& set) {
    if (set == "Joker") return SeedEngine::ShopItemKind::Joker;
    if (set == "Tarot") return SeedEngine::ShopItemKind::Tarot;
    if (set == "Planet") return SeedEngine::ShopItemKind::Planet;
    if (set == "Spectral") return SeedEngine::ShopItemKind::Spectral;
    if (set == "Playing Card") return SeedEngine::ShopItemKind::PlayingCard;
    return std::nullopt;
}

bool vectorCombo(const char* label, int& index, const std::vector<std::string>& values, float width) {
    bool changed = false;
    ImGui::SetNextItemWidth(width);
    if (ImGui::BeginCombo(label, values[static_cast<std::size_t>(index)].c_str())) {
        for (int item = 0; item < static_cast<int>(values.size()); ++item) {
            const bool selected = item == index;
            if (ImGui::Selectable(values[static_cast<std::size_t>(item)].c_str(), selected)) {
                index = item;
                changed = true;
            }
            if (selected) {
                ImGui::SetItemDefaultFocus();
            }
        }
        ImGui::EndCombo();
    }
    return changed;
}

std::string stickerText(const SeedEngine::ShopItem& item) {
    std::string value;
    if (item.eternal) value += "Eternal";
    if (item.perishable) value += value.empty() ? "Perishable" : " | Perishable";
    if (item.rental) value += value.empty() ? "Rental" : " | Rental";
    return value.empty() ? "-" : value;
}

struct ShopChoice {
    SeedEngine::ShopItemKind kind;
    std::string name;
    std::string label;
};

void appendShopChoices(
    std::vector<ShopChoice>& choices,
    SeedEngine::ShopItemKind kind,
    const char* category,
    const std::vector<std::string>& names) {
    for (const std::string& name : names) {
        choices.push_back(ShopChoice{kind, name, std::string(category) + " | " + name});
    }
}

const std::vector<ShopChoice>& shopChoices() {
    static const std::vector<ShopChoice> choices = [] {
        std::vector<ShopChoice> result;
        result.reserve(JOKER_NAMES.size() + 56);
        appendShopChoices(result, SeedEngine::ShopItemKind::Joker, "Joker", JOKER_NAMES);
        appendShopChoices(result, SeedEngine::ShopItemKind::Tarot, "Tarot", SeedEngine::tarotNames());
        appendShopChoices(result, SeedEngine::ShopItemKind::Planet, "Planet", SeedEngine::planetNames());
        appendShopChoices(result, SeedEngine::ShopItemKind::Spectral, "Spectral", SeedEngine::spectralNames());
        result.push_back(ShopChoice{SeedEngine::ShopItemKind::PlayingCard, "Playing Card", "Playing Card"});
        return result;
    }();
    return choices;
}

bool containsInsensitive(const std::string& value, const char* filter) {
    if (!filter || !*filter) {
        return true;
    }
    const std::string needle(filter);
    return std::search(
        value.begin(),
        value.end(),
        needle.begin(),
        needle.end(),
        [](char left, char right) {
            return std::tolower(static_cast<unsigned char>(left)) ==
                std::tolower(static_cast<unsigned char>(right));
        }) != value.end();
}

int stringIndex(const std::vector<std::string>& values, const std::string& value) {
    const auto found = std::find(values.begin(), values.end(), value);
    return found == values.end() ? -1 : static_cast<int>(found - values.begin());
}

bool optionalCombo(
    const char* label,
    int& index,
    const std::vector<std::string>& values,
    const char* anyLabel = "Any") {
    bool changed = false;
    const char* preview = index < 0 ? anyLabel : values[static_cast<std::size_t>(index)].c_str();
    ImGui::SetNextItemWidth(-1.0f);
    if (ImGui::BeginCombo(label, preview)) {
        const bool anySelected = index < 0;
        if (ImGui::Selectable(anyLabel, anySelected)) {
            index = -1;
            changed = true;
        }
        for (int item = 0; item < static_cast<int>(values.size()); ++item) {
            const bool selected = item == index;
            if (ImGui::Selectable(values[static_cast<std::size_t>(item)].c_str(), selected)) {
                index = item;
                changed = true;
            }
            if (selected) {
                ImGui::SetItemDefaultFocus();
            }
        }
        ImGui::EndCombo();
    }
    return changed;
}

bool shopItemCombo(int slot, int& index, std::array<char, 64>& filter) {
    const std::vector<ShopChoice>& choices = shopChoices();
    const char* preview = index < 0 ? "Any item" : choices[static_cast<std::size_t>(index)].label.c_str();
    char label[32];
    char filterId[32];
    std::snprintf(label, sizeof(label), "Slot %d", slot + 1);
    std::snprintf(filterId, sizeof(filterId), "##shop_filter_%d", slot);

    bool changed = false;
    ImGui::SetNextItemWidth(-1.0f);
    ImGui::SetNextWindowSizeConstraints(ImVec2(260.0f, 0.0f), ImVec2(520.0f, 340.0f));
    if (ImGui::BeginCombo(label, preview)) {
        ImGui::SetNextItemWidth(-1.0f);
        ImGui::InputTextWithHint(filterId, "Filter items", filter.data(), filter.size());
        ImGui::Separator();
        if (ImGui::Selectable("Any item", index < 0)) {
            index = -1;
            filter.fill('\0');
            changed = true;
        }
        for (int item = 0; item < static_cast<int>(choices.size()); ++item) {
            const ShopChoice& choice = choices[static_cast<std::size_t>(item)];
            if (!containsInsensitive(choice.label, filter.data())) {
                continue;
            }
            const bool selected = item == index;
            if (ImGui::Selectable(choice.label.c_str(), selected)) {
                index = item;
                filter.fill('\0');
                changed = true;
            }
            if (selected) {
                ImGui::SetItemDefaultFocus();
            }
        }
        ImGui::EndCombo();
    }
    return changed;
}

std::string observationSummary(const SeedEngine::SearchObservation& observation) {
    std::string result;
    const auto append = [&result](const std::string& value) {
        if (value.empty()) return;
        if (!result.empty()) result += "  |  ";
        result += value;
    };
    if (!observation.boss.empty()) append("Boss: " + observation.boss);
    if (!observation.voucher.empty()) append("Voucher: " + observation.voucher);
    if (!observation.tags[0].empty()) append("Tag 1: " + observation.tags[0]);
    if (!observation.tags[1].empty()) append("Tag 2: " + observation.tags[1]);
    for (std::size_t index = 0; index < observation.shopSlots.size(); ++index) {
        const SeedEngine::ShopCriterion& slot = observation.shopSlots[index];
        if (!slot.enabled) continue;
        append("Slot " + std::to_string(index + 1) + ": " + slot.name);
    }
    return result;
}
}

SeedLab::SeedLab(
    TextureCache& textures,
    std::filesystem::path investigationPath,
    std::filesystem::path assetManifestPath,
    std::function<void()> requestImageImport)
    : textures_(textures),
      investigationPath_(std::move(investigationPath)),
      assetManifestPath_(std::move(assetManifestPath)),
      requestImageImport_(std::move(requestImageImport)) {
    if (!assetManifestPath_.empty()) {
        gameAssets_.load(assetManifestPath_, std::filesystem::path("assets") / "game_reference");
    }
    loadInvestigation();
}

SeedLab::~SeedLab() {
    search_.cancel();
    saveInvestigation(false);
}

void SeedLab::draw(const PlayerProfile& profile) {
    if (resetScroll_) {
        ImGui::SetScrollY(0.0f);
        resetScroll_ = false;
    }
    ImGui::TextUnformatted("SEED LAB");
    ImGui::SameLine(0.0f, 24.0f);
    if (Ui::choiceButton("seed_analyze_mode", "Analyze", mode_ == Mode::Analyze, Ui::Red, ImVec2(130.0f, 34.0f))) {
        resetScroll_ = mode_ != Mode::Analyze;
        mode_ = Mode::Analyze;
    }
    ImGui::SameLine();
    if (Ui::choiceButton("seed_search_mode", "Reverse search", mode_ == Mode::Search, Ui::Orange, ImVec2(170.0f, 34.0f))) {
        resetScroll_ = mode_ != Mode::Search;
        mode_ = Mode::Search;
    }
    ImGui::SameLine();
    if (Ui::choiceButton("seed_recognize_mode", "Identify image", mode_ == Mode::Recognize, Ui::Dark, ImVec2(160.0f, 34.0f))) {
        resetScroll_ = mode_ != Mode::Recognize;
        mode_ = Mode::Recognize;
    }
    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();

    if (mode_ == Mode::Analyze) {
        drawAnalyzer(profile);
    }
    else if (mode_ == Mode::Search) {
        drawSearch(profile);
    }
    else {
        drawRecognition();
    }
}

void SeedLab::importRecognitionImage(const std::filesystem::path& imagePath) {
    recognitionImagePath_ = imagePath;
    recognitionRegions_.clear();
    selectedRecognitionRegion_ = -1;
    recognitionCropStart_ = ImVec2(0.0f, 0.0f);
    recognitionCropEnd_ = ImVec2(1.0f, 1.0f);
    mode_ = Mode::Recognize;
    resetScroll_ = true;
    runRecognition();
}

void SeedLab::drawRecognition() {
    if (requestImageImport_ && Ui::choiceButton(
            "choose_recognition_image",
            "Choose crop",
            false,
            Ui::Red,
            ImVec2(150.0f, 36.0f))) {
        requestImageImport_();
    }
    ImGui::SameLine();
    if (Ui::choiceButton(
            "capture_balatro_window",
            "Capture Balatro",
            false,
            Ui::Orange,
            ImVec2(170.0f, 36.0f))) {
        captureBalatroWindow();
    }
    ImGui::SameLine();
    const int previousSet = recognitionSetIndex_;
    vectorCombo("Asset type", recognitionSetIndex_, recognitionSets(), 180.0f);
    if (previousSet != recognitionSetIndex_ && !recognitionImagePath_.empty()) {
        if (recognitionRegions_.empty()) runRecognition();
        else detectRecognitionRegions();
    }

    if (!recognitionMessage_.empty()) {
        ImGui::PushStyleColor(
            ImGuiCol_Text,
            recognitionMessageIsError_
                ? ImVec4(1.0f, 0.42f, 0.38f, 1.0f)
                : ImVec4(0.45f, 0.92f, 0.58f, 1.0f));
        ImGui::TextUnformatted(recognitionMessage_.c_str());
        ImGui::PopStyleColor();
    }

    ImGui::Spacing();
    if (recognitionImagePath_.empty()) {
        ImGui::TextDisabled("Choose an image or capture a visible Balatro window");
        return;
    }

    if (Ui::choiceButton(
            "detect_recognition_regions",
            "Detect regions",
            false,
            Ui::Dark,
            ImVec2(150.0f, 34.0f))) {
        detectRecognitionRegions();
    }
    if (!recognitionRegions_.empty()) {
        ImGui::SameLine(0.0f, 12.0f);
        ImGui::TextDisabled("%llu candidate regions", static_cast<unsigned long long>(recognitionRegions_.size()));
    }

    if (ImGui::BeginTable("recognition_layout", 2, ImGuiTableFlags_SizingStretchProp)) {
        ImGui::TableSetupColumn("Input", ImGuiTableColumnFlags_WidthFixed, 230.0f);
        ImGui::TableSetupColumn("Candidates", ImGuiTableColumnFlags_WidthStretch);
        ImGui::TableNextRow();
        ImGui::TableNextColumn();
        ImGui::SeparatorText("Selection");
        const TextureAsset& input = textures_.get(recognitionImagePath_);
        if (input && input.height > 0) {
            const float aspect = static_cast<float>(input.width) / static_cast<float>(input.height);
            float previewWidth = 210.0f;
            float previewHeight = previewWidth / aspect;
            if (previewHeight > 300.0f) {
                previewHeight = 300.0f;
                previewWidth = previewHeight * aspect;
            }
            const ImVec2 imageMin = ImGui::GetCursorScreenPos();
            ImGui::Image(
                ImTextureRef(reinterpret_cast<ImTextureID>(input.texture)),
                ImVec2(previewWidth, previewHeight));
            const ImVec2 imageMax(imageMin.x + previewWidth, imageMin.y + previewHeight);
            const auto normalizedMouse = [imageMin, previewWidth, previewHeight]() {
                const ImVec2 mouse = ImGui::GetIO().MousePos;
                return ImVec2(
                    std::clamp((mouse.x - imageMin.x) / previewWidth, 0.0f, 1.0f),
                    std::clamp((mouse.y - imageMin.y) / previewHeight, 0.0f, 1.0f));
            };
            if (ImGui::IsItemHovered() && ImGui::IsMouseClicked(ImGuiMouseButton_Left)) {
                selectedRecognitionRegion_ = -1;
                recognitionDragAnchor_ = normalizedMouse();
                recognitionCropStart_ = recognitionDragAnchor_;
                recognitionCropEnd_ = recognitionDragAnchor_;
                recognitionDragging_ = true;
            }
            if (recognitionDragging_ && ImGui::IsMouseDown(ImGuiMouseButton_Left)) {
                recognitionCropStart_ = recognitionDragAnchor_;
                recognitionCropEnd_ = normalizedMouse();
            }
            if (recognitionDragging_ && ImGui::IsMouseReleased(ImGuiMouseButton_Left)) {
                recognitionCropEnd_ = normalizedMouse();
                recognitionDragging_ = false;
            }
            const ImVec2 cropMin(
                imageMin.x + std::min(recognitionCropStart_.x, recognitionCropEnd_.x) * previewWidth,
                imageMin.y + std::min(recognitionCropStart_.y, recognitionCropEnd_.y) * previewHeight);
            const ImVec2 cropMax(
                imageMin.x + std::max(recognitionCropStart_.x, recognitionCropEnd_.x) * previewWidth,
                imageMin.y + std::max(recognitionCropStart_.y, recognitionCropEnd_.y) * previewHeight);
            for (int index = 0; index < static_cast<int>(recognitionRegions_.size()); ++index) {
                const ImageRegion& region = recognitionRegions_[static_cast<std::size_t>(index)].region;
                const ImVec2 regionMin(
                    imageMin.x + region.left * previewWidth,
                    imageMin.y + region.top * previewHeight);
                const ImVec2 regionMax(
                    imageMin.x + region.right * previewWidth,
                    imageMin.y + region.bottom * previewHeight);
                const ImU32 color = index == selectedRecognitionRegion_ ? Ui::Orange : Ui::LightDark;
                ImGui::GetWindowDrawList()->AddRect(regionMin, regionMax, color, 2.0f, 0, 2.0f);
                const std::string number = std::to_string(index + 1);
                ImGui::GetWindowDrawList()->AddText(regionMin, color, number.c_str());
            }
            ImGui::GetWindowDrawList()->AddRect(cropMin, cropMax, Ui::Orange, 2.0f, 0, 3.0f);
            ImGui::GetWindowDrawList()->AddRect(imageMin, imageMax, Ui::Dark, 2.0f);
        }
        ImGui::TextWrapped("%s", recognitionImagePath_.filename().string().c_str());
        if (ImGui::Button("Match selection", ImVec2(145.0f, 34.0f))) {
            runRecognition();
        }
        ImGui::SameLine();
        if (ImGui::Button("Reset", ImVec2(62.0f, 34.0f))) {
            selectedRecognitionRegion_ = -1;
            recognitionCropStart_ = ImVec2(0.0f, 0.0f);
            recognitionCropEnd_ = ImVec2(1.0f, 1.0f);
            runRecognition();
        }
        if (!recognitionRegions_.empty()) {
            const char* preview = selectedRecognitionRegion_ >= 0
                ? recognitionRegions_[static_cast<std::size_t>(selectedRecognitionRegion_)].match.name.c_str()
                : "Choose detected region";
            ImGui::SetNextItemWidth(210.0f);
            if (ImGui::BeginCombo("Detected region", preview)) {
                for (int index = 0; index < static_cast<int>(recognitionRegions_.size()); ++index) {
                    const RecognitionRegion& region = recognitionRegions_[static_cast<std::size_t>(index)];
                    char label[160];
                    std::snprintf(
                        label,
                        sizeof(label),
                        "%d. %s | %s | %.1f%%",
                        index + 1,
                        region.match.set.c_str(),
                        region.match.name.c_str(),
                        region.match.confidence);
                    if (ImGui::Selectable(label, selectedRecognitionRegion_ == index)) {
                        selectRecognitionRegion(index);
                    }
                }
                ImGui::EndCombo();
            }
        }

        ImGui::TableNextColumn();
        ImGui::SeparatorText("Ranked matches");
        if (recognitionMatches_.empty()) {
            ImGui::TextDisabled("No matches");
        }
        else if (ImGui::BeginTable(
                "recognition_matches",
                4,
                ImGuiTableFlags_RowBg | ImGuiTableFlags_BordersInnerH | ImGuiTableFlags_SizingStretchProp)) {
            ImGui::TableSetupColumn("Art", ImGuiTableColumnFlags_WidthFixed, 70.0f);
            ImGui::TableSetupColumn("Type", ImGuiTableColumnFlags_WidthFixed, 105.0f);
            ImGui::TableSetupColumn("Candidate", ImGuiTableColumnFlags_WidthStretch);
            ImGui::TableSetupColumn("Confidence", ImGuiTableColumnFlags_WidthFixed, 100.0f);
            ImGui::TableHeadersRow();
            for (int index = 0; index < static_cast<int>(recognitionMatches_.size()); ++index) {
                const ImageMatch& match = recognitionMatches_[static_cast<std::size_t>(index)];
                const TextureAsset& candidate = textures_.get(match.path);
                ImGui::TableNextRow(ImGuiTableRowFlags_None, 72.0f);
                ImGui::TableNextColumn();
                ImGui::PushID(index);
                const float imageHeight = 58.0f;
                const float imageWidth = candidate && candidate.height > 0
                    ? imageHeight * static_cast<float>(candidate.width) / static_cast<float>(candidate.height)
                    : imageHeight;
                if (candidate && ImGui::ImageButton(
                        "candidate",
                        ImTextureRef(reinterpret_cast<ImTextureID>(candidate.texture)),
                        ImVec2(imageWidth, imageHeight))) {
                    selectedRecognition_ = index;
                }
                ImGui::TableNextColumn();
                ImGui::TextUnformatted(match.set.c_str());
                ImGui::TableNextColumn();
                if (ImGui::Selectable(
                        match.name.c_str(),
                        selectedRecognition_ == index,
                        ImGuiSelectableFlags_SpanAllColumns,
                        ImVec2(0.0f, 58.0f))) {
                    selectedRecognition_ = index;
                }
                ImGui::TableNextColumn();
                ImGui::Text("%.1f%%", match.confidence);
                ImGui::PopID();
            }
            ImGui::EndTable();
        }
        ImGui::EndTable();
    }

    if (selectedRecognition_ < 0 || selectedRecognition_ >= static_cast<int>(recognitionMatches_.size())) {
        return;
    }

    const ImageMatch& selected = recognitionMatches_[static_cast<std::size_t>(selectedRecognition_)];
    ImGui::Spacing();
    ImGui::SeparatorText("Confirm");
    ImGui::Text("%s  |  %s  |  %.1f%%", selected.set.c_str(), selected.name.c_str(), selected.confidence);
    if (shopKindForSet(selected.set)) {
        ImGui::SameLine(0.0f, 24.0f);
        ImGui::SetNextItemWidth(150.0f);
        ImGui::SliderInt("Shop slot", &recognitionShopSlot_, 1, SeedEngine::SearchShopSlots, "Slot %d");
    }
    else if (selected.set == "Tag") {
        ImGui::SameLine(0.0f, 24.0f);
        ImGui::SetNextItemWidth(150.0f);
        ImGui::SliderInt("Tag slot", &recognitionTagSlot_, 1, 2, "Tag %d");
    }
    if (Ui::choiceButton("confirm_recognition", "Use as clue", false, Ui::Orange, ImVec2(150.0f, 36.0f))) {
        applyRecognitionMatch(selected);
    }
    ImGui::SameLine();
    if (ImGui::Button("Open reverse search", ImVec2(190.0f, 36.0f))) {
        mode_ = Mode::Search;
        resetScroll_ = true;
    }
}

void SeedLab::detectRecognitionRegions() {
    selectedRecognition_ = -1;
    selectedRecognitionRegion_ = -1;
    recognitionRegions_.clear();
    if (recognitionImagePath_.empty()) return;

    std::string error;
    if (!imageMatcher_.loaded() && !imageMatcher_.load(
            assetManifestPath_,
            std::filesystem::path("assets") / "game_reference",
            &error)) {
        recognitionMessage_ = error;
        recognitionMessageIsError_ = true;
        return;
    }

    int width = 0;
    int height = 0;
    int channels = 0;
    const std::string filename = recognitionImagePath_.string();
    unsigned char* pixels = stbi_load(filename.c_str(), &width, &height, &channels, STBI_rgb_alpha);
    if (!pixels) {
        recognitionMessage_ = "Could not load " + recognitionImagePath_.filename().string();
        recognitionMessageIsError_ = true;
        return;
    }

    const std::string filter = recognitionSetIndex_ == 0
        ? std::string()
        : recognitionSets()[static_cast<std::size_t>(recognitionSetIndex_)];
    std::vector<float> aspectRatios;
    if (filter.empty()) aspectRatios = {142.0f / 190.0f, 1.0f};
    else if (filter == "Blind" || filter == "Tag" || filter == "Stake") aspectRatios = {1.0f};
    else aspectRatios = {142.0f / 190.0f};

    std::vector<ImageRegion> regions = ImageRegionDetector::detectRgba(
        pixels,
        width,
        height,
        aspectRatios,
        36);
    regions.insert(regions.begin(), ImageRegion{0.0f, 0.0f, 1.0f, 1.0f, 0.65f});
    for (const ImageRegion& region : regions) {
        std::vector<ImageMatch> matches = imageMatcher_.matchRgbaRegion(
            pixels,
            width,
            height,
            region.left,
            region.top,
            region.right,
            region.bottom,
            filter.empty() ? 16 : 1,
            filter);
        if (filter.empty()) {
            matches.erase(
                std::remove_if(matches.begin(), matches.end(), [](const ImageMatch& match) {
                    return !isRecognizableSet(match.set);
                }),
                matches.end());
        }
        if (matches.empty()) continue;
        const float score = matches.front().confidence * 0.78f + region.score * 22.0f;
        recognitionRegions_.push_back(RecognitionRegion{region, std::move(matches.front()), score});
    }
    stbi_image_free(pixels);

    std::stable_sort(
        recognitionRegions_.begin(),
        recognitionRegions_.end(),
        [](const RecognitionRegion& left, const RecognitionRegion& right) {
            return left.score > right.score;
        });
    if (recognitionRegions_.size() > 12) recognitionRegions_.resize(12);
    if (recognitionRegions_.empty()) {
        recognitionMessage_ = "No likely game-art regions were found";
        recognitionMessageIsError_ = true;
        return;
    }

    selectRecognitionRegion(0);
    recognitionMessage_ = "Detected " + std::to_string(recognitionRegions_.size()) +
        " likely regions | Best: " + recognitionRegions_.front().match.name;
    recognitionMessageIsError_ = false;
}

void SeedLab::selectRecognitionRegion(int index) {
    if (index < 0 || index >= static_cast<int>(recognitionRegions_.size())) return;
    selectedRecognitionRegion_ = index;
    const ImageRegion& region = recognitionRegions_[static_cast<std::size_t>(index)].region;
    recognitionCropStart_ = ImVec2(region.left, region.top);
    recognitionCropEnd_ = ImVec2(region.right, region.bottom);
    runRecognition();
}

void SeedLab::captureBalatroWindow() {
    std::filesystem::path capturePath;
    try {
        capturePath = std::filesystem::temp_directory_path() / "Balatrauto" / "latest_balatro_capture.bmp";
    }
    catch (const std::exception&) {
        recognitionMessage_ = "Windows did not provide a temporary capture folder";
        recognitionMessageIsError_ = true;
        return;
    }

    std::string error;
    if (!WindowCapture::captureBalatro(capturePath, &error)) {
        recognitionMessage_ = error;
        recognitionMessageIsError_ = true;
        return;
    }
    textures_.invalidate(capturePath);
    importRecognitionImage(capturePath);
    detectRecognitionRegions();
}

void SeedLab::runRecognition() {
    selectedRecognition_ = -1;
    recognitionMatches_.clear();
    if (recognitionImagePath_.empty()) return;

    std::string error;
    if (!imageMatcher_.loaded() && !imageMatcher_.load(
            assetManifestPath_,
            std::filesystem::path("assets") / "game_reference",
            &error)) {
        recognitionMessage_ = error;
        recognitionMessageIsError_ = true;
        return;
    }

    const std::string filter = recognitionSetIndex_ == 0
        ? std::string()
        : recognitionSets()[static_cast<std::size_t>(recognitionSetIndex_)];
    recognitionMatches_ = imageMatcher_.matchFileRegion(
        recognitionImagePath_,
        recognitionCropStart_.x,
        recognitionCropStart_.y,
        recognitionCropEnd_.x,
        recognitionCropEnd_.y,
        filter.empty() ? 32 : 8,
        filter,
        &error);
    if (!error.empty()) {
        recognitionMessage_ = error;
        recognitionMessageIsError_ = true;
        return;
    }
    if (filter.empty()) {
        recognitionMatches_.erase(
            std::remove_if(recognitionMatches_.begin(), recognitionMatches_.end(), [](const ImageMatch& match) {
                return !isRecognizableSet(match.set);
            }),
            recognitionMatches_.end());
        if (recognitionMatches_.size() > 8) recognitionMatches_.resize(8);
    }
    if (recognitionMatches_.empty()) {
        recognitionMessage_ = "No matching assets";
        recognitionMessageIsError_ = true;
        return;
    }
    selectedRecognition_ = 0;
    recognitionMessage_ = "Compared with " + std::to_string(imageMatcher_.size()) + " game assets";
    recognitionMessageIsError_ = false;
}

bool SeedLab::applyRecognitionMatch(const ImageMatch& match) {
    bool applied = false;
    if (match.set == "Blind") {
        observedBoss_ = stringIndex(SeedEngine::bossNames(), match.name);
        applied = observedBoss_ >= 0;
    }
    else if (match.set == "Voucher") {
        observedVoucher_ = stringIndex(SeedEngine::voucherNames(), match.name);
        applied = observedVoucher_ >= 0;
    }
    else if (match.set == "Tag") {
        const int index = stringIndex(SeedEngine::tagNames(), match.name);
        if (index >= 0) {
            observedTags_[static_cast<std::size_t>(recognitionTagSlot_ - 1)] = index;
            applied = true;
        }
    }
    else if (match.set == "Back") {
        const std::string deck = match.name == "Painted Deck" ? "Paint Deck" : match.name;
        const int index = stringIndex(DECK_NAMES, deck);
        if (index >= 0) {
            deckIndex_ = index;
            applied = true;
        }
    }
    else if (match.set == "Stake") {
        const int index = stringIndex(STAKE_NAMES, match.name);
        if (index >= 0) {
            stakeIndex_ = index;
            applied = true;
        }
    }
    else if (const std::optional<SeedEngine::ShopItemKind> kind = shopKindForSet(match.set)) {
        const std::string itemName = *kind == SeedEngine::ShopItemKind::PlayingCard
            ? "Playing Card"
            : match.name;
        const std::vector<ShopChoice>& choices = shopChoices();
        const auto found = std::find_if(choices.begin(), choices.end(), [&kind, &itemName](const ShopChoice& choice) {
            return choice.kind == *kind && choice.name == itemName;
        });
        if (found != choices.end()) {
            observedShopItems_[static_cast<std::size_t>(recognitionShopSlot_ - 1)] =
                static_cast<int>(found - choices.begin());
            applied = true;
        }
    }

    recognitionMessage_ = applied
        ? "Confirmed " + match.name
        : match.name + " is not a searchable clue yet";
    recognitionMessageIsError_ = !applied;
    if (applied) saveInvestigation(false);
    return applied;
}

void SeedLab::drawAnalyzer(const PlayerProfile& profile) {
    ImGui::SetNextItemWidth(130.0f);
    ImGui::InputText("Seed", seed_.data(), seed_.size(), ImGuiInputTextFlags_CharsUppercase);
    ImGui::SameLine();
    vectorCombo("Deck", deckIndex_, DECK_NAMES, 170.0f);
    ImGui::SameLine();
    vectorCombo("Stake", stakeIndex_, STAKE_NAMES, 150.0f);
    ImGui::SameLine();
    ImGui::SetNextItemWidth(90.0f);
    ImGui::SliderInt("Ante", &ante_, 1, 8);
    ImGui::SameLine();
    if (Ui::choiceButton("analyze_seed", "Analyze seed", false, Ui::Red, ImVec2(150.0f, 36.0f))) {
        runAnalysis(profile);
    }

    ImGui::SetNextItemWidth(110.0f);
    ImGui::SliderInt("Shop rolls", &shopSlots_, 2, 16);
    ImGui::SameLine();
    ImGui::SetNextItemWidth(120.0f);
    ImGui::SliderInt("Queue offset", &shopOffset_, 0, 200);
    ImGui::SameLine();
    ImGui::SetNextItemWidth(100.0f);
    ImGui::SliderInt("Packs", &packCount_, 0, 6);
    ImGui::SameLine();
    ImGui::Checkbox("Profile unlocks", &useProfileUnlocks_);
    ImGui::SameLine();
    ImGui::Checkbox("Fresh run gates", &freshRun_);

    if (!analysisMessage_.empty()) {
        ImGui::PushStyleColor(ImGuiCol_Text, hasAnalysis_
            ? ImVec4(0.45f, 0.92f, 0.58f, 1.0f)
            : ImVec4(1.0f, 0.42f, 0.38f, 1.0f));
        ImGui::TextUnformatted(analysisMessage_.c_str());
        ImGui::PopStyleColor();
    }

    ImGui::Spacing();
    if (!hasAnalysis_) {
        ImGui::TextDisabled("No seed analyzed");
        return;
    }

    if (ImGui::BeginTable("seed_metrics", 4, ImGuiTableFlags_SizingStretchSame)) {
        ImGui::TableNextColumn();
        drawAssetMetric("Boss", "Blind", analysis_.boss, Ui::Red);
        ImGui::TableNextColumn();
        drawAssetMetric("Voucher", "Voucher", analysis_.voucher, Ui::Orange);
        ImGui::TableNextColumn();
        drawAssetMetric("Tag 1", "Tag", analysis_.tags[0], Ui::Dark);
        ImGui::TableNextColumn();
        drawAssetMetric("Tag 2", "Tag", analysis_.tags[1], Ui::Dark);
        ImGui::EndTable();
    }

    ImGui::Spacing();
    ImGui::SeparatorText("Shop queue");
    if (ImGui::BeginTable(
            "shop_queue",
            6,
            ImGuiTableFlags_RowBg | ImGuiTableFlags_BordersInnerH | ImGuiTableFlags_SizingStretchProp,
            ImVec2(0.0f, std::min(245.0f, ImGui::GetContentRegionAvail().y * 0.46f)))) {
        ImGui::TableSetupColumn("#", ImGuiTableColumnFlags_WidthFixed, 38.0f);
        ImGui::TableSetupColumn("Art", ImGuiTableColumnFlags_WidthFixed, 44.0f);
        ImGui::TableSetupColumn("Type", ImGuiTableColumnFlags_WidthFixed, 100.0f);
        ImGui::TableSetupColumn("Item");
        ImGui::TableSetupColumn("Edition", ImGuiTableColumnFlags_WidthFixed, 120.0f);
        ImGui::TableSetupColumn("Stickers", ImGuiTableColumnFlags_WidthFixed, 180.0f);
        ImGui::TableHeadersRow();
        for (std::size_t index = 0; index < analysis_.shopItems.size(); ++index) {
            const SeedEngine::ShopItem& item = analysis_.shopItems[index];
            ImGui::TableNextRow();
            ImGui::TableNextColumn();
            ImGui::Text("%d", analysis_.shopOffset + static_cast<int>(index + 1));
            ImGui::TableNextColumn();
            drawGameAsset(shopAssetSet(item.kind), item.name, 38.0f);
            ImGui::TableNextColumn();
            ImGui::TextDisabled("%s", shopKindName(item.kind));
            ImGui::TableNextColumn();
            ImGui::TextUnformatted(item.name.c_str());
            ImGui::TableNextColumn();
            ImGui::TextUnformatted(item.edition.empty() ? "-" : item.edition.c_str());
            ImGui::TableNextColumn();
            const std::string stickers = stickerText(item);
            ImGui::TextUnformatted(stickers.c_str());
        }
        ImGui::EndTable();
    }

    ImGui::Spacing();
    ImGui::SeparatorText("Booster packs");
    if (ImGui::BeginTable("pack_queue", 3, ImGuiTableFlags_RowBg | ImGuiTableFlags_SizingStretchProp)) {
        ImGui::TableSetupColumn("Art", ImGuiTableColumnFlags_WidthFixed, 44.0f);
        ImGui::TableSetupColumn("Pack", ImGuiTableColumnFlags_WidthFixed, 190.0f);
        ImGui::TableSetupColumn("Contents");
        ImGui::TableHeadersRow();
        for (const SeedEngine::Pack& pack : analysis_.packs) {
            ImGui::TableNextRow();
            ImGui::TableNextColumn();
            drawGameAsset("Booster", pack.name, 40.0f);
            ImGui::TableNextColumn();
            ImGui::TextUnformatted(pack.name.c_str());
            ImGui::TableNextColumn();
            std::string contents;
            for (const std::string& item : pack.contents) {
                if (!contents.empty()) contents += "  |  ";
                contents += item;
            }
            ImGui::TextWrapped("%s", contents.c_str());
        }
        ImGui::EndTable();
    }
}

void SeedLab::drawSearch(const PlayerProfile& profile) {
    const SeedEngine::SearchSnapshot snapshot = search_.snapshot();
    if (snapshot.finished && searchMessage_ == "Search running") {
        if (!snapshot.error.empty()) {
            searchMessage_ = snapshot.error;
            searchMessageIsError_ = true;
        }
        else if (snapshot.cancelled) {
            searchMessage_ = "Search cancelled";
            searchMessageIsError_ = false;
        }
        else {
            searchMessage_ = snapshot.matches.empty()
                ? "Search complete - no matches"
                : "Search complete - " + std::to_string(snapshot.matches.size()) + " match(es)";
            searchMessageIsError_ = false;
        }
    }

    bool criteriaChanged = false;
    bool preserveCriteriaMessage = false;

    if (snapshot.running) {
        ImGui::BeginDisabled();
    }

    ImGui::SetNextItemWidth(130.0f);
    ImGui::InputText("Start seed", searchStart_.data(), searchStart_.size(), ImGuiInputTextFlags_CharsUppercase);
    ImGui::SameLine();
    ImGui::SetNextItemWidth(145.0f);
    ImGui::InputScalar("Seeds", ImGuiDataType_U64, &searchCount_);
    ImGui::SameLine();
    ImGui::SetNextItemWidth(130.0f);
    ImGui::SliderInt("Threads", &threadCount_, 0, 32, threadCount_ == 0 ? "Auto" : "%d");
    threadCount_ = std::clamp(threadCount_, 0, 64);
    ImGui::SameLine();
    ImGui::SetNextItemWidth(120.0f);
    ImGui::SliderInt("Matches", &resultLimit_, 1, 100);
    resultLimit_ = std::clamp(resultLimit_, 1, 100);

    criteriaChanged |= vectorCombo("Deck##search", deckIndex_, DECK_NAMES, 170.0f);
    ImGui::SameLine();
    criteriaChanged |= vectorCombo("Stake##search", stakeIndex_, STAKE_NAMES, 150.0f);
    ImGui::SameLine();
    ImGui::SetNextItemWidth(90.0f);
    criteriaChanged |= ImGui::SliderInt("Ante##search", &ante_, 1, 8);
    ImGui::SameLine();
    criteriaChanged |= ImGui::Checkbox("Profile unlocks##search", &useProfileUnlocks_);
    ImGui::SameLine();
    criteriaChanged |= ImGui::Checkbox("Fresh run gates##search", &freshRun_);

    ImGui::SeparatorText("Observed round clues");
    if (ImGui::BeginTable("round_clues", 4, ImGuiTableFlags_SizingStretchSame)) {
        ImGui::TableNextColumn();
        criteriaChanged |= optionalCombo("Boss##observed", observedBoss_, SeedEngine::bossNames(), "Any boss");
        ImGui::TableNextColumn();
        criteriaChanged |= optionalCombo("Voucher##observed", observedVoucher_, SeedEngine::voucherNames(), "Any voucher");
        ImGui::TableNextColumn();
        criteriaChanged |= optionalCombo("Tag 1##observed", observedTags_[0], SeedEngine::tagNames(), "Any tag");
        ImGui::TableNextColumn();
        criteriaChanged |= optionalCombo("Tag 2##observed", observedTags_[1], SeedEngine::tagNames(), "Any tag");
        ImGui::EndTable();
    }

    ImGui::SeparatorText("Run position");
    if (ImGui::BeginTable("run_position_controls", 4, ImGuiTableFlags_SizingStretchSame)) {
        ImGui::TableNextColumn();
        ImGui::SetNextItemWidth(-1.0f);
        criteriaChanged |= ImGui::SliderInt("Shop size", &shopSize_, 1, 6);
        ImGui::TableNextColumn();
        if (Ui::choiceButton("next_shop_action", "Next shop", false, Ui::Dark, ImVec2(-1.0f, 34.0f))) {
            recordAction(SeedRunActionKind::NextShop, ante_, std::min(200, shopOffset_ + shopSize_), shopSize_);
            criteriaChanged = true;
        }
        ImGui::TableNextColumn();
        if (Ui::choiceButton("reroll_shop_action", "Reroll shop", false, Ui::Orange, ImVec2(-1.0f, 34.0f))) {
            recordAction(SeedRunActionKind::RerollShop, ante_, std::min(200, shopOffset_ + shopSize_), shopSize_);
            criteriaChanged = true;
        }
        ImGui::TableNextColumn();
        const bool canAdvanceAnte = ante_ < 8;
        if (!canAdvanceAnte) ImGui::BeginDisabled();
        if (Ui::choiceButton("next_ante_action", "Next ante", false, Ui::Red, ImVec2(-1.0f, 34.0f))) {
            recordAction(SeedRunActionKind::AdvanceAnte, std::min(8, ante_ + 1), 0);
            criteriaChanged = true;
        }
        if (!canAdvanceAnte) ImGui::EndDisabled();
        ImGui::EndTable();
    }

    if (ImGui::BeginTable("run_log_controls", 4, ImGuiTableFlags_SizingStretchSame)) {
        ImGui::TableNextColumn();
        if (Ui::choiceButton("purchase_action", "Purchase", false, Ui::Dark, ImVec2(-1.0f, 30.0f))) {
            recordAction(SeedRunActionKind::Purchase, ante_, shopOffset_);
        }
        ImGui::TableNextColumn();
        if (Ui::choiceButton("pack_action", "Open pack", false, Ui::Dark, ImVec2(-1.0f, 30.0f))) {
            recordAction(SeedRunActionKind::OpenPack, ante_, shopOffset_);
        }
        ImGui::TableNextColumn();
        if (Ui::choiceButton("skip_action", "Skip blind", false, Ui::Dark, ImVec2(-1.0f, 30.0f))) {
            recordAction(SeedRunActionKind::SkipBlind, ante_, shopOffset_);
        }
        ImGui::TableNextColumn();
        const bool canUndoAction = !actions_.empty();
        if (!canUndoAction) ImGui::BeginDisabled();
        if (Ui::choiceButton("undo_action", "Undo action", false, Ui::Dark, ImVec2(-1.0f, 30.0f))) {
            undoLastAction();
            criteriaChanged = true;
        }
        if (!canUndoAction) ImGui::EndDisabled();
        ImGui::EndTable();
    }

    ImGui::SetNextItemWidth(150.0f);
    criteriaChanged |= ImGui::SliderInt("Shop queue offset##search", &shopOffset_, 0, 200);

    ImGui::SeparatorText("Observed shop slots");
    if (ImGui::BeginTable("observed_slots", static_cast<int>(SeedEngine::SearchShopSlots), ImGuiTableFlags_SizingStretchSame)) {
        for (int slot = 0; slot < static_cast<int>(SeedEngine::SearchShopSlots); ++slot) {
            ImGui::TableNextColumn();
            criteriaChanged |= shopItemCombo(
                slot,
                observedShopItems_[static_cast<std::size_t>(slot)],
                shopFilters_[static_cast<std::size_t>(slot)]);
            const int selectedItem = observedShopItems_[static_cast<std::size_t>(slot)];
            if (selectedItem >= 0) {
                const ShopChoice& choice = shopChoices()[static_cast<std::size_t>(selectedItem)];
                const float previewWidth = 52.0f * 142.0f / 190.0f;
                ImGui::SetCursorPosX(ImGui::GetCursorPosX() +
                    std::max(0.0f, (ImGui::GetContentRegionAvail().x - previewWidth) * 0.5f));
                drawGameAsset(shopAssetSet(choice.kind), choice.name, 52.0f);
            }
        }
        ImGui::EndTable();
    }

    if (!actions_.empty()) {
        ImGui::SeparatorText("Run actions");
        if (ImGui::BeginTable(
                "run_actions",
                3,
                ImGuiTableFlags_RowBg | ImGuiTableFlags_BordersInnerH | ImGuiTableFlags_SizingStretchProp,
                ImVec2(0.0f, std::min(118.0f, 34.0f + 25.0f * static_cast<float>(actions_.size()))))) {
            ImGui::TableSetupColumn("#", ImGuiTableColumnFlags_WidthFixed, 34.0f);
            ImGui::TableSetupColumn("Action");
            ImGui::TableSetupColumn("Position", ImGuiTableColumnFlags_WidthFixed, 160.0f);
            ImGui::TableHeadersRow();
            const std::size_t first = actions_.size() > 6 ? actions_.size() - 6 : 0;
            for (std::size_t index = first; index < actions_.size(); ++index) {
                const SeedRunAction& action = actions_[index];
                ImGui::TableNextRow();
                ImGui::TableNextColumn();
                ImGui::Text("%d", static_cast<int>(index + 1));
                ImGui::TableNextColumn();
                ImGui::TextUnformatted(seedRunActionName(action.kind));
                ImGui::TableNextColumn();
                ImGui::Text("Ante %d  |  Offset %llu", action.anteAfter,
                    static_cast<unsigned long long>(action.shopOffsetAfter));
            }
            ImGui::EndTable();
        }
    }

    ImGui::SeparatorText("Branch checkpoints");
    if (ImGui::BeginTable("branch_controls", 6, ImGuiTableFlags_SizingStretchSame)) {
        ImGui::TableNextColumn();
        ImGui::SetNextItemWidth(-1.0f);
        ImGui::InputTextWithHint("##branch_name", "Checkpoint name", branchName_.data(), branchName_.size());
        ImGui::TableNextColumn();
        const bool canCreateBranch = branches_.size() < 50;
        if (!canCreateBranch) ImGui::BeginDisabled();
        if (Ui::choiceButton("save_branch", "Save checkpoint", false, Ui::Orange, ImVec2(-1.0f, 32.0f))) {
            createBranch();
        }
        if (!canCreateBranch) ImGui::EndDisabled();
        ImGui::TableNextColumn();
        ImGui::SetNextItemWidth(-1.0f);
        const char* branchPreview = selectedBranch_ >= 0 && selectedBranch_ < static_cast<int>(branches_.size())
            ? branches_[static_cast<std::size_t>(selectedBranch_)].name.c_str()
            : "Choose checkpoint";
        if (ImGui::BeginCombo("##branch_selection", branchPreview)) {
            for (int index = 0; index < static_cast<int>(branches_.size()); ++index) {
                ImGui::PushID(index);
                const bool selected = index == selectedBranch_;
                if (ImGui::Selectable(branches_[static_cast<std::size_t>(index)].name.c_str(), selected)) {
                    selectedBranch_ = index;
                    branchName_.fill('\0');
                    saveInvestigation(false);
                }
                if (selected) ImGui::SetItemDefaultFocus();
                ImGui::PopID();
            }
            ImGui::EndCombo();
        }
        const bool hasSelectedBranch = selectedBranch_ >= 0 && selectedBranch_ < static_cast<int>(branches_.size());
        if (!hasSelectedBranch) ImGui::BeginDisabled();
        ImGui::TableNextColumn();
        if (Ui::choiceButton("restore_branch", "Restore", false, Ui::Dark, ImVec2(-1.0f, 32.0f))) {
            if (restoreBranch()) {
                criteriaChanged = true;
                preserveCriteriaMessage = true;
            }
        }
        ImGui::TableNextColumn();
        if (Ui::choiceButton("update_branch", "Update", false, Ui::Dark, ImVec2(-1.0f, 32.0f))) {
            updateBranch();
        }
        ImGui::TableNextColumn();
        if (Ui::choiceButton("delete_branch", "Delete", false, Ui::Red, ImVec2(-1.0f, 32.0f))) {
            deleteBranch();
        }
        if (!hasSelectedBranch) ImGui::EndDisabled();
        ImGui::EndTable();
    }
    if (selectedBranch_ >= 0 && selectedBranch_ < static_cast<int>(branches_.size())) {
        const SeedRunBranch& branch = branches_[static_cast<std::size_t>(selectedBranch_)];
        ImGui::TextDisabled(
            "Ante %d  |  Offset %llu  |  %llu clue(s)  |  %llu action(s)",
            branch.draft.ante,
            static_cast<unsigned long long>(branch.draft.shopOffset),
            static_cast<unsigned long long>(branch.observations.size()),
            static_cast<unsigned long long>(branch.actions.size()));
    }

    ImGui::SeparatorText("Clue timeline");
    const bool canAddObservation = hasCurrentObservation();
    if (!canAddObservation) ImGui::BeginDisabled();
    if (Ui::choiceButton("add_observation", "Add current clues", false, Ui::Orange, ImVec2(190.0f, 34.0f))) {
        timeline_.push_back(currentObservation());
        clearCurrentObservation();
        saveInvestigation(false);
        criteriaChanged = true;
    }
    if (!canAddObservation) ImGui::EndDisabled();
    ImGui::SameLine();
    const bool hasTimeline = !timeline_.empty();
    if (!hasTimeline) ImGui::BeginDisabled();
    if (Ui::choiceButton("undo_observation", "Undo last", false, Ui::Dark, ImVec2(125.0f, 34.0f))) {
        timeline_.pop_back();
        saveInvestigation(false);
        criteriaChanged = true;
    }
    ImGui::SameLine();
    if (Ui::choiceButton("clear_observations", "Clear timeline", false, Ui::Red, ImVec2(150.0f, 34.0f))) {
        timeline_.clear();
        saveInvestigation(false);
        criteriaChanged = true;
    }
    if (!hasTimeline) ImGui::EndDisabled();

    if (!timeline_.empty() && ImGui::BeginTable(
            "clue_timeline",
            4,
            ImGuiTableFlags_RowBg | ImGuiTableFlags_BordersInnerH | ImGuiTableFlags_SizingStretchProp,
            ImVec2(0.0f, std::min(150.0f, 34.0f + 27.0f * static_cast<float>(timeline_.size()))))) {
        ImGui::TableSetupColumn("#", ImGuiTableColumnFlags_WidthFixed, 34.0f);
        ImGui::TableSetupColumn("Ante", ImGuiTableColumnFlags_WidthFixed, 55.0f);
        ImGui::TableSetupColumn("Offset", ImGuiTableColumnFlags_WidthFixed, 65.0f);
        ImGui::TableSetupColumn("Observed clues");
        ImGui::TableHeadersRow();
        for (std::size_t index = 0; index < timeline_.size(); ++index) {
            const SeedEngine::SearchObservation& observation = timeline_[index];
            ImGui::TableNextRow();
            ImGui::TableNextColumn();
            ImGui::Text("%d", static_cast<int>(index + 1));
            ImGui::TableNextColumn();
            ImGui::Text("%d", observation.ante);
            ImGui::TableNextColumn();
            ImGui::Text("%llu", static_cast<unsigned long long>(observation.shopOffset));
            ImGui::TableNextColumn();
            const std::string summary = observationSummary(observation);
            ImGui::TextUnformatted(summary.c_str());
        }
        ImGui::EndTable();
    }

    if (snapshot.running) {
        ImGui::EndDisabled();
    }
    if (criteriaChanged && !snapshot.running && !searchMessageIsError_ && !preserveCriteriaMessage) {
        searchMessage_.clear();
    }

    if (snapshot.running) {
        if (Ui::choiceButton("cancel_seed_search", "Cancel", false, Ui::Red, ImVec2(130.0f, 36.0f))) {
            search_.cancel();
        }
    }
    else if (Ui::choiceButton("start_seed_search", "Start search", false, Ui::Orange, ImVec2(160.0f, 36.0f))) {
        startSearch(profile);
    }

    ImGui::SameLine(0.0f, 18.0f);
    const float progress = snapshot.total == 0
        ? 0.0f
        : static_cast<float>(snapshot.processed) / static_cast<float>(snapshot.total);
    char progressLabel[96];
    std::snprintf(
        progressLabel,
        sizeof(progressLabel),
        "%llu / %llu  |  %.0f seeds/s",
        static_cast<unsigned long long>(snapshot.processed),
        static_cast<unsigned long long>(snapshot.total),
        snapshot.seedsPerSecond);
    ImGui::ProgressBar(progress, ImVec2(std::max(260.0f, ImGui::GetContentRegionAvail().x), 32.0f), progressLabel);

    if (!searchMessage_.empty()) {
        ImGui::PushStyleColor(
            ImGuiCol_Text,
            searchMessageIsError_
                ? ImVec4(1.0f, 0.42f, 0.38f, 1.0f)
                : ImVec4(0.45f, 0.92f, 0.58f, 1.0f));
        ImGui::TextUnformatted(searchMessage_.c_str());
        ImGui::PopStyleColor();
    }

    ImGui::SeparatorText("Matches");
    if (snapshot.matches.empty()) {
        ImGui::TextDisabled(snapshot.finished ? "No matches in this batch" : "No matches yet");
        return;
    }

    if (ImGui::BeginTable("seed_matches", 4, ImGuiTableFlags_SizingStretchSame)) {
        for (std::size_t index = 0; index < snapshot.matches.size(); ++index) {
            ImGui::TableNextColumn();
            ImGui::PushID(static_cast<int>(index));
            if (Ui::choiceButton("match", snapshot.matches[index].c_str(), false, Ui::Dark, ImVec2(-1.0f, 38.0f))) {
                std::snprintf(seed_.data(), seed_.size(), "%s", snapshot.matches[index].c_str());
                runAnalysis(profile);
                mode_ = Mode::Analyze;
            }
            if (ImGui::IsItemClicked(ImGuiMouseButton_Right)) {
                ImGui::SetClipboardText(snapshot.matches[index].c_str());
            }
            ImGui::PopID();
        }
        ImGui::EndTable();
    }
}

void SeedLab::runAnalysis(const PlayerProfile& profile) {
    analysis_ = SeedEngine::analyze(
        seed_.data(),
        DECK_NAMES[static_cast<std::size_t>(deckIndex_)],
        STAKE_NAMES[static_cast<std::size_t>(stakeIndex_)],
        ante_,
        shopOffset_,
        shopSlots_,
        packCount_,
        rulesFor(profile));
    hasAnalysis_ = analysis_.valid;
    analysisMessage_ = hasAnalysis_
        ? "Analyzed " + analysis_.seed
        : analysis_.error;
    if (hasAnalysis_) {
        std::snprintf(seed_.data(), seed_.size(), "%s", analysis_.seed.c_str());
    }
}

void SeedLab::startSearch(const PlayerProfile& profile) {
    SeedEngine::SearchQuery query;
    query.startSeed = searchStart_.data();
    query.seedCount = searchCount_;
    query.threadCount = static_cast<unsigned int>(threadCount_);
    query.resultLimit = static_cast<std::size_t>(resultLimit_);
    query.deck = DECK_NAMES[static_cast<std::size_t>(deckIndex_)];
    query.stake = STAKE_NAMES[static_cast<std::size_t>(stakeIndex_)];
    query.rules = rulesFor(profile);
    query.observations = timeline_;
    if (hasCurrentObservation()) {
        query.observations.push_back(currentObservation());
    }

    saveInvestigation(false);
    std::string error;
    if (search_.start(query, &error)) {
        searchMessage_ = "Search running";
        searchMessageIsError_ = false;
    }
    else {
        searchMessage_ = error;
        searchMessageIsError_ = true;
    }
}

SeedEngine::SearchObservation SeedLab::currentObservation() const {
    SeedEngine::SearchObservation observation;
    observation.ante = ante_;
    observation.shopOffset = static_cast<std::size_t>(std::max(0, shopOffset_));
    if (observedBoss_ >= 0) {
        observation.boss = SeedEngine::bossNames()[static_cast<std::size_t>(observedBoss_)];
    }
    if (observedVoucher_ >= 0) {
        observation.voucher = SeedEngine::voucherNames()[static_cast<std::size_t>(observedVoucher_)];
    }
    for (std::size_t tag = 0; tag < observation.tags.size(); ++tag) {
        if (observedTags_[tag] >= 0) {
            observation.tags[tag] = SeedEngine::tagNames()[static_cast<std::size_t>(observedTags_[tag])];
        }
    }
    const std::vector<ShopChoice>& choices = shopChoices();
    for (std::size_t slot = 0; slot < observation.shopSlots.size(); ++slot) {
        const int item = observedShopItems_[slot];
        if (item >= 0) {
            const ShopChoice& choice = choices[static_cast<std::size_t>(item)];
            observation.shopSlots[slot] = SeedEngine::ShopCriterion{true, choice.kind, choice.name};
        }
    }
    return observation;
}

void SeedLab::applyObservation(const SeedEngine::SearchObservation& observation) {
    clearCurrentObservation();
    ante_ = observation.ante;
    shopOffset_ = static_cast<int>(observation.shopOffset);
    observedBoss_ = stringIndex(SeedEngine::bossNames(), observation.boss);
    observedVoucher_ = stringIndex(SeedEngine::voucherNames(), observation.voucher);
    for (std::size_t index = 0; index < observedTags_.size(); ++index) {
        observedTags_[index] = stringIndex(SeedEngine::tagNames(), observation.tags[index]);
    }
    const std::vector<ShopChoice>& choices = shopChoices();
    for (std::size_t slot = 0; slot < observedShopItems_.size(); ++slot) {
        const SeedEngine::ShopCriterion& expected = observation.shopSlots[slot];
        if (!expected.enabled) continue;
        const auto found = std::find_if(choices.begin(), choices.end(), [&expected](const ShopChoice& choice) {
            return choice.kind == expected.kind && choice.name == expected.name;
        });
        if (found != choices.end()) {
            observedShopItems_[slot] = static_cast<int>(found - choices.begin());
        }
    }
}

bool SeedLab::hasCurrentObservation() const {
    const SeedEngine::SearchObservation observation = currentObservation();
    return !observation.boss.empty() ||
        !observation.voucher.empty() ||
        std::any_of(observation.tags.begin(), observation.tags.end(), [](const std::string& value) {
            return !value.empty();
        }) ||
        std::any_of(observation.shopSlots.begin(), observation.shopSlots.end(), [](const SeedEngine::ShopCriterion& value) {
            return value.enabled;
        });
}

void SeedLab::clearCurrentObservation() {
    observedBoss_ = -1;
    observedVoucher_ = -1;
    observedTags_.fill(-1);
    observedShopItems_.fill(-1);
    for (auto& filter : shopFilters_) {
        filter.fill('\0');
    }
}

SeedInvestigation SeedLab::investigation() const {
    SeedInvestigation result;
    result.startSeed = searchStart_.data();
    result.seedCount = searchCount_;
    result.threadCount = static_cast<unsigned int>(std::max(0, threadCount_));
    result.resultLimit = static_cast<std::size_t>(std::max(1, resultLimit_));
    result.deck = DECK_NAMES[static_cast<std::size_t>(deckIndex_)];
    result.stake = STAKE_NAMES[static_cast<std::size_t>(stakeIndex_)];
    result.useProfileUnlocks = useProfileUnlocks_;
    result.freshRun = freshRun_;
    result.draft = currentObservation();
    result.observations = timeline_;
    result.shopSize = shopSize_;
    result.actions = actions_;
    result.branches = branches_;
    result.selectedBranch = selectedBranch_;
    return result;
}

void SeedLab::applyInvestigation(const SeedInvestigation& saved) {
    std::snprintf(searchStart_.data(), searchStart_.size(), "%s", saved.startSeed.c_str());
    searchCount_ = saved.seedCount;
    threadCount_ = static_cast<int>(saved.threadCount);
    resultLimit_ = static_cast<int>(saved.resultLimit);
    const int savedDeck = stringIndex(DECK_NAMES, saved.deck);
    const int savedStake = stringIndex(STAKE_NAMES, saved.stake);
    if (savedDeck >= 0) deckIndex_ = savedDeck;
    if (savedStake >= 0) stakeIndex_ = savedStake;
    useProfileUnlocks_ = saved.useProfileUnlocks;
    freshRun_ = saved.freshRun;
    timeline_ = saved.observations;
    shopSize_ = saved.shopSize;
    actions_ = saved.actions;
    branches_ = saved.branches;
    selectedBranch_ = std::clamp(saved.selectedBranch, -1, static_cast<int>(branches_.size()) - 1);
    branchName_.fill('\0');
    applyObservation(saved.draft);
}

bool SeedLab::saveInvestigation(bool showSuccess) {
    if (investigationPath_.empty()) return true;
    std::string error;
    if (!investigation().save(investigationPath_.string(), &error)) {
        searchMessage_ = error;
        searchMessageIsError_ = true;
        return false;
    }
    if (showSuccess) {
        searchMessage_ = "Investigation saved";
        searchMessageIsError_ = false;
    }
    return true;
}

void SeedLab::loadInvestigation() {
    if (investigationPath_.empty() || !std::filesystem::exists(investigationPath_)) return;
    SeedInvestigation saved;
    std::string error;
    if (!saved.load(investigationPath_.string(), &error)) {
        searchMessage_ = error;
        searchMessageIsError_ = true;
        return;
    }
    applyInvestigation(saved);
    searchMessage_ = "Investigation restored";
    searchMessageIsError_ = false;
}

void SeedLab::recordAction(SeedRunActionKind kind, int nextAnte, int nextOffset, int quantity) {
    SeedRunAction action;
    action.kind = kind;
    action.anteBefore = ante_;
    action.shopOffsetBefore = static_cast<std::size_t>(std::max(0, shopOffset_));
    action.anteAfter = std::clamp(nextAnte, 1, 8);
    action.shopOffsetAfter = static_cast<std::size_t>(std::clamp(nextOffset, 0, 200));
    action.quantity = std::clamp(quantity, 0, 20);
    actions_.push_back(action);
    ante_ = action.anteAfter;
    shopOffset_ = static_cast<int>(action.shopOffsetAfter);
    saveInvestigation(false);
}

void SeedLab::undoLastAction() {
    if (actions_.empty()) return;
    const SeedRunAction action = actions_.back();
    actions_.pop_back();
    ante_ = action.anteBefore;
    shopOffset_ = static_cast<int>(action.shopOffsetBefore);
    saveInvestigation(false);
}

SeedRunBranch SeedLab::branchSnapshot(const std::string& name) const {
    SeedRunBranch branch;
    branch.name = name;
    branch.draft = currentObservation();
    branch.observations = timeline_;
    branch.shopSize = shopSize_;
    branch.actions = actions_;
    return branch;
}

void SeedLab::applyBranch(const SeedRunBranch& branch) {
    timeline_ = branch.observations;
    shopSize_ = branch.shopSize;
    actions_ = branch.actions;
    applyObservation(branch.draft);
}

bool SeedLab::createBranch() {
    if (branches_.size() >= 50) {
        searchMessage_ = "Checkpoint limit reached";
        searchMessageIsError_ = true;
        return false;
    }

    std::string name = trim(branchName_.data());
    if (name.empty()) {
        int number = 1;
        do {
            name = "Branch " + std::to_string(number++);
        } while (std::any_of(branches_.begin(), branches_.end(), [&name](const SeedRunBranch& branch) {
            return branch.name == name;
        }));
    }
    if (std::any_of(branches_.begin(), branches_.end(), [&name](const SeedRunBranch& branch) {
            return branch.name == name;
        })) {
        searchMessage_ = "A checkpoint with that name already exists";
        searchMessageIsError_ = true;
        return false;
    }

    branches_.push_back(branchSnapshot(name));
    selectedBranch_ = static_cast<int>(branches_.size()) - 1;
    branchName_.fill('\0');
    if (!saveInvestigation(false)) return false;
    searchMessage_ = "Checkpoint saved: " + name;
    searchMessageIsError_ = false;
    return true;
}

bool SeedLab::updateBranch() {
    if (selectedBranch_ < 0 || selectedBranch_ >= static_cast<int>(branches_.size())) return false;
    const std::size_t selected = static_cast<std::size_t>(selectedBranch_);
    std::string name = trim(branchName_.data());
    if (name.empty()) name = branches_[selected].name;
    if (std::any_of(branches_.begin(), branches_.end(), [selected, &name, this](const SeedRunBranch& branch) {
            return static_cast<std::size_t>(&branch - branches_.data()) != selected && branch.name == name;
        })) {
        searchMessage_ = "A checkpoint with that name already exists";
        searchMessageIsError_ = true;
        return false;
    }
    branches_[selected] = branchSnapshot(name);
    branchName_.fill('\0');
    if (!saveInvestigation(false)) return false;
    searchMessage_ = "Checkpoint updated: " + name;
    searchMessageIsError_ = false;
    return true;
}

bool SeedLab::restoreBranch() {
    if (selectedBranch_ < 0 || selectedBranch_ >= static_cast<int>(branches_.size())) return false;
    const SeedRunBranch branch = branches_[static_cast<std::size_t>(selectedBranch_)];
    applyBranch(branch);
    if (!saveInvestigation(false)) return false;
    searchMessage_ = "Checkpoint restored: " + branch.name;
    searchMessageIsError_ = false;
    return true;
}

bool SeedLab::deleteBranch() {
    if (selectedBranch_ < 0 || selectedBranch_ >= static_cast<int>(branches_.size())) return false;
    const std::string name = branches_[static_cast<std::size_t>(selectedBranch_)].name;
    branches_.erase(branches_.begin() + selectedBranch_);
    selectedBranch_ = branches_.empty() ? -1 : std::min(selectedBranch_, static_cast<int>(branches_.size()) - 1);
    branchName_.fill('\0');
    if (!saveInvestigation(false)) return false;
    searchMessage_ = "Checkpoint deleted: " + name;
    searchMessageIsError_ = false;
    return true;
}

const TextureAsset* SeedLab::gameAsset(const std::string& set, const std::string& name) {
    const GameAssetEntry* entry = gameAssets_.find(set, name);
    if (!entry) return nullptr;
    const TextureAsset& texture = textures_.get(entry->path);
    return texture ? &texture : nullptr;
}

bool SeedLab::drawGameAsset(const std::string& set, const std::string& name, float height) {
    const TextureAsset* asset = gameAsset(set, name);
    if (!asset || asset->height <= 0) {
        ImGui::Dummy(ImVec2(0.0f, height));
        return false;
    }
    const float width = height * static_cast<float>(asset->width) / static_cast<float>(asset->height);
    ImGui::Image(
        ImTextureRef(reinterpret_cast<ImTextureID>(asset->texture)),
        ImVec2(width, height));
    return true;
}

void SeedLab::drawAssetMetric(
    const char* label,
    const std::string& set,
    const std::string& name,
    ImU32 color) {
    ImGui::TextDisabled("%s", label);
    if (gameAsset(set, name)) {
        drawGameAsset(set, name, 36.0f);
        ImGui::SameLine(0.0f, 8.0f);
    }
    Ui::badge(name.c_str(), color);
}

SeedEngine::Rules SeedLab::rulesFor(const PlayerProfile& profile) const {
    SeedEngine::Rules rules;
    if (useProfileUnlocks_) {
        std::vector<std::string> unlocked;
        unlocked.reserve(JOKER_NAMES.size());
        for (const std::string& name : JOKER_NAMES) {
            if (profile.jokers.at(name).unlocked) {
                unlocked.push_back(name);
            }
        }
        rules = SeedEngine::rulesFromUnlockedJokers(unlocked);
    }
    rules.freshRun = freshRun_;
    return rules;
}
