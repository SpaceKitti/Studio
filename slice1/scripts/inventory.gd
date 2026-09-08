extends Node
## Autoload: backpack kg inventory + hands_occupied.

signal changed
signal rejected(reason: String)

const BACKPACK_CAP_KG := 25.0

var backpack: Dictionary = {}
var hands_occupied: bool = false
var hands_item_id: String = ""
var hands_count: int = 0

func weight_kg() -> float:
	var total: float = 0.0
	for id in backpack:
		var item = ItemDB.get_item(id)
		if item:
			total += float(item.weight_kg) * float(backpack[id])
	return total

func count(id: String) -> int:
	return int(backpack.get(id, 0))

func can_add(id: String, amount: int = 1) -> bool:
	var item = ItemDB.get_item(id)
	if item == null:
		return false
	var add_w: float = float(item.weight_kg) * float(amount)
	return weight_kg() + add_w <= BACKPACK_CAP_KG + 0.0001

func add(id: String, amount: int = 1) -> bool:
	var item = ItemDB.get_item(id)
	if item == null:
		rejected.emit("Unknown item.")
		return false
	if not can_add(id, amount):
		if not hands_occupied and amount == 1:
			hands_occupied = true
			hands_item_id = id
			hands_count = 1
			changed.emit()
			rejected.emit("Backpack full (%.1f/%.1f kg) — took in hands." % [weight_kg(), BACKPACK_CAP_KG])
			return true
		rejected.emit("Too heavy. Backpack %.1f / %.1f kg." % [weight_kg(), BACKPACK_CAP_KG])
		return false
	var cur: int = count(id)
	if cur + amount > int(item.stack_max):
		rejected.emit("Stack full for %s." % str(item.display_name))
		return false
	backpack[id] = cur + amount
	changed.emit()
	return true

func remove(id: String, amount: int = 1) -> bool:
	var cur: int = count(id)
	if cur < amount:
		return false
	cur -= amount
	if cur <= 0:
		backpack.erase(id)
	else:
		backpack[id] = cur
	changed.emit()
	return true

func clear_hands() -> void:
	hands_occupied = false
	hands_item_id = ""
	hands_count = 0
	changed.emit()

func try_stow_hands() -> String:
	if not hands_occupied:
		return "Hands empty."
	var hid: String = hands_item_id
	var hcount: int = hands_count
	if add(hid, hcount):
		var item = ItemDB.get_item(hid)
		var nm: String = hid
		if item:
			nm = str(item.display_name)
		clear_hands()
		return "Stowed %s in backpack." % nm
	return "Cannot stow — backpack too heavy."

func summary_lines() -> PackedStringArray:
	var lines: PackedStringArray = PackedStringArray()
	lines.append("Backpack: %.1f / %.1f kg" % [weight_kg(), BACKPACK_CAP_KG])
	if backpack.is_empty():
		lines.append("  (empty)")
	else:
		for id in backpack.keys():
			var item = ItemDB.get_item(id)
			var n: int = int(backpack[id])
			var nm: String = id
			var w: float = 0.0
			if item:
				nm = str(item.display_name)
				w = float(item.weight_kg) * float(n)
			lines.append("  %s x%d  (%.2f kg)" % [nm, n, w])
	if hands_occupied:
		var item2 = ItemDB.get_item(hands_item_id)
		var nm2: String = hands_item_id
		if item2:
			nm2 = str(item2.display_name)
		lines.append("Hands: %s x%d (occupied)" % [nm2, hands_count])
	else:
		lines.append("Hands: free")
	return lines
