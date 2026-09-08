# Fire Escape — palette notes (Akitti → Prism, 2026-09-08)

Source: Akitti’s dusk + neon sheets (attached). Canon for M0 lighting/materials until revised.
Repo mirror: `docs/art/PALETTE.md` + `docs/art/palette-*.jpg` + `slice1/scripts/palette.gd` (or shared).

## Intent
Balcony-survival cyberpunk: dusk city, dirty plaster, sick neons — **not** full black-and-magenta cliché.
Walls stay Wet Asphalt / Stucco. Neon is **trim only**. One hue family per surface (magenta *or* cyan at full strength, not both).

## Core atmosphere (12)
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

## Primary neon accents (12) — signs / UI / emissives, not walls
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

## M0 lighting recipe
1. **Key:** Sick Amber from the side (sun)
2. **Fill:** Wet Asphalt — dark but readable
3. **Rim:** Cyan Rig on metal rail *or* Magenta Sign on distant billboard — not both at full strength
4. **Plants:** Toxic Lime on new growth; older pots = Oxidized Teal

## Emission cheat (Godot)
Albedo ≈ 30% of hex, emission energy 3–8 on same hex → reads as a sign, not a flat sticker.
Tube look = neon color + thin White Hot core.

## Pairing rules
- Garden LEDs: Acid Lime
- Battery charged: Cyan Shock
- Battery dead: Blood Neon
- Inventory/kg UI later: Warning Amber + Ice Blue + White Hot (UI sheet TBD if needed)

## Optional next sheets (Akitti offered)
- Rain night (colder, more cyan)
- UI-only inventory/kg bars

## Prism expansions (locked with Akitti’s set)
| Role | Name | Hex | Why |
|------|------|-----|-----|
| Herb leaf (non-neon) | Herb Sap | `#4A7A3C` | Basil/mint without Toxic Lime overload |
| Soil | Potting Soil | `#3B2A1E` | Planter dirt vs asphalt black |
| Glass cool | Balcony Glass | `#A8C4D4` | Sliding door daytime read |
| Hands-busy UI | Occupied Rose | `#C45C6A` | Soft “hands full” state, not Blood Neon yet |

## Aligns with
`docs/GAME_DESIGN.md` art direction (golden-hour / neon dusk, not grey noon).
Older `ART_MOOD_BRIEF` / `SLICE1_ASSET_BRIEF` daylight-first notes: **superseded for M0 lighting** by this palette + GAME_DESIGN; keep Ember = Cat Cream warmth/agency.
