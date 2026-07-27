#include "ui_helpers.hpp"

#include <algorithm>
#include <cmath>
#include <cstdio>

namespace {
ImVec4 toVec4(ImU32 color) {
    return ImGui::ColorConvertU32ToFloat4(color);
}

void pushButtonColors(ImU32 normal, ImU32 hovered, ImU32 active) {
    ImGui::PushStyleColor(ImGuiCol_Button, toVec4(normal));
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, toVec4(hovered));
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, toVec4(active));
}
}

namespace Ui {
void setupStyle() {
    ImGui::StyleColorsDark();
    ImGuiStyle& style = ImGui::GetStyle();
    style.WindowPadding = ImVec2(18.0f, 14.0f);
    style.WindowRounding = 0.0f;
    style.WindowBorderSize = 0.0f;
    style.ChildRounding = 6.0f;
    style.ChildBorderSize = 0.0f;
    style.FramePadding = ImVec2(12.0f, 8.0f);
    style.FrameRounding = 7.0f;
    style.FrameBorderSize = 0.0f;
    style.ItemSpacing = ImVec2(10.0f, 9.0f);
    style.ItemInnerSpacing = ImVec2(8.0f, 6.0f);
    style.ScrollbarRounding = 6.0f;
    style.GrabRounding = 6.0f;
    style.Colors[ImGuiCol_Text] = toVec4(Text);
    style.Colors[ImGuiCol_TextDisabled] = toVec4(IM_COL32(132, 145, 145, 255));
    style.Colors[ImGuiCol_WindowBg] = toVec4(IM_COL32(0, 0, 0, 0));
    style.Colors[ImGuiCol_ChildBg] = toVec4(IM_COL32(23, 29, 31, 230));
    style.Colors[ImGuiCol_PopupBg] = toVec4(IM_COL32(28, 35, 37, 252));
    style.Colors[ImGuiCol_Border] = toVec4(IM_COL32(100, 118, 118, 120));
    style.Colors[ImGuiCol_Separator] = toVec4(IM_COL32(107, 126, 126, 110));
    style.Colors[ImGuiCol_Button] = toVec4(Dark);
    style.Colors[ImGuiCol_ButtonHovered] = toVec4(LightDark);
    style.Colors[ImGuiCol_ButtonActive] = toVec4(IM_COL32(42, 52, 54, 255));
    style.Colors[ImGuiCol_Header] = toVec4(Red);
    style.Colors[ImGuiCol_HeaderHovered] = toVec4(RedHovered);
    style.Colors[ImGuiCol_HeaderActive] = toVec4(RedActive);
}

void drawAnimatedBackground(float timeSeconds) {
    ImDrawList* drawList = ImGui::GetWindowDrawList();
    const ImVec2 origin = ImGui::GetWindowPos();
    const ImVec2 windowSize = ImGui::GetWindowSize();
    const ImVec2 extent(origin.x + windowSize.x, origin.y + windowSize.y);
    drawList->AddRectFilled(origin, extent, IM_COL32(12, 22, 24, 255));

    struct Color {
        float red;
        float green;
        float blue;
    };
    const Color color1{0.055f, 0.285f, 0.305f};
    const Color color2{0.305f, 0.115f, 0.345f};
    const Color color3{0.315f, 0.105f, 0.185f};
    constexpr float cellSize = 24.0f;
    constexpr float spinAmount = 0.16f;
    constexpr float contrast = 1.15f;
    const float screenLength = std::max(1.0f, std::sqrt(windowSize.x * windowSize.x + windowSize.y * windowSize.y));
    const int columns = static_cast<int>(windowSize.x / cellSize) + 1;
    const int rows = static_cast<int>(windowSize.y / cellSize) + 1;

    for (int row = 0; row < rows; ++row) {
        for (int column = 0; column < columns; ++column) {
            const float screenX = std::min(windowSize.x, (static_cast<float>(column) + 0.5f) * cellSize);
            const float screenY = std::min(windowSize.y, (static_cast<float>(row) + 0.5f) * cellSize);
            float uvX = (screenX - windowSize.x * 0.5f) / screenLength - 0.12f;
            float uvY = (screenY - windowSize.y * 0.5f) / screenLength;
            const float uvLength = std::sqrt(uvX * uvX + uvY * uvY);
            const float spin = timeSeconds * 0.035f + 302.2f;
            const float angle = std::atan2(uvY, uvX) + spin -
                10.0f * (spinAmount * uvLength + 1.0f - spinAmount);
            uvX = uvLength * std::cos(angle) * 30.0f;
            uvY = uvLength * std::sin(angle) * 30.0f;
            float uv2X = uvX + uvY;
            float uv2Y = uvX + uvY;
            const float speed = timeSeconds * 2.0f;
            for (int iteration = 0; iteration < 5; ++iteration) {
                const float shared = std::sin(std::max(uvX, uvY));
                uv2X += shared + uvX;
                uv2Y += shared + uvY;
                uvX += 0.5f * std::cos(5.1123314f + 0.353f * uv2Y + speed * 0.131121f);
                uvY += 0.5f * std::sin(uv2X - speed * 0.113f);
                const float displacement = std::cos(uvX + uvY) - std::sin(uvX * 0.711f - uvY);
                uvX -= displacement;
                uvY -= displacement;
            }

            const float contrastMod = 0.25f * contrast + 0.5f * spinAmount + 1.2f;
            const float paint = std::clamp(
                std::sqrt(uvX * uvX + uvY * uvY) * 0.035f * contrastMod,
                0.0f,
                2.0f);
            const float color1Weight = std::max(0.0f, 1.0f - contrastMod * std::abs(1.0f - paint));
            const float color2Weight = std::max(0.0f, 1.0f - contrastMod * std::abs(paint));
            const float color3Weight = 1.0f - std::min(1.0f, color1Weight + color2Weight);
            const float baseWeight = 0.3f / contrast;
            const float mixWeight = 1.0f - baseWeight;
            const float red = baseWeight * color1.red + mixWeight *
                (color1.red * color1Weight + color2.red * color2Weight + color3.red * color3Weight);
            const float green = baseWeight * color1.green + mixWeight *
                (color1.green * color1Weight + color2.green * color2Weight + color3.green * color3Weight);
            const float blue = baseWeight * color1.blue + mixWeight *
                (color1.blue * color1Weight + color2.blue * color2Weight + color3.blue * color3Weight);
            const auto channel = [](float value) {
                return static_cast<int>(std::clamp(value * 255.0f, 0.0f, 255.0f));
            };
            const float x = origin.x + static_cast<float>(column) * cellSize;
            const float y = origin.y + static_cast<float>(row) * cellSize;
            drawList->AddRectFilled(
                ImVec2(x, y),
                ImVec2(std::min(extent.x, x + cellSize + 1.0f), std::min(extent.y, y + cellSize + 1.0f)),
                IM_COL32(channel(red), channel(green), channel(blue), 235));
        }
    }

    for (float y = origin.y; y < extent.y; y += 4.0f) {
        drawList->AddLine(ImVec2(origin.x, y), ImVec2(extent.x, y), IM_COL32(0, 0, 0, 13));
    }
}

bool sectionTab(const char* label, bool selected, const ImVec2& size) {
    pushButtonColors(
        selected ? Red : Dark,
        selected ? RedHovered : LightDark,
        selected ? RedActive : IM_COL32(45, 55, 57, 255));
    const bool clicked = ImGui::Button(label, size);
    ImGui::PopStyleColor(3);
    return clicked;
}

bool choiceButton(const char* id, const char* label, bool selected, ImU32 selectedColor, const ImVec2& size) {
    ImGui::PushID(id);
    pushButtonColors(
        selected ? selectedColor : Dark,
        selected ? selectedColor : LightDark,
        selected ? selectedColor : IM_COL32(45, 55, 57, 255));
    const bool clicked = ImGui::Button(label, size);
    ImGui::PopStyleColor(3);
    ImGui::PopID();
    return clicked;
}

bool saveButton(bool dirty, const ImVec2& size) {
    const ImU32 normal = dirty ? Orange : Dark;
    const ImU32 hovered = dirty ? IM_COL32(255, 184, 46, 255) : LightDark;
    const ImU32 active = dirty ? IM_COL32(221, 135, 0, 255) : IM_COL32(45, 55, 57, 255);
    pushButtonColors(normal, hovered, active);
    const bool clicked = ImGui::Button(dirty ? "Save *" : "Save", size);
    ImGui::PopStyleColor(3);
    return clicked;
}

int pageNav(int currentPage, int totalPages) {
    totalPages = std::max(totalPages, 1);
    currentPage = std::clamp(currentPage, 0, totalPages - 1);

    const ImVec2 arrowSize(50.0f, 38.0f);
    const float spacing = 10.0f;
    char label[32];
    std::snprintf(label, sizeof(label), "Page %d/%d", currentPage + 1, totalPages);
    const ImVec2 textSize = ImGui::CalcTextSize(label);
    const ImVec2 pageSize(std::max(180.0f, textSize.x + 56.0f), arrowSize.y);
    const float totalWidth = arrowSize.x * 2.0f + pageSize.x + spacing * 2.0f;
    const float cursorX = ImGui::GetCursorPosX();
    const float startX = cursorX + std::max(0.0f, (ImGui::GetContentRegionAvail().x - totalWidth) * 0.5f);
    ImGui::SetCursorPosX(startX);

    ImGui::PushID("page_nav");
    pushButtonColors(Red, RedHovered, RedActive);
    const bool previousDisabled = currentPage == 0;
    if (previousDisabled) {
        ImGui::BeginDisabled();
    }
    if (ImGui::Button("<", arrowSize)) {
        currentPage--;
    }
    if (previousDisabled) {
        ImGui::EndDisabled();
    }

    ImGui::SameLine(0.0f, spacing);
    ImGui::InvisibleButton("page_label", pageSize);
    const ImVec2 min = ImGui::GetItemRectMin();
    const ImVec2 max = ImGui::GetItemRectMax();
    ImDrawList* drawList = ImGui::GetWindowDrawList();
    drawList->AddRectFilled(ImVec2(min.x + 3.0f, min.y + 4.0f), ImVec2(max.x + 3.0f, max.y + 5.0f), IM_COL32(0, 0, 0, 90), 7.0f);
    drawList->AddRectFilled(min, max, Red, 7.0f);
    drawList->AddText(
        ImVec2(min.x + (pageSize.x - textSize.x) * 0.5f, min.y + (pageSize.y - textSize.y) * 0.5f),
        Text,
        label);

    ImGui::SameLine(0.0f, spacing);
    const bool nextDisabled = currentPage >= totalPages - 1;
    if (nextDisabled) {
        ImGui::BeginDisabled();
    }
    if (ImGui::Button(">", arrowSize)) {
        currentPage++;
    }
    if (nextDisabled) {
        ImGui::EndDisabled();
    }
    ImGui::PopStyleColor(3);
    ImGui::PopID();
    return std::clamp(currentPage, 0, totalPages - 1);
}

bool backButton(float width) {
    const ImVec2 size(width, 38.0f);
    const float cursorX = ImGui::GetCursorPosX();
    const float startX = cursorX + std::max(0.0f, (ImGui::GetContentRegionAvail().x - size.x) * 0.5f);
    ImGui::SetCursorPosX(startX);
    pushButtonColors(Orange, IM_COL32(255, 184, 46, 255), IM_COL32(221, 135, 0, 255));
    const bool clicked = ImGui::Button("Back", size);
    ImGui::PopStyleColor(3);
    return clicked;
}

CardButtonResult cardButton(
    const char* id,
    ImTextureRef texture,
    ImTextureRef overlay,
    const ImVec2& imageSize,
    bool selected,
    bool unavailable,
    const char* tooltip) {
    CardButtonResult result;
    const ImVec2 buttonSize(imageSize.x + 16.0f, imageSize.y + 16.0f);
    const float cursorX = ImGui::GetCursorPosX();
    const float centeredX = cursorX + std::max(0.0f, (ImGui::GetContentRegionAvail().x - buttonSize.x) * 0.5f);
    ImGui::SetCursorPosX(centeredX);

    ImGui::InvisibleButton(id, buttonSize);
    result.clicked = ImGui::IsItemClicked();
    result.hovered = ImGui::IsItemHovered();

    const bool active = ImGui::IsItemActive();
    const ImVec2 buttonMin = ImGui::GetItemRectMin();
    const ImVec2 buttonMax = ImGui::GetItemRectMax();
    const float scale = active ? 0.98f : (result.hovered ? 1.035f : 1.0f);
    const float width = imageSize.x * scale;
    const float height = imageSize.y * scale;
    const float lift = result.hovered ? 4.0f : 0.0f;
    const ImVec2 center((buttonMin.x + buttonMax.x) * 0.5f, (buttonMin.y + buttonMax.y) * 0.5f - lift);
    result.imageMin = ImVec2(center.x - width * 0.5f, center.y - height * 0.5f);
    result.imageMax = ImVec2(center.x + width * 0.5f, center.y + height * 0.5f);

    ImDrawList* drawList = ImGui::GetWindowDrawList();
    drawList->AddRectFilled(
        ImVec2(result.imageMin.x + 5.0f, result.imageMin.y + 7.0f),
        ImVec2(result.imageMax.x + 5.0f, result.imageMax.y + 7.0f),
        IM_COL32(0, 0, 0, result.hovered ? 125 : 80),
        6.0f);

    if (texture.GetTexID() != ImTextureID_Invalid) {
        drawList->AddImage(texture, result.imageMin, result.imageMax);
    }
    else {
        drawList->AddRectFilled(result.imageMin, result.imageMax, IM_COL32(37, 43, 45, 255), 5.0f);
        const ImVec2 missingSize = ImGui::CalcTextSize("Missing");
        drawList->AddText(
            ImVec2(center.x - missingSize.x * 0.5f, center.y - missingSize.y * 0.5f),
            IM_COL32(225, 95, 90, 255),
            "Missing");
    }

    if (overlay.GetTexID() != ImTextureID_Invalid) {
        drawList->AddImage(overlay, result.imageMin, result.imageMax);
    }

    const ImU32 border = selected
        ? Orange
        : result.hovered
            ? IM_COL32(242, 246, 243, 220)
            : unavailable
                ? IM_COL32(83, 94, 95, 120)
                : IM_COL32(110, 126, 126, 120);
    drawList->AddRect(
        ImVec2(result.imageMin.x - 2.0f, result.imageMin.y - 2.0f),
        ImVec2(result.imageMax.x + 2.0f, result.imageMax.y + 2.0f),
        border,
        6.0f,
        0,
        selected ? 3.0f : 1.0f);

    if (result.hovered && tooltip && *tooltip) {
        ImGui::BeginTooltip();
        ImGui::TextUnformatted(tooltip);
        ImGui::EndTooltip();
    }

    return result;
}

void badge(const char* label, ImU32 color) {
    const ImVec2 textSize = ImGui::CalcTextSize(label);
    const ImVec2 padding(10.0f, 5.0f);
    const ImVec2 position = ImGui::GetCursorScreenPos();
    const ImVec2 size(textSize.x + padding.x * 2.0f, textSize.y + padding.y * 2.0f);
    ImGui::InvisibleButton(label, size);
    ImDrawList* drawList = ImGui::GetWindowDrawList();
    drawList->AddRectFilled(position, ImVec2(position.x + size.x, position.y + size.y), color, size.y * 0.5f);
    drawList->AddText(ImVec2(position.x + padding.x, position.y + padding.y), Text, label);
}
}
