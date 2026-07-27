# Game Asset Pipeline

Balatrauto builds its recognition catalog from a local Balatro extraction. The generated files are deterministic and are not downloaded by the build.

## Generate References

```powershell
python tools/extract_game_assets.py `
  --game-root "C:\path\to\your\BalatroFILES" `
  --output "assets\game_reference"
```

The extractor reads the game's Lua center, tag, stake, blind, and playing-card definitions, then crops the matching atlas cells. It preserves definition order and records source keys, names, sets, dimensions, and relative paths in `asset_manifest.json`.

Current output:

| Set | Count |
| --- | ---: |
| Back | 16 |
| Blind | 30 |
| Booster | 32 |
| Default | 1 |
| Edition | 5 |
| Enhanced | 8 |
| Joker | 150 |
| Planet | 12 |
| Playing Card | 52 |
| Spectral | 18 |
| Stake | 8 |
| Tag | 24 |
| Tarot | 22 |
| Voucher | 32 |
| Total | 410 |

The extractor also records the SHA-256 digest of `game.lua`. Re-running it against the same game version produces stable names and paths.

## Runtime Use

`GameAssetCatalog` provides stable `(set, name)` lookup for UI art. `ImageMatcher` indexes every manifest entry, including visually distinct entries that share a display name.

Recognition combines normalized luminance structure, color differences, a 64-bit difference hash, and aspect ratio. The test suite covers exact matching, downscaling, brightness changes, and matching a selected region inside a larger image.

## Distribution

The extractor and manifest format are project code. The generated PNG files come from the user's local game installation. Keep generated references local unless redistribution rights have been reviewed separately.
