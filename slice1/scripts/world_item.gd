extends Area3D
class_name WorldItem
## Pickup lying in the world.

@export var item_id: String = "tuna_can"
@export var amount: int = 1

var _taken: bool = false

func _ready() -> void:
	collision_layer = 4
	collision_mask = 0
	_ensure_shape()

func _ensure_shape() -> void:
	if get_node_or_null("CollisionShape3D") == null:
		var col := CollisionShape3D.new()
		var shape := BoxShape3D.new()
		shape.size = Vector3(0.8, 0.8, 0.8)
		col.shape = shape
		add_child(col)

func get_prompt() -> String:
	if _taken:
		return ""
	var item = ItemDB.get_item(item_id)
	var nm: String = item_id
	if item:
		nm = str(item.display_name)
	return "[E] Pick up %s" % nm

func interact(_actor: Node) -> String:
	if _taken:
		return ""
	if not Inventory.add(item_id, amount):
		return "Can't pick up — backpack too heavy."
	_taken = true
	GameState.mark_pickup()
	visible = false
	set_deferred("monitorable", false)
	var item = ItemDB.get_item(item_id)
	var nm: String = item_id
	if item:
		nm = str(item.display_name)
	return "Picked up %s." % nm
