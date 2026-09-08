extends Resource
class_name Recipe
## Crafting recipe data (M0: visible, craft disabled).

@export var id: String = ""
@export var display_name: String = ""
@export var station: String = "engineering_desk"
@export var result_id: String = ""
@export var piece_ids: PackedStringArray = PackedStringArray()
@export var piece_counts: PackedInt32Array = PackedInt32Array()
