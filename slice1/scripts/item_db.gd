extends Node
## Autoload: known item definitions for Milestone 0.

const ItemScript = preload("res://scripts/item.gd")

var _items: Dictionary = {}

func _ready() -> void:
	_register("herb_basil", "Basil", 0.2, 5, ["herb", "planted"])
	_register("herb_mint", "Mint", 0.2, 5, ["herb", "planted"])
	_register("potato_seed", "Seed potato", 0.1, 8, ["seed", "crop"])
	_register("tomato_seed", "Tomato seedling", 0.3, 8, ["seed", "crop"])
	_register("water_bottle", "Water bottle", 0.5, 6, ["water", "grocery"])
	_register("watering_can", "Watering can", 1.2, 1, ["water", "tool"])
	_register("tuna_can", "Canned tuna", 0.15, 10, ["food", "grocery"])
	_register("flour_sr", "Self-raising flour", 1.0, 4, ["food", "grocery"])
	_register("salt", "Salt", 0.3, 8, ["food", "grocery"])
	_register("tomato_fresh", "Tomato", 0.2, 12, ["food", "grocery", "harvest"])
	_register("potato", "Potato", 0.25, 12, ["food", "harvest"])

func _register(id: String, display_name: String, kg: float, stack: int, tags: Array) -> void:
	var item = ItemScript.new()
	item.id = id
	item.display_name = display_name
	item.weight_kg = kg
	item.stack_max = stack
	item.tags = PackedStringArray(tags)
	_items[id] = item

func get_item(id: String):
	return _items.get(id, null)

func all_ids() -> Array:
	return _items.keys()

