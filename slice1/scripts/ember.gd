extends CharacterBody3D
## Ember — adopted cat. Follows loosely across both balconies.

var adopted: bool = false
var _follow_target: Node3D = null
var _gift_ready: bool = true
var _gap_toast_shown: bool = false

const FOLLOW_SPEED := 2.8
const FOLLOW_STOP := 1.4
const GAP_HOP_DIST := 4.5
const HOME_SIDE_MAX_X := 5.0
const NEIGHBOR_SIDE_MIN_X := 6.5
const HOP_OFFSET := 1.2

func _ready() -> void:
	# Start locked behind glass (kinematic idle)
	pass

func adopt(player: Node3D) -> void:
	if adopted:
		return
	adopted = true
	_follow_target = player
	# Move onto balcony in front of door
	# Appear on neighbor balcony in front of neighbor glass
	global_position = Vector3(10.3, 0.2, 0.6)
	visible = true

func _physics_process(delta: float) -> void:
	if not adopted or _follow_target == null:
		velocity = Vector3.ZERO
		return

	if _needs_gap_hop():
		_hop_to_player()
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

func _needs_gap_hop() -> bool:
	var px := _follow_target.global_position.x
	var ex := global_position.x
	var to := _follow_target.global_position - global_position
	to.y = 0.0
	if to.length() > GAP_HOP_DIST:
		return true
	# Different sides of the jump gap
	if ex < HOME_SIDE_MAX_X and px > NEIGHBOR_SIDE_MIN_X:
		return true
	if px < HOME_SIDE_MAX_X and ex > NEIGHBOR_SIDE_MIN_X:
		return true
	return false

func _hop_to_player() -> void:
	var p := _follow_target.global_position
	# Offset ~1.2m behind/beside on same balcony
	var dest := Vector3(p.x - HOP_OFFSET * 0.7, 0.2, p.z + HOP_OFFSET * 0.7)
	# Keep Ember on the same balcony slab as the player
	if p.x > NEIGHBOR_SIDE_MIN_X:
		dest.x = clampf(dest.x, 6.8, 10.6)
	else:
		dest.x = clampf(dest.x, 0.4, 3.9)
	dest.z = clampf(dest.z, -2.4, 2.4)
	global_position = dest
	velocity = Vector3.ZERO
	if not _gap_toast_shown:
		_gap_toast_shown = true
		var hud := get_tree().get_first_node_in_group("hud")
		if hud and hud.has_method("toast"):
			hud.toast("Ember hops the gap.")

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