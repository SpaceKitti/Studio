extends Node
## Autoload: M0 win tracking + recipe data.

signal win_updated
signal toast_request(text: String)

const RecipeScript = preload("res://scripts/recipe.gd")

var cat_adopted: bool = false
var visited_home: bool = true
var visited_neighbor: bool = false
var items_picked: int = 0
var planted_once: bool = false
var watered_once: bool = false
var harvested_once: bool = false
var inventory_opened: bool = false

var recipes: Array = []

func _ready() -> void:
	var solar = RecipeScript.new()
	solar.id = "solar_rig"
	solar.display_name = "Solar charger rig"
	solar.station = "engineering_desk"
	solar.result_id = "solar_charger"
	solar.piece_ids = PackedStringArray(["solar_panel_shard", "battery_cell"])
	solar.piece_counts = PackedInt32Array([3, 1])
	recipes.append(solar)

func mark_neighbor() -> void:
	if not visited_neighbor:
		visited_neighbor = true
		win_updated.emit()

func mark_cat() -> void:
	if not cat_adopted:
		cat_adopted = true
		win_updated.emit()
		toast_request.emit("Ember adopted. She'll stick close.")

func mark_pickup() -> void:
	items_picked += 1
	win_updated.emit()

func mark_plant() -> void:
	planted_once = true
	win_updated.emit()

func mark_water() -> void:
	watered_once = true
	win_updated.emit()

func mark_harvest() -> void:
	harvested_once = true
	win_updated.emit()

func mark_inventory_open() -> void:
	if not inventory_opened:
		inventory_opened = true
		win_updated.emit()

func win_met() -> bool:
	return (
		visited_home and visited_neighbor
		and cat_adopted
		and items_picked >= 3
		and planted_once and watered_once and harvested_once
		and inventory_opened
	)

func checklist() -> PackedStringArray:
	return PackedStringArray([
		_ck(visited_home and visited_neighbor, "Walk both balconies"),
		_ck(cat_adopted, "Free / adopt Ember"),
		_ck(items_picked >= 3, "Pick up 3 items (%d/3)" % mini(items_picked, 3)),
		_ck(planted_once and watered_once and harvested_once, "Plant / water / harvest 1 crop"),
		_ck(inventory_opened, "Open inventory (shows kg)"),
	])

func _ck(ok: bool, label: String) -> String:
	return ("[x] " if ok else "[ ] ") + label
