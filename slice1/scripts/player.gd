extends CharacterBody3D
## First-person player: WASD move, Space jump, mouse look, E interact.

const SPEED := 4.5
const JUMP_VELOCITY := 5.2
const MOUSE_SENS := 0.0025

@onready var camera: Camera3D = $Camera3D
@onready var interact_ray: RayCast3D = $Camera3D/InteractRay

var gravity: float = ProjectSettings.get_setting("physics/3d/default_gravity")
var _pitch: float = 0.0
var _nearby: Area3D = null
var _hud: Node = null

func _ready() -> void:
	Input.mouse_mode = Input.MOUSE_MODE_CAPTURED
	_hud = get_tree().get_first_node_in_group("hud")

func _unhandled_input(event: InputEvent) -> void:
	if event is InputEventMouseMotion and Input.mouse_mode == Input.MOUSE_MODE_CAPTURED:
		rotate_y(-event.relative.x * MOUSE_SENS)
		_pitch = clamp(_pitch - event.relative.y * MOUSE_SENS, deg_to_rad(-85.0), deg_to_rad(85.0))
		camera.rotation.x = _pitch
	elif event is InputEventKey and event.pressed and event.keycode == KEY_ESCAPE:
		if Input.mouse_mode == Input.MOUSE_MODE_CAPTURED:
			Input.mouse_mode = Input.MOUSE_MODE_VISIBLE
		else:
			Input.mouse_mode = Input.MOUSE_MODE_CAPTURED
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
	_update_nearby()

	if Input.is_action_just_pressed("interact"):
		_try_interact()

func _update_nearby() -> void:
	var found: Area3D = null
	if interact_ray.is_colliding():
		var col := interact_ray.get_collider()
		if col is Area3D and col.has_method("get_prompt"):
			found = col
	_nearby = found
	if _hud and _hud.has_method("set_prompt"):
		if _nearby:
			_hud.set_prompt(_nearby.call("get_prompt"))
		else:
			_hud.set_prompt("")

func _try_interact() -> void:
	if _nearby == null:
		return
	if _nearby.has_method("interact"):
		var msg: Variant = _nearby.call("interact", self)
		if _hud and _hud.has_method("toast") and typeof(msg) == TYPE_STRING and str(msg) != "":
			_hud.toast(str(msg))
