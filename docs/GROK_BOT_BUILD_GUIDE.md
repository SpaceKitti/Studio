# Fire Escape — Grok Bot Build Guide

**Audience:** Forge / Rivet / Prism / Quill (and any Grok Build run)  
**Status:** Living process doc (2026-09-11). Matches Akitti’s rebuild rules.  
**Engine:** Unreal map at `C:\Users\Akitt\Games\Studio\unreal` (`M0_FireEscape` for now).

---

## How to treat the current map

The current Unreal map is a **test / learning map** for Akitti and the bots together — not a forever “ship this exact spaghetti.”

- Expect messy furniture overlaps, shared-wall confusion, and soft layout mistakes.
- Prefer **small, calm fixes** and one ask at a time over rewriting the whole apt.
- When we rebuild for real, follow **floor sectioning** below. Do not invent a different room graph.

---

## Role lanes (do not blur)

| Bot | Owns | Does not own |
|-----|------|----------------|
| **Forge** | Layout only — big floor slab first, then divide; floors before walls; match image/BP | Decor, story, inventory/tools |
| **Rivet** | Mechanics — inventory, item use, tools, interacts, plant→seed | Inventing layout or art |
| **Prism** | Decor/art after rooms exist — materials, props, posters, placement briefs | Inventing room graph |
| **Quill** | Writing / story docs; optional music prompt cards; remedial checklists when asked | Layout, decor placement, mechanics code |

**One ask at a time.** No “do the whole apartment” handoffs.

---

## Why floors are labeled (not walls)

**Do not rely on labeling walls to know which room is which.**

Right now (and until bots are smarter about shared geometry):

- Walls are often **shared** between rooms (party wall, BR–LV, etc.).
- A wall label is ambiguous — two rooms own the same mesh.
- Bots that “fix” by tagging walls still get confused and invent halls.

**Future rule (locked intent):**

1. **Connected floor plates** carry the room identity: `BR_Floor`, `LV_Floor`, …
2. **Furniture / props** use the same prefix: `BR_Bed`, `LV_Sofa`, `KT_Fridge`, …
3. If it sits on `BR_Floor`, it is BR furniture. Do not place `BR_` props on `LV_Floor`.
4. Walls stay mostly unlabeled for room ID; openings are cut from the plate-edge plan instead.

That way any bot that moves or adds something can answer: *which floor am I on?* → *which furniture set is legal?*

---

## Floor sectioning (future rebuilds)

**Build order is fixed. Do not skip ahead.**

1. Floor plates (connected, walkable)  
2. Walls with holes already cut for doors and windows  
3. Door and window meshes in those holes  

**No furniture** in these passes.  
**No halls-as-fixes.**  
**No second building / fake rooftop mass.**

### Pass 0 — ruler

- One outline slab for the whole home unit. Name: `HOME_OUTLINE`.
- Size: **10m wide × 9m deep** (interior only).
- Collision **off**. Leave it in place until floors are done.
- Neighbor ruler later: `NBR_OUTLINE`, same size, against the home party-wall edge.

### Pass 1 — room floors

All plates:

- Same thickness, same Z (outline + a few cm).
- Collision **on**.
- No overlaps. No gaps over **2cm**.
- Each new plate shares a **full edge** with one already placed.

**Home unit** (north = glass / balcony side):

| Name | Size | Where |
|------|------|--------|
| `BR_Floor` | 6 × 4 m | North-west |
| `BA_Floor` | 4 × 4 m | North-east |
| `KT_Floor` | 4 × 5 m | South-east |
| `LV_Floor` | 6 × 5 m | South-west |

That’s one **10 × 9** rectangle.

- `BR` shares an edge with `BA` and with `LV`.
- `BA` shares with `BR` and `KT`.
- `KT` shares with `BA` and `LV`.

**Balconies** — same Z, on the **north (glass) side only**:

| Name | Size |
|------|------|
| `HB_Floor` | 6 × 2.5 m |
| `NB_Floor` | 6 × 2.5 m |

- Jump gap between balcony plates: **2.2m**.
- Apartments **side by side**, not facing across a courtyard.

**Neighbor floors:** duplicate home plates, prefix `N` (e.g. `NBR_Floor`, `NLV_Floor`). Slide so the **9m party edges** touch.

**Hard fails:**

- Do not add a fifth interior plate.
- Do not cut halls into the floor.
- If two plates overlap, **undo the last plate**.

### Pass 2 — walls

- Walls sit on **plate edges**. Shared party wall stays **one** wall.
- Cut openings in the wall meshes as you place them:
  - Interior doors **0.9m** wide: `BR`–`LV`, `BA`–hall/`LV`, `KT`–`LV` (as specified for that rebuild)
  - Sliding glass on the **north** wall of home onto `HB_Floor`
  - Window holes on the **balcony facade** only unless specified
- No extra corridor walls.
- No wall that sits in the **middle** of a floor plate.
- No ceiling windows / rotated wall meshes on roofs.

### Pass 3 — doors and windows

- Fill the holes from Pass 2 only.
- Do not invent new openings.

### Later — furniture / decor (Prism)

- May not move floor plates.
- Prefix props to the floor they sit on (`BR_…` on `BR_Floor`, etc.).
- Living-wall posters OK; keep Rex crumbs in home bedroom only (`BR_`).

---

## Name map

| Prefix | Room |
|--------|------|
| `BR_` | Bedroom |
| `BA_` | Bath |
| `KT_` | Kitchen |
| `LV_` | Living |
| `HB_` | Home balcony |
| `NB_` | Neighbor balcony (home-side naming for the neighbor unit’s balcony plate) |
| `N…` / `NBR_` / `NLV_` … | Neighbor unit duplicates |

Floors first, then walls, then fittings.

---

## Geometry rules (always)

1. Prefer **one big floor rectangle**, then divide — not 4–5 floated tiny rooms with their own floors.
2. If using block/square method: **floors before walls** — never walls-first tiny boxes.
3. Match Akitti’s image / BP when provided. Bot eyes are bad at screenshots — prefer named plates + sizes over “looks fine in Lit.”
4. Party wall must **not** cut the balcony jump gap.
5. No soft HAVE: if the ask isn’t visible in the **editor viewport**, it isn’t done.

---

## Quill / remedial

Quill can clarify asks, write checklists, and unstick docs when Forge/Rivet/Prism are stuck — **without** taking layout, mechanics, or decor jobs unless Akitti says so.

---

*Source: Akitti (2026-09-11) — test-map mindset, floor-named rooms, wall labels unreliable, floor-sectioning rebuild order.*
