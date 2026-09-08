extends CharacterBody3D
## First-person player. FOV 65. Look-at + E. Tab inventory.

const SPEED := 4.6
const JUMP_VELOCITY := 5.6
const MOUSE_SENS := 0.0024

@onready var camera: Camera3D = $Camera3D
@onready var interact_ray: RayCast3D = $Camera3D/InteractRay

var gravity: float = ProjectSettings.get_setting("physics/3d/default_gravity")
var _pitch: float = 0.0
var _nearby: Area3D = null
var _hud: Node = null

func _ready() -> void:
	Input.mouse_mode = Input.MOUSE_MODE_CAPTURED
	camera.fov = 65.0
	_hud = get_tree().get_first_node_in_group("hud")

func _unhandled_input(event: InputEvent) -> void:
	if event is InputEventMouseMotion and Input.mouse_mode == Input.MOUSE_MODE_CAPTURED:
		rotate_y(-event.relative.x * MOUSE_SENS)
		_pitch = clamp(_pitch - event.relative.y * MOUSE_SENS, deg_to_rad(-80.0), deg_to_rad(80.0))
		camera.rotation.x = _pitch
	elif event is InputEventKey and event.pressed and not event.echo:
		match event.keycode:
			KEY_ESCAPE:
				if Input.mouse_mode == Input.MOUSE_MODE_CAPTURED:
					Input.mouse_mode = Input.MOUSE_MODE_VISIBLE
				else:
					Input.mouse_mode = Input.MOUSE_MODE_CAPTURED
			KEY_TAB, KEY_I:
				if _hud and _hud.has_method("toggle_inventory"):
					_hud.call("toggle_inventory")
			KEY_G:
				# Debug Ember gift
				var ember := get_tree().get_first_node_in_group("ember")
				if ember and ember.has_method("drop_gift") and _hud and _hud.has_method("toast"):
					_hud.toast(str(ember.call("drop_gift")))
			KEY_H:
				# Try stow hands
				if _hud and _hud.has_method("toast"):
					_hud.toast(Inventory.try_stow_hands())
	elif event is InputEventMouseButton and event.pressed and event.button_index == MOUSE_BUTTON_LEFT:
		if Input.mouse_mode != Input.MOUSE_MODE_CAPTURED:
			Input.mouse_mode = Input.MOUSE_MODE_CAPTURED

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
	_track_zone()
	_update_nearby()

	if Input.is_action_just_pressed("interact"):
		_try_interact()

func _track_zone() -> void:
	# Neighbor balcony roughly x > 6.2
	if global_position.x > 6.2:
		GameState.mark_neighbor()

func _update_nearby() -> void:
	var found: Area3D = null
	if interact_ray.is_colliding():
		var col := interact_ray.get_collider()
		if col is Area3D and col.has_method("get_prompt"):
			found = col
	_nearby = found
	if _hud and _hud.has_method("set_prompt"):
		if _nearby:
			_hud.set_prompt(str(_nearby.call("get_prompt")))
		else:
			_hud.set_prompt("")

func _try_interact() -> void:
	if _nearby == null:
		return
	if _nearby.has_method("interact"):
		var msg: Variant = _nearby.call("interact", self)
		if _hud and _hud.has_method("toast") and typeof(msg) == TYPE_STRING and str(msg) != "":
			_hud.toast(str(msg))
