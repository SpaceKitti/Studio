# Working title: Fire Escape
# Genre: 3D semi-open vertical survival-craft
# Setting: post-collapse cyberpunk city. Neon still works. Viruses still exist.
# Infected exist later. Not this milestone.
# Engine: Unreal Engine 5.8 (current). Godot 4.x slice1/ is systems reference only.
# Perspective: first-person (FOV 65). Frozen.
# Protagonist: unnamed engineering intern (she). Incomplete knowledge. Can assemble from kits, not invent from first principles.

## Pillars
1. Vertical home, not a forest. The apartment stack IS the world.
2. Make do. Furniture becomes bridges. Drawers become loot tables. The cat becomes a scavenger.
3. Weight is real. kg in bag vs kg in arms.
4. Start tiny. Expand outward only when the current slice is playable.

## Frozen scope — Milestone 0 (do this first)
Playable space:
- Player balcony (home)
- Sliding glass door (interactable)
- Neighbor balcony to ONE side (jump gap, no furniture bridge yet)
- Fire escape visible but NOT usable as open world
- Skybox + street far below as backdrop only

Not in M0:
- Full apartment interior
- Hardware store
- Open streets
- Infected / combat
- Multi-floor building traversal
- Solar array as a finished loop

Win condition for M0:
Player can walk both balconies, free/adopt the cat, pick up 3 items, plant 1 crop, open inventory that shows kg.

## Spaces
### Home balcony
- Small outdoor table = engineering desk (tools already there; crafting UI later)
- 2–3 starter herb pots (alive)
- Sliding glass door = **your apartment** (blocked for M0 — no Ember). Interior is a black/blocked volume.
- Spawn on home; face the gap toward neighbor.

### Neighbor balcony
- Half-started garden: 1 potato pot, 1 tomato pot (mid-growth)
- A few searchable containers: planter box, plastic drawer, rusted toolbox
- Neighbor is gone. Note or empty food bowl explains the cat.
- Sliding glass = **neighbor’s** apartment. **Ember** (neighbor’s cat) paws here. Interact = free/adopt. Neighbor interior off-limits for M0.

### Traversal M0
- Gap between balconies is jumpable.
- Bridge-from-furniture is designed now, built in Milestone 1.

## Characters
### Player
- Engineering intern. Can follow a recipe if she has the parts. Cannot freeform-engineer yet.
- Starts with a worn backpack: 25.0 kg capacity.
- Arms carry: extra weight allowed, but both hands occupied → cannot use desk, harvest, jump-boost tools, or open containers.

### Cat
- Adopted immediately after door interact.
- Can path home balcony ↔ neighbor balcony.
- Later: uses fire escape.
- Loot table (tune later): every 2–4 in-game days, chance to drop a pigeon at player's feet.
- M0: cat exists, follows loosely, has a "drop gift" debug button. Real timer in M1.

## Systems (implement in this order)

### 1. Interaction
Look-at + E:
- Open door
- Pick up item
- Search container
- Harvest / water plant
- Use desk (disabled until M1)

### 2. Inventory (kg, not slots-only)
Each item has:
- id, display_name, weight_kg, stack_max, tags[]

Containers:
- backpack (25.0 kg)
- hands (no kg cap for M0, but hands_occupied bool)
- world piles

Rules:
- If backpack + new item > 25.0 → reject or force hands.
- If hands_occupied → block: crafting, searching, watering, climbing interact.

Starter items (M0 present in world):
| id | name | kg | notes |
|---|---|---|---|
| herb_basil | Basil | 0.2 | already potted |
| herb_mint | Mint | 0.2 | already potted |
| potato_seed | Seed potato | 0.1 | neighbor |
| tomato_seed | Tomato seedling | 0.3 | neighbor |
| water_bottle | Water bottle | 0.5 | drawer |
| tuna_can | Canned tuna | 0.15 | drawer |
| flour_sr | Self-raising flour | 1.0 | drawer |
| salt | Salt | 0.3 | drawer |
| tomato_fresh | Tomato | 0.2 | harvest later |

Grocery set of 5 is locked: tomato, tuna, self-raising flour, salt, water bottles.
Expand later. Do not add 40 foods now.

### 3. Garden (stupid-simple)
Plant states: empty → planted → growing → harvestable → dead
M0 only needs:
- Neighbor tomato + potato already in `growing`
- Player can water (consumes 1 water_bottle or a shared watering can later)
- One harvest action that yields tomato_fresh / potato

No seasons, no soil chemistry yet.

### 4. Crafting (data only in M0, UI in M1)
Desk is present. Recipes exist as data so we don't redesign later.

```
solar_rig:
  pieces: solar_panel_shard x3 + battery_cell x1
  station: engineering_desk
  result: solar_charger
```

Hardware store raid later feeds those pieces.
M0: recipe visible in a text list, craft button greyed out.

### 5. "Open world" is a map, on purpose
Fire escape interact in M1+ opens a node map, not a street level.
Nodes (locked until built):
- Hardware store (parts)
- Corner grocery (the 5 foods)
- Home balcony (return)

Replace map with real vertical level when those loops work.
This is the correct call. Do not build the street yet.

## Milestone ladder
M0 — two balconies, cat, kg inventory, 3 containers, 2 plants
M1 — apartment interior (small), desk crafting UI, furniture-as-bridge prototype, cat gift timer
M2 — fire-escape map: hardware store node, solar recipe craftable
M3 — bag upgrade, arm-carry penalties feel bad in a good way, second neighbor balcony
M4 — infected / panic beat / people fleeing (narrative, not a horde sim)
M5 — decide if the map becomes real geometry or stays a node graph

## Art direction (so it stops looking flat)
- 3D, not 2.5D.
- Golden-hour or sick neon dusk. Not grey noon.
- One key light + one neon trim light + cheap AO.
- Camera slightly low, FOV ~60–70, balcony rail in foreground.
- Kitbash: cheap apartment props > custom hero meshes.
- Stylized cyberpunk, not photoreal. Godot can look good here if materials aren't default grey.

## Prompt rules for any future Grok / bot session
1. Paste this file first.
2. "Implement ONLY Milestone 0. Do not add streets, enemies, main menu polish, or a second building."
3. Ask for: scene tree, item resource format, player controller, interaction ray, inventory kg math.
4. Reject 2D, reject HTML canvas, reject "vertical slice of the whole game."
5. After each session: commit scenes + this doc. Next chat starts from git, not memory.
