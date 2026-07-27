# Architecture

## Application Shell

`main.cpp` owns SDL, the renderer, ImGui, native file dialogs, drag-and-drop routing, and the frame loop. `AppPaths` resolves every runtime path from the executable directory so packaged builds do not depend on the launch directory.

## Collection Tracker

`TrackerApp` owns section navigation and the Joker, deck, and stake editor. `PlayerProfile` migrates legacy boolean Joker values into explicit unlock and discovery state, fills missing game entries, preserves collection order on save, and uses backup plus replacement writes.

## Seed Lab

`SeedEngine` is the deterministic domain layer. It wraps the pinned seed-finder logic and exposes analysis, observations, and cancellable multithreaded reverse search.

`SeedSession` persists the active investigation, clue timeline, named actions, and branch checkpoints. `SeedLab` is the UI and orchestration layer; it does not own the random-generation algorithm.

## Visual Catalog

`extract_game_assets.py` converts game definitions and atlases into a deterministic manifest. `GameAssetCatalog` resolves display art. `ImageMatcher` builds a local perceptual index and returns ranked candidates. `ImageRegionDetector` balances high-saliency proposals across several visual scales, while `WindowCapture` acquires the visible Balatro client area without injecting into the game. Seed Lab only mutates a clue after the user confirms a result.

## Rendering

SDL selects Direct3D 11 when available and falls back to the default renderer. Dear ImGui provides the operational UI. `Ui::drawAnimatedBackground` ports the extracted background shader's paint-field math to a low-risk CPU mesh; card shaders remain isolated from the core renderer until a dedicated shader path is added.

## Test Boundaries

- `profile_round_trip`: migration, ordered save, backup, and replacement behavior
- `seed_engine`: deterministic analysis, search, actions, persistence, and checkpoints
- `asset_catalog`: manifest loading and stable asset lookup
- `image_matcher`: exact, transformed, and selected-region recognition
- `image_region_detector`: multi-scale proposal coverage on a synthetic screenshot
