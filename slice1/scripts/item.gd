extends Resource
class_name Item
## Inventory item with real weight in kg.

@export var id: String = ""
@export var display_name: String = ""
@export var weight_kg: float = 0.1
@export var stack_max: int = 10
@export var tags: PackedStringArray = PackedStringArray()
