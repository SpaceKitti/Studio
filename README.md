# Studio — Fire Escape

Leftover-usage game project for Akitti. Canon design: [`docs/GAME_DESIGN.md`](docs/GAME_DESIGN.md).

## Run Milestone 0 — Unreal Engine 5.8 (current)

**Engine = Unreal 5.8.** Project lives in `unreal/`. Godot `slice1/` is the systems reference only — do not keep working there.

Details: [`docs/MILESTONE_0_UNREAL.md`](docs/MILESTONE_0_UNREAL.md).

```powershell
.\unreal\RunM0.ps1
```

Or open `unreal/FireEscape.uproject` in UE 5.8 → Play.

**WASD** move · **Space** jump · **Mouse** look · **E** interact · **Tab** inventory (kg) · **G** Ember gift · **H** stow · **Esc** mouse.

**M0 layout:** home balcony (outdoor desk, herbs, sliding glass — no Ember, no interior) → jump gap → neighbor (garden + 3 containers + Ember behind glass). City is backdrop only.

**Win:** walk both balconies, adopt cat, pick up 3 items, plant/water/harvest 1 crop, open inventory showing kg.

**Separate from Hive MHD.** Do not put MHD / Navier-Stokes / Hive solver work here. That lives in `SpaceKitti/Grok`.

---

## Run Slice 0 / slice1 (reference only)

1. `slice0/index.html` — throwaway browser stub. Do not expand.
2. `slice1/` — Godot 4.7 systems reference for M0. Do not keep developing here.

## Team

| Role | Who |
|------|-----|
| Coding lead | Forge |
| Coding helper / QA | Rivet |
| Storyline | Quill |
| Art / UI / visuals | Prism |
| Coordinator | Orion |

## Status

Milestone 0 Unreal 5.8 playable under `unreal/`. Godot `slice1/` kept as systems reference. Slice 0 HTML stub retained as reference. Design locked in `docs/GAME_DESIGN.md`.

## Canon docs

- [`docs/GAME_DESIGN.md`](docs/GAME_DESIGN.md) — **source of truth**
- [`docs/MILESTONE_0.md`](docs/MILESTONE_0.md) — M0 scope (Godot-era run notes)
- [`docs/MILESTONE_0_UNREAL.md`](docs/MILESTONE_0_UNREAL.md) — Unreal 5.8 how to run
- [`docs/narrative/NARRATIVE_BRIEF.md`](docs/narrative/NARRATIVE_BRIEF.md)
- [`docs/art/ART_MOOD_BRIEF.md`](docs/art/ART_MOOD_BRIEF.md)

## Engine

**Unreal Engine 5.8** is the current M0 home. Godot `slice1/` is scaffolding. No 2D primary, no HTML expansion, no Hive.
