# Studio

Leftover-usage game project for Akitti.

## Run Slice 1 — Godot 4 (current)

**Engine home = Godot 4.** Open `slice1/` in Godot 4.7 (Forward Plus).

Godot exe (this machine / WinGet):

```
C:\Users\Akitt\AppData\Local\Microsoft\WinGet\Packages\GodotEngine.GodotEngine_Microsoft.Winget.Source_8wekyb3d8bbwe\Godot_v4.7.2-stable_win64_console.exe
```

```powershell
& 'C:\Users\Akitt\AppData\Local\Microsoft\WinGet\Packages\GodotEngine.GodotEngine_Microsoft.Winget.Source_8wekyb3d8bbwe\Godot_v4.7.2-stable_win64_console.exe' --path 'C:\Users\Akitt\Games\Studio\slice1'
```

Or: Godot → Import / Open → select `slice1/project.godot` → Play (F5).

**WASD** move · **Space** jump · **Mouse** look · **E** interact · **Esc** mouse.

Layout: apartment interior (**desk indoors**) → balcony herbs → jump gap → neighbor garden → free Ember at glass. Details: [`docs/SLICE1.md`](docs/SLICE1.md).

**Separate from Hive MHD.** Do not put MHD / Navier-Stokes / Hive solver work here. That lives in `SpaceKitti/Grok`.

---

## Run Slice 0 (throwaway browser stub)

1. Open `slice0/index.html` in a browser (double-click on Windows works).
2. No install, no server, no build.
3. **WASD / Arrows** move · **Space** jump · **E** interact.

Reference only — do not expand. Prefer Slice 1 Godot. Details: [`docs/SLICE0.md`](docs/SLICE0.md).

## Team

| Role | Who |
|------|-----|
| Coding lead | Forge |
| Coding helper / QA | Rivet |
| Storyline | Quill |
| Art / UI / visuals | Prism |
| Coordinator | Orion |

## Status

Slice 1 Godot 4.7 playable on branch `slice-1-godot`. Slice 0 HTML stub retained as reference. Pitch + mood locked in docs; systems owned by Forge.

## Canon docs

- [`docs/narrative/NARRATIVE_BRIEF.md`](docs/narrative/NARRATIVE_BRIEF.md)
- [`docs/art/ART_MOOD_BRIEF.md`](docs/art/ART_MOOD_BRIEF.md)
- [`docs/SLICE1.md`](docs/SLICE1.md) — Godot slice what works / how to run
- [`docs/SLICE0.md`](docs/SLICE0.md) — browser stub (reference)

## Engine

**Godot 4** is the locked engine home. Slice 0 was a flat HTML5 Canvas stub only.
