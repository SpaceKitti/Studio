extends Node3D
## Builds Slice 1 layout with CSG / MeshInstance3D placeholders.
## Layout: apartment interior (desk inside) → door → player balcony (herbs)
## → gap → neighbor balcony (garden) → glass + Ember.

func _ready() -> void:
	_build_environment()
	_build_apartment()
	_build_player_balcony()
	_build_neighbor_balcony()
	_build_props()

func _mat(color: Color, rough: float = 0.85) -> StandardMaterial3D:
	var m := StandardMaterial3D.new()
	m.albedo_color = color
	m.roughness = rough
	return m

func _box(parent: Node, size: Vector3, pos: Vector3, color: Color, name: String = "Box") -> CSGBox3D:
	var b := CSGBox3D.new()
	b.name = name
	b.size = size
	b.position = pos
	b.material = _mat(color)
	b.use_collision = true
	parent.add_child(b)
	return b

func _build_environment() -> void:
	var env := WorldEnvironment.new()
	env.name = "WorldEnvironment"
	var e := Environment.new()
	e.background_mode = Environment.BG_COLOR
	e.background_color = Color(0.62, 0.72, 0.82) # soft sky haze
	e.ambient_light_source = Environment.AMBIENT_SOURCE_COLOR
	e.ambient_light_color = Color(0.95, 0.88, 0.78)
	e.ambient_light_energy = 0.55
	e.tonemap_mode = Environment.TONE_MAPPER_FILMIC
	env.environment = e
	add_child(env)

	var sun := DirectionalLight3D.new()
	sun.name = "Sun"
	sun.rotation_degrees = Vector3(-48, 35, 0)
	sun.light_color = Color(1.0, 0.95, 0.88)
	sun.light_energy = 1.15
	sun.shadow_enabled = true
	add_child(sun)

func _build_apartment() -> void:
	var apt := Node3D.new()
	apt.name = "Apartment"
	add_child(apt)

	# Warm concrete / terracotta interior — desk MUST be indoors (x < 0)
	var concrete := Color(0.72, 0.66, 0.58)
	var terracotta := Color(0.78, 0.48, 0.32)
	var wall := Color(0.86, 0.80, 0.72)
	var floor_c := Color(0.55, 0.48, 0.40)

	# Floor interior: x -6..0.4, z -3.5..3.5
	_box(apt, Vector3(6.4, 0.25, 7.0), Vector3(-2.8, -0.125, 0.0), floor_c, "Floor")
	# Ceiling
	_box(apt, Vector3(6.4, 0.2, 7.0), Vector3(-2.8, 3.1, 0.0), wall, "Ceiling")
	# Back wall (west)
	_box(apt, Vector3(0.25, 3.2, 7.0), Vector3(-6.0, 1.5, 0.0), wall, "WallBack")
	# Side walls
	_box(apt, Vector3(6.4, 3.2, 0.25), Vector3(-2.8, 1.5, -3.5), wall, "WallSouth")
	_box(apt, Vector3(6.4, 3.2, 0.25), Vector3(-2.8, 1.5, 3.5), wall, "WallNorth")
	# East wall with door opening (balcony side): solid left/right of opening
	_box(apt, Vector3(0.25, 3.2, 2.2), Vector3(0.35, 1.5, -2.4), wall, "WallEastL")
	_box(apt, Vector3(0.25, 3.2, 2.2), Vector3(0.35, 1.5, 2.4), wall, "WallEastR")
	_box(apt, Vector3(0.25, 1.0, 2.6), Vector3(0.35, 2.6, 0.0), wall, "WallEastHeader")

	# Lived-in clutter stubs
	_box(apt, Vector3(1.2, 0.9, 0.5), Vector3(-5.2, 0.45, -2.8), terracotta, "Shelf")
	_box(apt, Vector3(0.6, 0.7, 0.6), Vector3(-5.0, 0.35, 2.6), Color(0.45, 0.42, 0.38), "Crate")

	# Engineering DESK INSIDE apartment (not on balcony)
	var desk_root := Node3D.new()
	desk_root.name = "Desk"
	desk_root.position = Vector3(-3.2, 0.0, -1.2)
	apt.add_child(desk_root)
	_box(desk_root, Vector3(1.8, 0.08, 0.9), Vector3(0.0, 0.78, 0.0), Color(0.42, 0.32, 0.24), "DeskTop")
	_box(desk_root, Vector3(0.08, 0.78, 0.08), Vector3(-0.8, 0.39, -0.35), Color(0.35, 0.28, 0.22), "Leg1")
	_box(desk_root, Vector3(0.08, 0.78, 0.08), Vector3(0.8, 0.39, -0.35), Color(0.35, 0.28, 0.22), "Leg2")
	_box(desk_root, Vector3(0.08, 0.78, 0.08), Vector3(-0.8, 0.39, 0.35), Color(0.35, 0.28, 0.22), "Leg3")
	_box(desk_root, Vector3(0.08, 0.78, 0.08), Vector3(0.8, 0.39, 0.35), Color(0.35, 0.28, 0.22), "Leg4")
	_box(desk_root, Vector3(0.35, 0.12, 0.25), Vector3(0.4, 0.88, 0.1), Color(0.55, 0.55, 0.58), "ToolBox")
	_box(desk_root, Vector3(0.5, 0.05, 0.35), Vector3(-0.3, 0.85, 0.0), Color(0.2, 0.22, 0.25), "Pad")

	var desk_area := Area3D.new()
	desk_area.name = "DeskInteract"
	desk_area.collision_layer = 4
	desk_area.collision_mask = 0
	desk_area.position = Vector3(0.0, 0.9, 0.5)
	var desk_script: Script = load("res://scripts/interactable.gd")
	desk_area.set_script(desk_script)
	desk_area.set("prompt_text", "[E] Engineering desk")
	desk_area.set("interact_id", "desk")
	desk_root.add_child(desk_area)
	var desk_col := CollisionShape3D.new()
	var desk_shape := BoxShape3D.new()
	desk_shape.size = Vector3(2.0, 1.5, 1.5)
	desk_col.shape = desk_shape
	desk_area.add_child(desk_col)

	# Interior spill lamp
	var lamp := OmniLight3D.new()
	lamp.name = "DeskLamp"
	lamp.position = Vector3(-3.2, 1.6, -1.2)
	lamp.light_color = Color(1.0, 0.85, 0.65)
	lamp.light_energy = 1.4
	lamp.omni_range = 6.0
	apt.add_child(lamp)

func _build_player_balcony() -> void:
	var bal := Node3D.new()
	bal.name = "PlayerBalcony"
	add_child(bal)
	var concrete := Color(0.70, 0.64, 0.56)
	var rail := Color(0.78, 0.76, 0.72)

	# Floor x 0.4..4.4
	_box(bal, Vector3(4.0, 0.25, 6.0), Vector3(2.4, -0.125, 0.0), concrete, "BalconyFloor")
	# Railings (open toward gap on +X except low edge markers)
	_box(bal, Vector3(4.0, 0.9, 0.12), Vector3(2.4, 0.45, -3.0), rail, "RailS")
	_box(bal, Vector3(4.0, 0.9, 0.12), Vector3(2.4, 0.45, 3.0), rail, "RailN")
	_box(bal, Vector3(0.12, 0.9, 2.0), Vector3(4.35, 0.45, -2.0), rail, "RailE_S")
	_box(bal, Vector3(0.12, 0.9, 2.0), Vector3(4.35, 0.45, 2.0), rail, "RailE_N")
	# Gap opening in middle of east edge — no rail at z~0

	# Herb pots
	var herb_green := Color(0.28, 0.55, 0.32)
	var pot := Color(0.72, 0.42, 0.28)
	for i in range(3):
		var hx := 1.2 + i * 0.9
		_box(bal, Vector3(0.45, 0.35, 0.45), Vector3(hx, 0.2, -2.4), pot, "HerbPot%d" % i)
		_box(bal, Vector3(0.35, 0.25, 0.35), Vector3(hx, 0.48, -2.4), herb_green, "HerbTop%d" % i)

	var herbs_area := Area3D.new()
	herbs_area.name = "HerbsInteract"
	herbs_area.collision_layer = 4
	herbs_area.collision_mask = 0
	herbs_area.position = Vector3(2.1, 0.5, -2.2)
	herbs_area.set_script(load("res://scripts/interactable.gd"))
	herbs_area.set("prompt_text", "[E] Check herbs")
	herbs_area.set("interact_id", "herbs")
	bal.add_child(herbs_area)
	var hcol := CollisionShape3D.new()
	var hshape := BoxShape3D.new()
	hshape.size = Vector3(2.8, 1.2, 1.2)
	hcol.shape = hshape
	herbs_area.add_child(hcol)

func _build_neighbor_balcony() -> void:
	var nb := Node3D.new()
	nb.name = "NeighborBalcony"
	add_child(nb)
	var concrete := Color(0.68, 0.62, 0.55)
	var rail := Color(0.75, 0.73, 0.70)

	# Gap between x 4.4 and 6.6 (~2.2m jump)
	# Neighbor floor x 6.6..11.0
	_box(nb, Vector3(4.4, 0.25, 6.0), Vector3(8.8, -0.125, 0.0), concrete, "NeighborFloor")
	_box(nb, Vector3(4.4, 0.9, 0.12), Vector3(8.8, 0.45, -3.0), rail, "NRailS")
	_box(nb, Vector3(4.4, 0.9, 0.12), Vector3(8.8, 0.45, 3.0), rail, "NRailN")
	_box(nb, Vector3(0.12, 0.9, 2.0), Vector3(6.65, 0.45, -2.0), rail, "NRailW_S")
	_box(nb, Vector3(0.12, 0.9, 2.0), Vector3(6.65, 0.45, 2.0), rail, "NRailW_N")

	# Potato / tomato dressing
	var soil := Color(0.35, 0.25, 0.18)
	var tomato_leaf := Color(0.25, 0.50, 0.22)
	var tomato_fruit := Color(0.85, 0.25, 0.18)
	_box(nb, Vector3(1.6, 0.3, 1.0), Vector3(9.2, 0.2, -1.8), soil, "GardenBed")
	_box(nb, Vector3(0.15, 0.7, 0.15), Vector3(8.8, 0.55, -1.8), tomato_leaf, "Stake1")
	_box(nb, Vector3(0.15, 0.7, 0.15), Vector3(9.5, 0.55, -1.6), tomato_leaf, "Stake2")
	_box(nb, Vector3(0.5, 0.35, 0.4), Vector3(9.1, 0.7, -1.7), tomato_leaf, "Foliage")
	_box(nb, Vector3(0.12, 0.12, 0.12), Vector3(9.0, 0.85, -1.55), tomato_fruit, "Tomato")

	var garden_area := Area3D.new()
	garden_area.name = "GardenInteract"
	garden_area.collision_layer = 4
	garden_area.collision_mask = 0
	garden_area.position = Vector3(9.2, 0.5, -1.5)
	garden_area.set_script(load("res://scripts/interactable.gd"))
	garden_area.set("prompt_text", "[E] Inspect garden")
	garden_area.set("interact_id", "garden")
	nb.add_child(garden_area)
	var gcol := CollisionShape3D.new()
	var gshape := BoxShape3D.new()
	gshape.size = Vector3(2.0, 1.2, 1.5)
	gcol.shape = gshape
	garden_area.add_child(gcol)

	# Glass door wall
	var glass_mat := _mat(Color(0.55, 0.72, 0.85, 0.45), 0.15)
	glass_mat.transparency = BaseMaterial3D.TRANSPARENCY_ALPHA
	var glass := CSGBox3D.new()
	glass.name = "Glass"
	glass.size = Vector3(0.08, 2.4, 2.2)
	glass.position = Vector3(10.9, 1.2, 0.8)
	glass.material = glass_mat
	glass.use_collision = true
	nb.add_child(glass)
	_box(nb, Vector3(0.2, 2.6, 0.15), Vector3(10.9, 1.3, -0.35), Color(0.5, 0.45, 0.4), "GlassFrameL")
	_box(nb, Vector3(0.2, 2.6, 0.15), Vector3(10.9, 1.3, 1.95), Color(0.5, 0.45, 0.4), "GlassFrameR")

	# Ember at glass
	var ember := Node3D.new()
	ember.name = "Ember"
	ember.position = Vector3(10.5, 0.35, 0.8)
	ember.set_script(load("res://scripts/ember.gd"))
	nb.add_child(ember)
	# Cat body placeholder (warm ochre / cream)
	var body := MeshInstance3D.new()
	body.name = "Body"
	var sphere := SphereMesh.new()
	sphere.radius = 0.22
	sphere.height = 0.38
	body.mesh = sphere
	var fur := _mat(Color(0.85, 0.68, 0.42), 0.9)
	body.material_override = fur
	body.scale = Vector3(1.1, 0.85, 1.4)
	ember.add_child(body)
	var head := MeshInstance3D.new()
	head.name = "Head"
	var hs := SphereMesh.new()
	hs.radius = 0.14
	hs.height = 0.28
	head.mesh = hs
	head.material_override = fur
	head.position = Vector3(0.0, 0.18, 0.22)
	ember.add_child(head)

	var ember_area := Area3D.new()
	ember_area.name = "EmberInteract"
	ember_area.collision_layer = 4
	ember_area.collision_mask = 0
	ember_area.position = Vector3(0.0, 0.3, 0.0)
	ember_area.set_script(load("res://scripts/interactable.gd"))
	ember_area.set("prompt_text", "[E] Free / adopt Ember")
	ember_area.set("interact_id", "ember")
	ember_area.set("one_shot", true)
	ember.add_child(ember_area)
	var ecol := CollisionShape3D.new()
	var eshape := BoxShape3D.new()
	eshape.size = Vector3(1.4, 1.4, 1.4)
	ecol.shape = eshape
	ember_area.add_child(ecol)

func _build_props() -> void:
	# Distant neon bleed (accent only)
	var neon := OmniLight3D.new()
	neon.name = "NeonBleed"
	neon.position = Vector3(14.0, 2.5, -8.0)
	neon.light_color = Color(0.4, 0.7, 1.0)
	neon.light_energy = 0.6
	neon.omni_range = 12.0
	add_child(neon)
