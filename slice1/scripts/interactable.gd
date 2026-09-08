extends Area3D
class_name Interactable
## Base interactable: desk, herbs, garden, Ember.

@export var prompt_text: String = "Press E"
@export var interact_id: String = "generic"
@export var one_shot: bool = false

var _done: bool = false
signal interacted(id: String, actor: Node)

func get_prompt() -> String:
	if _done and one_shot:
		return ""
	return prompt_text

func interact(actor: Node) -> String:
	if _done and one_shot:
		return ""
	if one_shot:
		_done = true
	interacted.emit(interact_id, actor)
	match interact_id:
		"desk":
			return "Engineering desk — tools ready. Craft tree locked for later slices."
		"herbs":
			return "Basil and mint. Still green. Still yours."
		"garden":
			return "Neighbor's potato / tomato bed — half-started, cared for."
		"ember":
			return _free_ember(actor)
		_:
			return "Interacted."

func _free_ember(actor: Node) -> String:
	var ember := get_parent()
	if ember and ember.has_method("on_freed"):
		ember.call("on_freed", actor)
	elif ember:
		ember.global_position = Vector3(8.5, 0.35, 0.5)
	return "Ember presses against the glass — you open it. She's free. Adopted for now."
