extends CharacterBody3D
## Ember — adopted cat. Follows loosely across both balconies.

var adopted: bool = false
var _follow_target: Node3D = null
var _gift_ready: bool = true

const FOLLOW_SPEED := 2.8
const FOLLOW_STOP := 1.4

func _ready() -> void:
	# Start locked behind glass (kinematic idle)
	pass

func adopt(player: Node3D) -> void:
	if adopted:
		return
	adopted = true
	_follow_target = player
	# Move onto balcony in front of door
	global_position = Vector3(1.2, 0.2, 0.6)
	visible = true

func _physics_process(delta: float) -> void:
	if not adopted or _follow_target == null:
		velocity = Vector3.ZERO
		return
	var to := _follow_target.global_position - global_position
	to.y = 0.0
	var dist := to.length()
	if dist > FOLLOW_STOP:
		var dir := to.normalized()
		velocity.x = dir.x * FOLLOW_SPEED
		velocity.z = dir.z * FOLLOW_SPEED
		if dir.length() > 0.01:
			look_at(global_position + dir, Vector3.UP)
	else:
		velocity.x = move_toward(velocity.x, 0.0, FOLLOW_SPEED)
		velocity.z = move_toward(velocity.z, 0.0, FOLLOW_SPEED)
	# Stick to floor height
	if not is_on_floor():
		velocity.y -= ProjectSettings.get_setting("physics/3d/default_gravity") * delta
	else:
		velocity.y = 0.0
	move_and_slide()

func drop_gift() -> String:
	if not adopted:
		return "Ember isn't free yet."
	if not _gift_ready:
		return "Ember already dropped a gift (debug)."
	_gift_ready = false
	# Spawn a tuna near player feet as "pigeon stand-in" debug gift
	if Inventory.add("tuna_can", 1):
		GameState.mark_pickup()
		return "DEBUG: Ember drops a 'gift' (tuna stand-in) at your feet."
	return "DEBUG: Ember tried to gift but backpack rejected it."
