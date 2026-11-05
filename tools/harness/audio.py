"""Audio fixtures use the production structures, enums and capacities."""

from harness import constant, enum_with, read, structure


def manager_types():
    manager = read("source/sound/sound_manager.c")
    # The symbol table comment contains enum spellings before their declarations.
    manager = manager[manager.index("/* ---------- constants */"):]
    definitions = read("source/sound/sound_definitions.h")
    classes = read("source/sound/sound_classes.h")
    text = "typedef unsigned short word;\nstruct sound_preferences;\n"
    for source, member in [(manager, "_sound_impulse ="), (manager, "_sound_waiting_for_cache_bit"),
                           (manager, "_sound_channel_idle"), (manager, "_sound_fade_mode_linear"),
                           (manager, "_fade_in_at_start_bit"), (manager, "_sound_spatialization_mode_none"),
                           (definitions, "_sound_definition_linked_permutations_bit"),
                           (definitions, "_looping_sound_fake_impulse_sound_bit"),
                           (read("source/sound/sound_manager.h"), "_looping_sound_refresh_start")]:
        text += enum_with(source, member) + "\n"
    for source, names in [(classes, ["MAXIMUM_SOUND_INSTANCES_PER_DEFINITION",
                                     "MAXIMUM_SOUND_INSTANCES_PER_OBJECT_PER_DEFINITION"]),
                          (definitions, ["MAXIMUM_DETAIL_SOUNDS_PER_LOOPING_SOUND"]),
                          (read("source/networking/network_connection.h"), ["MAXIMUM_NUMBER_OF_LOCAL_PLAYERS"])]:
        for name in names:
            text += f"#define {name} {constant(source, name)}\n"
    for path, names in [
        ("source/math/real_math.h", ["real_matrix4x3"]),
        ("source/tag_files/tag_groups.h", ["tag_block", "tag_reference", "tag_data"]),
        ("source/objects/objects.h", ["location"]),
        ("source/sound/game_sound.h", ["sound_location"]),
        ("source/sound/sound_environment_definitions.h", ["sound_environment_definition"]),
        ("source/sound/sound_definitions.h", ["real_bounds", "sound_permutation", "sound_pitch_range",
          "sound_definition", "looping_sound_scale_modifiers", "looping_sound_track", "looping_sound_definition"]),
        ("source/sound/sound_classes.h", ["sound_class_definition"]),
        ("source/sound/sound_manager.c", ["sound_source", "sound_listener", "sound_channel_datum",
          "sound_channel_summary", "platform_sound_channel_properties", "platform_sound_listener_properties",
          "sound_datum", "looping_sound_datum", "sound_platform_definition", "sound_manager_globals"]),
    ]:
        for name in names:
            text += structure(read(path), name) + "\n"
            if name == "real_matrix4x3":
                text += "typedef struct real_matrix4x3 real_matrix4x3;\n"
    return text


def stream_types():
    backend = read("port/linux/src/dsound_sdl.c")
    game = read("source/sound/sound_dsound_xbox.c")
    xdk = read("port/include/xdk/xdk_pdb.h")
    text = """typedef unsigned long DWORD, ULONG;
typedef long LONG, HRESULT;
typedef int BOOL;
typedef void *LPVOID, *LPDIRECTSOUND, *LPDIRECTSOUNDBUFFER;
typedef long long __int64;
typedef void (*LPFNXMEDIAOBJECTCALLBACK)(void *, void *, unsigned long);
#define CALLBACK
#define STDMETHODCALLTYPE
#define S_OK 0
"""
    statuses = read("port/include/xdk/xdk_dsound.h")
    for name in ["XMEDIAPACKET_STATUS_SUCCESS", "XMEDIAPACKET_STATUS_FLUSHED",
                 "XMEDIAPACKET_STATUS_FAILURE", "XMEDIAPACKET_STATUS_PENDING"]:
        # These SDK status expressions include casts, unlike integer capacities.
        text += next(line for line in statuses.splitlines() if line.startswith("#define " + name + " ")) + "\n"
    for name in ["MAXIMUM_STREAM_PACKETS", "RESAMPLER_HISTORY", "MAXIMUM_OUTPUT_CHANNELS", "CUT_FADE_FRAMES"]:
        text += f"#define {name} {constant(backend, name)}\n"
    for name in ["MAXIMUM_SOUND_CHANNELS", "NUMBER_OF_SOUND_CHANNEL_TYPES"]:
        text += f"#define {name} {constant(game, name)}\n"
    text += enum_with(game, "_sound_channel_idle") + "\n"
    text += structure(xdk, "IDirectSoundStream") + "\n"
    text += structure(xdk, "_XMEDIAPACKET") + "\n"
    text += structure(xdk, "_DSCAPS") + "\n"
    text += """typedef struct IDirectSoundStream IDirectSoundStream, *LPDIRECTSOUNDSTREAM;
typedef struct _XMEDIAPACKET XMEDIAPACKET;
typedef struct _DSCAPS DSCAPS;
"""
    text += structure(read("source/sound/sound_environment_definitions.h"), "sound_environment_definition") + "\n"
    for name in ["sound_virtual_channel", "sound_channel", "dsound_globals"]:
        text += structure(game, name) + "\n"
    for name in ["voice_packet", "sdl_stream"]:
        text += structure(backend, name) + "\n"
    return text
