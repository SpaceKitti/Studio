# Milestone 0 — Unreal Engine 5.8 (current playable)

Source of truth for *what* M0 is: [`GAME_DESIGN.md`](GAME_DESIGN.md).
Godot `slice1/` is the **systems reference only**. Do not keep working in Godot.

Playable Unreal 5.8 C++ slice under `unreal/`.

## What ships

| Space | Contents |
|-------|----------|
| **Home balcony** | Outdoor engineering table (craft locked), 3 herb pots, 1 empty plantable pot, own sliding glass (blocked black volume — no interior, no Ember) |
| **Neighbor balcony** | Potato + tomato mid-growth pots, planter box / plastic drawer / rusted toolbox, note + empty bowl, neighbor sliding glass with **Ember** behind it (**E** opens → adopt HERE) |
| **Traversal** | Jumpable gap (~2.2 m). Fire escape visible, interact says locked. City backdrop only — no streets |
| **Systems** | Look-at + **E**; items with `weight_kg`; backpack **25 kg**; `hands_occupied`; garden plant/water/harvest; `solar_rig` recipe data with craft locked; Ember follows loosely; **G** debug gift drop |
| **Art** | Neon dusk / sick-amber key + cyan/magenta neon trim + SSAO/bloom; FOV **65**; palette from [`art/PALETTE.md`](art/PALETTE.md) |

### Win condition checklist

Player can:

1. Walk **both** balconies (jump the gap)
2. Free / adopt the cat (open **neighbor** sliding glass)
3. Pick up **3** items (search containers / harvest)
4. **Plant → water → harvest** path for 1 crop
5. Open **inventory** (**Tab** / **I**) showing **kg**

On-screen M0 checklist updates live.

## How to run (Windows)

Unreal Engine **5.8** at:

```
C:\Program Files\Epic Games\UE_5.8
```

Needs **VS 2022 Build Tools** with the C++ workload (MSVC + Windows SDK).

From repo root:

```powershell
.\unreal\RunM0.ps1
```

Or compile then PIE:

```powershell
& 'C:\Program Files\Epic Games\UE_5.8\Engine\Build\BatchFiles\Build.bat' FireEscapeEditor Win64 Development -Project='C:\Users\Akitt\Games\Studio\unreal\FireEscape.uproject' -WaitMutex
& 'C:\Program Files\Epic Games\UE_5.8\Engine\Binaries\Win64\UnrealEditor.exe' 'C:\Users\Akitt\Games\Studio\unreal\FireEscape.uproject' /Engine/Maps/Templates/Template_Default -game -windowed
```

Editor: open `unreal/FireEscape.uproject` → Play.

### Controls

| Key | Action |
|-----|--------|
| WASD | Move |
| Space | Jump |
| Mouse | Look |
| E | Interact |
| Tab / I / T | Inventory (kg) |
| G | Debug: Ember drop-gift |
| H | Stow hands → backpack |
| Esc | Release / capture mouse |

### Suggested M0 play path

1. Home balcony: herbs / desk / empty pot; own glass has no Ember.
2. Jump east gap to neighbor; **E** neighbor glass → adopt Ember.
3. Search **planter box** (seeds), **plastic drawer** (water/tuna/flour), **rusted toolbox** (salt/water) — get ≥3 pickups.
4. Jump back; plant seed in empty home pot → water with bottle → harvest.
5. **Tab** inventory — confirm kg readout.
6. Optional: **E** engineering table — see `solar_rig` recipe, craft greyed. **G** Ember gift.

## Explicitly out of scope (reject)

Streets, interiors, infected/enemies, hardware store, solar craft loop, open world, second building, usable fire-escape travel.

## Project layout

```
unreal/
  FireEscape.uproject
  Source/FireEscape/   # player, level builder, inventory, plants, Ember, HUD
  Config/
  Content/Icons/       # M0 item PNGs (runtime)
```

Game instance subsystem: `UFEGameSubsystem` (ItemDB + inventory + win flags).

## Next

See [`MILESTONE_1.md`](MILESTONE_1.md) for apartment interiors + water fixtures.
