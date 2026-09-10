import os
import time
import traceback
import unreal

LOG = r"C:\Users\Akitt\Games\Studio\unreal\Saved\Logs\BuildM0Map.txt"
ASSET_PATH = "/Game/Maps/M0_FireEscape"
SHOT_DIR = r"C:\Users\Akitt\Games\Studio\unreal\Saved\Screenshots\WindowsEditor"
FLAG_DIR = r"C:\Users\Akitt\Games\Studio\unreal\Saved\capture_flags"

# HomeFloor overview: north of balconies, elevated, looking south into living + doors.
# HomeFloor ~(-410, 625, -11), living ~(-310, 250), doors around Y=0..345.
# NOTE: unreal.Rotator positional is (roll, pitch, yaw) ÃƒÂ¢Ã¢â€šÂ¬Ã¢â‚¬Â always use keywords.
OVERVIEW_LOC = unreal.Vector(-200.0, 1200.0, 650.0)
OVERVIEW_ROT = unreal.Rotator(pitch=-28.0, yaw=-110.0, roll=0.0)
OVERVIEW_CAM_LABEL = "FE_EditorOverviewCam"


def log(msg):
    print(msg)
    os.makedirs(os.path.dirname(LOG), exist_ok=True)
    with open(LOG, "a", encoding="utf-8") as handle:
        handle.write(str(msg) + "\n")


def import_folder_textures(src_dir, dest_path, exts):
    import_tasks = []
    if not os.path.isdir(src_dir):
        log("skip import missing %s" % src_dir)
        return
    asset_tools = unreal.AssetToolsHelpers.get_asset_tools()
    for name in os.listdir(src_dir):
        low = name.lower()
        if not any(low.endswith(e) for e in exts):
            continue
        stem = os.path.splitext(name)[0]
        asset_name_path = dest_path + "/" + stem
        if unreal.EditorAssetLibrary.does_asset_exist(asset_name_path):
            log("exists %s" % asset_name_path)
            continue
        task = unreal.AssetImportTask()
        task.filename = os.path.join(src_dir, name)
        task.destination_path = dest_path
        task.destination_name = stem
        task.replace_existing = True
        task.automated = True
        task.save = True
        import_tasks.append(task)
    if import_tasks:
        asset_tools.import_asset_tasks(import_tasks)
        log("imported %d into %s" % (len(import_tasks), dest_path))
    else:
        log("nothing to import for %s" % dest_path)


def set_viewport_camera(editor_sub, level_editor, loc, rot, tag=""):
    """UE 5.8: prefer UnrealEditorSubsystem / LevelEditorSubsystem over deprecated EditorLevelLibrary."""
    used = []
    errors = []
    try:
        editor_sub.set_level_viewport_camera_info(loc, rot)
        used.append("UES")
    except Exception as exc:
        errors.append("UES:%s" % exc)
    keys = []
    try:
        keys = list(level_editor.get_viewport_config_keys())
    except Exception as exc:
        errors.append("keys:%s" % exc)
    try:
        active = level_editor.get_active_viewport_config_key()
        if active and active not in keys:
            keys.append(active)
    except Exception:
        pass
    if not keys:
        keys = [unreal.Name("Perspective")]
    for key in keys:
        try:
            level_editor.set_level_viewport_camera_info(loc, rot, key)
            try:
                level_editor.set_level_viewport_fov(90.0, key)
            except Exception:
                pass
            used.append("LES:%s" % key)
        except Exception as exc:
            errors.append("LES:%s:%s" % (key, exc))
    if used:
        log("cam via %s %s -> loc=%s rot=%s" % (used, tag, loc, rot))
        return ",".join(used)
    try:
        unreal.EditorLevelLibrary.set_level_viewport_camera_info(loc, rot)
        log("cam via EditorLevelLibrary (deprecated) %s" % tag)
        return "EditorLevelLibrary"
    except Exception as exc:
        errors.append("ELL:%s" % exc)
    log("cam FAILED %s: %s" % (tag, " | ".join(errors)))
    return None


def verify_viewport_camera(editor_sub, level_editor, expected_loc, tol=50.0):
    try:
        info = editor_sub.get_level_viewport_camera_info()
        if info:
            loc = info[0] if isinstance(info, (tuple, list)) else info
            if hasattr(loc, "x"):
                dx = abs(loc.x - expected_loc.x)
                dy = abs(loc.y - expected_loc.y)
                dz = abs(loc.z - expected_loc.z)
                ok = dx <= tol and dy <= tol and dz <= tol
                log("cam verify loc=(%.1f,%.1f,%.1f) delta=(%.1f,%.1f,%.1f) ok=%s" % (
                    loc.x, loc.y, loc.z, dx, dy, dz, ok))
                return ok
    except Exception as exc:
        log("cam verify UES failed: %s" % exc)
    try:
        info = level_editor.get_level_viewport_camera_info(unreal.Name("Perspective"))
        if info:
            loc = info[0] if isinstance(info, (tuple, list)) else info
            if hasattr(loc, "x"):
                log("cam verify LES loc=(%.1f,%.1f,%.1f)" % (loc.x, loc.y, loc.z))
                return True
    except Exception as exc:
        log("cam verify LES failed: %s" % exc)
    return False


def ensure_overview_camera_actor(actor_sub, loc, rot):
    existing = None
    for actor in actor_sub.get_all_level_actors():
        try:
            label = actor.get_actor_label()
        except Exception:
            label = actor.get_name()
        if label == OVERVIEW_CAM_LABEL or actor.get_name().startswith(OVERVIEW_CAM_LABEL):
            existing = actor
            break
    if existing is None:
        existing = actor_sub.spawn_actor_from_class(
            unreal.CameraActor,
            loc,
            rot,
        )
        if existing is None:
            log("FAILED spawn CameraActor")
            return None
        try:
            existing.set_actor_label(OVERVIEW_CAM_LABEL)
        except Exception:
            pass
        try:
            existing.tags = ["FE_M0", "FE_EditorOverview"]
        except Exception:
            try:
                existing.tags.append("FE_M0")
            except Exception:
                pass
        log("spawned %s" % OVERVIEW_CAM_LABEL)
    else:
        existing.set_actor_location(loc, False, True)
        existing.set_actor_rotation(rot, False)
        log("updated %s pose" % OVERVIEW_CAM_LABEL)
    try:
        existing.set_folder_path("M0")
    except Exception:
        pass
    return existing


def try_set_bookmark(level_editor, loc, rot):
    """Best-effort bookmark 0 so Ctrl+0 jumps to overview."""
    try:
        # Some 5.x builds expose bookmark helpers on LevelEditorSubsystem
        if hasattr(level_editor, "set_bookmark"):
            level_editor.set_bookmark(0)
            log("set_bookmark(0) after aiming")
            return
    except Exception as exc:
        log("set_bookmark skip: %s" % exc)
    try:
        world = unreal.get_editor_subsystem(unreal.UnrealEditorSubsystem).get_editor_world()
        unreal.SystemLibrary.execute_console_command(world, "SetActorLocation")
    except Exception:
        pass


def main():
    os.makedirs(os.path.dirname(LOG), exist_ok=True)
    os.makedirs(SHOT_DIR, exist_ok=True)
    os.makedirs(FLAG_DIR, exist_ok=True)
    with open(LOG, "w", encoding="utf-8") as handle:
        handle.write("")
    log("BEGIN")

    asset_tools = unreal.AssetToolsHelpers.get_asset_tools()

    def make_glass():
        path = "/Game/Materials/M_Glass"
        if unreal.EditorAssetLibrary.does_asset_exist(path):
            log("M_Glass exists")
            return
        factory = unreal.MaterialFactoryNew()
        mat = asset_tools.create_asset("M_Glass", "/Game/Materials", unreal.Material, factory)
        if mat is None:
            log("FAILED to create M_Glass")
            return
        mat.set_editor_property("blend_mode", unreal.BlendMode.BLEND_TRANSLUCENT)
        mat.set_editor_property("two_sided", True)
        try:
            mat.set_editor_property("translucency_lighting_mode", unreal.TranslucencyLightingMode.TLM_SURFACE_PER_PIXEL_LIGHTING)
        except Exception as exc:
            log("translucency mode skip: %s" % exc)
        base = unreal.MaterialEditingLibrary.create_material_expression(mat, unreal.MaterialExpressionConstant3Vector, -380, -120)
        base.set_editor_property("constant", unreal.LinearColor(0.66, 0.77, 0.83, 1.0))
        unreal.MaterialEditingLibrary.connect_material_property(base, "", unreal.MaterialProperty.MP_BASE_COLOR)
        spec = unreal.MaterialEditingLibrary.create_material_expression(mat, unreal.MaterialExpressionConstant, -380, 40)
        spec.set_editor_property("r", 0.65)
        unreal.MaterialEditingLibrary.connect_material_property(spec, "", unreal.MaterialProperty.MP_SPECULAR)
        rough = unreal.MaterialEditingLibrary.create_material_expression(mat, unreal.MaterialExpressionConstant, -380, 120)
        rough.set_editor_property("r", 0.06)
        unreal.MaterialEditingLibrary.connect_material_property(rough, "", unreal.MaterialProperty.MP_ROUGHNESS)
        opac = unreal.MaterialEditingLibrary.create_material_expression(mat, unreal.MaterialExpressionConstant, -380, 200)
        opac.set_editor_property("r", 0.22)
        unreal.MaterialEditingLibrary.connect_material_property(opac, "", unreal.MaterialProperty.MP_OPACITY)
        unreal.MaterialEditingLibrary.recompile_material(mat)
        unreal.EditorAssetLibrary.save_asset(path)
        log("created M_Glass")

    make_glass()
    import_folder_textures(r"C:\Users\Akitt\Games\Studio\unreal\Content\Icons", "/Game/Icons", [".png"])
    import_folder_textures(r"C:\Users\Akitt\Games\Studio\unreal\Content\Art\Posters", "/Game/Art/Posters", [".jpg", ".png"])
    level_editor = unreal.get_editor_subsystem(unreal.LevelEditorSubsystem)
    actor_sub = unreal.get_editor_subsystem(unreal.EditorActorSubsystem)
    editor_sub = unreal.get_editor_subsystem(unreal.UnrealEditorSubsystem)

    world_name = str(editor_sub.get_editor_world())
    log("current world %s" % world_name)
    if "M0_FireEscape" not in world_name:
        if unreal.EditorAssetLibrary.does_asset_exist(ASSET_PATH):
            loaded = level_editor.load_level(ASSET_PATH)
            log("load_level %s" % loaded)
        else:
            try:
                created = level_editor.new_level(ASSET_PATH, False)
            except TypeError:
                created = level_editor.new_level(ASSET_PATH)
            log("new_level %s" % created)
            if not created:
                log("FAILED: new_level returned false")
                return

    world = editor_sub.get_editor_world()
    log("world %s" % world)
    ws = world.get_world_settings()
    log("world_settings %s" % ws)
    try:
        ws.set_editor_property("default_game_mode", unreal.FEGameMode)
        log("default_game_mode = FEGameMode")
    except Exception as exc:
        log("set default_game_mode failed: %s" % exc)

    builder = None
    for actor in actor_sub.get_all_level_actors():
        if isinstance(actor, unreal.FELevelBuilder):
            builder = actor
            break
    if builder is None:
        builder = actor_sub.spawn_actor_from_class(
            unreal.FELevelBuilder,
            unreal.Vector(0.0, 0.0, 0.0),
            unreal.Rotator(pitch=0.0, yaw=0.0, roll=0.0),
        )
    log("builder %s" % builder)
    if builder is None:
        log("FAILED: could not spawn FELevelBuilder")
        return

    builder.force_rebuild()
    try:
        log("build_now returned built=%s" % builder.get_editor_property("b_built"))
    except Exception as exc:
        log("build_now returned (could not read b_built: %s)" % exc)

    actors = actor_sub.get_all_level_actors()
    n_sm = 0
    n_int = 0
    n_tag = 0
    names = []
    for actor in actors:
        names.append(actor.get_name())
        if actor.actor_has_tag("FE_M0"):
            n_tag += 1
        if isinstance(actor, unreal.StaticMeshActor):
            n_sm += 1
        if isinstance(actor, (unreal.FEInteractable, unreal.FELootContainer, unreal.FEPlantSpot, unreal.FEEmber, unreal.FEWaterFixture)):
            n_int += 1
    log("ACTOR_COUNT %d" % len(actors))
    log("STATIC_MESH_COUNT %d" % n_sm)
    log("INTERACTABLE_COUNT %d" % n_int)
    log("TAGGED_COUNT %d" % n_tag)
    log("NAMES %s" % ",".join(names[:60]))

    # Proof angles (do NOT leave viewport on a tight interior ÃƒÂ¢Ã¢â€šÂ¬Ã¢â‚¬Â that jammed wallpaper close-ups).
    shots = [
        (OVERVIEW_LOC, OVERVIEW_ROT, "M0_overview_north"),
        (unreal.Vector(-310.0, 200.0, 160.0), unreal.Rotator(pitch=-8.0, yaw=0.0, roll=0.0), "M0_living_interior"),
        (unreal.Vector(0.0, -700.0, 280.0), unreal.Rotator(pitch=-12.0, yaw=90.0, roll=0.0), "M0_south_hallway"),
        (unreal.Vector(-410.0, 900.0, 220.0), unreal.Rotator(pitch=-15.0, yaw=-40.0, roll=0.0), "M0_balconies"),
        (unreal.Vector(-200.0, 100.0, 140.0), unreal.Rotator(pitch=-5.0, yaw=90.0, roll=0.0), "M0_doors_kitchen"),
    ]
    for loc, rot, name in shots:
        try:
            set_viewport_camera(editor_sub, level_editor, loc, rot, name)
            time.sleep(1.0)
            out = os.path.join(SHOT_DIR, name + ".png")
            try:
                unreal.AutomationLibrary.take_high_res_screenshot(1600, 900, out)
                log("automation shot %s" % out)
            except Exception as exc1:
                log("automation failed %s: %s" % (name, exc1))
                unreal.SystemLibrary.execute_console_command(world, "Shot")
                log("console Shot fallback for %s" % name)
            time.sleep(1.5)
        except Exception as exc:
            log("shot failed %s: %s" % (name, exc))

    door_n = 0
    hall_n = 0
    for actor in actor_sub.get_all_level_actors():
        label = actor.get_actor_label() if hasattr(actor, "get_actor_label") else actor.get_name()
        low = str(label).lower()
        if "door" in low and "frame" not in low:
            door_n += 1
        if "apthall" in low or "hallfloor" in low or "doorhall" in low:
            hall_n += 1
    log("DOOR_LABEL_COUNT %d" % door_n)
    log("HALL_LABEL_COUNT %d" % hall_n)

    # CRITICAL: restore overview before save so opening the map is usable.
    ensure_overview_camera_actor(actor_sub, OVERVIEW_LOC, OVERVIEW_ROT)
    set_viewport_camera(editor_sub, level_editor, OVERVIEW_LOC, OVERVIEW_ROT, "FINAL_OVERVIEW")
    time.sleep(0.5)
    verify_viewport_camera(editor_sub, level_editor, OVERVIEW_LOC)
    try_set_bookmark(level_editor, OVERVIEW_LOC, OVERVIEW_ROT)
    try:
        level_editor.editor_invalidate_viewports()
    except Exception as exc:
        log("invalidate skip: %s" % exc)

    saved = level_editor.save_current_level()
    log("save_current_level %s" % saved)
    unreal.EditorAssetLibrary.save_asset(ASSET_PATH)
    log("asset saved")

    # Signal + hold briefly so an external CaptureWindow can grab a real editor shot.
    for fn in list(os.listdir(FLAG_DIR)):
        try:
            os.remove(os.path.join(FLAG_DIR, fn))
        except Exception:
            pass
    ready_path = os.path.join(FLAG_DIR, "ready_overview_final")
    with open(ready_path, "w", encoding="utf-8") as handle:
        handle.write("1\n")
    log("flag %s" % ready_path)

    # In-editor capture of the overview we just saved.
    try:
        unreal.AutomationLibrary.take_high_res_screenshot(1600, 900, os.path.join(SHOT_DIR, "M0_overview_final.png"))
        log("automation M0_overview_final.png")
    except Exception as exc:
        log("final automation fail: %s" % exc)
        try:
            unreal.SystemLibrary.execute_console_command(world, "HighResShot 1600x900")
            log("HighResShot fallback final")
        except Exception as exc2:
            log("HighResShot fail: %s" % exc2)

    if n_sm < 20:
        log("FAILED: too few static meshes (%d)" % n_sm)
    else:
        log("DONE")

    # Wait for optional external PrintWindow capture, then quit.
    deadline = time.time() + 45.0
    while time.time() < deadline:
        if os.path.exists(os.path.join(FLAG_DIR, "done_overview_final")):
            log("external capture acknowledged")
            break
        time.sleep(0.5)
    time.sleep(1.0)
    try:
        unreal.SystemLibrary.execute_console_command(world, "QUIT_EDITOR")
    except Exception as exc:
        log("quit failed: %s" % exc)


if __name__ == "__main__":
    try:
        main()
    except Exception:
        log(traceback.format_exc())
        raise

