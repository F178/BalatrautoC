# Development Roadmap

## Milestone 1: Native Profile Editor

Complete.

- Ordered Joker and deck collection views
- Unlock, discovery, and stake-sticker editing
- Legacy profile import and safe profile persistence
- Balatro-inspired responsive interface

## Milestone 2: Seed Lab Foundation

Complete.

- Deterministic seed analysis
- Boss, voucher, tag, shop, edition, sticker, and pack output
- Multithreaded reverse search
- Round clues and searchable shop-item clues
- Profile-aware Joker unlock rules

## Milestone 3: Run Timeline

Complete.

- Complete: combine observations from multiple antes and shop queue offsets
- Complete: undo and clear recorded clue observations
- Complete: carry candidates through every recorded observation
- Complete: model shops, rerolls, purchases, packs, skips, and ante changes as named actions
- Complete: named branch checkpoints with restore, update, and delete
- Complete: persist and reopen the active investigation session

## Milestone 4: Visual Recognition

Complete foundation.

- Complete: reproducible extraction of 410 references from the user's game files
- Complete: original art in analyzer, shop, pack, and clue views
- Complete: import or drop PNG, JPEG, and BMP images
- Complete: select regions directly inside full screenshots
- Complete: resize- and brightness-tolerant ranked matching
- Complete: send confirmed observations directly into Seed Lab
- Complete: keep recognition reviewable instead of silently guessing
- Complete: propose likely card and square-icon regions automatically across multiple scales
- Complete: capture a visible Balatro client window on Windows
- Complete: rank detected regions while preserving whole-image matching for clean crops
- Next: continuously watch a selected window without blocking the UI
- Next: infer shop slot ordering and add multiple detected clues in one review pass

## Milestone 5: Assisted Play

- Action planner driven by the deterministic run state
- Keyboard and mouse execution on PC with a visible confirmation queue
- Controller-output adapter for Remote Play workflows
- Emergency stop, dry-run mode, and complete action logging

## Milestone 6: Renderer Fidelity

- Complete: stable Direct3D 11 SDL renderer with automatic fallback
- Complete: CPU port of the extracted background shader's animated paint field
- Next: optional shader-capable card pass
- Next: port selected CRT, holographic, foil, and polychrome effects
- Keep a low-motion mode and the current stable renderer as fallbacks
