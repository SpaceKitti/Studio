extends Area3D
class_name Interactable
## Generic look-at + E interactable (door, desk, note, fire escape).

@export var prompt_text: String = "Press E"
@export var interact_id: String = "generic"
@export var one_shot: bool = false

var _done: bool = false
signal interacted(id: String, actor: Node)

func _ready() -> void:
	add_to_group("interactable")
	monitoring = true
	monitorable = true
	collision_layer = 4
	collision_mask = 0

func get_prompt() -> String:
	if _done and one_shot:
		return ""
	if interact_id in ["desk", "search", "water", "harvest", "container"] and Inventory.hands_occupied:
		return "[blocked] Hands full"
	return prompt_text

func interact(actor: Node) -> String:
	if _done and one_shot:
		return ""
	if interact_id == "desk" and Inventory.hands_occupied:
		return "Hands occupied — can't use the desk."
	if one_shot:
		_done = true
	interacted.emit(interact_id, actor)
	match interact_id:
		"door":
			return _open_door(actor)
		"desk":
			return _use_desk()
		"fire_escape":
			return "Fire escape is rusted shut for now. Visible only — M1+ map node."
		"note":
			return "Scrawled note: 'Fed the stray. Bowl's empty. Gone for parts — back never.'"
		"herbs":
			return "Basil and mint pots. Alive. Already rooted — leave them."
		_:
			return "Interacted."

func _open_door(actor: Node) -> String:
	prompt_text = "Door open — interior blocked"
	var level := get_tree().get_first_node_in_group("level")
	if level and level.has_method("on_door_opened"):
		level.call("on_door_opened", actor)
	GameState.mark_cat()
	return "Sliding glass opens. Ember bolts onto the balcony. Adopted."

func _use_desk() -> String:
	var hud := get_tree().get_first_node_in_group("hud")
	if hud and hud.has_method("show_desk"):
		hud.call("show_desk")
	return "Outdoor engineering table. Recipes listed — craft locked until M1."
