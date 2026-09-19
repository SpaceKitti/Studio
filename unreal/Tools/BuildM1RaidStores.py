# -*- coding: utf-8 -*-
"""
BuildM1RaidStores.py — layout-only bake for Fire Escape raid nodes.

Creates /Game/Maps/M1_RaidStores with labeled floors, walls, façade windows.
Does NOT touch M0_FireEscape or FELevelBuilder / FE_M0 content.

Run (UE 5.8 Python):
  UnrealEditor.exe FireEscape.uproject -ExecutePythonScript=.../BuildM1RaidStores.py -unattended -nosplash
"""
from __future__ import annotations

import json
import os
import traceback

import unreal

# ---------------------------------------------------------------------------
# Paths / constants
# ---------------------------------------------------------------------------
PROJECT_ROOT = r"C:\Users\Akitt\Games\Studio\unreal"
LOG = os.path.join(PROJECT_ROOT, r"Saved\Logs\BuildM1RaidStores.txt")
MANIFEST = os.path.join(PROJECT_ROOT, r"Saved\Logs\M1_RaidStores_manifest.json")
SHOT_DIR = os.path.join(PROJECT_ROOT, r"Saved\Screenshots\M1")
ASSET_PATH = "/Game/Maps/M1_RaidStores"
CUBE_PATH = "/Engine/BasicShapes/Cube.Cube"
GLASS_PATH = "/Game/Materials/M_Glass"
TAG = "FE_RaidLayout"
FOLDER = "M1_Raid"
CEILING_M = 3.2
FLOOR_THICK_M = 0.15
WALL_THICK_M = 0.2
DOOR_W_M = 1.4
WINDOW_H_M = 1.6
WINDOW_SILL_M = 1.0
WINDOW_T_M = 0.06

# Unreal cm: X=East, Y=North, Z=Up. Scale on BasicShapes Cube (==100cm) equals meters.
# Street strip between buildings (~9 m wide). Hardware west, grocery east.


def log(msg: str) -> None:
    print(msg)
    os.makedirs(os.path.dirname(LOG), exist_ok=True)
    with open(LOG, "a", encoding="utf-8") as handle:
        handle.write(str(msg) + "\n")


def m_to_loc(x_m: float, y_m: float, z_m: float) -> unreal.Vector:
    return unreal.Vector(x_m * 100.0, y_m * 100.0, z_m * 100.0)


def m_to_scale(sx_m: float, sy_m: float, sz_m: float) -> unreal.Vector:
    return unreal.Vector(sx_m, sy_m, sz_m)


def ensure_glass_material() -> None:
    if unreal.EditorAssetLibrary.does_asset_exist(GLASS_PATH):
        log("M_Glass exists")
        return
    asset_tools = unreal.AssetToolsHelpers.get_asset_tools()
    factory = unreal.MaterialFactoryNew()
    mat = asset_tools.create_asset("M_Glass", "/Game/Materials", unreal.Material, factory)
    if mat is None:
        log("WARN: could not create M_Glass")
        return
    mat.set_editor_property("blend_mode", unreal.BlendMode.BLEND_TRANSLUCENT)
    mat.set_editor_property("two_sided", True)
    try:
        mat.set_editor_property(
            "translucency_lighting_mode",
            unreal.TranslucencyLightingMode.TLM_SURFACE_PER_PIXEL_LIGHTING,
        )
    except Exception as exc:
        log("translucency mode skip: %s" % exc)
    base = unreal.MaterialEditingLibrary.create_material_expression(
        mat, unreal.MaterialExpressionConstant3Vector, -380, -120
    )
    base.set_editor_property("constant", unreal.LinearColor(0.66, 0.77, 0.83, 1.0))
    unreal.MaterialEditingLibrary.connect_material_property(
        base, "", unreal.MaterialProperty.MP_BASE_COLOR
    )
    spec = unreal.MaterialEditingLibrary.create_material_expression(
        mat, unreal.MaterialExpressionConstant, -380, 40
    )
    spec.set_editor_property("r", 0.65)
    unreal.MaterialEditingLibrary.connect_material_property(
        spec, "", unreal.MaterialProperty.MP_SPECULAR
    )
    rough = unreal.MaterialEditingLibrary.create_material_expression(
        mat, unreal.MaterialExpressionConstant, -380, 120
    )
    rough.set_editor_property("r", 0.06)
    unreal.MaterialEditingLibrary.connect_material_property(
        rough, "", unreal.MaterialProperty.MP_ROUGHNESS
    )
    opac = unreal.MaterialEditingLibrary.create_material_expression(
        mat, unreal.MaterialExpressionConstant, -380, 200
    )
    opac.set_editor_property("r", 0.22)
    unreal.MaterialEditingLibrary.connect_material_property(
        opac, "", unreal.MaterialProperty.MP_OPACITY
    )
    unreal.MaterialEditingLibrary.recompile_material(mat)
    unreal.EditorAssetLibrary.save_asset(GLASS_PATH)
    log("created M_Glass")


def apply_simple_color(mesh_comp, color: unreal.LinearColor, roughness: float = 0.85) -> None:
    try:
        mid = unreal.MaterialEditingLibrary.create_dynamic_material_instance(
            mesh_comp.get_material(0) if mesh_comp.get_material(0) else None
        )
    except Exception:
        mid = None
    if mid is None:
        # Fallback: leave engine default; label/geometry still valid for Prism.
        return
    try:
        mid.set_vector_parameter_value("BaseColor", color)
        mid.set_scalar_parameter_value("Roughness", roughness)
        mesh_comp.set_material(0, mid)
    except Exception:
        pass


def spawn_cube(
    actor_sub,
    cube_mesh,
    label: str,
    center_m,
    size_m,
    color: unreal.LinearColor,
    *,
    collision: bool = True,
    glass: bool = False,
    folder: str = FOLDER,
) -> unreal.Actor:
    """Spawn a labeled StaticMeshActor cube. center/size in meters (Unreal axes)."""
    loc = m_to_loc(center_m[0], center_m[1], center_m[2])
    actor = actor_sub.spawn_actor_from_class(
        unreal.StaticMeshActor, loc, unreal.Rotator(pitch=0.0, yaw=0.0, roll=0.0)
    )
    if actor is None:
        log("FAILED spawn %s" % label)
        return None
    mesh = actor.static_mesh_component
    mesh.set_mobility(unreal.ComponentMobility.MOVABLE)
    mesh.set_static_mesh(cube_mesh)
    mesh.set_world_scale3d(m_to_scale(size_m[0], size_m[1], size_m[2]))
    if glass and unreal.EditorAssetLibrary.does_asset_exist(GLASS_PATH):
        glass_mat = unreal.EditorAssetLibrary.load_asset(GLASS_PATH)
        if glass_mat:
            mesh.set_material(0, glass_mat)
        mesh.set_collision_enabled(unreal.CollisionEnabled.NO_COLLISION)
        mesh.set_cast_shadow(False)
    else:
        apply_simple_color(mesh, color)
        if collision:
            mesh.set_collision_enabled(unreal.CollisionEnabled.QUERY_AND_PHYSICS)
            mesh.set_collision_object_type(unreal.CollisionChannel.ECC_WORLD_STATIC)
            mesh.set_collision_response_to_all_channels(unreal.CollisionResponse.ECR_BLOCK)
            mesh.set_cast_shadow(True)
        else:
            mesh.set_collision_enabled(unreal.CollisionEnabled.NO_COLLISION)
    try:
        actor.set_actor_label(label)
    except Exception:
        pass
    try:
        actor.set_folder_path(folder)
    except Exception:
        pass
    try:
        tags = list(actor.tags) if actor.tags else []
        if TAG not in [str(t) for t in tags]:
            tags.append(TAG)
        actor.tags = tags
    except Exception:
        try:
            actor.tags.append(TAG)
        except Exception:
            pass
    try:
        actor.set_is_spatially_loaded(False)
    except Exception:
        pass
    log(
        "spawned %s center_m=(%.2f,%.2f,%.2f) size_m=(%.2f,%.2f,%.2f)"
        % (label, center_m[0], center_m[1], center_m[2], size_m[0], size_m[1], size_m[2])
    )
    return actor


def clear_prior_raid_actors(actor_sub) -> int:
    """Remove prior FE_RaidLayout actors only (never FE_M0 / M0)."""
    kill = []
    for actor in actor_sub.get_all_level_actors():
        try:
            if actor.actor_has_tag(TAG):
                kill.append(actor)
                continue
            label = ""
            try:
                label = actor.get_actor_label()
            except Exception:
                label = actor.get_name()
            # Safety: never touch M0 labels / tags
            if actor.actor_has_tag("FE_M0"):
                continue
            if str(label).startswith("M0") or "FireEscape" in str(label):
                continue
        except Exception:
            continue
    n = 0
    for actor in kill:
        try:
            actor_sub.destroy_actor(actor)
            n += 1
        except Exception as exc:
            log("destroy skip: %s" % exc)
    log("cleared %d prior %s actors" % (n, TAG))
    return n


def wall_segments_with_door_gap(
    y0: float,
    y1: float,
    door_center_y: float,
    door_w: float,
):
    """Split a Y-span wall into two segments leaving a door gap. Returns list of (y_center, y_size)."""
    gap0 = door_center_y - door_w * 0.5
    gap1 = door_center_y + door_w * 0.5
    segs = []
    if gap0 - y0 > 0.05:
        size = gap0 - y0
        segs.append((y0 + size * 0.5, size))
    if y1 - gap1 > 0.05:
        size = y1 - gap1
        segs.append((gap1 + size * 0.5, size))
    return segs


def build_layout(actor_sub, cube_mesh, manifest: dict) -> None:
    floors = []
    walls = []
    windows = []

    floor_col = unreal.LinearColor(0.35, 0.34, 0.32, 1.0)
    wall_col = unreal.LinearColor(0.55, 0.52, 0.48, 1.0)
    garden_col = unreal.LinearColor(0.30, 0.38, 0.28, 1.0)
    street_col = unreal.LinearColor(0.22, 0.22, 0.24, 1.0)
    sidewalk_col = unreal.LinearColor(0.42, 0.42, 0.40, 1.0)
    cooler_col = unreal.LinearColor(0.45, 0.55, 0.62, 1.0)
    stock_col = unreal.LinearColor(0.40, 0.36, 0.30, 1.0)

    # ---- Street (between buildings) ----
    # 9 m wide in X, 26 m long in Y
    street = {
        "name": "StreetFloor",
        "cx": 0.0,
        "cy": 0.0,
        "sx": 9.0,
        "sy": 26.0,
        "color": street_col,
    }
    # Sidewalk lips
    sidewalk_w = {
        "name": "SidewalkWest",
        "cx": -5.25,
        "cy": 0.0,
        "sx": 1.5,
        "sy": 26.0,
        "color": sidewalk_col,
    }
    sidewalk_e = {
        "name": "SidewalkEast",
        "cx": 5.25,
        "cy": 0.0,
        "sx": 1.5,
        "sy": 26.0,
        "color": sidewalk_col,
    }

    # ---- Hardware (west of street) ----
    # Main sales 12 m deep (X) x 18 m along street (Y)
    # East façade at X = -6.0 (street edge of building)
    hw_main = {
        "name": "HardwareMainFloor",
        "x0": -18.0,
        "x1": -6.0,
        "y0": -9.0,
        "y1": 9.0,
        "color": floor_col,
    }  # 12 x 18
    # Garden south of main, same depth, 8 m along Y
    hw_garden = {
        "name": "HardwareGardenFloor",
        "x0": -18.0,
        "x1": -6.0,
        "y0": -17.0,
        "y1": -9.0,
        "color": garden_col,
    }  # 12 x 8
    # Stock rear (further west), 6 x 8
    hw_stock = {
        "name": "HardwareStockFloor",
        "x0": -24.0,
        "x1": -18.0,
        "y0": -4.0,
        "y1": 4.0,
        "color": stock_col,
    }  # 6 x 8

    # ---- Grocery (east of street) ----
    # 10 m deep x 12 m along street
    gr_main = {
        "name": "GroceryFloor",
        "x0": 6.0,
        "x1": 16.0,
        "y0": -6.0,
        "y1": 6.0,
        "color": floor_col,
    }  # 10 x 12
    # Cooler strip along north interior wall
    gr_cooler = {
        "name": "GroceryCoolerFloor",
        "x0": 6.2,
        "x1": 15.8,
        "y0": 4.6,
        "y1": 5.8,
        "color": cooler_col,
    }  # ~9.6 x 1.2

    def floor_actor(spec, z_center=None):
        if "sx" in spec:
            cx, cy, sx, sy = spec["cx"], spec["cy"], spec["sx"], spec["sy"]
        else:
            sx = spec["x1"] - spec["x0"]
            sy = spec["y1"] - spec["y0"]
            cx = 0.5 * (spec["x0"] + spec["x1"])
            cy = 0.5 * (spec["y0"] + spec["y1"])
        if z_center is None:
            z_center = FLOOR_THICK_M * 0.5
        a = spawn_cube(
            actor_sub,
            cube_mesh,
            spec["name"],
            (cx, cy, z_center),
            (sx, sy, FLOOR_THICK_M),
            spec["color"],
        )
        floors.append(
            {
                "name": spec["name"],
                "size_m": [round(sx, 2), round(sy, 2), round(FLOOR_THICK_M, 2)],
                "center_m": [round(cx, 2), round(cy, 2), round(z_center, 2)],
            }
        )
        return a

    # FLOORS FIRST
    log("=== FLOORS ===")
    for spec in (street, sidewalk_w, sidewalk_e):
        floor_actor(spec)
    for spec in (hw_main, hw_garden, hw_stock, gr_main, gr_cooler):
        floor_actor(spec)

    # ---- Walls helper ----
    wall_z = FLOOR_THICK_M + CEILING_M * 0.5
    wall_h = CEILING_M

    def add_wall(label, cx, cy, sx, sy, sz=None, color=None):
        if sz is None:
            sz = wall_h
        if color is None:
            color = wall_col
        a = spawn_cube(
            actor_sub,
            cube_mesh,
            label,
            (cx, cy, FLOOR_THICK_M + sz * 0.5),
            (sx, sy, sz),
            color,
        )
        walls.append({"name": label, "size_m": [round(sx, 2), round(sy, 2), round(sz, 2)]})
        return a

    def add_window(label, cx, cy, sx, sy):
        zc = FLOOR_THICK_M + WINDOW_SILL_M + WINDOW_H_M * 0.5
        a = spawn_cube(
            actor_sub,
            cube_mesh,
            label,
            (cx, cy, zc),
            (sx, sy, WINDOW_H_M),
            unreal.LinearColor(0.7, 0.8, 0.85, 1.0),
            collision=False,
            glass=True,
        )
        windows.append({"name": label, "size_m": [round(sx, 2), round(sy, 2), round(WINDOW_H_M, 2)]})
        return a

    log("=== HARDWARE WALLS ===")
    # Exterior box for main (street façade at x1=-6 with door gap)
    hx0, hx1, hy0, hy1 = hw_main["x0"], hw_main["x1"], hw_main["y0"], hw_main["y1"]
    door_y_hw = 0.0  # centered on main façade

    # West wall (rear of main, opening toward stock later)
    add_wall("HardwareWall_West", hx0, 0.5 * (hy0 + hy1), WALL_THICK_M, hy1 - hy0)
    # North wall (full)
    add_wall("HardwareWall_North", 0.5 * (hx0 + hx1), hy1, hx1 - hx0, WALL_THICK_M)
    # South wall toward garden — leave 2.5 m clear opening (no wall in doorway)
    garden_open_w = 2.5
    garden_open_cx = 0.5 * (hx0 + hx1)
        # Explicit south stubs east/west of opening
    left_x0, left_x1 = hx0, garden_open_cx - garden_open_w * 0.5
    right_x0, right_x1 = garden_open_cx + garden_open_w * 0.5, hx1
    if left_x1 - left_x0 > 0.3:
        add_wall(
            "HardwareWall_South_W",
            0.5 * (left_x0 + left_x1),
            hy0,
            left_x1 - left_x0,
            WALL_THICK_M,
        )
    if right_x1 - right_x0 > 0.3:
        add_wall(
            "HardwareWall_South_E",
            0.5 * (right_x0 + right_x1),
            hy0,
            right_x1 - right_x0,
            WALL_THICK_M,
        )

    # East (street) façade with door gap
    for i, (yc, ys) in enumerate(
        wall_segments_with_door_gap(hy0, hy1, door_y_hw, DOOR_W_M)
    ):
        add_wall("HardwareWall_East_%d" % i, hx1, yc, WALL_THICK_M, ys)

    # Interior dividers: checkout near entrance (east), aisle volumes
    # Checkout divider ~3 m from entrance, with center walk gap
    checkout_x = hx1 - 3.0
    for i, (yc, ys) in enumerate(
        wall_segments_with_door_gap(hy0 + 0.5, hy1 - 0.5, door_y_hw, 2.0)
    ):
        add_wall("HardwareWall_Checkout_%d" % i, checkout_x, yc, WALL_THICK_M, ys)

    # Aisle divider walls (tools / fasteners / paint / plumbing) — empty room volumes
    aisle_xs = [hx1 - 6.0, hx1 - 9.0]
    for ai, ax in enumerate(aisle_xs):
        # partial walls leaving center circulation
        add_wall(
            "HardwareWall_Aisle%d_N" % ai,
            ax,
            hy1 - 3.5,
            WALL_THICK_M,
            5.0,
        )
        add_wall(
            "HardwareWall_Aisle%d_S" % ai,
            ax,
            hy0 + 3.5,
            WALL_THICK_M,
            5.0,
        )

    # Cross aisle labels as low stub walls (zone markers only — still walls, no props)
    zone_y = [-6.0, -2.0, 2.0, 6.0]
    zone_names = ["Tools", "Fasteners", "Paint", "Plumbing"]
    for zi, (zy, zn) in enumerate(zip(zone_y, zone_names)):
        add_wall(
            "HardwareZone_%s" % zn,
            hx0 + 3.0,
            zy,
            4.0,
            WALL_THICK_M,
            sz=1.0,
            color=unreal.LinearColor(0.5, 0.45, 0.4, 1.0),
        )

    # Opening main → garden (south wall already solid; cut gap via separate opening walls)
    # Replace: remove solid south as full barrier by adding door gap segment approach —
    # We already placed full south wall; add labeled opening as missing section by
    # rebuilding south as gapped for garden access.
    # Destroy full south and rebuild gapped:
    # (simpler: add garden walls separately and leave a 2.5 m opening centered)

    log("=== GARDEN WALLS (open-air / lower) ===")
    gx0, gx1, gy0, gy1 = hw_garden["x0"], hw_garden["x1"], hw_garden["y0"], hw_garden["y1"]
    garden_h = 1.4  # lower walls for open-air feel
    add_wall("GardenWall_West", gx0, 0.5 * (gy0 + gy1), WALL_THICK_M, gy1 - gy0, sz=garden_h)
    add_wall("GardenWall_South", 0.5 * (gx0 + gx1), gy0, gx1 - gx0, WALL_THICK_M, sz=garden_h)
    add_wall("GardenWall_East", gx1, 0.5 * (gy0 + gy1), WALL_THICK_M, gy1 - gy0, sz=garden_h)
    # Marker strip for clear main↔garden opening (not a blocking wall)
    shared_y = hy0
    gap_cx = 0.5 * (hx0 + hx1)
    spawn_cube(
        actor_sub,
        cube_mesh,
        "HardwareGardenOpening",
        (gap_cx, shared_y, FLOOR_THICK_M * 0.5 + 0.01),
        (2.5, 0.4, 0.05),
        unreal.LinearColor(0.2, 0.45, 0.25, 1.0),
        collision=False,
    )
    floors.append(
        {
            "name": "HardwareGardenOpening",
            "size_m": [2.5, 0.4, 0.05],
            "center_m": [round(gap_cx, 2), round(shared_y, 2), round(FLOOR_THICK_M * 0.5 + 0.01, 2)],
            "note": "clear opening main↔garden (door gap marker)",
        }
    )

    log("=== HARDWARE STOCK WALLS ===")
    sx0, sx1, sy0, sy1 = hw_stock["x0"], hw_stock["x1"], hw_stock["y0"], hw_stock["y1"]
    add_wall("StockWall_West", sx0, 0.5 * (sy0 + sy1), WALL_THICK_M, sy1 - sy0)
    add_wall("StockWall_North", 0.5 * (sx0 + sx1), sy1, sx1 - sx0, WALL_THICK_M)
    add_wall("StockWall_South", 0.5 * (sx0 + sx1), sy0, sx1 - sx0, WALL_THICK_M)
    # East toward main: door gap
    for i, (yc, ys) in enumerate(wall_segments_with_door_gap(sy0, sy1, 0.0, 1.5)):
        add_wall("StockWall_East_%d" % i, sx1, yc, WALL_THICK_M, ys)

    log("=== GROCERY WALLS ===")
    rx0, rx1, ry0, ry1 = gr_main["x0"], gr_main["x1"], gr_main["y0"], gr_main["y1"]
    door_y_gr = 0.0

    add_wall("GroceryWall_East", rx1, 0.5 * (ry0 + ry1), WALL_THICK_M, ry1 - ry0)
    add_wall("GroceryWall_North", 0.5 * (rx0 + rx1), ry1, rx1 - rx0, WALL_THICK_M)
    add_wall("GroceryWall_South", 0.5 * (rx0 + rx1), ry0, rx1 - rx0, WALL_THICK_M)
    # West (street) façade with door gap
    for i, (yc, ys) in enumerate(
        wall_segments_with_door_gap(ry0, ry1, door_y_gr, DOOR_W_M)
    ):
        add_wall("GroceryWall_West_%d" % i, rx0, yc, WALL_THICK_M, ys)

    # Checkout near street entrance
    checkout_x_g = rx0 + 2.5
    for i, (yc, ys) in enumerate(
        wall_segments_with_door_gap(ry0 + 0.4, ry1 - 0.4, door_y_gr, 2.0)
    ):
        add_wall("GroceryWall_Checkout_%d" % i, checkout_x_g, yc, WALL_THICK_M, ys)

    # Rear stock divider
    stock_x = rx1 - 3.0
    for i, (yc, ys) in enumerate(
        wall_segments_with_door_gap(ry0 + 0.4, ry1 - 0.4, 0.0, 1.6)
    ):
        add_wall("GroceryWall_Stock_%d" % i, stock_x, yc, WALL_THICK_M, ys)

    # Cooler backer wall (north)
    add_wall(
        "GroceryWall_CoolerBack",
        0.5 * (gr_cooler["x0"] + gr_cooler["x1"]),
        gr_cooler["y1"],
        gr_cooler["x1"] - gr_cooler["x0"],
        WALL_THICK_M,
        sz=2.2,
        color=unreal.LinearColor(0.4, 0.5, 0.55, 1.0),
    )

    log("=== WINDOWS (street façades) ===")
    # Hardware east façade windows (north and south of door)
    win_positions_hw = [-6.5, -3.5, 3.5, 6.5]
    for i, wy in enumerate(win_positions_hw):
        add_window("HardwareWindow_%d" % i, hx1 + WINDOW_T_M * 0.5, wy, WINDOW_T_M, 1.8)

    win_positions_gr = [-4.0, -2.0, 2.0, 4.0]
    for i, wy in enumerate(win_positions_gr):
        add_window("GroceryWindow_%d" % i, rx0 - WINDOW_T_M * 0.5, wy, WINDOW_T_M, 1.8)

    # Light + sky so viewport isn't black
    try:
        sun = actor_sub.spawn_actor_from_class(
            unreal.DirectionalLight,
            m_to_loc(0.0, 0.0, 8.0),
            unreal.Rotator(pitch=-40.0, yaw=35.0, roll=0.0),
        )
        if sun:
            sun.set_actor_label("M1_Sun")
            sun.set_folder_path(FOLDER)
            try:
                sun.tags = [TAG]
            except Exception:
                pass
        sky = actor_sub.spawn_actor_from_class(
            unreal.SkyAtmosphere,
            m_to_loc(0.0, 0.0, 0.0),
            unreal.Rotator(0.0, 0.0, 0.0),
        )
        if sky:
            sky.set_actor_label("M1_SkyAtmosphere")
            sky.set_folder_path(FOLDER)
        skylight = actor_sub.spawn_actor_from_class(
            unreal.SkyLight,
            m_to_loc(0.0, 0.0, 5.0),
            unreal.Rotator(0.0, 0.0, 0.0),
        )
        if skylight:
            skylight.set_actor_label("M1_SkyLight")
            skylight.set_folder_path(FOLDER)
    except Exception as exc:
        log("lighting spawn skip: %s" % exc)

    # Overview camera
    try:
        cam = actor_sub.spawn_actor_from_class(
            unreal.CameraActor,
            m_to_loc(0.0, -28.0, 22.0),
            unreal.Rotator(pitch=-35.0, yaw=90.0, roll=0.0),
        )
        if cam:
            cam.set_actor_label("FE_M1_OverviewCam")
            cam.set_folder_path(FOLDER)
            try:
                cam.tags = [TAG, "FE_EditorOverview"]
            except Exception:
                pass
    except Exception as exc:
        log("cam spawn skip: %s" % exc)

    manifest["floors"] = floors
    manifest["walls"] = walls
    manifest["windows"] = windows
    manifest["door_gaps_m"] = {
        "hardware_street": DOOR_W_M,
        "grocery_street": DOOR_W_M,
        "hardware_garden_opening": 2.5,
        "hardware_stock": 1.5,
    }
    manifest["ceiling_m"] = CEILING_M
    manifest["tag"] = TAG
    manifest["asset_path"] = ASSET_PATH


def set_overview_viewport(editor_sub, level_editor) -> None:
    loc = m_to_loc(0.0, -28.0, 22.0)
    rot = unreal.Rotator(pitch=-35.0, yaw=90.0, roll=0.0)
    try:
        editor_sub.set_level_viewport_camera_info(loc, rot)
        log("viewport overview set")
    except Exception as exc:
        log("viewport set failed: %s" % exc)
        try:
            level_editor.editor_set_game_view(False)
        except Exception:
            pass


def main() -> None:
    os.makedirs(os.path.dirname(LOG), exist_ok=True)
    os.makedirs(SHOT_DIR, exist_ok=True)
    with open(LOG, "w", encoding="utf-8") as handle:
        handle.write("")
    log("BEGIN BuildM1RaidStores")
    log("NOTE: will not load or modify M0_FireEscape")

    ensure_glass_material()

    level_editor = unreal.get_editor_subsystem(unreal.LevelEditorSubsystem)
    actor_sub = unreal.get_editor_subsystem(unreal.EditorActorSubsystem)
    editor_sub = unreal.get_editor_subsystem(unreal.UnrealEditorSubsystem)

    # Ensure we are NOT on M0 — create / load M1 only
    world_name = str(editor_sub.get_editor_world())
    log("current world %s" % world_name)
    if "M0_FireEscape" in world_name:
        log("WARNING: editor was on M0 — switching away without saving M0 changes from this script")

    if unreal.EditorAssetLibrary.does_asset_exist(ASSET_PATH):
        loaded = level_editor.load_level(ASSET_PATH)
        log("load_level %s -> %s" % (ASSET_PATH, loaded))
    else:
        # Ensure Maps folder exists
        if not unreal.EditorAssetLibrary.does_directory_exist("/Game/Maps"):
            unreal.EditorAssetLibrary.make_directory("/Game/Maps")
        try:
            created = level_editor.new_level(ASSET_PATH, False)
        except TypeError:
            created = level_editor.new_level(ASSET_PATH)
        log("new_level %s -> %s" % (ASSET_PATH, created))
        if not created and not unreal.EditorAssetLibrary.does_asset_exist(ASSET_PATH):
            log("FAILED: could not create level")
            return

    world = editor_sub.get_editor_world()
    log("world %s" % world)
    # Confirm not M0 asset path
    if "M0_FireEscape" in str(world):
        log("FAILED: still on M0 world — abort to protect M0")
        return

    cube_mesh = unreal.EditorAssetLibrary.load_asset(CUBE_PATH)
    if cube_mesh is None:
        log("FAILED: missing %s" % CUBE_PATH)
        return

    clear_prior_raid_actors(actor_sub)

    manifest = {"map": ASSET_PATH, "project": PROJECT_ROOT}
    build_layout(actor_sub, cube_mesh, manifest)

    set_overview_viewport(editor_sub, level_editor)

    # Screenshot
    try:
        shot = os.path.join(SHOT_DIR, "M1_RaidStores_overview.png")
        unreal.AutomationLibrary.take_high_res_screenshot(1600, 900, shot)
        log("screenshot %s" % shot)
        manifest["screenshot"] = shot
    except Exception as exc:
        log("screenshot skip: %s" % exc)

    saved = level_editor.save_current_level()
    log("save_current_level %s" % saved)
    try:
        unreal.EditorAssetLibrary.save_asset(ASSET_PATH)
        log("asset saved %s" % ASSET_PATH)
    except Exception as exc:
        log("save_asset: %s" % exc)

    # Verify M0 untouched by this process (existence only)
    m0 = "/Game/Maps/M0_FireEscape"
    manifest["m0_still_exists"] = unreal.EditorAssetLibrary.does_asset_exist(m0)
    log("M0_FireEscape exists (untouched by this script): %s" % manifest["m0_still_exists"])

    with open(MANIFEST, "w", encoding="utf-8") as handle:
        json.dump(manifest, handle, indent=2)
    log("manifest %s" % MANIFEST)
    log("FLOOR_NAMES %s" % ",".join(f["name"] for f in manifest.get("floors", [])))
    log("DONE")

    try:
        unreal.SystemLibrary.execute_console_command(world, "QUIT_EDITOR")
    except Exception as exc:
        log("quit: %s" % exc)


if __name__ == "__main__":
    try:
        main()
    except Exception:
        log(traceback.format_exc())
        raise
