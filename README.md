# Balatrauto

Balatrauto is a native Windows companion app for Balatro. It combines a visual collection/profile editor with deterministic seed analysis and reverse search.

## Current Features

- Edit all 150 Jokers in Balatro collection order.
- Track locked, unlocked, and discovered Joker states.
- Edit all 15 deck unlocks.
- Assign and remove stake stickers with the original icon assets.
- Import the legacy Python `player_profile.json` format.
- Save ordered, human-readable profiles with automatic backup files.
- Analyze an eight-character Balatro seed for boss, voucher, tags, shop items, editions, stickers, and booster packs.
- Reverse-search a seed range using visible boss, voucher, tags, and exact shop slots.
- Combine clues from multiple antes and shop queue offsets in one investigation timeline.
- Search all shop item types: Jokers, Tarot cards, Planets, Spectrals, and playing cards.
- Use the active profile's Joker unlock rules during analysis and search.
- Run cancellable multithreaded searches with progress and throughput reporting.
- Record shops, rerolls, purchases, packs, skipped blinds, and ante changes with one-step undo.
- Save, restore, update, and delete named run checkpoints without losing the active line.
- Display original game art for analyzed bosses, vouchers, tags, shop items, and booster packs.
- Identify imported images against 410 generated game references with ranked confidence scores.
- Select a region directly inside a full screenshot and send a confirmed match into Seed Lab.
- Detect likely card and icon regions automatically across multiple on-screen scales.
- Capture the visible Balatro client area directly on Windows and rank detected clues.

## Running The App

The packaged Windows build is self-contained. Extract the ZIP and open `Balatrauto.exe`. Keep the `Icons`, `assets`, and `SDL3.dll` entries beside the executable.

The app stores `player_profile.json` beside the executable. Import accepts both the original boolean Joker values and Balatrauto's richer unlock/discovery values.

## Seed Lab

`Analyze` shows a deterministic fresh-run view of a seed for the selected deck, stake, and ante.

`Reverse search` scans forward from a starting seed. Add any clues you know and leave unknown fields as `Any`. Shop selectors have a filter box; typing part of a name is the fastest way to find an item.

`Clue timeline` preserves multiple observations from the same run. Set the ante and shop queue offset, enter what you saw, and add the clues before moving to the next observation. Reverse search requires every saved observation to match.

The active investigation is restored automatically from `seed_investigation.json`. Saved observations and the current draft use safe replacement writes and keep a `.bak` backup.

Shop queue offsets count generated shop items. Offset `0` starts at the first item; offset `2` starts after the first two items. Higher-level purchase, reroll, and pack actions will build on this deterministic queue model.

Run controls advance the queue using the current shop size, distinguish new shops from rerolls, log purchases, packs, and skips, and support one-step undo. The shop and reroll offsets follow the generation calls in Balatro's extracted Lua.

Named checkpoints preserve the current draft, clue timeline, run actions, queue position, and shop size. Restoring a checkpoint replaces the active run state without deleting other branches.

## Image Recognition

`Identify image` accepts PNG, JPEG, or BMP files. The full image appears in the selection panel; drag over any visible card, Joker, tag, blind, voucher, deck, stake, or pack and choose `Match selection`.

`Detect regions` proposes card-shaped and square game-art areas, scores each proposal against the local reference index, and keeps the strongest candidates available in a reviewable selector. The complete image remains a candidate so clean crops are never degraded by detection.

`Capture Balatro` finds a visible `Balatro.exe` window, captures only its client area, detects likely regions, and opens the strongest match. The game must be visible and not minimized because capture deliberately reads visible desktop pixels rather than injecting into the game.

The matcher ranks local references and always waits for confirmation. Confirmed bosses, vouchers, tags, decks, stakes, and shop items are applied to the current reverse-search draft. Images can also be dropped directly onto the app window.

The matcher uses structural luminance, color, perceptual hashing, and aspect-ratio signals. It is designed to tolerate resizing and moderate brightness changes while keeping the result reviewable.

## Building

Requirements:

- Windows 10 or 11
- Visual Studio 2022 with Desktop development with C++
- CMake and Ninja from the Visual Studio installation

Open the repository as a CMake project in Visual Studio and select `x64-debug` or `x64-release`.

From a Visual Studio Developer Command Prompt:

```powershell
cmake --preset x64-release
cmake --build out/build/x64-release --parallel
ctest --test-dir out/build/x64-release --output-on-failure
cpack --config out/build/x64-release/CPackConfig.cmake
```

On the standard Visual Studio Community installation used by this project, `tools\build_windows.cmd x64-debug` and `tools\build_windows.cmd x64-release` perform configure and build with the bundled toolchain.

## Project Layout

- `src/profile.*`: compatible profile loading, migration, and safe saving.
- `src/tracker_app.*`: Joker, deck, and stake editor.
- `src/seed_engine.*`: deterministic analysis and threaded reverse search.
- `src/seed_lab.*`: Seed Lab interface.
- `src/seed_session.*`: persistent observations, actions, and branch checkpoints.
- `src/game_asset_catalog.*`: generated game-art manifest and lookup layer.
- `src/image_matcher.*`: local perceptual image index and ranked matching.
- `src/image_region_detector.*`: multi-scale screenshot region proposals.
- `src/window_capture.*`: visible Balatro client capture on Windows.
- `src/ui_helpers.*`: Balatro-inspired controls and animated background.
- `tools/extract_game_assets.py`: reproducible atlas-to-reference extraction.
- `third_party/balatro-seed-finder`: pinned seed-generation engine subset.
- `tests`: profile, seed-engine, asset-catalog, image-matcher, and region-detector regression tests.

## Third-Party Work

The deterministic engine is adapted from `izanagi1995/balatro-seed-finder` and remains under CC BY-NC-SA 4.0. See `THIRD_PARTY_NOTICES.md` and the bundled license. SDL, Dear ImGui, nlohmann/json, and stb retain their respective licenses.

Balatro is a game by LocalThunk and published by Playstack. Balatrauto is an independent companion project and is not affiliated with or endorsed by LocalThunk or Playstack. Generated game references are intended for local development and must be reviewed before any public redistribution.
