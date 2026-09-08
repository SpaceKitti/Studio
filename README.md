# Studio — Fire Escape

Leftover-usage game project for Akitti. Canon design: [`docs/GAME_DESIGN.md`](docs/GAME_DESIGN.md).

## Run Milestone 0 — Godot 4.x 3D (current)

**Engine = Godot 4 only.** Open `slice1/` in Godot 4.7 (Forward Plus).

Godot exe (TrinityOrb / WinGet):

```
C:\Users\Akitt\AppData\Local\Microsoft\WinGet\Packages\GodotEngine.GodotEngine_Microsoft.Winget.Source_8wekyb3d8bbwe\Godot_v4.7.2-stable_win64_console.exe
```

```powershell
& 'C:\Users\Akitt\AppData\Local\Microsoft\WinGet\Packages\GodotEngine.GodotEngine_Microsoft.Winget.Source_8wekyb3d8bbwe\Godot_v4.7.2-stable_win64_console.exe' --path 'C:\Users\Akitt\Games\Studio\slice1'
```

Or: Godot → Import / Open → `slice1/project.godot` → Play (**F5**).

**WASD** move · **Space** jump · **Mouse** look · **E** interact · **Tab** inventory (kg) · **G** Ember gift · **Esc** mouse.

**M0 layout:** home balcony (outdoor desk, herbs, sliding glass + Ember) → jump gap → neighbor (garden + 3 containers). Interior blocked. Details: [`docs/MILESTONE_0.md`](docs/MILESTONE_0.md).

**Win:** walk both balconies, adopt cat, pick up 3 items, plant/water/harvest 1 crop, open inventory showing kg.

**Separate from Hive MHD.** Do not put MHD / Navier-Stokes / Hive solver work here. That lives in `SpaceKitti/Grok`.

---

## Run Slice 0 (throwaway browser stub)

1. Open `slice0/index.html` in a browser.
2. Reference only — do not expand. Prefer Milestone 0 Godot.

## Team

| Role | Who |
|------|-----|
| Coding lead | Forge |
| Coding helper / QA | Rivet |
| Storyline | Quill |
| Art / UI / visuals | Prism |
| Coordinator | Orion |

## Status

Milestone 0 Godot 4.7 playable on branch `milestone-0`. Slice 0 HTML stub retained as reference. Design locked in `docs/GAME_DESIGN.md`.

## Canon docs

- [`docs/GAME_DESIGN.md`](docs/GAME_DESIGN.md) — **source of truth**
- [`docs/MILESTONE_0.md`](docs/MILESTONE_0.md) — what ships / how to run
- [`docs/narrative/NARRATIVE_BRIEF.md`](docs/narrative/NARRATIVE_BRIEF.md)
- [`docs/art/ART_MOOD_BRIEF.md`](docs/art/ART_MOOD_BRIEF.md)

## Engine

**Godot 4** is the locked engine home. No Unreal, no 2D primary, no HTML expansion.
