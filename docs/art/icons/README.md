# Fire Escape — M0 item icons

**Source:** Akitti sheet `FIRE ESCAPE — MO ITEM ICONS` (2026-09-08)  
**Split by:** Prism (no need for Akitti to separate)

## Files
| Item id | File | Label on sheet |
|---------|------|----------------|
| `tuna_can` | `tuna_can.png` / `_256.png` | TUNA CAN |
| `flour_sr` | `flour_sr.png` / `_256.png` | SELF-RAISING FLOUR |
| `salt` | `salt.png` / `_256.png` | SALT |
| `water_bottle` | `water_bottle.png` / `_256.png` | WATER BOTTLES |
| `tomato_fresh` | `tomato_fresh.png` / `_256.png` | TOMATO |
| `potato` | `potato.png` / `_256.png` | POTATO |
| `tomato_seed` | `tomato_seed.png` / `_256.png` | TOMATO SEEDS |
| `potato_seed` | `potato_seed.png` / `_256.png` | POTATO SEEDS |

Also: `m0_item_icons_sheet.jpg` (full sheet), `*_cell.png` (framed cell with label — reference only).

## Use
- Inventory slots: prefer `*_256.png` on dark Panel Face (`#1A2233`)
- Style is photoreal neon-rim — fine for M0 placeholders; stylize later if art direction shifts
- Missing from sheet (still text/color OK): `herb_basil`, `herb_mint`

## Godot hint
Map `Item.id` → `res://assets/icons/<id>_256.png` when HUD inventory is opaque.
