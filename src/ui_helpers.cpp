#include "ui_helpers.hpp"

#include "imgui.h"
#include <algorithm>
#include <cstdio>

int DrawPageNav(int currentPage, int totalPages) {
    if (totalPages <= 0) {
        return 0;
    }

    currentPage = std::clamp(currentPage, 0, totalPages - 1);

    constexpr float arrowWidth = 46.0f;
    constexpr float pageWidth = 170.0f;
    constexpr float height = 42.0f;
    constexpr float spacing = 8.0f;
    constexpr float rounding = 10.0f;

    const float totalWidth = arrowWidth * 2.0f + pageWidth + spacing * 2.0f;
    const float availableWidth = ImGui::GetContentRegionAvail().x;
    if (availableWidth > totalWidth) {
        ImGui::SetCursorPosX(ImGui::GetCursorPosX() + (availableWidth - totalWidth) * 0.5f);
    }

    ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, rounding);
    ImGui::PushStyleColor(ImGuiCol_Button, IM_COL32(168, 40, 45, 255));
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, IM_COL32(200, 54, 58, 255));
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, IM_COL32(143, 31, 36, 255));

    const bool canGoBack = currentPage > 0;
    if (!canGoBack) {
        ImGui::BeginDisabled();
    }
    if (ImGui::Button("<##page_prev", ImVec2(arrowWidth, height))) {
        --currentPage;
    }
    if (!canGoBack) {
        ImGui::EndDisabled();
    }

    ImGui::SameLine(0.0f, spacing);

    char pageLabel[32];
    std::snprintf(pageLabel, sizeof(pageLabel), "Page %d/%d##page_label", currentPage + 1, totalPages);
    ImGui::Button(pageLabel, ImVec2(pageWidth, height));

    ImGui::SameLine(0.0f, spacing);

    const bool canGoForward = currentPage < totalPages - 1;
    if (!canGoForward) {
        ImGui::BeginDisabled();
    }
    if (ImGui::Button(">##page_next", ImVec2(arrowWidth, height))) {
        ++currentPage;
    }
    if (!canGoForward) {
        ImGui::EndDisabled();
    }

    ImGui::PopStyleColor(3);
    ImGui::PopStyleVar();

    return currentPage;
}

bool DrawBackButton() {
    constexpr float width = 180.0f;
    constexpr float height = 44.0f;
    constexpr float rounding = 10.0f;

    const float targetY = ImGui::GetWindowHeight() - height - ImGui::GetStyle().WindowPadding.y;
    if (ImGui::GetCursorPosY() < targetY) {
        ImGui::SetCursorPosY(targetY);
    }

    const float availableWidth = ImGui::GetContentRegionAvail().x;
    if (availableWidth > width) {
        ImGui::SetCursorPosX(ImGui::GetCursorPosX() + (availableWidth - width) * 0.5f);
    }

    ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, rounding);
    ImGui::PushStyleColor(ImGuiCol_Button, IM_COL32(236, 126, 32, 255));
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, IM_COL32(255, 151, 50, 255));
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, IM_COL32(205, 101, 20, 255));

    const bool clicked = ImGui::Button("Back##back", ImVec2(width, height));

    ImGui::PopStyleColor(3);
    ImGui::PopStyleVar();

    return clicked;
}
