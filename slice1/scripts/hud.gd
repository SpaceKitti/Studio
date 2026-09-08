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

func _ready() -> void:
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
	help_label.text = "WASD | Space jump | Mouse look | E interact | Tab/I/T inventory | G Ember gift | Esc mouse"
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
	_refresh_win()
	_ensure_crosshair()

func _process(delta: float) -> void:
	if _toast_timer > 0.0:
		_toast_timer -= delta
		if _toast_timer <= 0.0:
			toast_label.text = ""

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

func toggle_inventory() -> void:
	_inv_open = not _inv_open
	inv_panel.visible = _inv_open
	if _inv_open:
		_desk_open = false
		desk_panel.visible = false
		GameState.mark_inventory_open()
		_refresh_inv()
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
	if not _inv_open:
		return
	var lines := Inventory.summary_lines()
	inv_text.text = "INVENTORY\n" + "\n".join(lines)

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
