extends Area3D
class_name WaterFixture
## Sink / bathtub: fill bottle, plug toggle, rising water when plugged + running.

enum Kind { SINK, BATHTUB }

@export var fixture_name: String = "Sink"
@export var kind: Kind = Kind.SINK

var plugged: bool = false
var water_level: float = 0.0
var running: bool = true  # M0/M1 fiction: water works
var _water_mesh: MeshInstance3D = null
var _basin_size: Vector3 = Vector3(0.55, 0.12, 0.4)

func _ready() -> void:
	add_to_group("interactable")
	collision_layer = 4
	collision_mask = 0
	monitoring = false
	monitorable = true
	if kind == Kind.BATHTUB:
		_basin_size = Vector3(1.4, 0.35, 0.7)
	_ensure_shape()
	_ensure_water_visual()

func _ensure_shape() -> void:
	if get_node_or_null("CollisionShape3D") == null:
		var col := CollisionShape3D.new()
		var shape := BoxShape3D.new()
		if kind == Kind.BATHTUB:
			shape.size = Vector3(1.8, 1.4, 1.2)
		else:
			shape.size = Vector3(1.2, 1.4, 1.0)
		col.shape = shape
		col.position = Vector3(0, 0.5, 0)
		add_child(col)

func _ensure_water_visual() -> void:
	if _water_mesh != null and is_instance_valid(_water_mesh):
		return
	_water_mesh = MeshInstance3D.new()
	_water_mesh.name = "WaterLevel"
	var box := BoxMesh.new()
	box.size = Vector3(_basin_size.x * 0.9, 0.02, _basin_size.z * 0.9)
	_water_mesh.mesh = box
	var mat := StandardMaterial3D.new()
	mat.albedo_color = Color(0.35, 0.55, 0.75, 0.55)
	mat.transparency = BaseMaterial3D.TRANSPARENCY_ALPHA
	mat.roughness = 0.15
	mat.metallic = 0.05
	_water_mesh.material_override = mat
	_water_mesh.position = Vector3(0, 0.08, 0)
	_water_mesh.visible = false
	add_child(_water_mesh)

func _process(delta: float) -> void:
	if plugged and running:
		water_level = clampf(water_level + delta * 0.22, 0.0, 1.0)
	elif not plugged:
		water_level = clampf(water_level - delta * 0.35, 0.0, 1.0)
	_refresh_water_visual()

func _refresh_water_visual() -> void:
	if _water_mesh == null:
		return
	if water_level <= 0.02:
		_water_mesh.visible = false
		return
	_water_mesh.visible = true
	var h := 0.02 + water_level * (_basin_size.y * 0.85)
	var box := _water_mesh.mesh as BoxMesh
	if box:
		box.size = Vector3(_basin_size.x * 0.9, h, _basin_size.z * 0.9)
	_water_mesh.position = Vector3(0, 0.06 + h * 0.5, 0)

func get_prompt() -> String:
	var plug_bit := "unplug" if plugged else "plug"
	var fill_ok := _can_fill_bottle()
	if fill_ok:
		return "[E] %s — fill bottle / %s" % [fixture_name, plug_bit]
	if Inventory.hands_occupied:
		return "[E] %s — %s (hands full for fill)" % [fixture_name, plug_bit]
	return "[E] %s — %s" % [fixture_name, plug_bit]

func interact(_actor: Node) -> String:
	# Prefer fill when possible; otherwise toggle plug.
	if _can_fill_bottle():
		if not Inventory.add("water_bottle", 1):
			return "Can't fill — backpack rejected the bottle."
		GameState.mark_pickup()
		return "Filled a water bottle from the %s." % fixture_name.to_lower()
	# Toggle plug
	plugged = not plugged
	if plugged:
		if running:
			return "%s plugged — water rising." % fixture_name
		return "%s plugged (no pressure yet)." % fixture_name
	if water_level > 0.05:
		return "%s unplugged — water draining." % fixture_name
	return "%s unplugged." % fixture_name

func _can_fill_bottle() -> bool:
	if Inventory.hands_occupied:
		return false
	return Inventory.can_add("water_bottle", 1)
