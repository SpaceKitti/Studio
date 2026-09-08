extends CanvasLayer
## Prompt, toast, inventory kg panel, desk recipe list, win checklist.

@onready var prompt_label: Label = $Root/Prompt
@onready var toast_label: Label = $Root/Toast
@onready var help_label: Label = $Root/Help
@onready var inv_panel: PanelContainer = $Root/InventoryPanel
@onready var inv_text: Label = $Root/InventoryPanel/Margin/InvText
@onready var desk_panel: PanelContainer = $Root/DeskPanel
@onready var desk_text: Label = $Root/DeskPanel/Margin/VBox/DeskText
@onready var craft_btn: Button = $Root/DeskPanel/Margin/VBox/CraftButton
@onready var win_label: Label = $Root/WinChecklist

var _toast_timer: float = 0.0
var _inv_open: bool = false
var _desk_open: bool = false
var _crosshair: Label = null
var _bag_hint: Label = null
var _inv_style_applied: bool = false
var _inv_ui_ready: bool = false
var _inv_header: Label = null
var _inv_list: VBoxContainer = null
var _hands_row: HBoxContainer = null
var _hands_label: Label = null
var _stow_btn: Button = null
var _icon_cache: Dictionary = {}

const _PLACEHOLDER_COLORS := {
	"herb_basil": Color(0.25, 0.75, 0.35),
	"herb_mint": Color(0.35, 0.85, 0.7),
}

func _ready() -> void:
	process_mode = Node.PROCESS_MODE_ALWAYS
	add_to_group("hud")
	prompt_label.text = ""
	prompt_label.visible = false
	prompt_label.add_theme_font_size_override("font_size", 28)
	prompt_label.horizontal_alignment = HORIZONTAL_ALIGNMENT_CENTER
	prompt_label.modulate = Color(1.0, 1.0, 0.55, 1.0)
	# Center-bottom prompt so E is unmistakable when looking at an interactable.
	prompt_label.set_anchors_preset(Control.PRESET_CENTER_BOTTOM)
	prompt_label.offset_left = -320.0
	prompt_label.offset_right = 320.0
	prompt_label.offset_top = -96.0
	prompt_label.offset_bottom = -40.0
	toast_label.text = ""
	help_label.text = "WASD | Space jump | Mouse look | E interact | T inventory (or Tab/I) | G Ember gift | Esc mouse"
	inv_panel.visible = false
	desk_panel.visible = false
	craft_btn.disabled = true
	craft_btn.text = "Craft (locked — M1)"
	craft_btn.pressed.connect(_on_craft_pressed)
	var close_btn: Button = $Root/DeskPanel/Margin/VBox/CloseButton
	close_btn.pressed.connect(_on_desk_close)
	Inventory.changed.connect(_refresh_inv)
	Inventory.rejected.connect(func(r: String): toast(r))
	GameState.win_updated.connect(_refresh_win)
	GameState.toast_request.connect(func(t: String): toast(t))
	_apply_inv_style()
	_ensure_inv_ui()
	_refresh_win()
	_ensure_crosshair()
	_ensure_bag_hint()
	# Unmistakable build stamp — always visible top-right
	var build_stamp := Label.new()
	build_stamp.name = "BuildStamp"
	build_stamp.text = "BUILD 4c4df2b | press T"
	build_stamp.add_theme_font_size_override("font_size", 18)
	build_stamp.add_theme_color_override("font_color", Color(1.0, 1.0, 0.2, 1.0))
	build_stamp.mouse_filter = Control.MOUSE_FILTER_IGNORE
	build_stamp.set_anchors_preset(Control.PRESET_TOP_RIGHT)
	build_stamp.grow_horizontal = Control.GROW_DIRECTION_BEGIN
	build_stamp.offset_left = -420.0
	build_stamp.offset_right = -12.0
	build_stamp.offset_top = 8.0
	build_stamp.offset_bottom = 36.0
	build_stamp.horizontal_alignment = HORIZONTAL_ALIGNMENT_RIGHT
	build_stamp.z_index = 200
	$Root.add_child(build_stamp)
	_refresh_inv()

func _process(delta: float) -> void:
	if _toast_timer > 0.0:
		_toast_timer -= delta
		if _toast_timer <= 0.0:
			toast_label.text = ""

func _unhandled_input(event: InputEvent) -> void:
	if event is InputEventKey and event.pressed and not event.echo:
		var codes: Array[int] = [event.keycode, event.physical_keycode]
		for c in codes:
			if c == KEY_TAB or c == KEY_I or c == KEY_T:
				toggle_inventory()
				get_viewport().set_input_as_handled()
				return

func set_prompt(text: String) -> void:
	if text.strip_edges() == "":
		prompt_label.text = ""
		prompt_label.visible = false
		return
	var shown := text
	if not (shown.begins_with("[E]") or shown.begins_with("E ") or shown.begins_with("[blocked]")):
		shown = "[E] " + shown
	prompt_label.text = shown
	prompt_label.visible = true

func toast(text: String, duration: float = 3.5) -> void:
	toast_label.text = text
	_toast_timer = duration

func _apply_inv_style() -> void:
	if _inv_style_applied or inv_panel == null:
		return
	var sb := StyleBoxFlat.new()
	# Opaque Panel Face (#1A2233)
	sb.bg_color = Color(0.102, 0.133, 0.2, 1.0)
	sb.border_color = Color(1.0, 0.85, 0.35)
	sb.set_border_width_all(2)
	sb.set_corner_radius_all(6)
	sb.content_margin_left = 8.0
	sb.content_margin_right = 8.0
	sb.content_margin_top = 8.0
	sb.content_margin_bottom = 8.0
	inv_panel.add_theme_stylebox_override("panel", sb)
	inv_panel.z_index = 100
	# Wider/taller for icon rows + buttons
	inv_panel.offset_left = -300.0
	inv_panel.offset_right = 300.0
	inv_panel.offset_top = -260.0
	inv_panel.offset_bottom = 260.0
	_inv_style_applied = true

func _ensure_inv_ui() -> void:
	if _inv_ui_ready:
		return
	var margin: MarginContainer = $Root/InventoryPanel/Margin
	# MarginContainer only positions one child — detach legacy InvText.
	if inv_text.get_parent() == margin:
		margin.remove_child(inv_text)
	inv_text.visible = false
	var root_vbox := VBoxContainer.new()
	root_vbox.name = "InvRoot"
	root_vbox.size_flags_horizontal = Control.SIZE_EXPAND_FILL
	root_vbox.size_flags_vertical = Control.SIZE_EXPAND_FILL
	root_vbox.add_theme_constant_override("separation", 8)
	margin.add_child(root_vbox)

	var title := Label.new()
	title.text = "INVENTORY"
	title.add_theme_font_size_override("font_size", 18)
	title.add_theme_color_override("font_color", Color(1.0, 0.9, 0.45))
	root_vbox.add_child(title)

	_inv_header = Label.new()
	_inv_header.name = "InvHeader"
	_inv_header.add_theme_font_size_override("font_size", 15)
	_inv_header.add_theme_color_override("font_color", Color(0.9, 0.95, 1.0))
	root_vbox.add_child(_inv_header)

	var scroll := ScrollContainer.new()
	scroll.name = "InvScroll"
	scroll.size_flags_vertical = Control.SIZE_EXPAND_FILL
	scroll.size_flags_horizontal = Control.SIZE_EXPAND_FILL
	scroll.custom_minimum_size = Vector2(0, 280)
	root_vbox.add_child(scroll)

	_inv_list = VBoxContainer.new()
	_inv_list.name = "InvList"
	_inv_list.size_flags_horizontal = Control.SIZE_EXPAND_FILL
	_inv_list.add_theme_constant_override("separation", 6)
	scroll.add_child(_inv_list)

	var hands_title := Label.new()
	hands_title.text = "HANDS"
	hands_title.add_theme_font_size_override("font_size", 14)
	hands_title.add_theme_color_override("font_color", Color(1.0, 0.85, 0.55))
	root_vbox.add_child(hands_title)

	_hands_row = HBoxContainer.new()
	_hands_row.name = "HandsRow"
	_hands_row.add_theme_constant_override("separation", 8)
	root_vbox.add_child(_hands_row)

	_hands_label = Label.new()
	_hands_label.size_flags_horizontal = Control.SIZE_EXPAND_FILL
	_hands_label.add_theme_font_size_override("font_size", 14)
	_hands_label.add_theme_color_override("font_color", Color(0.9, 0.95, 1.0))
	_hands_row.add_child(_hands_label)

	_stow_btn = Button.new()
	_stow_btn.text = "Stow"
	_stow_btn.visible = false
	_stow_btn.pressed.connect(_on_stow_pressed)
	_hands_row.add_child(_stow_btn)

	_inv_ui_ready = true

func toggle_inventory() -> void:
	_apply_inv_style()
	_ensure_inv_ui()
	_inv_open = not _inv_open
	inv_panel.visible = _inv_open
	if _inv_open:
		_desk_open = false
		desk_panel.visible = false
		GameState.mark_inventory_open()
		_refresh_inv()
		inv_panel.z_index = 100
		inv_panel.move_to_front()
		toast("Inventory — Tab/I/T to close")
		Input.mouse_mode = Input.MOUSE_MODE_VISIBLE
	else:
		Input.mouse_mode = Input.MOUSE_MODE_CAPTURED

func show_desk() -> void:
	_desk_open = true
	desk_panel.visible = true
	_inv_open = false
	inv_panel.visible = false
	_refresh_desk()
	Input.mouse_mode = Input.MOUSE_MODE_VISIBLE

func _refresh_inv() -> void:
	_ensure_inv_ui()
	if _inv_header == null or _inv_list == null:
		return
	_inv_header.text = "Backpack %.1f / %.1f kg" % [Inventory.weight_kg(), Inventory.BACKPACK_CAP_KG]
	# Rebuild stack rows
	while _inv_list.get_child_count() > 0:
		var child: Node = _inv_list.get_child(0)
		_inv_list.remove_child(child)
		child.free()
	if Inventory.backpack.is_empty():
		var empty := Label.new()
		empty.text = "(empty)"
		empty.add_theme_color_override("font_color", Color(0.7, 0.75, 0.85))
		_inv_list.add_child(empty)
	else:
		var ids: Array = Inventory.backpack.keys()
		ids.sort()
		for id in ids:
			_inv_list.add_child(_make_stack_row(str(id), int(Inventory.backpack[id])))
	# Hands
	if Inventory.hands_occupied:
		var item = ItemDB.get_item(Inventory.hands_item_id)
		var nm: String = Inventory.hands_item_id
		var w: float = 0.0
		if item:
			nm = str(item.display_name)
			w = float(item.weight_kg) * float(Inventory.hands_count)
		_hands_label.text = "%s x%d  (%.2f kg)" % [nm, Inventory.hands_count, w]
		var can_stow := Inventory.can_add(Inventory.hands_item_id, Inventory.hands_count)
		_stow_btn.visible = can_stow
		_stow_btn.disabled = not can_stow
	else:
		_hands_label.text = "Hands: free"
		_stow_btn.visible = false
	# Keep legacy InvText in sync for debug/fallback
	var lines := Inventory.summary_lines()
	inv_text.text = "INVENTORY\n" + "\n".join(lines)

func _make_stack_row(id: String, n: int) -> Control:
	var item = ItemDB.get_item(id)
	var nm: String = id
	var unit_w: float = 0.0
	if item:
		nm = str(item.display_name)
		unit_w = float(item.weight_kg)
	var row := HBoxContainer.new()
	row.add_theme_constant_override("separation", 8)
	row.custom_minimum_size = Vector2(0, 48)

	var icon_wrap := Control.new()
	icon_wrap.custom_minimum_size = Vector2(40, 40)
	row.add_child(icon_wrap)

	var tex: Texture2D = _texture_for_item(id)
	if tex != null:
		var tr := TextureRect.new()
		tr.texture = tex
		tr.expand_mode = TextureRect.EXPAND_IGNORE_SIZE
		tr.stretch_mode = TextureRect.STRETCH_KEEP_ASPECT_CENTERED
		tr.set_anchors_and_offsets_preset(Control.PRESET_FULL_RECT)
		icon_wrap.add_child(tr)
	else:
		var block := ColorRect.new()
		block.color = _PLACEHOLDER_COLORS.get(id, Color(0.45, 0.35, 0.55))
		block.set_anchors_and_offsets_preset(Control.PRESET_FULL_RECT)
		icon_wrap.add_child(block)
		var tip := Label.new()
		tip.text = nm.substr(0, 1).to_upper()
		tip.horizontal_alignment = HORIZONTAL_ALIGNMENT_CENTER
		tip.vertical_alignment = VERTICAL_ALIGNMENT_CENTER
		tip.set_anchors_and_offsets_preset(Control.PRESET_FULL_RECT)
		tip.add_theme_font_size_override("font_size", 16)
		icon_wrap.add_child(tip)

	var info := Label.new()
	info.size_flags_horizontal = Control.SIZE_EXPAND_FILL
	info.text = "%s\nx%d  ·  %.2f kg" % [nm, n, unit_w * float(n)]
	info.add_theme_font_size_override("font_size", 13)
	info.add_theme_color_override("font_color", Color(0.92, 0.95, 1.0))
	row.add_child(info)

	var half_btn := Button.new()
	half_btn.text = "Half"
	half_btn.disabled = n < 2
	half_btn.pressed.connect(func(): _on_half_pressed(id))
	row.add_child(half_btn)

	var take_btn := Button.new()
	take_btn.text = "Take 1"
	take_btn.disabled = n < 1
	take_btn.pressed.connect(func(): _on_take1_pressed(id))
	row.add_child(take_btn)

	return row

func _texture_for_item(id: String) -> Texture2D:
	if _icon_cache.has(id):
		return _icon_cache[id]
	var path := "res://assets/icons/%s_256.png" % id
	var tex: Texture2D = null
	if ResourceLoader.exists(path):
		tex = load(path) as Texture2D
	_icon_cache[id] = tex
	return tex

func _on_half_pressed(id: String) -> void:
	var msg := Inventory.half_to_hands(id)
	toast(msg)

func _on_take1_pressed(id: String) -> void:
	var msg := Inventory.take_to_hands(id, 1)
	toast(msg)

func _on_stow_pressed() -> void:
	var msg := Inventory.try_stow_hands()
	toast(msg)

func _refresh_desk() -> void:
	var lines: PackedStringArray = PackedStringArray()
	lines.append("ENGINEERING TABLE — recipes (data only)")
	lines.append("")
	for r in GameState.recipes:
		var recipe = r
		lines.append("%s  [%s]" % [recipe.display_name, recipe.id])
		lines.append("  station: %s" % recipe.station)
		for i in range(recipe.piece_ids.size()):
			var pid: String = recipe.piece_ids[i]
			var n := 1
			if i < recipe.piece_counts.size():
				n = int(recipe.piece_counts[i])
			lines.append("  - %s x%d" % [pid, n])
		lines.append("  → %s" % recipe.result_id)
		lines.append("")
	lines.append("Hardware store raid later feeds these pieces.")
	desk_text.text = "\n".join(lines)

func _on_craft_pressed() -> void:
	toast("Craft disabled in Milestone 0.")

func _on_desk_close() -> void:
	_desk_open = false
	desk_panel.visible = false
	Input.mouse_mode = Input.MOUSE_MODE_CAPTURED

func _refresh_win() -> void:
	var lines := GameState.checklist()
	var header := "M0 WIN"
	if GameState.win_met():
		header = "M0 WIN — COMPLETE"
	win_label.text = header + "\n" + "\n".join(lines)

func _ensure_crosshair() -> void:
	if _crosshair != null and is_instance_valid(_crosshair):
		return
	var root := $Root
	_crosshair = Label.new()
	_crosshair.name = "Crosshair"
	_crosshair.text = "+"
	_crosshair.horizontal_alignment = HORIZONTAL_ALIGNMENT_CENTER
	_crosshair.vertical_alignment = VERTICAL_ALIGNMENT_CENTER
	_crosshair.add_theme_font_size_override("font_size", 22)
	_crosshair.modulate = Color(1.0, 1.0, 1.0, 0.85)
	_crosshair.mouse_filter = Control.MOUSE_FILTER_IGNORE
	_crosshair.set_anchors_preset(Control.PRESET_CENTER)
	_crosshair.offset_left = -12.0
	_crosshair.offset_right = 12.0
	_crosshair.offset_top = -12.0
	_crosshair.offset_bottom = 12.0
	root.add_child(_crosshair)

func _ensure_bag_hint() -> void:
	if _bag_hint != null and is_instance_valid(_bag_hint):
		return
	var root := $Root
	_bag_hint = Label.new()
	_bag_hint.name = "BagHint"
	_bag_hint.text = "T: bag"
	_bag_hint.add_theme_font_size_override("font_size", 14)
	_bag_hint.add_theme_color_override("font_color", Color(1.0, 0.85, 0.35, 0.9))
	_bag_hint.mouse_filter = Control.MOUSE_FILTER_IGNORE
	_bag_hint.set_anchors_preset(Control.PRESET_BOTTOM_RIGHT)
	_bag_hint.offset_left = -96.0
	_bag_hint.offset_right = -16.0
	_bag_hint.offset_top = -36.0
	_bag_hint.offset_bottom = -12.0
	_bag_hint.horizontal_alignment = HORIZONTAL_ALIGNMENT_RIGHT
	root.add_child(_bag_hint)
