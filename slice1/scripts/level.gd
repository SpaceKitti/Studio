extends Node3D
## Milestone 0 layout: home balcony + blocked interior + jump gap + neighbor.
## Desk is OUTDOOR on home balcony. Interior = black volume. Golden-hour / neon dusk.

func _ready() -> void:
	add_to_group("level")
	_build_environment()
	_build_city_backdrop()
	_build_blocked_interior()
	_build_home_balcony()
	_build_neighbor_balcony()
	_build_fire_escape()
	_build_gap_markers()

func _mat(color: Color, rough: float = 0.8, emission: Color = Color(0, 0, 0, 0), e_energy: float = 0.0) -> StandardMaterial3D:
	var m := StandardMaterial3D.new()
	m.albedo_color = color
	m.roughness = rough
	m.metallic = 0.05
	if e_energy > 0.0:
		m.emission_enabled = true
		m.emission = emission
		m.emission_energy_multiplier = e_energy
	return m

func _box(parent: Node, size: Vector3, pos: Vector3, color: Color, name: String = "Box", rough: float = 0.85) -> CSGBox3D:
	var b := CSGBox3D.new()
	b.name = name
	b.size = size
	b.position = pos
	b.material = _mat(color, rough)
	b.use_collision = true
	parent.add_child(b)
	return b

func _build_environment() -> void:
	var env := WorldEnvironment.new()
	env.name = "WorldEnvironment"
	var e := Environment.new()
	e.background_mode = Environment.BG_COLOR
	# Neon dusk / golden hour — NOT grey noon
	e.background_color = Color(0.18, 0.12, 0.22)
	e.ambient_light_source = Environment.AMBIENT_SOURCE_COLOR
	e.ambient_light_color = Color(0.55, 0.35, 0.45)
	e.ambient_light_energy = 0.35
	e.tonemap_mode = Environment.TONE_MAPPER_FILMIC
	e.tonemap_exposure = 1.05
	e.glow_enabled = true
	e.glow_intensity = 0.35
	e.glow_bloom = 0.15
	e.ssao_enabled = true
	e.ssao_radius = 1.2
	e.ssao_intensity = 1.4
	env.environment = e
	add_child(env)

	# Key light — low golden sun
	var sun := DirectionalLight3D.new()
	sun.name = "KeyLight"
	sun.rotation_degrees = Vector3(-28, 55, 10)
	sun.light_color = Color(1.0, 0.62, 0.35)
	sun.light_energy = 1.35
	sun.shadow_enabled = true
	add_child(sun)

	# Neon trim fill
	var neon := OmniLight3D.new()
	neon.name = "NeonTrim"
	neon.position = Vector3(5.0, 2.2, -4.5)
	neon.light_color = Color(0.35, 0.85, 1.0)
	neon.light_energy = 2.2
	neon.omni_range = 14.0
	add_child(neon)

	var neon2 := OmniLight3D.new()
	neon2.name = "NeonMagenta"
	neon2.position = Vector3(10.0, 1.8, 4.0)
	neon2.light_color = Color(1.0, 0.25, 0.65)
	neon2.light_energy = 1.4
	neon2.omni_range = 10.0
	add_child(neon2)

func _build_city_backdrop() -> void:
	var bg := Node3D.new()
	bg.name = "CityBackdrop"
	add_child(bg)
	# Distant building slabs + street glow far below
	var colors := [
		Color(0.12, 0.1, 0.16),
		Color(0.16, 0.12, 0.2),
		Color(0.1, 0.14, 0.18),
	]
	for i in range(8):
		var h := 8.0 + float(i % 4) * 3.5
		var x := -6.0 + float(i) * 3.2
		var z := -14.0 - float(i % 3) * 2.0
		_box(bg, Vector3(2.4, h, 2.4), Vector3(x, -h * 0.5 - 2.0, z), colors[i % 3], "Tower%d" % i)
		# Neon window strip
		var strip := CSGBox3D.new()
		strip.size = Vector3(2.0, 0.15, 0.08)
		strip.position = Vector3(x, -2.0 - float(i % 5), z + 1.25)
		strip.material = _mat(Color(0.2, 0.9, 1.0), 0.4, Color(0.2, 0.9, 1.0), 2.5)
		strip.use_collision = false
		bg.add_child(strip)
	# Street plane far below
	_box(bg, Vector3(40, 0.2, 20), Vector3(4.0, -18.0, -8.0), Color(0.05, 0.05, 0.07), "Street")
	var street_glow := OmniLight3D.new()
	street_glow.position = Vector3(4.0, -12.0, -6.0)
	street_glow.light_color = Color(1.0, 0.45, 0.2)
	street_glow.light_energy = 3.0
	street_glow.omni_range = 25.0
	bg.add_child(street_glow)

func _build_blocked_interior() -> void:
	var interior := Node3D.new()
	interior.name = "BlockedInterior"
	add_child(interior)
	# Black volume west of home balcony — NOT enterable gameplay space
	var black := Color(0.02, 0.02, 0.03)
	_box(interior, Vector3(4.0, 3.0, 6.5), Vector3(-2.2, 1.4, 0.0), black, "BlackVolume")
	# Soft occlusion wall flush with door plane
	_box(interior, Vector3(0.2, 3.0, 6.5), Vector3(-0.15, 1.4, 0.0), Color(0.08, 0.07, 0.09), "InteriorWall")

func _build_home_balcony() -> void:
	var bal := Node3D.new()
	bal.name = "HomeBalcony"
	add_child(bal)
	var concrete := Color(0.42, 0.36, 0.40)
	var rail := Color(0.55, 0.5, 0.58)
	var rust := Color(0.55, 0.28, 0.18)

	# Floor x 0..4.2
	_box(bal, Vector3(4.2, 0.22, 6.0), Vector3(2.1, -0.11, 0.0), concrete, "Floor")
	_box(bal, Vector3(4.2, 0.85, 0.1), Vector3(2.1, 0.42, -3.0), rail, "RailS")
	_box(bal, Vector3(4.2, 0.85, 0.1), Vector3(2.1, 0.42, 3.0), rail, "RailN")
	# East rail with jump gap opening at z~0
	_box(bal, Vector3(0.1, 0.85, 1.8), Vector3(4.15, 0.42, -2.1), rail, "RailE_S")
	_box(bal, Vector3(0.1, 0.85, 1.8), Vector3(4.15, 0.42, 2.1), rail, "RailE_N")

	# Neon trim on rail
	var trim := CSGBox3D.new()
	trim.name = "RailNeon"
	trim.size = Vector3(4.0, 0.04, 0.06)
	trim.position = Vector3(2.1, 0.88, -3.0)
	trim.material = _mat(Color(0.3, 1.0, 0.95), 0.3, Color(0.3, 1.0, 0.95), 3.0)
	trim.use_collision = false
	bal.add_child(trim)

	# Sliding glass to OWN apartment (blocked black volume) — no Ember here
	var glass_mat := _mat(Color(0.35, 0.55, 0.7, 0.35), 0.1)
	glass_mat.transparency = BaseMaterial3D.TRANSPARENCY_ALPHA
	var glass := CSGBox3D.new()
	glass.name = "HomeSlidingGlass"
	glass.size = Vector3(0.06, 2.3, 1.8)
	glass.position = Vector3(0.05, 1.15, 0.0)
	glass.material = glass_mat
	glass.use_collision = true
	bal.add_child(glass)
	_box(bal, Vector3(0.12, 2.5, 0.12), Vector3(0.05, 1.25, -0.95), rust, "DoorFrameL")
	_box(bal, Vector3(0.12, 2.5, 0.12), Vector3(0.05, 1.25, 0.95), rust, "DoorFrameR")
	_box(bal, Vector3(0.12, 0.12, 2.0), Vector3(0.05, 2.35, 0.0), rust, "DoorFrameTop")

	# Blocked own-apartment entry (future M1). Cat door is on neighbor glass only.
	var home_door := Area3D.new()
	home_door.name = "HomeGlassInteract"
	home_door.position = Vector3(1.0, 1.0, 0.0)
	home_door.set_script(load("res://scripts/interactable.gd"))
	home_door.set("prompt_text", "[E] Own apartment (blocked — M1)")
	home_door.set("interact_id", "home_glass")
	home_door.set("one_shot", false)
	bal.add_child(home_door)
	home_door.add_to_group("interactable")
	home_door.collision_layer = 4
	var hdcol := CollisionShape3D.new()
	var hdshape := BoxShape3D.new()
	hdshape.size = Vector3(2.0, 2.4, 2.4)
	hdcol.shape = hdshape
	home_door.add_child(hdcol)

	# Outdoor engineering table (desk) — craft disabled
	var desk_root := Node3D.new()
	desk_root.name = "EngineeringTable"
	desk_root.position = Vector3(1.6, 0.0, 1.8)
	bal.add_child(desk_root)
	_box(desk_root, Vector3(1.6, 0.07, 0.8), Vector3(0, 0.78, 0), Color(0.35, 0.28, 0.22), "Top")
	_box(desk_root, Vector3(0.07, 0.78, 0.07), Vector3(-0.7, 0.39, -0.3), Color(0.25, 0.2, 0.18), "L1")
	_box(desk_root, Vector3(0.07, 0.78, 0.07), Vector3(0.7, 0.39, -0.3), Color(0.25, 0.2, 0.18), "L2")
	_box(desk_root, Vector3(0.07, 0.78, 0.07), Vector3(-0.7, 0.39, 0.3), Color(0.25, 0.2, 0.18), "L3")
	_box(desk_root, Vector3(0.07, 0.78, 0.07), Vector3(0.7, 0.39, 0.3), Color(0.25, 0.2, 0.18), "L4")
	_box(desk_root, Vector3(0.4, 0.1, 0.28), Vector3(0.35, 0.88, 0.05), Color(0.45, 0.5, 0.55), "Tools")
	# Neon underglow on desk
	var desk_neon := OmniLight3D.new()
	desk_neon.position = Vector3(0, 0.5, 0)
	desk_neon.light_color = Color(0.4, 0.9, 1.0)
	desk_neon.light_energy = 0.8
	desk_neon.omni_range = 3.0
	desk_root.add_child(desk_neon)

	var desk := Area3D.new()
	desk.name = "DeskInteract"
	desk.position = Vector3(0, 0.9, 0.55)
	desk.set_script(load("res://scripts/interactable.gd"))
	desk.set("prompt_text", "[E] Engineering table (craft locked)")
	desk.set("interact_id", "desk")
	desk_root.add_child(desk)
	desk.add_to_group("interactable")
	desk.collision_layer = 4
	var desk_col := CollisionShape3D.new()
	var desk_shape := BoxShape3D.new()
	desk_shape.size = Vector3(1.8, 1.4, 1.4)
	desk_col.shape = desk_shape
	desk.add_child(desk_col)

	# 2–3 herb pots (alive)
	var pot_c := Color(0.55, 0.32, 0.22)
	var herb_c := Color(0.25, 0.55, 0.28)
	for i in range(3):
		var hx := 1.0 + float(i) * 0.85
		_box(bal, Vector3(0.4, 0.32, 0.4), Vector3(hx, 0.18, -2.3), pot_c, "HerbPot%d" % i)
		_box(bal, Vector3(0.32, 0.22, 0.32), Vector3(hx, 0.42, -2.3), herb_c, "HerbLeaf%d" % i)
	var herbs := Area3D.new()
	herbs.name = "HerbsInteract"
	herbs.position = Vector3(1.85, 0.5, -2.1)
	herbs.set_script(load("res://scripts/interactable.gd"))
	herbs.set("prompt_text", "[E] Check herbs")
	herbs.set("interact_id", "herbs")
	bal.add_child(herbs)
	herbs.add_to_group("interactable")
	herbs.collision_layer = 4
	var hcol := CollisionShape3D.new()
	var hshape := BoxShape3D.new()
	hshape.size = Vector3(2.6, 1.2, 1.0)
	hcol.shape = hshape
	herbs.add_child(hcol)

	# Empty plantable pot for plant → water → harvest path
	_box(bal, Vector3(0.5, 0.35, 0.5), Vector3(1.9, 0.2, -1.85), Color(0.4, 0.28, 0.2), "EmptyPot")
	var plant := Area3D.new()
	plant.name = "HomePlantSpot"
	plant.position = Vector3(1.9, 0.2, -1.85)
	plant.set_script(load("res://scripts/plant_spot.gd"))
	plant.set("spot_name", "home pot")
	plant.set("initial_state", 0) # EMPTY
	plant.set("accept_seed_ids", PackedStringArray(["tomato_seed", "potato_seed"]))
	bal.add_child(plant)
	plant.add_to_group("interactable")
	# Soft lip near empty pot so plant path stays on slab (away from east gap)
	_box(bal, Vector3(1.2, 0.14, 0.12), Vector3(1.9, 0.05, -2.35), Color(0.5, 0.4, 0.35), "PotSafetyLip")


func _build_neighbor_balcony() -> void:
	var nb := Node3D.new()
	nb.name = "NeighborBalcony"
	add_child(nb)
	var concrete := Color(0.38, 0.34, 0.36)
	var rail := Color(0.5, 0.45, 0.52)

	# Gap ~2.0m between 4.2 and 6.4 — jumpable
	_box(nb, Vector3(4.6, 0.22, 6.0), Vector3(8.7, -0.11, 0.0), concrete, "Floor")
	_box(nb, Vector3(4.6, 0.85, 0.1), Vector3(8.7, 0.42, -3.0), rail, "RailS")
	_box(nb, Vector3(4.6, 0.85, 0.1), Vector3(8.7, 0.42, 3.0), rail, "RailN")
	_box(nb, Vector3(0.1, 0.85, 1.8), Vector3(6.45, 0.42, -2.1), rail, "RailW_S")
	_box(nb, Vector3(0.1, 0.85, 1.8), Vector3(6.45, 0.42, 2.1), rail, "RailW_N")
	_box(nb, Vector3(0.1, 0.85, 6.0), Vector3(10.95, 0.42, 0.0), rail, "RailE")
	# Neighbor building face
	_box(nb, Vector3(0.3, 3.2, 6.2), Vector3(11.2, 1.5, 0.0), Color(0.22, 0.18, 0.24), "NeighborWall")

	# Neighbor sliding glass — Ember (neighbor's cat) behind it; adopt HERE
	var nglass_mat := _mat(Color(0.35, 0.55, 0.7, 0.35), 0.1)
	nglass_mat.transparency = BaseMaterial3D.TRANSPARENCY_ALPHA
	var nglass := CSGBox3D.new()
	nglass.name = "NeighborSlidingGlass"
	nglass.size = Vector3(0.06, 2.3, 1.8)
	nglass.position = Vector3(11.05, 1.15, 0.0)
	nglass.material = nglass_mat
	nglass.use_collision = true
	nb.add_child(nglass)
	var nrust := Color(0.55, 0.28, 0.18)
	_box(nb, Vector3(0.12, 2.5, 0.12), Vector3(11.05, 1.25, -0.95), nrust, "NDoorFrameL")
	_box(nb, Vector3(0.12, 2.5, 0.12), Vector3(11.05, 1.25, 0.95), nrust, "NDoorFrameR")
	_box(nb, Vector3(0.12, 0.12, 2.0), Vector3(11.05, 2.35, 0.0), nrust, "NDoorFrameTop")

	var door := Area3D.new()
	door.name = "DoorInteract"
	# Balcony side of neighbor glass — cat door only
	door.position = Vector3(10.2, 1.0, 0.0)
	door.set_script(load("res://scripts/interactable.gd"))
	door.set("prompt_text", "[E] Open sliding glass (cat pawing)")
	door.set("interact_id", "door")
	door.set("one_shot", true)
	nb.add_child(door)
	door.add_to_group("interactable")
	door.collision_layer = 4
	var dcol := CollisionShape3D.new()
	var dshape := BoxShape3D.new()
	dshape.size = Vector3(2.0, 2.4, 2.4)
	dcol.shape = dshape
	door.add_child(dcol)

	# Ember behind neighbor glass (inside neighbor apartment)
	var ember := CharacterBody3D.new()
	ember.name = "Ember"
	ember.position = Vector3(11.45, 0.15, 0.15)
	ember.collision_layer = 0
	ember.collision_mask = 1
	ember.set_script(load("res://scripts/ember.gd"))
	ember.add_to_group("ember")
	nb.add_child(ember)
	var ebody := MeshInstance3D.new()
	var es := SphereMesh.new()
	es.radius = 0.2
	es.height = 0.36
	ebody.mesh = es
	ebody.material_override = _mat(Color(0.9, 0.7, 0.4), 0.9)
	ebody.scale = Vector3(1.1, 0.85, 1.35)
	ember.add_child(ebody)
	var ehead := MeshInstance3D.new()
	var hs := SphereMesh.new()
	hs.radius = 0.13
	hs.height = 0.26
	ehead.mesh = hs
	ehead.material_override = _mat(Color(0.9, 0.7, 0.4), 0.9)
	ehead.position = Vector3(-0.05, 0.16, 0.18)
	ember.add_child(ehead)
	var ecol := CollisionShape3D.new()
	var ecaps := CapsuleShape3D.new()
	ecaps.radius = 0.18
	ecaps.height = 0.4
	ecol.shape = ecaps
	ecol.position = Vector3(0, 0.2, 0)
	ember.add_child(ecol)

	# Paw decal cue on neighbor glass
	_box(nb, Vector3(0.08, 0.08, 0.08), Vector3(10.98, 0.7, 0.25), Color(0.85, 0.65, 0.4), "PawMark")


	# Potato + tomato mid-growth pots
	var soil := Color(0.28, 0.2, 0.14)
	_box(nb, Vector3(0.55, 0.35, 0.55), Vector3(8.2, 0.2, -2.0), soil, "PotatoPot")
	var potato := Area3D.new()
	potato.name = "PotatoPlant"
	potato.position = Vector3(8.2, 0.2, -2.0)
	potato.set_script(load("res://scripts/plant_spot.gd"))
	potato.set("spot_name", "potato pot")
	potato.set("initial_state", 2) # GROWING
	potato.set("crop_id", "potato")
	potato.set("seed_id", "potato_seed")
	nb.add_child(potato)
	potato.add_to_group("interactable")

	_box(nb, Vector3(0.55, 0.35, 0.55), Vector3(9.2, 0.2, -2.0), soil, "TomatoPot")
	var tomato := Area3D.new()
	tomato.name = "TomatoPlant"
	tomato.position = Vector3(9.2, 0.2, -2.0)
	tomato.set_script(load("res://scripts/plant_spot.gd"))
	tomato.set("spot_name", "tomato pot")
	tomato.set("initial_state", 2) # GROWING
	tomato.set("crop_id", "tomato_fresh")
	tomato.set("seed_id", "tomato_seed")
	nb.add_child(tomato)
	tomato.add_to_group("interactable")

	# Note + empty bowl
	_box(nb, Vector3(0.35, 0.05, 0.25), Vector3(9.8, 0.85, 1.5), Color(0.85, 0.8, 0.7), "Note")
	_box(nb, Vector3(0.3, 0.08, 0.3), Vector3(9.5, 0.08, 1.2), Color(0.6, 0.55, 0.5), "FoodBowl")
	var note := Area3D.new()
	note.name = "NoteInteract"
	note.position = Vector3(9.8, 0.9, 1.5)
	note.set_script(load("res://scripts/interactable.gd"))
	note.set("prompt_text", "[E] Read note")
	note.set("interact_id", "note")
	nb.add_child(note)
	note.add_to_group("interactable")
	note.collision_layer = 4
	var ncol := CollisionShape3D.new()
	var nshape := BoxShape3D.new()
	nshape.size = Vector3(1.0, 1.0, 1.0)
	ncol.shape = nshape
	note.add_child(ncol)

	# 3 searchable containers
	_add_container(nb, "PlanterBox", Vector3(7.4, 0.0, 1.8), Color(0.35, 0.45, 0.3), Vector3(1.1, 0.45, 0.7),
		PackedStringArray(["potato_seed", "tomato_seed"]), PackedInt32Array([2, 1]))
	_add_container(nb, "PlasticDrawer", Vector3(8.8, 0.0, 2.0), Color(0.55, 0.58, 0.62), Vector3(0.7, 0.7, 0.55),
		PackedStringArray(["water_bottle", "tuna_can", "flour_sr"]), PackedInt32Array([2, 1, 1]))
	_add_container(nb, "RustedToolbox", Vector3(10.2, 0.0, 1.6), Color(0.55, 0.32, 0.18), Vector3(0.7, 0.4, 0.4),
		PackedStringArray(["salt", "water_bottle"]), PackedInt32Array([1, 1]))

func _add_container(parent: Node, cname: String, pos: Vector3, color: Color, size: Vector3, ids: PackedStringArray, counts: PackedInt32Array) -> void:
	_box(parent, size, pos + Vector3(0, size.y * 0.5, 0), color, cname + "Mesh")
	var area := Area3D.new()
	area.name = cname
	area.position = pos + Vector3(0, size.y * 0.5, 0)
	area.set_script(load("res://scripts/container.gd"))
	area.set("container_name", cname.replace("Box", " box").replace("Drawer", " drawer").replace("Toolbox", " toolbox"))
	# prettier names
	match cname:
		"PlanterBox":
			area.set("container_name", "planter box")
		"PlasticDrawer":
			area.set("container_name", "plastic drawer")
		"RustedToolbox":
			area.set("container_name", "rusted toolbox")
	area.set("loot_ids", ids)
	area.set("loot_counts", counts)
	parent.add_child(area)
	area.add_to_group("interactable")

func _build_fire_escape() -> void:
	var fe := Node3D.new()
	fe.name = "FireEscape"
	fe.position = Vector3(5.3, 0.0, 3.4)
	add_child(fe)
	var metal := Color(0.35, 0.32, 0.3)
	# Visible ladder/platform — NOT usable as open world
	_box(fe, Vector3(1.2, 0.1, 1.6), Vector3(0, 0.05, 0), metal, "Landing")
	for i in range(6):
		_box(fe, Vector3(0.08, 0.5, 0.08), Vector3(-0.4, -0.4 - float(i) * 0.55, 0.5), metal, "RungL%d" % i)
		_box(fe, Vector3(0.08, 0.5, 0.08), Vector3(0.4, -0.4 - float(i) * 0.55, 0.5), metal, "RungR%d" % i)
		_box(fe, Vector3(0.9, 0.06, 0.06), Vector3(0, -0.2 - float(i) * 0.55, 0.5), metal, "Step%d" % i)
	var area := Area3D.new()
	area.name = "FireEscapeInteract"
	area.position = Vector3(0, 0.8, 0)
	area.set_script(load("res://scripts/interactable.gd"))
	area.set("prompt_text", "[E] Fire escape (locked)")
	area.set("interact_id", "fire_escape")
	fe.add_child(area)
	area.add_to_group("interactable")
	area.collision_layer = 4
	var col := CollisionShape3D.new()
	var shape := BoxShape3D.new()
	shape.size = Vector3(1.6, 2.0, 1.8)
	col.shape = shape
	area.add_child(col)

func _build_gap_markers() -> void:
	# Subtle ledge lips so jump reads clearly
	_box(self, Vector3(0.25, 0.12, 1.6), Vector3(4.25, 0.02, 0.0), Color(0.5, 0.4, 0.35), "GapLipHome")
	_box(self, Vector3(0.25, 0.12, 1.6), Vector3(6.35, 0.02, 0.0), Color(0.5, 0.4, 0.35), "GapLipNeighbor")

func on_door_opened(actor: Node) -> void:
	var ember := get_tree().get_first_node_in_group("ember")
	if ember and ember.has_method("adopt"):
		ember.call("adopt", actor)
