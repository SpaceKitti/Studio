extends Area3D
class_name LootContainer
## Searchable container with a small loot table.

@export var container_name: String = "Container"
@export var loot_ids: PackedStringArray = PackedStringArray()
@export var loot_counts: PackedInt32Array = PackedInt32Array()

var _remaining: Dictionary = {}
var _searched: bool = false

func _ready() -> void:
	add_to_group("interactable")
	monitoring = true
	monitorable = true
	collision_layer = 4
	collision_mask = 0
	for i in range(loot_ids.size()):
		var id: String = loot_ids[i]
		var n: int = 1
		if i < loot_counts.size():
			n = int(loot_counts[i])
		_remaining[id] = n
	_ensure_shape()

func _ensure_shape() -> void:
	if get_node_or_null("CollisionShape3D") == null:
		var col := CollisionShape3D.new()
		var shape := BoxShape3D.new()
		shape.size = Vector3(1.4, 1.2, 1.4)
		col.shape = shape
		col.position = Vector3(0, 0.5, 0)
		add_child(col)

func get_prompt() -> String:
	if Inventory.hands_occupied:
		return "[blocked] Hands full"
	if _remaining.is_empty():
		return "[E] Empty %s" % container_name
	return "[E] Search %s" % container_name

func interact(_actor: Node) -> String:
	if Inventory.hands_occupied:
		return "Hands occupied — can't search."
	if _remaining.is_empty():
		return "%s is empty." % container_name
	var id: String = String(_remaining.keys()[0])
	var left: int = int(_remaining[id])
	if not Inventory.add(id, 1):
		return "Couldn't take item — check weight."
	left -= 1
	if left <= 0:
		_remaining.erase(id)
	else:
		_remaining[id] = left
	_searched = true
	GameState.mark_pickup()
	var item = ItemDB.get_item(id)
	var nm: String = id
	var w: float = 0.0
	if item:
		nm = str(item.display_name)
		w = float(item.weight_kg)
	return "Took %s (%.2f kg) from %s." % [nm, w, container_name]
