#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>

#include "imgui.h"
#include "imgui_impl_sdl3.h"
#include "imgui_impl_sdlrenderer3.h"

#include "app_paths.hpp"
#include "profile.hpp"
#include "texture_cache.hpp"
#include "tracker_app.hpp"
#include "ui_helpers.hpp"

#include <filesystem>
#include <mutex>
#include <optional>
#include <string>

namespace {
struct FileDialogState {
    std::mutex mutex;
    std::optional<std::filesystem::path> selectedPath;
    std::string defaultLocation;
    bool open = false;
};

void SDLCALL fileDialogCallback(void* userdata, const char* const* filelist, int) {
    auto& state = *static_cast<FileDialogState*>(userdata);
    std::lock_guard<std::mutex> lock(state.mutex);
    state.open = false;
    if (filelist && filelist[0]) {
        state.selectedPath = std::filesystem::path(filelist[0]);
    }
}
}

int main(int, char**) {
    if (!SDL_Init(SDL_INIT_VIDEO)) {
        SDL_Log("SDL initialization failed: %s", SDL_GetError());
        return 1;
    }

    SDL_Window* window = SDL_CreateWindow(
        "Balatrauto",
        1100,
        780,
        SDL_WINDOW_RESIZABLE | SDL_WINDOW_HIGH_PIXEL_DENSITY);
    if (!window) {
        SDL_Log("Window creation failed: %s", SDL_GetError());
        SDL_Quit();
        return 1;
    }
    SDL_SetWindowMinimumSize(window, 920, 720);

    SDL_Renderer* renderer = SDL_CreateRenderer(window, "direct3d11");
    if (!renderer) {
        renderer = SDL_CreateRenderer(window, nullptr);
    }
    if (!renderer) {
        SDL_Log("Renderer creation failed: %s", SDL_GetError());
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }
    const bool rendererVSync = SDL_SetRenderVSync(renderer, 1);

    const AppPaths paths = AppPaths::discover();
    const std::filesystem::path profilePath = paths.profile();
    const bool profileExisted = std::filesystem::exists(profilePath);
    std::string profileMessage;
    const bool profileLoaded = playerProfile.load(profilePath.string(), &profileMessage);
    const bool initialDirty = !profileExisted;
    const bool profileError = profileExisted && !profileLoaded;

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;

    const std::filesystem::path fontPath = paths.asset(std::filesystem::path("assets") / "fonts" / "m6x11plus.ttf");
    const std::string fontFilename = fontPath.string();
    if (std::filesystem::exists(fontPath)) {
        io.Fonts->AddFontFromFileTTF(fontFilename.c_str(), 20.0f);
    }
    else {
        io.Fonts->AddFontDefault();
        profileMessage = "Game font is missing";
    }

    Ui::setupStyle();
    if (!ImGui_ImplSDL3_InitForSDLRenderer(window, renderer) || !ImGui_ImplSDLRenderer3_Init(renderer)) {
        SDL_Log("ImGui backend initialization failed");
        ImGui::DestroyContext();
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }

    bool running = true;
    {
        TextureCache textures(renderer, paths.base());
        FileDialogState profileDialogState;
        FileDialogState imageDialogState;
        profileDialogState.defaultLocation = paths.base().string();
        imageDialogState.defaultLocation = paths.base().string();
        const SDL_DialogFileFilter profileFilters[] = {
            {"JSON profile", "json"}
        };
        const SDL_DialogFileFilter imageFilters[] = {
            {"Images", "png;jpg;jpeg;bmp"}
        };
        const auto requestProfileImport = [&profileDialogState, &profileFilters, window]() {
            std::lock_guard<std::mutex> lock(profileDialogState.mutex);
            if (profileDialogState.open) {
                return;
            }
            profileDialogState.open = true;
            SDL_ShowOpenFileDialog(
                fileDialogCallback,
                &profileDialogState,
                window,
                profileFilters,
                1,
                profileDialogState.defaultLocation.c_str(),
                false);
        };
        const auto requestImageImport = [&imageDialogState, &imageFilters, window]() {
            std::lock_guard<std::mutex> lock(imageDialogState.mutex);
            if (imageDialogState.open) return;
            imageDialogState.open = true;
            SDL_ShowOpenFileDialog(
                fileDialogCallback,
                &imageDialogState,
                window,
                imageFilters,
                1,
                imageDialogState.defaultLocation.c_str(),
                false);
        };
        TrackerApp app(
            textures,
            playerProfile,
            profilePath,
            initialDirty,
            profileLoaded ? std::string() : profileMessage,
            profileError,
            requestProfileImport,
            requestImageImport);

        while (running) {
            const Uint64 frameStart = SDL_GetTicksNS();
            SDL_Event event;
            while (SDL_PollEvent(&event)) {
                ImGui_ImplSDL3_ProcessEvent(&event);
                if (event.type == SDL_EVENT_QUIT) {
                    running = false;
                }
                else if (event.type == SDL_EVENT_DROP_FILE && event.drop.data) {
                    const std::filesystem::path dropped(event.drop.data);
                    if (dropped.extension() == ".json") app.importProfile(dropped);
                    else app.importRecognitionImage(dropped);
                }
            }

            std::optional<std::filesystem::path> importedPath;
            {
                std::lock_guard<std::mutex> lock(profileDialogState.mutex);
                importedPath.swap(profileDialogState.selectedPath);
            }
            if (importedPath) {
                app.importProfile(*importedPath);
            }

            std::optional<std::filesystem::path> imagePath;
            {
                std::lock_guard<std::mutex> lock(imageDialogState.mutex);
                imagePath.swap(imageDialogState.selectedPath);
            }
            if (imagePath) {
                app.importRecognitionImage(*imagePath);
            }

            ImGui_ImplSDLRenderer3_NewFrame();
            ImGui_ImplSDL3_NewFrame();
            ImGui::NewFrame();

            const float timeSeconds = static_cast<float>(SDL_GetTicks()) / 1000.0f;
            if (!app.draw(timeSeconds)) {
                running = false;
            }

            ImGui::Render();
            SDL_SetRenderDrawColor(renderer, 12, 22, 24, 255);
            SDL_RenderClear(renderer);
            ImGui_ImplSDLRenderer3_RenderDrawData(ImGui::GetDrawData(), renderer);
            SDL_RenderPresent(renderer);
            if (!rendererVSync) {
                constexpr Uint64 frameDuration = SDL_NS_PER_SECOND / 60;
                const Uint64 elapsed = SDL_GetTicksNS() - frameStart;
                if (elapsed < frameDuration) {
                    SDL_DelayNS(frameDuration - elapsed);
                }
            }
        }

        if (app.dirty() && !app.save()) {
            SDL_Log("Profile save failed on exit");
        }
    }

    ImGui_ImplSDLRenderer3_Shutdown();
    ImGui_ImplSDL3_Shutdown();
    ImGui::DestroyContext();
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}
