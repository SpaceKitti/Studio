# Fire Escape — item icons

**Split by Prism from Akitti sheets (2026-09-08).**

## Grocery / garden (sheet 1)
| Item id | File |
|---------|------|
| `tuna_can` | `tuna_can_256.png` |
| `flour_sr` | `flour_sr_256.png` |
| `salt` | `salt_256.png` |
| `water_bottle` | `water_bottle_256.png` |
| `tomato_fresh` | `tomato_fresh_256.png` |
| `potato` | `potato_256.png` |
| `tomato_seed` | `tomato_seed_256.png` |
| `potato_seed` | `potato_seed_256.png` |

## Salvage & solar (sheet 2)
| Item id | File | Sheet label | Notes |
|---------|------|-------------|-------|
| `solar_panel_shard` | `solar_panel_shard_256.png` | CRACKED SOLAR CELL | GAME_DESIGN solar recipe piece |
| `panel_frame` | `panel_frame_256.png` | PANEL FRAME | |
| `copper_wire` | `copper_wire_256.png` | COPPER WIRE | |
| `battery_cell` | `battery_cell_256.png` | BATTERY CELL | GAME_DESIGN solar recipe piece |
| `wooden_board` | `wooden_board_256.png` | WOODEN BOARD | furniture-bridge later |
| `nails` | `nails_256.png` | NAILS | |
| `screws` | `screws_256.png` | SCREWS | |
| `duct_tape` | `duct_tape_256.png` | DUCT TAPE | |
| `scrap_metal` | `scrap_metal_256.png` | SCRAP METAL | |
| `glass_shard` | `glass_shard_256.png` | GLASS SHARD | |
| `salvaged_pcb` | `salvaged_pcb_256.png` | SALVAGED PCB | |
| `electrical_tape` | `electrical_tape_256.png` | ELECTRICAL TAPE | |

Sheets: `m0_item_icons_sheet.jpg`, `m0_salvage_solar_sheet.jpg`

## Still missing icons
`herb_basil`, `herb_mint` — text/color OK for now.

## Godot
`Item.id` → `res://assets/icons/<id>_256.png` on dark Panel Face.


## Tools + watering (sheet 3 + cards, 2026-09-10)
| Item id | Use |
|---------|-----|
| `hammer` | Break furniture → boards + nails |
| `crowbar` | Faster salvage, pry containers |
| `hand_saw` | Cut boards / furniture bridges |
| `hacksaw` | Cut scrap metal for solar frame |
| `screwdriver_set` | Metal frame + screws |
| `soldering_kit` | Finish copper-wire builds (solar, lamp) |
| `wire_cutters` | Salvage wire (w/ pliers) |
| `pliers` | Salvage wire (w/ wire_cutters) |
| `adjustable_wrench` | Later pump / rail / hardware |
| `utility_knife` | Open cans, cut tape, harvest twine |
| `tape_measure` | Flavor / bridge precision bonus |
| `file` | Clean rusted parts |
| `watering_can` | Garden watering |
| `bucket` | Fill/carry water — **icon pending** (resend if needed) |

Sheet: `m0_tools_sheet.jpg`. Tool-use doc: `docs/art/TOOL_USE.md`.

## Herbs (2026-09-11)
| id | notes |
|----|-------|
| `herb_basil` / `herb_basil_seed` | terracotta pot art |
| `herb_oregano` / `herb_oregano_seed` | |
| `herb_mint` / `herb_mint_seed` | |
Balcony: raised beds for potato/tomato; herbs can stay terracotta pots.
