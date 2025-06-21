#include <SDL3/SDL.h>
#include "imgui.h"
#include "imgui_impl_sdl3.h"
#include "imgui_impl_sdlrenderer3.h"
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include <cstring>
#include <string>
#include <unordered_map>
#include "data.hpp"
#include "profile.hpp"

// Path to player profile JSON
static constexpr const char* PROFILE_PATH = "player_profile.json";

// Load an SDL_Texture from disk
static SDL_Texture* LoadTexture(SDL_Renderer* renderer, const char* filename) {
    int w, h, channels;
    unsigned char* data = stbi_load(filename, &w, &h, &channels, STBI_rgb_alpha);
    if (!data) {
        SDL_Log("Failed to load image: %s", filename);
        return nullptr;
    }
    SDL_Surface* surface = SDL_CreateSurface(w, h, SDL_PIXELFORMAT_RGBA32);
    if (!surface) {
        SDL_Log("Failed to create surface: %s", SDL_GetError());
        stbi_image_free(data);
        return nullptr;
    }
    std::memcpy(surface->pixels, data, static_cast<size_t>(w) * static_cast<size_t>(h) * 4);
    SDL_Texture* tex = SDL_CreateTextureFromSurface(renderer, surface);
    SDL_DestroySurface(surface);
    stbi_image_free(data);
    return tex;
}

// Texture cache
static std::unordered_map<std::string, SDL_Texture*> textureCache;

// Retrieve or load a joker texture by name
static SDL_Texture* getTextureForJoker(const std::string& name, SDL_Renderer* renderer) {
    if (auto it = textureCache.find(name); it != textureCache.end())
        return it->second;
    std::string path = std::string("Icons/Jokers/") + name + ".png";
    SDL_Texture* tex = LoadTexture(renderer, path.c_str());
    if (tex)
        textureCache[name] = tex;
    return tex;
}

int main(int argc, char* argv[]) {
    // Initialize SDL (video only)
    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        SDL_Log("SDL_Init Error: %s", SDL_GetError());
        return 1;
    }

    // Create a 1024x768 resizable window
    SDL_Window* window = SDL_CreateWindow(
        "Balatrauto - Jokers",
        SDL_WINDOW_RESIZABLE,
        1024, 768
    );
    if (!window) {
        SDL_Log("SDL_CreateWindow Error: %s", SDL_GetError());
        SDL_Quit();
        return 1;
    }

    // Ensure normal mouse behavior (no grab)
    SDL_SetWindowMouseMode(window, SDL_MOUSEMODE_NORMAL);

    // Create renderer
    SDL_Renderer* renderer = SDL_CreateRenderer(window, nullptr);
    if (!renderer) {
        SDL_Log("SDL_CreateRenderer Error: %s", SDL_GetError());
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }

    // Load or create player profile
    playerProfile.load(PROFILE_PATH);

    // Setup ImGui
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    io.Fonts->AddFontFromFileTTF("assets/fonts/m6x11plus.ttf", 16.0f);
    ImGui_ImplSDL3_InitForSDLRenderer(window, renderer);
    ImGui_ImplSDLRenderer3_Init(renderer);
    ImGui::StyleColorsDark();

    const int jokersPerPage = 15;
    const int columns = 5;
    const ImVec2 cardSize(90, 130);

    bool running = true;
    int currentPage = 0;
    int selectedIndex = -1;
    SDL_Event event;

    while (running) {
        // Poll and handle events
        while (SDL_PollEvent(&event)) {
            ImGui_ImplSDL3_ProcessEvent(&event);
            if (event.type == SDL_EVENT_QUIT)
                running = false;
        }

        // Begin ImGui frame
        ImGui_ImplSDL3_NewFrame();
        ImGui_ImplSDLRenderer3_NewFrame();
        ImGui::NewFrame();

        // Main Jokers window (fixed size, no collapse or resize)
        ImGui::SetNextWindowSize(ImVec2(1024, 768), ImGuiCond_Always);
        ImGui::Begin("Jokers", nullptr,
            ImGuiWindowFlags_NoCollapse |
            ImGuiWindowFlags_NoResize);

        int totalPages = (static_cast<int>(JOKER_NAMES.size()) + jokersPerPage - 1) / jokersPerPage;
        int start = currentPage * jokersPerPage;
        int end = std::min(start + jokersPerPage, static_cast<int>(JOKER_NAMES.size()));

        // Page navigation
        ImGui::Text("Page %d / %d", currentPage + 1, totalPages);
        if (ImGui::ArrowButton("##prev", ImGuiDir_Left) && currentPage > 0) currentPage--;
        ImGui::SameLine();
        if (ImGui::ArrowButton("##next", ImGuiDir_Right) && currentPage < totalPages - 1) currentPage++;

        ImGui::Spacing(); ImGui::Separator(); ImGui::Spacing();

        // Joker grid
        for (int row = 0; row < 3; ++row) {
            for (int col = 0; col < columns; ++col) {
                int idx = start + row * columns + col;
                if (idx >= end) break;
                const std::string& name = JOKER_NAMES[idx];
                const JokerStatus& st = playerProfile.jokers[name];
                SDL_Texture* tex = !st.unlocked ? getTextureForJoker("Locked_Joker", renderer)
                    : !st.discovered ? getTextureForJoker("Undiscovered", renderer)
                    : getTextureForJoker(name, renderer);
                if (!tex) continue;

                ImTextureID imgId = reinterpret_cast<ImTextureID>(tex);
                std::string btnID = "##joker_" + std::to_string(idx);
                if (ImGui::ImageButton(btnID.c_str(), imgId, cardSize)) {
                    selectedIndex = idx;
                }
                if (col < columns - 1)
                    ImGui::SameLine();
            }
        }

        // Detail panel
        if (selectedIndex != -1) {
            const std::string& sel = JOKER_NAMES[selectedIndex];
            JokerStatus& st = playerProfile.jokers[sel];
            ImGui::Spacing(); ImGui::Separator(); ImGui::Spacing();
            ImGui::Text("Selected: %s", sel.c_str());

            if (st.unlocked) {
                if (ImGui::Button("Lock")) { st.unlocked = false; st.discovered = false; }
            }
            else {
                if (ImGui::Button("Unlock")) { st.unlocked = true; }
            }
            ImGui::SameLine();
            if (!st.unlocked) ImGui::BeginDisabled();
            if (st.discovered) {
                if (ImGui::Button("Undiscover")) st.discovered = false;
            }
            else {
                if (ImGui::Button("Discover")) st.discovered = true;
            }
            if (!st.unlocked) ImGui::EndDisabled();

            ImGui::Spacing();
            if (ImGui::Button("Close")) selectedIndex = -1;
        }
        // -----------------------------------------------------------------------------
//  build the page‐nav & “back” footer
// -----------------------------------------------------------------------------

// pull style ref
        ImGuiStyle& style = ImGui::GetStyle();

        // colors from your game UI
        ImU32 colRed = IM_COL32(184, 36, 36, 255);
        ImU32 colRedH = IM_COL32(204, 56, 56, 255);
        ImU32 colGray = IM_COL32(48, 52, 60, 255);
        ImU32 colOrange = IM_COL32(238, 130, 30, 255);

        // footer container: full width, height = button height + padding
        float footerH = 48.0f;
        ImGui::SetCursorPosY(ImGui::GetWindowHeight() - footerH - style.WindowPadding.y);
        ImGui::BeginChild("##footer", ImVec2(0, footerH), false, ImGuiWindowFlags_NoScrollbar);

        // center everything
        float avail = ImGui::GetContentRegionAvail().x;
        float blockW = 50 + 200 + 50; // arrow + page + arrow
        float leftX = (avail - blockW) * 0.5f;
        ImGui::SetCursorPosX(leftX);

        //  ◀  button
        ImGui::PushStyleColor(ImGuiCol_Button, colRed);
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, colRedH);
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, colRedH);
        ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 8.0f);
        if (ImGui::ArrowButton("##prev", ImGuiDir_Left) && currentPage > 0)
            currentPage--;

        // spacing then Page X/Y pill
        ImGui::SameLine(); ImGui::Dummy(ImVec2(10, 0)); ImGui::SameLine();
        char buf[32]; sprintf(buf, "Page %d/%d", currentPage + 1, totalPages);
        if (ImGui::Button(buf, ImVec2(200, 40))) {
            // nothing — just visual
        }

        // ▶ button
        ImGui::SameLine(); ImGui::Dummy(ImVec2(10, 0)); ImGui::SameLine();
        if (ImGui::ArrowButton("##next", ImGuiDir_Right) && currentPage < totalPages - 1)
            currentPage++;

        // restore
        ImGui::PopStyleVar();
        ImGui::PopStyleColor(3);

        // now the full-width “Back” bar
        ImGui::Dummy(ImVec2(0, 8));  // small vertical gap
        ImGui::PushStyleColor(ImGuiCol_Button, colOrange);
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, IM_COL32(255, 150, 50, 255));
        ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 8.0f);
        ImGui::SetCursorPosX((avail - 300) * 0.5f);
        if (ImGui::Button("Back", ImVec2(300, 36))) {
            // your “go back” logic here
        }
        ImGui::PopStyleVar();
        ImGui::PopStyleColor(2);

        ImGui::EndChild();


        ImGui::End();

        // Render
        ImGui::Render();
        SDL_SetRenderDrawColor(renderer, 31, 27, 36, 255);
        SDL_RenderClear(renderer);
        ImGui_ImplSDLRenderer3_RenderDrawData(ImGui::GetDrawData(), renderer);
        SDL_RenderPresent(renderer);
    }

    // Save on exit
    playerProfile.save(PROFILE_PATH);

    // Cleanup
    ImGui_ImplSDLRenderer3_Shutdown();
    ImGui_ImplSDL3_Shutdown();
    ImGui::DestroyContext();
    for (auto& kv : textureCache) if (kv.second) SDL_DestroyTexture(kv.second);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}
