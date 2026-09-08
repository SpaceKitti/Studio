# Studio — Slice 1 (Godot 4)

**Owner:** Forge (systems) · narrative Quill · mood Prism · QA Rivet  
**Status:** Playable Godot 4.7 3D slice (2026-09-08)  
**Branch:** `slice-1-godot`  
**Engine lock:** Godot 4 is the real home. Slice 0 HTML stub is throwaway reference only.

---

## How to run

Godot exe (TrinityOrb / WinGet install):

```
C:\Users\Akitt\AppData\Local\Microsoft\WinGet\Packages\GodotEngine.GodotEngine_Microsoft.Winget.Source_8wekyb3d8bbwe\Godot_v4.7.2-stable_win64_console.exe
```

Open the project folder `slice1/` in Godot, or from a shell:

```powershell
& 'C:\Users\Akitt\AppData\Local\Microsoft\WinGet\Packages\GodotEngine.GodotEngine_Microsoft.Winget.Source_8wekyb3d8bbwe\Godot_v4.7.2-stable_win64_console.exe' --path 'C:\Users\Akitt\Games\Studio\slice1'
```

Controls: **WASD** move · **Space** jump · **Mouse** look · **E** interact · **Esc** release/capture mouse.

---

## Layout (locked product space)

1. **Player apartment interior** — lived-in room (warm concrete / terracotta placeholders)
2. **Engineering desk INSIDE the apartment** — not on the balcony
3. **Door/opening onto player balcony** with herb pots
4. **Jump / gap** to **neighbor balcony** with potato/tomato dressing
5. **Ember (cat)** at neighbor glass — free/adopt interact

No monsters. No street roam. Daylight hopeful scrappy cyberpunk per `docs/art/ART_MOOD_BRIEF.md`.

---

## What works

| Feature | Notes |
|---------|--------|
| Apartment interior | Walkable room, door opening to balcony |
| Engineering desk | Indoors; E → toast stub (no craft tree) |
| Player balcony | Herbs interact |
| Gap jump | ~2.2 m between balconies; Space jump |
| Neighbor balcony | Garden inspect |
| Ember | At glass; E → free/adopt (moves onto balcony) |
| HUD | Help strip, prompt, toast |
| Placeholders | CSGBox3D / MeshInstance3D primitives |

Path to play: spawn at desk → walk through door → balcony → jump gap → free Ember.

---

## Out of scope (Slice 1)

- Monsters / infected / street roam
- Full craft tree / inventory / kg UI
- Neighbor full interior
- Fire-escape travel / furniture bridges
- Real art meshes / audio
- Water failure systems
- Expanding Slice 0 browser stub

---

## Files

```
slice1/project.godot
slice1/scenes/main.tscn
slice1/scripts/player.gd
slice1/scripts/level.gd
slice1/scripts/interactable.gd
slice1/scripts/ember.gd
slice1/scripts/hud.gd
docs/SLICE1.md
```

---

*Forge — Slice 1 Godot playable for Rivet / Akitti.*
