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


def main():
    os.makedirs(os.path.dirname(LOG), exist_ok=True)
    with open(LOG, "w", encoding="utf-8") as handle:
        handle.write("")
    log("BEGIN")

    asset_tools = unreal.AssetToolsHelpers.get_asset_tools()
    level_editor = unreal.get_editor_subsystem(unreal.LevelEditorSubsystem)
    actor_sub = unreal.get_editor_subsystem(unreal.EditorActorSubsystem)
    editor_sub = unreal.get_editor_subsystem(unreal.UnrealEditorSubsystem)

    if unreal.EditorAssetLibrary.does_asset_exist(ASSET_PATH):
        log("Deleting existing M0_FireEscape")
        unreal.EditorAssetLibrary.delete_asset(ASSET_PATH)

    # new_level creates the map and makes it current. Do not load_level after —
    # that double-opens the world and fatals in -unattended.
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

    builder = actor_sub.spawn_actor_from_class(
        unreal.FELevelBuilder,
        unreal.Vector(0.0, 0.0, 0.0),
        unreal.Rotator(0.0, 0.0, 0.0),
    )
    log("builder %s" % builder)
    if builder is None:
        log("FAILED: could not spawn FELevelBuilder")
        return

    builder.build_now()
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
        if isinstance(actor, (unreal.FEInteractable, unreal.FELootContainer, unreal.FEPlantSpot, unreal.FEEmber)):
            n_int += 1
    log("ACTOR_COUNT %d" % len(actors))
    log("STATIC_MESH_COUNT %d" % n_sm)
    log("INTERACTABLE_COUNT %d" % n_int)
    log("TAGGED_COUNT %d" % n_tag)
    log("NAMES %s" % ",".join(names[:60]))

    try:
        unreal.EditorLevelLibrary.set_level_viewport_camera_info(
            unreal.Vector(210.0, -650.0, 220.0),
            unreal.Rotator(-12.0, 90.0, 0.0),
        )
        log("viewport camera set")
    except Exception as exc:
        log("camera failed: %s" % exc)

    saved = level_editor.save_current_level()
    log("save_current_level %s" % saved)
    unreal.EditorAssetLibrary.save_asset(ASSET_PATH)
    log("asset saved")

    if n_sm < 20:
        log("FAILED: too few static meshes (%d)" % n_sm)
    else:
        log("DONE")

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
