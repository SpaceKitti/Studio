extends Area3D
class_name PlantSpot
## Stupid-simple garden: empty → planted → growing → harvestable.

enum State { EMPTY, PLANTED, GROWING, HARVESTABLE, DEAD }

@export var spot_name: String = "Pot"
@export var initial_state: State = State.EMPTY
@export var crop_id: String = "tomato_fresh"
@export var seed_id: String = "tomato_seed"
@export var accept_seed_ids: PackedStringArray = PackedStringArray(["tomato_seed", "potato_seed"])

var state: State = State.EMPTY
var watered: bool = false
var _mesh_plant: MeshInstance3D

func _ready() -> void:
	add_to_group("interactable")
	collision_layer = 4
	collision_mask = 0
	monitoring = false
	monitorable = true
	state = initial_state
	if state == State.GROWING:
		watered = false
	_ensure_shape()
	_refresh_visual()

func _ensure_shape() -> void:
	if get_node_or_null("CollisionShape3D") == null:
		var col := CollisionShape3D.new()
		var shape := BoxShape3D.new()
		shape.size = Vector3(1.2, 1.4, 1.2)
		col.shape = shape
		col.position = Vector3(0, 0.5, 0)
		add_child(col)

func get_prompt() -> String:
	if Inventory.hands_occupied:
		return "[blocked] Hands full"
	match state:
		State.EMPTY:
			return "[E] Plant seed (%s)" % spot_name
		State.PLANTED:
			return "[E] Water %s" % spot_name
		State.GROWING:
			if watered:
				return "[E] Wait… almost ready"
			return "[E] Water %s" % spot_name
		State.HARVESTABLE:
			return "[E] Harvest %s" % spot_name
		_:
			return ""

func interact(_actor: Node) -> String:
	if Inventory.hands_occupied:
		return "Hands occupied — can't garden."
	match state:
		State.EMPTY:
			return _try_plant()
		State.PLANTED, State.GROWING:
			if state == State.GROWING and watered:
				return "Already watered. Growth is mid — harvest soon (press E again after a beat)."
			return _try_water()
		State.HARVESTABLE:
			return _try_harvest()
		_:
			return "Dead plant."

func _try_plant() -> String:
	var chosen: String = ""
	for sid in accept_seed_ids:
		if Inventory.count(sid) > 0:
			chosen = sid
			break
	if chosen == "":
		return "Need a seed in backpack (tomato seedling or seed potato)."
	Inventory.remove(chosen, 1)
	seed_id = chosen
	if chosen == "potato_seed":
		crop_id = "potato"
	else:
		crop_id = "tomato_fresh"
	state = State.PLANTED
	GameState.mark_plant()
	_refresh_visual()
	var item = ItemDB.get_item(chosen)
	var nm: String = chosen
	if item:
		nm = str(item.display_name)
	return "Planted %s in %s." % [nm, spot_name]

func _try_water() -> String:
	if Inventory.count("water_bottle") <= 0:
		return "Need a water bottle."
	Inventory.remove("water_bottle", 1)
	watered = true
	GameState.mark_water()
	if state == State.PLANTED:
		state = State.HARVESTABLE
	elif state == State.GROWING:
		state = State.HARVESTABLE
	_refresh_visual()
	return "Watered %s. Ready to harvest." % spot_name

func _try_harvest() -> String:
	var item = ItemDB.get_item(crop_id)
	if item == null:
		return "Nothing to harvest."
	if not Inventory.add(crop_id, 1):
		return "Could not take harvest — inventory full."
	GameState.mark_harvest()
	GameState.mark_pickup()
	state = State.EMPTY
	watered = false
	_refresh_visual()
	return "Harvested %s (+%.2f kg)." % [str(item.display_name), float(item.weight_kg)]

func _refresh_visual() -> void:
	if _mesh_plant and is_instance_valid(_mesh_plant):
		_mesh_plant.queue_free()
	_mesh_plant = null
	var color := Color(0.2, 0.45, 0.22)
	var scale_y := 0.15
	match state:
		State.EMPTY:
			return
		State.PLANTED:
			scale_y = 0.2
			color = Color(0.35, 0.5, 0.25)
		State.GROWING:
			scale_y = 0.45
			color = Color(0.28, 0.55, 0.28)
		State.HARVESTABLE:
			scale_y = 0.55
			if crop_id == "tomato_fresh":
				color = Color(0.75, 0.3, 0.2)
			else:
				color = Color(0.72, 0.62, 0.35)
		State.DEAD:
			color = Color(0.35, 0.28, 0.2)
			scale_y = 0.2
	_mesh_plant = MeshInstance3D.new()
	var box := BoxMesh.new()
	box.size = Vector3(0.28, scale_y, 0.28)
	_mesh_plant.mesh = box
	var mat := StandardMaterial3D.new()
	mat.albedo_color = color
	mat.roughness = 0.85
	_mesh_plant.material_override = mat
	_mesh_plant.position = Vector3(0, 0.35 + scale_y * 0.5, 0)
	add_child(_mesh_plant)
