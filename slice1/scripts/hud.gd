extends CanvasLayer
## Toast + interact prompt + help strip.

@onready var prompt_label: Label = $Root/Prompt
@onready var toast_label: Label = $Root/Toast
@onready var help_label: Label = $Root/Help

var _toast_timer: float = 0.0

func _ready() -> void:
	add_to_group("hud")
	prompt_label.text = ""
	toast_label.text = ""
	help_label.text = "WASD move · Space jump · Mouse look · E interact · Esc mouse"

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
