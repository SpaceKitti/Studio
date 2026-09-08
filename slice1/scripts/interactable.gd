extends Area3D
class_name Interactable
## Generic look-at + E interactable (door, desk, note, fire escape, glass).

@export var prompt_text: String = "Press E"
@export var interact_id: String = "generic"
@export var one_shot: bool = false

var _done: bool = false
var glass_open: bool = false
signal interacted(id: String, actor: Node)

func _ready() -> void:
	add_to_group("interactable")
	monitoring = true
	monitorable = true
	collision_layer = 4
	collision_mask = 0

func get_prompt() -> String:
	if _done and one_shot:
		# Neighbor door one_shot opens Ember path; after that re-label for entry
		if interact_id == "door" and glass_open:
			return "[E] Enter neighbor apartment"
		return ""
	if interact_id in ["desk", "search", "harvest", "container"] and Inventory.hands_occupied:
		return "[blocked] Hands full"
	if interact_id == "home_glass":
		if glass_open:
			return "[E] Enter home apartment"
		return "[E] Open home sliding glass"
	if interact_id == "door":
		if glass_open:
			return "[E] Enter neighbor apartment"
		return prompt_text
	return prompt_text

func interact(actor: Node) -> String:
	if interact_id == "desk" and Inventory.hands_occupied:
		return "Hands occupied — can't use the desk."
	# Neighbor glass: first open frees Ember; later allows re-entry toast
	if interact_id == "door":
		return _open_neighbor_glass(actor)
	if interact_id == "home_glass":
		return _open_home_glass(actor)
	if _done and one_shot:
		return ""
	if one_shot:
		_done = true
	interacted.emit(interact_id, actor)
	match interact_id:
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

func _open_neighbor_glass(actor: Node) -> String:
	var level := get_tree().get_first_node_in_group("level")
	if not glass_open:
		glass_open = true
		_done = true
		one_shot = true
		prompt_text = "[E] Enter neighbor apartment"
		interacted.emit("door", actor)
		if level and level.has_method("on_neighbor_glass_opened"):
			level.call("on_neighbor_glass_opened", actor)
		elif level and level.has_method("on_door_opened"):
			level.call("on_door_opened", actor)
		GameState.mark_cat()
		return "Neighbor sliding glass opens. Ember bolts onto their balcony. Adopted. Apartment unlocked."
	# Already open — nudge into apt
	if level and level.has_method("nudge_into_apartment"):
		level.call("nudge_into_apartment", "neighbor", actor)
	return "Neighbor apartment is open — walk through the glass."

func _open_home_glass(actor: Node) -> String:
	var level := get_tree().get_first_node_in_group("level")
	if not glass_open:
		glass_open = true
		prompt_text = "[E] Enter home apartment"
		interacted.emit("home_glass", actor)
		if level and level.has_method("on_home_glass_opened"):
			level.call("on_home_glass_opened", actor)
		return "Home sliding glass opens. Your apartment is unlocked."
	if level and level.has_method("nudge_into_apartment"):
		level.call("nudge_into_apartment", "home", actor)
	return "Home apartment is open — walk through the glass."

func _use_desk() -> String:
	var hud := get_tree().get_first_node_in_group("hud")
	if hud and hud.has_method("show_desk"):
		hud.call("show_desk")
	return "Outdoor engineering table. Recipes listed — craft locked until later."
