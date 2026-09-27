# Money Mod (Geode / Geometry Dash)

Adds a price tag to editor objects. You start with a budget, every object you
place deducts its price, and placement is blocked once you run out of money.

## What it does

- **HUD label** in the top-left of the editor showing your current `$` balance.
- **Price table** (`getPriceTable()` in `src/main.cpp`) mapping object IDs to
  prices — a few common blocks/spikes/orbs are seeded as examples.
- **Default price** for any object not explicitly listed, configurable in
  mod settings.
- **Starting money**, configurable in mod settings.
- Optional **persistence**: keep leftover money across editor sessions
  instead of resetting every time you open the editor.
- Placement is refused (with a "Not enough money!" notification) if you
  can't afford the object.

## Requirements

- [Geode CLI / SDK](https://docs.geode-sdk.org/getting-started/) installed
  and set up (this gives you the `GEODE_SDK` environment variable and the
  `geode` command).
- CMake 3.21+ and a C++ toolchain matching your platform (matches whatever
  Geode itself requires).

## Building

From inside this folder:

```bash
geode build
```

This produces a `.geode` file you can install with:

```bash
geode install-mod build/MoneyMod.geode
```

or just drag the `.geode` file into your Geode mods folder / use the in-game
mod loader's "install from file" option.

## Customizing prices

Open `src/main.cpp` and edit the `getPriceTable()` map:

```cpp
static const std::unordered_map<int, int> prices = {
    {1, 1},     // object ID -> price
    {8, 5},
    // add more here
};
```

Object IDs can be found via the Geode object ID overlay mods, or GD's own
editor object search (the ID is shown when you hover/search for an object).

Anything not in the table uses the **Default Object Price** setting instead
of needing to be listed individually.

## Mod settings (editable in-game via the mod's settings page)

| Setting | Default | Description |
|---|---|---|
| Starting Money | 1000 | Balance when entering the editor |
| Default Object Price | 1 | Price for unlisted object IDs |
| Persist Money Between Sessions | off | Carry leftover money into the next session |

## Notes / possible extensions

- Refunds on delete/undo aren't implemented — currently deleting a placed
  object does not give money back. This is a natural next hook if you want
  it (look at `EditorUI`'s delete/undo handlers).
- The `createObject(int, CCPoint, bool)` signature matches current Geode
  bindings as of GD 2.206. If your Geode/bindings version differs and the
  project fails to compile on that line, check the actual signature Geode
  generates for `EditorUI::createObject` (autocomplete in your IDE, or the
  bindings reference) and adjust accordingly.
