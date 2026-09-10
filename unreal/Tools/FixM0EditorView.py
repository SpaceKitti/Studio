"""Aim M0_FireEscape editor viewport at HomeFloor overview and save (no rebuild)."""
import os
import time
import traceback
import unreal

LOG = r"C:\Users\Akitt\Games\Studio\unreal\Saved\Logs\FixM0EditorView.txt"
ASSET_PATH = "/Game/Maps/M0_FireEscape"
SHOT_DIR = r"C:\Users\Akitt\Games\Studio\unreal\Saved\Screenshots\WindowsEditor"
FLAG_DIR = r"C:\Users\Akitt\Games\Studio\unreal\Saved\capture_flags"
# High birdseye over Home living/balcony â€” rooms + doors readable.
# NOTE: unreal.Rotator positional is (roll, pitch, yaw) â€” always use keywords.
OVERVIEW_LOC = unreal.Vector(-200.0, 1200.0, 650.0)
OVERVIEW_ROT = unreal.Rotator(pitch=-28.0, yaw=-110.0, roll=0.0)
OVERVIEW_CAM_LABEL = "FE_EditorOverviewCam"


def log(msg):
    print(msg)
    os.makedirs(os.path.dirname(LOG), exist_ok=True)
    with open(LOG, "a", encoding="utf-8") as handle:
        handle.write(str(msg) + "\n")


def set_all_viewports(editor_sub, level_editor, loc, rot, tag=""):
    used = []
    try:
        editor_sub.set_level_viewport_camera_info(loc, rot)
        used.append("UES")
    except Exception as exc:
        log("UES set fail: %s" % exc)
    keys = []
    try:
        keys = list(level_editor.get_viewport_config_keys())
        log("viewport keys: %s" % [str(k) for k in keys])
    except Exception as exc:
        log("get_viewport_config_keys fail: %s" % exc)
    try:
        active = level_editor.get_active_viewport_config_key()
        log("active viewport key: %s" % active)
        if active and active not in keys:
            keys.append(active)
    except Exception as exc:
        log("get_active_viewport_config_key fail: %s" % exc)
    if not keys:
        keys = [unreal.Name("Perspective"), unreal.Name("None")]
    for key in keys:
        try:
            level_editor.set_level_viewport_camera_info(loc, rot, key)
            try:
                level_editor.set_level_viewport_fov(90.0, key)
            except Exception:
                pass
            try:
                level_editor.editor_set_viewport_realtime(True, key)
            except Exception:
                pass
            try:
                level_editor.editor_set_game_view(False, key)
            except Exception:
                pass
            used.append("LES:%s" % key)
        except Exception as exc:
            log("LES set fail key=%s: %s" % (key, exc))
    log("cam set %s via %s -> %s %s" % (tag, used, loc, rot))
    return used


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
        existing = actor_sub.spawn_actor_from_class(unreal.CameraActor, loc, rot)
        if existing is None:
            log("FAILED spawn CameraActor")
            return None
        try:
            existing.set_actor_label(OVERVIEW_CAM_LABEL)
        except Exception:
            pass
        log("spawned %s" % OVERVIEW_CAM_LABEL)
    else:
        existing.set_actor_location(loc, False, True)
        existing.set_actor_rotation(rot, False)
        log("updated %s" % OVERVIEW_CAM_LABEL)
    try:
        existing.set_folder_path("M0")
    except Exception:
        pass
    try:
        if "FE_M0" not in list(existing.tags):
            existing.tags.append("FE_M0")
    except Exception:
        pass
    return existing


def select_homefloor(actor_sub):
    targets = []
    for actor in actor_sub.get_all_level_actors():
        try:
            label = actor.get_actor_label()
        except Exception:
            label = actor.get_name()
        if label in ("HomeFloor", "HomeLivFloor", "HomeSlidingGlass", "FE_EditorOverviewCam"):
            targets.append(actor)
    if targets:
        try:
            actor_sub.set_selected_level_actors(targets[:3])
            log("selected %s" % [a.get_actor_label() for a in targets[:3]])
        except Exception as exc:
            log("select fail: %s" % exc)
    return targets


def main():
    os.makedirs(os.path.dirname(LOG), exist_ok=True)
    os.makedirs(SHOT_DIR, exist_ok=True)
    os.makedirs(FLAG_DIR, exist_ok=True)
    with open(LOG, "w", encoding="utf-8") as handle:
        handle.write("")
    log("BEGIN FixM0EditorView")

    level_editor = unreal.get_editor_subsystem(unreal.LevelEditorSubsystem)
    actor_sub = unreal.get_editor_subsystem(unreal.EditorActorSubsystem)
    editor_sub = unreal.get_editor_subsystem(unreal.UnrealEditorSubsystem)

    world_name = str(editor_sub.get_editor_world())
    log("world %s" % world_name)
    if "M0_FireEscape" not in world_name:
        loaded = level_editor.load_level(ASSET_PATH)
        log("load_level %s" % loaded)

    world = editor_sub.get_editor_world()
    cam = ensure_overview_camera_actor(actor_sub, OVERVIEW_LOC, OVERVIEW_ROT)
    used = set_all_viewports(editor_sub, level_editor, OVERVIEW_LOC, OVERVIEW_ROT, "overview")

    # Pilot camera actor briefly to force viewport onto that transform, then eject.
    if cam is not None:
        try:
            level_editor.pilot_level_actor(cam)
            log("piloting overview cam")
            time.sleep(0.5)
            level_editor.eject_pilot_level_actor()
            log("ejected pilot")
            # Re-apply explicit camera after eject (pilot can leave odd state)
            set_all_viewports(editor_sub, level_editor, OVERVIEW_LOC, OVERVIEW_ROT, "post_pilot")
        except Exception as exc:
            log("pilot fail: %s" % exc)

    select_homefloor(actor_sub)
    time.sleep(1.0)
    try:
        info = editor_sub.get_level_viewport_camera_info()
        if info:
            loc, rot = info[0], info[1]
            log("get_cam loc=(%.1f,%.1f,%.1f) pitch=%.1f yaw=%.1f roll=%.1f" % (
                loc.x, loc.y, loc.z, rot.pitch, rot.yaw, rot.roll))
        else:
            log("get_cam empty")
    except Exception as exc:
        log("get_cam fail: %s" % exc)
    try:
        level_editor.editor_invalidate_viewports()
    except Exception as exc:
        log("invalidate: %s" % exc)

    # Give GPU a couple frames to redraw before save/capture.
    time.sleep(2.5)

    saved = level_editor.save_current_level()
    log("save_current_level %s via=%s" % (saved, used))
    unreal.EditorAssetLibrary.save_asset(ASSET_PATH)
    log("asset saved")

    for fn in list(os.listdir(FLAG_DIR)):
        try:
            os.remove(os.path.join(FLAG_DIR, fn))
        except Exception:
            pass
    ready = os.path.join(FLAG_DIR, "ready_overview_final")
    with open(ready, "w", encoding="utf-8") as handle:
        handle.write("1\n")
    log("flag %s" % ready)

    try:
        unreal.AutomationLibrary.take_high_res_screenshot(
            1600, 900, os.path.join(SHOT_DIR, "M0_overview_final.png")
        )
        log("automation M0_overview_final.png")
    except Exception as exc:
        log("automation fail: %s" % exc)
    try:
        unreal.SystemLibrary.execute_console_command(world, "HighResShot 1600x900")
        log("HighResShot issued")
    except Exception as exc:
        log("HighResShot fail: %s" % exc)

    deadline = time.time() + 90.0
    while time.time() < deadline:
        if os.path.exists(os.path.join(FLAG_DIR, "done_overview_final")):
            log("external capture done")
            break
        time.sleep(0.4)

    log("DONE")
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