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

func _ready() -> void:
	add_to_group("hud")
	prompt_label.text = ""
	toast_label.text = ""
	help_label.text = "WASD · Space jump · Mouse look · E interact · Tab inventory · G Ember gift · Esc mouse"
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

func _process(delta: float) -> void:
	if _toast_timer > 0.0:
		_toast_timer -= delta
		if _toast_timer <= 0.0:
			toast_label.text = ""

func set_prompt(text: String) -> void:
	prompt_label.text = text

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
