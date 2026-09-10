import os
import traceback
import unreal

LOG = r"C:\Users\Akitt\Games\Studio\unreal\Saved\Logs\BuildM0Map.txt"
ASSET_PATH = "/Game/Maps/M0_FireEscape"


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


def main():
    os.makedirs(os.path.dirname(LOG), exist_ok=True)
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
            unreal.Rotator(0.0, 0.0, 0.0),
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


    import time
    shot_dir = r"C:\Users\Akitt\Games\Studio\unreal\Saved\Screenshots\WindowsEditor"
    os.makedirs(shot_dir, exist_ok=True)
    shots = [
        (unreal.Vector(0.0, 1400.0, 420.0), unreal.Rotator(-18.0, -90.0, 0.0), "M0_overview_north"),
        (unreal.Vector(-310.0, 200.0, 160.0), unreal.Rotator(-8.0, 0.0, 0.0), "M0_living_interior"),
        (unreal.Vector(0.0, -700.0, 280.0), unreal.Rotator(-12.0, 90.0, 0.0), "M0_south_hallway"),
        (unreal.Vector(-410.0, 900.0, 220.0), unreal.Rotator(-15.0, -40.0, 0.0), "M0_balconies"),
        (unreal.Vector(-200.0, 100.0, 140.0), unreal.Rotator(-5.0, 90.0, 0.0), "M0_doors_kitchen"),
    ]
    for loc, rot, name in shots:
        try:
            unreal.EditorLevelLibrary.set_level_viewport_camera_info(loc, rot)
            time.sleep(1.0)
            out = os.path.join(shot_dir, name + ".png")
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

    saved = level_editor.save_current_level()
    log("save_current_level %s" % saved)
    unreal.EditorAssetLibrary.save_asset(ASSET_PATH)
    log("asset saved")

    if n_sm < 20:
        log("FAILED: too few static meshes (%d)" % n_sm)
    else:
        log("DONE")

    time.sleep(2.0)
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
