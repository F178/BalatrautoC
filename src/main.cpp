#include <SDL3/SDL.h>
#include "imgui.h"
#include "imgui_impl_sdl3.h"
#include "imgui_impl_sdlrenderer3.h"
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include <algorithm>
#include <cstring>
#include <string>
#include <unordered_map>
#include "data.hpp"
#include "profile.hpp"
#include "ui_helpers.hpp"

static constexpr const char* PROFILE_PATH = "player_profile.json";

static SDL_Texture* LoadTexture(SDL_Renderer* renderer, const char* filename) {
    int width = 0;
    int height = 0;
    int channels = 0;
    unsigned char* pixels = stbi_load(filename, &width, &height, &channels, STBI_rgb_alpha);
    if (!pixels) {
        SDL_Log("Failed to load image '%s': %s", filename, stbi_failure_reason());
        return nullptr;
    }

    SDL_Surface* surface = SDL_CreateSurface(width, height, SDL_PIXELFORMAT_RGBA32);
    if (!surface) {
        SDL_Log("Failed to create surface for '%s': %s", filename, SDL_GetError());
        stbi_image_free(pixels);
        return nullptr;
    }

    for (int y = 0; y < height; ++y) {
        std::memcpy(
            static_cast<unsigned char*>(surface->pixels) + static_cast<size_t>(y) * surface->pitch,
            pixels + static_cast<size_t>(y) * width * 4,
            static_cast<size_t>(width) * 4
        );
    }

    SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
    SDL_DestroySurface(surface);
    stbi_image_free(pixels);

    if (!texture) {
        SDL_Log("Failed to create texture for '%s': %s", filename, SDL_GetError());
    }

    return texture;
}

static std::unordered_map<std::string, SDL_Texture*> textureCache;

static SDL_Texture* GetJokerTexture(const std::string& name, SDL_Renderer* renderer) {
    const auto existing = textureCache.find(name);
    if (existing != textureCache.end()) {
        return existing->second;
    }

    SDL_Texture* texture = LoadTexture(renderer, ("Icons/Jokers/" + name + ".png").c_str());
    if (texture) {
        textureCache.emplace(name, texture);
    }
    return texture;
}

int main(int, char*[]) {
    if (!SDL_Init(SDL_INIT_VIDEO)) {
        SDL_Log("SDL_Init failed: %s", SDL_GetError());
        return 1;
    }

    SDL_Window* window = SDL_CreateWindow(
        "Balatrauto - Jokers",
        900,
        900,
        SDL_WINDOW_RESIZABLE
    );

    if (!window) {
        SDL_Log("SDL_CreateWindow failed: %s", SDL_GetError());
        SDL_Quit();
        return 1;
    }

    SDL_Renderer* renderer = SDL_CreateRenderer(window, nullptr);
    if (!renderer) {
        SDL_Log("SDL_CreateRenderer failed: %s", SDL_GetError());
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }

    if (!playerProfile.load(PROFILE_PATH)) {
        playerProfile.save(PROFILE_PATH);
    }

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.Fonts->AddFontFromFileTTF("assets/fonts/m6x11plus.ttf", 16.0f);

    ImGui::StyleColorsDark();
    ImGuiStyle& style = ImGui::GetStyle();
    style.WindowPadding = ImVec2(16.0f, 16.0f);
    style.FramePadding = ImVec2(10.0f, 6.0f);
    style.ItemSpacing = ImVec2(10.0f, 10.0f);
    style.FrameRounding = 8.0f;
    style.WindowRounding = 0.0f;
    style.FrameBorderSize = 0.0f;

    if (!ImGui_ImplSDL3_InitForSDLRenderer(window, renderer) || !ImGui_ImplSDLRenderer3_Init(renderer)) {
        SDL_Log("Failed to initialize ImGui backends");
        ImGui::DestroyContext();
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }

    constexpr int jokersPerPage = 15;
    constexpr int columns = 5;
    const ImVec2 cardSize(90.0f, 130.0f);

    bool running = true;
    bool profileDirty = false;
    int currentPage = 0;
    int selectedIndex = -1;

    while (running) {
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            ImGui_ImplSDL3_ProcessEvent(&event);
            if (event.type == SDL_EVENT_QUIT) {
                running = false;
            }
        }

        ImGui_ImplSDL3_NewFrame();
        ImGui_ImplSDLRenderer3_NewFrame();
        ImGui::NewFrame();

        ImGui::SetNextWindowPos(ImVec2(0.0f, 0.0f), ImGuiCond_Always);
        ImGui::SetNextWindowSize(io.DisplaySize, ImGuiCond_Always);
        ImGui::Begin(
            "##Balatrauto",
            nullptr,
            ImGuiWindowFlags_NoDecoration |
            ImGuiWindowFlags_NoMove |
            ImGuiWindowFlags_NoResize |
            ImGuiWindowFlags_NoSavedSettings
        );

        const int totalPages = (static_cast<int>(JOKER_NAMES.size()) + jokersPerPage - 1) / jokersPerPage;
        currentPage = DrawPageNav(currentPage, totalPages);

        const int start = currentPage * jokersPerPage;
        const int end = std::min(start + jokersPerPage, static_cast<int>(JOKER_NAMES.size()));
        const float gridWidth = columns * cardSize.x + (columns - 1) * ImGui::GetStyle().ItemSpacing.x;

        ImGui::Spacing();

        for (int row = 0; row < 3; ++row) {
            const float availableWidth = ImGui::GetContentRegionAvail().x;
            if (availableWidth > gridWidth) {
                ImGui::SetCursorPosX(ImGui::GetCursorPosX() + (availableWidth - gridWidth) * 0.5f);
            }

            for (int column = 0; column < columns; ++column) {
                const int index = start + row * columns + column;
                if (index >= end) {
                    break;
                }

                const std::string& name = JOKER_NAMES[index];
                const JokerStatus& status = playerProfile.jokers[name];

                SDL_Texture* texture = nullptr;
                if (!status.unlocked) {
                    texture = GetJokerTexture("Locked_Joker", renderer);
                } else if (!status.discovered) {
                    texture = GetJokerTexture("Undiscovered", renderer);
                } else {
                    texture = GetJokerTexture(name, renderer);
                }

                ImGui::PushID(index);
                if (texture) {
                    const ImTextureID textureId = reinterpret_cast<ImTextureID>(texture);
                    if (ImGui::ImageButton("##joker", textureId, cardSize)) {
                        selectedIndex = index;
                    }
                } else {
                    if (ImGui::Button("Missing", cardSize)) {
                        selectedIndex = index;
                    }
                }
                ImGui::PopID();

                if (column < columns - 1 && index + 1 < end) {
                    ImGui::SameLine();
                }
            }
        }

        if (selectedIndex >= 0 && selectedIndex < static_cast<int>(JOKER_NAMES.size())) {
            const std::string& selectedName = JOKER_NAMES[selectedIndex];
            JokerStatus& status = playerProfile.jokers[selectedName];

            ImGui::Spacing();
            ImGui::Separator();
            ImGui::Spacing();
            ImGui::TextUnformatted(selectedName.c_str());

            if (status.unlocked) {
                if (ImGui::Button("Lock")) {
                    status.unlocked = false;
                    status.discovered = false;
                    profileDirty = true;
                }
            } else if (ImGui::Button("Unlock")) {
                status.unlocked = true;
                profileDirty = true;
            }

            ImGui::SameLine();
            if (!status.unlocked) {
                ImGui::BeginDisabled();
            }

            if (status.discovered) {
                if (ImGui::Button("Undiscover")) {
                    status.discovered = false;
                    profileDirty = true;
                }
            } else if (ImGui::Button("Discover")) {
                status.discovered = true;
                profileDirty = true;
            }

            if (!status.unlocked) {
                ImGui::EndDisabled();
            }

            ImGui::SameLine();
            if (ImGui::Button("Close")) {
                selectedIndex = -1;
            }
        }

        if (DrawBackButton()) {
            running = false;
        }

        ImGui::End();
        ImGui::Render();

        SDL_SetRenderDrawColor(renderer, 31, 27, 36, 255);
        SDL_RenderClear(renderer);
        ImGui_ImplSDLRenderer3_RenderDrawData(ImGui::GetDrawData(), renderer);
        SDL_RenderPresent(renderer);
    }

    if (profileDirty) {
        playerProfile.save(PROFILE_PATH);
    }

    ImGui_ImplSDLRenderer3_Shutdown();
    ImGui_ImplSDL3_Shutdown();
    ImGui::DestroyContext();

    for (const auto& [name, texture] : textureCache) {
        if (texture) {
            SDL_DestroyTexture(texture);
        }
    }

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}
