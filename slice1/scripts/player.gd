extends CharacterBody3D
## First-person player. FOV 65. Look-at + E. Tab/I/T inventory.

const SPEED := 4.6
const JUMP_VELOCITY := 5.6
const MOUSE_SENS := 0.0024
const FALL_Y := -8.0

@onready var camera: Camera3D = $Camera3D
@onready var interact_ray: RayCast3D = $Camera3D/InteractRay

var gravity: float = ProjectSettings.get_setting("physics/3d/default_gravity")
var _pitch: float = 0.0
var _nearby: Area3D = null
var _hud: Node = null
var _spawn_xform: Transform3D
var _inv_toggle_frame: int = -1

func _ready() -> void:
	Input.mouse_mode = Input.MOUSE_MODE_CAPTURED
	camera.fov = 65.0
	_resolve_hud()
	call_deferred("_resolve_hud")
	# Force face east (+X toward gap) regardless of scene basis typos.
	look_at(global_position + Vector3(1, 0, 0), Vector3.UP)
	_spawn_xform = global_transform
	# Godot 4 RayCast3D defaults collide_with_areas=false; interactables are Area3D on layer 4.
	interact_ray.enabled = true
	interact_ray.collide_with_areas = true
	interact_ray.collide_with_bodies = false
	interact_ray.collision_mask = 4

func _resolve_hud() -> void:
	_get_hud()

func _get_hud() -> Node:
	if _hud == null or not is_instance_valid(_hud):
		_hud = get_tree().get_first_node_in_group("hud")
	return _hud

func _input(event: InputEvent) -> void:
	_try_inventory_key(event)

func _unhandled_input(event: InputEvent) -> void:
	if event is InputEventMouseMotion and Input.mouse_mode == Input.MOUSE_MODE_CAPTURED:
		rotate_y(-event.relative.x * MOUSE_SENS)
		_pitch = clamp(_pitch - event.relative.y * MOUSE_SENS, deg_to_rad(-80.0), deg_to_rad(80.0))
		camera.rotation.x = _pitch
	elif event is InputEventKey and event.pressed and not event.echo:
		if _is_inventory_key(event):
			_toggle_inv_handled(event)
			return
		match event.keycode:
			KEY_ESCAPE:
				if Input.mouse_mode == Input.MOUSE_MODE_CAPTURED:
					Input.mouse_mode = Input.MOUSE_MODE_VISIBLE
				else:
					Input.mouse_mode = Input.MOUSE_MODE_CAPTURED
			KEY_G:
				# Debug Ember gift
				var ember := get_tree().get_first_node_in_group("ember")
				var hud_g := _get_hud()
				if ember and ember.has_method("drop_gift") and hud_g and hud_g.has_method("toast"):
					hud_g.toast(str(ember.call("drop_gift")))
			KEY_H:
				# Try stow hands
				var hud_h := _get_hud()
				if hud_h and hud_h.has_method("toast"):
					hud_h.toast(Inventory.try_stow_hands())
	elif event is InputEventMouseButton and event.pressed and event.button_index == MOUSE_BUTTON_LEFT:
		if Input.mouse_mode != Input.MOUSE_MODE_CAPTURED:
			Input.mouse_mode = Input.MOUSE_MODE_CAPTURED

func _try_inventory_key(event: InputEvent) -> void:
	if event is InputEventKey and event.pressed and not event.echo and _is_inventory_key(event):
		_toggle_inv_handled(event)

func _is_inventory_key(event: InputEventKey) -> bool:
	# Prefer physical_keycode — project.godot binds Tab/I/T that way; keycode can be 0.
	var codes: Array[int] = [event.keycode, event.physical_keycode]
	for c in codes:
		if c == KEY_TAB or c == KEY_I or c == KEY_T:
			return true
	return false

func _toggle_inv_handled(event: InputEvent) -> void:
	var f := Engine.get_process_frames()
	if f == _inv_toggle_frame:
		get_viewport().set_input_as_handled()
		return
	_inv_toggle_frame = f
	var hud := _get_hud()
	if hud and hud.has_method("toggle_inventory"):
		hud.call("toggle_inventory")
	else:
		push_error("Player: inventory key pressed but HUD not in group 'hud'")
		print("Player: inventory key pressed but HUD not in group 'hud'")
	get_viewport().set_input_as_handled()

func _physics_process(delta: float) -> void:
	if not is_on_floor():
		velocity.y -= gravity * delta
	if Input.is_action_just_pressed("jump") and is_on_floor():
		velocity.y = JUMP_VELOCITY

	var input_dir := Input.get_vector("move_left", "move_right", "move_forward", "move_back")
	var direction := (transform.basis * Vector3(input_dir.x, 0.0, input_dir.y)).normalized()
	if direction != Vector3.ZERO:
		velocity.x = direction.x * SPEED
		velocity.z = direction.z * SPEED
	else:
		velocity.x = move_toward(velocity.x, 0.0, SPEED)
		velocity.z = move_toward(velocity.z, 0.0, SPEED)

	move_and_slide()
	if global_position.y < FALL_Y:
		_respawn()
	_track_zone()
	_update_nearby()

	if Input.is_action_just_pressed("interact"):
		_try_interact()
	# Keep action poll as backup (Tab/I/T bound on inventory action).
	if Input.is_action_just_pressed("inventory"):
		var f := Engine.get_process_frames()
		if f != _inv_toggle_frame:
			_inv_toggle_frame = f
			var hud := _get_hud()
			if hud and hud.has_method("toggle_inventory"):
				hud.call("toggle_inventory")
			else:
				push_error("Player: inventory action but HUD not in group 'hud'")
				print("Player: inventory action but HUD not in group 'hud'")

func _respawn() -> void:
	global_transform = _spawn_xform
	velocity = Vector3.ZERO
	var hud_r := _get_hud()
	if hud_r and hud_r.has_method("toast"):
		hud_r.toast("Whoa — back on the balcony.")

func _track_zone() -> void:
	# Neighbor balcony roughly x > 6.2
	if global_position.x > 6.2:
		GameState.mark_neighbor()

func _update_nearby() -> void:
	# Prefer ray hit; fall back to nearest interactable Area3D within PROX_RANGE
	# so slight off-angle aim still shows E (door / desk / pots).
	const PROX_RANGE := 2.8
	var found: Area3D = null
	if interact_ray.is_colliding():
		var col := interact_ray.get_collider()
		if col is Area3D and col.has_method("get_prompt"):
			found = col
	if found == null:
		var best: Area3D = null
		var best_d := PROX_RANGE
		var origin := global_position
		for n in get_tree().get_nodes_in_group("interactable"):
			if not (n is Area3D):
				continue
			var area := n as Area3D
			if not area.has_method("get_prompt"):
				continue
			var d := origin.distance_to(area.global_position)
			if d < best_d:
				best_d = d
				best = area
		found = best
	_nearby = found
	var hud_p := _get_hud()
	if hud_p and hud_p.has_method("set_prompt"):
		if _nearby:
			hud_p.set_prompt(str(_nearby.call("get_prompt")))
		else:
			hud_p.set_prompt("")

func _try_interact() -> void:
	if _nearby == null:
		return
	if _nearby.has_method("interact"):
		var msg: Variant = _nearby.call("interact", self)
		var hud_i := _get_hud()
		if hud_i and hud_i.has_method("toast") and typeof(msg) == TYPE_STRING and str(msg) != "":
			hud_i.toast(str(msg))
