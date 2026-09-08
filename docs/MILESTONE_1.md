# Milestone 1 — Apartment interiors (home + neighbor)

Source: [`GAME_DESIGN.md`](GAME_DESIGN.md), art brief [`art/INTERIOR_ROOM_BRIEF.md`](art/INTERIOR_ROOM_BRIEF.md).  
Builds on Milestone 0 balcony loop under `slice1/`.

## What ships (this pass)

| Space | Contents |
|-------|----------|
| **Home glass** | Opens into **home apartment** (was blocked black volume) |
| **Neighbor glass** | Still frees **Ember** first; after open, enter neighbor apt. Glass = Balcony Glass alpha (see-through) |
| **Apartments ×2** | Same layout copy-paste: living · kitchen · bedroom · bath (CSG/placeholder OK) |
| **Living** | **Lamp Pocket** warm island; rest space (craft desk stays **outdoor** balcony stub) |
| **Kitchen** | Cooler light; cupboard loot; **kitchen sink** fill bottle; empty bottles / pot set dressing |
| **Bedroom** | Quill crumbs: open **suitcase** + **dual cup**; quieter Lamp Pocket |
| **Bath** | Sink + bathtub: fill bottle / plug / rising water (running water ON) |
| **Tools** | Hands holding `water_bottle` / `watering_can` can **water** plants (prompt says use tool, not only blocked). Stow + kg rules unchanged |

## Out of scope (reject)

Streets, infected/enemies, cat art pass, Unreal, full craft unlock, ACE music integration.

## How to run

```powershell
& 'C:\Users\Akitt\AppData\Local\Microsoft\WinGet\Packages\GodotEngine.GodotEngine_Microsoft.Winget.Source_8wekyb3d8bbwe\Godot_v4.7.2-stable_win64.exe' --path 'C:\Users\Akitt\Games\Studio\slice1'
```

Do **not** auto-launch from agents unless asked. Press **T** for inventory.
