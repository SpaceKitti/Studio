# Milestone 0 — Fire Escape (two balconies)

Source of truth: [`GAME_DESIGN.md`](GAME_DESIGN.md). This document records **what ships** and **how to run**.

> **Layout note (Akitti):** Ember is freed at **neighbor** sliding glass. GAME_DESIGN glass-on-home was wrong vs Akitti — cat belongs to the neighbor.

## What ships

Playable Godot 4.x **3D** slice under `slice1/`:

| Space | Contents |
|-------|----------|
| **Home balcony** | Outdoor desk stub (craft locked; moves inside M1), 3 herb pots, 1 empty plantable pot, **blocked** own sliding glass (no Ember), interior = black volume |
| **Neighbor balcony** | Potato + tomato mid-growth pots, 3 containers, note + empty bowl, neighbor sliding glass with **Ember** behind it (**E** opens → adopt HERE) |
| **Traversal** | Jumpable gap (~2 m). Fire escape visible, interact says locked. Skybox/street backdrop only |
| **Systems** | Look-at + **E**; Item resources with `weight_kg`; backpack **25 kg**; `hands_occupied`; garden plant/water/harvest; `solar_rig` recipe data with craft locked; Ember follows loosely; **G** debug gift drop |
| **Art** | Neon dusk / golden-hour key light + cyan/magenta neon trim + SSAO; FOV **65**; stylized cyberpunk materials (not grey noon) |

### Win condition checklist

Player can:

1. Walk **both** balconies (jump the gap)
2. Free / adopt the cat (open **neighbor** sliding glass)
3. Pick up **3** items (search containers / harvest)
4. **Plant → water → harvest** path for 1 crop (empty home pot + seed from planter; or water/harvest neighbor after planting path on home)
5. Open **inventory** (**Tab** / **I**) showing **kg**

On-screen M0 checklist updates live.

## How to run (Windows / TrinityOrb)

Godot 4.7 Forward Plus:

```
C:\Users\Akitt\AppData\Local\Microsoft\WinGet\Packages\GodotEngine.GodotEngine_Microsoft.Winget.Source_8wekyb3d8bbwe\Godot_v4.7.2-stable_win64_console.exe
```

```powershell
& 'C:\Users\Akitt\AppData\Local\Microsoft\WinGet\Packages\GodotEngine.GodotEngine_Microsoft.Winget.Source_8wekyb3d8bbwe\Godot_v4.7.2-stable_win64_console.exe' --path 'C:\Users\Akitt\Games\Studio\slice1'
```

Or: Godot → Open `slice1/project.godot` → **F5**.

### Controls

| Key | Action |
|-----|--------|
| WASD | Move |
| Space | Jump |
| Mouse | Look |
| E | Interact |
| Tab / I | Inventory (kg) |
| G | Debug: Ember drop-gift |
| H | Stow hands → backpack |
| Esc | Release / capture mouse |

### Suggested M0 play path

1. On home balcony: herbs / desk / empty pot; own glass is blocked (no Ember).
2. Jump east gap to neighbor balcony; **E** neighbor glass → adopt Ember.
3. Search **planter box** (seeds), **plastic drawer** (water/tuna/flour), **rusted toolbox** (salt/water) — get ≥3 pickups.
4. Jump back; plant seed in empty home pot → water with bottle → harvest.
5. **Tab** inventory — confirm kg readout.
6. Optional: **E** engineering table — see `solar_rig` recipe, craft greyed. **G** Ember gift.

## Explicitly out of scope (reject)

Streets, infected/enemies, full apartment interior, hardware store, Unreal, 2D, HTML, main-menu polish, second building, usable fire-escape open world, finished solar craft loop.

## Project layout

```
slice1/
  project.godot
  scenes/main.tscn
  scripts/   # player, level, inventory, item, plant_spot, container, ember, hud, …
```

Autoloads: `ItemDB`, `Inventory`, `GameState`.
