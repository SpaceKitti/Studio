extends Node3D
## Ember placeholder cat at neighbor glass.

var freed: bool = false

func on_freed(_actor: Node) -> void:
	if freed:
		return
	freed = true
	global_position = Vector3(8.2, 0.35, 0.8)
	rotation_degrees = Vector3(0, -40, 0)
	var label := get_node_or_null("Label")
	if label:
		label.text = "Ember (free)"
