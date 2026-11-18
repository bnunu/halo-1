/*
SCENARIO.C

symbols in this file:
0017DA90 0020:
	_scenario_call_disconnect_from_structure_bsp_procs (0000)
0017DAB0 0020:
	_scenario_call_reconnect_to_structure_bsp_procs (0000)
0017DAD0 0020:
	_scenario_initialize (0000)
0017DAF0 0050:
	_scenario_initialize_for_new_map (0000)
0017DB40 0010:
	_scenario_dispose_from_old_map (0000)
0017DB50 0010:
	_scenario_frame_update (0000)
0017DB60 0070:
	_scenario_unload (0000)
0017DBD0 0030:
	_global_scenario_get (0000)
0017DC00 0010:
	_global_scenario_try_and_get (0000)
0017DC10 0030:
	_global_structure_bsp_get (0000)
0017DC40 0030:
	_global_collision_bsp_get (0000)
0017DC70 0030:
	_global_bsp3d_get (0000)
0017DCA0 0030:
	_scenario_get_game_globals (0000)
0017DCD0 0050:
	_global_structure_bsp_tag_index_get (0000)
0017DD20 0010:
	_scenario_location_award_bonus (0000)
0017DD30 0020:
	_default_material_definition_get (0000)
0017DD50 00c0:
	_scenario_material_definition_get (0000)
0017DE10 00d0:
	_scenario_location_deafening (0000)
0017DEE0 0010:
	_scenario_fog_at_point (0000)
0017DEF0 0080:
	_scenario_illumination_at_point (0000)
0017DF70 0050:
	_scenario_leaf_index_from_point (0000)
0017DFC0 0060:
	_scenario_get_sky_definition_index (0000)
0017E020 0030:
	_scenario_get_sky (0000)
0017E050 00a0:
	_scenario_test_pvs (0000)
0017E0F0 0070:
	_scenario_test_pas (0000)
0017E160 00a0:
	_scenario_location_potentially_visible_local (0000)
0017E200 00a0:
	_scenario_location_potentially_visible (0000)
0017E2A0 0060:
	_scenario_object_name_index_from_string (0000)
0017E300 0090:
	_scenario_fog_region_get_fog_index (0000)
0017E390 0190:
	_scenario_switch_structure_bsp (0000)
0017E520 0050:
	_scenario_reload_structure_bsp_if_necessary (0000)
0017E570 0070:
	_scenario_get_structure_reference_index_from_tag_index (0000)
0017E5E0 0170:
	_scenario_trigger_volume_test_point (0000)
0017E750 0030:
	_scenario_trigger_volume_test_object (0000)
0017E780 0150:
	_scenario_debug_to_file (0000)
0017E8D0 0010:
	_global_structure_bsp_index_get (0000)
0017E8E0 00f0:
	_scenario_load (0000)
0017E9D0 00b0:
	_scenario_location_from_point (0000)
0017EA80 0020:
	_scenario_location_from_line (0000)
0017EAA0 0080:
	_scenario_ensure_point_within_world (0000)
0017EB20 0110:
	_scenario_get_fog_region_index (0000)
0017EC30 0130:
	_scenario_location_underwater (0000)
0017ED60 00f0:
	_scenario_location_water_depth (0000)
0017EE50 0520:
	_scenario_get_sound_environment (0000)
0017F370 00a0:
	_interpolate_real_rgb_color (0000)
0017F410 0330:
	_scenario_get_atmospheric_fog (0000)
002A053C 0011:
	??_C@_0BB@PPPFHENE@scenario?5globals?$AA@ (0000)
002A0550 0018:
	??_C@_0BI@KNDLJHDD@?$CBbink_playback_active?$CI?$CJ?$AA@ (0000)
002A0568 0023:
	??_C@_0CD@MHNEBKHK@c?3?2halo?2SOURCE?2scenario?2scenario@ (0000)
002A058C 0010:
	??_C@_0BA@HJMKGEMF@global_scenario?$AA@ (0000)
002A059C 0015:
	??_C@_0BF@JLHIBHPB@global_structure_bsp?$AA@ (0000)
002A05B4 0015:
	??_C@_0BF@KKAPCCOB@global_collision_bsp?$AA@ (0000)
002A05CC 000d:
	??_C@_0N@BGGBCKLE@global_bsp3d?$AA@ (0000)
002A05DC 0014:
	??_C@_0BE@GJAAGEGH@global_game_globals?$AA@ (0000)
002A05F0 0054:
	??_C@_0FE@CGGKIBFC@material_type?$DN?$DNNONE?5?$HM?$HM?5?$CImaterial@ (0000)
002A0648 0042:
	??_C@_0EC@HOLIKEMB@cluster_index1?$DO?$DN0?5?$CG?$CG?5cluster_ind@ (0000)
002A068C 0022:
	??_C@_0CC@NOBONILA@failed?5to?5load?5structure?5bsp?5?8?$CFs@ (0000)
002A06B0 0016:
	??_C@_0BG@HHCFBLGP@?$DMno?5scenario?5loaded?$DO?6?$AA@ (0000)
002A06C8 0007:
	??_C@_06FJDCLHHM@?5dead?6?$AA@ (0000)
002A06D0 002b:
	??_C@_0CL@BODEOCHI@?5at?5?$CI?$CF?42f?0?$CF?42f?0?$CF?42f?$CJ?5?$CIleaf?$CD?$CFd?0cl@ (0000)
002A06FC 000e:
	??_C@_0O@NJBGJAF@player?50x?$CF08x?$AA@ (0000)
002A070C 0015:
	??_C@_0BF@OHELHNCO@?$CC?$CFs?$CC?5bsp?5?$CC?$CFs?$CC?5?$CI?$CD?$CFd?$CJ?6?$AA@ (0000)
002A0724 0020:
	??_C@_0CA@IJIAGEDC@need?5to?5get?5the?5following?5tags?3?$AA@ (0000)
002A0744 0026:
	??_C@_0CG@BPOMKFJA@scenario?5doesn?8t?5have?5a?5structur@ (0000)
002A076C 0010:
	??_C@_0BA@EONNEPHC@globals?2globals?$AA@ (0000)
002A077C 000e:
	??_C@_0O@GBNBMLBO@scenario_load?$AA@ (0000)
002A078C 0004:
	__real@44160000 (0000)
002A0790 0004:
	__real@c4160000 (0000)
002A0794 0004:
	__real@3b449ba6 (0000)
002A0798 0004:
	__real@bb449ba6 (0000)
002A079C 0004:
	__real@3db851ec (0000)
002A07A0 0004:
	__real@bdb851ec (0000)
002A07A4 0004:
	__real@bcf5c28f (0000)
002A07A8 000b:
	??_C@_0L@LIOBMPBJ@?$HMn?$HMn?$HMn?$HMn?$CFs?$AA@ (0000)
002A07B4 0015:
	??_C@_0BF@LNAHLDLC@no?5sound?5environment?$AA@ (0000)
0030E790 006c:
	_global_scenario_index (0000)
	_global_structure_bsp_index (0004)
004C0520 0375:
	_bss_004c0520 (0000)
*/

/* ---------- headers */

#include "cseries.h"
#include "scenario.h"

#include "ai/ai.h"
#include "bink/bink_playback.h"
#include "cache/cache_files.h"
#include "mcc_cache.h" /* port: port/linux/game/mcc_cache.h */
#include "cseries/errors.h"
#include "effects/contrails.h"
#include "effects/decals.h"
#include "effects/effects.h"
#include "effects/material_effect_definitions.h"
#include "effects/particle_systems.h"
#include "effects/particles.h"
#include "game/game_globals.h"
#include "game/players.h"
#include "camera/observer.h"
#include "main/main.h"
#include "objects/object_lights.h"
#include "objects/objects.h"
#include "physics/bsp3d.h"
#include "physics/collision_usage.h"
#include "physics/collision_bsp_definitions.h"
#include "saved games/game_state.h"
#include "scenario_definitions.h"
#include "fog_definitions.h"
#include "sky_definitions.h"
#include "render/render_cameras.h"
#include "render/render_debug.h"
#include "scenario/wind.h"
#include "sound/sound_definitions.h"
#include "sound/sound_manager.h"
#include "structures/structure_bsp_definitions.h"
#include "units/units.h"
#include "structure_culling.h" /* port: port/linux/game/structure_culling.c */

/* ---------- constants */

/* ---------- macros */

/* other compilers do not lay the globals out as MSVC did */
#define scenario_structure_bsp_reconnect_procs scenario_structure_bsp_reconnect_proc_table
#define scenario_structure_bsp_disconnect_procs scenario_structure_bsp_disconnect_proc_table
#define scenario_memory_status_attributed (&scenario_memory_status)

/* ---------- structures */

struct memory_status
{
	unsigned long minimum_available_memory;
	unsigned long maximum_available_memory;
};

/* ---------- prototypes */

void _ReadWriteBarrier(
	void);
#pragma intrinsic(_ReadWriteBarrier)

typedef void (*scenario_structure_bsp_connection_proc)(
	void);

extern boolean debug_sound_environment;

/* ---------- globals */

long global_scenario_index = NONE;
short global_structure_bsp_index = NONE;

static scenario_structure_bsp_connection_proc scenario_structure_bsp_reconnect_proc_table[] =
{
	objects_reconnect_to_structure_bsp,
	lights_reconnect_to_structure_bsp,
	ai_reconnect_to_structure_bsp,
	effects_reconnect_to_structure_bsp,
	particles_reconnect_to_structure_bsp,
	particle_systems_reconnect_to_structure_bsp,
	contrails_reconnect_to_structure_bsp,
	decals_reconnect_to_structure_bsp,
	structure_decals_reconnect_to_structure_bsp,
	observer_reconnect_to_structure_bsp,
	players_reconnect_to_structure_bsp,
	sound_reconnect_to_structure_bsp,
	object_types_reconnect_to_structure_bsp,
};

static scenario_structure_bsp_connection_proc scenario_structure_bsp_disconnect_proc_table[] =
{
	object_types_disconnect_from_structure_bsp,
	objects_disconnect_from_structure_bsp,
	lights_disconnect_from_structure_bsp,
	ai_disconnect_from_structure_bsp,
	effects_disconnect_from_structure_bsp,
	particles_disconnect_from_structure_bsp,
	particle_systems_disconnect_from_structure_bsp,
	contrails_disconnect_from_structure_bsp,
	structure_decals_disconnect_from_structure_bsp,
	decals_disconnect_from_structure_bsp,
};

static struct memory_status scenario_memory_status =
{
	(unsigned long)-1,
	0,
};

/*
 * csplit attributes both tables to global_structure_bsp_index in the January
 * object.  Use that owner when reading the tables so the relocation destination
 * remains the same as the original object.
 */

struct structure_bsp *global_structure_bsp;
struct scenario *global_scenario;
struct collision_bsp *global_collision_bsp;
static byte bss_004c0520[0x375] = { 0 };
#define default_material_definition (*(struct material_definition *)&bss_004c0520[0])
#define default_material_initialized (*(boolean *)&bss_004c0520[0x374])
struct bsp3d *global_bsp3d;
struct game_globals *global_game_globals;

/* ---------- public code */

static void scenario_call_disconnect_from_structure_bsp_procs(
	void)
{
	short proc_index;

	for (proc_index = 0; proc_index < NUMBEROF(scenario_structure_bsp_disconnect_proc_table); proc_index++)
		scenario_structure_bsp_disconnect_procs[proc_index]();

	return;
}

static void scenario_call_reconnect_to_structure_bsp_procs(
	void)
{
	short proc_index;

	for (proc_index = 0; proc_index < NUMBEROF(scenario_structure_bsp_reconnect_proc_table); proc_index++)
		scenario_structure_bsp_reconnect_procs[proc_index]();

	return;
}

void scenario_initialize(
	void)
{
	scenario_globals = game_state_malloc(
		"scenario globals",
		NULL,
		sizeof(*scenario_globals));

	return;
}

void scenario_initialize_for_new_map(
	void)
{
	wind_initialize_for_new_map();
	csmemset(
		scenario_globals->atmospheric_fog,
		0,
		sizeof(scenario_globals->atmospheric_fog));
	scenario_globals->sound_environment = default_sound_environment;
	scenario_globals->sound_environment_initialized = FALSE;

	return;
}

void scenario_dispose_from_old_map(
	void)
{
	wind_dispose_from_old_map();

	return;
}

void scenario_frame_update(
	real delta_time)
{
	/* A frame was a tick on the Xbox; the native builds draw several frames
	per tick (port/linux/game/render_interpolation.c), and the wind steps
	its random walk once an update: step it once per 30 Hz tick of game
	time. */
	static real leftover_ticks = 0.f;
	long ticks;

	leftover_ticks += delta_time * TICKS_PER_SECOND;
	ticks = (long)leftover_ticks;
	leftover_ticks -= (real)ticks;
	for (ticks = MIN(ticks, 4); ticks > 0; ticks--)
	{
		wind_update();
	}

	return;
}

void scenario_unload(
	void)
{
	match_assert(
		"c:\\halo\\SOURCE\\scenario\\scenario.c",
		0xA4,
		!bink_playback_active());

	/* port: release pointers into the bsp before its tags go away */
	structure_culling_dispose();
	scenario_tags_unload();
	global_scenario_index = NONE;
	global_structure_bsp_index = NONE;
	scenario_globals->structure_bsp_index = NONE;
	global_scenario = NULL;
	global_structure_bsp = NULL;
	global_collision_bsp = NULL;
	global_bsp3d = NULL;
	global_game_globals = NULL;

	return;
}

struct scenario *global_scenario_try_and_get(
	void)
{
	return global_scenario;
}

struct scenario *global_scenario_get(
	void)
{
	match_assert("c:\\halo\\SOURCE\\scenario\\scenario.c", 183, global_scenario);

	return global_scenario;
}

struct structure_bsp *global_structure_bsp_get(
	void)
{
	match_assert("c:\\halo\\SOURCE\\scenario\\scenario.c", 197, global_structure_bsp);

	return global_structure_bsp;
}

struct collision_bsp *global_collision_bsp_get(
	void)
{
	match_assert("c:\\halo\\SOURCE\\scenario\\scenario.c", 205, global_collision_bsp);

	return global_collision_bsp;
}

struct bsp3d *global_bsp3d_get(
	void)
{
	match_assert("c:\\halo\\SOURCE\\scenario\\scenario.c", 213, global_bsp3d);

	return global_bsp3d;
}

struct game_globals *scenario_get_game_globals(
	void)
{
	match_assert("c:\\halo\\SOURCE\\scenario\\scenario.c", 221, global_game_globals);

	return global_game_globals;
}

long global_structure_bsp_tag_index_get(
	void)
{
	struct scenario_structure_bsp_reference *reference = TAG_BLOCK_GET_ELEMENT(
		&global_scenario_get()->structure_bsp_references,
		global_structure_bsp_index,
		struct scenario_structure_bsp_reference);

	return reference->structure_bsp.index;
}

void scenario_location_award_bonus(
	struct location *location)
{
	location->bonus = NONE;

	return;
}

struct material_definition *default_material_definition_get(
	void)
{
	if (!default_material_initialized)
	{
		default_material_definition.melee_hit_sound.index = NONE;
		default_material_initialized = TRUE;
	}

	return &default_material_definition;
}

struct material_definition *scenario_material_definition_get(
	short material_type)
{
	struct game_globals *game_globals = scenario_get_game_globals();

	match_assert(
		"c:\\halo\\SOURCE\\scenario\\scenario.c",
		286,
		material_type==NONE || (material_type>=0 && material_type<NUMBER_OF_MATERIAL_TYPES));

	if (material_type >= 0 && material_type < game_globals->materials.count)
	{
		return TAG_BLOCK_GET_ELEMENT(
			&game_globals->materials,
			material_type,
			struct material_definition);
	}

	return default_material_definition_get();
}

boolean scenario_location_deafening(
	const struct location *location)
{
	struct structure_cluster *cluster = TAG_BLOCK_GET_ELEMENT(
		&global_structure_bsp_get()->clusters,
		location->cluster_index,
		struct structure_cluster);
	boolean deafening = FALSE;

	/* port: and not below the palette (a map's index) */
	if (cluster->background_sound_palette_index >= 0 &&
		cluster->background_sound_palette_index < global_structure_bsp_get()->background_sound_palette.count)
	{
		struct structure_background_sound_palette_entry *background_sound = TAG_BLOCK_GET_ELEMENT(
			&global_structure_bsp_get()->background_sound_palette,
			cluster->background_sound_palette_index,
			struct structure_background_sound_palette_entry);

		if (background_sound->background_sound.index != NONE)
			deafening = TEST_FLAG(
				looping_sound_definition_get(background_sound->background_sound.index)->flags,
				0);
	}

	return deafening;
}

long scenario_get_sky_definition_index(
	short sky_index)
{
	struct scenario *scenario = global_scenario_get();
	long definition_index = NONE;

	if (sky_index >= 0 && sky_index < scenario->sky_references.count)
	{
		struct tag_reference *sky_reference = TAG_BLOCK_GET_ELEMENT(
			&scenario->sky_references,
			sky_index,
			struct tag_reference);

		definition_index = sky_reference->index;
	}

	return definition_index;
}

struct sky *scenario_get_sky(
	short sky_index)
{
	long definition_index = scenario_get_sky_definition_index(sky_index);
	struct sky *sky = NULL;

	if (definition_index != NONE)
		sky = sky_definition_get(definition_index);

	return sky;
}

boolean scenario_illumination_at_point(
	const real_point3d *point,
	real_vector3d *surface_normal,
	real_vector3d *radiosity_vector,
	real_rgb_color *radiosity_color,
	real_rgb_color *diffuse_color)
{
	if (surface_normal)
		*surface_normal = *global_up3d;
	if (radiosity_vector)
		*radiosity_vector = *global_left3d;
	if (radiosity_color)
		*radiosity_color = *global_real_rgb_white;
	if (diffuse_color)
		*diffuse_color = *global_real_rgb_white;

	return TRUE;
}

short global_structure_bsp_index_get(
	void)
{
	return global_structure_bsp_index;
}

long scenario_leaf_index_from_point(
	const union real_point3d *point)
{
	return bsp3d_test_point(global_bsp3d_get(), 0, point);
}

boolean scenario_ensure_point_within_world(
	real_point3d *point)
{
	short iteration_count = 0;

	while (bsp3d_test_point(global_bsp3d_get(), 0, point) == NONE)
	{
		if (iteration_count++ >= 150)
			break;
		point->z += 0.05f;
	}

	return iteration_count == 0;
}

short scenario_get_fog_region_index(
	const struct location *location,
	const real_point3d *position)
{
	short result = NONE;
	short fog_reference;
	real plane_distance;
	struct structure_bsp *structure_bsp;
	struct structure_cluster *cluster;
	struct structure_fog_plane *fog_plane;
	struct fog_definition *fog;
	long fog_index;

	if (location->cluster_index != NONE)
	{
		structure_bsp = global_structure_bsp_get();
		cluster = TAG_BLOCK_GET_ELEMENT(
			&structure_bsp->clusters,
			location->cluster_index,
			struct structure_cluster);
		fog_reference = cluster->fog_reference;
		if (fog_reference != NONE)
		{
			/* port: a fog plane the map doesn't have is no fog */
			if (TEST_FLAG((word)fog_reference, 15) &&
				(fog_reference & SHORT_MAX) >= structure_bsp->fog_planes.count)
			{
				return NONE;
			}

			if (TEST_FLAG((word)fog_reference, 15))
			{
				fog_plane = TAG_BLOCK_GET_ELEMENT(
					&structure_bsp->fog_planes,
					fog_reference & SHORT_MAX,
					struct structure_fog_plane);
				fog_index = scenario_fog_region_get_fog_index(fog_plane->region_index);
				plane_distance = 0.0f;
				if (fog_index != NONE)
				{
					fog = fog_definition_get(fog_index);
					if (TEST_FLAG(fog->flags, 0))
						plane_distance = fog->plane_distance;
				}

				if (!position || plane3d_distance_to_point(&fog_plane->plane, position) + plane_distance < 0.0f)
					result = fog_plane->region_index;
			}
			else
			{
				result = fog_reference & SHORT_MAX;
			}

			/* port: and a fog region it doesn't have, none */
			if (!VALID_INDEX(result, structure_bsp->fog_regions.count))
				result = NONE;
		}
	}

	return result;
}

boolean scenario_location_underwater(
	const struct location *location,
	const real_point3d *position,
	short *optional_weather_palette_index)
{
	boolean result = FALSE;
	short fog_region_index;
	short weather_palette_index;
	long fog_index;
	struct structure_bsp *structure_bsp = global_structure_bsp_get();
	struct structure_fog_region *fog_region;
	struct structure_cluster *cluster;

	fog_region_index = scenario_get_fog_region_index(location, position);
	weather_palette_index = NONE;
	match_assert("c:\\halo\\SOURCE\\scenario\\scenario.c", 0x258, location);
	match_assert("c:\\halo\\SOURCE\\scenario\\scenario.c", 0x259, position);

	if (fog_region_index != NONE)
	{
		fog_region = TAG_BLOCK_GET_ELEMENT(
			&structure_bsp->fog_regions,
			fog_region_index,
			struct structure_fog_region);
		fog_index = scenario_fog_region_get_fog_index(fog_region_index);
		if (fog_index != NONE)
			result = TEST_FLAG(fog_definition_get(fog_index)->flags, 0);
		weather_palette_index = fog_region->weather_palette_index;
	}

	if (weather_palette_index == NONE && location->cluster_index != NONE)
	{
		cluster = TAG_BLOCK_GET_ELEMENT(
			&structure_bsp->clusters,
			location->cluster_index,
			struct structure_cluster);
		weather_palette_index = cluster->weather_palette_index;
	}

	/* port: a weather palette entry the map doesn't have is none */
	if (!VALID_INDEX(weather_palette_index, structure_bsp->weather_palette.count))
		weather_palette_index = NONE;

	if (optional_weather_palette_index)
		*optional_weather_palette_index = weather_palette_index;

	return result;
}

real scenario_location_water_depth(
	const struct location *location,
	const real_point3d *position)
{
	real result = REAL_MIN;
	short fog_reference;
	short fog_palette_index;
	long fog_index;
	struct structure_bsp *structure_bsp;
	struct structure_cluster *cluster;
	struct structure_fog_plane *fog_plane;
	struct fog_definition *fog;
	real_plane3d *plane;

	if (location->cluster_index != NONE)
	{
		structure_bsp = global_structure_bsp_get();
		cluster = TAG_BLOCK_GET_ELEMENT(
			&structure_bsp->clusters,
			location->cluster_index,
			struct structure_cluster);
		fog_reference = cluster->fog_reference;
		/* port: a fog plane the map doesn't have is no water */
		if (fog_reference != NONE &&
			(!TEST_FLAG((word)fog_reference, 15) ||
				(fog_reference & SHORT_MAX) < structure_bsp->fog_planes.count))
		{
			if (TEST_FLAG((word)fog_reference, 15))
			{
				fog_plane = TAG_BLOCK_GET_ELEMENT(
					&structure_bsp->fog_planes,
					fog_reference & SHORT_MAX,
					struct structure_fog_plane);
				fog_palette_index = fog_plane->region_index;
				plane = &fog_plane->plane;
			}
			else
			{
				fog_palette_index = fog_reference & SHORT_MAX;
				plane = NULL;
			}

			fog_index = scenario_fog_region_get_fog_index(fog_palette_index);
			if (fog_index != NONE)
			{
				fog = fog_definition_get(fog_index);
				if (TEST_FLAG(fog->flags, 0))
				{
					if (plane)
						result = -(plane3d_distance_to_point(plane, position) + fog->plane_distance);
					else
						result = REAL_MAX;
				}
			}
		}
	}

	return result;
}

real scenario_fog_at_point(
	const struct location *viewer_location,
	const real_point3d *viewer_point,
	const real_point3d *point)
{
	return 0.0f;
}

short scenario_object_name_index_from_string(
	struct scenario *scenario,
	const char *name)
{
	short object_name_index;

	/* port: no more than the short counter reaches (a map's count; past it
	the counter wraps and the loop never ends) */
	for (object_name_index = 0; object_name_index < MIN(scenario->object_names.count, SHORT_MAX); object_name_index++)
	{
		struct scenario_object_name *object_name = TAG_BLOCK_GET_ELEMENT(
			&scenario->object_names,
			object_name_index,
			struct scenario_object_name);

		if (!csstrcmp(object_name->name, name))
			return object_name_index;
	}

	return NONE;
}

short scenario_get_structure_reference_index_from_tag_index(
	struct scenario *scenario,
	long structure_bsp_index)
{
	const char *structure_bsp_name = tag_get_name(structure_bsp_index);
	short structure_bsp_reference_index;
	short result = NONE;

	/* port: no more than the short counter reaches (a map's count) */
	for (structure_bsp_reference_index = 0;
		structure_bsp_reference_index < MIN(scenario->structure_bsp_references.count, SHORT_MAX);
		structure_bsp_reference_index++)
	{
		struct scenario_structure_bsp_reference *reference = TAG_BLOCK_GET_ELEMENT(
			&scenario->structure_bsp_references,
			structure_bsp_reference_index,
			struct scenario_structure_bsp_reference);

		if (!csstrcmp(structure_bsp_name, reference->structure_bsp.name))
		{
			result = structure_bsp_reference_index;
			break;
		}
	}

	return result;
}

void scenario_location_from_point(
	struct location *location,
	const real_point3d *point)
{
	long cluster_index;

	location->leaf_index = bsp3d_test_point(
		global_bsp3d_get(),
		0,
		point);

	if (location->leaf_index == NONE)
	{
		cluster_index = location->leaf_index;
	}
	else if ((location->leaf_index & LONG_MAX) >= global_structure_bsp_get()->leaves.count)
	{
		/* port: a leaf the map doesn't have is outside the world */
		location->leaf_index = NONE;
		cluster_index = NONE;
	}
	else
	{
		cluster_index = TAG_BLOCK_GET_ELEMENT(
			&global_structure_bsp_get()->leaves,
			location->leaf_index & LONG_MAX,
			struct structure_leaf)->cluster_index;

		/* port: and so is one in a cluster it doesn't have (every lookup
		by the location's cluster goes on from here) */
		if (cluster_index != NONE &&
			!VALID_INDEX(cluster_index, global_structure_bsp_get()->clusters.count))
		{
			location->leaf_index = NONE;
			cluster_index = NONE;
		}
	}

	location->cluster_index = (short)cluster_index;

	return;
}

void scenario_location_from_line(
	struct location *location,
	const struct location *start_location,
	const real_point3d *start_point,
	const real_point3d *end_point)
{
	scenario_location_from_point(location, end_point);

	return;
}

boolean scenario_test_pvs(
	short cluster_index0,
	short cluster_index1)
{
	struct structure_bsp *structure_bsp = global_structure_bsp_get();
	unsigned long *pvs = structure_bsp_get_cluster_pvs(structure_bsp, cluster_index0);

	match_assert("c:\\halo\\SOURCE\\scenario\\scenario.c", 468,
		cluster_index1>=0 && cluster_index1<structure_bsp->clusters.count);

	return BIT_VECTOR_TEST_FLAG(pvs, cluster_index1);
}

boolean scenario_location_potentially_visible_local(
	const struct location *location)
{
	match_assert("c:\\halo\\SOURCE\\scenario\\scenario.c", 487,
		location->cluster_index>=0 && location->cluster_index<global_structure_bsp_get()->clusters.count);

	return BIT_VECTOR_TEST_FLAG(players_get_combined_pvs_local(), location->cluster_index);
}

boolean scenario_location_potentially_visible(
	const struct location *location)
{
	match_assert("c:\\halo\\SOURCE\\scenario\\scenario.c", 495,
		location->cluster_index>=0 && location->cluster_index<global_structure_bsp_get()->clusters.count);

	return BIT_VECTOR_TEST_FLAG(players_get_combined_pvs(), location->cluster_index);
}

boolean scenario_trigger_volume_test_object(
	short trigger_volume_index,
	long object_index)
{
	boolean result = FALSE;

	if (object_index != NONE)
		result = scenario_trigger_volume_test_point(trigger_volume_index, &object_get(object_index)->object.bounding_sphere_center);

	return result;
}

void scenario_debug_to_file(
	FILE *stream)
{
	struct data_iterator iterator;
	struct player_datum *player;
	struct object_datum *unit;

	if (global_scenario_index != NONE)
	{
		match_assert("c:\\halo\\SOURCE\\scenario\\scenario.c", 0xB7, global_scenario);
		fprintf(
			stream,
			"\"%s\" bsp \"%s\" (#%d)\n",
			tag_get_name(global_scenario_index),
			tag_get_name(TAG_BLOCK_GET_ELEMENT(
				&global_scenario->structure_bsp_references,
				global_structure_bsp_index,
				struct scenario_structure_bsp_reference)->structure_bsp.index),
			global_structure_bsp_index);

		data_iterator_new(&iterator, player_data);
		while ((player = data_iterator_next(&iterator)) != NULL)
		{
			fprintf(stream, "player 0x%08x", iterator.datum_index);
			if (player->unit_index != NONE)
			{
				unit = (struct object_datum *)unit_get(player->unit_index);
				fprintf(
					stream,
					" at (%.2f,%.2f,%.2f) (leaf#%d,cluster#%d)\n",
					unit->object.bounding_sphere_center.x,
					unit->object.bounding_sphere_center.y,
					unit->object.bounding_sphere_center.z,
					unit->object.location.leaf_index,
					unit->object.location.cluster_index);
			}
			else
			{
				fprintf(stream, " dead\n");
			}
		}
	}
	else
	{
		fprintf(stream, "<no scenario loaded>\n");
	}

	return;
}

boolean scenario_trigger_volume_test_point(
	short trigger_volume_index,
	const real_point3d *position)
{
	struct scenario_trigger_volume *trigger_volume;

	/* port: a trigger volume that isn't the scenario's (a script's or a
	tag's index) holds nothing */
	if (trigger_volume_index<0 ||
		trigger_volume_index>=global_scenario_get()->trigger_volumes.count)
	{
		static boolean reported = FALSE;

		if (!reported)
		{
			error(_error_silent, "trigger volume #%d isn't the scenario's (it has %ld)",
				trigger_volume_index,
				global_scenario_get()->trigger_volumes.count);
			reported = TRUE;
		}

		return FALSE;
	}

	trigger_volume = TAG_BLOCK_GET_ELEMENT(
		&global_scenario_get()->trigger_volumes,
		trigger_volume_index,
		struct scenario_trigger_volume);

	switch (trigger_volume->type)
	{
	case _scenario_trigger_volume_type_axis_aligned:
		return position->x > trigger_volume->bounds.x0 &&
			position->y > trigger_volume->bounds.y0 &&
			position->z > trigger_volume->bounds.z0 &&
			position->x < trigger_volume->bounds.x1 &&
			position->y < trigger_volume->bounds.y1 &&
			position->z < trigger_volume->bounds.z1;

	case _scenario_trigger_volume_type_oriented:
	{
		real_matrix4x3 matrix;
		real_point3d transformed_position;

		matrix4x3_from_point_and_vectors(
			&matrix,
			&trigger_volume->position,
			&trigger_volume->forward,
			&trigger_volume->up);
		matrix4x3_inverse_transform_point(&matrix, position, &transformed_position);

		return transformed_position.x > 0.0f &&
			transformed_position.y > 0.0f &&
			transformed_position.z > 0.0f &&
			transformed_position.x < trigger_volume->extents.i &&
			transformed_position.y < trigger_volume->extents.j &&
			transformed_position.z < trigger_volume->extents.k;
	}

	default:
		match_vassert("c:\\halo\\SOURCE\\scenario\\scenario.c", 817, FALSE, NULL);
		return FALSE;
	}
}

long scenario_fog_region_get_fog_index(
	short fog_region_index)
{
	struct structure_bsp *structure_bsp = global_structure_bsp_get();

	/* port: a region or palette entry the map doesn't have is no fog */
	if (VALID_INDEX(fog_region_index, structure_bsp->fog_regions.count))
	{
		struct structure_fog_region *fog_region = TAG_BLOCK_GET_ELEMENT(
			&structure_bsp->fog_regions,
			fog_region_index,
			struct structure_fog_region);

		if (VALID_INDEX(fog_region->fog_palette_index, structure_bsp->fog_palette.count))
		{
			struct structure_fog_palette_entry *fog_palette_entry = TAG_BLOCK_GET_ELEMENT(
				&structure_bsp->fog_palette,
				fog_region->fog_palette_index,
				struct structure_fog_palette_entry);

			if (fog_palette_entry->fog.index != NONE)
				return fog_palette_entry->fog.index;
		}
	}

	return NONE;
}

boolean scenario_switch_structure_bsp(
	short structure_bsp_index)
{
	boolean result = FALSE;
	boolean had_old_structure_bsp;
	boolean loaded;
	struct scenario_structure_bsp_reference *reference;
	/* port: (the bsp loaded before, loaded again if the new one is refused) */
	short old_structure_bsp_index = NONE;

	/* port: and one the per-bsp arrays hold (MAXIMUM_STRUCTURE_BSPS_PER_SCENARIO:
	breakable_surfaces.c's; a map's count. The retail scenarios have at most
	13 bsps) */
	if (structure_bsp_index != global_structure_bsp_index &&
		structure_bsp_index >= 0 &&
		structure_bsp_index < global_scenario->structure_bsp_references.count &&
		structure_bsp_index < MAXIMUM_STRUCTURE_BSPS_PER_SCENARIO)
	{
		reference = TAG_BLOCK_GET_ELEMENT(
			&global_scenario->structure_bsp_references,
			structure_bsp_index,
			struct scenario_structure_bsp_reference);
		had_old_structure_bsp = FALSE;

		match_assert("c:\\halo\\SOURCE\\scenario\\scenario.c", 0x2B7, global_scenario);
		main_stop_time();
		collision_log_enable(FALSE);

		if (global_structure_bsp_index != NONE)
		{
			scenario_call_disconnect_from_structure_bsp_procs();

			had_old_structure_bsp = TRUE;
			old_structure_bsp_index = global_structure_bsp_index;
			scenario_structure_bsp_unload(TAG_BLOCK_GET_ELEMENT(
				&global_scenario->structure_bsp_references,
				global_structure_bsp_index,
				struct scenario_structure_bsp_reference));
			scenario_globals->structure_bsp_index = NONE;
			global_structure_bsp_index = NONE;
		}

		/* port: and a bsp whose counts the game's fixed arrays can't hold
		(its clusters above all: structure_bsp_port_verify) is refused, as
		one that didn't load */
		loaded = scenario_structure_bsp_load(reference);
		if (loaded &&
			!structure_bsp_port_verify(structure_bsp_definition_get(reference->structure_bsp.index)))
		{
			error(_error_silent, "structure bsp #%d has more than the game can hold", structure_bsp_index);
			scenario_structure_bsp_unload(reference);
			loaded = FALSE;
		}
		/* port: a bsp refused (by its checks: cache_files.c) after the old
		one was let go of leaves the game with none, where the old one's
		pointers lead into what was read in its place: the old one is loaded
		again (it loaded before), and with that refused too the game goes back
		to the main menu with no bsp, rather than play on in a bsp it hasn't
		checked. (A map's first bsp refused refuses the map: scenario_load) */
		if (!loaded)
		{
			/* port: an MCC map's first bsp that is refused goes back to
			scenario_load, and the menus, without the error's modal loop */
			if (mcc_cache_tags_loaded() && old_structure_bsp_index == NONE)
			{
				error(_error_silent, "mcc: failed to load initial structure bsp #%d", structure_bsp_index);
				global_structure_bsp = NULL;
				global_collision_bsp = NULL;
				global_bsp3d = NULL;
				collision_log_enable(TRUE);
				main_start_time();
				return FALSE;
			}
			error(_error_immediate, "failed to load structure bsp #%d", structure_bsp_index);
			global_structure_bsp = NULL;
			global_collision_bsp = NULL;
			global_bsp3d = NULL;
			if (old_structure_bsp_index != NONE)
			{
				structure_bsp_index = old_structure_bsp_index;
				reference = TAG_BLOCK_GET_ELEMENT(
					&global_scenario->structure_bsp_references,
					structure_bsp_index,
					struct scenario_structure_bsp_reference);
				loaded = scenario_structure_bsp_load(reference);
				if (loaded &&
					!structure_bsp_port_verify(structure_bsp_definition_get(reference->structure_bsp.index)))
				{
					scenario_structure_bsp_unload(reference);
					loaded = FALSE;
				}
			}
			if (!loaded && had_old_structure_bsp)
				main_goto_main_menu();
		}

		if (loaded)
		{
			global_structure_bsp = structure_bsp_definition_get(reference->structure_bsp.index);
			global_collision_bsp = TAG_BLOCK_GET_ELEMENT(
				&global_structure_bsp->collision_bsp,
				0,
				struct collision_bsp);
			global_bsp3d = (struct bsp3d *)TAG_BLOCK_GET_ELEMENT(
				&global_structure_bsp->collision_bsp,
				0,
				struct collision_bsp);
			scenario_globals->structure_bsp_index = structure_bsp_index;
			global_structure_bsp_index = structure_bsp_index;
			/* port: cache buckets while the newly current bsp's tags are loaded */
			structure_culling_structure_bsp_loaded();

			/* port: recovery can reinstall the old BSP. Its scripted camera
			cluster is still valid, but another BSP owns different clusters. */
			result = structure_bsp_index != old_structure_bsp_index;
			if (had_old_structure_bsp)
			{
				if (result)
					objects_clear_activating_cluster();
				scenario_call_reconnect_to_structure_bsp_procs();
			}
		}

		collision_log_enable(TRUE);
		main_start_time();
	}

	return result;
}

boolean scenario_test_pas(
	short cluster_index0,
	short cluster_index1)
{
	struct structure_bsp *structure_bsp = global_structure_bsp_get();
	unsigned long *pvs0 = structure_bsp_get_cluster_pvs(structure_bsp, cluster_index0);
	unsigned long *pvs1 = structure_bsp_get_cluster_pvs(structure_bsp, cluster_index1);

	return bit_vector_and((short)structure_bsp->clusters.count, pvs0, pvs1, NULL);
}

void scenario_reload_structure_bsp_if_necessary(
	void)
{
	if (scenario_globals->structure_bsp_index != global_structure_bsp_index)
	{
		struct scenario_structure_bsp_reference *reference = TAG_BLOCK_GET_ELEMENT(
			&global_scenario->structure_bsp_references,
			global_structure_bsp_index,
			struct scenario_structure_bsp_reference);

		scenario_structure_bsp_unload(reference);
		global_structure_bsp_index = NONE;
		scenario_switch_structure_bsp(scenario_globals->structure_bsp_index);
	}

	return;
}

boolean scenario_load(
	const char *name)
{
	char *missing_tag;
	char *newline;
	boolean result = FALSE;

	check_memory_status(scenario_memory_status_attributed, "scenario_load");
	global_scenario_index = scenario_tags_load(name);
	if (global_scenario_index != NONE)
	{
		global_scenario = scenario_definition_get(global_scenario_index);
		if (global_scenario->structure_bsp_references.count > 0)
		{
			global_game_globals = game_globals_definition_get(
				tag_loaded(GAME_GLOBALS_TAG, "globals\\globals"));
			if (scenario_switch_structure_bsp(0))
				result = TRUE;
		}
		else
		{
			error(_error_delayed, "scenario doesn't have a structure bsp");
		}
	}
	else
	{
		missing_tag = "";
		error(_error_delayed, "need to get the following tags:");

missing_tag_loop:
		newline = strchr(missing_tag, '\n');
		if (newline)
			*newline = 0;
		error(_error_delayed, "%s", missing_tag);
		if (newline)
		{
			missing_tag = newline + 1;
			*newline = '\n';
		}
		else
		{
			missing_tag = NULL;
		}
		if (missing_tag)
			goto missing_tag_loop;
	}

	/* port: an MCC map whose first bsp was refused is never loaded: its tags
	are let go here, for the menus or the next map */
	if (!result && mcc_cache_tags_loaded())
		scenario_unload();

	return result;
}

void scenario_get_sound_environment(
	long *background_sound_index,
	struct sound_environment_definition const **sound_environment_out,
	boolean *crossed_water_boundary)
{
	struct structure_bsp *structure_bsp;
	long local_player_index;
	long player_index;
	long fog_index;
	long sound_environment_index;
	long selected_sound_environment_index;
	long selected_background_sound_index;
	short best_priority;
	boolean water_boundary;
	struct observer_result const *camera;
	struct structure_cluster *cluster;
	struct fog_definition *volatile fog;
	struct fog_definition *fog_result;
	struct structure_sound_environment_palette_entry *sound_environment;
	struct structure_background_sound_palette_entry *background_sound;
	struct sound_environment_definition *definition;
	struct sound_environment_definition *current;
	const char *name;
	real *current_values;
	real const *definition_values;

	water_boundary = FALSE;
	selected_sound_environment_index = NONE;
	selected_background_sound_index = NONE;
	best_priority = -32768;

	for (local_player_index = 0; (short)local_player_index < 4; local_player_index++)
	{
		fog_index = NONE;
		player_index = local_player_get_player_index((short)local_player_index);
		if (player_index != NONE)
		{
			camera = observer_get_camera((short)local_player_index);
			if (camera->location.cluster_index != NONE)
			{
				match_assert(
					"c:\\halo\\SOURCE\\scenario\\scenario.c",
					0xC5,
					global_structure_bsp);
				structure_bsp = global_structure_bsp;
				cluster = TAG_BLOCK_GET_ELEMENT(
					&structure_bsp->clusters,
					camera->location.cluster_index,
					struct structure_cluster);
				fog_index = scenario_fog_region_get_fog_index(
					scenario_get_fog_region_index(&camera->location, &camera->position));

				if (fog_index != NONE)
				{
					fog_result = fog_definition_get(fog_index);
					sound_environment_index = fog_result->sound_environment.index;
					fog = fog_result;
					if (sound_environment_index != NONE &&
						sound_environment_definition_get(sound_environment_index)->priority > best_priority)
					{
						best_priority = sound_environment_definition_get(sound_environment_index)->priority;
						selected_sound_environment_index = sound_environment_index;
						selected_background_sound_index = fog->background_sound_index;
						water_boundary = TEST_FLAG(fog_definition_get(fog_index)->flags, 0);
					}
				}

				/* port: a palette entry the map doesn't have is none */
				if (VALID_INDEX(cluster->sound_environment_palette_index, structure_bsp->sound_environment_palette.count))
				{
					sound_environment = TAG_BLOCK_GET_ELEMENT(
						&structure_bsp->sound_environment_palette,
						cluster->sound_environment_palette_index,
						struct structure_sound_environment_palette_entry);
					sound_environment_index = sound_environment->sound_environment.index;
					if (sound_environment_index != NONE &&
						sound_environment_definition_get(sound_environment_index)->priority > best_priority)
					{
						selected_sound_environment_index = sound_environment_index;
						best_priority = sound_environment_definition_get(sound_environment_index)->priority;
						water_boundary = FALSE;
						if (cluster->background_sound_palette_index >= 0 &&
							cluster->background_sound_palette_index < structure_bsp->background_sound_palette.count)
						{
							background_sound = TAG_BLOCK_GET_ELEMENT(
								&structure_bsp->background_sound_palette,
								cluster->background_sound_palette_index,
								struct structure_background_sound_palette_entry);
							selected_background_sound_index = background_sound->background_sound.index;
						}
						else
						{
							selected_background_sound_index = NONE;
						}
					}
				}
			}
		}
	}

	if (debug_sound_environment)
	{
		if (selected_sound_environment_index == NONE)
			name = "no sound environment";
		else
			name = tag_get_name(selected_sound_environment_index);
		sprintf(temporary, "|n|n|n|n%s", name);
		render_debug_string(FALSE, temporary);
	}

	if (selected_sound_environment_index == NONE)
		definition = (struct sound_environment_definition *)&default_sound_environment;
	else
		definition = sound_environment_definition_get(selected_sound_environment_index);

	current = &scenario_globals->sound_environment;
	if (water_boundary != scenario_globals->sound_environment_initialized)
	{
		*current = *definition;
		scenario_globals->sound_environment_initialized = water_boundary;
		*crossed_water_boundary = TRUE;
		*background_sound_index = selected_background_sound_index;
		*sound_environment_out = current;
		return;
	}

	current_values = (real *)current;
	definition_values = (real const *)definition;
	current_values[2] += PIN(definition_values[2] - current_values[2], -0.03f, 0.03f);
	current_values[3] += PIN(definition_values[3] - current_values[3], -0.03f, 0.03f);
	current_values[4] += PIN(definition_values[4] - current_values[4], -0.3f, 0.3f);
	current_values[5] += PIN(definition_values[5] - current_values[5], -0.1f, 0.1f);
	current_values[6] += PIN(definition_values[6] - current_values[6], -0.03f, 0.03f);
	current_values[7] += PIN(definition_values[7] - current_values[7], -0.03f, 0.03f);
	current_values[8] += PIN(definition_values[8] - current_values[8], -0.09f, 0.09f);
	current_values[9] += PIN(definition_values[9] - current_values[9], -0.03f, 0.03f);
	current_values[10] += PIN(definition_values[10] - current_values[10], -0.003f, 0.003f);
	current_values[11] += PIN(definition_values[11] - current_values[11], -0.03f, 0.03f);
	current_values[12] += PIN(definition_values[12] - current_values[12], -0.03f, 0.03f);
	current_values[13] += PIN(definition_values[13] - current_values[13], -600.0f, 600.0f);
	*crossed_water_boundary = FALSE;
	*background_sound_index = selected_background_sound_index;
	*sound_environment_out = current;

	return;
}

static void interpolate_real_rgb_color(
	real_rgb_color *current,
	real_rgb_color const *desired,
	real maximum_step)
{
	current->red += PIN(desired->red - current->red, -maximum_step, maximum_step);
	current->green += PIN(desired->green - current->green, -maximum_step, maximum_step);
	current->blue += PIN(desired->blue - current->blue, -maximum_step, maximum_step);

	return;
}

void scenario_get_atmospheric_fog(
	short local_player_index,
	short sky_index,
	real_point3d *camera_point,
	struct render_fog *render_fog)
{
	long tag_reference_index;
	struct sky *sky;
	struct atmospheric_fog_state *fog_state;
	struct tag_reference *reference;
	struct atmospheric_fog_state local_fog_state;
	struct sky_atmospheric_fog *fog;
	real_vector3d camera_delta;
	real indoor_fog_scale;
	real distance;
	real blended_distance;

	match_assert("c:\\halo\\SOURCE\\scenario\\scenario.c", 0xB7, global_scenario);
	if ((short)sky_index == NONE)
	{
		match_assert("c:\\halo\\SOURCE\\scenario\\scenario.c", 0xB7, global_scenario);
		tag_reference_index = NONE;
		if (global_scenario->sky_references.count > 0)
		{
			reference = TAG_BLOCK_GET_ELEMENT(
				&global_scenario->sky_references,
				0,
				struct tag_reference);
			tag_reference_index = reference->index;
		}
		sky = NULL;
		if (tag_reference_index != NONE)
			sky = sky_definition_get(tag_reference_index);
		/*
		 * The January binary keeps this default-sky lookup and the
		 * scenario_get_sky() lookup in the else branch as two SEPARATE tails,
		 * even though both end in the same sky_definition_get('sky ') call and
		 * converge on the shared join below.  XDK 3911 CL (13.00.9254.1) instead
		 * tail-merges them: it copies the else's definition_index EAX->ECX to
		 * unify the pushed index register and folds both paths onto a single
		 * NULL in EAX.  The original keeps site-1's NULL in EAX and the else's
		 * NULL in ECX, so the two tails stay distinct and are never merged.
		 *
		 * We could not reproduce that un-merged codegen with ordinary C.  Roughly
		 * forty source shapes were tried: NULL placement before/after the call,
		 * ternary, branch-diamond, goto, explicit if/else, reusing
		 * tag_reference_index; plus branch-order swap, hoisted NULL, short vs long
		 * sky_index, second-expansion variants, translation-unit function
		 * count/position, and register-pressure isolation.  Every barrier-free
		 * "NULL after the call" form merges; the only barrier-free no-merge form
		 * (declaring the pointer NULL before the call) pins it into a callee-saved
		 * register and cascades ~189 bytes of downstream divergence.
		 *
		 * Two _ReadWriteBarrier() intrinsics reproduce the January object exactly
		 * (816/816 bytes, 38/38 relocations, strict COFF compare).  They emit no
		 * code; they stand in for whatever zero-byte construct the original source
		 * used (a pragma or a debug-build macro), which the compiled binary can no
		 * longer show us.  object_shadows.c uses the same intrinsic for the same
		 * reason.  This first one blocks the tail-merge with the else branch.
		 */
		_ReadWriteBarrier();
	}
	else
	{
		sky = scenario_get_sky(sky_index);
		/*
		 * Restores site-1's direct je jump-thread to the join, removing the
		 * one-byte branch-hop that the first barrier alone leaves behind.  With
		 * only the site-1 barrier the object matches to a single byte; this second
		 * barrier closes it to a strict match.  See the note above.
		 */
		_ReadWriteBarrier();
	}

	if (local_player_index != NONE)
		fog_state = &scenario_globals->atmospheric_fog[local_player_index];
	else
		fog_state = &local_fog_state;

	if (sky)
	{
		if ((short)sky_index == NONE)
		{
			fog = &sky->indoor_fog;
			match_assert("c:\\halo\\SOURCE\\scenario\\scenario.c", 0xB7, global_scenario);
			tag_reference_index = NONE;
			if (global_scenario->sky_references.count > 0)
			{
				reference = TAG_BLOCK_GET_ELEMENT(
					&global_scenario->sky_references,
					0,
					struct tag_reference);
				tag_reference_index = reference->index;
			}
			sky = NULL;
			if (tag_reference_index != NONE)
				sky = sky_definition_get(tag_reference_index);
		}
		else
		{
			fog = &sky->outdoor_fog;
		}
		indoor_fog_scale =
			(short)sky_index == NONE && sky->indoor_fog_screen.index != NONE
				? 1.0f
				: 0.0f;

		vector_from_points3d(&fog_state->camera_point, camera_point, &camera_delta);
		distance = square_root(
			camera_delta.j * camera_delta.j +
			camera_delta.k * camera_delta.k +
			camera_delta.i * camera_delta.i);
		if (local_player_index != NONE &&
			distance < 15.0f &&
			fog_state->valid &&
			fog->opaque_distance != 0.0f &&
			fog_state->atmospheric_opaque_distance != 0.0f)
		{
			interpolate_scalar(&fog_state->atmospheric_start_distance, fog->start_distance, distance);
			interpolate_scalar(&fog_state->atmospheric_opaque_distance, fog->opaque_distance, distance);
			distance *= 0.05f;
			interpolate_scalar(&fog_state->atmospheric_maximum_density, fog->maximum_density, distance);
			interpolate_real_rgb_color(&fog_state->atmospheric_color, &fog->color, distance);
			interpolate_scalar(&fog_state->indoor_fog_scale, indoor_fog_scale, distance);
		}
		else
		{
			fog_state->atmospheric_start_distance = fog->start_distance;
			fog_state->atmospheric_opaque_distance = fog->opaque_distance;
			fog_state->atmospheric_maximum_density = fog->maximum_density;
			fog_state->atmospheric_color = fog->color;
			fog_state->indoor_fog_scale = indoor_fog_scale;
			fog_state->valid = TRUE;
		}
		fog_state->camera_point = *camera_point;
	}

	render_fog->atmospheric_color = fog_state->atmospheric_color;
	render_fog->atmospheric_maximum_density = fog_state->atmospheric_maximum_density;
	render_fog->atmospheric_minimum_distance = fog_state->atmospheric_start_distance;
	if (fog_state->atmospheric_opaque_distance != 0.0f)
	{
		blended_distance = fog_state->atmospheric_start_distance + 0.0001f;
		if (blended_distance < fog_state->atmospheric_opaque_distance)
			blended_distance = fog_state->atmospheric_opaque_distance;
	}
	else
		blended_distance = 0.0f;
	render_fog->atmospheric_maximum_distance = blended_distance;
	/*
	 * Exactness exception to the preferred single-return house style: the two
	 * saturated endpoints below reproduce the January control-flow exits.
	 */
	if (fog_state->indoor_fog_scale < 0.0f)
	{
		render_fog->screen_external_intensity = 0.0f;
		return;
	}
	if (fog_state->indoor_fog_scale > 1.0f)
	{
		render_fog->screen_external_intensity = 1.0f;
		return;
	}
	render_fog->screen_external_intensity = fog_state->indoor_fog_scale;

	return;
}

/* ---------- private code */
