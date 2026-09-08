## Fire Escape palette — Akitti + Prism (2026-09-08)
## Drop into Godot project; pick dusk vs rain-night lighting sets.
class_name FireEscapePalette
extends Object

# --- Core atmosphere (dusk / M0) ---
const NIGHT_SLATE := Color("#0B1020")
const WET_ASPHALT := Color("#1A1F2E")
const CONCRETE_DUST := Color("#8A8490")
const STUCCO := Color("#C4B8A8")
const SICK_AMBER := Color("#E8A84A")
const SODIUM_DUSK := Color("#FF6B2C")
const MAGENTA_SIGN := Color("#FF2D95")
const CYAN_RIG := Color("#2DE2E6")
const TOXIC_LIME := Color("#B6FF3B")
const OXIDIZED_TEAL := Color("#1F6F6A")
const RUST := Color("#7A2E2E")
const CAT_CREAM := Color("#F3E6D0")

# --- Primary neon accents ---
const HOT_MAGENTA := Color("#FF1090")
const HOT_PINK := Color("#FF4DD2")
const ELECTRIC_VIOLET := Color("#B026FF")
const LASER_PURPLE := Color("#7A00FF")
const CYAN_SHOCK := Color("#00F5FF")
const ICE_BLUE := Color("#7DF9FF")
const ACID_LIME := Color("#D6FF00")
const RADIOACTIVE_GREEN := Color("#39FF14")
const WARNING_AMBER := Color("#FFD100")
const MOLTEN_ORANGE := Color("#FF5A00")
const BLOOD_NEON := Color("#FF003C")
const WHITE_HOT := Color("#F8FBFF")

# --- Rain night ---
const NIGHT_VOID := Color("#050810")
const FLOODED_ASPHALT := Color("#121820")
const WET_CONCRETE := Color("#5A6570")
const COLD_STUCCO := Color("#9AA3A8")
const MOON_FILL := Color("#7A9BB0")
const CYAN_SOAK := Color("#1AC8D4")
const WINDOW_GLOW := Color("#4DE8FF")
const MAGENTA_LEAK := Color("#C2186A")
const POOL_GREEN := Color("#2D5A4A")
const MOLD_TEAL := Color("#0F3D3A")
const WET_RUST := Color("#5A2428")
const LAMP_POCKET := Color("#E8D4B8")

# --- UI / kg inventory ---
const PANEL_VOID := Color("#0E1420")
const PANEL_FACE := Color("#1A2233")
const PANEL_EDGE := Color("#2A3548")
const TEXT_PRIMARY := Color("#E8EEF5")
const TEXT_MUTED := Color("#8A96A8")
const KG_OK := Color("#39FF14")
const KG_WARN := Color("#FFD100")
const KG_OVER := Color("#FF003C")
const HANDS_BUSY := Color("#C45C6A")
const SLOT_EMPTY := Color("#243040")
const FOCUS_CYAN := Color("#00F5FF")
const TUBE_CORE := Color("#F8FBFF")

# --- Prism expansions ---
const HERB_SAP := Color("#4A7A3C")
const POTTING_SOIL := Color("#3B2A1E")
const BALCONY_GLASS := Color("#A8C4D4")
const OCCUPIED_ROSE := Color("#C45C6A")

## Emission cheat: albedo ~30% of hex, emission_energy 3.0–8.0 on same hex.
static func emission_albedo(c: Color) -> Color:
	return Color(c.r * 0.3, c.g * 0.3, c.b * 0.3, c.a)
