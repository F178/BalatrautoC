from __future__ import annotations

import argparse
import hashlib
import json
import re
from dataclasses import dataclass
from pathlib import Path

try:
    from PIL import Image
except ImportError as error:
    raise SystemExit("Pillow is required: python -m pip install Pillow") from error


ENTRY_START = re.compile(r"^\s*([A-Za-z_][A-Za-z0-9_]*)\s*=\s*\{")


@dataclass(frozen=True)
class Atlas:
    filename: str
    logical_width: int
    logical_height: int


ATLASES = {
    "centers": Atlas("Enhancers.png", 71, 95),
    "Joker": Atlas("Jokers.png", 71, 95),
    "Tarot": Atlas("Tarots.png", 71, 95),
    "Voucher": Atlas("Vouchers.png", 71, 95),
    "Booster": Atlas("boosters.png", 71, 95),
    "cards_1": Atlas("8BitDeck.png", 71, 95),
    "tags": Atlas("tags.png", 34, 34),
    "chips": Atlas("chips.png", 29, 29),
    "blind_chips": Atlas("BlindChips.png", 34, 34),
}


SET_ATLASES = {
    "Default": "centers",
    "Enhanced": "centers",
    "Back": "centers",
    "Joker": "Joker",
    "Tarot": "Tarot",
    "Planet": "Tarot",
    "Spectral": "Tarot",
    "Voucher": "Voucher",
    "Booster": "Booster",
}


def strip_comment(line: str) -> str:
    quote = ""
    escaped = False
    for index, character in enumerate(line):
        if escaped:
            escaped = False
            continue
        if character == "\\" and quote:
            escaped = True
            continue
        if character in "'\"":
            if not quote:
                quote = character
            elif quote == character:
                quote = ""
            continue
        if not quote and character == "-" and index + 1 < len(line) and line[index + 1] == "-":
            return line[:index]
    return line


def brace_delta(text: str) -> int:
    quote = ""
    escaped = False
    delta = 0
    for character in text:
        if escaped:
            escaped = False
            continue
        if character == "\\" and quote:
            escaped = True
            continue
        if character in "'\"":
            if not quote:
                quote = character
            elif quote == character:
                quote = ""
            continue
        if quote:
            continue
        if character == "{":
            delta += 1
        elif character == "}":
            delta -= 1
    return delta


def table_entries(lua_text: str, marker: str) -> list[tuple[str, str]]:
    lines = lua_text.splitlines()
    start = next((index for index, line in enumerate(lines) if marker in line and "{" in line), None)
    if start is None:
        raise ValueError(f"Could not find {marker}")

    depth = brace_delta(strip_comment(lines[start]))
    entries: list[tuple[str, str]] = []
    current_key = ""
    current_lines: list[str] = []

    for line in lines[start + 1 :]:
        clean = strip_comment(line)
        if not current_key and depth == 1:
            match = ENTRY_START.match(clean)
            if match:
                current_key = match.group(1)
                current_lines = [clean]
        elif current_key:
            current_lines.append(clean)

        depth += brace_delta(clean)
        if current_key and depth == 1:
            entries.append((current_key, "\n".join(current_lines)))
            current_key = ""
            current_lines = []
        if depth == 0:
            break

    return entries


def string_field(entry: str, field: str) -> str:
    match = re.search(rf"\b{re.escape(field)}\s*=\s*(['\"])(.*?)\1", entry, re.DOTALL)
    return match.group(2) if match else ""


def integer_field(entry: str, field: str) -> int | None:
    match = re.search(rf"\b{re.escape(field)}\s*=\s*(-?\d+)", entry)
    return int(match.group(1)) if match else None


def position_field(entry: str) -> tuple[int, int] | None:
    match = re.search(
        r"\bpos\s*=\s*\{\s*x\s*=\s*(-?\d+)\s*,\s*y\s*=\s*(-?\d+)",
        entry,
        re.DOTALL,
    )
    return (int(match.group(1)), int(match.group(2))) if match else None


def safe_name(value: str) -> str:
    value = re.sub(r"[<>:\"/\\|?*\x00-\x1f]", "_", value).strip(" .")
    value = re.sub(r"\s+", " ", value)
    return value or "unnamed"


def center_assets(lua_text: str) -> list[dict[str, object]]:
    assets: list[dict[str, object]] = []
    for key, entry in table_entries(lua_text, "self.P_CENTERS ="):
        name = string_field(entry, "name")
        item_set = string_field(entry, "set")
        position = position_field(entry)
        if not name or not item_set or position is None:
            continue
        atlas_name = string_field(entry, "atlas") or SET_ATLASES.get(item_set, "")
        if atlas_name not in ATLASES:
            continue
        assets.append(
            {
                "key": key,
                "name": name,
                "set": item_set,
                "order": integer_field(entry, "order"),
                "atlas": atlas_name,
                "x": position[0],
                "y": position[1],
            }
        )
    return assets


def table_assets(
    lua_text: str,
    marker: str,
    item_set: str,
    atlas_name: str,
) -> list[dict[str, object]]:
    assets: list[dict[str, object]] = []
    for key, entry in table_entries(lua_text, marker):
        name = string_field(entry, "name")
        position = position_field(entry)
        if not name or position is None:
            continue
        assets.append(
            {
                "key": key,
                "name": name,
                "set": item_set,
                "order": integer_field(entry, "order"),
                "atlas": atlas_name,
                "x": position[0],
                "y": position[1],
            }
        )
    return assets


def collect_assets(lua_text: str) -> list[dict[str, object]]:
    assets = center_assets(lua_text)
    assets.extend(table_assets(lua_text, "self.P_TAGS =", "Tag", "tags"))
    assets.extend(table_assets(lua_text, "self.P_STAKES =", "Stake", "chips"))
    assets.extend(table_assets(lua_text, "self.P_BLINDS =", "Blind", "blind_chips"))
    assets.extend(table_assets(lua_text, "self.P_CARDS =", "Playing Card", "cards_1"))
    return sorted(
        assets,
        key=lambda asset: (
            str(asset["set"]),
            int(asset["order"]) if asset["order"] is not None else 9999,
            str(asset["key"]),
        ),
    )


def extract(game_root: Path, output: Path, scale: int) -> dict[str, object]:
    lua_path = game_root / "out" / "game.lua"
    texture_root = game_root / "resources" / "textures" / f"{scale}x"
    if not lua_path.is_file():
        raise FileNotFoundError(f"Missing game metadata: {lua_path}")
    if not texture_root.is_dir():
        raise FileNotFoundError(f"Missing texture folder: {texture_root}")

    lua_bytes = lua_path.read_bytes()
    lua_text = lua_bytes.decode("utf-8", errors="replace")
    source_assets = collect_assets(lua_text)
    output.mkdir(parents=True, exist_ok=True)

    images: dict[str, Image.Image] = {}
    manifest_assets: list[dict[str, object]] = []
    counts: dict[str, int] = {}

    for asset in source_assets:
        atlas_name = str(asset["atlas"])
        atlas = ATLASES[atlas_name]
        if atlas_name not in images:
            atlas_path = texture_root / atlas.filename
            if not atlas_path.is_file():
                raise FileNotFoundError(f"Missing atlas: {atlas_path}")
            images[atlas_name] = Image.open(atlas_path).convert("RGBA")

        cell_width = atlas.logical_width * scale
        cell_height = atlas.logical_height * scale
        left = int(asset["x"]) * cell_width
        top = int(asset["y"]) * cell_height
        right = left + cell_width
        bottom = top + cell_height
        image = images[atlas_name]
        if left < 0 or top < 0 or right > image.width or bottom > image.height:
            raise ValueError(
                f"{asset['key']} references {atlas_name} cell ({asset['x']}, {asset['y']}) "
                f"outside {image.width}x{image.height}"
            )

        item_set = str(asset["set"])
        order = int(asset["order"]) if asset["order"] is not None else 999
        filename = f"{order:03d}_{safe_name(str(asset['name']))}__{asset['key']}.png"
        relative_path = Path(safe_name(item_set)) / filename
        destination = output / relative_path
        destination.parent.mkdir(parents=True, exist_ok=True)
        image.crop((left, top, right, bottom)).save(destination)

        record = dict(asset)
        record["path"] = relative_path.as_posix()
        record["width"] = cell_width
        record["height"] = cell_height
        manifest_assets.append(record)
        counts[item_set] = counts.get(item_set, 0) + 1

    manifest = {
        "version": 1,
        "scale": scale,
        "game_lua_sha256": hashlib.sha256(lua_bytes).hexdigest(),
        "counts": dict(sorted(counts.items())),
        "assets": manifest_assets,
    }
    (output / "asset_manifest.json").write_text(json.dumps(manifest, indent=2) + "\n", encoding="utf-8")
    (output / ".balatrauto-generated-assets").write_text("generated\n", encoding="ascii")
    return manifest


def main() -> int:
    parser = argparse.ArgumentParser(description="Extract Balatro sprites using the game's Lua atlas metadata")
    parser.add_argument("--game-root", type=Path, required=True)
    parser.add_argument("--output", type=Path, required=True)
    parser.add_argument("--scale", type=int, choices=(1, 2), default=2)
    arguments = parser.parse_args()

    manifest = extract(arguments.game_root.resolve(), arguments.output.resolve(), arguments.scale)
    print(f"Extracted {len(manifest['assets'])} assets to {arguments.output.resolve()}")
    for item_set, count in manifest["counts"].items():
        print(f"  {item_set}: {count}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
