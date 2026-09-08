# Fire Escape — palette notes (Akitti → Prism, 2026-09-08)

Source: Akitti’s dusk + neon sheets; Prism rain-night + UI/kg sheets. Canon for lighting/materials/UI until revised.
Sheets: `palette-core-atmosphere.jpg`, `palette-neon-signage.jpg`, `palette-rain-night.jpg`, `palette-ui-inventory.jpg`
Godot: `docs/art/palette.gd` and `slice1/scripts/palette.gd` (`FireEscapePalette`)

## Intent
Balcony-survival cyberpunk: dusk city, dirty plaster, sick neons — **not** full black-and-magenta cliché.
Walls stay Wet Asphalt / Stucco (or rain-night equivalents). Neon is **trim only**. One hue family per surface (magenta *or* cyan at full strength, not both).

**Night / home-base:** rain-night set is for stuck-indoors / balcony-at-night beats. Keep a **Lamp Pocket** warm island so home still feels yours.

---

## Core atmosphere (dusk / M0 day-dusk)
| Role | Name | Hex | Use |
|------|------|-----|-----|
| Night void | Night Slate | `#0B1020` | Sky far below, unlit interiors |
| Building mass | Wet Asphalt | `#1A1F2E` | Walls in shadow, metal frames |
| Dirty city | Concrete Dust | `#8A8490` | Rails, floor slabs, old paint |
| Lived-in | Sun-Bleached Stucco | `#C4B8A8` | Sun-hit balcony plaster |
| Warm key | Sick Amber | `#E8A84A` | Sunset bounce, desk lamp |
| Warm neon | Sodium Dusk | `#FF6B2C` | Street lamps, window glow |
| Signage | Magenta Sign | `#FF2D95` | Distant billboard (sparingly) |
| Tech / solar | Cyan Rig | `#2DE2E6` | Rail rim / solar (sparingly) |
| Garden / toxin | Toxic Lime | `#B6FF3B` | New growth only |
| Planters, copper | Oxidized Teal | `#1F6F6A` | Older pots, oxidized metal |
| Rust, old blood | Dried Blood Rust | `#7A2E2E` | Rust, weathered stains |
| Cat, paper, skin | Cat Cream | `#F3E6D0` | Ember, paper, soft highlights |

### M0 dusk lighting
1. **Key:** Sick Amber from the side (sun)
2. **Fill:** Wet Asphalt — dark but readable
3. **Rim:** Cyan Rig on metal rail *or* Magenta Sign on distant billboard — not both at full strength
4. **Plants:** Toxic Lime on new growth; older pots = Oxidized Teal

---

## Primary neon accents (signs / UI / emissives, not walls)
| Role | Name | Hex |
|------|------|-----|
| Main sign | Hot Magenta | `#FF1090` |
| Soft sign | Hot Pink | `#FF4DD2` |
| Night club | Electric Violet | `#B026FF` |
| Deep neon | Laser Purple | `#7A00FF` |
| Tech / solar | Cyan Shock | `#00F5FF` |
| Glass / hologram | Ice Blue | `#7DF9FF` |
| Fresh growth | Acid Lime | `#D6FF00` |
| Power good | Radioactive Green | `#39FF14` |
| UI warning | Warning Amber | `#FFD100` |
| Sodium / heat | Molten Orange | `#FF5A00` |
| Danger / infected later | Blood Neon | `#FF003C` |
| Tube core | White Hot | `#F8FBFF` |

**Emission cheat:** albedo ≈ 30% of hex, emission energy 3–8 on same hex. Tube = neon + thin White Hot core.

---

## Rain night (home base / stuck indoors)
Colder, wetter, more cyan — for night cycles and “we’re stuck here till morning.”

| Role | Name | Hex | Use |
|------|------|-----|-----|
| Night void | Night Void | `#050810` | Sky, deep unlit |
| Building mass | Flooded Asphalt | `#121820` | Wet walls in shadow |
| Dirty city | Wet Concrete | `#5A6570` | Rails, slabs in rain |
| Lived-in cold | Cold Stucco | `#9AA3A8` | Rain-hit plaster |
| Soft fill | Moon Fill | `#7A9BB0` | Overcast / moon bounce |
| Key wet neon | Cyan Soak | `#1AC8D4` | Rain reflections, primary rim |
| Window | Window Glow | `#4DE8FF` | Lit windows across the street |
| Distant sign | Magenta Leak | `#C2186A` | Far signage, keep dim |
| Standing water / moss | Pool Green | `#2D5A4A` | Puddle sheen, damp plants |
| Decay | Mold Teal | `#0F3D3A` | Corners, planter undersides |
| Rust wet | Wet Rust | `#5A2428` | Dark wet rust |
| Sanctuary | Lamp Pocket | `#E8D4B8` | Desk lamp / home warm island |

### Rain-night lighting
1. **Key:** Cyan Soak (soft) + optional Window Glow across the gap
2. **Fill:** Flooded Asphalt / Moon Fill — keep readable, not black crush
3. **Warm island:** Lamp Pocket at outdoor desk / later interior — home still yours
4. **Rim:** Magenta Leak only as distant bleed, never competing with Cyan Soak
5. Ember: Cat Cream still, or slightly cooler — never lose warmth/agency

---

## UI / kg inventory
Handheld / notebook HUD — daylight-readable and night-readable.

| Role | Name | Hex | Use |
|------|------|-----|-----|
| Backdrop | Panel Void | `#0E1420` | Modal / inventory backdrop |
| Panel | Panel Face | `#1A2233` | Panel body |
| Border | Panel Edge | `#2A3548` | Edges, dividers |
| Text | Text Primary | `#E8EEF5` | Labels, kg numbers |
| Secondary | Text Muted | `#8A96A8` | Hints, tags |
| Weight OK | Kg OK | `#39FF14` | Under cap |
| Weight warn | Kg Warn | `#FFD100` | Near 25 kg |
| Weight fail | Kg Over | `#FF003C` | Reject / over |
| Hands occupied | Hands Busy | `#C45C6A` | Hands-busy state |
| Empty slot | Slot Empty | `#243040` | Empty bag slots |
| Focus | Focus Cyan | `#00F5FF` | Selected / hover |
| Highlight core | Tube Core | `#F8FBFF` | Cursor / active pip |

### UI rules
- Kg bar: OK → Warn → Over using those three only
- Don’t use Blood Neon for everyday UI until infected milestone
- Focus Cyan for selection; Hands Busy for blocked actions

---

## Prism expansions (props)
| Role | Name | Hex | Why |
|------|------|-----|-----|
| Herb leaf (non-neon) | Herb Sap | `#4A7A3C` | Basil/mint without Toxic Lime overload |
| Soil | Potting Soil | `#3B2A1E` | Planter dirt vs asphalt black |
| Glass cool | Balcony Glass | `#A8C4D4` | Sliding door daytime read |
| Hands-busy soft | Occupied Rose | `#C45C6A` | Same as Hands Busy |

## Aligns with
`docs/GAME_DESIGN.md` art direction. Older ART_MOOD_BRIEF daylight-first notes: **M0 dusk + this palette win**; Ember = Cat Cream warmth/agency.
