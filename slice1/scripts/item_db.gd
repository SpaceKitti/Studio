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
	# Weapons (m0_weapons_sheet)
	_register("machete", "Machete", 1.2, 1, ["weapon"])
	_register("spear", "Spear", 1.5, 1, ["weapon"])
	_register("baseball_bat", "Baseball bat", 1.0, 1, ["weapon"])
	_register("pipe", "Pipe", 1.8, 1, ["weapon"])
	_register("kitchen_knife", "Kitchen knife", 0.3, 1, ["weapon"])
	_register("fire_axe", "Fire axe", 2.5, 1, ["weapon"])
	_register("shiv", "Shiv", 0.2, 1, ["weapon"])
	_register("bow", "Bow", 1.0, 1, ["weapon"])
	# Materials / craft (m0_materials_sheet; nails+duct_tape already elsewhere)
	_register("flint", "Flint", 0.15, 20, ["material", "craft"])
	_register("string", "String", 0.05, 30, ["material", "craft"])
	_register("twine", "Twine", 0.1, 20, ["material", "craft"])
	_register("logs", "Logs", 2.0, 8, ["material", "craft"])
	_register("sticks", "Sticks", 0.4, 20, ["material", "craft"])
	_register("scrap_cloth", "Scrap cloth", 0.15, 15, ["material", "craft"])


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

