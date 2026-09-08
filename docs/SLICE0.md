# Studio — Slice 0

**Owner:** Forge (systems) · narrative Quill · mood Prism · QA Rivet  
**Status:** Playable HTML5 Canvas stub (2026-09-08)  
**Branch:** `slice-0-playable`

---

## How to run

Open `slice0/index.html` in a browser (double-click on Windows is fine). No build step, no server required.

Controls: **WASD / Arrows** move · **Space** jump · **E** interact.

---

## What works

| Feature | Notes |
|---------|--------|
| Player balcony | Warm concrete platform, railing, herb pots, desk stub |
| Neighbor balcony | Jumpable gap; potato/tomato half-garden; glass door |
| Move / jump | Between the two balconies only |
| Herbs | Interact → short status message |
| Potato / tomato bed | Interact → inspect half-started garden |
| Cat at glass | Interact → free/adopt (placeholder; cat moves onto balcony) |
| Engineering desk | Interact → tiny stub message (no craft tree) |
| On-screen help | Controls strip + HUD status / toast |
| Daylight mood | Warm concrete, terracotta, herb greens, soft sky haze; distant neon bleed only |

Narrative hooks covered as stubs: H1 (glass/cat), H2 (two balconies), H4 (desk stub). Art placeholders follow `docs/art/ART_MOOD_BRIEF.md`.

---

## Out of scope (Slice 0)

- Monsters / infected
- Street roam / open world
- Full neighbor interior
- Craft tree / inventory / kg weight UI
- Fire-escape travel (visual hint only)
- Cat pigeon loot beat
- Water failure systems
- Furniture-bridge traversal
- Real art assets / Blender meshes
- Audio

See `docs/narrative/NARRATIVE_BRIEF.md` for locked pitch and later hooks (H0, H3, H5, H6).

---

## Files

```
slice0/index.html
slice0/css/style.css
slice0/js/game.js
docs/SLICE0.md
```

---

*Forge — Slice 0 playable stub for Rivet to open locally.*
