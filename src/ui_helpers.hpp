#pragma once

#include <imgui.h>

namespace Ui {
inline constexpr ImU32 Red = IM_COL32(254, 95, 85, 255);
inline constexpr ImU32 RedHovered = IM_COL32(255, 126, 116, 255);
inline constexpr ImU32 RedActive = IM_COL32(210, 66, 60, 255);
inline constexpr ImU32 Orange = IM_COL32(253, 162, 0, 255);
inline constexpr ImU32 Dark = IM_COL32(55, 66, 68, 255);
inline constexpr ImU32 LightDark = IM_COL32(79, 99, 103, 255);
inline constexpr ImU32 Text = IM_COL32(238, 242, 240, 255);

struct CardButtonResult {
    bool clicked = false;
    bool hovered = false;
    ImVec2 imageMin;
    ImVec2 imageMax;
};

void setupStyle();
void drawAnimatedBackground(float timeSeconds);
bool sectionTab(const char* label, bool selected, const ImVec2& size);
bool choiceButton(const char* id, const char* label, bool selected, ImU32 selectedColor, const ImVec2& size);
bool saveButton(bool dirty, const ImVec2& size);
int pageNav(int currentPage, int totalPages);
bool backButton(float width = 300.0f);
CardButtonResult cardButton(
    const char* id,
    ImTextureRef texture,
    ImTextureRef overlay,
    const ImVec2& imageSize,
    bool selected,
    bool unavailable,
    const char* tooltip);
void badge(const char* label, ImU32 color);
}
