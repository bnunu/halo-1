/*
MAIN.C

symbols in this file:
000EF6F0 0010:
	_main_get_seconds_elapsed (0000)
000EF700 0080:
	_gamepad_button_is_down (0000)
000EF780 0010:
	_game_connection_set (0000)
000EF790 0010:
	_game_connection (0000)
000EF7A0 0010:
	_main_disallow_persistent_storage (0000)
000EF7B0 0060:
	_main_set_map_name (0000)
000EF810 0010:
	_main_defer_map_map_change (0000)
000EF820 0030:
	_main_set_multiplayer_map_name (0000)
000EF850 0010:
	_main_get_map_name (0000)
000EF860 0010:
	_main_get_multiplayer_map_name (0000)
000EF870 0020:
	_main_set_difficulty (0000)
000EF890 0010:
	_main_get_difficulty (0000)
000EF8A0 0040:
	_sort_desired_local_player_controllers (0000)
000EF8E0 01d0:
	_code_000ef8e0 (0000)
000EFAB0 0020:
	_main_reset_map (0000)
000EFAD0 0020:
	_main_revert_map (0000)
000EFAF0 0020:
	_main_skip_cinematic (0000)
000EFB10 0010:
	_main_save_map_nonsafe (0000)
000EFB20 0010:
	_main_saving_map (0000)
000EFB30 0010:
	_main_save_cancel (0000)
000EFB40 0040:
	_main_save_map_safe (0000)
000EFB80 0010:
	_main_won_map (0000)
000EFB90 0010:
	_main_lost_map (0000)
000EFBA0 0020:
	_main_respawn (0000)
000EFBC0 0020:
	_main_save_core (0000)
000EFBE0 0050:
	_main_save_core_name (0000)
000EFC30 0020:
	_main_load_core (0000)
000EFC50 0020:
	_main_load_core_at_startup (0000)
000EFC70 0050:
	_main_load_core_name (0000)
000EFCC0 0050:
	_main_load_core_name_at_startup (0000)
000EFD10 0060:
	_main_switch_structure_bsp (0000)
000EFD70 0030:
	_main_skip (0000)
000EFDA0 0040:
	_main_queue_map_name (0000)
000EFDE0 0050:
	_code_000efde0 (0000)
000EFE30 0020:
	_main_goto_main_menu (0000)
000EFE50 0050:
	_main_menu_precache_resources (0000)
000EFEA0 0020:
	_main_menu_unload (0000)
000EFEC0 0010:
	_main_menu_ensure_player_queues_exist (0000)
000EFED0 0010:
	_main_menu_fade_active (0000)
000EFEE0 0010:
	_main_menu_switch_to_single_player (0000)
000EFEF0 0010:
	_main_set_game_connection_to_film_playback (0000)
000EFF00 0170:
	_main_get_solo_level_from_name (0000)
000F0070 0010:
	_main_get_current_solo_level (0000)
000F0080 0020:
	_main_get_solo_level_name (0000)
000F00A0 0010:
	_main_run_demos (0000)
000F00B0 0070:
	_code_000f00b0 (0000)
000F0120 01f0:
	_compute_window_bounds (0000)
000F0310 0040:
	_main_get_window_count (0000)
000F0350 00d0:
	_code_000f0350 (0000)
000F0420 0180:
	_code_000f0420 (0000)
000F05A0 0020:
	_code_000f05a0 (0000)
000F05C0 0030:
	_code_000f05c0 (0000)
000F05F0 00b0:
	_code_000f05f0 (0000)
000F06A0 0020:
	_code_000f06a0 (0000)
000F06C0 00e0:
	_code_000f06c0 (0000)
000F07A0 0030:
	_code_000f07a0 (0000)
000F07D0 0040:
	_code_000f07d0 (0000)
000F0810 0040:
	_code_000f0810 (0000)
000F0850 0060:
	_code_000f0850 (0000)
000F08B0 0020:
	_code_000f08b0 (0000)
000F08D0 0020:
	_code_000f08d0 (0000)
000F08F0 0040:
	_code_000f08f0 (0000)
000F0930 0010:
	_code_000f0930 (0000)
000F0940 0220:
	_code_000f0940 (0000)
000F0B60 0050:
	_code_000f0b60 (0000)
000F0BB0 0030:
	_code_000f0bb0 (0000)
000F0BE0 0010:
	_main_framerate_throttle_enabled (0000)
000F0BF0 05a0:
	_code_000f0bf0 (0000)
000F1190 0210:
	_main_rasterizer_throttle (0000)
000F13A0 0020:
	_code_000f13a0 (0000)
000F13C0 0020:
	_main_taking_screenshot (0000)
000F13E0 00b0:
	_main_movie_start (0000)
000F1490 0020:
	_main_movie_stop (0000)
000F14B0 0010:
	_main_stop_time (0000)
000F14C0 0010:
	_main_start_time (0000)
000F14D0 0010:
	_main_crash (0000)
000F14E0 0010:
	_main_print_version (0000)
000F14F0 00c0:
	_main_vertical_blank_interrupt_handler (0000)
000F15B0 0070:
	_main_save_current_solo_map (0000)
000F1620 00c0:
	_main_load_last_solo_map (0000)
000F16E0 0040:
	_main_save_map_no_timeout (0000)
000F1720 00e0:
	_main_load_ui_scenario (0000)
000F1800 0090:
	_main_menu_load (0000)
000F1890 0020:
	_main_roll_credits (0000)
000F18B0 0130:
	_main_pregame_render (0000)
000F19E0 01b0:
	_set_window_camera_values (0000)
000F1B90 0090:
	_main_present_frame (0000)
000F1C20 00c0:
	_code_000f1c20 (0000)
000F1CE0 0050:
	_code_000f1ce0 (0000)
000F1D30 01f0:
	_code_000f1d30 (0000)
000F1F20 02a0:
	_main_framerate_render (0000)
000F21C0 02a0:
	_halt_and_catch_fire (0000)
000F2460 0040:
	_main_loop_of_death (0000)
000F24A0 01c0:
	_code_000f24a0 (0000)
000F2660 0670:
	_main_loop (0000)
002795A8 003a:
	??_C@_0DK@GMPCKHNN@button_index?$DO?$DN0?5?$CG?$CG?5button_index?$DM@ (0000)
002795E4 001b:
	??_C@_0BL@EKABHJPH@c?3?2halo?2SOURCE?2main?2main?4c?$AA@ (0000)
00279600 0037:
	??_C@_0DH@DGKBOONK@?$CIgamepad_index?$DO?$DN0?$CJ?5?$CG?$CG?5?$CIgamepad_i@ (0000)
00279638 0013:
	??_C@_0BD@OOOKAGFE@j?$DMMAXIMUM_GAMEPADS?$AA@ (0000)
00279650 0049:
	??_C@_0EJ@DIAKEACK@?$CIdesired_controllers?$FLi?$FN?$DO?$DN0?$CJ?5?$CG?$CG?5?$CI@ (0000)
0027969C 002c:
	??_C@_0CM@EOEJOPGF@game_connection?$CI?$CJ?5?$DN?$DN?5_game_conne@ (0000)
002796C8 0009:
	??_C@_08FLLKDKH@core?4bin?$AA@ (0000)
002796D4 003b:
	??_C@_0DL@LKJEADLD@warning?0?5core?5file?5name?5will?5be?5@ (0000)
00279710 002c:
	??_C@_0CM@BPJFOCNH@tried?5to?5switch?5to?5invalid?5struc@ (0000)
0027973C 002c:
	??_C@_0CM@IOHNOMHF@tried?5to?5switch?5to?5current?5struc@ (0000)
00279768 0030:
	??_C@_0DA@FGIPPFOE@cannot?5skip?5more?5than?515?5frames?5@ (0000)
00279798 0029:
	??_C@_0CJ@GNBMIDNC@scenario?9?$DOtype?$DN?$DN_scenario_type_m@ (0000)
002797C4 000e:
	??_C@_0O@OJNPMJKK@num_players?$DO0?$AA@ (0000)
002797D4 0039:
	??_C@_0DJ@LGIIGICN@horizontal_index?$DO?$DN0?5?$CG?$CG?5horizonta@ (0000)
00279810 0033:
	??_C@_0DD@NNOBFMAG@vertical_index?$DO?$DN0?5?$CG?$CG?5vertical_in@ (0000)
00279844 0019:
	??_C@_0BJ@LDIFOEOI@player_index?$DMnum_players?$AA@ (0000)
00279860 0017:
	??_C@_0BH@PGKHPIFN@main_new_map?$CI?$CJ?5failed?4?$AA@ (0000)
00279878 0014:
	??_C@_0BE@HPJEDLPC@game_load?$CI?$CJ?5failed?4?$AA@ (0000)
00279890 0041:
	??_C@_0EB@NLDLOJCN@manual?5skipping?5doesn?8t?5work?5out@ (0000)
002798D4 000c:
	??_C@_0M@NBLPJHAH@unsafe?5save?$AA@ (0000)
002798E0 0017:
	??_C@_0BH@JAFGAFKC@gave?5up?5trying?5to?5save?$AA@ (0000)
002798F8 001f:
	??_C@_0BP@JOHDPJOL@?$DLcore_load_name_at_startup?5?$CFs?6?$AA@ (0000)
00279918 0004:
	??_C@_03CCGKKFHG@a?$CLt?$AA@ (0000)
0027991C 000d:
	??_C@_0N@HMFDKLFI@map_name?5?$CFs?6?$AA@ (0000)
0027992C 0003:
	??_C@_02BKFDOEMK@wt?$AA@ (0000)
00279930 000f:
	??_C@_0P@JFNDGJMB@d?3?2?$CFs_init?4txt?$AA@ (0000)
00279940 001e:
	??_C@_0BO@EFIKKGLE@?$CFs_slow_?$CFd_?$CFd_?$CFd_?$CFd_?$CFd_?$CFd?4bin?$AA@ (0000)
00279960 0008:
	__real@3fa26e978d4fdf3c (0000)
00279968 0013:
	??_C@_0BD@BANPCAEP@?5des?5?$CFd?5targ?$CF6I64d?$AA@ (0000)
0027997C 000d:
	??_C@_0N@ONKBMNKN@?5MAINTAIN?5?$CFd?$AA@ (0000)
0027998C 000d:
	??_C@_0N@MLBLHBF@?5RESTORE?5?5?$CFd?$AA@ (0000)
0027999C 000d:
	??_C@_0N@KHJNGILI@?5FAILDOWN?5?$CFd?$AA@ (0000)
002799AC 000d:
	??_C@_0N@KAJPDIPB@?$CI?$CFs?$CF2d?1?$CF2d?$CJ?5?$AA@ (0000)
002799BC 0003:
	??_C@_02LEBOADDI@dn?$AA@ (0000)
002799C0 000c:
	??_C@_0M@FONKJGHM@?$CIok?5?5?5?$CF2d?$CJ?5?$AA@ (0000)
002799CC 0006:
	??_C@_05MPKPLMMK@ignor?$AA@ (0000)
002799D4 0009:
	??_C@_08NGBFCLJE@?$CI?$CFs?$CF2d?$CJ?5?$AA@ (0000)
002799E0 0006:
	??_C@_05KGKFECIJ@fail?5?$AA@ (0000)
002799E8 003a:
	??_C@_0DK@KGLGNIMB@last?$CF6I64d?5init?$CF6I64d?5achv?$CF6I64d@ (0000)
00279A24 0019:
	??_C@_0BJ@GBFFHJIM@?$CF6I64d?$CItarg?$CF6I64d?5?$CFs?$CF2d?$CJ?$AA@ (0000)
00279A40 0009:
	??_C@_08EEAEDAMO@LAPSED?5?5?$AA@ (0000)
00279A4C 0009:
	??_C@_08KMBEHCPI@SYNCED?5?5?$AA@ (0000)
00279A58 0009:
	??_C@_08PLAGFMEO@THROTTLE?$AA@ (0000)
00279A68 004a:
	??_C@_0EK@OBDHCGCK@stuck?5waiting?5for?5VBLANK?5callbac@ (0000)
00279AB4 0006:
	??_C@_05PAONDCEJ@movie?$AA@ (0000)
00279ABC 0019:
	??_C@_0BJ@MFOKCFOP@main_globals?4movie?$DN?$DNNULL?$AA@ (0000)
00279AD8 002a:
	??_C@_0CK@IJIKLLCA@chucky?5was?5here?$CB?5?5NULL?5belongs?5t@ (0000)
00279B04 0031:
	??_C@_0DB@HPIOCFDH@halobeta?5xbox?501?401?414?42342?5Jan?5@ (0000)
00279B38 0038:
	??_C@_0DI@IKBOMHNJ@Couldn?8t?5create?5a?5file?5to?5write?5@ (0000)
00279B70 0011:
	??_C@_0BB@HAHOOKLD@z?3?2last_solo?4txt?$AA@ (0000)
00279B84 0028:
	??_C@_0CI@KPDFMEMF@?$CBmain_globals?4main_menu_scenario@ (0000)
00279BAC 000d:
	??_C@_0N@CBIELCEF@levels?2ui?2ui?$AA@ (0000)
00279BBC 0023:
	??_C@_0CD@IMDNHNHI@congratulations?0?5you?5won?5the?5gam@ (0000)
00279BE0 0014:
	??_C@_0BE@BDDOCJAN@movie?2frame?$CF06d?4tga?$AA@ (0000)
00279BF4 0019:
	??_C@_0BJ@CKCGDFO@error?5opening?5saved?5film?$AA@ (0000)
00279C10 0015:
	??_C@_0BF@LPIHHLNO@?$CFdscreenshot?$CFd?$CFd?4tif?$AA@ (0000)
00279C28 003b:
	??_C@_0DL@LMMABMNB@halobeta?5xbox?501?401?414?42342?5buil@ (0000)
00279C64 001f:
	??_C@_0BP@HEAODFIJ@old?5tags?2internal?5system?5plain?$AA@ (0000)
00279C84 0012:
	??_C@_0BC@FLAPBDJB@end?5of?5saved?5film?$AA@ (0000)
00279C98 0018:
	??_C@_0BI@HBMJOOEK@the?5game?5host?5went?5down?$AA@ (0000)
00307818 0034:
	_global_difficulty_level (0000)
	_player_spawn_count (0004)
	_global_frame_rate_throttle (0006)
	_global_screenshot_size (0008)
00455750 0a63:
	_bss_00455750 (0000)
	_debug_force_frame_rate_update (06d0)
	_debug_no_drawing (06d1)
	_debug_game_save (06d2)
	_debug_frame_rate (06d3)
	_display_framerate (06d4)
	_display_vblank_deltas (06d5)
	_display_precache_progress (06d6)
	_global_screenshot_count (06d8)
*/

/* ---------- headers */

#include "cseries.h"
#include "errors.h"
#include "cseries/profile.h"
#include "cseries/cseries_windows.h"
#include "main.h"
#include "real_math.h"
#include "game.h"
#include "game_engine.h"
#include "camera/camera_scripting.h"
#include "game/cheats.h"
#include "game/player_control_runtime.h"
#include "game/players.h"
#include "game/local_players.h"
#include "game/player_queues_new.h"
#include "integer_math.h"
#include "input.h"
#include "input/input_abstraction.h"
#include "shell.h"
#include "event_manager.h"
#include "telnet_console.h"
#include "ui_widget.h"
#include "editor_stubs.h"
#include "console.h"
#include "debug_keys.h"
#include "scenario.h"
#include "scenario/scenario_definitions.h"
#include "cache/cache_files.h"
#include "cache/predicted_resources.h"
#include "bitmaps/bitmap_group.h"
#include "bitmaps/bitmaps.h"
#include "bitmaps/tiff_file.h"
#include "interface/hud.h"
#include "interface/hud_definitions.h"
#include "interface/attract_mode.h"
#include "interface/interface.h"
#include "interface/marketing_and_strategic_business_development.h"
#include "interface/player_ui.h"
#include "interface/terminal.h"
#include "saved games/player_profile.h"
#include "saved games/game_state.h"
#include "sound/sound_manager.h"
#include "rasterizer/rasterizer.h"
#include "rasterizer/rasterizer_console_vars.h"
#include "bink/bink_playback.h"
#include "main/d3d_intimacy.h"
#include "networking/network_game_globals.h"
#include "networking/network_game_manager.h"
#include "networking/network_server_manager.h" /* port: a co-op game's level won */
#include "network_coop.h"
#include "coop_scripts.h" /* port: port/linux/game/coop_scripts.c */
#include "network_voice.h" /* port: port/linux/game/network_voice.c */
#include "network_chat.h" /* port: port/linux/game/network_chat.c */
#include "network_aim_assist.h" /* port: port/linux/game/network_aim_assist.c */
#include "network_shield_effects.h" /* port: port/linux/game/network_shield_effects.c */
#include "network_riding_map.h" /* port: port/linux/game/network_riding_map.c */
#include "map_share.h" /* port: port/linux/game/map_share.c */
#include "profile_sections.h" /* port: port/linux/include/profile_sections.h */
#ifdef HALO_PROFILE
#include "profile_trace.h" /* port: port/linux/src/profile_trace.c */
#endif
#ifdef HALO_WEB
#include <emscripten.h> /* port: the browser runs the main loop (port/web/README.md) */
#endif
#include "camera/director.h"
#include "camera/observer.h"
#include "cutscene/cinematics.h"
#include "effects/player_effects.h"
#include "physics/collision_usage.h"
#include "physics/collision_debug.h"
#include "hs/hs.h"
#include "render/render.h"
#include "text/draw_string.h"
#include "text/font_group.h"
#include "tag_files/files.h"
#include "custom_edition_cache.h" /* port: custom_edition_level_name */
#include "mcc_cache.h" /* port: port/linux/game/mcc_cache.h */
#include "mcc_main.h" /* port: port/linux/game/mcc_main.h */
#include "custom_edition_maps.h" /* port: main_script_map_name, a Custom Edition campaign's maps */
#include "view_fov.h" /* port: display.fov, before the projection and culling */

#if defined(HALO_WINDOWS) || defined(HALO_ANDROID) || defined(__linux__)
#define HALO_NATIVE_BUILD_INFO 1
#ifndef HALO_BUILD_NUMBER
#define HALO_BUILD_NUMBER 0
#endif
#ifndef HALO_BUILD_FLAVOR
#define HALO_BUILD_FLAVOR "local"
#endif
#endif

/* ---------- constants */

enum
{
	_bitmap_format_x8r8g8b8 = 10,
};

enum
{
	_text_justification_left = 0,
	_text_justification_right,
};

/* ---------- macros */

/* ---------- structures */

/*
struct _main_globals
{
	long long last_time_clocks; // 0x0
	unsigned long last_time_msec; // 0x8
	long long last_render_clocks; // 0x10
	long long last_vblank_index; // 0x18
	long long last_initial_vblank_index; // 0x20
	long long last_achievable_vblank_index; // 0x28
	long long last_present_vblank_index; // 0x30
	unsigned char did_time_overflow_occur; // 0x38
	float seconds_elapsed; // 0x3C
	short connection; // 0x40
	unsigned short screenshot_identifier; // 0x42
	bitmap_data *movie; // 0x44
	long recording_start_tick; // 0x48
	long recording_stop_tick; // 0x4C
	long recording_frame_index; // 0x50
	float recording_dt; // 0x54
	unsigned char reset_map; // 0x58
	unsigned char rename_map; // 0x59
	unsigned char revert_map; // 0x5A
	unsigned char skip_cinematic; // 0x5B
	unsigned char save_map; // 0x5C
	unsigned char save_map_safely; // 0x5D
	unsigned char save_map_timeout; // 0x5E
	unsigned char saving_map; // 0x5F
	long ticks_until_next_save_check; // 0x60
	long ticks_unable_to_save; // 0x64
	unsigned long map_change_load_timer; // 0x68
	short safe_intervals; // 0x6C
	unsigned char won_map; // 0x6E
	unsigned char lost_map; // 0x6F
	unsigned char respawn; // 0x70
	unsigned char save_core; // 0x71
	unsigned char load_core; // 0x72
	unsigned char load_core_at_startup; // 0x73
	short switch_to_structure_bsp_index; // 0x74
	unsigned char main_menu_scenario_loaded; // 0x76
	unsigned char want_to_be_at_main_menu; // 0x77
	unsigned char run_xdemos; // 0x78
	unsigned char fade_to_dashboard; // 0x79
	unsigned char exit_to_dashboard; // 0x7A
	unsigned char want_to_exit; // 0x7B
	long idle_timeout; // 0x7C
	long idle_last_interesting; // 0x80
	long idle_last_activity; // 0x84
	unsigned char playback_last_recording; // 0x88
	unsigned char halt_time_scale; // 0x89
	unsigned char restart_time; // 0x8A
	unsigned char load_last_solo_level; // 0x8B
	unsigned char cutscene_skip; // 0x8C
	short skip_ticks; // 0x8E
	short loss_timer; // 0x90
	short respawn_timer; // 0x92
	unsigned char queue_map; // 0x94
	unsigned char pad0[3]; // 0x95
	unsigned char solo_try_and_load_from_persistent_storage; // 0x98
	char soloplayer_map_name[256]; // 0x99
	char multiplayer_map_name[256]; // 0x199
	char queued_map_name[256]; // 0x299
	unsigned char want_to_connect; // 0x399
	char connect_address[32]; // 0x39A
	char connect_password[9]; // 0x3BA
	short vblank_interval_current; // 0x3C4
	short vblank_interval_minimum; // 0x3C6
	unsigned char vblank_interval_held; // 0x3C8
	short vblank_failure_count[6]; // 0x3CA
	long long vblank_last_failure_time[6]; // 0x3D8
	unsigned long *vblank_flip_counter; // 0x408
	short vblank_flip_delta_next_index; // 0x40C
	short vblank_flip_deltas[15]; // 0x40E
};
*/

struct _main_globals
{
	unsigned long frame_start_milliseconds;
	byte reserved04[4];
	union
	{
		__int64 rasterizer_target_index;
		struct
		{
			unsigned long rasterizer_frame_index;
			unsigned long rasterizer_vertical_blank_index;
		};
	};
	__int64 rasterizer_initial_index;
	__int64 rasterizer_throttle_start_index;
	__int64 rasterizer_throttle_end_index;
	real seconds_elapsed;
	short connection;
	unsigned short screenshot_identifier;
	struct bitmap_data *movie;
	long recording_start_tick;
	long recording_stop_tick;
	long recording_frame_index;
	real recording_dt;
	boolean reset_map;
	boolean defer_map_change;
	boolean revert_map;
	boolean skip_cinematic;
	boolean saving_map;
	boolean save_map_safely;
	boolean save_map_timeout;
	boolean save_map_completed;
	long ticks_until_next_save_check;
	long ticks_unable_to_save;
	unsigned long map_change_load_timer;
	short safe_intervals;
	boolean won_map;
	boolean lost_map;
	boolean respawn;
	boolean save_core;
	boolean load_core;
	boolean load_core_at_startup;
	short switch_to_structure_bsp_index;
	boolean main_menu_scenario_loaded;
	boolean want_to_be_at_main_menu;
	boolean run_xdemos;
	boolean playback_last_recording;
	boolean halt_time_scale;
	boolean restart_time;
	boolean load_last_solo_level;
	boolean skip;
	short skip_ticks;
	short loss_timer;
	short respawn_timer;
	boolean queue_map;
	byte __unknown121[0x3];
	boolean allow_persistent_storage;
	char soloplayer_map_name[256];
	char multiplayer_map_name[256];
	char queued_map_name[256];
	char core_name[64];
	byte reserved3B5;
	short vblank_interval_current;
	short vblank_interval_requested;
	boolean vblank_interval_held;
	byte reserved3BB;
	short vblank_failure_counts[6];
	__int64 vblank_last_failure_indices[6];
	volatile unsigned int *d3d_flip_count;
	short vblank_flip_delta_index;
	short vblank_flip_deltas[15];
	char vblank_debug_string[0x200];
	byte reserved61C[4];
};

typedef char main_hud_globals_font_tag_index_offset_assert[
	offsetof(struct hud_globals_definition, messaging.single_player_font.index) == 0x54 ? 1 : -1];

typedef char main_globals_size_assert[
	sizeof(struct _main_globals) == 0x620 ? 1 : -1];
typedef char main_globals_frame_start_milliseconds_offset_assert[
	offsetof(struct _main_globals, frame_start_milliseconds) == 0x00 ? 1 : -1];
typedef char main_globals_rasterizer_target_index_offset_assert[
	offsetof(struct _main_globals, rasterizer_target_index) == 0x08 ? 1 : -1];
typedef char main_globals_rasterizer_initial_index_offset_assert[
	offsetof(struct _main_globals, rasterizer_initial_index) == 0x10 ? 1 : -1];
typedef char main_globals_rasterizer_throttle_start_index_offset_assert[
	offsetof(struct _main_globals, rasterizer_throttle_start_index) == 0x18 ? 1 : -1];
typedef char main_globals_rasterizer_throttle_end_index_offset_assert[
	offsetof(struct _main_globals, rasterizer_throttle_end_index) == 0x20 ? 1 : -1];
typedef char main_globals_seconds_elapsed_offset_assert[
	offsetof(struct _main_globals, seconds_elapsed) == 0x28 ? 1 : -1];
typedef char main_globals_connection_offset_assert[
	offsetof(struct _main_globals, connection) == 0x2C ? 1 : -1];
typedef char main_globals_movie_offset_assert[
	offsetof(struct _main_globals, movie) == 0x30 ? 1 : -1];
typedef char main_globals_defer_map_change_offset_assert[
	offsetof(struct _main_globals, defer_map_change) == 0x45 ? 1 : -1];
typedef char main_globals_reset_map_offset_assert[
	offsetof(struct _main_globals, reset_map) == 0x44 ? 1 : -1];
typedef char main_globals_revert_map_offset_assert[
	offsetof(struct _main_globals, revert_map) == 0x46 ? 1 : -1];
typedef char main_globals_skip_cinematic_offset_assert[
	offsetof(struct _main_globals, skip_cinematic) == 0x47 ? 1 : -1];
typedef char main_globals_saving_map_offset_assert[
	offsetof(struct _main_globals, saving_map) == 0x48 ? 1 : -1];
typedef char main_globals_save_map_safely_offset_assert[
	offsetof(struct _main_globals, save_map_safely) == 0x49 ? 1 : -1];
typedef char main_globals_won_map_offset_assert[
	offsetof(struct _main_globals, won_map) == 0x5A ? 1 : -1];
typedef char main_globals_halt_time_scale_offset_assert[
	offsetof(struct _main_globals, halt_time_scale) == 0x66 ? 1 : -1];
typedef char main_globals_respawn_timer_offset_assert[
	offsetof(struct _main_globals, respawn_timer) == 0x6E ? 1 : -1];
typedef char main_globals_loss_timer_offset_assert[
	offsetof(struct _main_globals, loss_timer) == 0x6C ? 1 : -1];
typedef char main_globals_allow_persistent_storage_offset_assert[
	offsetof(struct _main_globals, allow_persistent_storage) == 0x74 ? 1 : -1];
typedef char main_globals_soloplayer_map_name_offset_assert[
	offsetof(struct _main_globals, soloplayer_map_name) == 0x75 ? 1 : -1];
typedef char main_globals_multiplayer_map_name_offset_assert[
	offsetof(struct _main_globals, multiplayer_map_name) == 0x175 ? 1 : -1];
typedef char main_globals_core_name_offset_assert[
	offsetof(struct _main_globals, core_name) == 0x375 ? 1 : -1];
typedef char main_globals_vblank_interval_current_offset_assert[
	offsetof(struct _main_globals, vblank_interval_current) == 0x3B6 ? 1 : -1];
typedef char main_globals_vblank_interval_requested_offset_assert[
	offsetof(struct _main_globals, vblank_interval_requested) == 0x3B8 ? 1 : -1];
typedef char main_globals_vblank_interval_held_offset_assert[
	offsetof(struct _main_globals, vblank_interval_held) == 0x3BA ? 1 : -1];
typedef char main_globals_vblank_failure_counts_offset_assert[
	offsetof(struct _main_globals, vblank_failure_counts) == 0x3BC ? 1 : -1];
typedef char main_globals_vblank_last_failure_indices_offset_assert[
	offsetof(struct _main_globals, vblank_last_failure_indices) == 0x3C8 ? 1 : -1];
typedef char main_globals_d3d_flip_count_offset_assert[
	offsetof(struct _main_globals, d3d_flip_count) == 0x3F8 ? 1 : -1];
typedef char main_globals_vblank_flip_delta_index_offset_assert[
	offsetof(struct _main_globals, vblank_flip_delta_index) == 0x3FC ? 1 : -1];
typedef char main_globals_vblank_flip_deltas_offset_assert[
	offsetof(struct _main_globals, vblank_flip_deltas) == 0x3FE ? 1 : -1];
typedef char main_globals_vblank_debug_string_offset_assert[
	offsetof(struct _main_globals, vblank_debug_string) == 0x41C ? 1 : -1];

struct game_options
{
	unsigned long flags;
	short code_version;
	short difficulty;
	unsigned long random_seed;
	char map_name[256];
};

typedef char game_options_size_assert[
	sizeof(struct game_options) == 0x10C ? 1 : -1];

#pragma pack(push, 1)
struct _screenshot_and_framerate_globals
{
	short count;
	byte reserved002[6];
	struct render_window windows[MAXIMUM_WINDOWS + 1];
	real framerate_samples[8];
	word framerate_flags;
	char framerate_sample_index;
	boolean framerate_active;
	char framerate_counter;
	boolean framerate_reset;
	boolean halt_recursion_lock;
};
#pragma pack(pop)

typedef char screenshot_and_framerate_globals_size_assert[
	sizeof(struct _screenshot_and_framerate_globals) == 0x38B ? 1 : -1];

void network_test_update(boolean main_menu_loaded, real seconds);
#include "server_director.h" /* port: port/linux/game/server_director.c */

/* ---------- prototypes */

static long sort_desired_local_player_controllers(
	short const *a,
	short const *b);
static void main_setup_connection(
	void);
static void main_initialize_time(
	void);
static void main_skip_private(
	void);
static void main_update_time(
	void);
static void main_frame_rate_debug(
	void);

static void main_new_map(
	struct game_options *options);
static void main_game_render(
	double time_delta_since_tick_sec);

/* ---------- globals */

short global_difficulty_level = 1;
short player_spawn_count = 1;
boolean global_frame_rate_throttle = TRUE;
short global_screenshot_size = 1;

static char const *scenario_paths[10] =
{
	"levels\\a10\\a10",
	"levels\\a30\\a30",
	"levels\\a50\\a50",
	"levels\\b30\\b30",
	"levels\\b40\\b40",
	"levels\\c10\\c10",
	"levels\\c20\\c20",
	"levels\\c40\\c40",
	"levels\\d20\\d20",
	"levels\\d40\\d40"
};

static struct _main_globals main_globals = { 0 };
/* Keep the original globals layout; these clocks belong to pending requests. */
static long main_loss_last_tick;
static long main_respawn_last_tick;
/* port: the game tick a checkpoint last counted (main_save_map_private) */
static long main_save_last_tick;
boolean debug_force_frame_rate_update = FALSE;
boolean debug_no_drawing = FALSE;
boolean debug_game_save = FALSE;
boolean debug_frame_rate = FALSE;
boolean display_framerate = FALSE;
boolean display_vblank_deltas = FALSE;
boolean display_precache_progress = FALSE;
struct _screenshot_and_framerate_globals global_screenshot_count = { 0 };
boolean debug_render_freeze;

/* ---------- public code */

real main_get_seconds_elapsed(
	void)
{
	return main_globals.seconds_elapsed;
}

boolean gamepad_button_is_down(
	short button_index)
{
	short gamepad_index;
	boolean result = FALSE;

	match_assert("c:\\halo\\SOURCE\\main\\main.c", 245,
		button_index>=0 && button_index<NUMBER_OF_GAMEPAD_BUTTONS);
	gamepad_index = 0;

	do
	{
		if (input_has_gamepad(gamepad_index))
			break;

		gamepad_index++;
	}
	while (gamepad_index < MAXIMUM_GAMEPADS);

	if (gamepad_index < MAXIMUM_GAMEPADS)
	{
		struct gamepad_state const *gamepad = input_get_gamepad_state(gamepad_index);

		result = gamepad->buttons[button_index] > 0;
	}

	return result;
}

static void create_local_players(
	void)
{
	short used_controllers[MAXIMUM_GAMEPADS];
	short default_controllers[MAXIMUM_GAMEPADS];
	short desired_controllers[MAXIMUM_GAMEPADS];
	long i;
	long j;
	long player;
	short gamepad_index;

	if (main_globals.main_menu_scenario_loaded)
	{
		player = player_new(0, NONE, 0, NULL);
		local_player_set_player_index(
			0,
			player);
	}
	else
	{
		csmemset(used_controllers, NONE, sizeof(used_controllers));
		csmemset(desired_controllers, NONE, sizeof(desired_controllers));
		default_controllers[0] = 0;
		default_controllers[1] = 1;
		default_controllers[2] = 2;
		default_controllers[3] = 3;

		match_assert(
			"c:\\halo\\SOURCE\\main\\main.c",
			741,
			game_connection() == _game_connection_local);
		for (i = 0; i < player_spawn_count; i++)
		{
			gamepad_index =
				player_ui_get_single_player_local_player_controller(i);
			desired_controllers[i] = gamepad_index;
			if (desired_controllers[i] == NONE)
				desired_controllers[i] = default_controllers[i];
			match_assert(
				"c:\\halo\\SOURCE\\main\\main.c",
				755,
				(desired_controllers[i]>=0) && (desired_controllers[i]<MAXIMUM_GAMEPADS));

			if (used_controllers[desired_controllers[i]] != NONE)
			{
				for (j = 0; j < MAXIMUM_GAMEPADS; j++)
				{
					if (used_controllers[j] == NONE)
					{
						desired_controllers[i] = (short)j;
						used_controllers[(short)j] = (short)j;
						break;
					}
				}
				match_assert(
					"c:\\halo\\SOURCE\\main\\main.c",
					768,
					j<MAXIMUM_GAMEPADS);
			}
			else
			{
				used_controllers[desired_controllers[i]] = desired_controllers[i];
			}
		}

		qsort(
			desired_controllers,
			MAXIMUM_GAMEPADS,
			sizeof(*desired_controllers),
			(int (__cdecl *)(void const *, void const *))sort_desired_local_player_controllers);
		for (i = 0; i < player_spawn_count; i++)
		{
			gamepad_index = desired_controllers[i];
			match_assert(
				"c:\\halo\\SOURCE\\main\\main.c",
				784,
				(gamepad_index>=0) && (gamepad_index<MAXIMUM_GAMEPADS));
			player = player_new(0, NONE, gamepad_index, NULL);
			local_player_set_player_index(
				gamepad_index,
				player);
		}
	}

	return;
}

void game_connection_set(
	short connection)
{
	main_globals.connection = connection;
	return;
}

short game_connection(
	void)
{
	return main_globals.connection;
}

void main_disallow_persistent_storage(
	void)
{
	main_globals.allow_persistent_storage = FALSE;
	return;
}

void main_defer_map_map_change(
	void)
{
	main_globals.defer_map_change = FALSE;
	return;
}

char *main_get_map_name(
	void)
{
	return main_globals.soloplayer_map_name;
}

char *main_get_multiplayer_map_name(
	void)
{
	return main_globals.multiplayer_map_name;
}

short main_get_difficulty(
	void)
{
	return global_difficulty_level;
}

boolean main_saving_map(
	void)
{
	return main_globals.saving_map;
}

void main_save_cancel(
	void)
{
	main_globals.saving_map = FALSE;
	return;
}

void main_won_map(
	void)
{
	main_globals.saving_map = FALSE;
	main_globals.won_map = TRUE;
	return;
}

/* port: the level a campaign map's own script named with map_name, which
main_won_map_private takes next; empty for none */
static char main_script_next_level[64];

/* port: the level the scripts name in the same campaign as `current`: a
mission of the same MCC package, else a Custom Edition campaign map; FALSE
for any other. The name may have a path and an extension (".map"); only its
last part counts */
static boolean main_script_level_resolve(
	char const *current,
	char const *requested,
	char *next,
	size_t size)
{
	char const *leaf = requested;
	char const *cursor;
	char candidate[160];
	size_t length;

	if (!current || !current[0] || !requested)
		return FALSE;
	/* port: package mission identities and lookup belong to mcc_main.c. */
	if (mcc_level_name(current))
		return mcc_main_script_level_resolve(current, requested, next, size);
	for (cursor = requested; *cursor; cursor++)
	{
		if (*cursor == '\\' || *cursor == '/')
			leaf = cursor + 1;
	}
	length = strlen(leaf);
	if (length > 4 && !_stricmp(leaf + length - 4, ".map"))
		length -= 4;
	if (!length || length > 64)
		return FALSE;
	if (custom_edition_level_name(current))
	{
		snprintf(candidate, sizeof(candidate), "%s%.*s", CUSTOM_EDITION_LEVEL_NAME_PREFIX, (int)length, leaf);
		if (!custom_edition_maps_level_campaign(candidate) || strlen(candidate) >= size)
			return FALSE;
		csstrncpy(next, candidate, size - 1);
		next[size - 1] = 0;
		return TRUE;
	}

	return FALSE;
}

/* port: H1A and Custom Edition campaigns end a level by naming the next with
map_name in its scripts, where the Xbox's game_won takes it to the next of
its own levels. A campaign map's script that does so ends the level as a won
one, with the level it names next (main_won_map_private): alone, it loads; in
network co-op the host takes every machine there, and a dedicated server
plays its rotation's next, as after any won level. Only a level of the same
campaign is taken; any other name does nothing, and the map's other scripts
run on (no map's script may switch to any map: hs.c) */
void main_script_map_name(
	char const *map_name)
{
	struct network_game *game = game_connection() == _game_connection_network_server ?
		network_game_get_game() : NULL;
	char const *current = game ? game->map.name : main_globals.soloplayer_map_name;
	char next[NUMBEROF(main_script_next_level)];

	if (!main_script_level_resolve(current, map_name, next, sizeof(next)))
	{
		error(_error_silent, "the scripts' map_name %s is no level of %s's campaign: nothing happens",
			map_name ? map_name : "", current);
		return;
	}
	error(_error_silent, "the scripts end %s, with %s next", current, next);
	csstrncpy(main_script_next_level, next, sizeof(main_script_next_level) - 1);
	main_script_next_level[sizeof(main_script_next_level) - 1] = 0;
	main_won_map();
	return;
}

void main_lost_map(
	void)
{
	if (!main_globals.lost_map)
	{
		main_globals.loss_timer = 0;
		main_loss_last_tick = game_time_get();
	}
	main_globals.saving_map = FALSE;
	main_globals.lost_map = TRUE;
	return;
}

void main_stop_time(
	void)
{
	main_globals.halt_time_scale = FALSE;
	main_globals.restart_time = FALSE;
	return;
}

void main_start_time(
	void)
{
	main_globals.restart_time = TRUE;
	return;
}

void main_reset_map(
	void)
{
	main_globals.switch_to_structure_bsp_index = NONE;
	main_globals.saving_map = FALSE;
	main_globals.reset_map = TRUE;
	main_globals.lost_map = FALSE;
	return;
}

void main_revert_map(
	void)
{
	main_globals.switch_to_structure_bsp_index = NONE;
	main_globals.saving_map = FALSE;
	main_globals.revert_map = TRUE;
	main_globals.lost_map = FALSE;
	return;
}

void main_skip_cinematic(
	void)
{
	main_globals.switch_to_structure_bsp_index = NONE;
	main_globals.saving_map = FALSE;
	main_globals.skip_cinematic = TRUE;
	return;
}

void main_save_map_nonsafe(
	void)
{
	main_globals.saving_map = TRUE;
	main_globals.save_map_safely = FALSE;
	return;
}

void main_respawn(
	boolean in_multiplayer)
{
	if (!main_globals.respawn)
	{
		main_globals.respawn_timer = 0;
		main_respawn_last_tick = game_time_get();
	}
	main_globals.respawn = TRUE;
	if (in_multiplayer)
		main_globals.respawn_timer = 92;
	return;
}

void main_save_core(
	void)
{
	main_globals.save_core = TRUE;
	csstrcpy(main_globals.core_name, "core.bin");
	return;
}

void main_save_core_name(
	char const *core_name)
{
	match_vwarn("c:\\halo\\SOURCE\\main\\main.c", 0x3a5, csstrlen(core_name) < 64, "warning, core file name will be truncated to 63 characters");
	csstrncpy(main_globals.core_name, core_name, 63);
	main_globals.save_core = TRUE;
	return;
}

void main_load_core(
	void)
{
	main_globals.load_core = TRUE;
	csstrcpy(main_globals.core_name, "core.bin");
	return;
}

void main_load_core_at_startup(
	void)
{
	main_globals.load_core_at_startup = TRUE;
	csstrcpy(main_globals.core_name, "core.bin");
	return;
}

void main_load_core_name(
	char const *core_name)
{
	match_vwarn("c:\\halo\\SOURCE\\main\\main.c", 0x3c9, csstrlen(core_name) < 64, "warning, core file name will be truncated to 63 characters");
	csstrncpy(main_globals.core_name, core_name, 63);
	main_globals.load_core = TRUE;
	return;
}

void main_load_core_name_at_startup(
	char const *core_name)
{
	match_vwarn("c:\\halo\\SOURCE\\main\\main.c", 0x3d7, csstrlen(core_name) < 64, "warning, core file name will be truncated to 63 characters");
	csstrncpy(main_globals.core_name, core_name, 63);
	main_globals.load_core_at_startup = TRUE;
	return;
}

void main_switch_structure_bsp(
	short structure_bsp_index)
{
	struct scenario *scenario = global_scenario_get();
	if (structure_bsp_index < 0 || structure_bsp_index >= scenario->structure_bsp_references.count)
	{
		console_warning("tried to switch to invalid structure-bsp %d", structure_bsp_index);
		return;
	}
	if (structure_bsp_index == global_structure_bsp_index)
	{
		console_warning("tried to switch to current structure-bsp %d", structure_bsp_index);
		return;
	}
	main_globals.switch_to_structure_bsp_index = structure_bsp_index;
	hud_load(TRUE);
	return;
}

void main_goto_main_menu(
	void)
{
	main_globals.switch_to_structure_bsp_index = NONE;
	main_globals.saving_map = FALSE;
	main_globals.want_to_be_at_main_menu = TRUE;
	return;
}

static void main_queue_map_private(
	void)
{
	real progress;
	if (cache_files_precache_in_progress() && cache_files_precache_map_status(&progress) == 1)
		cache_files_precache_map_end();
	if (!cache_files_precache_in_progress())
	{
		cache_files_precache_map_begin(main_globals.queued_map_name, FALSE);
		main_globals.queue_map = FALSE;
	}
	return;
}

void main_menu_precache_resources(
	void)
{
	struct scenario *scenario = global_scenario_get();
	if (scenario)
	{
		match_assert("c:\\halo\\SOURCE\\main\\main.c", 0x46d, scenario->type==_scenario_type_main_menu);
		predicted_resources_precache(&scenario->predicted_ui_resources);
	}
	return;
}

void main_menu_unload(
	void)
{
	ui_stop_main_menu_music();
	main_menu_active(FALSE);
	main_globals.main_menu_scenario_loaded = FALSE;
	return;
}

void main_menu_load(
	void)
{
	if (!main_globals.main_menu_scenario_loaded)
		main_load_ui_scenario(FALSE);
	main_screen_shell_load();
	main_menu_precache_resources();
	update_server_delete();
	update_server_new();
	update_server_start();
	game_time_dispose_from_old_map();
	game_time_initialize_for_new_map();
	game_time_start();
	hs_runtime_dispose_from_old_map();
	hs_runtime_initialize_for_new_map();
	main_globals.want_to_be_at_main_menu = FALSE;

	return;
}

static void screenshot_record(
	struct bitmap_data *screen,
	struct file_reference *file)
{
	char const *error_message = tiff_export(file, screen);

	if (error_message)
		error(_error_silent, error_message);
	return;
}

void set_window_camera_values(
	struct render_window *window,
	struct observer_result const *observer)
{
	if (observer)
	{
		window->rasterizer_camera.position = observer->position;
		window->rasterizer_camera.forward = observer->forward;
		window->rasterizer_camera.up = observer->up;
		/* port: display.fov's view, the local render view only (view_fov.c) */
		window->rasterizer_camera.vertical_field_of_view =
			render_fov_vertical(window->local_player_index, 2.0f * arctangent(
				0.75f * render_camera_get_adjusted_field_of_view_tangent(
					observer->field_of_view),
				1.0f));

		if (window->local_player_index != NONE &&
			!console_is_active() &&
			!game_time_get_paused() &&
			director_get_perspective(window->local_player_index) !=
				_director_perspective_neutral)
		{
			real_matrix4x3 effect_matrix;
			real_matrix4x3 view_matrix;

			player_effect_get_camera_effect_matrix(
				window->local_player_index,
				&effect_matrix);
			matrix4x3_from_point_and_vectors(
				&view_matrix,
				&observer->position,
				&observer->forward,
				&observer->up);
			matrix4x3_multiply(&view_matrix, &effect_matrix, &view_matrix);
			matrix4x3_to_point_and_vectors(
				&view_matrix,
				&window->rasterizer_camera.position,
				&window->rasterizer_camera.forward,
				&window->rasterizer_camera.up);
		}
	}
	else
	{
		window->rasterizer_camera.position = *global_origin3d;
		window->rasterizer_camera.forward = *global_forward3d;
		window->rasterizer_camera.up = *global_up3d;
		window->rasterizer_camera.vertical_field_of_view =
			2.0f * arctangent(
				0.75f * render_camera_get_adjusted_field_of_view_tangent(
					DEGREES_TO_RADIANS(80.0f)),
				1.0f);
	}

	window->rasterizer_camera.mirrored = FALSE;
	window->rasterizer_camera.z_near = rasterizer_globals.near_clip_distance;
	window->rasterizer_camera.z_far = rasterizer_globals.far_clip_distance;
	if (!debug_render_freeze)
		window->render_camera = window->rasterizer_camera;

	return;
}

void main_present_frame(
	void)
{
	struct file_reference reference;
	char path[512];

	render_frame_present(NULL, main_globals.movie);
	if (global_screenshot_count.count <= 0 && main_globals.movie)
	{
		_snprintf(
			path,
			NUMBEROF(path),
			"movie\\frame%06d.tga",
			main_globals.recording_frame_index++);
		file_reference_create_from_path(&reference, path, FALSE);
		screenshot_record(main_globals.movie, &reference);
	}

	return;
}

void main_menu_ensure_player_queues_exist(
	void)
{
	update_server_delete();
	update_server_new();
	update_server_start();
	return;
}

boolean main_menu_fade_active(
	void)
{
	return main_globals.map_change_load_timer != 0;
}

void main_menu_switch_to_single_player(
	void)
{
	main_globals.defer_map_change = TRUE;
	return;
}

void main_set_game_connection_to_film_playback(
	void)
{
	main_globals.playback_last_recording = TRUE;
	return;
}

short main_get_solo_level_from_name(
	char const *name)
{
	char lower_name[128] = { 0 };
	short level;

	if (mcc_level_name(name))
		return NONE;

	/* port: a Custom Edition map (custom_maps\a30) is never one of the
	campaign's levels, whatever its name holds
	(port/linux/game/custom_edition_cache.c) */
	if (custom_edition_level_name(name))
		return NONE;
	csstrncpy(lower_name, name, NUMBEROF(lower_name) - 1);
	lower_name[NUMBEROF(lower_name) - 1] = 0;
	strlwr(lower_name);

	if (strstr(lower_name, "a10"))
		level = 0;
	else if (strstr(lower_name, "a30"))
		level = 1;
	else if (strstr(lower_name, "a50"))
		level = 2;
	else if (strstr(lower_name, "b30"))
		level = 3;
	else if (strstr(lower_name, "b40"))
		level = 4;
	else if (strstr(lower_name, "c10"))
		level = 5;
	else if (strstr(lower_name, "c20"))
		level = 6;
	else if (strstr(lower_name, "c40"))
		level = 7;
	else if (strstr(lower_name, "d20"))
		level = 8;
	else
		level = strstr(lower_name, "d40") ? 9 : NONE;

	return level;
}

short main_get_current_solo_level(
	void)
{
	return main_get_solo_level_from_name(main_globals.soloplayer_map_name);
}

char const *main_get_solo_level_name(
	short level)
{
	char const *result = NULL;

	if (level >= 0 && level < NUMBEROF(scenario_paths))
		result = scenario_paths[level];

	return result;
}

void main_run_demos(
	void)
{
	main_globals.run_xdemos = TRUE;
	return;
}

static void compute_subframe_counts(
	long num_players,
	long *horizontal_count,
	long *vertical_count)
{
	long horizontal = 1;
	long vertical = 1;
	boolean split_horizontally = TRUE;

	match_assert(
		"c:\\halo\\SOURCE\\main\\main.c",
		1308,
		num_players>0);
	while (horizontal*vertical<num_players)
	{
		boolean add_column = split_horizontally ? horizontal<vertical : horizontal<=vertical;

		if (add_column)
		{
			horizontal++;
		}
		else
		{
			horizontal = 1;
			vertical++;
		}
	}

	*horizontal_count = horizontal;
	*vertical_count = vertical;
	return;
}

void compute_window_bounds(
	long player_index,
	long num_players,
	rectangle2d *pixel_bounds,
	rectangle2d *safe_frame_bounds)
{
	long vertical_count;
	long horizontal_count;
	long horizontal_index;
	long vertical_index;
	long subframe_width;
	long subframe_height;
	short safe_frame_inset;
	boolean first_player_spans_two_columns;

	match_assert(
		"c:\\halo\\SOURCE\\main\\main.c",
		1359,
		player_index<num_players);
	safe_frame_inset = num_players>1 ? 4 : 0;
	first_player_spans_two_columns = FALSE;
	compute_subframe_counts(
		num_players,
		&horizontal_count,
		&vertical_count);

	if (horizontal_count*vertical_count>num_players)
	{
		if (!player_index)
			first_player_spans_two_columns = TRUE;
		else
			player_index++;
	}

	vertical_index = player_index/horizontal_count;
	horizontal_index = player_index-vertical_index*horizontal_count;
	match_assert(
		"c:\\halo\\SOURCE\\main\\main.c",
		1390,
		vertical_index>=0 && vertical_index<vertical_count);
	match_assert(
		"c:\\halo\\SOURCE\\main\\main.c",
		1391,
		horizontal_index>=0 && horizontal_index<horizontal_count);

	subframe_height =
		(rasterizer_globals.reserved04.frame_bounds.y1-
			rasterizer_globals.reserved04.frame_bounds.y0)/vertical_count;
	subframe_width =
		(rasterizer_globals.reserved04.frame_bounds.x1-
			rasterizer_globals.reserved04.frame_bounds.x0)/horizontal_count;
	subframe_width *= first_player_spans_two_columns ? 2 : 1;

	safe_frame_bounds->x0 =
		subframe_width*horizontal_index+
		rasterizer_globals.reserved04.frame_bounds.x0;
	safe_frame_bounds->x1 =
		subframe_width*(horizontal_index+1)+
		rasterizer_globals.reserved04.frame_bounds.x0;
	safe_frame_bounds->y0 =
		subframe_height*vertical_index+
		rasterizer_globals.reserved04.frame_bounds.y0;
	safe_frame_bounds->y1 =
		subframe_height*(vertical_index+1)+
		rasterizer_globals.reserved04.frame_bounds.y0;
	*pixel_bounds = *safe_frame_bounds;

	safe_frame_bounds->x0 += horizontal_index*safe_frame_inset;
	safe_frame_bounds->x1 -= (horizontal_index==0)*safe_frame_inset;
	safe_frame_bounds->y0 += vertical_index*safe_frame_inset;
	safe_frame_bounds->y1 -= (vertical_index==0)*safe_frame_inset;

	if (!horizontal_index)
		pixel_bounds->x0 = rasterizer_globals.reserved04.screen_bounds.x0;
	if ((first_player_spans_two_columns!=FALSE)+horizontal_index+1==horizontal_count)
		pixel_bounds->x1 = rasterizer_globals.reserved04.screen_bounds.x1;
	if (!vertical_index)
		pixel_bounds->y0 = rasterizer_globals.reserved04.screen_bounds.y0;
	if (vertical_index+1==vertical_count)
		pixel_bounds->y1 = rasterizer_globals.reserved04.screen_bounds.y1;

	return;
}

short main_get_window_count(
	void)
{
	boolean single_window = game_engine_force_single_screen() || cinematic_in_progress();

	return single_window ? 1 : PIN(local_player_count(), 1, MAXIMUM_WINDOWS);
}

#ifdef HALO_PROFILE
/* port: whether the main menu's map is the one loaded (profile_console.c: a
recording of debug.profile_record_when = "game" waits for another) */
boolean main_menu_is_loaded(
	void)
{
	return main_globals.main_menu_scenario_loaded;
}
#endif

static void main_new_map(
	struct game_options *options)
{
	input_flush();
	if (game_load(options))
	{
		game_initialize_for_new_map();
	}
	else
	{
		/* port: an MCC map that fails to load goes back to the menus (mcc_main.c) */
		if (mcc_main_load_failed(options->map_name))
			return;
		error(_error_immediate, "game_load() failed.");
	}

	if (!errors_handle())
	{
		create_local_players();
		game_time_start();
	}
	else
	{
		error(_error_immediate, "main_new_map() failed.");
	}
	game_initial_pulse();

	main_globals.reset_map = FALSE;
	main_globals.defer_map_change = FALSE;
	main_globals.revert_map = FALSE;
	main_globals.skip_cinematic = FALSE;
	main_globals.saving_map = FALSE;
	main_globals.won_map = FALSE;
	main_globals.lost_map = FALSE;
	main_globals.respawn = FALSE;
	main_globals.save_core = FALSE;
	main_globals.switch_to_structure_bsp_index = NONE;
	main_globals.load_core = main_globals.load_core_at_startup;
	main_globals.load_core_at_startup = FALSE;

	if (main_globals.allow_persistent_storage)
		game_state_try_and_load_from_persistent_storage();
	ui_widgets_disable_pause_game(30);

	return;
}

static void main_change_map_name(
	void)
{
	if (main_globals.main_menu_scenario_loaded == TRUE)
	{
		if (!main_globals.map_change_load_timer)
		{
			if (ui_main_menu_music_active() == TRUE)
			{
				main_globals.map_change_load_timer =
					main_globals.frame_start_milliseconds + 1000;
				main_screen_shell_begin_fade(1000);
				ui_widgets_inhibit_processing(TRUE);
				ui_widgets_set_fade_value(0.0f);
			}
		}
		else
		{
			unsigned long remaining_milliseconds =
				main_globals.map_change_load_timer - main_globals.frame_start_milliseconds;

			ui_widgets_set_fade_value(
				1.0f - (real)remaining_milliseconds * 0.001f);
		}
	}
	else
	{
		main_globals.map_change_load_timer = 0;
	}

	if (main_globals.frame_start_milliseconds >= main_globals.map_change_load_timer)
	{
		ui_widgets_set_fade_value(-1.0f);
		ui_stop_main_menu_music();
		main_menu_active(FALSE);
		main_globals.main_menu_scenario_loaded = FALSE;
		ui_widgets_inhibit_processing(FALSE);

		if (game_in_progress() && main_globals.connection == _game_connection_local)
		{
			struct game_options options;
			short local_player_index;

			game_options_new(&options);
			csstrncpy(
				options.map_name,
				main_globals.soloplayer_map_name,
				NUMBEROF(options.map_name) - 1);
			options.map_name[NUMBEROF(options.map_name) - 1] = 0;
			options.difficulty = global_difficulty_level;
			game_dispose_from_old_map();
			game_precache_new_map(options.map_name, TRUE);
			game_unload();
			main_new_map(&options);

			for (local_player_index = 0;
				local_player_index < player_spawn_count;
				local_player_index++)
			{
				player_profile_save_last_level_played(local_player_index);
			}
		}

		main_globals.map_change_load_timer = 0;
	}

	return;
}

void main_set_multiplayer_map_name(
	char const *map_name)
{
	csstrncpy(main_globals.multiplayer_map_name, map_name, NUMBEROF(main_globals.multiplayer_map_name) - 1);
	main_globals.multiplayer_map_name[NUMBEROF(main_globals.multiplayer_map_name) - 1] = 0;
	cache_files_give_time_to_precache(main_globals.multiplayer_map_name);
	return;
}

void main_set_difficulty(
	short difficulty)
{
	if (difficulty >= _game_difficulty_level_easy && difficulty < NUMBER_OF_GAME_DIFFICULTY_LEVELS)
		global_difficulty_level = difficulty;
	return;
}

static long sort_desired_local_player_controllers(
	short const *a,
	short const *b)
{
	short value_a = *a;
	short value_b = *b;
	if (value_a == NONE && value_b != value_a)
		return 1;
	if (value_a != NONE && value_b == NONE)
		return -1;
	if (value_a > value_b)
		return 1;
	return (value_a >= value_b) - 1;
}

void main_save_map_safe(
	void)
{
	if (!main_globals.saving_map || main_globals.save_map_timeout)
	{
		main_globals.saving_map = TRUE;
		main_globals.save_map_safely = TRUE;
		main_globals.save_map_timeout = TRUE;
		main_globals.ticks_until_next_save_check = 0;
		main_globals.ticks_unable_to_save = 0;
		main_globals.safe_intervals = 0;
		/* port: the tick before: the first check is at once */
		main_save_last_tick = game_time_get()-1;
	}
	return;
}

void main_save_map_no_timeout(
	void)
{
	if (!main_globals.saving_map || main_globals.save_map_timeout)
	{
		main_globals.saving_map = TRUE;
		main_globals.save_map_safely = TRUE;
		main_globals.ticks_until_next_save_check = 0;
		main_globals.ticks_unable_to_save = 0;
		main_globals.safe_intervals = 0;
		/* port: the tick before: the first check is at once */
		main_save_last_tick = game_time_get()-1;
	}
	main_globals.save_map_timeout = FALSE;
	return;
}

void main_skip(
	short ticks)
{
	if (ticks <= 15)
	{
		main_globals.skip_ticks = ticks;
		main_globals.skip = TRUE;
	}
	else
	{
		error(_error_silent, "cannot skip more than 15 frames (half a second)");
	}
	return;
}

void main_queue_map_name(
	char const *map_name)
{
	if (map_name)
	{
		csstrncpy(main_globals.queued_map_name, map_name, NUMBEROF(main_globals.queued_map_name) - 1);
		main_globals.queue_map = TRUE;
	}
	else
	{
		main_globals.queued_map_name[0] = 0;
		main_globals.queue_map = FALSE;
	}
	return;
}

boolean main_taking_screenshot(
	void)
{
	return global_screenshot_count.count > 0 || main_globals.movie != NULL;
}

void main_movie_start(
	real frames_per_second)
{
	match_assert("c:\\halo\\SOURCE\\main\\main.c", 2715, main_globals.movie==NULL);

	main_globals.movie = bitmap_2d_new(640, 480, 0, 10);
	if (main_globals.movie)
	{
		directory_create_or_delete_contents("movie");
		main_globals.recording_frame_index = 0;
		if (frames_per_second > 0.0001f)
			main_globals.recording_dt = 1.0f / frames_per_second;
		else
			main_globals.recording_dt = 1.0f / 30.0f;
		game_time_set_speed(1.0f);
	}

	return;
}

void main_movie_stop(
	void)
{
	if (main_globals.movie)
	{
		bitmap_delete(main_globals.movie);
		main_globals.movie = NULL;
	}
	return;
}

void main_crash(
	char const *str)
{
	/* BUG (original, deliberate): the "crash" script command ("crashes (for debugging).")
	 * faults on purpose by storing this literal through the null pointer; the August and
	 * September 2001 builds (debug and retail) and January all emit this one store. */
	*(char **)NULL = "chucky was here!  NULL belongs to me!!!!!";
	return;
}

#ifdef HALO_NATIVE_BUILD_INFO
/* The original Xbox version string describes the map format, not this port.
   Keep it on Xbox; native builds name the binary that is actually running. */
static void main_native_build_label(char *label, size_t capacity)
{
	char const *platform;

	#ifdef HALO_ANDROID
	platform = "Android";
	#elif defined(HALO_WINDOWS)
	platform = "Windows";
	#else
	platform = "Linux";
	#endif
	if (HALO_BUILD_NUMBER > 0)
		_snprintf(label, capacity - 1, "OpenCE %s | build %d (%s)",
			platform, HALO_BUILD_NUMBER, HALO_BUILD_FLAVOR);
	else
		_snprintf(label, capacity - 1, "OpenCE %s | local build (%s)",
			platform, HALO_BUILD_FLAVOR);
	label[capacity - 1] = 0;
}

/* The 2 KB error buffer keeps recent lines at its end. Put the newest first
   so the failure is visible even if older messages run off the screen. */
static char const *main_native_error_tail(char const *messages)
{
	enum { MAX_LINES = 8, MAX_LINE_BYTES = 110 };
	static char recent[MAX_LINES * (MAX_LINE_BYTES + 5) + 1];
	char const *lines[MAX_LINES];
	size_t lengths[MAX_LINES];
	char const *cursor;
	char const *start;
	size_t length;
	size_t copied;
	size_t used = 0;
	unsigned int count = 0;
	unsigned int index;

	for (cursor = messages; *cursor; )
	{
		start = cursor;
		while (*cursor && *cursor != '\r' && *cursor != '\n')
			cursor++;
		length = cursor - start;
		while (*cursor == '\r' || *cursor == '\n')
			cursor++;
		if (!length || (length >= sizeof("[...too many errors to print...]") - 1 &&
			!strncmp(start, "[...too many errors to print...]",
				sizeof("[...too many errors to print...]") - 1)))
			continue;
		if (count == MAX_LINES)
		{
			for (index = 1; index < MAX_LINES; index++)
			{
				lines[index - 1] = lines[index];
				lengths[index - 1] = lengths[index];
			}
			count--;
		}
		lines[count] = start;
		lengths[count++] = length;
	}
	if (!count)
		return "No recent messages. See debug.txt for details.\r\n";
	for (index = count; index > 0; index--)
	{
		length = lengths[index - 1];
		copied = length < MAX_LINE_BYTES ? length : MAX_LINE_BYTES;
		memcpy(recent + used, lines[index - 1], copied);
		used += copied;
		if (copied < length)
		{
			memcpy(recent + used, "...", 3);
			used += 3;
		}
		recent[used++] = '\r';
		recent[used++] = '\n';
	}
	recent[used] = 0;
	return recent;
}
#endif

void main_print_version(
	void)
{
	#ifdef HALO_NATIVE_BUILD_INFO
	char label[96];

	main_native_build_label(label, sizeof(label));
	console_printf(FALSE, "%s | compiled %s %s", label, __DATE__, __TIME__);
	#else
	console_printf(FALSE, "halobeta xbox 01.01.14.2342 Jan 14 2002 12:49:20");
	#endif
	return;
}

void main_vertical_blank_interrupt_handler(
	unsigned long context)
{
	rasterizer_globals.frame_and_vertical_blank_index++;
	if (!main_globals.d3d_flip_count)
	{
		rasterizer_globals.previous_frame_and_vertical_blank_index =
			rasterizer_globals.frame_and_vertical_blank_index;
	}
	else if (*main_globals.d3d_flip_count != rasterizer_globals.d3d_flip_count)
	{
		main_globals.vblank_flip_deltas[main_globals.vblank_flip_delta_index] =
			(short)((short)rasterizer_globals.frame_and_vertical_blank_index -
				(short)rasterizer_globals.previous_frame_and_vertical_blank_index);
		main_globals.vblank_flip_delta_index =
			(short)((main_globals.vblank_flip_delta_index + 1) % 15);
		rasterizer_globals.previous_frame_and_vertical_blank_index =
			rasterizer_globals.frame_and_vertical_blank_index;
		rasterizer_globals.d3d_flip_count = *main_globals.d3d_flip_count;
	}

	input_vertical_blank_interrupt();
	return;
}

void main_save_current_solo_map(
	char const *map_name)
{
	FILE *file;

	if ((short)main_get_solo_level_from_name(map_name) != NONE)
	{
		file = fopen("z:\\last_solo.txt", "w");
		if (file)
		{
			fwrite(map_name, 1, csstrlen(map_name) + 1, file);
			fclose(file);
		}
		else
		{
			error(_error_silent, "Couldn't create a file to write the current solo map to");
		}
	}

	return;
}

void main_load_last_solo_map(
	void)
{
	char map_name[256];
	boolean valid_map_name = FALSE;

	if (main_globals.load_last_solo_level && !bink_playback_active())
	{
		FILE *file;

		file = fopen("z:\\last_solo.txt", "r");

		if (file)
		{
			long character_count;

			character_count = fread(map_name, 1, 255, file);
			fclose(file);
			map_name[MIN(character_count, 255)] = 0;
			valid_map_name = main_get_solo_level_from_name(map_name) != NONE;
		}

		if (valid_map_name)
			main_set_map_name(map_name);
		else
			main_set_map_name(scenario_paths[0]);
		main_globals.defer_map_change = FALSE;
		main_globals.load_last_solo_level = FALSE;
	}

	return;
}

void main_load_ui_scenario(
	boolean precache_resources)
{
	struct game_options options;

	game_precache_new_map("levels\\ui\\ui", TRUE);
	match_assert("c:\\halo\\SOURCE\\main\\main.c", 1092,
		!main_globals.main_menu_scenario_loaded);
	game_options_new(&options);
	csstrncpy(options.map_name, "levels\\ui\\ui", NUMBEROF(options.map_name) - 1);
	options.map_name[NUMBEROF(options.map_name) - 1] = 0;
	game_precache_new_map(options.map_name, TRUE);
	game_dispose_from_old_map();
	game_unload();
	game_engine_dispose();
	game_set_game_variant(NULL);
	main_globals.main_menu_scenario_loaded = TRUE;
	main_new_map(&options);
	director_script_camera(TRUE);
	scripted_camera_set(0, 0, NONE);
	main_menu_active(TRUE);
	main_globals.load_last_solo_level = TRUE;
	if (precache_resources)
		main_menu_precache_resources();

	return;
}

void main_roll_credits(
	void)
{
	/* port: standalone MCC scenarios finish through their own menu transition. */
	if (mcc_main_map_completed())
		return;
	error(_error_silent, "congratulations, you won the game!");
	main_menu_load();
	game_end_credits_start();
	return;
}

void main_pregame_render(
	void)
{
	/* Name, type and function scope from the 2003 PC demo PDB and the HCEX PDB (static local
	   struct render_window window of main_pregame_render). Neither PDB records the block: placing it
	   at the top of the function is unattested. January corroborates: .bss +0, the 0xAC-byte window
	   padded to 0xB0 before main_globals, referenced only by this function. */
	static struct render_window window;

	collision_log_continue_period(TRUE);
	sound_render();
	{
		real_point3d position = { 0.0f, 0.0f, 0.0f };
		real_vector3d forward = { 0.0f, 0.0f, 1.0f };
		real_vector3d up = { 0.0f, 1.0f, 0.0f };

		window.local_player_index = NONE;
		window.console_window = TRUE;
		window.rasterizer_camera.position = position;
		window.rasterizer_camera.forward = forward;
		window.rasterizer_camera.up = up;
		window.rasterizer_camera.mirrored = FALSE;
		window.rasterizer_camera.vertical_field_of_view =
			2.0f * arctangent(
				0.75f * render_camera_get_adjusted_field_of_view_tangent(
					DEGREES_TO_RADIANS(80.0f)),
				1.0f);
		compute_window_bounds(
			0,
			1,
			&window.rasterizer_camera.viewport_bounds,
			&window.rasterizer_camera.window_bounds);
		window.rasterizer_camera.z_near = 0.01f;
		window.rasterizer_camera.z_far = 1.0f;
		window.render_camera = window.rasterizer_camera;
		render_frame_pregame(
			&window,
			main_globals.movie);
	}
	collision_log_end_period();

	return;
}

/* port: a network co-op host reverts to its last saved state with its
clock kept going forward, and the clients follow through the co-op syncs
(network_coop.c). FALSE without a saved state, which would reset the map
on the host alone. */
static boolean main_coop_host_revert(
	void)
{
	/* (the last reverts' times: a script whose failure holds again at once
	after the revert would pull everyone back to the save over and over; a
	third in a minute respawns the players at the checkpoint instead) */
	static unsigned long revert_times[2];
	unsigned long milliseconds = system_milliseconds();
	long now = game_time_get();

	if (!game_state_port_saved_game_valid())
		return FALSE;
	if (revert_times[0] && milliseconds - revert_times[0] < 60000)
	{
		error(_error_log, "co-op: a third revert in a minute: the players respawn at the checkpoint instead");
		revert_times[0] = revert_times[1] = 0;
		return FALSE;
	}
	revert_times[0] = revert_times[1];
	revert_times[1] = milliseconds;
	game_state_revert();
	network_coop_reverted(now);
	ui_widgets_disable_pause_game(30);
	return TRUE;
}

static boolean main_coop_host(
	void)
{
	return game_connection() == _game_connection_network_server && network_coop_active();
}

static void main_revert_map_private(
	void)
{
	/* (a network client never reverts on its own: its game is the host's) */
	if (main_coop_host())
		main_coop_host_revert();
	else if (game_connection() != _game_connection_network_client)
	{
		game_state_revert();
		ui_widgets_disable_pause_game(30);
	}
	main_globals.revert_map = FALSE;
	return;
}

static void main_skip_cinematic_private(
	void)
{
	/* port: only a local game or a network co-op host reverts to skip; a
	network game's other machines would be left out of step */
	boolean skippable = cinematic_can_be_skipped();
	boolean skipped = FALSE;

	if (skippable && main_coop_host())
	{
		skipped = main_coop_host_revert();
		if (skipped)
			network_coop_skip_done();
	}
	else if (skippable && game_connection() == _game_connection_local)
	{
		game_state_revert();
		ui_widgets_disable_pause_game(30);
		skipped = TRUE;
	}
	if (skipped)
		main_globals.revert_map = FALSE;
	else if (main_coop_host())
	{
		error(_error_silent, "co-op: cutscene not skipped (skippable %d, saved state %d)",
			skippable, game_state_port_saved_game_valid());
	}
	main_globals.skip_cinematic = FALSE;
	return;
}

static void main_skip_private(
	void)
{
	if (main_globals.skip_ticks && cinematic_in_progress())
	{
		real saved_speed = game_time_get_speed();

		game_time_set_speed(1.0f);
		while (main_globals.skip_ticks-- > 0)
			game_time_update(1.0f / TICKS_PER_SECOND);
		game_time_set_speed(saved_speed);
	}
	else
	{
		error(
			_error_silent,
			"manual skipping doesn't work outside of cinemtatic start/stop...");
	}

	main_globals.skip_ticks = 0;
	main_globals.skip = FALSE;

	return;
}

static void main_saving_map_private(
	void)
{
	game_state_save();
	hud_autosave(FALSE);
	main_globals.save_map_completed = FALSE;
	return;
}

static void main_save_map_private(
	void)
{
	/* port: none while a cinematic can be skipped and the checkpoint its skip
	goes back to is taken: a script that saves every so often would replace it,
	and the skip would play the rest of the cinematic again. The save waits
	(its ticks not counted) until cinematic_skip_stop; a skip lets go of it
	(main_skip_cinematic) */
	if (cinematic_port_holds_checkpoints())
	{
		main_save_last_tick = game_time_get();
		return;
	}
	if (!game_time_get_paused())
	{
		boolean save_map_safely = main_globals.save_map_safely;
		boolean save_map = FALSE;

		if (save_map_safely)
		{
			/* port: the counts below are of game ticks, as on the Xbox, whose
			loop ran once a tick (30 a second). This loop runs once a frame, and
			at 60 frames a second or more a checkpoint gave up in a few seconds,
			before the fight it waited on was over: few checkpoints came */
			long current_tick = game_time_get();
			long ticks = current_tick-main_save_last_tick;

			if (current_tick < main_save_last_tick)
				ticks = 0;
			main_save_last_tick = current_tick;

			while (ticks-- > 0 && !save_map)
			{
				if (main_globals.ticks_unable_to_save++ >= 240 &&
					main_globals.save_map_timeout)
				{
					if (debug_game_save)
						console_printf(FALSE, "gave up trying to save");
					main_globals.saving_map = FALSE;
					return;
				}
				else if (main_globals.ticks_until_next_save_check-- <= 0)
				{
					/* port: in network co-op one player safe is enough, as for
					the scripts' own test (coop_scripts_safe_to_save): the game's
					asks it of every player at once, whom one dead, one in the air
					or one driving past held off, and a level's checkpoint (the
					Maw's before its Warthog run) gave up; a team wipe went back
					to the one before */
					if (coop_scripts_safe_to_save())
					{
						if (main_globals.safe_intervals++ >= 3)
							save_map = TRUE;
					}
					else
					{
						main_globals.safe_intervals = 0;
					}
					main_globals.ticks_until_next_save_check = 10;
				}
			}
		}
		else
		{
			if (debug_game_save)
				console_printf(FALSE, "unsafe save");
			save_map = TRUE;
		}

		if (save_map)
		{
			/* port: remember where the players are, for network co-op respawns */
			players_note_checkpoint();
			hud_autosave(TRUE);
			main_globals.save_map_completed = TRUE;
			main_globals.saving_map = FALSE;
		}
	}

	return;
}

static void main_switch_to_structure_bsp_private(
	void)
{
	scenario_switch_structure_bsp(main_globals.switch_to_structure_bsp_index);
	main_globals.switch_to_structure_bsp_index = NONE;
	hud_load(0);
	return;
}

/* The original post-increment test expires on its 92nd 30 Hz update.
   Render-only frames must not advance it. Saturate so blocked co-op
   respawns can retry indefinitely without overflowing the short counter. */
static boolean main_death_timer_expired(
	short *timer,
	long *last_tick,
	boolean advance)
{
	long current_tick = game_time_get();
	unsigned long elapsed_ticks = 0;

	if (current_tick < *last_tick)
	{
		/* A checkpoint/core load can move the simulation clock backwards. */
		*timer = 0;
	}
	else
	{
		elapsed_ticks = (unsigned long)current_tick - (unsigned long)*last_tick;
	}
	*last_tick = current_tick;

	if (!advance)
		return FALSE;

	*timer = (short)MIN(92, (unsigned long)*timer + elapsed_ticks);
	return *timer >= 92;
}

static void main_lost_map_private(
	void)
{
	if (main_death_timer_expired(
		&main_globals.loss_timer, &main_loss_last_tick,
		!game_time_get_paused()))
	{
		main_globals.lost_map = FALSE;
		main_globals.loss_timer = 0;
		/* port: the co-op host chooses whether a team wipe rewinds or only
		respawns. Scripted failures still revert, or their cutscene would
		never end. A missing save falls back to checkpoint positions. */
		if (game_connection() != _game_connection_network_server)
			game_state_revert();
		else if ((players_are_all_dead() && !network_coop_wipe_reverts()) || !main_coop_host_revert())
			players_respawn_at_checkpoint();
	}
	return;
}

static void main_respawn_private(
	void)
{
	if (main_death_timer_expired(
		&main_globals.respawn_timer, &main_respawn_last_tick,
		!game_time_get_paused() && !cinematic_in_progress()) &&
		players_respawn_coop())
	{
		main_globals.respawn = FALSE;
		main_globals.respawn_timer = 0;
	}
	return;
}

static void main_load_core_private(
	void)
{
	game_state_load_core(main_globals.core_name);
	main_globals.load_core = FALSE;
	return;
}

static void main_save_core_private(
	void)
{
	game_state_save_core(main_globals.core_name);
	main_globals.save_core = FALSE;
	return;
}

static void main_run_demos_private(
	void)
{
	main_globals.run_xdemos = FALSE;
	xbox_demos_launch();
	return;
}

static void main_frame_rate_debug(
	void)
{
	char sample_index;

	if (global_screenshot_count.framerate_reset)
	{
		if (debug_frame_rate)
		{
			sample_index = global_screenshot_count.framerate_sample_index;
		}
		else
		{
			global_screenshot_count.framerate_reset = FALSE;
			csmemset(
				global_screenshot_count.framerate_samples,
				0,
				sizeof(global_screenshot_count.framerate_samples));
			sample_index = 0;
			global_screenshot_count.framerate_flags = 0;
			global_screenshot_count.framerate_sample_index = sample_index;
			global_screenshot_count.framerate_active = FALSE;
			global_screenshot_count.framerate_counter = 0;
			global_screenshot_count.framerate_reset = FALSE;
		}
	}
	else
	{
		sample_index = global_screenshot_count.framerate_sample_index;
	}

	if (debug_frame_rate)
	{
		global_screenshot_count.framerate_samples[sample_index] =
			main_globals.seconds_elapsed;
		SET_FLAG(
			global_screenshot_count.framerate_flags,
			sample_index,
			main_globals.seconds_elapsed > 1.08/TICKS_PER_SECOND);

		sample_index++;
		sample_index = (char)(sample_index % (long)NUMBEROF(global_screenshot_count.framerate_samples));
		global_screenshot_count.framerate_sample_index = sample_index;
		global_screenshot_count.framerate_reset = TRUE;

		if (global_screenshot_count.framerate_active)
		{
			if (global_screenshot_count.framerate_sample_index == 0)
			{
				if (global_screenshot_count.framerate_flags == 0)
				{
					if (++global_screenshot_count.framerate_counter >= 60)
					{
						global_screenshot_count.framerate_counter = 0;
						global_screenshot_count.framerate_active = FALSE;
						return;
					}
				}
				else
				{
					global_screenshot_count.framerate_counter = 0;
					return;
				}
			}
		}
		else if (global_screenshot_count.framerate_flags == 0xFF)
		{
			SYSTEMTIME system_time;
			char core_name[256];
			char init_file_name[260];
			char const *scenario_name =
				tag_name_strip_path(tag_get_name(global_scenario_index));
			FILE *file;

			GetSystemTime(&system_time);
			/* port: snprintf, here and below (a map's tag's name may be any
			length) */
			snprintf(
				core_name,
				sizeof(core_name),
				"%s_slow_%d_%d_%d_%d_%d_%d.bin",
				scenario_name,
				system_time.wMonth,
				system_time.wDay,
				system_time.wYear,
				system_time.wHour,
				system_time.wMinute,
				system_time.wSecond);
			game_state_save_core(core_name);
			snprintf(init_file_name, sizeof(init_file_name), "d:\\%s_init.txt", scenario_name);

			file = fopen(init_file_name, "r");
			if (!file)
			{
				file = fopen(init_file_name, "wt");
				fprintf(file, "map_name %s\n", scenario_name);
			}
			else
			{
				fclose(file);
				file = fopen(init_file_name, "a+t");
			}

			fprintf(file, ";core_load_name_at_startup %s\n", core_name);
			fflush(file);
			fclose(file);
			global_screenshot_count.framerate_active = TRUE;
		}
	}

	return;
}

static void main_won_map_private(
	void)
{
	short level;
	short local_player_index;
	/* port: the level the scripts named with map_name (main_script_map_name),
	if they did */
	char next[NUMBEROF(main_script_next_level)];

	csstrncpy(next, main_script_next_level, sizeof(next) - 1);
	next[sizeof(next) - 1] = 0;
	main_script_next_level[0] = 0;

	/* port: explicit MCC campaigns own their mission order and host decision.
	 * The Xbox/CE level table below cannot represent package mission identities. */
	if (mcc_main_campaign_won(next[0] ? next : NULL))
	{
		main_globals.won_map = FALSE;
		return;
	}

	/* port: when a network co-op level is won, the round ends for everyone as
	in multiplayer, back to the lobby, and the next round is the campaign's
	next level (The Maw's: The Pillar of Autumn), or the one the scripts named.
	A level not in the campaign repeats. */
	if (game_connection() == _game_connection_network_server && network_coop_active())
	{
		struct network_game *game = network_game_get_game();

		main_globals.won_map = FALSE;
		level = game ? main_get_solo_level_from_name(game->map.name) : NONE;
		player_profile_save_level_completed(0);
		network_game_server_port_cooperative_won(next[0] ? next : level != NONE ?
			main_get_solo_level_name((level + 1) % NUMBER_OF_SINGLE_PLAYER_LEVELS) : NULL);
		return;
	}
	main_globals.want_to_be_at_main_menu = TRUE;
	main_globals.won_map = FALSE;
	level = main_get_solo_level_from_name(main_globals.soloplayer_map_name);
	/* port: a level not in the campaign (a Custom Edition map's) has no next
	one, rather than the first */
	level = level == NONE ? NONE : level + 1;
	if (level >= 10)
		level = NONE;
	for (local_player_index = 0; local_player_index < player_spawn_count; local_player_index++)
		player_profile_save_level_completed(local_player_index);
	/* port: a Custom Edition campaign's next level, as its scripts named it,
	begun anew as ui_set_next_level begins one (no saved game taken) */
	if (next[0])
	{
		main_set_map_name(next);
		main_disallow_persistent_storage();
		return;
	}
	ui_set_next_level(level);
	return;
}

void main_set_map_name(
	char const *map_name)
{
	main_globals.want_to_be_at_main_menu = FALSE;
	csstrncpy(main_globals.soloplayer_map_name, map_name, NUMBEROF(main_globals.soloplayer_map_name) - 1);
	main_globals.soloplayer_map_name[NUMBEROF(main_globals.soloplayer_map_name) - 1] = 0;
	main_globals.allow_persistent_storage = TRUE;
	if ((game_in_editor() || game_in_progress()) && main_globals.connection == _game_connection_local)
		main_globals.defer_map_change = TRUE;
	return;
}

static void main_exit(
	void)
{
#ifdef HALO_PROFILE
	/* port: the profiling build's recording written out before the game goes */
	profile_trace_shutdown();
#endif
	switch (main_globals.connection)
	{
	case _game_connection_network_server:
		dispose_global_network_game_client();
		dispose_global_network_game_server();
		break;
	case _game_connection_network_client:
		dispose_global_network_game_client();
		break;
	}

	game_dispose_from_old_map();
	game_dispose();
	debug_keys_dispose();
	console_dispose();

	return;
}

static void main_reset_time(
	void)
{
	main_globals.frame_start_milliseconds = system_milliseconds();
	main_globals.rasterizer_target_index = rasterizer_globals.vertical_blank_index;
	return;
}

static boolean main_framerate_throttle_enabled(
	void)
{
	return rasterizer_globals.framerate_throttle;
}

/* The native ports draw a frame whenever the display can show one, paced
by vsync, and frames fall between the 30 Hz ticks
(port/linux/game/render_interpolation.c): no vertical blank throttle, and
the frame is timed with the performance counter. */
static void main_update_time_unthrottled(
	void)
{
	static LARGE_INTEGER previous_counter;
	LARGE_INTEGER counter;
	LARGE_INTEGER frequency;
	real seconds_elapsed = 0.0f;

	QueryPerformanceCounter(&counter);
	QueryPerformanceFrequency(&frequency);
	if (previous_counter.QuadPart && frequency.QuadPart)
	{
		seconds_elapsed = (real)((double)(counter.QuadPart - previous_counter.QuadPart) /
			(double)frequency.QuadPart);
	}
	previous_counter = counter;

	if (main_globals.movie)
	{
		seconds_elapsed = main_globals.recording_dt;
	}
	else if (halo_fixed_step() && main_globals.connection == _game_connection_local)
	{
		/* port: debug.fixed_step, a frame a tick whatever time has passed, as
		a movie's frames are its recording's: what runs once a frame (aiming)
		sees the game's time, and the game plays the same every time */
		seconds_elapsed = 1.0f / TICKS_PER_SECOND;
	}
	else
	{
		seconds_elapsed = PIN(seconds_elapsed, 0.0f, 1.0f);
		if (main_globals.connection == _game_connection_local)
		{
			if (debug_force_frame_rate_update)
				seconds_elapsed = CEILING(seconds_elapsed, 0.03333333507180214f);
			else
				seconds_elapsed = CEILING(seconds_elapsed, 0.06666667014360428f);
		}
	}

	main_globals.frame_start_milliseconds = system_milliseconds();
	main_globals.seconds_elapsed = seconds_elapsed;
	profile_seconds_elapsed(seconds_elapsed);
	main_globals.rasterizer_initial_index =
		rasterizer_globals.frame_and_vertical_blank_index;
}

static void main_update_time(
	void)
{
	unsigned long end_milliseconds;
	long milliseconds_elapsed;
	short requested_rate;
	short requested_interval;
	short elapsed_game_ticks;
	short slot;
	boolean framerate_throttle;
	__int64 target_index;
	__int64 minimum_target_index;
	short short_target_index;
	real seconds_elapsed;

	if (halo_interpolation_enabled())
	{
		main_update_time_unthrottled();
		return;
	}
	end_milliseconds = system_milliseconds();
	minimum_target_index = MAX(
		main_globals.rasterizer_target_index,
		main_globals.rasterizer_throttle_end_index);
	target_index = minimum_target_index;
	framerate_throttle = main_framerate_throttle_enabled();

	if (framerate_throttle)
	{
		csstrcpy(main_globals.vblank_debug_string, "");
		if (global_frame_rate_throttle && rasterizer_globals.framerate_throttle_target >= 0)
		{
			requested_interval = (short)(
				60 / (rasterizer_globals.framerate_throttle_target ? rasterizer_globals.framerate_throttle_target : 30));
			main_globals.vblank_interval_requested = requested_interval;

			if (rasterizer_globals.framerate_throttle_debug)
			{
				short best_interval;
				short current_interval;

				elapsed_game_ticks = game_time_get_elapsed();

				_snprintf(
					main_globals.vblank_debug_string + csstrlen(main_globals.vblank_debug_string),
					sizeof(main_globals.vblank_debug_string) - csstrlen(main_globals.vblank_debug_string),
					"last%6I64d init%6I64d achv%6I64d pres%6I64d g%d cur%d... ",
					main_globals.rasterizer_target_index,
					main_globals.rasterizer_initial_index,
					main_globals.rasterizer_throttle_start_index,
					main_globals.rasterizer_throttle_end_index,
					elapsed_game_ticks,
					main_globals.vblank_interval_current);

				best_interval = 5;
				short_target_index = (short)minimum_target_index;
				for (slot = 5; slot > 0; slot--)
				{
					short displayed_failure_count;
					short target_age;
					short slot_bucket;
					boolean ignore_failure;
					char const *label;
					__int64 last_failure_index;
					__int64 target_age_raw;

					ignore_failure = FALSE;
					displayed_failure_count = main_globals.vblank_failure_counts[slot];
					if (displayed_failure_count >= 99)
						displayed_failure_count = 99;

					last_failure_index = main_globals.vblank_last_failure_indices[slot];
					target_age_raw = short_target_index - last_failure_index;
					if (target_age_raw < 99)
						target_age = (short)target_age_raw;
					else
						target_age = 99;

					slot_bucket = (short)((slot + 1) / 2);
					current_interval = main_globals.vblank_interval_current;
					if ((short)((current_interval + 1) / 2) > slot_bucket &&
						(short)(current_interval / 2) <= slot_bucket &&
						elapsed_game_ticks > slot_bucket)
					{
						ignore_failure = TRUE;
					}

					if (main_globals.rasterizer_throttle_start_index >
						main_globals.rasterizer_initial_index + slot)
					{
						if (!ignore_failure)
						{
							long failure_count;

							failure_count = main_globals.vblank_failure_counts[slot];
							main_globals.vblank_failure_counts[slot] =
								(short)(failure_count + 1);
							main_globals.vblank_last_failure_indices[slot] = minimum_target_index;
						}

						_snprintf(
							main_globals.vblank_debug_string + csstrlen(main_globals.vblank_debug_string),
							sizeof(main_globals.vblank_debug_string) - csstrlen(main_globals.vblank_debug_string),
							"(%s%2d) ",
							ignore_failure ? "ignor" : "fail ",
							displayed_failure_count);
					}
					else
					{
						if (minimum_target_index >= last_failure_index + 15)
						{
							main_globals.vblank_failure_counts[slot] = 0;
							_snprintf(
								main_globals.vblank_debug_string + csstrlen(main_globals.vblank_debug_string),
								sizeof(main_globals.vblank_debug_string) - csstrlen(main_globals.vblank_debug_string),
								"(ok   %2d) ",
								target_age);
						}
						else
						{
							label = main_globals.vblank_failure_counts[slot] >= 4 ? "dn" : "wt";
							_snprintf(
								main_globals.vblank_debug_string + csstrlen(main_globals.vblank_debug_string),
								sizeof(main_globals.vblank_debug_string) - csstrlen(main_globals.vblank_debug_string),
								"(%s%2d/%2d) ",
								label,
								displayed_failure_count,
								target_age);
						}
					}

					if (slot >= main_globals.vblank_interval_requested &&
						main_globals.vblank_failure_counts[slot] < 4)
					{
						best_interval = slot;
					}
				}

				if (best_interval == 0)
					requested_rate = 999;
				else
					requested_rate = (short)(60 / best_interval);

				if (main_globals.vblank_interval_current < best_interval)
				{
					_snprintf(
						main_globals.vblank_debug_string + csstrlen(main_globals.vblank_debug_string),
						sizeof(main_globals.vblank_debug_string) - csstrlen(main_globals.vblank_debug_string),
						" FAILDOWN %d",
						requested_rate);
				}
				else if (best_interval < main_globals.vblank_interval_current)
				{
					_snprintf(
						main_globals.vblank_debug_string + csstrlen(main_globals.vblank_debug_string),
						sizeof(main_globals.vblank_debug_string) - csstrlen(main_globals.vblank_debug_string),
						" RESTORE  %d",
						requested_rate);
				}
				else
				{
					_snprintf(
						main_globals.vblank_debug_string + csstrlen(main_globals.vblank_debug_string),
						sizeof(main_globals.vblank_debug_string) - csstrlen(main_globals.vblank_debug_string),
						" MAINTAIN %d",
						requested_rate);
				}

				_snprintf(
					main_globals.vblank_debug_string + csstrlen(main_globals.vblank_debug_string),
					sizeof(main_globals.vblank_debug_string) - csstrlen(main_globals.vblank_debug_string),
					" des %d targ%6I64d",
					best_interval,
					minimum_target_index + best_interval);
				main_globals.vblank_interval_current = best_interval;
			}
			else
			{
				main_globals.vblank_interval_current = requested_interval;
			}

			target_index += main_globals.vblank_interval_current;
		}
	}
	else
	{
		milliseconds_elapsed = (long)(real)(
			(long)end_milliseconds - (long)main_globals.frame_start_milliseconds);
		main_globals.vblank_interval_held = FALSE;
		if (milliseconds_elapsed < 33)
		{
			profile_idle_start();
			if (global_frame_rate_throttle)
				Sleep(33 - milliseconds_elapsed);
			profile_idle_end();
		}
		else
		{
			profile_lapsed_msec(milliseconds_elapsed - 33);
		}
	}

	end_milliseconds = system_milliseconds();
	if ((__int64)rasterizer_globals.frame_and_vertical_blank_index > target_index)
		target_index = (__int64)rasterizer_globals.frame_and_vertical_blank_index;

	if (framerate_throttle)
	{
		seconds_elapsed =
			(real)(target_index - (__int64)main_globals.rasterizer_target_index) *
			0.01666666753590107f;
	}
	else
	{
		seconds_elapsed =
			(real)(end_milliseconds - main_globals.frame_start_milliseconds) *
			0.001000000047497451f;
	}

	if (main_globals.movie)
	{
		seconds_elapsed = main_globals.recording_dt;
	}
	else if (halo_fixed_step() && main_globals.connection == _game_connection_local)
	{
		/* port: debug.fixed_step, as main_update_time_unthrottled */
		seconds_elapsed = 1.0f / TICKS_PER_SECOND;
	}
	else
	{
		seconds_elapsed = PIN(seconds_elapsed, 0.0f, 1.0f);
		if (main_globals.connection == _game_connection_local)
		{
			if (debug_force_frame_rate_update)
				seconds_elapsed = CEILING(seconds_elapsed, 0.03333333507180214f);
			else
				seconds_elapsed = CEILING(seconds_elapsed, 0.06666667014360428f);
		}
	}

	main_globals.frame_start_milliseconds = end_milliseconds;
	main_globals.rasterizer_target_index = target_index;
	main_globals.seconds_elapsed = seconds_elapsed;
	profile_seconds_elapsed(seconds_elapsed);
	main_globals.rasterizer_initial_index =
		rasterizer_globals.frame_and_vertical_blank_index;

	return;
}

void main_rasterizer_throttle(
	void)
{
	__int64 entry_index;
	__int64 target_index;
	__int64 end_index;
	__int64 index_delta;
	__int64 elapsed_index;
	unsigned long start_milliseconds;
	short lapsed_frames;
	boolean did_throttle;
	boolean synchronized;
	char const *description;

	entry_index = rasterizer_globals.frame_and_vertical_blank_index;
	did_throttle = FALSE;
	main_globals.rasterizer_throttle_start_index =
		rasterizer_globals.frame_and_vertical_blank_index + 1;
	if (rasterizer_globals.framerate_throttle && !halo_interpolation_enabled())
	{
		target_index = main_globals.rasterizer_target_index;
		target_index--;
		if ((__int64)rasterizer_globals.frame_and_vertical_blank_index < target_index)
		{
			start_milliseconds = system_milliseconds();
			did_throttle = TRUE;
			profile_idle_start();
			while ((__int64)rasterizer_globals.frame_and_vertical_blank_index < target_index)
			{
				/* port: a sleep between looks, not only while a map
				precaches: the Xbox spun here, which on a port holds a core
				busy for most of each frame of a game held at 30 frames a
				second (display.interpolation off). A millisecond is well
				inside a 33 ms frame (Windows sets its timer to 1 ms:
				win32_posix.c) */
				Sleep(1);
				if (system_milliseconds() > start_milliseconds + 1000)
				{
					console_warning(
						"stuck waiting for VBLANK callback! disabling rasterizer framerate control");
					rasterizer_globals.framerate_throttle = FALSE;
					break;
				}
			}
			profile_idle_end();
		}
	}

	end_index = rasterizer_globals.frame_and_vertical_blank_index + 1;
	synchronized =
		main_globals.vblank_interval_current == main_globals.vblank_interval_requested;
	main_globals.rasterizer_throttle_end_index = end_index;
	index_delta = end_index - main_globals.rasterizer_target_index;
	lapsed_frames = (short)PIN(index_delta, 0, 0x7FFF);

	if (did_throttle)
	{
		elapsed_index = rasterizer_globals.frame_and_vertical_blank_index - entry_index;
		description = "THROTTLE";
	}
	else
	{
		elapsed_index = lapsed_frames;
		description = lapsed_frames == 0 ? "SYNCED  " : "LAPSED  ";
	}

	_snprintf(
		main_globals.vblank_debug_string + csstrlen(main_globals.vblank_debug_string),
		sizeof(main_globals.vblank_debug_string) - csstrlen(main_globals.vblank_debug_string),
		"%6I64d(targ%6I64d %s%2d)",
		entry_index,
		main_globals.rasterizer_target_index,
		description,
		elapsed_index);
	main_globals.vblank_interval_held =
		main_globals.vblank_interval_current > 0 && lapsed_frames == 0;
	profile_lapsed_frames(
		lapsed_frames,
		synchronized,
		main_globals.vblank_debug_string);

	return;
}

static void main_setup_connection(
	void)
{
	struct game_options options;

	if (main_globals.playback_last_recording)
		main_globals.want_to_be_at_main_menu = FALSE;

	if (main_globals.want_to_be_at_main_menu)
	{
		main_menu_load();
		return;
	}

	if (main_globals.playback_last_recording)
	{
		main_globals.connection = _game_connection_film_playback;
		error(_error_silent, "error opening saved film");
		main_globals.want_to_be_at_main_menu = TRUE;
		main_menu_load();
	}
	else
	{
		main_globals.connection = _game_connection_local;
		game_options_new(&options);
		csstrncpy(
			options.map_name,
			main_globals.soloplayer_map_name,
			NUMBEROF(options.map_name) - 1);
		options.map_name[NUMBEROF(options.map_name) - 1] = 0;
		options.difficulty = global_difficulty_level;
		game_precache_new_map(options.map_name, TRUE);
		game_dispose_from_old_map();
		main_new_map(&options);
	}

	return;
}

static void main_initialize_time(
	void)
{
	main_globals.frame_start_milliseconds = system_milliseconds();
	main_globals.rasterizer_target_index = 0;
	rasterizer_set_vblank_callback(main_vertical_blank_interrupt_handler);
	main_globals.vblank_flip_delta_index = 0;
	csmemset(main_globals.vblank_flip_deltas, 0, sizeof(main_globals.vblank_flip_deltas));
	main_globals.d3d_flip_count = d3d_find_flipcount();
	return;
}

static void main_reset_map_private(
	void)
{
	if (!game_time_get_paused())
	{
		scenario_switch_structure_bsp(0);
		game_dispose_from_old_map();
		input_flush();
		game_initialize_for_new_map();
		create_local_players();
		game_time_start();
		game_initial_pulse();
		ui_widgets_disable_pause_game(30);

		main_globals.reset_map = FALSE;
	}

	return;
}

static void screenshot_render(
	struct render_window *windows)
{
	char path[512];
	struct file_reference reference;
	point2d screenshot_page_index;
	point2d screenshot_index;
	struct bitmap_data *bitmap;
	long screenshot_width;
	long screenshot_height;

	global_screenshot_size = PIN(global_screenshot_size, 1, 3);
	screenshot_width =
		rasterizer_globals.reserved04.screen_bounds.x1 -
		rasterizer_globals.reserved04.screen_bounds.x0;
	screenshot_height =
		rasterizer_globals.reserved04.screen_bounds.y1 -
		rasterizer_globals.reserved04.screen_bounds.y0;
	bitmap = bitmap_2d_new(
		global_screenshot_size * screenshot_width,
		global_screenshot_size * screenshot_height,
		0,
		_bitmap_format_x8r8g8b8);

	if (bitmap && bitmap->base_address)
	{
		console_printf(TRUE, "");
		console_close();

		for (screenshot_page_index.y = 0;
			screenshot_page_index.y < global_screenshot_count.count;
			screenshot_page_index.y++)
		{
			for (screenshot_page_index.x = 0;
				screenshot_page_index.x < global_screenshot_count.count;
				screenshot_page_index.x++)
			{
				for (screenshot_index.y = 0;
					screenshot_index.y < global_screenshot_size;
					screenshot_index.y++)
				{
					for (screenshot_index.x = 0;
						screenshot_index.x < global_screenshot_size;
						screenshot_index.x++)
					{
						if (global_screenshot_count.count > 1 ||
							global_screenshot_size > 1)
						{
							render_frame(
								windows,
								1,
								&screenshot_page_index,
								&screenshot_index,
								bitmap,
								0.0f);
							render_frame_present(&screenshot_index, bitmap);
						}
						else
						{
							render_frame(windows, 1, NULL, NULL, bitmap, 0.0f);
							render_frame_present(NULL, bitmap);
						}
					}
				}

				sprintf(
					path,
					"%dscreenshot%d%d.tif",
					main_globals.screenshot_identifier,
					screenshot_page_index.y,
					screenshot_page_index.x);
				file_reference_create_from_path(&reference, path, FALSE);
				screenshot_record(bitmap, &reference);
			}
		}

		main_globals.screenshot_identifier++;
		bitmap_delete(bitmap);
	}

	global_screenshot_count.count = 0;
	return;
}

void main_framerate_render(
	void)
{
	if (display_framerate)
	{
		long font_tag_index;

		font_tag_index = hud_globals->messaging.single_player_font.index;
		if (font_tag_index != NONE)
		{
			real frame_seconds;
			real frame_rate_real;
			long frame_rate;
			rectangle2d bounds;
			/* room for three digits under C99 snprintf, which (unlike MSVC's
			_snprintf) keeps a byte of the count for the terminator */
			char frame_rate_string[8];

			bounds = render.camera.window_bounds;
			/* The native ports draw at the display's refresh rate, up to
			hundreds of frames a second: show frames per second averaged over
			half a second, counting each frame once (split screen draws this
			once per view), with no 100 cap. */
			{
				static unsigned long counted_frame_start = 0;
				static real window_seconds = 0.0f;
				static long window_frames = 0;
				static long average_frame_rate = 0;

				if (main_globals.frame_start_milliseconds != counted_frame_start)
				{
					counted_frame_start = main_globals.frame_start_milliseconds;
					window_seconds += main_globals.seconds_elapsed;
					window_frames++;
					if (window_seconds >= 0.5f)
					{
						average_frame_rate = fast_ftol(window_frames / window_seconds);
						window_seconds = 0.0f;
						window_frames = 0;
					}
				}
				frame_rate = PIN(average_frame_rate, 0, 999);
				(void)frame_seconds;
				(void)frame_rate_real;
			}

			_snprintf(
				frame_rate_string,
				NUMBEROF(frame_rate_string) - 1,
				"%d",
				(short)frame_rate);
			frame_rate_string[NUMBEROF(frame_rate_string) - 1] = 0;
			bounds.x0 = (short)(bounds.x1 - 50);
			bounds.y0 = (short)(bounds.y1 - 50);
			draw_string_set_format(NONE, _text_justification_left, 0);
			draw_string_set_color(
				(short)frame_rate >= 30 ? global_real_argb_green : global_real_argb_red);
			draw_string_set_font(font_tag_index);
			rasterizer_draw_string(&bounds, NULL, NULL, 0, frame_rate_string);
		}
	}

	if (display_vblank_deltas)
	{
		long font_tag_index;

		font_tag_index = hud_globals->messaging.single_player_font.index;
		if (font_tag_index != NONE)
		{
			short index;
			rectangle2d bounds;
			char delta_string[4];

			bounds = render.camera.window_bounds;
			bounds.x0 = (short)(bounds.x1 - 50);
			bounds.y0 = (short)(bounds.y1 - 50);
			index = (short)((main_globals.vblank_flip_delta_index + 14) % 15);
			while (index != main_globals.vblank_flip_delta_index)
			{
				bounds.y0 -= 20;
				bounds.y1 -= 20;
				_snprintf(
					delta_string,
					NUMBEROF(delta_string) - 1,
					"%d",
					main_globals.vblank_flip_deltas[index]);
				delta_string[NUMBEROF(delta_string) - 1] = 0;
				draw_string_set_format(NONE, _text_justification_right, 0);
				draw_string_set_font(font_tag_index);
				draw_string_set_color(
					main_globals.vblank_flip_deltas[index] == 2
						? global_real_argb_white
						: global_real_argb_red);
				rasterizer_draw_string(&bounds, NULL, NULL, 0, delta_string);
				index = (short)((index + 14) % 15);
			}
		}
	}

	if (display_precache_progress && cache_files_precache_in_progress())
	{
		long font_tag_index;

		font_tag_index = hud_globals->messaging.single_player_font.index;
		if (font_tag_index != NONE)
		{
			real progress;
			long progress_percent;
			rectangle2d bounds;
			char progress_string[4];

			if (cache_files_precache_map_status(&progress) == 0)
			{
				real progress_percent_real;

				bounds = render.camera.window_bounds;
				progress_percent_real = progress * 100.0f;
				progress_percent = fast_ftol(progress_percent_real);
				_snprintf(
					progress_string,
					NUMBEROF(progress_string) - 1,
					"%d",
					(short)progress_percent);
				progress_string[NUMBEROF(progress_string) - 1] = 0;
				bounds.x0 = (short)(bounds.x1 - 50);
				bounds.y0 = (short)(bounds.y1 - 100);
				draw_string_set_format(NONE, _text_justification_left, 0);
				draw_string_set_color(global_real_argb_purple);
				draw_string_set_font(font_tag_index);
				rasterizer_draw_string(&bounds, NULL, NULL, 0, progress_string);
			}
		}
	}

	return;
}

void halt_and_catch_fire(
	void)
{
	short gamepad_index;
	long font_tag_index;
	struct scenario *scenario;
	struct rasterizer_frame_begin_parameters frame_parameters;
	struct rasterizer_window_begin_parameters window_parameters;
	#ifdef HALO_NATIVE_BUILD_INFO
	char banner[256];
	char label[96];
	#endif

#ifdef HALO_WEB
	/* port: a browser shows only what a frame of its own main loop drew, and
	this screen never returns to it: the game stops, and the page shows the
	error from the log, where it already is (port/web/README.md) */
	emscripten_cancel_main_loop();
	abort();
#endif
	/* port: a dedicated server has no screen to halt on: its error logged,
	it ends with exit code 1 (port/linux/game/server_director.c) */
	if (halo_server_mode())
		server_fatal(error_get());
	if (!global_screenshot_count.halt_recursion_lock)
	{
		scenario = global_scenario_try_and_get();
		global_screenshot_count.halt_recursion_lock = TRUE;
		for (gamepad_index = 0; gamepad_index < MAXIMUM_GAMEPADS; gamepad_index++)
		{
			if (input_has_gamepad(gamepad_index))
			{
				input_set_gamepad_rumbler_state(
					gamepad_index,
					0,
					0);
			}
		}

		if (scenario)
		{
			font_tag_index = interface_get_tag_index(_interface_font_terminal);
		}
		if (!scenario || font_tag_index == NONE)
		{
			font_tag_index = tag_loaded(
				FONT_GROUP_TAG,
				"old tags\\internal system plain");
		}
		#ifdef HALO_NATIVE_BUILD_INFO
		main_native_build_label(label, sizeof(label));
		_snprintf(banner, sizeof(banner) - 1,
			"%s\r\nCompiled: %s %s\r\nFull log: debug.txt (game data folder)\r\nRecent messages (newest first):",
			label, __DATE__, __TIME__);
		banner[sizeof(banner) - 1] = 0;
		#endif

		while (TRUE)
		{
			rasterizer_reset_state();
			csmemset(&frame_parameters, 0, sizeof(frame_parameters));
			rasterizer_frame_begin(&frame_parameters);
			rasterizer_windows_begin();

			csmemset(&window_parameters, 0, sizeof(window_parameters));
			window_parameters.camera.position = *global_origin3d;
			window_parameters.camera.forward = *global_forward3d;
			window_parameters.camera.up = *global_up3d;
			window_parameters.camera.mirrored = FALSE;
			window_parameters.camera.vertical_field_of_view =
				2.0f * arctangent(
					0.75f * render_camera_get_adjusted_field_of_view_tangent(
						DEGREES_TO_RADIANS(80.0f)),
					1.0f);
			window_parameters.camera.z_near = rasterizer_globals.near_clip_distance;
			window_parameters.camera.viewport_bounds.x0 = 0;
			window_parameters.camera.viewport_bounds.x1 = (short)halo_screen_width();
			window_parameters.camera.viewport_bounds.y0 = 0;
			window_parameters.camera.viewport_bounds.y1 = 480;
			window_parameters.camera.z_far = rasterizer_globals.far_clip_distance;
			render_camera_build_frustum(
				&window_parameters.camera,
				NULL,
				&window_parameters.frustum,
				TRUE);
			window_parameters.rasterizer_target = 0;
			window_parameters.fog.atmospheric_color = *global_real_rgb_blue;
			window_parameters.fog.atmospheric_maximum_distance = 0.0f;
			window_parameters.fog.atmospheric_minimum_distance = 0.0f;
			window_parameters.fog.planar_mode = 0;
			render.camera = window_parameters.camera;
			rasterizer_window_begin(&window_parameters);

			if (font_tag_index != NONE)
			{
				point2d cursor = { 0, 0 };
				rectangle2d bounds = rasterizer_globals.reserved04.frame_bounds;

				draw_string_set_draw_mode(
					font_tag_index,
					NONE,
					0,
					0,
					global_real_argb_white);
				draw_string_set_tab_stops(NULL, 0);
				draw_string_set_color(global_real_argb_white);
				rasterizer_draw_string(
					&bounds,
					NULL,
					&cursor,
					-4,
					#ifdef HALO_NATIVE_BUILD_INFO
					banner);
				#else
					"halobeta xbox 01.01.14.2342 built at: Jan 14 2002 12:49:20");
				#endif
				bounds.y0 = cursor.y - 1;
				rasterizer_draw_string(
					&bounds,
					NULL,
					&cursor,
					-4,
					#ifdef HALO_NATIVE_BUILD_INFO
					main_native_error_tail(error_get()));
				#else
					error_get());
				#endif
			}

			rasterizer_transparent_geometry_draw(TRUE);
			rasterizer_transparent_geometry_draw(FALSE);
			rasterizer_debug_draw();
			rasterizer_window_end();
			rasterizer_windows_end();
			rasterizer_frame_end();
			rasterizer_present(NULL, NULL);
			input_update();
		}
	}

	exit(0);
	return;
}

void main_loop_of_death(
	void)
{
	/* port: as halt_and_catch_fire, on a dedicated server */
	if (halo_server_mode())
		server_fatal(error_get());
	while (TRUE)
	{
		input_frame_begin();
		input_update();
		shell_idle();
		event_manager_update();
		telnet_console_process();
		process_ui_widgets();
		main_pregame_render();
		main_rasterizer_throttle();
		main_present_frame();
		input_frame_end();
	}

	return;
}

static void main_game_render(
	double time_delta_since_tick_sec)
{
	boolean force_single_screen;
	long window_index;
	struct render_window *window;
	struct observer_result const *observer;
	long player_window_count;
	long window_count;
	short last_local_player_index;

	lock_global_random_seed();
	collision_log_continue_period(TRUE);
	sound_render();
	force_single_screen = game_engine_force_single_screen();
	last_local_player_index = NONE;

	window_count = PIN(local_player_count(), 1, MAXIMUM_LOCAL_PLAYERS);
	player_window_count = window_count;
	if (force_single_screen || cinematic_in_progress())
	{
		window_count = 1;
		player_window_count = 1;
	}

	for (window_index = 0; window_index < player_window_count; window_index++)
	{
		window = &global_screenshot_count.windows[window_index];
		observer = NULL;

		compute_window_bounds(
			window_index,
			player_window_count,
			&window->rasterizer_camera.viewport_bounds,
			&window->rasterizer_camera.window_bounds);
		if (force_single_screen)
		{
			window->local_player_index = NONE;
		}
		else if (window_index < window_count)
		{
			if (!rasterizer_debug_options.force_all_player_views_to_default_player ||
				last_local_player_index == NONE)
			{
				if (game_connection() == _game_connection_film_playback)
				{
					last_local_player_index = 0;
				}
				else
				{
					last_local_player_index = local_player_get_next(last_local_player_index);
				}
			}

			window->local_player_index = last_local_player_index;
			observer = observer_get_camera(window->local_player_index);
			observer = render_interpolation_camera(window->local_player_index, observer);
		}
		else
		{
			window->local_player_index = NONE;
		}

		set_window_camera_values(window, observer);
		window->console_window = FALSE;
	}

	window = &global_screenshot_count.windows[player_window_count];
	compute_window_bounds(
		0,
		1,
		&window->rasterizer_camera.viewport_bounds,
		&window->rasterizer_camera.window_bounds);
	window->local_player_index = NONE;
	window->console_window = TRUE;
	set_window_camera_values(window, NULL);

	if (global_screenshot_count.count <= 0)
	{
		render_frame(
			global_screenshot_count.windows,
			player_window_count + 1,
			NULL,
			NULL,
			main_globals.movie,
			(real)time_delta_since_tick_sec);
	}
	else
	{
		screenshot_render(global_screenshot_count.windows);
	}

	collision_log_end_period();
	unlock_global_random_seed();
	return;
}

/* port: the main loop's steps, timed in the profiling build */
PROFILE_SECTION(main_input_section, "input")
PROFILE_SECTION(main_platform_events_section, "platform_events")
PROFILE_SECTION(main_network_start_frame_section, "network_start_frame")
PROFILE_SECTION(main_update_time_section, "main_update_time")
PROFILE_SECTION(main_ui_update_section, "ui_update")
PROFILE_SECTION(main_network_end_frame_section, "network_end_frame")
PROFILE_SECTION(main_game_time_update_section, "game_time_update")
PROFILE_SECTION(main_non_deterministic_update_section, "non_deterministic_update")
PROFILE_SECTION(main_throttle_section, "throttle")
PROFILE_SECTION(main_present_section, "present")

#ifdef HALO_WEB
/* port: in a browser, the page's frames call the main loop one iteration at a
time, and what an iteration drew shows once it returns (port/web/README.md) */
static boolean main_loop_iteration(void);

static void main_loop_web_frame(
	void)
{
	if (!main_loop_iteration())
	{
		emscripten_cancel_main_loop();
		error(_error_silent, "end of saved film");
		main_exit();
	}
}
#endif

void main_loop(
	void)
{
#ifndef HALO_WEB
	boolean render_frame;
	long connection;
#endif

	if (!game_in_editor())
	{
		csstrncpy(main_globals.soloplayer_map_name, "levels\\b30\\b30", NUMBEROF(main_globals.soloplayer_map_name)-1);
		main_globals.soloplayer_map_name[NUMBEROF(main_globals.soloplayer_map_name)-1] = '\0';
	}

	main_globals.want_to_be_at_main_menu = !game_in_editor();
	main_globals.switch_to_structure_bsp_index = NONE;
	main_globals.halt_time_scale = TRUE;

	console_initialize();
	debug_keys_initialize();
	game_initialize();
	console_startup();
	main_setup_connection();
	main_initialize_time();

#ifdef HALO_WEB
	emscripten_set_main_loop(main_loop_web_frame, 0, TRUE);
}

static boolean main_loop_iteration(
	void)
{
	boolean render_frame;
	long connection;

	do
#else
	while (TRUE)
#endif
	{
		if (!game_in_editor())
		{
			if (main_globals.switch_to_structure_bsp_index!=NONE)
			{
				main_switch_to_structure_bsp_private();
			}

			if (main_globals.lost_map)
			{
				main_lost_map_private();
			}

			if (main_globals.won_map)
			{
				main_won_map_private();
			}

			if (main_globals.respawn)
			{
				main_respawn_private();
			}

			if (main_globals.save_map_completed)
			{
				main_saving_map_private();
			}

			if (main_globals.defer_map_change)
			{
				main_change_map_name();
			}

			if (main_globals.revert_map)
			{
				main_revert_map_private();
			}

			if (main_globals.skip_cinematic)
			{
				main_skip_cinematic_private();
			}

			if (main_globals.reset_map)
			{
				main_reset_map_private();
			}

			if (main_globals.save_core)
			{
				main_save_core_private();
			}

			if (main_globals.load_core)
			{
				main_load_core_private();
			}

			if (main_globals.want_to_be_at_main_menu)
			{
				main_menu_load();
			}

			if (main_globals.load_last_solo_level)
			{
				main_load_last_solo_map();
			}

			if (main_globals.run_xdemos)
			{
				main_run_demos_private();
			}

			if (main_globals.skip)
			{
				main_skip_private();
			}

			if (main_globals.queue_map)
			{
				main_queue_map_private();
			}
		}
		else if (main_globals.reset_map)
		{
			main_reset_map_private();
		}

		profile_frame_start();
		profile_scope_enter(main_input_section)
		input_frame_begin();
		input_update();
		input_abstraction_update();
		profile_scope_exit(main_input_section)
		profile_scope_enter(main_platform_events_section)
		shell_idle();
		event_manager_update();
		telnet_console_process();
		profile_scope_exit(main_platform_events_section)

		if (!shell_application_is_paused())
		{
			render_frame = TRUE;

			/* automated system link tests (port/linux/game/network_test.c) */
			network_test_update(main_globals.main_menu_scenario_loaded, main_globals.seconds_elapsed);
			/* port: a dedicated server's hosting, rotation and commands
			(port/linux/game/server_director.c) */
			server_update(main_globals.main_menu_scenario_loaded, main_globals.seconds_elapsed);
			/* port: voice chat, in the lobby and in game (port/linux/game/network_voice.c) */
			network_voice_update();
			/* port: apply and replicate the host's mouse aim-assist policy. */
			network_aim_assist_update();
			/* port: the host's shield effects, for every machine (port/linux/game/network_shield_effects.c) */
			network_shield_effects_update();
			/* port: the map's riding choice, for every machine (port/linux/game/network_riding_map.c) */
			network_riding_map_update();
			/* port: map sharing, in the lobby and in game (port/linux/game/map_share.c) */
			map_share_update();
			/* port: network games' text chat (port/linux/game/network_chat.c) */
			network_chat_update();
			profile_scope_enter(main_network_start_frame_section)
			connection = main_globals.connection;
			if (connection==_game_connection_network_client)
			{
				if (!network_game_client_start_frame())
				{
					display_error_when_main_menu_loaded(6);
					error(_error_silent, "the game host went down");
					network_game_abort();
				}
			}
			else if (connection==_game_connection_network_server)
			{
				if (!network_game_client_start_frame())
				{
					display_error_when_main_menu_loaded(1);
					error(_error_silent, "the game host went down");
					network_game_abort();
				}
				else if (!network_game_server_start_frame())
				{
					display_error_when_main_menu_loaded(1);
					error(_error_silent, "the game host went down");
					network_game_abort();
				}
			}
			else if (connection==_game_connection_film_playback)
			{
				profile_scope_exit(main_network_start_frame_section)
				break;
			}
			profile_scope_exit(main_network_start_frame_section)

			/* port: a dedicated server's frames paced by its clock, with no
			display to wait for (port/linux/src/server_mode.c) */
			if (halo_server_mode())
			{
				server_mode_pace_frame();
				profile_scope(main_update_time_section, main_update_time_unthrottled();)
			}
			else
			{
				profile_scope(main_update_time_section, main_update_time();)
			}
			profile_scope_enter(main_ui_update_section)
			process_ui_widgets();
			bink_playback_update();
			profile_scope_exit(main_ui_update_section)

			/* port: not the Xbox debug keyboard's End and Escape, which stop
			the movie and restart the map: this keyboard reaches the game only
			through the console and the menus' text boxes, whose End and Escape
			they are (port/linux/src/xinput_sdl.c) */
			if (editor_should_exit())
			{
				main_movie_stop();

				if (!game_engine_running())
				{
					main_reset_map();
				}
			}

			if (game_in_progress())
			{
				terminal_update();

				if (!console_update() || main_globals.connection!=_game_connection_local)
				{
					/* port: not while a chat line is typed (its K plays an animation) */
					if (!network_chat_typing())
						debug_keys_update();
					cheats_update();
					player_control_update((real)main_globals.halt_time_scale*main_globals.seconds_elapsed);

					connection = main_globals.connection;
					profile_scope_enter(main_network_end_frame_section)
					if (connection>_game_connection_local && connection<=_game_connection_network_server && !network_game_client_end_frame())
					{
						display_error_when_main_menu_loaded(1);
						network_game_abort();
					}
					profile_scope_exit(main_network_end_frame_section)

					profile_scope(main_game_time_update_section,
						game_time_update((real)main_globals.halt_time_scale*main_globals.seconds_elapsed);)

					render_frame = main_globals.main_menu_scenario_loaded ||
						(main_globals.halt_time_scale &&
							(game_time_get_paused() || game_time_get_elapsed()>0 || game_time_get_speed()<1.0f));
					/* frames between ticks too (render_interpolation.c) */
					if (halo_interpolation_enabled())
						render_frame = main_globals.main_menu_scenario_loaded || main_globals.halt_time_scale;
					render_frame &= !game_engine_running() || game_time_get()>=3;

					profile_scope_enter(main_non_deterministic_update_section)
					collision_log_continue_period(1);
					director_update((real)main_globals.halt_time_scale*main_globals.seconds_elapsed);
					observer_update((real)main_globals.halt_time_scale*main_globals.seconds_elapsed);
					collision_log_end_period();
					game_engine_update_non_deterministic((real)main_globals.halt_time_scale*main_globals.seconds_elapsed);
					profile_scope_exit(main_non_deterministic_update_section)
				}

				if (main_globals.saving_map)
				{
					main_save_map_private();
				}

				if (render_frame && !debug_no_drawing)
				{
					profile_render_start();
					render_interpolation_frame_begin();
					main_game_render((double)main_globals.seconds_elapsed);
					render_interpolation_frame_end();
					profile_render_end();
				}
			}
			/* port: nothing drawn on a dedicated server, nor waited for */
			else if (!halo_server_mode())
			{
				profile_render_start();
				main_pregame_render();
				profile_render_end();
			}

			if (!halo_server_mode())
				profile_scope(main_throttle_section, main_rasterizer_throttle();)

			if (render_frame && !debug_no_drawing)
			{
				profile_scope(main_present_section, main_present_frame();)
			}
		}

		input_frame_end();
		profile_frame_end();
		main_frame_rate_debug();

		if (main_globals.restart_time)
		{
			main_globals.restart_time = FALSE;
			main_reset_time();
			main_globals.halt_time_scale = TRUE;
		}
#ifdef HALO_WEB
		return TRUE;
#endif
	}
#ifdef HALO_WEB
	while (FALSE);
	/* (a film played to its end) */
	return FALSE;
#else

	error(_error_silent, "end of saved film");
	main_exit();

	return;
#endif
}

/* ---------- private code */

/* port: what this build is, which a client tells the hosts it joins
(network_distributed.c's _distributed_message_client_build): its number (0
for a build CI did not number), configuration, the GitHub repository it is
made from (owner/name; empty if not known) and the label it shows */
#ifndef HALO_BUILD_NUMBER
#define HALO_BUILD_NUMBER 0
#endif
#ifndef HALO_BUILD_FLAVOR
#define HALO_BUILD_FLAVOR "local"
#endif
#ifndef HALO_BUILD_REPOSITORY
#define HALO_BUILD_REPOSITORY ""
#endif
void main_build_identity(
	long *number,
	char *flavor,
	size_t flavor_size,
	char *repository,
	size_t repository_size,
	char *version,
	size_t version_size)
{
	*number = HALO_BUILD_NUMBER;
	_snprintf(flavor, flavor_size - 1, "%s", HALO_BUILD_FLAVOR);
	flavor[flavor_size - 1] = 0;
	_snprintf(repository, repository_size - 1, "%s", HALO_BUILD_REPOSITORY);
	repository[repository_size - 1] = 0;
#ifdef HALO_NATIVE_BUILD_INFO
	main_native_build_label(version, version_size);
#else
	if (HALO_BUILD_NUMBER > 0)
		_snprintf(version, version_size - 1, "OpenCE Web | build %d (%s)", HALO_BUILD_NUMBER, HALO_BUILD_FLAVOR);
	else
		_snprintf(version, version_size - 1, "OpenCE Web | local build (%s)", HALO_BUILD_FLAVOR);
	version[version_size - 1] = 0;
#endif
}
