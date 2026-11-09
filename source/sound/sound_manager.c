/*
SOUND_MANAGER.C

symbols in this file:
001BB280 0070:
	_sound_valid_for_channel (0000)
001BB2F0 0010:
	_current_platform_definition (0000)
001BB300 0010:
	_sound_initialize_for_new_map (0000)
001BB310 0060:
	_sound_dispose (0000)
001BB370 0020:
	_sound_is_active (0000)
001BB390 0040:
	_sound_pause (0000)
001BB3D0 0010:
	_sound_render_time (0000)
001BB3E0 0070:
	_sound_reconnect_to_structure_bsp (0000)
001BB450 0020:
	_sound_try_and_get (0000)
001BB470 0010:
	_sound_enable (0000)
001BB480 0020:
	_sound_scripted_dialog_is_playing (0000)
001BB4A0 0020:
	_sound_manager_set_sound_environment (0000)
001BB4C0 0030:
	_code_001bb4c0 (0000)
001BB4F0 0020:
	_code_001bb4f0 (0000)
001BB510 0060:
	_code_001bb510 (0000)
001BB570 0040:
	_code_001bb570 (0000)
001BB5B0 0040:
	_code_001bb5b0 (0000)
001BB5F0 00a0:
	_code_001bb5f0 (0000)
001BB690 0030:
	_code_001bb690 (0000)
001BB6C0 0050:
	_code_001bb6c0 (0000)
001BB710 0020:
	_code_001bb710 (0000)
001BB730 00f0:
	_code_001bb730 (0000)
001BB820 0200:
	_code_001bb820 (0000)
001BBA20 0080:
	_code_001bba20 (0000)
001BBAA0 00a0:
	_code_001bbaa0 (0000)
001BBB40 00f0:
	_code_001bbb40 (0000)
001BBC30 0080:
	_code_001bbc30 (0000)
001BBCB0 0040:
	_code_001bbcb0 (0000)
001BBCF0 00f0:
	_code_001bbcf0 (0000)
001BBDE0 0020:
	_code_001bbde0 (0000)
001BBE00 0130:
	_code_001bbe00 (0000)
001BBF30 0050:
	_code_001bbf30 (0000)
001BBF80 0060:
	_code_001bbf80 (0000)
001BBFE0 00c0:
	_code_001bbfe0 (0000)
001BC0A0 0160:
	_code_001bc0a0 (0000)
001BC200 01b0:
	_sound_initialize (0000)
001BC3B0 0030:
	_code_001bc3b0 (0000)
001BC3E0 0170:
	_code_001bc3e0 (0000)
001BC550 0180:
	_code_001bc550 (0000)
001BC6D0 00c0:
	_code_001bc6d0 (0000)
001BC790 00d0:
	_code_001bc790 (0000)
001BC860 0110:
	_code_001bc860 (0000)
001BC970 0100:
	_code_001bc970 (0000)
001BCA70 0210:
	_code_001bca70 (0000)
001BCC80 0130:
	_code_001bcc80 (0000)
001BCDB0 00d0:
	_code_001bcdb0 (0000)
001BCE80 00c0:
	_code_001bce80 (0000)
001BCF40 0080:
	_sound_stop_impulse (0000)
001BCFC0 0070:
	_sound_stop_impulse_by_source_and_definition (0000)
001BD030 0060:
	_sound_stop_all (0000)
001BD090 00f0:
	_code_001bd090 (0000)
001BD180 0220:
	_code_001bd180 (0000)
001BD3A0 01a0:
	_code_001bd3a0 (0000)
001BD540 01e0:
	_code_001bd540 (0000)
001BD720 0550:
	_code_001bd720 (0000)
001BDC70 03d0:
	_sound_new_impulse (0000)
001BE040 0470:
	_sound_refresh_looping (0000)
001BE4B0 01f0:
	_code_001be4b0 (0000)
001BE6A0 01f0:
	_code_001be6a0 (0000)
001BE890 0360:
	_code_001be890 (0000)
001BEBF0 01f0:
	_code_001bebf0 (0000)
001BEDE0 0070:
	_sound_idle (0000)
001BEE50 02e0:
	_code_001bee50 (0000)
001BF130 00f0:
	_sound_dispose_from_old_map (0000)
001BF220 00f0:
	_sound_render (0000)
002AF820 000d:
	??_C@_0N@ICDMEFNC@sound_render?$AA@ (0000)
002AF830 001c:
	_rdata_002af830 (0000)
002AF84C 0036:
	??_C@_0DG@GEFEIAPP@index?$DO?$DN0?5?$CG?$CG?5index?$DMsound_manager_@ (0000)
002AF884 0025:
	??_C@_0CF@OMMIMEGJ@c?3?2halo?2SOURCE?2sound?2sound_manag@ (0000)
002AF8AC 0032:
	??_C@_0DC@IDLFGNMI@index?$DO?$DN0?5?$CG?$CG?5index?$DMMAXIMUM_NUMBER@ (0000)
002AF8E0 0034:
	??_C@_0DE@IJMAACKJ@sound_get?$CIsound_index?$CJ?9?$DOplaying_@ (0000)
002AF918 006a:
	??_C@_0GK@GIGJNEHE@sound?9?$DOplaying_channel_index?$DN?$DNNO@ (0000)
002AF988 0042:
	??_C@_0EC@NBNLKGJ@summary?9?$DOlike_source_count?$DMsumma@ (0000)
002AF9CC 003f:
	??_C@_0DP@JPHEDLPF@summary?9?$DOlike_definition_count?$DMs@ (0000)
002AFA10 0053:
	??_C@_0FD@BLJIIJII@summary?9?$DOmaximum_instance_count?$DM@ (0000)
002AFA68 004f:
	??_C@_0EP@DKNAFCF@summary?9?$DOmaximum_source_instance@ (0000)
002AFAB8 0016:
	??_C@_0BG@LLOIFBDE@properties?9?$DOpitch?$DO0?4f?$AA@ (0000)
002AFAD0 001d:
	??_C@_0BN@IEJOFEHG@?$CBchannel?9?$DOqueued_permutation?$AA@ (0000)
002AFAF0 000a:
	??_C@_09LAFJOCBG@?$CFs?$HMn?$CFf?5?$CFf?$AA@ (0000)
002AFAFC 003c:
	??_C@_0DM@PJIONELA@sound_manager_globals?4channel_co@ (0000)
002AFB38 000f:
	??_C@_0P@FIIBGGGF@looping?5sounds?$AA@ (0000)
002AFB48 0007:
	??_C@_06OHLGPBPO@sounds?$AA@ (0000)
002AFB50 0038:
	??_C@_0DI@CEKFNNAG@fade_in_sound_index?$CB?$DNNONE?5?$HM?$HM?5fad@ (0000)
002AFB88 000d:
	??_C@_0N@CFIAGGKK@seconds?$DO?$DN0?4f?$AA@ (0000)
002AFB98 0042:
	??_C@_0EC@MNDJFMMK@mode?$DN?$DN_sound_fade_mode_linear?5?$HM?$HM@ (0000)
002AFBDC 0024:
	??_C@_0CE@FPLGCI@listener_get?$CIlistener_index?$CJ?9?$DOva@ (0000)
002AFC00 002d:
	??_C@_0CN@BJAODDBE@challenger_sound_index?$CB?$DNchampion@ (0000)
002AFC30 0026:
	??_C@_0CG@JFKDGJLJ@sound_cache_sound_loaded?$CIpermuta@ (0000)
002AFC58 002d:
	??_C@_0CN@GMMAODPG@sound_get?$CIsound_index?$CJ?9?$DOtype?$DN?$DN_s@ (0000)
002AFC88 0004:
	__real@3be56042 (0000)
002AFC90 0048:
	??_C@_0EI@NEANMMGP@TEST_FLAG?$CIdefinition?9?$DOflags?0?5_so@ (0000)
002AFCD8 001c:
	??_C@_0BM@DILLGPMI@sound?9?$DOtype?$CB?$DN_sound_impulse?$AA@ (0000)
002AFCF8 006a:
	??_C@_0GK@CFIFHDCA@attempt?5to?5play?5a?5sound?5that?5was@ (0000)
002AFD64 0012:
	??_C@_0BC@IFEANPKO@sound?9?$DOtrack_data?$AA@ (0000)
002AFD78 006f:
	??_C@_0GP@PNOIFFBD@source?9?$DOspatialization_mode?$DN?$DN_so@ (0000)
002AFDE8 002d:
	??_C@_0CN@FLAINKPN@track_data_size?$DM?$DNMAXIMUM_SOUND_C@ (0000)
002AFE18 0029:
	??_C@_0CJ@HAJEDJHJ@loop?9?$DOdefinition_index?$DN?$DNdefiniti@ (0000)
002AFE48 0044:
	??_C@_0EE@CBCNGIMC@channel_get?$CIsound?9?$DOplaying_chann@ (0000)
002AFE8C 0010:
	??_C@_0BA@NKIBLFLK@listener?9?$DOvalid?$AA@ (0000)
002AFEA0 00ad:
	??_C@_0KN@CMACAPDF@TEST_FLAG?$CIsound?9?$DOflags?0?5_sound_d@ (0000)
002AFF50 002c:
	??_C@_0CM@MDCHMPMH@sound?9?$DOplaying_channel_index?$DN?$DNch@ (0000)
002AFF80 0098:
	??_C@_0JI@MGGHJBNL@sound_valid_for_channel?$CIdefiniti@ (0000)
002B0018 0004:
	__real@43960000 (0000)
00317478 060c:
	_sound_gain_under_dialog (0000)
	_platform_definitions (0004)
	_sound_fade_exponent (0608)
004D2D60 0178:
	_bss_004d2d60 (0000)
*/

/* ---------- headers */

#include "cseries.h"
#include "errors.h"
#include "data.h"
#include "cseries/profile.h"
#include "cseries/cseries_windows.h"
#include "cache/sound_cache.h"
#include "math/real_math.h"
#include "sound_manager.h"
#include "sound_classes.h"
#include "sound_definitions.h"
#include "sound_dsound.h"
#include "sound_environment_definitions.h"
#include "game_sound.h"
#include "sound_preferences.h"
#include "camera/director.h" /* port: director_get_followed_unit */
#include "camera/observer.h"
#include "effects/player_effects.h"
#include "game/game.h"
#include "game/game_globals.h"
#include "game/players.h"
#include "objects/objects.h" /* port: object_get_point_velocity */
#include "render/render_debug.h"
#include "scenario/scenario.h"
#include "tag_files/tag_files.h"

#include <math.h>
#include <stdio.h>

/* port: a stereo channel's sound in the world panned towards it, muffled
and reverberated as a 3D channel's is (sound_dsound_xbox.c; update_channels) */
void dsound_port_set_channel_stereo_position(short virtual_channel_index, boolean positioned, real pan,
	real distance, real minimum_distance, real distance_fade, real occlusion, real obstruction,
	boolean attenuate_direct_path, real approach);

/* ---------- constants */

enum
{
	MAXIMUM_SOUND_CHANNELS = 256,
	MAXIMUM_SOUND_CALLBACK_DATA = 0x30,
};

enum sound_promotion_result
{
	_sound_promotion_dont,
	_sound_promotion_do,
	_sound_promotion_dont_play,
};

enum sound_spatialization_mode
{
	_sound_spatialization_mode_none,
	_sound_spatialization_mode_absolute,
	_sound_spatialization_mode_relative,
	NUMBER_OF_SOUND_SPATIALIZATION_MODES,
};

enum sound_type
{
	_sound_impulse = 0,
	_sound_start_track = 1,
	_sound_loop_track = 2,
	_sound_stopping_track = 3,
	_sound_stop_track = 4,
	NUMBER_OF_SOUND_TYPES = 5,
};

enum sound_fade_mode
{
	_sound_fade_mode_linear,
	_sound_fade_mode_crossfade,
};

enum sound_cache_miss_mode
{
	_sound_cache_miss_mode_discard,
	_sound_cache_miss_mode_postpone,
};

enum sound_encoding
{
	_sound_encoding_mono = 0,
	_sound_encoding_stereo = 1,
	NUMBER_OF_SOUND_ENCODINGS = 2,
};

enum sound_compression
{
	_sound_compression_none = 0,
	_sound_compression_xbox_adpcm = 1,
	_sound_compression_ima_adpcm = 2,
	_sound_compression_ogg = 3,
	NUMBER_OF_SOUND_COMPRESSION_TYPES = 4,
};

enum sound_datum_flags
{
	_sound_delayed_bit,
	_sound_cached_bit,
	_sound_inaudible_bit,
	_sound_waiting_for_cache_bit,
	NUMBER_OF_SOUND_FLAGS,
};

enum sound_channel_flags
{
	_sound_channel_3d_bit,
	_sound_channel_stereo_bit,
	_sound_channel_44k_bit,
	_sound_channel_compressed_bit,
};

enum sound_channel_state
{
	_sound_channel_idle,
	_sound_channel_playing,
	_sound_channel_queued,
	NUMBER_OF_SOUND_CHANNEL_STATES,
};

enum looping_sound_track_flags
{
	_fade_in_at_start_bit,
	_fade_out_at_stop_bit,
	_fade_in_alternate_bit,
	NUMBER_OF_LOOPING_SOUND_TRACK_FLAGS,
};

/* ---------- macros */

#define sound_get(index) ((struct sound_datum *)datum_get(sound_data, (index)))
#define looping_sound_get(index) \
	((struct looping_sound_datum *)datum_get(looping_sound_data, (index)))
#define sound_permutation_get(definition_index, pitch_range_index, permutation_index) \
	TAG_BLOCK_GET_ELEMENT( \
		&TAG_BLOCK_GET_ELEMENT( \
			&sound_definition_get(definition_index)->pitch_ranges, \
			pitch_range_index, \
			struct sound_pitch_range)->permutations, \
		permutation_index, \
		struct sound_permutation)
#define sound_cache_sound_loaded(sound) \
	_sound_cache_sound_request((sound), FALSE, FALSE, FALSE)

/* ---------- structures */

struct platform_sound_listener_properties;
struct sound_location;
struct sound_permutation;
struct sound_preferences;

struct sound_source
{
	short spatialization_mode;
	short pad_2;
	real scale;
	real gain;
	struct sound_location location;
	real obstruction;
	real occlusion;
};

struct loop_impulse_sound_tracking_data
{
	real_vector3d position_offset;
};

struct sound_listener
{
	boolean valid;
	boolean underwater;
	byte pad_2[2];
	real_matrix4x3 matrix;
	real_vector3d velocity;
};

struct sound_channel_datum
{
	long sound_index;
	short type_flags;
	short pad_6;
	real estimated_tick_time;
	real pitch;
	struct sound_permutation *playing_permutation;
	struct sound_permutation *queued_permutation;
};

struct sound_channel_summary
{
	short like_definition_count;
	short like_definition_channels[MAXIMUM_SOUND_INSTANCES_PER_DEFINITION];
	short maximum_instance_count;
	short like_source_count;
	short like_source_channels[MAXIMUM_SOUND_INSTANCES_PER_OBJECT_PER_DEFINITION];
	short maximum_source_instance_count;
};

struct platform_sound_channel_properties
{
	real minimum_distance;
	real maximum_distance;
	real pitch;
	real gain;
	real cone_inside_angle;
	real cone_outside_angle;
	real cone_outside_gain;
	real reverb_attenuation;
};

struct platform_sound_listener_properties
{
	real_point3d position;
	real_vector3d forward;
	real_vector3d up;
	real_vector3d velocity;
	struct sound_environment_definition const *environment;
};

struct sound_datum
{
	short identifier;
	short type;
	word flags;
	short listener_index;
	long definition_index;
	long source_identifier;
	boolean (*track_proc)(
		long source_identifier,
		void const *track_data,
		struct sound_source *source);
	struct sound_source source;
	byte track_data[0x30];
	long start_time;
	real pitch;
	short playing_channel_index;
	short pitch_range_index;
	short permutation_index;
	short fade_mode;
	short loop_track_index;
	short pad_96;
	long next_definition_index;
	real fade_interpolation_start;
	real fade_interpolation_end;
	long fade_start_time;
	long fade_stop_time;
};

struct looping_sound_datum
{
	short identifier;
	short pad_2;
	long definition_index;
	long loop_identifier;
	struct sound_source source;
	boolean flip_flop;
	boolean alternate;
	boolean ordered_sounds_finished;
	byte pad_4F;
	short component_sound_count;
	short state;
	long detail_play_times[MAXIMUM_DETAIL_SOUNDS_PER_LOOPING_SOUND];
	struct
	{
		long primary_sound_index;
	} tracks[4];
};

struct sound_platform_definition
{
	short platform_code;
	byte reserved0[2];
	boolean (*initialize)(
		struct sound_preferences *preferences);
	void (*dispose)(
		void);
	void (*set_listener_properties)(
		struct platform_sound_listener_properties const *properties);
	void (*begin_scene)(
		void);
	void (*end_scene)(
		void);
	void (*queue_sound_to_channel)(
		short channel_index,
		struct sound_permutation *permutation);
	void (*channel_update)(
		short channel_index);
	void (*stop_channel)(
		short channel_index);
	short (*get_channel_state)(
		short channel_index);
	void (*set_pause)(
		boolean paused);
	void (*flush)(
		void);
	void (*set_channel_location)(
		short channel_index,
		boolean spatialized,
		struct sound_location const *location,
		real obstruction,
		real occlusion,
		boolean attenuate_direct_path);
	void (*set_channel_properties)(
		short channel_index,
		struct platform_sound_channel_properties const *properties,
		boolean gain_only);
	real direct_path_gain;
};

struct sound_manager_globals
{
	boolean initialized;
	boolean enabled;
	boolean paused;
	boolean idling;
	long game_time_when_no_scripted_dialog_will_be_playing;
	struct sound_platform_definition *platform_definition;
	long render_time;
	real ticks_elapsed;
	boolean flip_flop;
	byte pad_15[3];
	struct sound_listener listeners[MAXIMUM_NUMBER_OF_LOCAL_PLAYERS];
	struct sound_environment_definition sound_environment;
	real nondialog_gain;
	short channel_count;
	short pad_176;
};

typedef char verify_sound_class_definition_size[
	sizeof(struct sound_class_definition) == 0x2C ? 1 : -1];
typedef char verify_sound_source_size[
	sizeof(struct sound_source) == 0x40 ? 1 : -1];
typedef char verify_sound_listener_size[
	sizeof(struct sound_listener) == 0x44 ? 1 : -1];
typedef char verify_sound_channel_datum_size[
	sizeof(struct sound_channel_datum) == 0x18 ? 1 : -1];
typedef char verify_sound_channel_summary_size[
	sizeof(struct sound_channel_summary) == 0x48 ? 1 : -1];
typedef char verify_platform_sound_channel_properties_size[
	sizeof(struct platform_sound_channel_properties) == 0x20 ? 1 : -1];
typedef char verify_sound_datum_size[
	sizeof(struct sound_datum) == 0xAC ? 1 : -1];
typedef char verify_looping_sound_datum_size[
	sizeof(struct looping_sound_datum) == 0xE4 ? 1 : -1];

typedef char verify_sound_manager_globals_size[
	sizeof(struct sound_manager_globals) == 0x178 ? 1 : -1];
typedef char verify_sound_platform_definition_size[
	sizeof(struct sound_platform_definition) == 0x3C ? 1 : -1];
typedef char verify_sound_platform_dispose_offset[
	offsetof(struct sound_platform_definition, dispose) == 0x8 ? 1 : -1];
typedef char verify_sound_platform_pause_offset[
	offsetof(struct sound_platform_definition, set_pause) == 0x28 ? 1 : -1];
typedef char verify_sound_manager_paused_offset[
	offsetof(struct sound_manager_globals, paused) == 0x2 ? 1 : -1];
typedef char verify_sound_manager_dialog_time_offset[
	offsetof(
		struct sound_manager_globals,
		game_time_when_no_scripted_dialog_will_be_playing) == 0x4 ? 1 : -1];
typedef char verify_sound_manager_listeners_offset[
	offsetof(struct sound_manager_globals, listeners) == 0x18 ? 1 : -1];
typedef char verify_sound_manager_environment_offset[
	offsetof(struct sound_manager_globals, sound_environment) == 0x128 ? 1 : -1];
typedef char verify_sound_manager_channel_count_offset[
	offsetof(struct sound_manager_globals, channel_count) == 0x174 ? 1 : -1];

/* ---------- prototypes */

static void sound_update_time(
	void);
static void detail_sound_random_offset(
	struct looping_sound_detail const *detail,
	real_vector3d *offset);
static real sound_scale_value(
	real base,
	real lower_bound_modifier,
	real upper_bound_modifier,
	real scale);
static boolean sound_definition_is_playable(
	long definition_index);
static struct sound_channel_datum *channel_get(
	short channel_index);
static struct sound_listener *listener_get(
	short listener_index);
static short sound_definition_promote(
	long definition_index);
static real sound_manager_master_gain(
	short class_index);
static void sound_delete(
	long sound_index);
static long sound_travel_milliseconds(
	real distance);
static boolean refresh_sound(
	long sound_index);
static void sound_channel_summary_build(
	struct sound_channel_summary *summary,
	long sound_index);
static void channel_queue_sound(
	short channel_index,
	struct sound_permutation *permutation);
static void channel_set_properties_hardware(
	short channel_index,
	struct platform_sound_channel_properties *properties,
	boolean gain_only);
static short channel_get_state(
	short channel_index);
static boolean track_loop_track_sound(
	long looping_sound_index,
	void const *track_data,
	struct sound_source *source);
static boolean track_loop_impulse_sound(
	long looping_sound_index,
	void const *track_data,
	struct sound_source *source);
static void sound_set_definition_begin(
	long sound_index,
	long definition_index);
static long looping_sound_find(
	long identifier);
static long looping_sound_new(
	long definition_index,
	long identifier,
	struct sound_source const *source);
/* port: report whether instance limiting leaves this voice alive. */
static boolean sound_set_definition_end(
	long sound_index);
static long update_potentially_audible_looping_sound(
	long definition_index,
	long looping_sound_index,
	short track_index,
	short type);
static real limit_pitch(
	real desired_pitch,
	real old_pitch,
	real maximum_bend);
static short sound_find_like_channel(
	long sound_index,
	short const *channel_indices,
	short channel_count);
static void update_channel_for_impulse_sound(
	short channel_index,
	real fade);
static void update_channel_for_looping_sound(
	short channel_index,
	real fade);
static void update_channels(
	void);
static void render_debug_sound(
	long sound_index);
static void render_debug_looping_sound(
	long definition_index,
	struct sound_source const *source);
static real sound_scale_random_value(
	real base_lower_bound,
	real base_upper_bound,
	real lower_bound_modifier,
	real upper_bound_modifier,
	real scale);
static real source_distance(
	short listener_index,
	struct sound_source *source);
static real source_distance_squared(
	short listener_index,
	struct sound_source *source);
static boolean sound_preempts_sound(
	long challenger_sound_index,
	long champion_sound_index,
	real challenger_distance_squared);
static short sound_find_best_channel(
	long sound_index);
static short source_audible(
	struct sound_source *source,
	real maximum_distance);
static void refresh_sounds(
	void);
static void refresh_listener(
	void);
static void process_looping_sounds(
	void);
static real sound_calculate_fade(
	long sound_index);
static void sound_start_fade(
	short mode,
	real seconds,
	long fade_in_sound_index,
	long fade_out_sound_index);
static void channel_stop(
	short channel_index);
static void sound_stop(
	long sound_index);
static short sound_find_channel(
	long sound_index);
static void prioritize_sounds(
	void);

/* ---------- globals */

struct data_array *looping_sound_data;
struct data_array *sound_data;
struct sound_channel_datum sound_channels[MAXIMUM_SOUND_CHANNELS];
boolean loud_dialog_hack;
boolean debug_looping_sound;

static real const sound_pitch_range_fade_time = 0.5f;
static real const sound_inaudible_fade_out_time = 2.f;
static real const sound_inaudible_fade_back_in_time = 0.5f;
static real const sound_player_fade_out_time = 0.3f;
static real const oo_speed_of_sound = 8.9647064f;
static long const speed_of_sound_threshold = 250;
static real const sound_priority_epsilon = 0.1f;

real sound_gain_under_dialog = 0.7f;
struct sound_platform_definition *platform_definitions[2] =
{
	&platform_sound_dsound,
	NULL,
};
static struct profile_section sound_render_section =
	{"sound_render", NONE, TRUE};
real sound_fade_exponent = 2.5f;
static struct sound_manager_globals sound_manager_globals = { 0 };
boolean debug_sound;
boolean debug_sound_channels;

/* ---------- public code */

boolean sound_valid_for_channel(
	short compression,
	short encoding,
	short sample_rate,
	short spatialization_mode,
	short channel_type_flags)
{
	boolean valid = TRUE;

	if (!TEST_FLAG(
			channel_type_flags,
			_sound_channel_compressed_bit) != (compression == 0))
	{
		valid = FALSE;
	}

	if (!TEST_FLAG(
			channel_type_flags,
			_sound_channel_stereo_bit) != (encoding == 0))
	{
		valid = FALSE;
	}

	if (TEST_FLAG(
			channel_type_flags,
			_sound_channel_44k_bit) != sample_rate)
	{
		valid = FALSE;
	}

	if (!TEST_FLAG(channel_type_flags, _sound_channel_stereo_bit) &&
		!TEST_FLAG(channel_type_flags, _sound_channel_3d_bit) !=
			(spatialization_mode == _sound_spatialization_mode_none))
	{
		valid = FALSE;
	}

	return valid;
}

struct sound_platform_definition *current_platform_definition(
	void)
{
	return sound_manager_globals.platform_definition;
}

void sound_dispose(
	void)
{
	if (sound_manager_globals.initialized)
	{
		sound_manager_globals.platform_definition->dispose();
		data_make_invalid(sound_data);
		data_make_invalid(looping_sound_data);
		sound_manager_globals.initialized = FALSE;
	}

	if (sound_data)
	{
		data_dispose(sound_data);
	}

	if (looping_sound_data)
	{
		data_dispose(looping_sound_data);
	}

	sound_cache_delete();

	return;
}

void sound_enable(
	boolean enabled)
{
	sound_manager_globals.enabled = enabled;

	return;
}

boolean sound_is_active(
	void)
{
	return sound_manager_globals.initialized && sound_manager_globals.enabled;
}

void sound_pause(
	boolean paused)
{
	if (paused != sound_manager_globals.paused)
	{
		sound_manager_globals.paused = paused;
		sound_manager_globals.platform_definition->set_pause(paused);

		if (!paused)
		{
			sound_manager_globals.render_time = system_milliseconds();
		}
	}

	return;
}

boolean sound_try_and_get(
	long sound_index)
{
	return datum_try_and_get(sound_data, sound_index) != NULL;
}

void sound_manager_set_sound_environment(
	struct sound_environment_definition const *environment)
{
	sound_manager_globals.sound_environment = *environment;

	return;
}

long sound_render_time(
	void)
{
	return sound_manager_globals.render_time;
}

boolean sound_scripted_dialog_is_playing(
	void)
{
	long game_time = game_time_get();

	return game_time <
		sound_manager_globals.game_time_when_no_scripted_dialog_will_be_playing;
}

void sound_initialize_for_new_map(
	void)
{
	return;
}

void sound_reconnect_to_structure_bsp(
	void)
{
	if (sound_manager_globals.initialized && sound_manager_globals.enabled)
	{
		long sound_index = data_next_index(sound_data, NONE);

		while (sound_index != NONE)
		{
			struct sound_datum *sound = sound_get(sound_index);

			if (sound->source.spatialization_mode ==
				_sound_spatialization_mode_absolute)
			{
				scenario_location_from_point(
					&sound->source.location.game_location,
					&sound->source.location.position);
			}

			sound_index = data_next_index(sound_data, sound_index);
		}
	}

	return;
}

void sound_initialize(
	void)
{
	struct sound_preferences *preferences;
	short platform_code;
	struct sound_platform_definition *platform_definition;
	short channel_index;
	short channel_type;

	sound_manager_globals.initialized = FALSE;
	sound_manager_globals.enabled = TRUE;
	read_sound_preferences(&preferences);
	sound_cache_new();
	sound_manager_globals.sound_environment = default_sound_environment;
	sound_manager_globals.nondialog_gain = 1.f;

	platform_code = preferences->platform;
	if (platform_code < 0 || platform_code >= 2)
	{
		return;
	}

	platform_definition = platform_definitions[platform_code];
	if (!platform_definition || platform_definition->platform_code != platform_code)
	{
		return;
	}

	sound_manager_globals.platform_definition = platform_definition;
	sound_data = data_new("sounds", 0x200, sizeof(struct sound_datum));
	if (!sound_data)
	{
		return;
	}

	looping_sound_data = data_new("looping sounds", 0x80, 0xE4);
	if (!looping_sound_data)
	{
		return;
	}

	if (!sound_manager_globals.platform_definition->initialize(preferences))
	{
		return;
	}

	channel_index = 0;
	data_make_valid(sound_data);
	data_make_valid(looping_sound_data);

	for (channel_type = 0; channel_type < NUMBEROF(sound_channel_type_flags); channel_type++)
	{
		short index;

		sound_manager_globals.channel_count +=
			preferences->virtual_channel_counts[channel_type];
		match_assert(
			"c:\\halo\\SOURCE\\sound\\sound_manager.c",
			0x168,
			sound_manager_globals.channel_count<=MAXIMUM_SOUND_CHANNELS);

		for (
			index = 0;
			index < preferences->virtual_channel_counts[channel_type];
			index++)
		{
			struct sound_channel_datum *channel = channel_get(channel_index++);

			channel->sound_index = NONE;
			channel->type_flags = sound_channel_type_flags[channel_type];
			channel->playing_permutation = NULL;
			channel->queued_permutation = NULL;
		}
	}

	sound_manager_globals.initialized = TRUE;

	return;
}

void sound_stop_impulse(
	long sound_index)
{
	if (sound_try_and_get(sound_index))
	{
		match_assert(
			"c:\\halo\\SOURCE\\sound\\sound_manager.c",
			0x2C1,
			sound_get(sound_index)->type==_sound_impulse);

		if (sound_get(sound_index)->type == _sound_impulse)
		{
			sound_start_fade(
				_sound_fade_mode_linear,
				0.3f,
				NONE,
				sound_index);
		}
	}

	return;
}

void sound_stop_impulse_by_source_and_definition(
	long source_identifier,
	long definition_index)
{
	long sound_index = data_next_index(sound_data, NONE);

	while (sound_index != NONE)
	{
		struct sound_datum *sound = sound_get(sound_index);

		if (sound->type == _sound_impulse &&
			sound->source_identifier == source_identifier &&
			sound->definition_index == definition_index)
		{
			sound_stop_impulse(sound_index);
			break;
		}

		sound_index = data_next_index(sound_data, sound_index);
	}

	return;
}

/* ---------- private code */

static void sound_update_time(
	void)
{
	long render_time = system_milliseconds();

	sound_manager_globals.ticks_elapsed =
		((real)render_time - sound_manager_globals.render_time) * 0.029999999f;
	sound_manager_globals.render_time = render_time;

	return;
}

static real sound_scale_value(
	real base,
	real lower_bound_modifier,
	real upper_bound_modifier,
	real scale)
{
	return ((upper_bound_modifier - lower_bound_modifier) * scale +
		lower_bound_modifier) * base;
}

static boolean sound_definition_is_playable(
	long definition_index)
{
	struct sound_definition *definition = sound_definition_get(definition_index);

	return definition->pitch_ranges.count &&
		TAG_BLOCK_GET_ELEMENT(
			&definition->pitch_ranges,
			0,
			struct sound_pitch_range)->permutations.count &&
		!sound_class_get(definition->sound_class)->disabled;
}

static struct sound_channel_datum *channel_get(
	short index)
{
	match_assert(
		"c:\\halo\\SOURCE\\sound\\sound_manager.c",
		0x428,
		index>=0 && index<sound_manager_globals.channel_count);

	return &sound_channels[index];
}

static struct sound_listener *listener_get(
	short index)
{
	match_assert(
		"c:\\halo\\SOURCE\\sound\\sound_manager.c",
		0x430,
		index>=0 && index<MAXIMUM_NUMBER_OF_LOCAL_PLAYERS);

	return &sound_manager_globals.listeners[index];
}

static short sound_definition_promote(
	long definition_index)
{
	short result = _sound_promotion_dont;
	struct sound_definition *definition = sound_definition_get(definition_index);

	if (definition->promotion_count)
	{
		definition->promotion_counter += definition->promotion_time - sound_manager_globals.render_time;
		definition->promotion_counter = MAX(0, definition->promotion_counter);
		definition->promotion_time = sound_manager_globals.render_time;
		definition->promotion_counter += definition->longest_permutation_length;

		if (definition->promotion_counter > definition->promotion_count * definition->longest_permutation_length)
		{
			if (definition->promotion_sound.index != NONE)
			{
				definition->promotion_counter = 0;
				result = _sound_promotion_do;
			}
			else
			{
				definition->promotion_counter -= definition->longest_permutation_length;
				result = _sound_promotion_dont_play;
			}
		}
	}

	return result;
}

/* port: config.toml's audio.music_volume and audio.effects_volume, read
again when Settings changes them */
double config_real(const char *name);
unsigned long config_changes(void);

static real sound_manager_port_volume(
	short class_index)
{
	static unsigned long read_at = (unsigned long)-1;
	static real music_volume = 1.f;
	static real effects_volume = 1.f;

	if (read_at != config_changes())
	{
		read_at = config_changes();
		music_volume = PIN((real)config_real("audio.music_volume"), 0.f, 1.f);
		effects_volume = PIN((real)config_real("audio.effects_volume"), 0.f, 1.f);
	}
	return class_index == _sound_class_music ? music_volume : effects_volume;
}

static real sound_manager_master_gain(
	short class_index)
{
	real gain = sound_class_get_gain(class_index) * sound_manager_port_volume(class_index);

	if (class_index != _sound_class_scripted_dialog_to_player &&
		class_index != _sound_class_scripted_dialog_to_other &&
		class_index != _sound_class_scripted_dialog_force_unspatialized)
	{
		gain *= sound_manager_globals.nondialog_gain;
	}

	return gain;
}

static void sound_delete(
	long sound_index)
{
	match_assert(
		"c:\\halo\\SOURCE\\sound\\sound_manager.c",
		0x4CF,
		sound_get(sound_index)->playing_channel_index==NONE);
	datum_delete(sound_data, sound_index);

	return;
}

static long sound_travel_milliseconds(
	real distance)
{
	return (long)(distance * oo_speed_of_sound);
}

static boolean refresh_sound(
	long sound_index)
{
	struct sound_datum *sound = sound_get(sound_index);

	match_assert(
		"c:\\halo\\SOURCE\\sound\\sound_manager.c",
		0x59F,
		sound->playing_channel_index==NONE ||
			channel_get(sound->playing_channel_index)->sound_index==sound_index);

	/* port: a track of a looping sound that is gone stops, even when it
	started this millisecond. The test of the start time below skips the
	tracking, which is what notices the looping sound is gone, and a
	looping sound not refreshed since the last frame is deleted
	(process_looping_sounds): with three frames in one millisecond (no
	frame limit, the null renderer) the track went on to
	update_channel_for_looping_sound, which read the deleted looping
	sound. BUG (original): the Xbox's frames were never that close. */
	if (sound->type != _sound_impulse &&
		!datum_try_and_get(looping_sound_data, sound->source_identifier))
	{
		return FALSE;
	}

	if (TEST_FLAG(sound->flags, _sound_delayed_bit) ||
		!sound->track_proc ||
		sound->start_time >= sound_manager_globals.render_time ||
		sound->track_proc(
			sound->source_identifier,
			sound->track_data,
			&sound->source))
	{
		return TRUE;
	}

	if (sound->type == _sound_impulse &&
		!sound_class_get(
			sound_definition_get(sound->definition_index)->sound_class)->speech)
	{
		sound->track_proc = NULL;
		return TRUE;
	}

	return FALSE;
}

static void sound_channel_summary_build(
	struct sound_channel_summary *summary,
	long sound_index)
{
	struct sound_datum *sound = sound_get(sound_index);
	struct sound_definition *definition =
		sound_definition_get(sound->definition_index);
	short channel_index;

	summary->like_definition_count = 0;
	summary->like_source_count = 0;
	summary->maximum_instance_count =
		sound_class_get(definition->sound_class)->maximum_number_per_definition;
	summary->maximum_source_instance_count =
		sound_class_get(definition->sound_class)->maximum_number_per_object;
	match_assert(
		"c:\\halo\\SOURCE\\sound\\sound_manager.c",
		0x6A4,
		summary->maximum_source_instance_count<=MAXIMUM_SOUND_INSTANCES_PER_DEFINITION);
	match_assert(
		"c:\\halo\\SOURCE\\sound\\sound_manager.c",
		0x6A5,
		summary->maximum_instance_count<=MAXIMUM_SOUND_INSTANCES_PER_OBJECT_PER_DEFINITION);

	for (
		channel_index = 0;
		channel_index < sound_manager_globals.channel_count;
		channel_index++)
	{
		struct sound_channel_datum *channel = channel_get(channel_index);

		if (channel->sound_index != NONE && channel->sound_index != sound_index)
		{
			struct sound_datum *other_sound = sound_get(channel->sound_index);

			if (sound_valid_for_channel(
					definition->compression,
					definition->encoding,
					definition->sample_rate,
					sound->source.spatialization_mode,
					channel->type_flags) &&
				sound->definition_index == other_sound->definition_index)
			{
				match_assert(
					"c:\\halo\\SOURCE\\sound\\sound_manager.c",
					0x6B5,
					summary->like_definition_count<summary->maximum_instance_count);
				summary->like_definition_channels[
					summary->like_definition_count++] = channel_index;

				if (sound->source_identifier != NONE &&
					sound->source_identifier == other_sound->source_identifier)
				{
					match_assert(
						"c:\\halo\\SOURCE\\sound\\sound_manager.c",
						0x6BB,
						summary->like_source_count<summary->maximum_source_instance_count);
					summary->like_source_channels[
						summary->like_source_count++] = channel_index;
				}
			}
		}
	}

	return;
}

static void channel_queue_sound(
	short channel_index,
	struct sound_permutation *permutation)
{
	struct sound_channel_datum *channel = channel_get(channel_index);

	if (channel->queued_permutation)
	{
		sound_cache_sound_finished(channel->queued_permutation);
	}

	sound_manager_globals.platform_definition->queue_sound_to_channel(
		channel_index,
		permutation);

	if (channel->playing_permutation)
	{
		channel->queued_permutation = permutation;
	}
	else
	{
		channel->playing_permutation = permutation;
		channel->estimated_tick_time = 0.f;
	}

	return;
}

static void channel_set_properties_hardware(
	short channel_index,
	struct platform_sound_channel_properties *properties,
	boolean gain_only)
{
	struct sound_channel_datum *channel = channel_get(channel_index);

	if (!gain_only)
	{
		match_assert(
			"c:\\halo\\SOURCE\\sound\\sound_manager.c",
			0x848,
			properties->pitch>0.f);
		channel->pitch = properties->pitch;
	}

	sound_manager_globals.platform_definition->set_channel_properties(
		channel_index,
		properties,
		gain_only);

	return;
}

static boolean track_loop_track_sound(
	long looping_sound_index,
	void const *track_data,
	struct sound_source *source)
{
	struct looping_sound_datum *looping_sound =
		datum_try_and_get(looping_sound_data, looping_sound_index);

	if (looping_sound)
	{
		*source = looping_sound->source;

		return TRUE;
	}

	return FALSE;
}

static boolean track_loop_impulse_sound(
	long looping_sound_index,
	void const *track_data,
	struct sound_source *source)
{
	struct looping_sound_datum *looping_sound =
		datum_try_and_get(looping_sound_data, looping_sound_index);
	boolean result = FALSE;

	if (looping_sound)
	{
		struct loop_impulse_sound_tracking_data const *tracking_data = track_data;

		source->obstruction = looping_sound->source.obstruction;
		source->occlusion = looping_sound->source.occlusion;

		if (looping_sound->source.spatialization_mode !=
			_sound_spatialization_mode_none)
		{
			source->location.translational_velocity =
				looping_sound->source.location.translational_velocity;
			source->location.forward = looping_sound->source.location.forward;
			source->location.game_location =
				looping_sound->source.location.game_location;
		}
		else
		{
			source->location.forward = *global_forward3d;
			source->location.translational_velocity = *global_zero_vector3d;
		}

		*(real_vector3d *)&source->location.position =
			tracking_data->position_offset;

		if (source->spatialization_mode == _sound_spatialization_mode_absolute)
		{
			source->location.position.x +=
				looping_sound->source.location.position.x;
			source->location.position.y +=
				looping_sound->source.location.position.y;
			source->location.position.z +=
				looping_sound->source.location.position.z;
		}

		result = TRUE;
	}

	return result;
}

static void sound_set_definition_begin(
	long sound_index,
	long definition_index)
{
	struct sound_datum *sound = datum_get(sound_data, sound_index);

	if (sound->definition_index != definition_index)
	{
		sound->next_definition_index = definition_index;
	}

	return;
}

static long looping_sound_find(
	long identifier)
{
	long looping_sound_index = data_next_index(looping_sound_data, NONE);

	if (looping_sound_index == NONE)
	{
		return NONE;
	}

	while (looping_sound_index != NONE)
	{
		if (looping_sound_get(looping_sound_index)->loop_identifier == identifier)
		{
			return looping_sound_index;
		}

		looping_sound_index = data_next_index(
			looping_sound_data,
			looping_sound_index);
	}

	return NONE;
}

static long looping_sound_new(
	long definition_index,
	long identifier,
	struct sound_source const *source)
{
	long looping_sound_index = NONE;

	if (sound_manager_globals.initialized && sound_manager_globals.enabled)
	{
		looping_sound_index = datum_new(looping_sound_data);
		if (looping_sound_index != NONE)
		{
			struct looping_sound_datum *looping_sound =
				looping_sound_get(looping_sound_index);
			struct looping_sound_definition *definition =
				looping_sound_definition_get(definition_index);
			short detail_index;

			looping_sound->definition_index = definition_index;
			looping_sound->loop_identifier = identifier;
			looping_sound->component_sound_count = 0;
			looping_sound->ordered_sounds_finished = FALSE;

			/* port: no more tracks or details than the looping sound holds. A
			tag with more is cut to that, and said once. */
			if (definition->tracks.count > (long)NUMBEROF(looping_sound->tracks) ||
				definition->details.count > (long)NUMBEROF(looping_sound->detail_play_times))
			{
				error(
					_error_silent,
					"looping sound %s has %d tracks and %d details (only %d and %d are played)",
					tag_get_name(definition_index),
					definition->tracks.count,
					definition->details.count,
					(long)NUMBEROF(looping_sound->tracks),
					(long)NUMBEROF(looping_sound->detail_play_times));
				definition->tracks.count = MIN(
					definition->tracks.count,
					(long)NUMBEROF(looping_sound->tracks));
				definition->details.count = MIN(
					definition->details.count,
					(long)NUMBEROF(looping_sound->detail_play_times));
			}

			for (detail_index = 0;
				detail_index < definition->details.count;
				detail_index++)
			{
				struct looping_sound_detail *detail = TAG_BLOCK_GET_ELEMENT(
					&definition->details,
					detail_index,
					struct looping_sound_detail);

				sound_definition_get(detail->sound.index);
				looping_sound->detail_play_times[detail_index] = (long)(
					sound_scale_random_value(
						detail->period_bounds.lower,
						detail->period_bounds.upper,
						definition->scale_lower_bound.detail_period,
						definition->scale_upper_bound.detail_period,
						source->scale) * 1000.f +
					sound_manager_globals.render_time);
			}
		}
	}

	return looping_sound_index;
}

static boolean sound_set_definition_end(
	long sound_index)
{
	struct sound_datum *sound = sound_get(sound_index);
	struct sound_definition *definition =
		sound_definition_get(sound->next_definition_index);

	SET_FLAG(sound->flags, _sound_waiting_for_cache_bit, TRUE);
	sound->definition_index = sound->next_definition_index;
	sound->next_definition_index = NONE;
	sound->pitch_range_index = sound_definition_find_pitch_range_by_pitch(
		definition,
		sound->pitch,
		sound->pitch_range_index);
	sound->permutation_index = sound_definition_next_permutation(
		definition,
		sound->pitch_range_index,
		NONE);

	if (sound->playing_channel_index != NONE)
	{
		struct sound_channel_summary summary;
		short channel_index = NONE;

		sound_channel_summary_build(&summary, sound_index);
		if (summary.like_source_count >= summary.maximum_source_instance_count)
		{
			channel_index = sound_find_like_channel(
				sound_index,
				summary.like_source_channels,
				summary.like_source_count);
		}
		else if (summary.like_definition_count >= summary.maximum_instance_count)
		{
			channel_index = sound_find_like_channel(
				sound_index,
				summary.like_definition_channels,
				summary.like_definition_count);
		}
		else
		{
			return TRUE;
		}

		if (channel_index != NONE)
		{
			/* port: sound_find_like_channel excludes the current voice. */
			sound_stop(channel_get(channel_index)->sound_index);
			return TRUE;
		}

		/* port: no other voice can be preempted; tell the caller this one
		was retired before it writes to the freed channel. */
		sound_stop(sound_index);
		return FALSE;
	}

	return TRUE;
}

static long update_potentially_audible_looping_sound(
	long definition_index,
	long looping_sound_index,
	short track_index,
	short type)
{
	long sound_index = NONE;
	struct looping_sound_datum *looping_sound =
		looping_sound_get(looping_sound_index);
	real scale = looping_sound->source.scale;

	if (sound_definition_is_playable(definition_index))
	{
		struct sound_definition *definition =
			sound_definition_get(definition_index);
		real maximum_distance =
			sound_definition_get_maximum_distance(definition_index);
		short listener_index = source_audible(
			&looping_sound->source,
			maximum_distance);

		if (listener_index != NONE)
		{
			sound_index = datum_new(sound_data);
			if (sound_index != NONE)
			{
				struct sound_datum *sound = sound_get(sound_index);

				sound->listener_index = listener_index;
				sound->definition_index = definition_index;
				sound->playing_channel_index = NONE;
				sound->flags = 0;
				{
					real pitch_upper_bound =
						definition->random_pitch_bounds.upper;
					real pitch_lower_bound =
						definition->random_pitch_bounds.lower;

					sound->pitch = real_seed_random_range(
						get_global_local_random_seed_address(),
						pitch_lower_bound,
						pitch_upper_bound);
				}
				sound->source_identifier = looping_sound_index;
				sound->source = looping_sound->source;
				sound->type = type;
				sound->start_time = sound_manager_globals.render_time;
				sound->loop_track_index = track_index;
				sound->track_proc = track_loop_track_sound;
				sound->fade_stop_time = 0;
				sound->fade_start_time = 0;
				sound->next_definition_index = NONE;
				sound->pitch_range_index =
					sound_definition_find_pitch_range_by_pitch(
						definition,
						sound_scale_value(
							sound->pitch,
							definition->zero_pitch_modifier,
							definition->one_pitch_modifier,
							scale),
						NONE);
				sound->permutation_index = sound_definition_next_permutation(
					definition,
					sound->pitch_range_index,
					NONE);
				_sound_cache_sound_request(
					sound_permutation_get(
						sound->definition_index,
						sound->pitch_range_index,
						sound->permutation_index),
					FALSE,
					TRUE,
					FALSE);
				looping_sound->component_sound_count++;
			}
		}
	}

	return sound_index;
}

static real limit_pitch(
	real desired_pitch,
	real old_pitch,
	real maximum_bend)
{
	real pitch;

	if (maximum_bend == 0.f || desired_pitch == old_pitch)
	{
		pitch = desired_pitch;
	}
	else if (desired_pitch > old_pitch)
	{
		pitch = MIN(desired_pitch, old_pitch * maximum_bend);
	}
	else
	{
		pitch = MAX(desired_pitch, old_pitch / maximum_bend);
	}

	return pitch;
}

static void render_debug_sound(
	long sound_index)
{
	if (debug_sound)
	{
		struct sound_datum *sound = sound_get(sound_index);
		char string[512];

		sound_definition_get(sound->definition_index);
		render_debug_sphere(
			FALSE,
			&sound->source.location.position,
			sound_definition_get_maximum_distance(sound->definition_index),
			global_real_argb_yellow);
		render_debug_sphere(
			FALSE,
			&sound->source.location.position,
			sound_definition_get_minimum_distance(sound->definition_index),
			global_real_argb_red);
		sprintf(
			string,
			"%s|n%f %f",
			tag_get_name(sound->definition_index),
			sound->source.obstruction,
			sound->source.occlusion);
		render_debug_string_at_point(
			FALSE,
			&sound->source.location.position,
			string,
			global_real_argb_white);
	}

	return;
}

static void render_debug_looping_sound(
	long definition_index,
	struct sound_source const *source)
{
	if (debug_looping_sound &&
		source->spatialization_mode == _sound_spatialization_mode_absolute)
	{
		struct looping_sound_definition *definition =
			looping_sound_definition_get(definition_index);
		real minimum_distance = 0.f;
		real maximum_distance = 0.f;
		short index;

		for (index = 0; index < definition->tracks.count; index++)
		{
			struct looping_sound_track *track = TAG_BLOCK_GET_ELEMENT(
				&definition->tracks,
				0,
				struct looping_sound_track);

			if (track->loop_sound.index != NONE)
			{
				sound_definition_get(track->loop_sound.index);
				minimum_distance = sound_definition_get_minimum_distance(
					track->loop_sound.index);
				maximum_distance = sound_definition_get_maximum_distance(
					track->loop_sound.index);
				break;
			}
		}

		if (minimum_distance == 0.f)
		{
			for (index = 0; index < definition->details.count; index++)
			{
				struct looping_sound_detail *detail = TAG_BLOCK_GET_ELEMENT(
					&definition->details,
					0,
					struct looping_sound_detail);

				if (detail->sound.index != NONE)
				{
					sound_definition_get(detail->sound.index);
					minimum_distance = sound_definition_get_minimum_distance(
						detail->sound.index);
					maximum_distance = sound_definition_get_maximum_distance(
						detail->sound.index);
					break;
				}
			}
		}

		render_debug_string_at_point(
			FALSE,
			&source->location.position,
			tag_get_name(definition_index),
			global_real_argb_white);
		render_debug_sphere(
			FALSE,
			&source->location.position,
			maximum_distance,
			global_real_argb_cyan);
		render_debug_sphere(
			FALSE,
			&source->location.position,
			minimum_distance,
			global_real_argb_blue);
	}

	return;
}

static real sound_scale_random_value(
	real base_lower_bound,
	real base_upper_bound,
	real lower_bound_modifier,
	real upper_bound_modifier,
	real scale)
{
	real base = real_seed_random_range(
		get_global_local_random_seed_address(),
		base_lower_bound,
		base_upper_bound);

	return sound_scale_value(
		base,
		lower_bound_modifier,
		upper_bound_modifier,
		scale);
}

static real sound_calculate_fade(
	long sound_index)
{
	struct sound_datum *sound = sound_get(sound_index);
	real fade = 1.f;

	if (sound->fade_start_time != sound->fade_stop_time)
	{
		fade = ((real)sound_manager_globals.render_time - sound->fade_start_time) /
			(sound->fade_stop_time - sound->fade_start_time);
		fade = PIN(fade, 0.f, 1.f);

		switch (sound->fade_mode)
		{
		case _sound_fade_mode_linear:
			break;

		case _sound_fade_mode_crossfade:
			if (sound->fade_interpolation_end > sound->fade_interpolation_start)
			{
				fade = (real)pow(fade, 1.f / sound_fade_exponent);
			}
			else
			{
				fade = (real)(1.0 - pow(
					1.f - fade,
					1.f / sound_fade_exponent));
			}
			break;

		default:
			match_vassert(
				"c:\\halo\\SOURCE\\sound\\sound_manager.c",
				0xA76,
				FALSE,
				NULL);
			break;
		}

		if (fade == 1.f)
		{
			sound->fade_stop_time = 0;
			sound->fade_start_time = 0;
		}

		fade = (sound->fade_interpolation_end -
			sound->fade_interpolation_start) * fade +
			sound->fade_interpolation_start;
	}

	return fade;
}

static void sound_start_fade(
	short mode,
	real seconds,
	long fade_in_sound_index,
	long fade_out_sound_index)
{
	long fade_start_time;
	long fade_stop_time;

	match_assert(
		"c:\\halo\\SOURCE\\sound\\sound_manager.c",
		0x43F,
		mode==_sound_fade_mode_linear || mode==_sound_fade_mode_crossfade);
	match_assert(
		"c:\\halo\\SOURCE\\sound\\sound_manager.c",
		0x440,
		seconds>=0.f);
	match_assert(
		"c:\\halo\\SOURCE\\sound\\sound_manager.c",
		0x441,
		fade_in_sound_index!=NONE || fade_out_sound_index!=NONE);

	fade_start_time = sound_manager_globals.render_time - 1;
	fade_stop_time = (long)(seconds * 1000.f + fade_start_time);
	fade_stop_time = MAX(fade_stop_time, sound_manager_globals.render_time);

	if (fade_in_sound_index != NONE)
	{
		struct sound_datum *sound = sound_get(fade_in_sound_index);

		if (sound->fade_start_time != sound->fade_stop_time)
		{
			sound->fade_interpolation_start =
				sound_calculate_fade(fade_in_sound_index);
		}
		else
		{
			sound->fade_interpolation_start = 0.f;
		}

		sound->fade_interpolation_end = 1.f;
		sound->fade_mode = mode;
		sound->fade_start_time = fade_start_time;
		sound->fade_stop_time = fade_stop_time;
	}

	if (fade_out_sound_index != NONE)
	{
		struct sound_datum *sound = sound_get(fade_out_sound_index);

		sound->fade_interpolation_start =
			sound_calculate_fade(fade_out_sound_index);
		sound->fade_interpolation_end = 0.f;
		sound->fade_mode = mode;
		sound->fade_start_time = fade_start_time;
		sound->fade_stop_time = fade_stop_time;
	}

	return;
}

/* port: retire every owned intro/loop voice when the primary handle changes. */
static void sound_fade_looping_track_components(
	long looping_sound_index,
	short track_index,
	long except_sound_index,
	real seconds)
{
	long sound_index;

	for (sound_index = data_next_index(sound_data, NONE);
		sound_index != NONE;
		sound_index = data_next_index(sound_data, sound_index))
	{
		struct sound_datum *sound = sound_get(sound_index);

		if (sound_index != except_sound_index &&
			(sound->type == _sound_start_track || sound->type == _sound_loop_track) &&
			sound->source_identifier == looping_sound_index &&
			sound->loop_track_index == track_index)
		{
			sound_start_fade(_sound_fade_mode_linear, seconds, NONE, sound_index);
		}
	}
}

static short channel_get_state(
	short channel_index)
{
	struct sound_channel_datum *channel = channel_get(channel_index);
	short state = sound_manager_globals.platform_definition->get_channel_state(
		channel_index);

	if (channel->queued_permutation && state < 2)
	{
		sound_cache_sound_finished(channel->playing_permutation);
		channel->playing_permutation = channel->queued_permutation;
		channel->queued_permutation = NULL;
		channel->estimated_tick_time = 0.f;

		if (!_sound_cache_sound_request(
			channel->playing_permutation,
			FALSE,
			FALSE,
			FALSE))
		{
			state = 0;
		}
	}

	if (channel->playing_permutation && state < 1)
	{
		match_assert(
			"c:\\halo\\SOURCE\\sound\\sound_manager.c",
			0x868,
			!channel->queued_permutation);
		sound_cache_sound_finished(channel->playing_permutation);
		channel->playing_permutation = NULL;
	}

	channel->estimated_tick_time +=
		channel->pitch * sound_manager_globals.ticks_elapsed;

	return state;
}

static void channel_stop(
	short channel_index)
{
	struct sound_channel_datum *channel = channel_get(channel_index);

	if (channel->queued_permutation)
	{
		sound_cache_sound_finished(channel->queued_permutation);
		channel->queued_permutation = NULL;
	}

	if (channel->playing_permutation)
	{
		sound_cache_sound_finished(channel->playing_permutation);
		channel->playing_permutation = NULL;
	}

	sound_manager_globals.platform_definition->stop_channel(channel_index);

	return;
}

static void sound_stop(
	long sound_index)
{
	struct sound_datum *sound = sound_get(sound_index);
	struct sound_definition *definition =
		sound_definition_get(sound->definition_index);

	if (sound->playing_channel_index != NONE)
	{
		channel_get(sound->playing_channel_index)->sound_index = NONE;
		channel_stop(sound->playing_channel_index);
		sound->playing_channel_index = NONE;
	}
	else if (TEST_FLAG(sound->flags, _sound_cached_bit))
	{
		struct sound_permutation *permutation = sound_permutation_get(
			sound->definition_index,
			sound->pitch_range_index,
			sound->permutation_index);

		sound_cache_sound_finished(permutation);
	}

	if (sound->type != _sound_impulse)
	{
		struct looping_sound_datum *looping_sound = datum_try_and_get(
			looping_sound_data,
			sound->source_identifier);

		if (looping_sound)
		{
			looping_sound->component_sound_count--;
			if (looping_sound->tracks[sound->loop_track_index].primary_sound_index ==
				sound_index)
			{
				looping_sound->tracks[sound->loop_track_index].primary_sound_index = NONE;
			}
		}
	}

	if (definition->scripting_sound_index == sound_index)
	{
		definition->scripting_sound_index = NONE;
	}

	sound_delete(sound_index);

	return;
}

static real source_distance(
	short listener_index,
	struct sound_source *source)
{
	real distance;

	switch (source->spatialization_mode)
	{
	case _sound_spatialization_mode_none:
		distance = 0.f;
		break;

	case _sound_spatialization_mode_absolute:
		{
			real_point3d const *listener_position =
				&listener_get(listener_index)->matrix.position;
			real x = listener_position->x - source->location.position.x;
			real y = listener_position->y - source->location.position.y;
			real z = listener_position->z - source->location.position.z;

			distance = square_root(y * y + (x * x + z * z));
		}
		match_assert(
			"c:\\halo\\SOURCE\\sound\\sound_manager.c",
			0x58D,
			listener_get(listener_index)->valid);
		break;

	case _sound_spatialization_mode_relative:
		distance = square_root(
			source->location.position.x * source->location.position.x +
			source->location.position.y * source->location.position.y +
			source->location.position.z * source->location.position.z);
		break;

	default:
		/* BUG (original): January returns the uninitialized distance after
		 * reporting an invalid spatialization mode. The HCEA binary preserves
		 * the same invalid-mode stack read; valid enum values never take it. */
		match_vassert(
			"c:\\halo\\SOURCE\\sound\\sound_manager.c",
			0x593,
			FALSE,
			NULL);
		break;
	}

	return distance;
}

static real source_distance_squared(
	short listener_index,
	struct sound_source *source)
{
	real distance_squared = 0.0f;

	switch (source->spatialization_mode)
	{
	case _sound_spatialization_mode_none:
		distance_squared = 0.f;
		break;

	case _sound_spatialization_mode_absolute:
		distance_squared = distance_squared3d(
			&source->location.position,
			&listener_get(listener_index)->matrix.position);
		match_assert(
			"c:\\halo\\SOURCE\\sound\\sound_manager.c",
			0x574,
			listener_get(listener_index)->valid);
		break;

	case _sound_spatialization_mode_relative:
		distance_squared =
			source->location.position.x * source->location.position.x +
			source->location.position.y * source->location.position.y +
			source->location.position.z * source->location.position.z;
		break;

	default:
		match_vassert(
			"c:\\halo\\SOURCE\\sound\\sound_manager.c",
			0x57A,
			FALSE,
			NULL);
		break;
	}

	return distance_squared;
}

static boolean sound_preempts_sound(
	long challenger_sound_index,
	long champion_sound_index,
	real challenger_distance_squared)
{
	struct sound_datum *challenger_sound = sound_get(challenger_sound_index);
	struct sound_definition *challenger_definition =
		sound_definition_get(challenger_sound->definition_index);
	struct sound_datum *champion_sound = sound_get(champion_sound_index);
	struct sound_definition *champion_definition =
		sound_definition_get(champion_sound->definition_index);

	match_assert(
		"c:\\halo\\SOURCE\\sound\\sound_manager.c",
		0x763,
		challenger_sound_index!=champion_sound_index);

	return
		sound_class_get(challenger_definition->sound_class)->priority >
			sound_class_get(champion_definition->sound_class)->priority ||
		(sound_class_get(challenger_definition->sound_class)->priority ==
			sound_class_get(champion_definition->sound_class)->priority &&
			challenger_distance_squared < source_distance_squared(
				(word)champion_sound->listener_index,
				&champion_sound->source));
}

static short sound_find_best_channel(
	long sound_index)
{
	short best_channel_index = NONE;
	struct sound_datum *sound = sound_get(sound_index);
	struct sound_definition *definition =
		sound_definition_get(sound->definition_index);
	long best_sound_index = NONE;
	struct sound_source *source = &sound->source;
	real sound_distance_squared = source_distance_squared(
		sound->listener_index,
		source);
	real best_sound_distance_squared;
	short channel_index;

	for (
		channel_index = 0;
		channel_index < sound_manager_globals.channel_count;
		channel_index++)
	{
		struct sound_channel_datum *channel = channel_get(channel_index);

		if (sound_valid_for_channel(
				definition->compression,
				definition->encoding,
				definition->sample_rate,
				sound->source.spatialization_mode,
				channel->type_flags))
		{
			if (channel->sound_index == NONE)
			{
				return channel_index;
			}

			if (sound_preempts_sound(
					sound_index,
					channel->sound_index,
					sound_distance_squared) &&
				(best_channel_index == NONE ||
					sound_preempts_sound(
						best_sound_index,
						channel->sound_index,
						best_sound_distance_squared)))
			{
				struct sound_datum *other_sound = sound_get(channel->sound_index);

				best_channel_index = channel_index;
				best_sound_index = channel->sound_index;
				best_sound_distance_squared = source_distance_squared(
					other_sound->listener_index,
					&other_sound->source);
			}
		}
	}

	return best_channel_index;
}

static short sound_find_channel(
	long sound_index)
{
	struct sound_datum *sound = sound_get(sound_index);

	if (sound->playing_channel_index != NONE)
	{
		match_assert(
			"c:\\halo\\SOURCE\\sound\\sound_manager.c",
			0x6D0,
			channel_get(sound->playing_channel_index)->sound_index==sound_index);

		return sound->playing_channel_index;
	}

	if (sound_class_get(
			sound_definition_get(sound->definition_index)->sound_class)->speech &&
		sound->source_identifier != NONE)
	{
		short channel_index;

		for (
			channel_index = 0;
			channel_index < sound_manager_globals.channel_count;
			channel_index++)
		{
			struct sound_channel_datum *channel = channel_get(channel_index);

			if (channel->sound_index != NONE)
			{
				struct sound_datum *other_sound = sound_get(channel->sound_index);

				if (other_sound->source_identifier == sound->source_identifier &&
					sound_class_get(
						sound_definition_get(
							other_sound->definition_index)->sound_class)->speech)
				{
					sound->source.spatialization_mode =
						other_sound->source.spatialization_mode;

					return channel_index;
				}
			}
		}

		return sound_find_best_channel(sound_index);
	}
	else
	{
		struct sound_channel_summary summary;

		sound_channel_summary_build(&summary, sound_index);
		if (summary.like_source_count >= summary.maximum_source_instance_count)
		{
			return sound_find_like_channel(
				sound_index,
				summary.like_source_channels,
				summary.like_source_count);
		}

		if (summary.like_definition_count >= summary.maximum_instance_count)
		{
			return sound_find_like_channel(
				sound_index,
				summary.like_definition_channels,
				summary.like_definition_count);
		}

		return sound_find_best_channel(sound_index);
	}
}

static short sound_find_like_channel(
	long sound_index,
	short const *channel_indices,
	short channel_count)
{
	struct sound_datum *sound = sound_get(sound_index);
	struct sound_definition *definition =
		sound_definition_get(sound->definition_index);
	real sound_distance_squared = source_distance_squared(
		sound->listener_index,
		&sound->source);
	short index;

	for (index = 0; index < channel_count; index++)
	{
		short channel_index = channel_indices[index];
		struct sound_datum *other_sound =
			sound_get(channel_get(channel_index)->sound_index);

		if (sound_manager_globals.render_time - other_sound->start_time >=
			sound_class_get(definition->sound_class)->preemption_time)
		{
			real other_distance_squared = source_distance_squared(
				other_sound->listener_index,
				&other_sound->source);

			if (sound_distance_squared - other_distance_squared < 1.f)
			{
				return channel_index;
			}
		}
	}

	return NONE;
}

static void update_channel_for_impulse_sound(
	short channel_index,
	real fade)
{
	struct sound_channel_datum *channel = channel_get(channel_index);
	struct sound_datum *sound = sound_get(channel->sound_index);
	struct sound_definition *definition =
		sound_definition_get(sound->definition_index);
	real scale = sound->source.scale;
	real gain = sound_scale_value(
		fade * sound->source.gain * sound_manager_master_gain(definition->sound_class),
		definition->zero_gain_modifier,
		definition->one_gain_modifier,
		scale);

	if (sound->playing_channel_index == NONE)
	{
		struct platform_sound_channel_properties properties;
		struct sound_pitch_range *pitch_range = TAG_BLOCK_GET_ELEMENT(
			&definition->pitch_ranges,
			sound->pitch_range_index,
			struct sound_pitch_range);
		struct sound_permutation *permutation = TAG_BLOCK_GET_ELEMENT(
			&pitch_range->permutations,
			sound->permutation_index,
			struct sound_permutation);

		properties.gain = permutation->gain * definition->gain_modifier * gain;
		properties.pitch = sound->pitch * pitch_range->playback_rate;
		properties.minimum_distance = sound_definition_get_minimum_distance(
			sound->definition_index);
		properties.maximum_distance = FLT_MAX;
		properties.cone_inside_angle = definition->inner_cone_angle;
		properties.cone_outside_angle = definition->outer_cone_angle;
		properties.cone_outside_gain = definition->outer_cone_gain;
		properties.reverb_attenuation =
			sound_class_get(definition->sound_class)->wet_gain;

		match_assert(
			"c:\\halo\\SOURCE\\sound\\sound_manager.c",
			0x79A,
			sound_cache_sound_loaded(permutation));
		channel_set_properties_hardware(channel_index, &properties, FALSE);
		channel_queue_sound(channel_index, permutation);
		sound->playing_channel_index = channel_index;
	}
	else
	{
		struct platform_sound_channel_properties properties;

		properties.gain =
			channel->playing_permutation->gain * definition->gain_modifier * gain;

		channel_set_properties_hardware(channel_index, &properties, TRUE);
	}

	sound_manager_globals.platform_definition->channel_update(channel_index);

	return;
}

void sound_stop_all(
	void)
{
	if (sound_manager_globals.initialized)
	{
		long sound_index = data_next_index(sound_data, NONE);

		while (sound_index != NONE)
		{
			sound_stop(sound_index);
			sound_index = data_next_index(sound_data, sound_index);
		}

		data_delete_all(looping_sound_data);
		sound_manager_globals.platform_definition->flush();
	}

	sound_manager_globals.game_time_when_no_scripted_dialog_will_be_playing = 0;

	return;
}

long sound_new_impulse(
	long definition_index,
	struct sound_source *source,
	long source_identifier,
	boolean (*track_proc)(
		long source_identifier,
		void const *track_data,
		struct sound_source *source),
	void const *track_data,
	short track_data_size)
{
	long sound_index = NONE;
	struct sound_definition *definition = sound_definition_get(definition_index);
	real scale = source->scale;

	match_assert(
		"c:\\halo\\SOURCE\\sound\\sound_manager.c",
		0x240,
		track_data_size<=MAXIMUM_SOUND_CALLBACK_DATA);
	match_assert(
		"c:\\halo\\SOURCE\\sound\\sound_manager.c",
		0x242,
		source->spatialization_mode==_sound_spatialization_mode_none ||
			valid_real_normal3d(&source->location.forward));

	if (definition->sound_class == _sound_class_scripted_dialog_to_player ||
		definition->sound_class == _sound_class_scripted_dialog_to_other ||
		definition->sound_class ==
			_sound_class_scripted_dialog_force_unspatialized)
	{
		long dialog_stop_time = game_time_get() +
			30 * definition->longest_permutation_length / 1000 + 10;

		if (dialog_stop_time >
			sound_manager_globals.game_time_when_no_scripted_dialog_will_be_playing)
		{
			sound_manager_globals.game_time_when_no_scripted_dialog_will_be_playing =
				dialog_stop_time;
		}

		if (loud_dialog_hack)
		{
			source->spatialization_mode = _sound_spatialization_mode_none;
		}
	}

	if (definition->sound_class ==
		_sound_class_scripted_dialog_force_unspatialized)
	{
		source->spatialization_mode = _sound_spatialization_mode_none;
	}

	if (sound_manager_globals.initialized && sound_manager_globals.enabled)
	{
		if (definition->compression == _sound_compression_xbox_adpcm &&
			((definition->encoding == _sound_encoding_mono &&
				definition->sample_rate == 0) ||
				definition->encoding == _sound_encoding_stereo))
		{
			if (source->scale != 0.f || definition->zero_gain_modifier != 0.f)
			{
				real random = real_seed_random(
					get_global_local_random_seed_address());

				if (random > sound_scale_value(
					definition->skip_fraction,
					definition->zero_skip_fraction_modifier,
					definition->one_skip_fraction_modifier,
					scale))
				{
					real maximum_distance =
						sound_definition_get_maximum_distance(definition_index);

					if (sound_definition_is_playable(definition_index))
					{
						short listener_index = source_audible(
							source,
							maximum_distance);

						if (listener_index != NONE)
						{
							short promotion_result =
								sound_definition_promote(definition_index);

							if (promotion_result == _sound_promotion_dont)
							{
								sound_index = datum_new(sound_data);
								if (sound_index != NONE)
								{
									struct sound_datum *sound = sound_get(sound_index);
									long travel_milliseconds = sound_travel_milliseconds(
										source_distance(listener_index, source));

									sound->definition_index = definition_index;
									sound->playing_channel_index = NONE;
									sound->listener_index = listener_index;
									sound->type = _sound_impulse;
									sound->pitch = sound_scale_random_value(
										definition->random_pitch_bounds.lower,
										definition->random_pitch_bounds.upper,
										definition->zero_pitch_modifier,
										definition->one_pitch_modifier,
										source->scale);
									sound->flags = 0;
									sound->source_identifier = source_identifier;
									sound->source = *source;
									sound->track_proc = track_proc;

									if (track_proc)
									{
										match_assert(
											"c:\\halo\\SOURCE\\sound\\sound_manager.c",
											0x28E,
											sound->track_data);
										csmemcpy(
											sound->track_data,
											track_data,
											track_data_size);
									}

									sound->pitch_range_index =
										sound_definition_find_pitch_range_by_pitch(
											definition,
											sound->pitch,
											NONE);
									sound->permutation_index =
										sound_definition_next_permutation(
											definition,
											sound->pitch_range_index,
											NONE);
									sound->fade_stop_time = 0;
									sound->fade_start_time = 0;
									sound->loop_track_index = NONE;
									_sound_cache_sound_request(
										sound_permutation_get(
											sound->definition_index,
											sound->pitch_range_index,
											sound->permutation_index),
										FALSE,
										TRUE,
										FALSE);

									if (travel_milliseconds > speed_of_sound_threshold)
									{
										sound->start_time =
											sound_manager_globals.render_time +
											travel_milliseconds;
										SET_FLAG(sound->flags, _sound_delayed_bit, TRUE);
									}
									else
									{
										sound->start_time = sound_manager_globals.render_time;
									}
								}
							}
							else if (promotion_result == _sound_promotion_do)
							{
								sound_index = sound_new_impulse(
									definition->promotion_sound.index,
									source,
									source_identifier,
									track_proc,
									track_data,
									track_data_size);
							}
							else
							{
								/* the promotion throttle rejected this play */
								sound_index = NONE;
							}
						}
					}
				}
			}
		}
		else
		{
			/* (port: which sound, and what it is) */
			error(
				_error_silent,
				"attempt to play a sound that was not a mono 22k compressed sound or a stereo 22k or 44k compressed sound: %s (compression %d, encoding %d, sample rate %d)",
				tag_get_name(definition_index),
				definition->compression,
				definition->encoding,
				definition->sample_rate);
		}
	}

	return sound_index;
}

boolean sound_refresh_looping(
	long definition_index,
	long looping_sound_identifier,
	struct sound_source *source,
	short refresh_state,
	boolean alternate,
	real fade_time)
{
	boolean result = refresh_state == _looping_sound_refresh_stop;

	match_assert(
		"c:\\halo\\SOURCE\\sound\\sound_manager.c",
		0x2F4,
		source->spatialization_mode==_sound_spatialization_mode_none ||
			valid_real_normal3d(&source->location.forward));
	render_debug_looping_sound(definition_index, source);

	if (sound_is_active())
	{
		long looping_sound_index = looping_sound_find(looping_sound_identifier);
		boolean new_looping_sound = FALSE;

		result = TRUE;
		if (looping_sound_index == NONE &&
			refresh_state != _looping_sound_refresh_stop)
		{
			looping_sound_index = looping_sound_new(
				definition_index,
				looping_sound_identifier,
				source);
			new_looping_sound = TRUE;
		}

		if (looping_sound_index != NONE)
		{
			struct looping_sound_datum *loop =
				looping_sound_get(looping_sound_index);
			struct looping_sound_definition *definition =
				looping_sound_definition_get(definition_index);
			short track_index;

			match_assert(
				"c:\\halo\\SOURCE\\sound\\sound_manager.c",
				0x30E,
				loop->definition_index==definition_index);

			loop->source = *source;
			loop->flip_flop = sound_manager_globals.flip_flop;

			if ((refresh_state == _looping_sound_refresh_stop ||
				loop->ordered_sounds_finished) &&
				loop->component_sound_count == 0)
			{
				datum_delete(looping_sound_data, looping_sound_index);
			}
			else
			{
				result = FALSE;

				if (definition->continuous_damage_effect.index != NONE)
				{
					player_effect_continuous_refresh(
						definition->continuous_damage_effect.index,
						&source->location.position);
				}

				/* port: no more tracks than the looping sound holds (the map's count) */
				for (track_index = 0;
					track_index < definition->tracks.count &&
						track_index < (short)NUMBEROF(loop->tracks);
					track_index++)
				{
					struct looping_sound_track *track = TAG_BLOCK_GET_ELEMENT(
						&definition->tracks,
						track_index,
						struct looping_sound_track);
					long *playing_sound_index =
						&loop->tracks[track_index].primary_sound_index;
					long sound_definition_index = 0;

					if (new_looping_sound)
					{
						*playing_sound_index = NONE;
					}

					if (refresh_state == _looping_sound_refresh_start)
					{
						/* port: a restart must retire the old primary and pending components. */
						if (track->start_sound.index != NONE ||
							TEST_FLAG(track->flags, _fade_in_at_start_bit))
						{
							sound_fade_looping_track_components(
								looping_sound_index, track_index, NONE,
								track->fade_out_duration);
						}
						if (track->start_sound.index != NONE)
						{
							*playing_sound_index =
								update_potentially_audible_looping_sound(
									track->start_sound.index,
									looping_sound_index,
									track_index,
									_sound_start_track);
						}
					}

					if (refresh_state != _looping_sound_refresh_stop &&
						!loop->ordered_sounds_finished)
					{
						sound_definition_index = track->loop_sound.index;
						if (alternate && track->alternate_loop_sound.index != NONE)
						{
							sound_definition_index = track->alternate_loop_sound.index;
						}

						if (sound_definition_index != NONE)
						{
							if (*playing_sound_index != NONE &&
								(refresh_state != _looping_sound_refresh_start ||
									!TEST_FLAG(track->flags, _fade_in_at_start_bit)))
							{
								sound_get(*playing_sound_index);

								if (alternate != loop->alternate &&
									TEST_FLAG(track->flags, _fade_in_alternate_bit))
								{
									long new_sound_index =
										update_potentially_audible_looping_sound(
											sound_definition_index,
											looping_sound_index,
											track_index,
											_sound_loop_track);

									if (new_sound_index != NONE)
									{
										sound_start_fade(
											_sound_fade_mode_linear,
											track->fade_out_duration,
											new_sound_index,
											*playing_sound_index);
										*playing_sound_index = new_sound_index;
									}
								}
								else if (!new_looping_sound)
								{
									sound_set_definition_begin(
										*playing_sound_index,
										sound_definition_index);
								}
							}
							else
							{
								long new_sound_index =
									update_potentially_audible_looping_sound(
										sound_definition_index,
										looping_sound_index,
										track_index,
										_sound_loop_track);

								if (new_sound_index != NONE)
								{
									sound_get(new_sound_index);

									if (refresh_state == _looping_sound_refresh_start)
									{
										if (TEST_FLAG(track->flags, _fade_in_at_start_bit))
										{
											sound_start_fade(
												_sound_fade_mode_linear,
												track->fade_in_duration,
												new_sound_index,
												NONE);
										}
									}
									else
									{
										sound_start_fade(
											_sound_fade_mode_linear,
											sound_inaudible_fade_out_time,
											new_sound_index,
											NONE);
									}

									*playing_sound_index = new_sound_index;
								}
							}
						}
					}
					else if (loop->state != _looping_sound_refresh_stop)
					{
						/* port: stopping the loop also stops its other owned components. */
						if (fade_time != 0.f ||
							TEST_FLAG(track->flags, _fade_out_at_stop_bit) ||
							(track->stop_sound.index == NONE &&
								!TEST_FLAG(definition->flags, _looping_sound_fake_impulse_sound_bit)))
						{
							sound_fade_looping_track_components(
								looping_sound_index, track_index, *playing_sound_index,
								fade_time != 0.f ? fade_time : track->fade_out_duration);
						}
						if (fade_time != 0.f)
						{
							sound_start_fade(
								_sound_fade_mode_linear,
								fade_time,
								NONE,
								*playing_sound_index);
						}
						else
						{
							if (*playing_sound_index != NONE &&
								(TEST_FLAG(track->flags, _fade_out_at_stop_bit) ||
									(track->stop_sound.index == NONE &&
										!TEST_FLAG(
											definition->flags,
											_looping_sound_fake_impulse_sound_bit))))
							{
								sound_start_fade(
									_sound_fade_mode_linear,
									track->fade_out_duration,
									NONE,
									*playing_sound_index);
							}

							if (track->stop_sound.index != NONE)
							{
								sound_definition_index = track->stop_sound.index;
								if (alternate &&
									track->alternate_stop_sound.index != NONE)
								{
									sound_definition_index =
										track->alternate_stop_sound.index;
								}

								if (TEST_FLAG(
									track->flags,
									_fade_out_at_stop_bit))
								{
									update_potentially_audible_looping_sound(
										sound_definition_index,
										looping_sound_index,
										track_index,
										_sound_stop_track);
								}
								else if (*playing_sound_index != NONE)
								{
									struct sound_datum *playing_sound =
										sound_get(*playing_sound_index);

									if (playing_sound->playing_channel_index != NONE)
									{
										sound_set_definition_begin(
											*playing_sound_index,
											sound_definition_index);
										playing_sound->type = _sound_stopping_track;
									}
								}
							}
						}
					}
				}

				if (loop->component_sound_count == 0 &&
					source_audible(source, definition->runtime_maximum_distance) == NONE)
				{
					datum_delete(looping_sound_data, looping_sound_index);
				}

				loop->alternate = alternate;
				loop->state = refresh_state;
			}
		}
		else if (new_looping_sound)
		{
			result = FALSE;
		}
	}

	return result;
}

static void update_channel_for_looping_sound(
	short channel_index,
	real fade)
{
	struct sound_channel_datum *channel = channel_get(channel_index);
	struct sound_datum *sound = sound_get(channel->sound_index);
	struct sound_definition *definition =
		sound_definition_get(sound->definition_index);
	struct looping_sound_datum *looping_sound =
		looping_sound_get(sound->source_identifier);
	long *primary_sound_index =
		&looping_sound->tracks[sound->loop_track_index].primary_sound_index;
	struct looping_sound_definition *looping_definition =
		looping_sound_definition_get(looping_sound->definition_index);
	struct looping_sound_track *track = TAG_BLOCK_GET_ELEMENT(
		&looping_definition->tracks,
		sound->loop_track_index,
		struct looping_sound_track);
	real scale = sound->source.scale;
	real pitch = sound_scale_value(
		sound->pitch,
		definition->zero_pitch_modifier,
		definition->one_pitch_modifier,
		scale);
	struct platform_sound_channel_properties properties;
	struct sound_pitch_range *pitch_range;

	match_assert(
		"c:\\halo\\SOURCE\\sound\\sound_manager.c",
		0x9C4,
		sound->type!=_sound_impulse);

	properties.minimum_distance = sound_definition_get_minimum_distance(
		sound->definition_index);
	properties.maximum_distance = FLT_MAX;
	properties.cone_inside_angle = definition->inner_cone_angle;
	properties.cone_outside_angle = definition->outer_cone_angle;
	properties.cone_outside_gain = definition->outer_cone_gain;
	properties.reverb_attenuation =
		sound_class_get(definition->sound_class)->wet_gain;
	properties.gain = sound_manager_master_gain(definition->sound_class) *
		track->gain *
		sound_scale_value(
			definition->gain_modifier,
			definition->zero_gain_modifier,
			definition->one_gain_modifier,
			scale) *
		sound->source.gain * fade;

	if (sound->playing_channel_index == NONE)
	{
		struct sound_permutation *permutation;

		pitch_range = TAG_BLOCK_GET_ELEMENT(
			&definition->pitch_ranges,
			sound->pitch_range_index,
			struct sound_pitch_range);
		permutation = TAG_BLOCK_GET_ELEMENT(
			&pitch_range->permutations,
			sound->permutation_index,
			struct sound_permutation);
		properties.gain *= permutation->gain;
		properties.pitch = pitch * pitch_range->playback_rate;

		match_assert(
			"c:\\halo\\SOURCE\\sound\\sound_manager.c",
			0x9DA,
			sound_cache_sound_loaded(permutation));
		channel_set_properties_hardware(channel_index, &properties, FALSE);
		channel_queue_sound(channel_index, permutation);
		sound->playing_channel_index = channel_index;
	}
	else
	{
		pitch_range = TAG_BLOCK_GET_ELEMENT(
			&definition->pitch_ranges,
			sound->pitch_range_index,
			struct sound_pitch_range);
		pitch = limit_pitch(
			pitch,
			channel_get(sound->playing_channel_index)->pitch *
				pitch_range->natural_pitch,
			definition->maximum_bend_per_second);
		properties.pitch = pitch * pitch_range->playback_rate;

		if (sound->type == _sound_loop_track &&
			(sound->fade_start_time == sound->fade_stop_time ||
				sound->fade_interpolation_end != 0.f) &&
			sound_definition_find_pitch_range_by_pitch(
				definition,
				pitch,
				sound->pitch_range_index) != sound->pitch_range_index &&
			channel->sound_index == *primary_sound_index &&
			!sound_manager_globals.idling)
		{
			long new_sound_index = update_potentially_audible_looping_sound(
				sound->definition_index,
				sound->source_identifier,
				sound->loop_track_index,
				_sound_loop_track);

			if (new_sound_index != NONE)
			{
				sound_start_fade(
					_sound_fade_mode_crossfade,
					sound_pitch_range_fade_time,
					new_sound_index,
					channel->sound_index);
				*primary_sound_index = new_sound_index;
			}
		}

		if (sound->type != _sound_stop_track &&
			(sound->type != _sound_start_track ||
				!TEST_FLAG(track->flags, _fade_in_at_start_bit)))
		{
			short channel_state = channel_get_state(sound->playing_channel_index);

			if (channel_state != _sound_channel_queued ||
				TEST_FLAG(sound->flags, _sound_waiting_for_cache_bit) ||
				(channel_state == _sound_channel_queued &&
					channel->playing_permutation->next_permutation_index == NONE &&
					sound->next_definition_index != NONE))
			{
				if (sound->next_definition_index != NONE &&
					(!channel->playing_permutation ||
						channel->playing_permutation->next_permutation_index == NONE))
				{
					/* port: a definition transition can retire its own voice. */
					if (!sound_set_definition_end(channel->sound_index))
					{
						return;
					}
					definition = sound_definition_get(sound->definition_index);
					pitch_range = TAG_BLOCK_GET_ELEMENT(
						&definition->pitch_ranges,
						sound->pitch_range_index,
						struct sound_pitch_range);
				}
				else if (!TEST_FLAG(sound->flags, _sound_waiting_for_cache_bit))
				{
					short permutation_index = sound_definition_next_permutation(
						definition,
						sound->pitch_range_index,
						sound->permutation_index);

					if (permutation_index == NONE)
					{
						match_assert(
							"c:\\halo\\SOURCE\\sound\\sound_manager.c",
							0xA1C,
							TEST_FLAG(definition->flags, _sound_definition_linked_permutations_bit));

						if (!TEST_FLAG(
							looping_definition->flags,
							_looping_sound_fake_impulse_sound_bit))
						{
							permutation_index = sound_definition_next_permutation(
								definition,
								sound->pitch_range_index,
								NONE);
						}
						else
						{
							sound->type = _sound_stop_track;
							looping_sound->ordered_sounds_finished = TRUE;
						}
					}

					if (permutation_index != NONE)
					{
						sound->permutation_index = permutation_index;
						SET_FLAG(sound->flags, _sound_waiting_for_cache_bit, TRUE);
					}
				}

				{
					struct sound_permutation *permutation = TAG_BLOCK_GET_ELEMENT(
						&pitch_range->permutations,
						sound->permutation_index,
						struct sound_permutation);

					if (sound->type != _sound_stop_track &&
						_sound_cache_sound_request(
							permutation,
							FALSE,
							TRUE,
							TRUE))
					{
						SET_FLAG(sound->flags, _sound_waiting_for_cache_bit, FALSE);
						channel_queue_sound(channel_index, permutation);

						if (sound->next_definition_index == NONE &&
							permutation->next_permutation_index == NONE)
						{
							if (sound->type == _sound_start_track)
							{
								sound->type = _sound_loop_track;
							}
							else if (sound->type == _sound_stopping_track)
							{
								sound->type = _sound_stop_track;
							}
						}
					}
				}
			}
		}

		properties.gain *= TAG_BLOCK_GET_ELEMENT(
			&pitch_range->permutations,
			sound->permutation_index,
			struct sound_permutation)->gain;
		channel_set_properties_hardware(channel_index, &properties, FALSE);
	}

	sound_manager_globals.platform_definition->channel_update(channel_index);

	return;
}

static void process_looping_sounds(
	void)
{
	long looping_sound_index = data_next_index(looping_sound_data, NONE);

	while (looping_sound_index != NONE)
	{
		struct looping_sound_datum *looping_sound =
			looping_sound_get(looping_sound_index);
		struct looping_sound_definition *definition =
			looping_sound_definition_get(looping_sound->definition_index);

		if (looping_sound->flip_flop != sound_manager_globals.flip_flop)
		{
			datum_delete(looping_sound_data, looping_sound_index);
		}
		else if (looping_sound->state != _looping_sound_refresh_stop)
		{
			short detail_index;

			/* port: no more details than the looping sound holds (the map's count) */
			for (detail_index = 0;
				detail_index < definition->details.count &&
					detail_index < (short)NUMBEROF(looping_sound->detail_play_times);
				detail_index++)
			{
				struct looping_sound_detail *detail = TAG_BLOCK_GET_ELEMENT(
					&definition->details,
					detail_index,
					struct looping_sound_detail);
				long *play_time = &looping_sound->detail_play_times[detail_index];

				if (*play_time < sound_manager_globals.render_time &&
					detail->sound.index != NONE)
				{
					struct sound_definition *sound_definition =
						sound_definition_get(detail->sound.index);
					real scale = looping_sound->source.scale;
					real upper_scale;
					real lower_scale;
					real period_upper_bound;
					real period_lower_bound;
					real period;

					if ((!TEST_FLAG(
							detail->flags,
							_detail_dont_play_with_alternate_bit) ||
							!looping_sound->alternate) &&
						(!TEST_FLAG(
							detail->flags,
							_detail_dont_play_without_alternate_bit) ||
							looping_sound->alternate))
					{
						struct loop_impulse_sound_tracking_data tracking_data;
						struct sound_source source;

						source.spatialization_mode =
							(looping_sound->source.spatialization_mode ==
								_sound_spatialization_mode_none) +
							_sound_spatialization_mode_absolute;
						source.gain = detail->gain;
						source.scale = scale;
						detail_sound_random_offset(
							detail,
							&tracking_data.position_offset);
						track_loop_impulse_sound(
							looping_sound_index,
							&tracking_data,
							&source);
						sound_new_impulse(
							detail->sound.index,
							&source,
							looping_sound_index,
							track_loop_impulse_sound,
							&tracking_data,
							sizeof(tracking_data));
					}

					upper_scale = definition->scale_upper_bound.detail_period;
					lower_scale = definition->scale_lower_bound.detail_period;
					period_upper_bound = detail->period_bounds.upper;
					period_lower_bound = detail->period_bounds.lower;
					period = real_local_random_range(
						period_lower_bound,
						period_upper_bound);

					*play_time = (long)(
						((upper_scale - lower_scale) * scale + lower_scale) *
							period * 1000.f +
						sound_definition->longest_permutation_length +
						sound_manager_globals.render_time);
				}
			}
		}

		looping_sound_index = data_next_index(
			looping_sound_data,
			looping_sound_index);
	}

	return;
}

void sound_dispose_from_old_map(
	void)
{
	if (!sound_manager_globals.paused &&
		sound_manager_globals.initialized &&
		sound_manager_globals.enabled)
	{
		long start_time = (long)system_milliseconds();
		long sound_index = data_next_index(sound_data, NONE);

		if (sound_index != NONE)
		{
			real stop_time;
			boolean done = FALSE;

			do
			{
				sound_start_fade(
					_sound_fade_mode_linear,
					0.3f,
					NONE,
					sound_index);
				sound_index = data_next_index(sound_data, sound_index);
			}
			while (sound_index != NONE);

			stop_time = (real)start_time + 300.f;
			while (!done)
			{
				if ((real)system_milliseconds() < stop_time)
				{
					sound_idle();
				}
				else
				{
					done = TRUE;
				}
			}
		}
	}

	if (sound_manager_globals.paused)
	{
		sound_manager_globals.paused = FALSE;
		sound_manager_globals.platform_definition->set_pause(FALSE);
		sound_manager_globals.render_time = system_milliseconds();
	}

	sound_stop_all();
	if (looping_sound_data)
	{
		data_delete_all(looping_sound_data);
	}

	/* port: a channel still holding a permutation that no sound owns any
	longer (a weapon's charging loop, as a network game ended) stopped too,
	so that its cache count is given back while the map's sounds are still
	there; else the next map's sound_render finished it against the next
	map's sound cache (sound_cache_sound_finished: "xbox sound index ... is
	unused or changed") */
	if (sound_manager_globals.initialized)
	{
		short channel_index;

		for (
			channel_index = 0;
			channel_index < sound_manager_globals.channel_count;
			channel_index++)
		{
			struct sound_channel_datum *channel = channel_get(channel_index);

			if (channel->playing_permutation || channel->queued_permutation)
			{
				channel_stop(channel_index);
				channel->sound_index = NONE;
			}
		}
	}

	return;
}

static short source_audible(
	struct sound_source *source,
	real maximum_distance)
{
	short nearest_listener_index = NONE;

	if (source->spatialization_mode == _sound_spatialization_mode_none)
	{
		nearest_listener_index = 0;
	}
	else if (source->spatialization_mode == _sound_spatialization_mode_relative)
	{
		if (source_distance_squared(NONE, source) < maximum_distance)
		{
			nearest_listener_index = 0;
		}
	}
	else
	{
		real nearest_distance_squared = REAL_MAX;
		short listener_index;

		for (
			listener_index = 0;
			listener_index < MAXIMUM_NUMBER_OF_LOCAL_PLAYERS;
			listener_index++)
		{
			if (listener_get(listener_index)->valid)
			{
				real distance_squared = source_distance_squared(
					listener_index,
					source);

				if (distance_squared < nearest_distance_squared)
				{
					nearest_listener_index = listener_index;
					nearest_distance_squared = distance_squared;
				}
			}
		}

		if (nearest_listener_index != NONE)
		{
			compute_sound_obstruction(
				nearest_listener_index,
				source,
				square_root(nearest_distance_squared));
		}

		if (nearest_distance_squared > maximum_distance * maximum_distance ||
			source->occlusion == 1.f)
		{
			nearest_listener_index = NONE;
		}
	}

	return nearest_listener_index;
}

static void refresh_sounds(
	void)
{
	boolean all_players_dead = players_are_all_dead();
	boolean dialog_playing = FALSE;
	long sound_index = data_next_index(sound_data, NONE);

	while (sound_index != NONE)
	{
		struct sound_datum *sound = sound_get(sound_index);
		struct sound_definition *definition =
			sound_definition_get(sound->definition_index);
		boolean stop_sound = FALSE;

		if (sound->playing_channel_index != NONE &&
			!channel_get_state(sound->playing_channel_index) &&
			sound->type != _sound_loop_track &&
			sound->type != _sound_stopping_track)
		{
			stop_sound = TRUE;
		}

		if (!stop_sound && !refresh_sound(sound_index))
		{
			stop_sound = TRUE;
		}

		if (!stop_sound)
		{
			short listener_index = source_audible(
				&sound->source,
				sound_definition_get_maximum_distance(
					sound->definition_index));

			render_debug_sound(sound_index);

			if (definition->sound_class ==
					_sound_class_scripted_dialog_to_player ||
				definition->sound_class ==
					_sound_class_scripted_dialog_to_other ||
				definition->sound_class ==
					_sound_class_scripted_dialog_force_unspatialized)
			{
				dialog_playing = TRUE;
			}

			if (listener_index == NONE)
			{
				/* port: no listener hears it, and the one it had may have gone (a
				local player who left the game: its listener is no longer valid, and
				the channels' update asserts on a sound's): another's stands in */
				if (!listener_get(sound->listener_index)->valid)
				{
					short other_listener_index;

					for (
						other_listener_index = 0;
						other_listener_index < MAXIMUM_NUMBER_OF_LOCAL_PLAYERS &&
							!listener_get(other_listener_index)->valid;
						other_listener_index++)
					{
					}

					if (other_listener_index < MAXIMUM_NUMBER_OF_LOCAL_PLAYERS)
					{
						sound->listener_index = other_listener_index;
					}
				}

				if (!TEST_FLAG(sound->flags, _sound_inaudible_bit))
				{
					sound_start_fade(
						_sound_fade_mode_linear,
						sound_inaudible_fade_out_time,
						NONE,
						sound_index);
					SET_FLAG(sound->flags, _sound_inaudible_bit, TRUE);
				}
			}
			else
			{
				sound->listener_index = listener_index;
				if (TEST_FLAG(sound->flags, _sound_inaudible_bit))
				{
					sound_start_fade(
						_sound_fade_mode_linear,
						sound_inaudible_fade_back_in_time,
						sound_index,
						NONE);
					SET_FLAG(sound->flags, _sound_inaudible_bit, FALSE);
				}
			}

			if (all_players_dead)
			{
				if (definition->sound_class ==
					_sound_class_scripted_dialog_to_player)
				{
					if (sound->playing_channel_index == NONE)
					{
						stop_sound = TRUE;
					}
					else
					{
						sound_start_fade(
							_sound_fade_mode_linear,
							sound_player_fade_out_time,
							NONE,
							sound_index);
					}
				}
				else if (definition->sound_class ==
						_sound_class_scripted_dialog_to_other &&
					sound->playing_channel_index == NONE)
				{
					stop_sound = TRUE;
				}
			}
		}

		if (stop_sound)
		{
			sound_stop(sound_index);
		}

		sound_index = data_next_index(sound_data, sound_index);
	}

	if (dialog_playing)
	{
		real rate = sound_manager_globals.ticks_elapsed * 0.03f;
		real delta = sound_gain_under_dialog -
			sound_manager_globals.nondialog_gain;

		sound_manager_globals.nondialog_gain += PIN(delta, -rate, rate);
	}
	else
	{
		real rate = sound_manager_globals.ticks_elapsed * 0.007f;
		real delta = 1.f - sound_manager_globals.nondialog_gain;

		sound_manager_globals.nondialog_gain += PIN(delta, -rate, rate);
	}

	return;
}

static void update_channels(
	void)
{
	short channel_index;

	for (
		channel_index = 0;
		channel_index < sound_manager_globals.channel_count;
		channel_index++)
	{
		struct sound_channel_datum *channel = channel_get(channel_index);

		if (channel->sound_index != NONE)
		{
			struct sound_datum *sound = sound_get(channel->sound_index);
			struct sound_definition *definition =
				sound_definition_get(sound->definition_index);
			real fade = sound_calculate_fade(channel->sound_index);

			if (fade == 0.f && sound->fade_interpolation_end == 0.f)
			{
				sound_stop(channel->sound_index);
				channel->sound_index = NONE;
				continue;
			}

			if (TEST_FLAG(channel->type_flags, _sound_channel_3d_bit))
			{
				switch (sound->source.spatialization_mode)
				{
				case _sound_spatialization_mode_none:
					match_vassert(
						"c:\\halo\\SOURCE\\sound\\sound_manager.c",
						0x7D4,
						FALSE,
						NULL);
					break;

				case _sound_spatialization_mode_absolute:
					{
						struct sound_listener *listener =
							listener_get(sound->listener_index);
						struct sound_location location;

						match_assert(
							"c:\\halo\\SOURCE\\sound\\sound_manager.c",
							0x7DB,
							listener->valid);
						matrix4x3_inverse_transform_point(
							&listener->matrix,
							&sound->source.location.position,
							&location.position);
						matrix4x3_inverse_transform_normal(
							&listener->matrix,
							&sound->source.location.forward,
							&location.forward);
						matrix4x3_inverse_transform_vector(
							&listener->matrix,
							&sound->source.location.translational_velocity,
							&location.translational_velocity);
						location.translational_velocity.i =
							TICKS_PER_SECOND * location.translational_velocity.i -
							listener->velocity.i;
						location.translational_velocity.j =
							TICKS_PER_SECOND * location.translational_velocity.j -
							listener->velocity.j;
						location.translational_velocity.k =
							TICKS_PER_SECOND * location.translational_velocity.k -
							listener->velocity.k;
						sound_manager_globals.platform_definition->set_channel_location(
							channel_index,
							TRUE,
							&location,
							sound->source.obstruction,
							sound->source.occlusion,
							listener->underwater);
					}
					break;

				case _sound_spatialization_mode_relative:
					sound_manager_globals.platform_definition->set_channel_location(
						channel_index,
						TRUE,
						&sound->source.location,
						0.f,
						0.f,
						FALSE);
					break;

				default:
					match_vassert(
						"c:\\halo\\SOURCE\\sound\\sound_manager.c",
						0x7EC,
						FALSE,
						NULL);
					break;
				}
			}
			else
			{
				real_point3d relative_position = sound->source.location.position;
				/* port: what a 3D channel is given of its sound's surroundings,
				for a stereo one's (dsound_port_set_channel_stereo_position) */
				real stereo_obstruction = 0.f;
				real stereo_occlusion = 0.f;
				boolean stereo_underwater = FALSE;
				/* port: and its velocity from the listener, for its Doppler effect */
				real_vector3d stereo_velocity = *global_zero_vector3d;

				switch (sound->source.spatialization_mode)
				{
				case _sound_spatialization_mode_none:
					/* port: (unpanned and dry: dsound_port_set_channel_stereo_position) */
					if (TEST_FLAG(channel->type_flags, _sound_channel_stereo_bit))
					{
						dsound_port_set_channel_stereo_position(channel_index, FALSE, 0.f, 0.f, 0.f, 1.f,
							0.f, 0.f, FALSE, 0.f);
					}
					break;

				case _sound_spatialization_mode_absolute:
					{
						struct sound_listener *listener =
							listener_get(sound->listener_index);

						match_assert(
							"c:\\halo\\SOURCE\\sound\\sound_manager.c",
							0x7FA,
							listener->valid);
						matrix4x3_inverse_transform_point(
							&listener->matrix,
							&sound->source.location.position,
							&relative_position);
						stereo_obstruction = sound->source.obstruction;
						stereo_occlusion = sound->source.occlusion;
						stereo_underwater = listener->underwater;
						/* port: as a 3D channel's velocity is given (above) */
						matrix4x3_inverse_transform_vector(
							&listener->matrix,
							&sound->source.location.translational_velocity,
							&stereo_velocity);
						stereo_velocity.i = TICKS_PER_SECOND * stereo_velocity.i - listener->velocity.i;
						stereo_velocity.j = TICKS_PER_SECOND * stereo_velocity.j - listener->velocity.j;
						stereo_velocity.k = TICKS_PER_SECOND * stereo_velocity.k - listener->velocity.k;
					}
					/* fall through */

				case _sound_spatialization_mode_relative:
					{
						real minimum_distance =
							sound_definition_get_minimum_distance(
								sound->definition_index);
						real maximum_distance =
							sound_definition_get_maximum_distance(
								sound->definition_index);
					real distance = square_root(
						relative_position.x * relative_position.x +
						(relative_position.y * relative_position.y +
							relative_position.z * relative_position.z));
						real attenuation = 1.f -
							(distance - minimum_distance) /
							(maximum_distance - minimum_distance);
						real distance_fade = PIN(attenuation, 0.f, 1.f);

						/* port: positioned stereo uses a 2D stream, so it misses
						the 3D stream's inverse-distance gain: it takes that
						instead of the linear fade (the listener's rolloff
						factor is 1), so that it fades as a mono sound does. A
						3D sound is not cut off at the tag's maximum distance
						either: none starts past it. */
						if (TEST_FLAG(channel->type_flags, _sound_channel_stereo_bit))
						{
							distance_fade = minimum_distance > 0.f && distance > minimum_distance ?
								minimum_distance / distance : 1.f;
						}
						fade *= distance_fade;
						/* port: and a stereo sound is panned towards where it
						is, as the mixer pans a 3D one (port/linux/src/dsound_sdl.c,
						spatialize: ahead is x, right -y; centred when close),
						and muffled and reverberated as one is, which the Xbox's
						stereo channels never were: a Custom Edition map's stereo
						gunfire came from nowhere. (Obstruction and occlusion go
						where the 3D channels' call above puts them.) */
						if (TEST_FLAG(channel->type_flags, _sound_channel_stereo_bit))
						{
							real horizontal = square_root(
								relative_position.x * relative_position.x +
								relative_position.y * relative_position.y);
							real pan = horizontal > 1.0e-4f ? -relative_position.y / horizontal : 0.f;
							/* port: how fast it comes towards the listener, for
							its Doppler effect, as a 3D channel's (dsound_sdl.c,
							doppler_ratio) */
							real approach = distance > 1.0e-4f ?
								-(stereo_velocity.i * relative_position.x +
									stereo_velocity.j * relative_position.y +
									stereo_velocity.k * relative_position.z) / distance :
								0.f;

							if (distance < minimum_distance && minimum_distance > 0.f)
							{
								pan *= distance / minimum_distance;
							}
							dsound_port_set_channel_stereo_position(channel_index, TRUE, 0.75f * pan,
								distance, minimum_distance, distance_fade,
								stereo_obstruction, stereo_occlusion, stereo_underwater, approach);
						}
					}
					break;

				default:
					match_vassert(
						"c:\\halo\\SOURCE\\sound\\sound_manager.c",
						0x80A,
						FALSE,
						NULL);
					break;
				}
			}

			if (sound->type == _sound_impulse)
			{
				update_channel_for_impulse_sound(channel_index, fade);
			}
			else
			{
				update_channel_for_looping_sound(channel_index, fade);
			}

			if (sound_class_get(definition->sound_class)->speech &&
				sound->track_proc == track_object_impulse_sound)
			{
				game_sound_set_mouth_aperture(
					sound->source_identifier,
					sound_permutation_get_real_mouth_aperture(
						channel->playing_permutation,
						(long)channel->estimated_tick_time));
			}
		}
	}

	return;
}

static void prioritize_sounds(
	void)
{
	long sound_index = data_next_index(sound_data, NONE);

	while (sound_index != NONE)
	{
		struct sound_datum *sound = sound_get(sound_index);

		if (sound->start_time <= sound_manager_globals.render_time)
		{
			if (sound->playing_channel_index != NONE ||
				_sound_cache_sound_request(
					sound_permutation_get(
						sound->definition_index,
						sound->pitch_range_index,
						sound->permutation_index),
					FALSE,
					TRUE,
					TRUE))
			{
				short channel_index;

				SET_FLAG(sound->flags, _sound_cached_bit, TRUE);
				channel_index = sound_find_channel(sound_index);
				if (channel_index != NONE)
				{
					struct sound_channel_datum *channel = channel_get(channel_index);

					if (channel->sound_index != sound_index)
					{
						struct sound_definition *definition =
							sound_definition_get(sound->definition_index);

						match_vassert(
							"c:\\halo\\SOURCE\\sound\\sound_manager.c",
							0x642,
							sound_valid_for_channel(
								definition->compression,
								definition->encoding,
								definition->sample_rate,
								sound->source.spatialization_mode,
								channel->type_flags),
							"sound_valid_for_channel(definition->compression, definition->encoding, definition->sample_rate, sound->source.spatialization_mode, channel->type_flags)");
						if (channel->sound_index != NONE)
						{
							sound_stop(channel->sound_index);
						}

						channel->sound_index = sound_index;
						sound->start_time = sound_manager_globals.render_time;
					}
					else
					{
						match_assert(
							"c:\\halo\\SOURCE\\sound\\sound_manager.c",
							0x654,
							sound->playing_channel_index==channel_index);
					}
				}
				else
				{
					sound_stop(sound_index);
				}
			}
			else if (sound->playing_channel_index == NONE &&
				sound->track_proc != track_loop_impulse_sound)
			{
				struct sound_definition *definition =
					sound_definition_get(sound->definition_index);

				switch (sound_class_get(definition->sound_class)->cache_miss_mode)
				{
				case _sound_cache_miss_mode_discard:
					{
						struct sound_pitch_range *pitch_range = TAG_BLOCK_GET_ELEMENT(
							&definition->pitch_ranges,
							sound->pitch_range_index,
							struct sound_pitch_range);

						if (pitch_range->forced_permutation_index == NONE)
						{
							pitch_range->forced_permutation_index =
								sound->permutation_index;
						}
						sound_stop(sound_index);
					}
					break;

				case _sound_cache_miss_mode_postpone:
					break;

				default:
					match_vassert(
						"c:\\halo\\SOURCE\\sound\\sound_manager.c",
						0x679,
						FALSE,
						NULL);
					break;
				}
			}
		}
		else
		{
			match_vassert(
				"c:\\halo\\SOURCE\\sound\\sound_manager.c",
				0x683,
				TEST_FLAG(sound->flags, _sound_delayed_bit) ||
					sound_class_get(
						sound_definition_get(
							sound->definition_index)->sound_class)->cache_miss_mode ==
						_sound_cache_miss_mode_postpone,
				"TEST_FLAG(sound->flags, _sound_delayed_bit) || sound_class_get(sound_definition_get(sound->definition_index)->class_index)->cache_miss_mode==_sound_cache_miss_mode_postpone");
		}

		sound_index = data_next_index(sound_data, sound_index);
	}

	return;
}

void sound_idle(
	void)
{
	sound_manager_globals.idling = TRUE;

	if (sound_manager_globals.initialized && sound_manager_globals.enabled)
	{
		sound_manager_globals.platform_definition->begin_scene();

		if (!sound_manager_globals.paused)
		{
			sound_update_time();
			update_channels();
		}

		sound_manager_globals.platform_definition->end_scene();
	}

	sound_cache_idle();
	sound_manager_globals.idling = FALSE;

	return;
}

/* port: the listener's velocity, for the Doppler effect: how fast the
camera's focus moves through the world (the head in first person, the
vehicle a chase camera circles, as the game's own velocity was). Where the
camera follows a unit, the game's velocity for the camera's point as if it
were fixed to that unit (object_get_point_velocity), as the sounds on it are
given theirs: the Doppler effect takes the velocities along the line from a
sound to the camera, and two points on one turning body move alike along the
line between them, so they cancel exactly, from a chase camera behind the
vehicle too, and swinging that camera round adds nothing. Measured, a network client's corrections of a vehicle
it rides jerked the focus and the vehicle's own engine with it, and a
Warthog's turret swung the gunner round. Otherwise (a cinematic's camera, a
flying one) measured from one game tick to the next. Not the camera's
position: swinging a chase
camera round a Warthog sweeps it through the air, and the engine rose and
fell. The observer's own velocity is no use for this on the native ports:
the observer runs every frame (port/linux/game/render_interpolation.c) and
divides a tick's motion by a frame's time, so riding a Pelican at 144 frames
a second the camera claimed nearly five times its speed. It also reads zero
while a cinematic's camera is forced along its track. Either way a sound
moving with the camera seemed to come at it, and rose in pitch. A camera
that jumps further in a tick than any vehicle moves (a cut, a teleport) has
no velocity for that tick. */
#define LISTENER_SNAP_DISTANCE 10.f

static struct
{
	boolean valid;
	long game_time;
	real_point3d position;
	real_vector3d velocity;
} listener_motion[MAXIMUM_NUMBER_OF_LOCAL_PLAYERS];

static void listener_measure_velocity(
	short local_player_index,
	real_point3d const *position,
	real_vector3d *velocity)
{
	long game_time = game_time_get();
	long ticks = game_time - listener_motion[local_player_index].game_time;

	if (!listener_motion[local_player_index].valid || ticks < 0)
	{
		listener_motion[local_player_index].valid = TRUE;
		listener_motion[local_player_index].game_time = game_time;
		listener_motion[local_player_index].position = *position;
		listener_motion[local_player_index].velocity = *global_zero_vector3d;
	}
	else if (ticks > 0)
	{
		real_vector3d displacement;

		vector_from_points3d(
			&listener_motion[local_player_index].position,
			position,
			&displacement);

		if (magnitude3d(&displacement) < LISTENER_SNAP_DISTANCE*ticks)
		{
			scale_vector3d(
				&displacement,
				(real)TICKS_PER_SECOND/(real)ticks,
				&listener_motion[local_player_index].velocity);
		}
		else
		{
			listener_motion[local_player_index].velocity = *global_zero_vector3d;
		}

		listener_motion[local_player_index].game_time = game_time;
		listener_motion[local_player_index].position = *position;
	}

	*velocity = listener_motion[local_player_index].velocity;

	return;
}

static void refresh_listener(
	void)
{
	struct platform_sound_listener_properties properties;
	short local_player_index;
	struct sound_listener *listener;

	if (!game_in_progress())
	{
		return;
	}

	for (local_player_index = 0;
		local_player_index < MAXIMUM_NUMBER_OF_LOCAL_PLAYERS;
		local_player_index++)
	{
		listener = listener_get(local_player_index);

		if (local_player_get_player_index(local_player_index) != NONE)
		{
			struct observer_result const *camera =
				observer_get_camera(local_player_index);
			real_vector3d velocity; /* port: listener_measure_velocity */
			boolean underwater;

			match_assert(
				"c:\\halo\\SOURCE\\sound\\sound_manager.c",
				0x4EF,
				camera);
			listener->valid = TRUE;
			underwater = scenario_location_underwater(
				&camera->location,
				&camera->position,
				NULL);

			if (listener->underwater != underwater)
			{
				struct game_globals *game_globals = scenario_get_game_globals();
				struct sound_source splash;

				splash.spatialization_mode = _sound_spatialization_mode_none;
				splash.scale = 1.f;
				splash.gain = 1.f;

				if (underwater)
				{
					if (game_globals->sounds.count > 0)
					{
						long sound_index = TAG_BLOCK_GET_ELEMENT(
							&game_globals->sounds,
							0,
							struct tag_reference)->index;

						if (sound_index != NONE)
						{
							sound_new_impulse(
								sound_index,
								&splash,
								NONE,
								NULL,
								NULL,
								0);
						}
					}
				}
				else if (game_globals->sounds.count > 1)
				{
					long sound_index = TAG_BLOCK_GET_ELEMENT(
						&game_globals->sounds,
						1,
						struct tag_reference)->index;

					if (sound_index != NONE)
					{
						sound_new_impulse(
							sound_index,
							&splash,
							NONE,
							NULL,
							NULL,
							0);
					}
				}
			}

			listener->underwater = underwater;
			matrix4x3_from_point_and_vectors(
				&listener->matrix,
				&camera->position,
				&camera->forward,
				&camera->up);
			/* port: not the observer's velocity (camera->velocity) but the
			game's for the camera's point as if fixed to the unit it follows,
			or, with none, the focus's motion measured
			(listener_measure_velocity) */
			{
				long followed_unit_index = director_get_followed_unit(local_player_index);

				if (followed_unit_index != NONE && object_try_and_get(followed_unit_index))
				{
					object_get_point_velocity(
						followed_unit_index,
						&camera->position,
						&velocity);
					scale_vector3d(&velocity, (real)TICKS_PER_SECOND, &velocity);
					listener_motion[local_player_index].valid = FALSE;
				}
				else
				{
					listener_measure_velocity(
						local_player_index,
						observer_get_focus_position(local_player_index),
						&velocity);
				}
			}
			matrix4x3_inverse_transform_vector(
				&listener->matrix,
				&velocity,
				&listener->velocity);
		}
		else
		{
			listener->valid = FALSE;
			/* port: measured again from nothing once there is a camera */
			listener_motion[local_player_index].valid = FALSE;
		}
	}

	properties.forward = *global_forward3d;
	properties.up = *global_up3d;
	properties.position = *global_origin3d;
	properties.velocity = *global_zero_vector3d;
	properties.environment = &sound_manager_globals.sound_environment;
	sound_manager_globals.platform_definition->set_listener_properties(
		&properties);

	return;
}

static void detail_sound_random_offset(
	struct looping_sound_detail const *detail,
	real_vector3d *offset)
{
	real distance = real_local_random_range(
		detail->distance_bounds.lower,
		detail->distance_bounds.upper);

	if (distance != 0.f)
	{
		real_euler_angles2d angles;

		angles.pitch = real_local_random_range(
			detail->phi_bounds.lower,
			detail->phi_bounds.upper);
		angles.yaw = real_local_random_range(
			detail->theta_bounds.lower,
			detail->theta_bounds.upper);
		vector3d_from_euler_angles2d(offset, &angles);
		scale_vector3d(offset, distance, offset);
	}
	else
	{
		*offset = *global_zero_vector3d;
	}

	return;
}

void sound_render(
	void)
{
	profile_enter(sound_render_section);

	if (sound_manager_globals.initialized && sound_manager_globals.enabled)
	{
		sound_manager_globals.platform_definition->begin_scene();

		if (!sound_manager_globals.paused)
		{
			long render_time = system_milliseconds();

			sound_manager_globals.ticks_elapsed =
				((real)render_time - sound_manager_globals.render_time) *
				0.029999999f;
			sound_manager_globals.render_time = render_time;
			/* Sounds are rendered once a frame, and a frame is well under a
			tick on the native builds, so the ticks truncate to none and
			scripted sound class fades would never move: carry the
			fraction over. */
			{
				static real leftover_ticks = 0.f;
				long ticks;

				leftover_ticks += sound_manager_globals.ticks_elapsed;
				ticks = (long)leftover_ticks;
				leftover_ticks -= (real)ticks;
				sound_classes_update(ticks);
			}
			refresh_listener();
			process_looping_sounds();
			refresh_sounds();
			prioritize_sounds();
			update_channels();
			sound_manager_globals.flip_flop = !sound_manager_globals.flip_flop;
		}

		sound_manager_globals.platform_definition->end_scene();
	}

	if (!sound_manager_globals.paused)
	{
		sound_cache_idle();
	}

	profile_exit(sound_render_section);

	return;
}
