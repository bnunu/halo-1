/*
ACTORS.C

symbols in this file:
00028FA0 00c0:
	_actors_initialize (0000)
00029060 0010:
	_actors_dispose (0000)
00029070 0030:
	_actors_initialize_for_new_map (0000)
000290A0 0030:
	_actors_dispose_from_old_map (0000)
000290D0 00c0:
	_actor_set_team (0000)
00029190 00a0:
	_actor_activation_debug_color (0000)
00029230 0080:
	_code_00029230 (0000)
000292B0 00e0:
	_actor_verify_activation (0000)
00029390 00e0:
	_actor_detach_from_unit (0000)
00029470 01d0:
	_actor_swarm_detach_from_unit (0000)
00029640 0080:
	_actor_swarm_cache_delete (0000)
000296C0 0050:
	_code_000296c0 (0000)
00029710 0020:
	_actor_is_noncombat (0000)
00029730 0030:
	_actor_in_combat (0000)
00029760 0040:
	_actor_is_fighting (0000)
000297A0 00b0:
	_actor_attacking_target (0000)
00029850 0030:
	_actor_is_leaping (0000)
00029880 00b0:
	_actor_get_weapon (0000)
00029930 0060:
	_actor_has_ranged_weapon (0000)
00029990 0040:
	_actor_target_unit_index (0000)
000299D0 0050:
	_actor_derive_target_information (0000)
00029A20 01d0:
	_actor_switch_props (0000)
00029BF0 0050:
	_actor_flush_position_indices (0000)
00029C40 00c0:
	_actor_flush_structure_indices (0000)
00029D00 0010:
	_actor_has_unlimited_grenades (0000)
00029D10 00e0:
	_actor_handle_damage (0000)
00029DF0 0080:
	_code_00029df0 (0000)
00029E70 00a0:
	_code_00029e70 (0000)
00029F10 0040:
	_actors_freeze (0000)
00029F50 00c0:
	_code_00029f50 (0000)
0002A010 00a0:
	_actors_move_randomly (0000)
0002A0B0 0070:
	_actor_change_encounter (0000)
0002A120 0030:
	_code_0002a120 (0000)
0002A150 0010:
	_code_0002a150 (0000)
0002A160 00a0:
	_code_0002a160 (0000)
0002A200 00a0:
	_code_0002a200 (0000)
0002A2A0 00c0:
	_actor_find_pathfinding_location (0000)
0002A360 0090:
	_actor_destination_tolerance (0000)
0002A3F0 0070:
	_actor_input_sample_position (0000)
0002A460 0040:
	_code_0002a460 (0000)
0002A4A0 0230:
	_code_0002a4a0 (0000)
0002A6D0 0100:
	_actors_handle_spatial_effect (0000)
0002A7D0 00a0:
	_actor_handle_communication (0000)
0002A870 0040:
	_actor_unit_control_crouch (0000)
0002A8B0 0030:
	_actor_unit_control_jump (0000)
0002A8E0 0060:
	_actor_unit_control_primary_trigger (0000)
0002A940 0040:
	_actor_unit_control_secondary_trigger (0000)
0002A980 0030:
	_actor_unit_control_throw_grenade (0000)
0002A9B0 0040:
	_actor_unit_control_exact_facing (0000)
0002A9F0 0030:
	_actor_unit_control_stop_animation_impulse (0000)
0002AA20 03b0:
	_actor_new (0000)
0002ADD0 0280:
	_actor_customize_unit (0000)
0002B050 0110:
	_actor_set_dormant (0000)
0002B160 0070:
	_code_0002b160 (0000)
0002B1D0 0050:
	_actor_delete_props (0000)
0002B220 0120:
	_actor_delete (0000)
0002B340 0110:
	_actor_swarm_cache_new (0000)
0002B450 00d0:
	_actor_get_running_blind_vector (0000)
0002B520 00e0:
	_actor_kill (0000)
0002B600 0340:
	_actor_died (0000)
0002B940 00a0:
	_actor_swarm_unit_died (0000)
0002B9E0 0060:
	_actor_braindead (0000)
0002BA40 01c0:
	_actor_handle_unit_effect (0000)
0002BC00 00d0:
	_actor_set_active (0000)
0002BCD0 0290:
	_actor_swarm_attach_unit (0000)
0002BF60 00a0:
	_actor_erase (0000)
0002C000 0230:
	_code_0002c000 (0000)
0002C230 0950:
	_code_0002c230 (0000)
0002CB80 0230:
	_actors_handle_unit_effect (0000)
0002CDB0 0310:
	_code_0002cdb0 (0000)
0002D0C0 01d0:
	_actor_attach_unit (0000)
0002D290 0140:
	_code_0002d290 (0000)
0002D3D0 0270:
	_actor_create_for_unit (0000)
0002D640 0320:
	_actor_place (0000)
0002D960 02a0:
	_actors_spawn_from_unit (0000)
0002DC00 0080:
	_actors_update (0000)
00246834 000a:
	_global_movement_animation_states (0000)
00246840 0015:
	??_C@_0BF@IKFFBLEB@swarm_component_data?$AA@ (0000)
00246858 0010:
	??_C@_0BA@EHGCJOFL@swarm?5component?$AA@ (0000)
00246868 000b:
	??_C@_0L@JFMPKLIM@swarm_data?$AA@ (0000)
00246874 0006:
	??_C@_05IMMFCEFO@swarm?$AA@ (0000)
0024687C 000b:
	??_C@_0L@NNGPDIPH@actor_data?$AA@ (0000)
00246888 001b:
	??_C@_0BL@IFEJIDPD@c?3?2halo?2SOURCE?2ai?2actors?4c?$AA@ (0000)
002468A4 0006:
	??_C@_05OJNICPJM@actor?$AA@ (0000)
002468AC 001f:
	??_C@_0BP@DJKNNBPO@?$CFs?5unit?5activation?5logic?5error?$AA@ (0000)
002468CC 000b:
	??_C@_0L@KOKJAGEJ@individual?$AA@ (0000)
002468D8 000f:
	??_C@_0P@KCHBJAML@inactive?5swarm?$AA@ (0000)
002468E8 000d:
	??_C@_0N@JNMFNIL@active?5swarm?$AA@ (0000)
002468F8 001b:
	??_C@_0BL@FHOFLAAE@actor?5dormancy?5logic?5error?$AA@ (0000)
00246914 0023:
	??_C@_0CD@KACFCMHE@encounter?9?$DOunique_leader_count?5?$DO@ (0000)
00246938 0026:
	??_C@_0CG@JABJGCJL@unit?9?$DOunit?4actor_index?5?$DN?$DN?5actor_@ (0000)
00246960 0006:
	??_C@_05JEMLEKKA@found?$AA@ (0000)
00246968 0032:
	??_C@_0DC@FACDKDBJ@swarm?9?$DOunit_count?5?$DN?$DN?5actor?9?$DOmeta@ (0000)
0024699C 0022:
	??_C@_0CC@KIKJBOEI@swarm?9?$DOactor_index?5?$DN?$DN?5actor_inde@ (0000)
002469C0 000e:
	??_C@_0O@CIOBJKKB@attack_vector?$AA@ (0000)
002469D0 004c:
	??_C@_0EM@LLEBOIJP@AI?5error?5condition?5detected?0?5att@ (0000)
00246A1C 002b:
	??_C@_0CL@CDNDAKNI@actor?9type?5?$CFs?5?$CFs?5internal?5logic?5@ (0000)
00246A48 001d:
	??_C@_0BN@EGBIINPF@?5infinite?5decision?5loop?5?$CI?$CFs?$CJ?$AA@ (0000)
00246A68 0002:
	??_C@_01KMDKNFGN@?1?$AA@ (0000)
00246A6C 000f:
	??_C@_0P@CACMJKNL@actor?9type?5?$CFs?5?$AA@ (0000)
00246A80 0072:
	??_C@_0HC@JKKAFOMD@cannot?5add?5grenades?5or?5non?9power@ (0000)
00246AF4 0036:
	??_C@_0DG@HDAOIBGG@swarm?9?$DOunit_count?5?$DM?5MAXIMUM_NUMB@ (0000)
00246B2C 002b:
	??_C@_0CL@CPOHAIKA@actor_index?5?$CB?$DN?5global_updating_a@ (0000)
00246B58 0034:
	??_C@_0DE@EGNJPMD@unable?5to?5create?5any?5more?5swarm?5@ (0000)
00246B90 0042:
	??_C@_0EC@PMEGKNAL@actor?9?$DOmeta?4swarm_unit_count?5?$DM?$DN?5@ (0000)
00246BD4 002e:
	??_C@_0CO@DAKJODHF@exceeded?5MAXIMUM_NUMBER_OF_ACTIV@ (0000)
00246C04 000b:
	??_C@_0L@IJDNIGGG@run_vector?$AA@ (0000)
00246C10 0025:
	??_C@_0CF@EMPOOLED@actor?9?$DOmeta?4swarm_unit_index?5?$DN?$DN?5@ (0000)
00246C38 0012:
	??_C@_0BC@OPFCJMCE@actor?9?$DOmeta?4swarm?$AA@ (0000)
00246C4C 003c:
	??_C@_0DM@ICAEGPLB@swarm?5tried?5to?5become?5active?5but@ (0000)
00246C88 0035:
	??_C@_0DF@HONDOMG@swarm_first_unit?9?$DOunit?4swarm_pre@ (0000)
00246CC0 0041:
	??_C@_0EB@KMMOKFOD@actor?9?$DOmeta?4swarm_unit_count?5?$DM?5M@ (0000)
00246D04 0025:
	??_C@_0CF@OBGEKIIO@unit?9?$DOunit?4swarm_actor_index?5?$DN?$DN?5@ (0000)
00246D2C 001f:
	??_C@_0BP@PKBDPEJ@unit?9?$DOunit?4actor_index?5?$DN?$DN?5NONE?$AA@ (0000)
00246D4C 001f:
	??_C@_0BP@NOLNBLLP@actor?9?$DOmeta?4unit_index?5?$DN?$DN?5NONE?$AA@ (0000)
00246D70 0046:
	??_C@_0EG@IJNEHKGD@tried?5to?5update?5a?5swarm?5actor?5wi@ (0000)
00246DB8 002c:
	??_C@_0CM@CIODPINB@realcmp?$CIactor?9?$DOinput?4facing_vect@ (0000)
00246DE4 002e:
	??_C@_0CO@NNCPOMJ@?$CIreal_vector2d?5?$CK?$CJ?5?$CGactor?9?$DOinput?4@ (0000)
00246E14 001d:
	??_C@_0BN@LPPNLGMB@?$CGactor?9?$DOinput?4looking_vector?$AA@ (0000)
00246E34 001c:
	??_C@_0BM@FKDFBMGA@?$CGactor?9?$DOinput?4aiming_vector?$AA@ (0000)
00246E50 004d:
	??_C@_0EN@IKDAGGPK@actor?5?$CFs?5is?5not?5flying?5but?5was?5p@ (0000)
00246EA0 004d:
	??_C@_0EN@LFGKGCAN@actor?5?$CFs?5is?5flying?5but?5was?5place@ (0000)
00246EF0 0004:
	__real@42200000 (0000)
00246EF8 007e:
	??_C@_0HO@FHPPIAO@?$CIfabs?$CIcontrol_data?4throttle?4i?$CJ?5?$DM@ (0000)
00246F78 0008:
	__real@3ff0000000000000 (0000)
00246F80 001d:
	??_C@_0BN@KECJEOEE@?$CGcontrol_data?4looking_vector?$AA@ (0000)
00246FA0 001c:
	??_C@_0BM@IPGIOCDF@?$CGcontrol_data?4aiming_vector?$AA@ (0000)
00246FBC 001c:
	??_C@_0BM@FJJHEIIG@?$CGcontrol_data?4facing_vector?$AA@ (0000)
00246FD8 002e:
	??_C@_0CO@NIIPAAJM@?$CIreal_vector2d?5?$CK?$CJ?5?$CGcontrol_data?4@ (0000)
00247008 0044:
	??_C@_0EE@LFBDCPIL@?$CFs?5actor?5variant?5?$CFs?5cannot?5have?5@ (0000)
0024704C 0023:
	??_C@_0CD@BMFCNAIM@WARNING?3?5cannot?5create?5actor?5?$CFs?5@ (0000)
00247070 002c:
	??_C@_0CM@DONKLMPF@WARNING?3?5cannot?5create?5unit?5for?5@ (0000)
0024709C 0010:
	??_C@_0BA@NEIKIEMF@?$DMencounterless?$DO?$AA@ (0000)
002470AC 0027:
	??_C@_0CH@HGDOJFAF@actor_variant_definition_index?5?$CB@ (0000)
002470D4 0012:
	??_C@_0BC@CHJAJEBN@starting_location?$AA@ (0000)
002470E8 0035:
	??_C@_0DF@LOENPPFC@WARNING?3?5cannot?5create?5actor?5to?5@ (0000)
002B6CE8 0004:
	_global_updating_actor_index (0000)
*/

/* ---------- headers */

#include "cseries.h"
#include "mcc_grenades.h" /* port: port/linux/game/mcc_grenades.h */
#include "actors.h"
#include "actor_definitions.h"
#include "actor_placement.h"
#include "actor_types.h"
#include "encounters.h"
#include "props.h"

#include "ai.h"
#include "ai/ai_globals.h"
#include "math/real_math.h"
#include "ai_communication.h"
#include "ai_debug.h"
#include "ai_profile.h"
#include "ai_scenario_definitions.h"
#include "bitmaps/bitmaps.h"
#include "encounters.h"
#include "cseries/errors.h"
#include "game/game.h"
#include "game/game_allegiance.h"
#include "game/players.h"
#include "items/equipment_definitions.h"
#include "items/projectiles.h"
#include "items/weapons.h"
#include "objects/damage.h"
#include "saved games/game_state.h"
#include "scenario/scenario.h"
#include "scenario/scenario_definitions.h"
#include "structures/structures.h"
#include "structures/structure_bsp_definitions.h"
#include "tag_files/tag_groups.h"
#include "units/bipeds.h"
#include "units/unit_control_data.h"
#include "units/unit_definitions.h"
#include "units/units.h"
#include "units/vehicle_definitions.h"
#include "units/vehicles.h"
#include "networking/network_game_globals.h"

/* ---------- constants */

enum
{
	_actor_mode_braindead = 0,
	_actor_mode_alert = 2,
	_actor_mode_combat,
	_actor_fire_target_none = 0,
	_actor_fire_target_prop,
};

enum actor_default_state
{
	actor_default_state_none = 0,
	actor_default_state_asleep,
	actor_default_state_alert,
	actor_default_state_moving_repeat_position,
	actor_default_state_moving_loop,
	actor_default_state_moving_loop_back_and_forth,
	actor_default_state_moving_loop_randomly,
	actor_default_state_moving_randomly,
	actor_default_state_guarding,
	actor_default_state_guarding_at_guard_point,
	actor_default_state_searching,
	actor_default_state_fleeing,
	number_of_actor_default_states,
};

enum
{
	_rgb_color_interpolation_hsv_bit = 0,
	_ai_reference_squad_bit = 15,
};

/* vehicle_definition.flags; the shared vehicle tag header has not yet
 * reconstructed these AI-facing constants. */
enum
{
	_vehicle_ai_weapon_cannot_rotate_bit = 8,
	_vehicle_ai_driver_enable_bit = 11,
	_vehicle_ai_driver_flying_bit,
	_vehicle_ai_driver_nondirectional_bit,
	_vehicle_ai_driver_hovering_bit,
};

/* projectile_datum.flags; projectiles.c currently owns the otherwise private
 * enum, so actors keeps the shared January bit spelling local for now. */
enum
{
	_projectile_will_super_explode_bit = 7,
};

/* January names these limits in its assert and error strings; the shared
header spells them MAXIMUM_SWARMS and MAXIMUM_UNIT_INDICES_PER_SWARM. */
enum
{
	MAXIMUM_NUMBER_OF_ACTIVE_SWARMS = MAXIMUM_SWARMS,
	MAXIMUM_NUMBER_OF_UNITS_PER_SWARM = MAXIMUM_UNIT_INDICES_PER_SWARM,
};

/* decision loop bounds; the history length and iteration cap come from the
 * January error path (five remembered actions, ten passes) */
enum
{
	MAXIMUM_DECISION_LOOP_ITERATIONS = 10,
	NUMBER_OF_DECISION_LOOP_HISTORY_ENTRIES = 5,
};

/* actor state.combat_status levels (actors.h does not yet declare these) */
enum
{
	_actor_combat_status_none = 0,
	_actor_combat_status_wary,
	_actor_combat_status_investigate,
	_actor_combat_status_definite,
	_actor_combat_status_certain,
	_actor_combat_status_clear_los,
	_actor_combat_status_dangerous,
	_actor_combat_status_visible,
	NUMBER_OF_ACTOR_COMBAT_STATUS_LEVELS,
};

/* ai sound volumes (ai.h does not yet declare these) */
enum
{
	_ai_sound_volume_silent = 0,
	_ai_sound_volume_medium,
	_ai_sound_volume_loud,
	_ai_sound_volume_shout,
	_ai_sound_volume_quiet,
	NUMBER_OF_AI_SOUND_VOLUMES,
};

/* ai spatial effect types (ai.h does not yet declare these) */
enum
{
	_ai_spatial_effect_environmental_noise = 0,
	_ai_spatial_effect_weapon_impact,
	_ai_spatial_effect_weapon_detonation,
	NUMBER_OF_AI_SPATIAL_EFFECTS,
};

/* ai unit effect types (ai.h does not yet declare these) */
enum
{
	_ai_unit_effect_bump = 0,
	_ai_unit_effect_shooting,
	_ai_unit_effect_death_scream,
	_ai_unit_effect_magic_sight,
	NUMBER_OF_AI_UNIT_EFFECTS,
};

/* encounter follow-target types (encounters.h does not yet declare these) */
enum
{
	_follow_target_none = 0,
	_follow_target_players,
	_follow_target_unit,
	_follow_target_ai,
	NUMBER_OF_FOLLOW_TARGET_TYPES,
};

/* ai_information_packet.information_type (ai.h does not yet declare these) */
enum
{
	_ai_information_none = 0,
	_ai_information_allegiance,
	_ai_information_combat_stimulus,
	_ai_information_target_knowledge,
	_ai_information_flee,
	NUMBER_OF_AI_INFORMATION_TYPES,
};

/* ---------- macros */

#define prop_orphaned(prop) \
	((prop)->state >= _prop_state_uninspected_orphan && \
		(prop)->state <= _prop_state_inspected_orphan)

/* ---------- structures */

struct actor_iterator
{
	struct data_iterator encounter_iterator;
	boolean iterated_encounterless_list;
	boolean active_only;
	byte pad[2];
	long index;
	long next_index;
};

struct encounter_actor_iterator
{
	long encounter_index;
	long index;
	long next_index;
};

typedef char encounter_actor_iterator_size_assert[
	sizeof(struct encounter_actor_iterator) == 0xC ? 1 : -1];
typedef char encounter_actor_iterator_index_offset_assert[
	offsetof(struct encounter_actor_iterator, index) == 0x4 ? 1 : -1];

typedef char actor_iterator_size_assert[
	sizeof(struct actor_iterator) == 0x1C ? 1 : -1];
typedef char actor_iterator_index_offset_assert[
	offsetof(struct actor_iterator, index) == 0x14 ? 1 : -1];

struct actor_variant_change_colors
{
	real_rgb_color color_lower_bound;
	real_rgb_color color_upper_bound;
	unsigned long unused[2];
};

typedef char actor_variant_change_colors_size_assert[
	sizeof(struct actor_variant_change_colors) == 0x20 ? 1 : -1];

typedef char swarm_component_datum_size_assert[
	sizeof(struct swarm_component_datum) == SWARM_COMPONENT_DATUM_SIZE ? 1 : -1];
typedef char swarm_component_datum_combat_target_prop_offset_assert[
	offsetof(struct swarm_component_datum, combat_target_prop_index) == 0x14 ? 1 : -1];

typedef char actor_datum_output_control_flags_offset_assert[
	offsetof(struct actor_datum, output.control_flags) == 0x6D0 ? 1 : -1];
typedef char actor_datum_output_animation_impulse_offset_assert[
	offsetof(struct actor_datum, output.animation.impulse) == 0x6EC ? 1 : -1];
typedef char actor_datum_meta_encounterless_offset_assert[
	offsetof(struct actor_datum, meta.encounterless) == 0x09 ? 1 : -1];
typedef char actor_datum_meta_encounter_index_offset_assert[
	offsetof(struct actor_datum, meta.encounter_index) == 0x34 ? 1 : -1];
typedef char actor_datum_meta_first_prop_index_offset_assert[
	offsetof(struct actor_datum, meta.first_prop_index) == 0x50 ? 1 : -1];

/* The shared vehicle tag layout is still opaque past the common unit
 * definition (vehicle_definitions.h only forward-declares it).  Actors reads
 * the January-authenticated AI destination radius at 0x384 only, so keep that
 * ownership local to this TU. */
struct vehicle_definition
{
	struct unit_definition unit;
	unsigned long flags;
	byte __unknown2F4[0x90];
	real ai_destination_radius;
};

typedef char vehicle_definition_flags_offset_assert[
	offsetof(struct vehicle_definition, flags) == 0x2F0 ? 1 : -1];
typedef char vehicle_definition_ai_destination_radius_offset_assert[
	offsetof(struct vehicle_definition, ai_destination_radius) == 0x384 ? 1 : -1];

typedef char ai_globals_service_data_time_given_offset_assert[
	offsetof(struct ai_globals, time_given_this_frame) == 0x3 ? 1 : -1];
typedef char ai_globals_service_data_current_highest_offset_assert[
	offsetof(struct ai_globals, current_highest_service_timer) == 0x6 ? 1 : -1];
typedef char ai_globals_service_data_grenades_enabled_offset_assert[
	offsetof(struct ai_globals, grenades_enabled) == 0x3B4 ? 1 : -1];

/* ---------- prototypes */

/* January keeps this function private to ACTORS.C. */
static void actor_input_update(
	long actor_index);

/* ---------- globals */

struct data_array *swarm_data;
struct data_array *swarm_component_data;
struct data_array *actor_data;
long global_updating_actor_index = NONE;

short const global_movement_animation_states[NUMBER_OF_ACTOR_MOVEMENT_TYPES] =
{
	_unit_animation_state_alert,
	_unit_animation_state_asleep,
	_unit_animation_state_in_combat,
	_unit_animation_state_flee,
	_unit_animation_state_flaming,
};

/* ---------- public code */

void actors_initialize(
	void)
{
	actor_types_initialize();
	actor_data = game_state_data_new("actor", MAXIMUM_ACTORS, sizeof(struct actor_datum));
	match_assert("c:\\halo\\SOURCE\\ai\\actors.c", 121, actor_data);
	swarm_data = game_state_data_new("swarm", MAXIMUM_SWARMS, sizeof(struct swarm_datum));
	match_assert("c:\\halo\\SOURCE\\ai\\actors.c", 124, swarm_data);
	swarm_component_data = game_state_data_new(
		"swarm component",
		MAXIMUM_SWARM_COMPONENTS,
		SWARM_COMPONENT_DATUM_SIZE);
	match_assert("c:\\halo\\SOURCE\\ai\\actors.c", 127, swarm_component_data);

	return;
}

void actors_dispose(
	void)
{
	return;
}

void actors_initialize_for_new_map(
	void)
{
	data_make_valid(actor_data);
	data_make_valid(swarm_data);
	data_make_valid(swarm_component_data);

	return;
}

void actors_dispose_from_old_map(
	void)
{
	data_make_invalid(actor_data);
	data_make_invalid(swarm_data);
	data_make_invalid(swarm_component_data);

	return;
}

void actor_set_team(
	long actor_index,
	short team_index)
{
	struct actor_datum *actor = actor_get(actor_index);

	if (actor->meta.swarm)
	{
		if (actor->meta.swarm_cache_index != NONE)
		{
			struct swarm_datum *swarm = swarm_get(actor->meta.swarm_cache_index);
			short unit_index;

			for (unit_index = 0; unit_index < swarm->unit_count; unit_index++)
			{
				struct unit_datum *unit = unit_get(swarm->unit_indices[unit_index]);

				unit->object.owner_team_index = team_index;
			}
		}
		else
		{
			long unit_index = actor->meta.swarm_unit_index;

			while (unit_index != NONE)
			{
				struct unit_datum *unit = unit_get(unit_index);

				unit->object.owner_team_index = team_index;
				unit_index = unit->unit.swarm_next_unit_index;
			}
		}
	}
	else if (actor->meta.unit_index != NONE)
	{
		unit_get(actor->meta.unit_index)->object.owner_team_index = team_index;
	}

	return;
}

real_argb_color const *actor_activation_debug_color(
	long actor_index)
{
	struct actor_datum *actor = actor_get(actor_index);

	if (actor->meta.active)
	{
		if (actor->meta.dormant)
		{
			return global_real_argb_blue;
		}

		if (actor->meta.dormant_desire)
		{
			if (actor->meta.become_dormant_timer > 0)
			{
				return global_real_argb_green;
			}

			return global_real_argb_yellow;
		}

		return global_real_argb_white;
	}

	if (actor->meta.encounter_index != NONE)
	{
		struct encounter_definition *encounter_definition = TAG_BLOCK_GET_ELEMENT(
			&global_scenario_get()->ai_encounters,
			DATUM_INDEX_TO_ABSOLUTE_INDEX(actor->meta.encounter_index),
			struct encounter_definition);

		if (encounter_definition->runtime_structure_bsp_reference_index != NONE &&
			encounter_definition->runtime_structure_bsp_reference_index != global_structure_bsp_index)
		{
			return global_real_argb_red;
		}
	}

	return global_real_argb_purple;
}

static void actor_verify_unit_activation(
	long actor_index,
	long unit_index,
	char const *name)
{
	struct object_header_datum *object_header = object_header_get(unit_index);
	struct unit_datum *unit = unit_get(unit_index);
	struct actor_datum *actor = actor_get(actor_index);

	if (unit->object.parent_object_index == NONE &&
		unit->unit.game_time_at_last_vehicle_exit + 30 < game_time_get())
	{
		if (actor->meta.dormant != !TEST_FLAG(object_header->flags, _object_header_active_bit))
		{
			error(_error_silent, "%s unit activation logic error", name);
		}
	}

	return;
}

void actor_verify_activation(
	long actor_index)
{
	struct actor_datum *actor = actor_get(actor_index);

	if (!actor->meta.active && !actor->meta.dormant)
	{
		error(_error_silent, "actor dormancy logic error");
	}

	if (actor->meta.swarm)
	{
		if (actor->meta.swarm_cache_index != NONE)
		{
			struct swarm_datum *swarm = swarm_get(actor->meta.swarm_cache_index);
			short unit_index;

			for (unit_index = 0; unit_index < swarm->unit_count; unit_index++)
			{
				actor_verify_unit_activation(actor_index, swarm->unit_indices[unit_index], "active swarm");
			}
		}
		else
		{
			long unit_index = actor->meta.swarm_unit_index;

			while (unit_index != NONE)
			{
				struct unit_datum *unit = unit_get(unit_index);

				actor_verify_unit_activation(actor_index, unit_index, "inactive swarm");
				unit_index = unit->unit.swarm_next_unit_index;
			}
		}
	}
	else if (actor->meta.unit_index != NONE)
	{
		actor_verify_unit_activation(actor_index, actor->meta.unit_index, "individual");
	}

	return;
}

void actor_detach_from_unit(
	long actor_index)
{
	struct actor_datum *actor = actor_get(actor_index);

	if (actor->meta.unit_index != NONE)
	{
		struct unit_datum *unit = unit_get(actor->meta.unit_index);

		object_set_automatic_deactivation(actor->meta.unit_index, TRUE);
		unit_set_actively_controlled(actor->meta.unit_index, FALSE);
		match_assert("c:\\halo\\SOURCE\\ai\\actors.c", 1374, unit->unit.actor_index == actor_index);
		unit->unit.actor_index = NONE;

		if (actor->meta.unique_leader && actor->meta.encounter_index != NONE)
		{
			struct encounter_datum *encounter = encounter_get(actor->meta.encounter_index);

			match_assert("c:\\halo\\SOURCE\\ai\\actors.c", 1381, encounter->unique_leader_count > 0);
			encounter->unique_leader_count--;
		}

		actor->meta.unit_index = NONE;
		actor->meta.unique_leader = FALSE;
	}

	return;
}

void actor_swarm_detach_from_unit(
	long actor_index,
	long unit_index)
{
	struct actor_datum *actor = actor_get(actor_index);
	struct unit_datum *unit = unit_get(unit_index);

	if (unit->unit.swarm_actor_index == actor_index)
	{
		match_assert("c:\\halo\\SOURCE\\ai\\actors.c", 1401, actor->meta.swarm_unit_count > 0);
		object_set_automatic_deactivation(unit_index, TRUE);
		unit_set_actively_controlled(unit_index, FALSE);

		if (actor->meta.swarm_cache_index != NONE)
		{
			struct swarm_datum *swarm = swarm_get(actor->meta.swarm_cache_index);
			boolean found = FALSE;
			short unit_slot;

			match_assert("c:\\halo\\SOURCE\\ai\\actors.c", 1413, swarm->actor_index == actor_index);
			match_assert("c:\\halo\\SOURCE\\ai\\actors.c", 1414, swarm->unit_count == actor->meta.swarm_unit_count);

			for (unit_slot = 0; unit_slot < swarm->unit_count; unit_slot++)
			{
				if (swarm->unit_indices[unit_slot] == unit_index)
				{
					long component_index = swarm->component_indices[unit_slot];

					swarm->unit_count--;
					if (unit_slot < swarm->unit_count)
					{
						swarm->unit_indices[unit_slot] = swarm->unit_indices[swarm->unit_count];
						swarm->component_indices[unit_slot] = swarm->component_indices[swarm->unit_count];
					}

					datum_delete(swarm_component_data, component_index);
					found = TRUE;
					break;
				}
			}

			match_assert("c:\\halo\\SOURCE\\ai\\actors.c", 1436, found);
		}

		if (unit->unit.swarm_prev_unit_index != NONE)
		{
			unit_get(unit->unit.swarm_prev_unit_index)->unit.swarm_next_unit_index =
				unit->unit.swarm_next_unit_index;
		}
		else
		{
			actor->meta.swarm_unit_index = unit->unit.swarm_next_unit_index;
		}

		if (unit->unit.swarm_next_unit_index != NONE)
		{
			unit_get(unit->unit.swarm_next_unit_index)->unit.swarm_prev_unit_index =
				unit->unit.swarm_prev_unit_index;
		}

		unit->unit.swarm_actor_index = NONE;
		actor->meta.swarm_unit_count--;
	}

	return;
}

void actor_swarm_cache_delete(
	long actor_index)
{
	struct actor_datum *actor = actor_get(actor_index);

	if (actor->meta.swarm_cache_index != NONE)
	{
		struct swarm_datum *swarm = swarm_get(actor->meta.swarm_cache_index);
		short component_index;

		for (component_index = 0;
			component_index < swarm->unit_count;
			component_index++)
		{
			datum_delete(swarm_component_data, swarm->component_indices[component_index]);
		}

		datum_delete(swarm_data, actor->meta.swarm_cache_index);
		actor->meta.swarm_cache_index = NONE;
	}

	return;
}

static void actor_swarm_component_refresh(
	long unit_index,
	long component_index)
{
	struct unit_datum *unit = unit_get(unit_index);
	struct swarm_component_datum *component = swarm_component_get(component_index);
	long surface_index = NONE;

	if (unit->object.type == _object_type_biped)
	{
		surface_index = ((struct biped_datum *)unit)->biped.support_surface_index;
	}

	object_get_origin(unit_index, &component->position);
	component->surface_index = surface_index;

	return;
}

boolean actor_has_unlimited_grenades(
	long actor_index)
{
	return TRUE;
}

boolean actor_is_noncombat(
	long actor_index)
{
	boolean result = actor_get(actor_index)->state.mode < _actor_mode_combat;

	return result;
}

boolean actor_in_combat(
	long actor_index)
{
	struct actor_datum *actor = actor_get(actor_index);

	if (actor->state.mode == _actor_mode_combat &&
		actor->state.combat_status > actor->state.artificial_combat_status)
	{
		return TRUE;
	}

	return FALSE;
}

boolean actor_is_fighting(
	long actor_index)
{
	struct actor_datum *actor = actor_get(actor_index);
	boolean result = actor->state.combat_status >= _actor_combat_status_visible;

	if (result &&
		actor->state.action == _actor_action_flee &&
		actor->state.action_data.flee.panic_type > 0)
	{
		result = FALSE;
	}

	return result;
}

boolean actor_attacking_target(
	long actor_index,
	real_vector3d *attack_vector)
{
	struct actor_datum *actor = actor_get(actor_index);
	boolean result = FALSE;

	match_assert("c:\\halo\\SOURCE\\ai\\actors.c", 1688, attack_vector);

	if (actor->target.target_type > _actor_target_clear_line_of_sight_enemy &&
		actor_action_class(actor_index) == _action_class_active)
	{
		if (actor->control.grenade_trying_to_throw)
		{
			result = TRUE;
			*attack_vector = actor->input.aiming_vector;
		}
		else if (actor->control.current_fire_target_type > _actor_fire_target_none)
		{
			result = TRUE;
			*attack_vector = actor->control.current_fire_target_aim_vector;
		}
	}

	return result;
}

boolean actor_is_leaping(
	long actor_index)
{
	struct actor_datum *actor = actor_get(actor_index);
	boolean result = FALSE;

	if (actor->state.action == _actor_action_charge)
	{
		result = action_charge_is_leaping(actor_index);
	}

	return result;
}

long actor_get_weapon(
	long actor_index)
{
	struct actor_datum *actor = actor_get(actor_index);
	long result = NONE;

	if (actor->input.vehicle_gunner && actor->input.vehicle_index != NONE)
	{
		struct unit_datum *unit = unit_get(actor->input.vehicle_index);

		result = unit_inventory_get_weapon(
			actor->input.vehicle_index,
			(word)unit->unit.current_weapon_index);
	}

	if (result == NONE && actor->meta.unit_index != NONE)
	{
		struct actor_variant_definition *actor_variant_definition =
			actor_variant_definition_get(actor->meta.variant_definition_index);

		if (!TEST_FLAG(
			actor_variant_definition->flags,
			_actor_variant_definition_cannot_use_ranged_weapons_bit))
		{
			struct unit_datum *unit = unit_get(actor->meta.unit_index);

			result = unit_inventory_get_weapon(
				actor->meta.unit_index,
				unit->unit.current_weapon_index);
		}
	}

	return result;
}

boolean actor_has_ranged_weapon(
	long actor_index)
{
	struct actor_datum *actor = actor_get(actor_index);
	boolean result = actor_get_weapon(actor_index) != NONE;

	if (result && actor->meta.unit_index != NONE)
	{
		struct unit_datum *unit = unit_get(actor->meta.unit_index);

		if (TEST_FLAG(unit->object.damage_flags, _object_ranged_attack_inhibited_bit))
		{
			result = FALSE;
		}
	}

	return result;
}

long actor_target_unit_index(
	long actor_index)
{
	struct actor_datum *actor = actor_get(actor_index);
	long result = NONE;

	if (actor->target.target_prop_index != NONE)
	{
		result = prop_get(actor->target.target_prop_index)->unit_index;
	}

	return result;
}

void actor_derive_target_information(
	long actor_index,
	long target_actor_index)
{
	struct actor_datum *target_actor = actor_get(target_actor_index);
	long target_prop_index = target_actor->target.target_prop_index;

	if (target_prop_index != NONE)
	{
		struct prop_datum *target_prop = prop_get(target_prop_index);

		actor_perception_create_orphan_from_friend(
			actor_index,
			target_prop->unit_index,
			target_actor_index,
			target_prop_index);
	}

	return;
}

void actor_switch_props(
	long actor_index,
	long old_prop_index,
	long new_prop_index)
{
	struct actor_datum *actor = actor_get(actor_index);
	short component_index;

	if (actor->target.target_prop_index == old_prop_index)
	{
		actor->target.target_prop_index = new_prop_index;

		if (new_prop_index == NONE)
			actor->target.target_type = _actor_target_none;
	}

	if (actor->control.current_fire_target_type == _actor_fire_target_prop &&
		actor->control.current_fire_target_prop_index == old_prop_index)
	{
		actor->control.current_fire_target_prop_index = new_prop_index;

		if (new_prop_index == NONE)
			actor->control.current_fire_target_type = _actor_fire_target_none;
	}

	if (actor->control.grenade_current_prop_index == old_prop_index)
		actor->control.grenade_current_prop_index = new_prop_index;

	if (actor->stimuli.surprise_prop_index == old_prop_index)
		actor->stimuli.surprise_prop_index = new_prop_index;

	if (actor->stimuli.panic_prop_index == old_prop_index)
		actor->stimuli.panic_prop_index = new_prop_index;

	if (actor->stimuli.combat_transition_prop_index == old_prop_index)
		actor->stimuli.combat_transition_prop_index = new_prop_index;

	if (actor->emotions.unopposable_retreat_prop_index == old_prop_index)
	{
		if (new_prop_index == NONE)
			actor->emotions.unopposable_retreat_timer = 0;

		actor->emotions.unopposable_retreat_prop_index = new_prop_index;
	}

	if (actor->external_orders.pursuit_group_prop_index == old_prop_index)
		actor->external_orders.pursuit_group_prop_index = new_prop_index;

	if (actor->external_orders.postcombat_prop_index == old_prop_index)
	{
		actor->external_orders.postcombat_prop_index = new_prop_index;

		if (new_prop_index == NONE)
			actor->external_orders.postcombat_type = _actor_postcombat_none;
	}

	if (actor->control.path.destination_orders.destination_type ==
			_destination_prop &&
		actor->control.path.destination_orders.prop.prop_index == old_prop_index)
	{
		if (new_prop_index == NONE)
		{
			actor->control.path.destination_orders.destination_type = _destination_none;
			actor->control.path.destination_orders.ignore_target_object_index = NONE;
		}
		else
		{
			actor->control.path.destination_orders.prop.prop_index = new_prop_index;
		}
	}

	if (actor->control.secondary_look_direction.type == _direction_specification_prop &&
		actor->control.secondary_look_direction.prop_index == old_prop_index)
	{
		actor->control.secondary_look_direction.prop_index = new_prop_index;
	}

	if (actor->control.idle_major_direction.type == _direction_specification_prop &&
		actor->control.idle_major_direction.prop_index == old_prop_index)
	{
		actor->control.idle_major_direction.prop_index = new_prop_index;
	}

	if (actor->control.idle_minor_direction.type == _direction_specification_prop &&
		actor->control.idle_minor_direction.prop_index == old_prop_index)
	{
		actor->control.idle_minor_direction.prop_index = new_prop_index;
	}

	if (actor->meta.swarm && actor->meta.swarm_cache_index != NONE)
	{
		struct swarm_datum *swarm = swarm_get(actor->meta.swarm_cache_index);

		for (component_index = 0;
			component_index < swarm->unit_count;
			component_index++)
		{
			struct swarm_component_datum *component =
				swarm_component_get(
					swarm->component_indices[component_index]);

			if (component->combat_target_prop_index == old_prop_index)
				component->combat_target_prop_index = new_prop_index;
		}
	}

	actor_action_replace_prop(actor_index, old_prop_index, new_prop_index);

	return;
}

void actor_flush_position_indices(
	long actor_index)
{
	struct actor_datum *actor = actor_get(actor_index);

	actor->firing_positions.current_position_index = NONE;

	if (actor->control.path.destination_orders.destination_type == _destination_firing_position ||
		actor->control.path.destination_orders.destination_type == _destination_move_position)
	{
		actor->control.path.destination_orders.destination_type = _destination_none;
		actor->control.path.destination_orders.ignore_target_object_index = NONE;
	}

	actor_action_flush_position_indices(actor_index);

	return;
}

void actor_flush_structure_indices(
	long actor_index)
{
	struct actor_datum *actor = actor_get(actor_index);

	actor->input.position.body_location.cluster_index = NONE;
	actor->input.position.body_location.leaf_index = NONE;
	actor->input.pathfinding_surface_index = NONE;
	actor->stimuli.combat_transition_guard_point_surface_index = NONE;

	if (actor->orders.move.destination.destination_type == _destination_raw_location)
	{
		actor->orders.move.destination.raw.surface_index = NONE;
	}

	if (actor->control.path.destination_orders.destination_type == _destination_raw_location)
	{
		actor->control.path.destination_orders.raw.surface_index = NONE;
	}

	actor->control.path.destination.surface_index = NONE;

	if (actor->meta.swarm && actor->meta.swarm_cache_index != NONE)
	{
		struct swarm_datum *swarm = swarm_get(actor->meta.swarm_cache_index);
		short component_index;

		for (component_index = 0;
			component_index < swarm->unit_count;
			component_index++)
		{
			swarm_component_get(swarm->component_indices[component_index])->surface_index = NONE;
		}
	}

	actor_action_flush_structure_indices(actor_index);

	return;
}

void actor_handle_damage(
	long actor_index,
	long aggressor_unit_index,
	real damage_fraction,
	real_vector3d *damage_velocity)
{
	if (aggressor_unit_index != NONE)
	{
		long prop_index = prop_get_base_by_unit_index(actor_index, aggressor_unit_index, TRUE, TRUE);

		if (prop_index != NONE)
		{
			struct prop_datum *prop = prop_get(prop_index);

			prop->damage_inflicted_on_me += damage_fraction;
			prop->ticks_since_damage = 0;
			prop->currently_damaging_me = TRUE;
			match_assert("c:\\halo\\SOURCE\\ai\\actors.c", 2038, !prop_orphaned(prop));

			if (prop->parent_prop_index != NONE)
			{
				struct prop_datum *parent_prop = prop_get(prop->parent_prop_index);

				parent_prop->damage_inflicted_on_me += damage_fraction;
				parent_prop->ticks_since_damage = 0;
				parent_prop->currently_damaging_me = TRUE;
			}

			if (prop->state < _prop_state_becoming_unacknowledged ||
				prop->state > _prop_state_acknowledged)
			{
				prop_index = NONE;
			}
		}

		actor_stimulus_damage(actor_index, prop_index, damage_fraction, damage_velocity);
	}

	return;
}

static void actor_freeze_unit(
	long actor_index,
	long unit_index)
{
	struct unit_control_data control_data;

	csmemset(&control_data, 0, sizeof(control_data));
	control_data.animation_state = _unit_animation_state_alert;
	control_data.control_flags = 0;
	control_data.throttle = *global_zero_vector3d;
	control_data.aiming_speed = 1;
	control_data.weapon_index = NONE;
	control_data.grenade_index = NONE;
	control_data.zoom_level = NONE;
	unit_get_facing_vector(unit_index, &control_data.facing_vector);
	unit_get_aiming_vector(unit_index, &control_data.aiming_vector);
	unit_get_looking_vector(unit_index, &control_data.looking_vector);
	unit_control(unit_index, &control_data);
	unit_set_actively_controlled(unit_index, FALSE);

	return;
}

static void actor_freeze(
	long actor_index)
{
	struct actor_datum *actor = actor_get(actor_index);

	if (actor->meta.swarm)
	{
		if (actor->meta.swarm_cache_index != NONE)
		{
			struct swarm_datum *swarm = swarm_get(actor->meta.swarm_cache_index);
			short unit_index;

			for (unit_index = 0; unit_index < swarm->unit_count; unit_index++)
			{
				actor_freeze_unit(actor_index, swarm->unit_indices[unit_index]);
			}
		}
	}
	else
	{
		actor_freeze_unit(actor_index, actor->meta.unit_index);
	}

	actor->meta.frozen = TRUE;

	return;
}

void actor_change_encounter(
	long actor_index,
	long encounter_index,
	short squad_index)
{
	struct actor_datum *actor = actor_get(actor_index);

	actor_flush_position_indices(actor_index);

	if (actor->meta.encounterless)
	{
		encounterless_detach_actor(actor_index);
	}
	else if (actor->meta.encounter_index != NONE)
	{
		encounter_detach_actor(actor_index, FALSE);
	}

	if (encounter_index == NONE)
	{
		encounterless_attach_actor(actor_index);
	}
	else
	{
		encounter_attach_actor(actor_index, encounter_index, squad_index, TRUE);
	}

	return;
}

static void actor_update_begin(
	long actor_index)
{
	struct actor_datum *actor = actor_get(actor_index);

	global_updating_actor_index = actor_index;
	if (ai_debug.enter_debugger && ai_debug.selected_actor_index == actor_index)
	{
		ai_debug.enter_debugger = FALSE;
	}

	return;
}

static void actor_update_end(
	void)
{
	global_updating_actor_index = NONE;

	return;
}

static boolean actor_get_timeslice(
	long actor_index)
{
	struct actor_datum *actor = actor_get(actor_index);
	boolean result = FALSE;
	boolean jumping = FALSE;

	if (actor->state.action == _actor_action_charge)
	{
		short goal = actor->state.action_data.charge.goal;

		if (goal == _charge_goal_melee ||
			goal == _charge_goal_melee_leaping ||
			goal == _charge_goal_vehicle_strafing ||
			goal == _charge_goal_vehicle_ramming)
		{
			jumping = TRUE;
		}
	}

	actor->meta.service_timer += jumping ? 3 : 1;

	if (!ai_globals->time_given_this_frame &&
		actor->meta.service_timer > ai_globals->last_highest_service_timer &&
		actor->meta.service_timer > 15)
	{
		actor->meta.service_timer = 0;
		ai_globals->time_given_this_frame = TRUE;
		result = TRUE;
	}
	else if (actor->meta.service_timer > ai_globals->current_highest_service_timer)
	{
		ai_globals->current_highest_service_timer = actor->meta.service_timer;
	}

	actor->meta.timeslice = result;

	return result;
}

static void actor_clear_output(
	long actor_index)
{
	struct actor_datum *actor = actor_get(actor_index);

	actor->output.facing_vector = actor->input.facing_vector;
	actor->output.aiming_vector = actor->input.aiming_vector;
	actor->output.looking_vector = actor->input.looking_vector;
	actor->output.control_flags = 0;
	actor->output.analog_primary_trigger = 0.f;
	actor->output.throttle = *global_zero_vector3d;
	actor->output.animation.impulse = NONE;

	return;
}

void actor_find_pathfinding_location(
	long actor_index)
{
	struct actor_datum *actor = actor_get(actor_index);
	struct actor_definition *actor_definition = actor_definition_get(actor->meta.definition_index);

	if (actor->input.pathfinding_surface_index == NONE)
	{
		actor->input.pathfinding_point = actor->input.position.body_position;

		if (!actor->state.flying)
		{
			if (actor->input.vehicle_index != NONE)
			{
				enum actor_vehicle_driver_type vehicle_driver_type = actor->input.vehicle_driver_type;

				if (vehicle_driver_type >= _actor_vehicle_driver_hovering_ground &&
					vehicle_driver_type <= _actor_vehicle_driver_nondirectional_ground)
				{
					actor->input.pathfinding_surface_index = vehicle_find_pathfinding_surface_index(
						actor->input.vehicle_index,
						&actor->input.pathfinding_point);
				}
			}
			else if (biped_try_and_get(actor->meta.unit_index))
			{
				actor->input.pathfinding_surface_index = biped_find_pathfinding_surface_index(
					actor->meta.unit_index,
					&actor->input.pathfinding_point);
			}
		}
	}

	return;
}

real actor_destination_tolerance(
	long actor_index)
{
	struct actor_datum *actor = actor_get(actor_index);
	real tolerance = 0.5f;

	if (actor->input.vehicle_index != NONE)
	{
		struct unit_datum *vehicle = vehicle_get(actor->input.vehicle_index);
		struct vehicle_definition *vehicle_definition =
			vehicle_specific_definition_get(vehicle->definition_index);

		tolerance = vehicle_definition->ai_destination_radius;
	}

	if (actor->state.action == _actor_action_obey &&
		actor->state.action_data.obey.complex_control.destination_radius_valid)
	{
		tolerance = actor->state.action_data.obey.complex_control.destination_radius;
	}

	if (actor->state.action == _actor_action_vehicle)
	{
		return 0.7f;
	}

	return MAX(tolerance, 0.2f);
}

void actor_input_sample_position(
	long actor_index,
	long unit_index,
	struct actor_position_data *position)
{
	struct unit_datum *unit = unit_get(unit_index);
	long ultimate_parent_index;

	object_get_origin(unit_index, &position->body_position);
	position->body_facing = unit->object.forward;
	unit_get_head_position(unit_index, &position->head_position);
	object_get_velocities(unit_index, &position->velocity, NULL);
	ultimate_parent_index = object_get_ultimate_parent(unit_index);
	position->body_location = object_get(ultimate_parent_index)->object.location;

	return;
}

static void actor_input_update(
	long actor_index)
{
	struct actor_datum *actor = actor_get(actor_index);
	struct actor_definition *actor_definition =
		actor_definition_get(actor->meta.definition_index);

	if (actor->meta.swarm)
	{
		struct swarm_datum *swarm = swarm_get(actor->meta.swarm_cache_index);
		short unit_index;

		swarm->swarm_center = *global_origin3d;
		for (unit_index = 0; unit_index < swarm->unit_count; unit_index++)
		{
			struct swarm_component_datum *component =
				swarm_component_get(swarm->component_indices[unit_index]);

			actor_swarm_component_refresh(
				swarm->unit_indices[unit_index],
				swarm->component_indices[unit_index]);
			add_vectors3d(
				(real_vector3d *)&component->position,
				(real_vector3d *)&swarm->swarm_center,
				(real_vector3d *)&swarm->swarm_center);
		}

		if (swarm->unit_count > 0)
		{
			scale_vector3d(
				(real_vector3d *)&swarm->swarm_center,
				1.f / (real)swarm->unit_count,
				(real_vector3d *)&swarm->swarm_center);
		}

		csmemset(&actor->input.position, 0, sizeof(actor->input));
		actor->input.vehicle_index = NONE;
		actor->input.pathfinding_surface_index = NONE;

		if (actor->meta.swarm_unit_index != NONE)
		{
			actor_input_sample_position(
				actor_index,
				actor->meta.swarm_unit_index,
				&actor->input.position);
		}
	}
	else
	{
		struct unit_datum *unit = unit_get(actor->meta.unit_index);
		long vehicle_index = unit->object.parent_object_index;
		struct object_datum *parent_object = vehicle_index == NONE
			? NULL
			: object_get(vehicle_index);
		real_point3d center_of_mass;

		actor_input_sample_position(
			actor_index,
			actor->meta.unit_index,
			&actor->input.position);
		unit_get_center_of_mass(actor->meta.unit_index, &center_of_mass);
		actor->input.underwater = scenario_location_underwater(
			&actor->input.position.body_location,
			&center_of_mass,
			NULL);
		actor->state.flying =
			TEST_FLAG(actor_definition->flags, _actor_definition_flying_bit);

		if (parent_object && parent_object->object.type == _object_type_vehicle)
		{
			struct unit_datum *vehicle = (struct unit_datum *)parent_object;
			struct vehicle_definition *vehicle_definition =
				vehicle_specific_definition_get(vehicle->definition_index);

			actor->input.vehicle_index = vehicle_index;
			actor->input.vehicle_gunner = FALSE;
			actor->input.vehicle_gunner_bombardment = FALSE;
			actor->input.vehicle_driver_type = _actor_vehicle_driver_none;

			if (vehicle->unit.driver_object_index == actor->meta.unit_index)
			{
				actor->input.vehicle_driver_type =
					_actor_vehicle_driver_unknown;
				if (TEST_FLAG(vehicle_definition->flags, _vehicle_ai_driver_enable_bit))
				{
					if (TEST_FLAG(vehicle_definition->flags, _vehicle_ai_driver_flying_bit))
					{
						actor->input.vehicle_driver_type =
							_actor_vehicle_driver_directional_flying;
						actor->state.flying = TRUE;
					}
					else if (TEST_FLAG(
						vehicle_definition->flags,
						_vehicle_ai_driver_nondirectional_bit))
					{
						actor->input.vehicle_driver_type =
							TEST_FLAG(vehicle_definition->flags, _vehicle_ai_driver_hovering_bit)
								? _actor_vehicle_driver_hovering_ground
								: _actor_vehicle_driver_nondirectional_ground;
					}
				}
			}

			if (vehicle->unit.gunner_object_index == actor->meta.unit_index)
			{
				struct actor_variant_definition *firing_variant_definition;

				actor->input.vehicle_gunner = TRUE;
				firing_variant_definition =
					actor_combat_get_firing_variant_definition(actor_index);
				actor->input.vehicle_gunner_bombardment =
					firing_variant_definition->ranged_combat.weapon_bombardment_range > 0.f;
			}

			actor->input.vehicle_passenger =
				actor->input.vehicle_driver_type <=
					_actor_vehicle_driver_unknown;

			if (vehicle->unit.fake_encounter_index != NONE)
			{
				boolean migrate = FALSE;

				if (DATUM_INDEX_TO_ABSOLUTE_INDEX(actor->meta.encounter_index) !=
					vehicle->unit.fake_encounter_index)
				{
					migrate = TRUE;
				}
				else
				{
					if (vehicle->unit.fake_squad_index != NONE &&
						actor->meta.squad_index != vehicle->unit.fake_squad_index)
					{
						struct encounter_datum *encounter =
							encounter_get(actor->meta.encounter_index);

						migrate = TRUE;
						if (encounter->follow_target_type > 0)
						{
							struct squad_datum *actor_squad =
								encounter_get_squad(
									encounter,
									actor->meta.squad_index);
							struct squad_datum *vehicle_squad =
								encounter_get_squad(
								encounter,
								vehicle->unit.fake_squad_index);

							if (actor_squad->automatic_migration_target &&
								vehicle_squad->automatic_migration_target)
							{
								migrate = FALSE;
							}
						}
					}
				}

				if (migrate)
				{
					if (!actor->meta.stored_prevehicle_encounter)
					{
						actor->meta.prevehicle_encounter_index =
							actor->meta.encounter_index;
						actor->meta.prevehicle_squad_index =
							actor->meta.squad_index;
						actor->meta.stored_prevehicle_encounter = TRUE;

						if (actor->meta.encounter_index != NONE)
						{
							struct encounter_datum *prevehicle_encounter =
								encounter_get(actor->meta.encounter_index);

							prevehicle_encounter->is_prevehicle_encounter = TRUE;
						}
					}

					actor_change_encounter(
						actor_index,
						vehicle->unit.fake_encounter_index,
						vehicle->unit.fake_squad_index);
				}
			}
		}
		else
		{
			actor->input.vehicle_index = NONE;
			actor->input.vehicle_driver_type = _actor_vehicle_driver_none;
			actor->input.vehicle_passenger = FALSE;
			actor->input.vehicle_gunner = FALSE;

			if (actor->meta.stored_prevehicle_encounter)
			{
				actor_change_encounter(
					actor_index,
					actor->meta.prevehicle_encounter_index,
					actor->meta.prevehicle_squad_index);
				actor->meta.stored_prevehicle_encounter = FALSE;
			}
		}

		if (actor->meta.encounter_index != NONE)
		{
			struct actor_debug_info *actor_debug_info =
				&actor_debug_array[DATUM_INDEX_TO_ABSOLUTE_INDEX(actor_index)];
			struct encounter_definition *encounter_definition =
				TAG_BLOCK_GET_ELEMENT(
					&global_scenario_get()->ai_encounters,
					DATUM_INDEX_TO_ABSOLUTE_INDEX(
						actor->meta.encounter_index),
					struct encounter_definition);
			boolean firing_position_type_mismatch = FALSE;

			if (actor->state.flying &&
				!TEST_FLAG(encounter_definition->flags, _encounter_3d_firing_positions_bit))
			{
				firing_position_type_mismatch = TRUE;
			}
			else if (!actor->state.flying &&
				TEST_FLAG(encounter_definition->flags, _encounter_3d_firing_positions_bit))
			{
				firing_position_type_mismatch = TRUE;
			}

			if (firing_position_type_mismatch)
			{
				if (actor_debug_info->firing_position_type_mismatch_ticks < 150)
				{
					actor_debug_info->firing_position_type_mismatch_ticks++;
					if (actor_debug_info->firing_position_type_mismatch_ticks == 150)
					{
						if (actor->state.flying)
						{
							error(
								_error_silent,
								"actor %s is flying but was placed in a non-3d-firing-position encounter (%s)",
								tag_get_name(actor->meta.definition_index),
								encounter_definition->name);
						}
						else
						{
							error(
								_error_silent,
								"actor %s is not flying but was placed in a 3d-firing-position encounter (%s)",
								tag_get_name(actor->meta.definition_index),
								encounter_definition->name);
						}
					}
				}
			}
			else
			{
				actor_debug_info->firing_position_type_mismatch_ticks = 0;
			}
		}

		actor->input.burning_to_death = unit->unit.flaming_death_delay > 0;
		actor->input.melee_attacker_attached = FALSE;
		actor->input.delayed_attached_projectile_index = NONE;

		{
			long threat_index = unit->object.first_child_object_index;

			while (threat_index != NONE)
			{
				struct object_datum *threat = object_get(threat_index);

				if (threat->object.type == _object_type_biped &&
					game_team_is_enemy(
						actor->meta.team_index,
						threat->object.owner_team_index))
				{
					actor->input.melee_attacker_attached = TRUE;
				}
				else if (threat->object.type == _object_type_projectile)
				{
					struct projectile_datum *projectile = (struct projectile_datum *)threat;

					if (TEST_FLAG(projectile->projectile.flags, _projectile_will_super_explode_bit))
					{
						actor->input.delayed_attached_projectile_index = threat_index;
					}
					else if (actor->danger_zone.danger_type == _actor_danger_zone_projectile &&
						threat_index == actor->danger_zone.object_index)
					{
						actor->input.delayed_attached_projectile_index = threat_index;
					}
				}

				threat_index = threat->object.next_object_index;
			}
		}

		actor->input.in_midair = FALSE;
		actor->input.pathfinding_surface_index = NONE;
		if (unit->object.type == _object_type_biped &&
			actor->input.vehicle_index == NONE)
		{
			struct biped_datum *biped = biped_get(actor->meta.unit_index);

			if (biped->biped.airborne_ticks >= 6)
			{
				actor->input.in_midair = TRUE;
			}
			actor->input.pathfinding_surface_index =
				biped->biped.pathfinding_surface_index;
			actor->input.pathfinding_point = biped->biped.pathfinding_point;
		}

		if (actor->input.vehicle_driver_type > _actor_vehicle_driver_none)
		{
			unit_get_facing_vector(
				actor->input.vehicle_index,
				&actor->input.facing_vector);
		}
		else
		{
			unit_get_facing_vector(
				actor->meta.unit_index,
				&actor->input.facing_vector);
		}
		if (!actor->state.flying)
		{
			if (normalize2d((real_vector2d *)&actor->input.facing_vector) > 0.f)
			{
				actor->input.facing_vector.k = 0.f;
			}
			else
			{
				actor->input.facing_vector = *global_forward3d;
			}
		}

		if (actor->input.vehicle_gunner)
		{
			struct unit_datum *gunner_vehicle =
				vehicle_get(actor->input.vehicle_index);
			struct vehicle_definition *gunner_vehicle_definition =
				vehicle_specific_definition_get(
					gunner_vehicle->definition_index);

			if (TEST_FLAG(
				gunner_vehicle_definition->flags,
				_vehicle_ai_weapon_cannot_rotate_bit))
			{
				unit_get_facing_vector(
					actor->meta.unit_index,
					&actor->input.aiming_vector);
			}
			else
			{
				actor->input.aiming_vector =
					gunner_vehicle->unit.aiming_vector;
			}
		}
		else
		{
			actor->input.aiming_vector = unit->unit.aiming_vector;
		}

		actor->input.looking_vector = unit->unit.looking_vector;
		cross_product3d(
			global_up3d,
			&actor->input.looking_vector,
			&actor->input.looking_left_vector);
		normalize3d(&actor->input.looking_left_vector);
		cross_product3d(
			&actor->input.looking_vector,
			&actor->input.looking_left_vector,
			&actor->input.looking_up_vector);

		match_assert_valid_real_normal3d(
			"c:\\halo\\SOURCE\\ai\\actors.c",
			3313,
			&actor->input.facing_vector);
		match_assert_valid_real_normal3d(
			"c:\\halo\\SOURCE\\ai\\actors.c",
			3314,
			&actor->input.aiming_vector);
		match_assert_valid_real_normal3d(
			"c:\\halo\\SOURCE\\ai\\actors.c",
			3315,
			&actor->input.looking_vector);

		if (!actor->state.flying)
		{
			match_vassert(
				"c:\\halo\\SOURCE\\ai\\actors.c",
				3318,
				valid_real_normal2d(
					(real_vector2d *)&actor->input.facing_vector),
				csprintf(
					temporary,
					"%s: assert_valid_real_normal2d(%f, %f)",
					"(real_vector2d *) &actor->input.facing_vector",
					actor->input.facing_vector.i,
					actor->input.facing_vector.j));
			match_assert(
				"c:\\halo\\SOURCE\\ai\\actors.c",
				3319,
				realcmp(actor->input.facing_vector.k, 0.0f));
		}

		actor->input.body_vitality = unit->object.body_vitality;
		actor->input.shield_vitality = unit->object.shield_vitality;
		actor->input.recent_body_damage = unit->object.recent_body_damage;
		actor->input.recent_shield_damage = unit->object.recent_shield_damage;
	}

	return;
}

static void actor_clear_orders(
	long actor_index)
{
	struct actor_datum *actor = actor_get(actor_index);

	csmemset(&actor->orders, 0, sizeof(actor->orders));
	actor->orders.move.animation.impulse = NONE;
	actor->orders.move.override_movement_type = NONE;
	actor->orders.move.override_movement_facing = NONE;

	return;
}

static void actor_decision_loop(
	long actor_index)
{
	struct actor_datum *actor = actor_get(actor_index);
	char message[1024];
	char encounter_name[256];
	short action_history[NUMBER_OF_DECISION_LOOP_HISTORY_ENTRIES];
	boolean action_performed = FALSE;
	long iteration = 0;
	long history_index = 0;

	csmemset(action_history, NONE, sizeof(action_history));

	for (;;)
	{
		action_history[history_index] = actor->state.action;
		iteration++;
		history_index = (history_index + 1) % NUMBER_OF_DECISION_LOOP_HISTORY_ENTRIES;
		actor->state.action_changed = FALSE;
		actor_type_decide_action(actor_index);
		actor_stimulus_clear(actor_index);

		if ((action_performed && !actor->state.action_changed) ||
			iteration >= MAXIMUM_DECISION_LOOP_ITERATIONS)
		{
			break;
		}

		action_performed = actor_action_perform(actor_index);
		if (!action_performed && !actor->state.action_changed)
		{
			return;
		}
	}

	if (actor->meta.encounter_index == NONE)
	{
		csstrcpy(encounter_name, "<no encounter>");
	}
	else
	{
		struct encounter_definition *encounter_definition = TAG_BLOCK_GET_ELEMENT(
			&global_scenario_get()->ai_encounters,
			DATUM_INDEX_TO_ABSOLUTE_INDEX(actor->meta.encounter_index),
			struct encounter_definition);
		struct squad_definition *squad_definition = TAG_BLOCK_GET_ELEMENT(
			&encounter_definition->squads,
			actor->meta.squad_index,
			struct squad_definition);

		sprintf(encounter_name, "%s/%s", encounter_definition->name, squad_definition->name);
	}

	if (iteration >= MAXIMUM_DECISION_LOOP_ITERATIONS)
	{
		long entry_index;

		sprintf(message, "actor-type %s ", actor_type_get_name(actor->meta.type));
		entry_index = history_index;
		do
		{
			if (action_history[entry_index] != NONE)
			{
				csstrcat(message, actor_action_name(action_history[entry_index]));
				csstrcat(message, "/");
			}
			entry_index = (entry_index + 1) % NUMBER_OF_DECISION_LOOP_HISTORY_ENTRIES;
		}
		while (entry_index != history_index);
		sprintf(temporary, " infinite decision loop (%s)", encounter_name);
		csstrcat(message, temporary);
	}
	else
	{
		sprintf(
			message,
			"actor-type %s %s internal logic error (%s)",
			actor_type_get_name(actor->meta.type),
			actor_action_name(actor->state.action),
			encounter_name);
	}

	match_vwarn("c:\\halo\\SOURCE\\ai\\actors.c", 3436, FALSE, message);
	error(_error_silent, "AI error condition detected, attempting to recover (please tell butcher)...");
	actor_action_change(actor_index, _actor_action_none, NULL);

	return;
}

void actors_handle_spatial_effect(
	long object_index,
	short effect_type,
	real_point3d const *position,
	short volume,
	short count)
{
	struct location location;
	struct actor_iterator iterator;
	struct actor_datum *actor;

	scenario_location_from_point(&location, position);

	actor_iterator_new(&iterator, TRUE);
	while ((actor = actor_iterator_next(&iterator)) != NULL)
	{
		if (actor->state.combat_status < _actor_combat_status_visible)
		{
			struct actor_position_data sense_position;

			actor_perception_find_sense_position(iterator.index, position, NONE, &sense_position);
			if (actor_audibility_at_point(iterator.index, &sense_position, position, &location, volume, 1.f, 0) >=
				_ai_sound_volume_loud)
			{
				switch (effect_type)
				{
				case _ai_spatial_effect_environmental_noise:
					actor_stimulus_environmental_noise(iterator.index, object_index, position, count);
					break;

				case _ai_spatial_effect_weapon_impact:
					actor_stimulus_weapon_impact(iterator.index, object_index, position, count);
					break;

				case _ai_spatial_effect_weapon_detonation:
					actor_stimulus_weapon_detonation(iterator.index, object_index, position, count);
					break;

				default:
					display_assert(NULL, "c:\\halo\\SOURCE\\ai\\actors.c", 3498, TRUE);
					system_exit(-1);
					break;
				}
			}
		}
	}

	return;
}

void actor_handle_communication(
	long actor_index,
	long prop_index,
	struct ai_information_packet *information)
{
	if (information)
	{
		switch (information->information_type)
		{
		case _ai_information_combat_stimulus:
			actor_stimulus_enter_combat_friend_in_combat(actor_index, prop_index);
			break;

		case _ai_information_target_knowledge:
			{
				struct prop_datum *prop = prop_get(prop_index);

				if (prop->actor_index != NONE)
				{
					struct prop_datum *friend_prop = prop_try_and_get(
						information->information_data.target_knowledge.prop_index);

					if (friend_prop)
					{
						actor_perception_create_orphan_from_friend(
							actor_index,
							friend_prop->unit_index,
							prop->actor_index,
							information->information_data.target_knowledge.prop_index);
					}
				}
			}
			break;

		case _ai_information_flee:
			actor_stimulus_prop_fleeing(actor_index, prop_index);
			break;
		}
	}

	return;
}

void actor_unit_control_crouch(
	long actor_index,
	boolean crouch)
{
	struct actor_datum *actor = actor_get(actor_index);

	if (crouch)
		actor->output.control_flags |= FLAG(_unit_control_crouch_modifier_bit);
	else
		actor->output.control_flags &= ~FLAG(_unit_control_crouch_modifier_bit);

	return;
}

void actor_unit_control_jump(
	long actor_index)
{
	struct actor_datum *actor = actor_get(actor_index);

	actor->output.control_flags |= FLAG(_unit_control_jump_bit);

	return;
}

void actor_unit_control_primary_trigger(
	long actor_index,
	boolean firing,
	real analog_primary_trigger)
{
	struct actor_datum *actor = actor_get(actor_index);

	if (firing)
		actor->output.control_flags |= FLAG(_unit_control_weapon_primary_trigger_bit);
	else
		actor->output.control_flags &= ~FLAG(_unit_control_weapon_primary_trigger_bit);
	actor->output.analog_primary_trigger = analog_primary_trigger;

	return;
}

void actor_unit_control_secondary_trigger(
	long actor_index,
	boolean firing)
{
	struct actor_datum *actor = actor_get(actor_index);

	if (firing)
		actor->output.control_flags |= FLAG(_unit_control_weapon_secondary_trigger_bit);
	else
		actor->output.control_flags &= ~FLAG(_unit_control_weapon_secondary_trigger_bit);

	return;
}

void actor_unit_control_throw_grenade(
	long actor_index)
{
	struct actor_datum *actor = actor_get(actor_index);

	actor->output.control_flags |= FLAG(_unit_control_throw_grenade_bit);

	return;
}

void actor_unit_control_exact_facing(
	long actor_index,
	boolean exact_facing)
{
	struct actor_datum *actor = actor_get(actor_index);

	if (exact_facing)
		actor->output.control_flags |= FLAG(_unit_control_exact_facing_bit);
	else
		actor->output.control_flags &= ~FLAG(_unit_control_exact_facing_bit);

	return;
}

void actor_unit_control_stop_animation_impulse(
	long actor_index)
{
	struct actor_datum *actor = actor_get(actor_index);

	actor->output.animation.impulse = NONE;

	return;
}

long actor_new(
	long actor_variant_definition_index)
{
	struct actor_variant_definition *actor_variant_definition;
	struct actor_definition *actor_definition;
	struct actor_debug_info *actor_debug_info;
	struct actor_datum *actor;
	long actor_definition_index;
	long actor_index = NONE;

	/* port: a client of the distributed netcode runs no AI: it has the host's
	actors' units, driven as the host sends them (port/linux/game/network_actors.c),
	and its own (placed at the map's start or by its scripts) would be extras
	the host doesn't have */
	if (network_game_distributed_client())
		return NONE;
	if (actor_variant_definition_index != NONE)
	{
		actor_variant_definition = actor_variant_definition_get(actor_variant_definition_index);
		actor_definition_index = actor_variant_definition->actor_reference.index;
		if (actor_definition_index != NONE)
		{
			actor_definition = actor_definition_get(actor_definition_index);
			/* port: only an actor of a type the actor type tables have (a
			map's type indexes their definitions and function pointers);
			else no actor is made, as when the pool is full, said once */
			if (!VALID_INDEX(actor_definition->type, NUMBER_OF_ACTOR_TYPES))
			{
				static boolean reported = FALSE;

				if (!reported)
				{
					error(_error_silent, "actor %s is of type %d (there are %d)",
						tag_get_name(actor_definition_index),
						actor_definition->type,
						NUMBER_OF_ACTOR_TYPES);
					reported = TRUE;
				}
				actor_index = NONE;
			}
			else
				actor_index = datum_new(actor_data);
			if (actor_index != NONE)
			{
				actor = actor_get(actor_index);
				actor->meta.swarm = TEST_FLAG(actor_definition->flags, _actor_definition_swarm_actor_bit);
				actor->meta.variant_definition_index = actor_variant_definition_index;
				actor->meta.definition_index = actor_definition_index;
				actor->meta.type = actor_definition->type;
				actor->meta.unit_index = NONE;
				actor->meta.unique_leader = FALSE;
				actor->meta.encounter_index = NONE;
				actor->meta.squad_index = NONE;
				actor->meta.platoon_index = NONE;
				actor->meta.encounterless = FALSE;
				actor->meta.disconnected_encounter_index = NONE;
				actor->meta.disconnected_squad_index = NONE;
				actor->meta.swarm_unit_count = 0;
				actor->meta.swarm_original_unit_count = 0;
				actor->meta.swarm_unit_index = NONE;
				actor->meta.swarm_cache_index = NONE;
				actor->meta.frozen = TRUE;
				actor->meta.active = FALSE;
				actor->meta.last_active_time = NONE;
				actor->meta.dormant = TRUE;
				actor->meta.dormant_desire = TRUE;
				actor->meta.service_timer = 0;
				actor->meta.first_prop_index = NONE;
				actor->meta.interesting_orphan_index = NONE;
				actor->firing_positions.current_position_index = NONE;
				actor->state.initial_state = NONE;
				actor->state.default_state = NONE;
				actor->state.last_default_state_time = NONE;
				actor->state.command_list_immediate = FALSE;
				actor->state.command_list_index = NONE;
				actor->state.last_command_list_time = NONE;
				actor->state.action = _actor_action_none;
				actor->state.mode = _actor_mode_alert;
				actor->state.combat_status = _actor_combat_status_none;
				actor->state.artificial_combat_status = _actor_combat_status_none;
				actor->state.suspicion_combat_status = _actor_combat_status_none;
				actor->state.uncertain_combat_timer = NONE;
				actor->state.searching = FALSE;
				actor->state.flying = TEST_FLAG(actor_definition->flags, _actor_definition_flying_bit);
				actor->input.pathfinding_surface_index = NONE;
				actor->input.vehicle_index = NONE;
				actor->external_orders.defending = FALSE;
				actor->external_orders.pursuit_is_coordinator = FALSE;
				actor->external_orders.pursuit_group_prop_index = NONE;
				actor->external_orders.desired_target_type = _actor_target_none;
				actor->external_orders.conversation_index = NONE;

				csmemset(&actor->emotions, 0, sizeof(actor->emotions));
				actor->emotions.last_active_cover_seeking_time = NONE;
				actor->emotions.last_melee_check_time = NONE;
				actor->emotions.last_melee_attack_time = NONE;
				actor->emotions.last_defensive_cover_seeking_time = NONE;
				actor->emotions.last_vehicle_check_time = NONE;
				actor->emotions.last_vehicle_charge_time = NONE;
				actor->emotions.last_flee_failed_time = NONE;
				actor->emotions.corpse_ignore_time = NONE;
				actor->emotions.unopposable_friend_ignore_time = NONE;
				actor->emotions.unopposable_retreat_prop_index = NONE;
				actor->emotions.unopposable_retreat_start_time = NONE;
				actor->emotions.original_body_vitality = 1.f;
				actor->emotions.vehicle_ignore_index = NONE;
				actor->emotions.vehicle_ignore_time = NONE;
				actor->emotions.flee_with_friends_disable_time = NONE;

				if (actor_definition->moving.glass_ignorance_chance > 0.f)
				{
					actor->emotions.ignorant_of_broken_surfaces =
						real_seed_random(get_global_random_seed_address()) <
						actor_definition->moving.glass_ignorance_chance;
				}

				actor->orders.look.primary_priority = _primary_priority_none;
				actor->orders.move.destination.destination_type = _destination_none;
				actor->control.path.destination_orders.destination_type = _destination_none;
				actor->control.path.destination_orders.ignore_target_object_index = NONE;
				actor->control.path.destination.surface_index = NONE;
				csmemset(&actor->control.path.path, 0, sizeof(actor->control.path.path));
				actor->control.moving = FALSE;
				actor->control.moving_forced_by_aiming = FALSE;
				actor->control.fire_state = _actor_fire_state_holding;
				actor->control.fire_state_timer = 0;
				actor->control.burst_disable_timer = 0;
				actor->control.trigger_delay_timer = 0;
				actor->control.blocked_communication_timer = 0;
				actor->control.current_fire_target_timer = 0;
				actor->control.current_fire_target_prop_index = NONE;
				actor->control.grenade_last_check_time = NONE;
				actor->control.grenade_current_prop_index = NONE;
				csmemset(
					actor->control.vector_avoidance_clear_times,
					NONE,
					sizeof(actor->control.vector_avoidance_clear_times));
				actor->control.vector_avoidance_current_direction = NONE;
				actor->control.vector_avoidance_sharp_turn_timer = NONE;
				actor->control.secondary_look_type = _secondary_look_none;
				actor->control.secondary_look_timer = 0;
				actor->control.desired_aiming_vector = *global_forward3d;
				actor->control.desired_facing_vector = *global_forward3d;
				actor->control.desired_looking_vector = *global_forward3d;
				actor->control.idle_vocalization_combat = FALSE;
				actor->control.idle_vocalization_timer = TICKS_PER_SECOND;
				actor->target.target_type = _actor_target_none;
				actor->target.target_prop_index = NONE;
				actor->target.target_last_visible_time = NONE;
				actor->target.since_any_target_visible_timer = NONE;
				actor_clear_discarded_firing_positions(actor_index, FALSE);
				actor->firing_positions.pursuit_prop_index = NONE;

				actor_debug_info =
					&actor_debug_array[DATUM_INDEX_TO_ABSOLUTE_INDEX(actor_index)];
				csmemset(actor_debug_info, 0, sizeof(*actor_debug_info));
				actor_debug_info->last_path_refresh = NONE;
				actor_debug_info->last_projectile_aiming_time = NONE;
				actor_debug_info->last_vehicle_avoidance_time = NONE;
				actor_debug_info->last_melee_time = NONE;
				actor_debug_info->grenade_eval_time = NONE;
				actor_debug_info->danger_avoidance_time = NONE;
				actor_debug_info->charge_last_time = NONE;
				actor_debug_info->field_19C = NONE;
				actor_debug_info->vision_last_time = NONE;
				actor_debug_info->perception_awareness_speed = NONE;

				actor_type_initialize(actor_index);
			}
		}
	}

	return actor_index;
}

void actor_customize_unit(
	long actor_variant_definition_index,
	long unit_index)
{
	struct actor_variant_definition *actor_variant_definition =
		actor_variant_definition_get(actor_variant_definition_index);
	struct actor_definition *actor_definition =
		actor_definition_get(actor_variant_definition->actor_reference.index);
	struct unit_datum *unit = unit_get(unit_index);
	struct object_placement_data placement_data;
	short change_color_index;

	if (actor_variant_definition->unit.maximum_body_vitality > 0.f ||
		actor_variant_definition->unit.maximum_shield_vitality > 0.f)
	{
		object_initialize_vitality(
			unit_index,
			&actor_variant_definition->unit.maximum_body_vitality,
			&actor_variant_definition->unit.maximum_shield_vitality);
	}

	if (actor_variant_definition->unit.forced_shader_permutation_index)
	{
		unit->object.forced_shader_permutation_index =
			actor_variant_definition->unit.forced_shader_permutation_index;
	}

	/* port: no more than the object's change colors (a map's count; those
	past them did nothing, and past SHORT_MAX the short counter wraps and the
	loop never ends) */
	for (change_color_index = 0;
		change_color_index < MIN(actor_variant_definition->change_colors.count, NUMBER_OF_OBJECT_CHANGE_COLORS);
		change_color_index++)
	{
		struct actor_variant_change_colors *change_colors = TAG_BLOCK_GET_ELEMENT(
			&actor_variant_definition->change_colors,
			change_color_index,
			struct actor_variant_change_colors);

		if (change_color_index < NUMBER_OF_OBJECT_CHANGE_COLORS)
		{
			rgb_colors_interpolate(
				&unit->object.base_change_colors[change_color_index],
				FLAG(_rgb_color_interpolation_hsv_bit),
				&change_colors->color_lower_bound,
				&change_colors->color_upper_bound,
				real_seed_random(get_global_random_seed_address()));
			unit->object.outgoing_change_colors[change_color_index] =
				unit->object.base_change_colors[change_color_index];
		}
	}

	if (actor_variant_definition->ranged_combat.reference.index != NONE)
	{
		long weapon_index;

		object_placement_data_new(
			&placement_data,
			actor_variant_definition->ranged_combat.reference.index,
			unit_index);
		weapon_index = object_new(&placement_data);
		if (weapon_index != NONE &&
			!unit_add_weapon_to_inventory(
				unit_index,
				weapon_index,
				_unit_add_weapon_replace))
		{
			object_delete(weapon_index);
		}
	}

	if (actor_variant_definition->grenade_combat.grenade_type != NONE)
	{
		short grenade_count = random_range(
			actor_variant_definition->items.grenades_lower_bound,
			actor_variant_definition->items.grenades_upper_bound + 1);

		unit_add_grenade_type_to_inventory(
			unit_index,
			actor_variant_definition->grenade_combat.grenade_type,
			grenade_count);
	}

	if (actor_variant_definition->items.equipment_reference.index != NONE)
	{
		struct equipment_definition *equipment_definition = equipment_definition_get(
			actor_variant_definition->items.equipment_reference.index);

		if (equipment_definition->equipment.powerup_type == _equipment_powerup_none ||
			equipment_definition->equipment.powerup_type == _equipment_powerup_grenade)
		{
			error(
				_error_silent,
				"cannot add grenades or non-powerups to an actor's inventory as equipment... try using the 'grenade' fields maybe?");
		}
		else
		{
			long equipment_index;

			object_placement_data_new(
				&placement_data,
				actor_variant_definition->items.equipment_reference.index,
				unit_index);
			equipment_index = object_new(&placement_data);
			if (equipment_index != NONE &&
				!unit_add_equipment_to_inventory(
					unit_index,
					equipment_index,
					TRUE))
			{
				object_delete(equipment_index);
			}
		}
	}

	if ((actor_variant_definition->flags &
		(FLAG(_actor_variant_definition_active_camouflage_bit) |
			FLAG(_actor_variant_definition_super_active_camouflage_bit))) != 0)
	{
		if (TEST_FLAG(
			actor_variant_definition->flags,
			_actor_variant_definition_super_active_camouflage_bit))
		{
			SET_FLAG(unit->unit.flags, _unit_super_camouflaged_bit, TRUE);
		}
		SET_FLAG(unit->unit.flags, _unit_active_camouflaged_bit, TRUE);
		unit->unit.active_camouflage = 1.f;
		unit->unit.active_camouflage_super_amount = TEST_FLAG(
			actor_definition->flags,
			_actor_definition_crouch_try_not_to_move_bit) ? 1.f : 0.f;
	}

	return;
}

long actor_create_for_unit(
	boolean swarm,
	long unit_index,
	long actor_variant_definition_index,
	long encounter_index,
	short squad_index,
	boolean allow_addition_to_other_squads,
	long disallow_actor_index,
	boolean initially_braindead,
	short initial_state,
	short default_state,
	short initial_command_list_index,
	short noncombat_sequence_id)
{
	long actor_index = NONE;
	struct actor_datum *actor;

	if (unit_index != NONE && actor_variant_definition_index != NONE)
	{
		boolean valid = TRUE;

		if (swarm)
		{
			struct encounter_actor_iterator iterator;

			encounter_actor_iterator_new(&iterator, encounter_index);
			while ((actor = encounter_actor_iterator_next(&iterator)) != NULL)
			{
				if (actor->meta.swarm &&
					iterator.index != disallow_actor_index &&
					actor->meta.swarm_unit_count < MAXIMUM_NUMBER_OF_UNITS_PER_SWARM &&
					actor->meta.variant_definition_index == actor_variant_definition_index &&
					(allow_addition_to_other_squads || actor->meta.squad_index == squad_index))
				{
					actor_index = iterator.index;
					break;
				}
			}
		}
		else
		{
			struct biped_datum *biped = biped_try_and_get(unit_index);

			if (!biped || TEST_FLAG(biped->object.damage_flags, _object_dead_bit))
			{
				valid = FALSE;
			}
		}

		if (valid)
		{
			if (actor_index == NONE)
			{
				actor_index = actor_new(actor_variant_definition_index);
				if (actor_index != NONE)
				{
					boolean actor_is_swarm;

					actor = actor_get(actor_index);
					if (encounter_index == NONE)
					{
						encounterless_attach_actor(actor_index);
					}
					else
					{
						struct encounter_datum *encounter = encounter_get(encounter_index);

						if ((encounter_index & 0xFFFF0000) == 0)
						{
							encounter_index = DATUM_INDEX_NEW(
								DATUM_INDEX_TO_ABSOLUTE_INDEX(encounter_index),
								encounter->identifier);
						}
						encounter_attach_actor(
							actor_index,
							encounter_index,
							squad_index,
							FALSE);
					}

					if (initially_braindead)
					{
						boolean active = actor->meta.active;

						actor->state.mode = _actor_mode_braindead;
						if (active)
						{
							actor_set_dormant(actor_index, FALSE);
						}
					}
					else
					{
						actor->state.mode = _actor_mode_alert;
					}

					actor->state.initial_state = initial_state;
					actor->state.default_state = default_state;
					if (default_state == NONE || default_state == 0)
					{
						actor->state.default_state = actor_action_get_default_state(initial_state);
					}
					actor->state.command_list_immediate = FALSE;
					actor->state.command_list_delay_timer = 2;
					actor->state.command_list_index = initial_command_list_index;
					actor->state.noncombat_sequence_id = noncombat_sequence_id;

					actor_is_swarm = actor->meta.swarm;
					if (actor_is_swarm != actor_type_get_swarm(actor->meta.type))
					{
						char const *actor_description = actor_is_swarm ? "swarm" : "individual";

						error(
							_error_silent,
							"%s actor variant %s cannot have type %s (swarm flag does not match)",
							actor_description,
							tag_name_strip_path(tag_get_name(actor_variant_definition_index)),
							actor_type_get_name(actor->meta.type));
						actor_delete(actor_index, FALSE);
						actor_index = NONE;
					}
				}
			}

			if (actor_index != NONE)
			{
				if (swarm)
				{
					if (!actor_swarm_attach_unit(actor_index, unit_index))
					{
						actor = actor_get(actor_index);
						if (actor->meta.swarm_unit_count == 0)
						{
							actor_delete(actor_index, FALSE);
						}
						actor_index = NONE;
					}
				}
				else
				{
					actor_attach_unit(actor_index, unit_index);
				}

				actor_verify_activation(actor_index);
			}
		}
	}

	return actor_index;
}

long actor_place(
	long actor_variant_definition_index,
	long encounter_index,
	long squad_index,
	struct actor_starting_location *starting_location,
	boolean upgrade_major,
	short initial_variant)
{
	struct actor_variant_definition *actor_variant_definition;
	struct actor_definition *actor_definition;
	struct object_placement_data placement_data;
	char encounter_name[256];
	long unit_index;

	match_assert("c:\\halo\\SOURCE\\ai\\actors.c", 603, starting_location);
	/* port: none on a client of the distributed netcode (actor_new) */
	if (network_game_distributed_client())
		return NONE;
	objects_garbage_collection();

	actor_variant_definition =
		actor_variant_definition_get(actor_variant_definition_index);
	if (upgrade_major)
	{
		actor_variant_definition_index =
			actor_variant_definition->major_upgrade_reference.index;
		match_assert(
			"c:\\halo\\SOURCE\\ai\\actors.c",
			613,
			actor_variant_definition_index != NONE);
		actor_variant_definition =
			actor_variant_definition_get(actor_variant_definition_index);
	}

	actor_definition =
		actor_definition_get(actor_variant_definition->actor_reference.index);
	object_placement_data_new(
		&placement_data,
		actor_variant_definition->unit_reference.index,
		NONE);
	placement_data.position = starting_location->position;
	vector3d_from_angle(&placement_data.forward, starting_location->facing);
	placement_data.variant_number = initial_variant;

	unit_index = object_new(&placement_data);
	if (unit_index == NONE)
	{
		if (encounter_index == NONE)
		{
			csstrcpy(encounter_name, "<encounterless>");
		}
		else
		{
			ai_index_to_string(
				DATUM_INDEX_NEW(
					DATUM_INDEX_TO_ABSOLUTE_INDEX(encounter_index),
					FLAG(_ai_reference_squad_bit) |
						(squad_index & UNSIGNED_CHAR_MAX)),
				global_scenario_get(),
				encounter_name,
				sizeof(encounter_name));
		}

		error(
			_error_silent,
			"WARNING: cannot create unit for actor %s %s",
			tag_name_strip_path(tag_get_name(actor_variant_definition_index)),
			encounter_name);

		return NONE;
	}

	if (encounter_index != NONE)
	{
		struct encounter_definition *encounter_definition =
			TAG_BLOCK_GET_ELEMENT(
				&global_scenario_get()->ai_encounters,
				DATUM_INDEX_TO_ABSOLUTE_INDEX(encounter_index),
				struct encounter_definition);

		if (encounter_definition)
		{
			/* Validate the squad reference before customization. */
			(void)TAG_BLOCK_GET_ELEMENT(
				&encounter_definition->squads,
				squad_index,
				struct squad_definition);
		}
	}

	{
		long actor_index;
		boolean swarm =
			TEST_FLAG(actor_definition->flags, _actor_definition_swarm_actor_bit);
		boolean initially_braindead = FALSE;
		short initial_state = actor_default_state_none;
		short default_state = actor_default_state_none;

		actor_customize_unit(actor_variant_definition_index, unit_index);

		if (encounter_index != NONE)
		{
			struct encounter_definition *encounter_definition =
				TAG_BLOCK_GET_ELEMENT(
					&global_scenario_get()->ai_encounters,
					DATUM_INDEX_TO_ABSOLUTE_INDEX(encounter_index),
					struct encounter_definition);
			struct squad_definition *squad_definition = TAG_BLOCK_GET_ELEMENT(
				&encounter_definition->squads,
				squad_index,
				struct squad_definition);

			initially_braindead =
				TEST_FLAG(encounter_definition->flags, _encounter_braindead_bit);
			initial_state = squad_definition->initial_state;
			default_state = squad_definition->default_state;
		}

		if (starting_location->initial_state > actor_default_state_none)
		{
			initial_state = starting_location->initial_state;
		}
		if (starting_location->default_state > actor_default_state_none)
		{
			default_state = starting_location->default_state;
		}

		{
			word command_list_index = starting_location->command_list_index;
			short noncombat_sequence_id = starting_location->noncombat_sequence_id;

			actor_index = actor_create_for_unit(
				swarm,
				unit_index,
				actor_variant_definition_index,
				encounter_index,
				squad_index,
				FALSE,
				NONE,
				initially_braindead,
				initial_state,
				default_state,
				command_list_index,
				noncombat_sequence_id);
		}
		if (actor_index == NONE)
		{
			if (encounter_index == NONE)
			{
				csstrcpy(encounter_name, "<encounterless>");
			}
			else
			{
				ai_index_to_string(
					DATUM_INDEX_NEW(
						DATUM_INDEX_TO_ABSOLUTE_INDEX(encounter_index),
						FLAG(_ai_reference_squad_bit) |
							(squad_index & UNSIGNED_CHAR_MAX)),
					global_scenario_get(),
					encounter_name,
					sizeof(encounter_name));
			}

			error(
				_error_silent,
				"WARNING: cannot create actor %s %s",
				tag_name_strip_path(tag_get_name(actor_variant_definition_index)),
				encounter_name);
			object_delete(unit_index);
		}
		else
		{
			actor_verify_activation(actor_index);
		}

		return actor_index;
	}
}

short actors_spawn_from_unit(
	long unit_index,
	long actor_variant_definition_index,
	short actor_count,
	real throw_velocity)
{
	short spawned_actor_count = 0;

	/* port: none on a client of the distributed netcode (actor_new) */
	if (actor_variant_definition_index != NONE && actor_count > 0 && !network_game_distributed_client())
	{
		struct unit_datum *source_unit = unit_get(unit_index);
		long source_actor_index = source_unit->unit.swarm_actor_index != NONE ?
			source_unit->unit.swarm_actor_index :
			source_unit->unit.actor_index;
		short encounter_index;
		short squad_index;

		if (source_actor_index != NONE)
		{
			struct actor_datum *source_actor = actor_get(source_unit->unit.actor_index);
			/* BUG: January reads actor_index even when source_actor_index selected swarm_actor_index. */
			encounter_index = source_actor->meta.encounter_index;
			squad_index = source_actor->meta.squad_index;
		}
		else
		{
			encounter_index = source_unit->unit.fake_encounter_index;
			squad_index = source_unit->unit.fake_squad_index;
		}

		if (encounter_index != NONE && squad_index != NONE)
		{
			struct actor_variant_definition *actor_variant_definition =
				actor_variant_definition_get(actor_variant_definition_index);
			struct actor_definition *actor_definition =
				actor_definition_get(actor_variant_definition->actor_reference.index);
			short spawn_index;

			for (spawn_index = 0; spawn_index < actor_count; spawn_index++)
			{
				struct object_placement_data placement_data;
				long spawned_unit_index;
				real angle = real_random_range(0.f, 2.f * _pi);

				object_placement_data_new(
					&placement_data,
					actor_variant_definition->unit_reference.index,
					NONE);
				vector3d_from_angle(&placement_data.forward, angle);
				object_get_origin(unit_index, &placement_data.position);
				placement_data.position.z += 0.3f;
				point_from_line3d(
					&placement_data.position,
					&placement_data.forward,
					0.3f,
					&placement_data.position);

				spawned_unit_index = object_new(&placement_data);
				if (spawned_unit_index != NONE)
				{
					struct unit_datum *spawned_unit = unit_get(spawned_unit_index);
					long actor_index;

					if (spawned_unit->object.type == _object_type_biped)
					{
						biped_fix_position(
							spawned_unit_index,
							NONE,
							&placement_data.position,
							NULL,
							1.f,
							TRUE,
							FALSE,
							FALSE);
					}

					actor_customize_unit(actor_variant_definition_index, spawned_unit_index);
					actor_index = actor_create_for_unit(
						TEST_FLAG(actor_definition->flags, _actor_definition_swarm_actor_bit),
						spawned_unit_index,
						actor_variant_definition_index,
						encounter_index,
						squad_index,
						FALSE,
						NONE,
						FALSE,
						actor_default_state_alert,
						actor_default_state_none,
						NONE,
						0);
					if (actor_index == NONE)
					{
						error(
							_error_silent,
							"WARNING: cannot create actor to be spawned from unit");
						object_delete(spawned_unit_index);
					}
					else
					{
						actor_verify_activation(actor_index);
						if (throw_velocity > 0.f)
						{
							real_vector3d acceleration;
							real speed_factor = real_random_range(0.5f, 1.f);
							real vertical_factor = real_random_range(0.8f, 1.5f);

							acceleration.i = placement_data.forward.i * speed_factor;
							acceleration.j = placement_data.forward.j * speed_factor;
							acceleration.k = vertical_factor;
							scale_vector3d(&acceleration, throw_velocity, &acceleration);
							if (spawned_unit->object.type == _object_type_biped)
							{
								biped_accelerate(spawned_unit_index, &acceleration);
							}
						}
						spawned_actor_count++;
					}
				}
			}
		}
	}

	return spawned_actor_count;
}

void actor_set_dormant(
	long actor_index,
	boolean dormant)
{
	struct actor_datum *actor = actor_get(actor_index);

	if (actor->meta.active && actor->meta.dormant != dormant)
	{
		if (actor->meta.swarm)
		{
			if (actor->meta.swarm_cache_index != NONE)
			{
				struct swarm_datum *swarm = swarm_get(actor->meta.swarm_cache_index);
				short unit_index;

				for (unit_index = 0; unit_index < swarm->unit_count; unit_index++)
				{
					if (dormant)
					{
						object_deactivate(swarm->unit_indices[unit_index]);
					}
					else
					{
						object_activate(swarm->unit_indices[unit_index]);
					}
				}
			}
			else
			{
				long unit_index = actor->meta.swarm_unit_index;

				while (unit_index != NONE)
				{
					struct unit_datum *unit = unit_get(unit_index);

					if (dormant)
					{
						object_deactivate(unit_index);
					}
					else
					{
						object_activate(unit_index);
					}

					unit_index = unit->unit.swarm_next_unit_index;
				}
			}
		}
		else if (actor->meta.unit_index != NONE)
		{
			if (dormant)
			{
				object_deactivate(actor->meta.unit_index);
			}
			else
			{
				object_activate(actor->meta.unit_index);
			}
		}

		actor->meta.dormant = dormant;
		if (!dormant)
		{
			actor->meta.become_dormant_timer = 0;
		}
	}

	actor_verify_activation(actor_index);

	return;
}

static void actor_swarm_component_setup(
	long swarm_index,
	long unit_index,
	long component_index)
{
	struct swarm_datum *swarm = swarm_get(swarm_index);
	struct swarm_component_datum *component = swarm_component_get(component_index);

	component->combat_target_prop_index = NONE;
	match_assert("c:\\halo\\SOURCE\\ai\\actors.c", 1244, swarm->unit_count < MAXIMUM_NUMBER_OF_UNITS_PER_SWARM);
	swarm->unit_indices[swarm->unit_count] = unit_index;
	swarm->component_indices[swarm->unit_count] = component_index;
	swarm->unit_count++;
	actor_swarm_component_refresh(unit_index, component_index);

	return;
}

void actor_delete_props(
	long actor_index)
{
	struct actor_datum *actor = actor_get(actor_index);

	while (actor->meta.first_prop_index != NONE)
	{
		actor_switch_props(actor_index, actor->meta.first_prop_index, NONE);
		prop_delete(actor_index, actor->meta.first_prop_index);
	}

	return;
}

void actor_delete(
	long actor_index,
	boolean died)
{
	struct actor_datum *actor = actor_get(actor_index);
	struct data_iterator iterator;
	struct prop_datum *prop;

	match_assert("c:\\halo\\SOURCE\\ai\\actors.c", 1488, actor_index != global_updating_actor_index);

	if (ai_debug.selected_actor_index == actor_index)
	{
		ai_debug.selected_actor_index = NONE;
	}

	if (ai_debug.field_859F4 == actor_index)
	{
		ai_debug.field_859F4 = NONE;
	}

	if (actor->meta.encounterless)
	{
		encounterless_detach_actor(actor_index);
	}
	else
	{
		encounter_detach_actor(actor_index, died);
	}

	if (actor->meta.swarm)
	{
		actor_swarm_cache_delete(actor_index);
		while (actor->meta.swarm_unit_index != NONE)
		{
			actor_swarm_detach_from_unit(actor_index, actor->meta.swarm_unit_index);
		}
	}
	else
	{
		actor_detach_from_unit(actor_index);
	}

	actor_delete_props(actor_index);

	data_iterator_new(&iterator, prop_data);
	while ((prop = data_iterator_next(&iterator)) != NULL)
	{
		if (prop->actor_index == actor_index)
		{
			prop->actor_index = NONE;
		}
	}

	ai_debug_actor_deleted(actor_index);
	ai_conversation_actor_deleted(actor_index);
	datum_delete(actor_data, actor_index);

	return;
}

long actor_swarm_cache_new(
	long actor_index)
{
	struct actor_datum *actor = actor_get(actor_index);

	if (actor->meta.swarm_cache_index == NONE)
	{
		long swarm_index = datum_new(swarm_data);

		actor->meta.swarm_cache_index = swarm_index;
		if (swarm_index == NONE)
		{
			error(_error_silent, "exceeded MAXIMUM_NUMBER_OF_ACTIVE_SWARMS (%d)", MAXIMUM_NUMBER_OF_ACTIVE_SWARMS);
		}
		else
		{
			struct swarm_datum *swarm = swarm_get(swarm_index);
			long unit_index = actor->meta.swarm_unit_index;

			match_assert("c:\\halo\\SOURCE\\ai\\actors.c", 1570, actor->meta.swarm_unit_count <= MAXIMUM_NUMBER_OF_UNITS_PER_SWARM);
			swarm->actor_index = actor_index;
			swarm->unit_count = 0;

			while (unit_index != NONE)
			{
				struct unit_datum *unit = unit_get(unit_index);
				long component_index = datum_new(swarm_component_data);

				if (component_index == NONE)
				{
					error(_error_silent, "unable to create any more swarm components (max %d)", MAXIMUM_SWARM_COMPONENTS);
					break;
				}

				actor_swarm_component_setup(actor->meta.swarm_cache_index, unit_index, component_index);
				unit_index = unit->unit.swarm_next_unit_index;
			}
		}
	}

	return actor->meta.swarm_cache_index;
}

boolean actor_get_running_blind_vector(
	long actor_index,
	real_vector3d *run_vector)
{
	struct actor_datum *actor = actor_get(actor_index);
	boolean have_vector = FALSE;

	match_assert("c:\\halo\\SOURCE\\ai\\actors.c", 1966, run_vector);

	if (!actor->meta.swarm)
	{
		if (actor->control.moving)
		{
			*run_vector = actor->control.moving_towards_vector;
			have_vector = TRUE;
		}
		else if (actor->control.path.path.valid)
		{
			run_vector->i = actor->control.path.destination.point.x - actor->input.position.body_position.x;
			run_vector->j = actor->control.path.destination.point.y - actor->input.position.body_position.y;
			run_vector->k = actor->control.path.destination.point.z - actor->input.position.body_position.z;
			have_vector = TRUE;
		}
	}

	if (have_vector && normalize3d(run_vector) == 0.f)
	{
		return FALSE;
	}

	return have_vector;
}

void actor_kill(
	long actor_index,
	boolean silent,
	boolean delayed)
{
	struct actor_datum *actor = actor_get(actor_index);
	long encounter_index = actor->meta.encounter_index;

	if (actor->meta.swarm)
	{
		long unit_index = actor->meta.swarm_unit_index;

		while (unit_index != NONE)
		{
			struct unit_datum *unit = unit_get(unit_index);

			if (silent)
			{
				unit->object.damage_flags |= FLAG(_object_die_act_of_god_silent_bit);
			}
			else
			{
				unit->object.damage_flags |= FLAG(_object_die_act_of_god_bit);
			}

			if (!delayed)
			{
				actor_swarm_detach_from_unit(actor_index, unit_index);
			}

			unit_index = unit->unit.swarm_next_unit_index;
		}
	}
	else
	{
		struct unit_datum *unit = unit_get(actor->meta.unit_index);

		if (silent)
		{
			unit->object.damage_flags |= FLAG(_object_die_act_of_god_silent_bit);
		}
		else
		{
			unit->object.damage_flags |= FLAG(_object_die_act_of_god_bit);
		}

		if (!delayed)
		{
			actor_detach_from_unit(actor_index);
		}
	}

	if (!delayed)
	{
		actor_delete(actor_index, TRUE);
		if (encounter_index != NONE)
		{
			encounter_update_status(encounter_index);
		}
	}

	return;
}

void actor_died(
	long actor_index)
{
	struct actor_datum *actor = actor_get(actor_index);
	struct actor_variant_definition *actor_variant_definition =
		actor_variant_definition_get(actor->meta.variant_definition_index);
	long encounter_index = actor->meta.encounter_index;
	struct unit_datum *unit;

	if (actor->state.mode == _actor_mode_combat &&
		actor->state.combat_status >= _actor_combat_status_investigate)
	{
		unit = unit_get(actor->meta.unit_index);
		if (unit_controllable(actor->meta.unit_index) &&
			unit_inventory_get_weapon(
				actor->meta.unit_index,
				(word)unit_get(actor->meta.unit_index)->unit.current_weapon_index) != NONE &&
			unit->unit.weapon_drop_delay_ticks > 0)
		{
			real death_wildfire_chance;

			if (actor_variant_definition->ranged_combat.death_wildfire_chance < 0.1f)
			{
				death_wildfire_chance = 0.1f;
			}
			else if (actor_variant_definition->ranged_combat.death_wildfire_chance > 0.6f)
			{
				death_wildfire_chance = 0.6f;
			}
			else
			{
				death_wildfire_chance = actor_variant_definition->ranged_combat.death_wildfire_chance;
			}

			if (actor->emotions.berserk ||
				(actor->control.current_fire_target_type > _actor_fire_target_none &&
					actor->control.current_fire_target_distance < 3.f))
			{
				real boosted_chance = MIN(death_wildfire_chance * 4.f, 0.6f);

				death_wildfire_chance = MAX(death_wildfire_chance, boosted_chance);
			}

			if (real_seed_random(get_global_random_seed_address()) < death_wildfire_chance)
			{
				real death_wildfire_time;

				if (actor_variant_definition->ranged_combat.death_wildfire_time == 0.f)
				{
					death_wildfire_time = real_random_range(0.8f, 1.3f);
				}
				else if (actor_variant_definition->ranged_combat.death_wildfire_time < 0.8f)
				{
					death_wildfire_time = 0.8f;
				}
				else if (actor_variant_definition->ranged_combat.death_wildfire_time > 1.3f)
				{
					death_wildfire_time = 1.3f;
				}
				else
				{
					death_wildfire_time = actor_variant_definition->ranged_combat.death_wildfire_time;
				}

				{
					short control_ticks = (short)(death_wildfire_time * TICKS_PER_SECOND);

					unit_persistent_control(
						actor->meta.unit_index,
						control_ticks,
						FLAG(_unit_control_weapon_primary_trigger_bit));
					unit->unit.weapon_drop_delay_ticks = (char)control_ticks;
				}
			}
		}
	}

	unit = unit_get(actor->meta.unit_index);
	{
		real grenade_roll = real_seed_random(get_global_random_seed_address());
		long weapon_index = unit_inventory_get_weapon(
			actor->meta.unit_index,
			unit_get(actor->meta.unit_index)->unit.current_weapon_index);

		if (!ai_globals->grenades_enabled ||
			grenade_roll < actor_variant_definition->items.dont_drop_grenades_chance)
		{
			csmemset(unit->unit.grenade_counts, 0, sizeof(unit->unit.grenade_counts));
			/* port: and MCC's expanded grenade inventory */
			if (mcc_grenades_active())
				mcc_grenades_clear(actor->meta.unit_index);
		}

		if (weapon_index != NONE)
		{
			if (actor_variant_definition->items.weapon_loaded_lower_bound > 0.f ||
				actor_variant_definition->items.weapon_loaded_upper_bound > 0.f)
			{
				real weapon_loaded_upper_bound =
					actor_variant_definition->items.weapon_loaded_upper_bound;
				real weapon_loaded_lower_bound =
					actor_variant_definition->items.weapon_loaded_lower_bound;
				real amount = real_seed_random_range(
					get_global_random_seed_address(),
					weapon_loaded_lower_bound,
					weapon_loaded_upper_bound);

				weapon_set_current_amount(weapon_index, amount);
			}

			if (actor_variant_definition->items.weapon_ammo_lower_bound > 0 ||
				actor_variant_definition->items.weapon_ammo_upper_bound > 0)
			{
				short rounds[2];

				csmemset(rounds, 0, sizeof(rounds));
				{
					short weapon_ammo_upper_bound =
						actor_variant_definition->items.weapon_ammo_upper_bound + 1;
					short weapon_ammo_lower_bound =
						actor_variant_definition->items.weapon_ammo_lower_bound;

					rounds[0] = seed_random_range(
						get_global_random_seed_address(),
						weapon_ammo_lower_bound,
						weapon_ammo_upper_bound);
				}
				weapon_set_total_rounds(weapon_index, rounds);
			}
		}
	}

	actor_delete(actor_index, TRUE);
	if (encounter_index != NONE)
	{
		encounter_update_status(encounter_index);
	}

	return;
}

void actor_swarm_unit_died(
	long actor_index,
	long unit_index)
{
	struct actor_datum *actor = actor_get(actor_index);

	match_assert("c:\\halo\\SOURCE\\ai\\actors.c", 2272, actor->meta.swarm);
	actor_swarm_detach_from_unit(actor_index, unit_index);

	if (actor->meta.swarm_unit_count == 0)
	{
		long encounter_index = actor->meta.encounter_index;

		match_assert("c:\\halo\\SOURCE\\ai\\actors.c", 2279, actor->meta.swarm_unit_index == NONE);
		actor_delete(actor_index, TRUE);
		if (encounter_index != NONE)
		{
			encounter_update_status(encounter_index);
		}
	}

	return;
}

void actor_braindead(
	long actor_index,
	boolean braindead)
{
	struct actor_datum *actor = actor_get(actor_index);

	if (braindead)
	{
		actor->state.mode = _actor_mode_braindead;
		actor->state.action = _actor_action_none;
		actor_delete_props(actor_index);
		actor_freeze(actor_index);
		actor_set_dormant(actor_index, FALSE);
	}
	else if (actor->state.mode == _actor_mode_braindead)
	{
		actor->state.mode = _actor_mode_alert;
	}

	return;
}

void actor_handle_unit_effect(
	long actor_index,
	long prop_index,
	short effect_type)
{
	struct actor_datum *actor = actor_get(actor_index);
	struct prop_datum *prop = prop_get(prop_index);
	boolean deaf;

	if (actor->meta.encounter_index != NONE)
	{
		deaf = encounter_get(actor->meta.encounter_index)->deaf;
	}
	else
	{
		deaf = FALSE;
	}

	if (game_connection() == _game_connection_local && ai_debug.deaf)
	{
		deaf = TRUE;
	}

	if (prop->unit_effect == NONE || prop->unit_effect <= effect_type)
	{
		prop->unit_effect = effect_type;
		prop->unit_effect_decay_ticks = effect_type == _ai_unit_effect_magic_sight ? 150 : 30;
	}

	switch (effect_type)
	{
	case _ai_unit_effect_shooting:
		if (!deaf && !prop->ignore)
		{
			prop->shooting = TRUE;
			prop->audibility = _ai_sound_volume_shout;
			prop->perception = _actor_perception_unmistakable;
			prop->refresh_stimuli = TRUE;
			if (prop->player)
			{
				actor_set_dormant(actor_index, FALSE);
			}
			actor_stimulus_heard_shooting(actor_index, prop_index);
		}
		break;

	case _ai_unit_effect_death_scream:
		if (!deaf && !prop->ignore)
		{
			prop->dead = TRUE;
			prop->audibility = _ai_sound_volume_shout;
			prop->perception = _actor_perception_unmistakable;
			prop->refresh_stimuli = TRUE;
			actor_set_dormant(actor_index, FALSE);
			actor_stimulus_prop_just_killed(actor_index, prop_index);
		}
		break;

	case _ai_unit_effect_bump:
		if (!prop->ignore)
		{
			prop->ineffability = _actor_perception_unmistakable;
			prop->perception = _actor_perception_unmistakable;
			prop->refresh_stimuli = TRUE;
			actor_stimulus_bumped(actor_index, prop_index);
		}
		break;

	case _ai_unit_effect_magic_sight:
		if (!prop->ignore)
		{
			prop->ineffability = _actor_perception_unmistakable;
			prop->perception = _actor_perception_unmistakable;
			prop->refresh_stimuli = TRUE;
			if (prop->player)
			{
				actor_set_dormant(actor_index, FALSE);
			}
			actor_perception_become_acknowledged(actor_index, prop_index, NULL);
		}
		break;
	}

	return;
}

boolean actor_set_active(
	long actor_index,
	boolean active)
{
	struct actor_datum *actor = actor_get(actor_index);
	boolean result = TRUE;

	actor_verify_activation(actor_index);

	if (actor->meta.active != active)
	{
		if (active)
		{
			if (actor->meta.swarm)
			{
				actor_swarm_cache_new(actor_index);
				if (actor->meta.swarm_cache_index == NONE)
				{
					error(
						_error_silent,
						"swarm tried to become active but couldn't, erasing %d units",
						actor->meta.swarm_unit_count);
					actor->meta.erase_next_tick = TRUE;
					result = FALSE;
				}
			}
		}
		else
		{
			actor_delete_props(actor_index);
			actor_swarm_cache_delete(actor_index);
		}

		if (result)
		{
			if (active)
			{
				actor->meta.active = TRUE;
				if (actor->state.mode == _actor_mode_braindead)
				{
					actor_set_dormant(actor_index, FALSE);
				}
			}
			else
			{
				actor_set_dormant(actor_index, TRUE);
				actor->meta.active = FALSE;
				actor->meta.last_active_time = game_time_get();
			}
		}
	}

	actor_verify_activation(actor_index);

	return result;
}

boolean actor_swarm_attach_unit(
	long actor_index,
	long unit_index)
{
	struct actor_datum *actor;
	struct unit_datum *unit;
	boolean succeeded;

	actor = actor_get(actor_index);
	unit = unit_get(unit_index);
	succeeded = TRUE;
	if (unit->unit.swarm_actor_index != actor_index)
	{
		long component_index = NONE;

		if (actor->meta.swarm_cache_index != NONE)
		{
			component_index = datum_new(swarm_component_data);
			succeeded = component_index != NONE;
			if (!succeeded)
			{
				error(
					_error_silent,
					"unable to create any more swarm components (max %d)",
					MAXIMUM_SWARM_COMPONENTS);
			}
		}

		if (succeeded)
		{
			if (unit->unit.swarm_actor_index != NONE)
			{
				actor_swarm_detach_from_unit(unit->unit.swarm_actor_index, unit_index);
			}

			if (unit->unit.actor_index != NONE)
			{
				actor_delete(unit->unit.actor_index, FALSE);
			}

			if (actor->meta.unit_index != NONE)
			{
				actor_detach_from_unit(actor_index);
			}

			match_assert("c:\\halo\\SOURCE\\ai\\actors.c", 1299, actor->meta.swarm);
			match_assert("c:\\halo\\SOURCE\\ai\\actors.c", 1300, actor->meta.unit_index == NONE);
			match_assert("c:\\halo\\SOURCE\\ai\\actors.c", 1301, unit->unit.actor_index == NONE);
			match_assert("c:\\halo\\SOURCE\\ai\\actors.c", 1302, unit->unit.swarm_actor_index == NONE);
			match_assert("c:\\halo\\SOURCE\\ai\\actors.c", 1305, actor->meta.swarm_unit_count < MAXIMUM_NUMBER_OF_UNITS_PER_SWARM);

			unit->unit.swarm_actor_index = actor_index;
			unit->unit.swarm_next_unit_index = actor->meta.swarm_unit_index;
			unit->unit.swarm_prev_unit_index = NONE;
			if (actor->meta.swarm_unit_index != NONE)
			{
				struct unit_datum *swarm_first_unit = unit_get(actor->meta.swarm_unit_index);

				match_assert("c:\\halo\\SOURCE\\ai\\actors.c", 1316, swarm_first_unit->unit.swarm_prev_unit_index == NONE);
				swarm_first_unit->unit.swarm_prev_unit_index = unit_index;
			}
			actor->meta.swarm_unit_index = unit_index;

			if (actor->meta.swarm_cache_index != NONE)
			{
				actor_swarm_component_setup(
					actor->meta.swarm_cache_index,
					unit_index,
					component_index);
			}

			actor->meta.swarm_unit_count++;
			actor->meta.swarm_original_unit_count++;
			if (actor->meta.encounter_index != NONE)
			{
				struct encounter_datum *encounter = encounter_get(actor->meta.encounter_index);

				encounter_attach_unit(actor->meta.encounter_index, unit_index);
				unit->object.owner_team_index = encounter->team_index;
			}

			actor->meta.team_index = unit->object.owner_team_index;
			object_set_automatic_deactivation(unit_index, FALSE);
			if (actor->meta.dormant)
			{
				object_deactivate(unit_index);
			}
			else
			{
				object_activate(unit_index);
			}
			unit_set_actively_controlled(unit_index, TRUE);
		}
	}

	return succeeded;
}

void actors_freeze(
	void)
{
	struct actor_iterator iterator;

	actor_iterator_new(&iterator, TRUE);
	while (actor_iterator_next(&iterator))
	{
		actor_freeze(iterator.index);
	}

	return;
}

static void actor_randomly_control_unit(
	long actor_index,
	long unit_index)
{
	struct unit_control_data control_data;

	csmemset(&control_data, 0, sizeof(control_data));
	control_data.animation_state = _unit_animation_state_in_combat;
	control_data.control_flags = 0;
	control_data.throttle = *global_forward3d;
	control_data.aiming_speed = 1;
	control_data.weapon_index = NONE;
	control_data.grenade_index = NONE;
	control_data.zoom_level = NONE;
	unit_get_facing_vector(unit_index, &control_data.facing_vector);
	rotate_vector_about_axis(&control_data.facing_vector, global_up3d, 0.5f, 0.8660254f);
	control_data.aiming_vector = control_data.facing_vector;
	control_data.looking_vector = control_data.facing_vector;

	if ((game_time_get() + unit_index) % 5 == 0)
	{
		control_data.control_flags |= FLAG(_unit_control_weapon_primary_trigger_bit);
		control_data.primary_trigger = 1.0f;
	}

	unit_control(unit_index, &control_data);
	unit_set_actively_controlled(unit_index, FALSE);

	return;
}

void actors_move_randomly(
	void)
{
	struct actor_iterator iterator;
	struct actor_datum *actor;

	actor_iterator_new(&iterator, TRUE);
	while ((actor = actor_iterator_next(&iterator)) != NULL)
	{
		if (actor->meta.swarm)
		{
			if (actor->meta.swarm_cache_index != NONE)
			{
				struct swarm_datum *swarm = swarm_get(actor->meta.swarm_cache_index);
				short unit_index;

				for (unit_index = 0; unit_index < swarm->unit_count; unit_index++)
				{
					actor_randomly_control_unit(iterator.index, swarm->unit_indices[unit_index]);
				}
			}
		}
		else
		{
			actor_randomly_control_unit(iterator.index, actor->meta.unit_index);
		}
	}

	return;
}

void actor_attach_unit(
	long actor_index,
	long unit_index)
{
	struct actor_datum *actor = actor_get(actor_index);
	struct unit_datum *unit = unit_get(unit_index);

	if (unit->unit.actor_index != actor_index)
	{
		if (unit->unit.swarm_actor_index != NONE)
		{
			actor_swarm_detach_from_unit(unit->unit.swarm_actor_index, unit_index);
		}

		if (unit->unit.actor_index != NONE)
		{
			actor_delete(unit->unit.actor_index, FALSE);
		}

		if (actor->meta.unit_index != NONE)
		{
			actor_detach_from_unit(actor_index);
		}

		match_assert("c:\\halo\\SOURCE\\ai\\actors.c", 868, !actor->meta.swarm);
		match_assert("c:\\halo\\SOURCE\\ai\\actors.c", 869, actor->meta.unit_index == NONE);
		match_assert("c:\\halo\\SOURCE\\ai\\actors.c", 870, unit->unit.actor_index == NONE);
		match_assert("c:\\halo\\SOURCE\\ai\\actors.c", 871, unit->unit.swarm_actor_index == NONE);

		actor->meta.unit_index = unit_index;
		unit->unit.actor_index = actor_index;

		if (actor->meta.encounter_index != NONE)
		{
			struct encounter_datum *encounter = encounter_get(actor->meta.encounter_index);

			encounter_attach_unit(actor->meta.encounter_index, unit_index);
			unit->object.owner_team_index = encounter->team_index;
		}

		actor->meta.team_index = unit->object.owner_team_index;

		if (unit->object.variant_number >= 100)
		{
			actor->meta.unique_leader = TRUE;
			if (actor->meta.encounter_index != NONE)
			{
				struct encounter_datum *encounter = encounter_get(actor->meta.encounter_index);

				encounter->unique_leader_count++;
			}
		}

		actor_input_update(actor_index);
		object_set_automatic_deactivation(unit_index, FALSE);
		if (actor->meta.dormant)
		{
			object_deactivate(unit_index);
		}
		else
		{
			object_activate(unit_index);
		}
		unit_set_actively_controlled(unit_index, TRUE);
	}

	actor_verify_activation(actor_index);

	return;
}

void actor_erase(
	long actor_index,
	boolean immediate)
{
	struct actor_datum *actor = actor_get(actor_index);

	if (actor->meta.swarm)
	{
		long unit_index = actor->meta.swarm_unit_index;

		while (unit_index != NONE)
		{
			actor_swarm_detach_from_unit(actor_index, unit_index);
			if (immediate)
			{
				object_delete_immediately(unit_index);
			}
			else
			{
				object_delete(unit_index);
			}

			unit_index = actor->meta.swarm_unit_index;
		}

		actor_delete(actor_index, TRUE);
	}
	else
	{
		long unit_index = actor->meta.unit_index;

		actor_died(actor_index);
		if (immediate)
		{
			object_delete_immediately(unit_index);
		}
		else
		{
			object_delete(unit_index);
		}
	}

	return;
}

static boolean actor_general_update(
	long actor_index)
{
	struct actor_datum *actor = actor_get(actor_index);
	boolean result = TRUE;

	ai_profile.meters[_ai_meter_actors_updated].accumulator++;
	if (!actor->meta.dormant)
	{
		ai_profile.meters[_ai_meter_actors_active].accumulator++;
	}

	if (actor->meta.swarm)
	{
		ai_profile.meters[_ai_meter_units_updated].accumulator += actor->meta.swarm_unit_count;
		if (!actor->meta.dormant)
		{
			ai_profile.meters[_ai_meter_units_active].accumulator += actor->meta.swarm_unit_count;
		}
	}
	else
	{
		ai_profile.meters[_ai_meter_units_updated].accumulator++;
		if (!actor->meta.dormant)
		{
			ai_profile.meters[_ai_meter_units_active].accumulator++;
		}
	}

	if (actor->meta.swarm && actor->meta.swarm_cache_index == NONE)
	{
		match_vwarn(
			"c:\\halo\\SOURCE\\ai\\actors.c",
			2733,
			FALSE,
			csprintf(
				temporary,
				"tried to update a swarm actor without a swarm cache, erasing %d units",
				actor->meta.swarm_unit_count));
		actor_erase(actor_index, FALSE);
		result = FALSE;
	}

	if (result)
	{
		struct encounter_datum *encounter;
		boolean force_active;

		actor->control.path.refreshed_this_tick = FALSE;

		if (actor->state.suspicion_timer > 0)
		{
			actor->state.suspicion_timer--;
			if (actor->state.suspicion_timer == 0)
			{
				actor->state.suspicion_combat_status = _actor_combat_status_none;
			}
		}

		if (actor->state.command_list_delay_timer > 0)
		{
			actor->state.command_list_delay_timer--;
		}

		if (actor->meta.encounter_index != NONE)
		{
			encounter = encounter_get(actor->meta.encounter_index);
		}
		else
		{
			encounter = NULL;
		}

		force_active = actor->meta.force_active;
		if (encounter)
		{
			force_active |= encounter->force_active;
		}

		if (!actor->meta.dormant_desire || force_active)
		{
			actor_set_dormant(actor_index, FALSE);
		}
		else if (!actor->meta.dormant)
		{
			short action_class = actor_action_class(actor_index);
			boolean can_become_dormant = TRUE;

			if (action_class == _action_class_transitory)
			{
				can_become_dormant = FALSE;
			}
			else if (actor->target.target_prop_index != NONE)
			{
				struct prop_datum *prop = prop_get(actor->target.target_prop_index);

				if (prop->player && prop->enemy && !prop->dead)
				{
					if ((prop->state >= _prop_state_becoming_unacknowledged &&
							prop->state <= _prop_state_acknowledged) ||
						(prop_orphaned(prop) && action_class == _action_class_pursuit))
					{
						can_become_dormant = FALSE;
					}
				}
			}

			if (can_become_dormant && actor_path_has_path(actor_index))
			{
				switch (actor->control.path.destination_orders.destination_type)
				{
				case _destination_firing_position:
					if (actor->state.action == _actor_action_guard &&
						encounter->follow_target_type == _follow_target_players)
					{
						can_become_dormant = FALSE;
					}
					break;

				case _destination_prop:
					if (prop_get(actor->control.path.destination_orders.prop.prop_index)->player)
					{
						can_become_dormant = FALSE;
					}
					break;
				}
			}

			if (can_become_dormant)
			{
				actor->meta.become_dormant_timer++;
				if (actor->meta.become_dormant_timer >= 60)
				{
					actor_set_dormant(actor_index, TRUE);
				}
			}
		}
	}

	return result;
}

void actors_handle_unit_effect(
	long unit_index,
	short effect_type,
	short volume)
{
	struct unit_datum *unit = unit_get(unit_index);
	struct location const *location = &unit->object.location;
	struct structure_bsp *structure_bsp;
	struct actor_iterator iterator;
	struct actor_datum *actor;
	unsigned long audible_clusters[BIT_VECTOR_SIZE_IN_LONGS(MAXIMUM_CLUSTERS_PER_STRUCTURE)];
	real_point3d origin;
	long owner_actor_index;

	if (unit->unit.swarm_actor_index != NONE)
	{
		owner_actor_index = unit->unit.swarm_actor_index;
	}
	else
	{
		owner_actor_index = unit->unit.actor_index;
	}

	if (unit->object.parent_object_index != NONE)
	{
		long ultimate_parent_index = object_get_ultimate_parent(unit_index);

		location = &object_get(ultimate_parent_index)->object.location;
	}

	structure_bsp = global_structure_bsp_get();
	csmemset(audible_clusters, 0, BIT_VECTOR_SIZE_IN_BYTES(structure_bsp->clusters.count));

	if (location->cluster_index != NONE)
	{
		short cluster_index;

		for (cluster_index = 0; cluster_index < structure_bsp->clusters.count; cluster_index++)
		{
			byte encoded_sound_distance = structure_bsp_get_cluster_encoded_sound_distance(
				structure_bsp,
				location->cluster_index,
				cluster_index);

			if (!(encoded_sound_distance & FLAG(7)) &&
				(encoded_sound_distance & ~FLAG(7)) * 2.015748f < 40.f)
			{
				BIT_VECTOR_SET_FLAG(audible_clusters, cluster_index, TRUE);
			}
		}
	}

	object_get_origin(unit_index, &origin);

	actor_iterator_new(&iterator, TRUE);
	while ((actor = actor_iterator_next(&iterator)) != NULL)
	{
		if (iterator.index != owner_actor_index &&
			actor->input.position.body_location.cluster_index != NONE &&
			BIT_VECTOR_TEST_FLAG(audible_clusters, actor->input.position.body_location.cluster_index))
		{
			struct actor_position_data sense_position;

			actor_perception_find_sense_position(iterator.index, &origin, NONE, &sense_position);
			if (actor_audibility_at_point(iterator.index, &sense_position, &origin, location, volume, 1.f, 0) >=
				_ai_sound_volume_loud)
			{
				long prop_index = prop_get_base_by_unit_index(iterator.index, unit_index, TRUE, TRUE);

				if (prop_index != NONE)
				{
					struct prop_datum *prop = prop_get(prop_index);

					if (actor_audibility_at_point(
						iterator.index,
						&sense_position,
						&prop->body_position,
						&prop->body_location,
						volume,
						1.f,
						prop->line_of_sight) >= _ai_sound_volume_loud)
					{
						actor_handle_unit_effect(iterator.index, prop_index, effect_type);
					}
				}
			}
		}
	}

	return;
}

static void actor_unit_control(
	long actor_index)
{
	struct actor_datum *actor = actor_get(actor_index);
	struct unit_datum *unit = unit_get(actor->meta.unit_index);
	struct unit_control_data control_data;

	control_data.animation_state = global_movement_animation_states[actor->output.movement_type];
	control_data.control_flags = actor->output.control_flags;
	control_data.primary_trigger = actor->output.analog_primary_trigger;
	control_data.throttle = actor->output.throttle;
	control_data.aiming_speed = actor->output.aiming_speed;
	control_data.facing_vector = actor->output.facing_vector;
	control_data.aiming_vector = actor->output.aiming_vector;
	control_data.looking_vector = actor->output.looking_vector;
	control_data.weapon_index = NONE;
	control_data.grenade_index = NONE;
	control_data.zoom_level = NONE;

	if (!actor->state.flying)
	{
		match_vassert(
			"c:\\halo\\SOURCE\\ai\\actors.c",
			3846,
			valid_real_normal2d((real_vector2d *) &control_data.facing_vector),
			csprintf(
				temporary,
				"%s: assert_valid_real_normal2d(%f, %f)",
				"(real_vector2d *) &control_data.facing_vector",
				control_data.facing_vector.i,
				control_data.facing_vector.j));
	}
	match_assert_valid_real_normal3d("c:\\halo\\SOURCE\\ai\\actors.c", 3848, &control_data.facing_vector);
	match_assert_valid_real_normal3d("c:\\halo\\SOURCE\\ai\\actors.c", 3849, &control_data.aiming_vector);
	match_assert_valid_real_normal3d("c:\\halo\\SOURCE\\ai\\actors.c", 3850, &control_data.looking_vector);
	match_assert("c:\\halo\\SOURCE\\ai\\actors.c", 3851, (fabs(control_data.throttle.i) <= 1.0f) && (fabs(control_data.throttle.j) <= 1.0f) && (fabs(control_data.throttle.k) <= 1.0f));

	if (unit->unit.player_index == NONE || !player_input_enabled())
	{
		if (actor->meta.frozen)
		{
			unit_set_actively_controlled(actor->meta.unit_index, TRUE);
			actor->meta.frozen = FALSE;
		}

		unit_control(actor->meta.unit_index, &control_data);

		if (actor->output.animation.impulse != NONE)
		{
			unit_start_animation_impulse(
				actor->meta.unit_index,
				actor->output.animation.impulse,
				&actor->output.animation.alignment_vector);
		}

		if (actor->output.persistent_control_ticks > 0)
		{
			unit_persistent_control(
				actor->meta.unit_index,
				actor->output.persistent_control_ticks,
				actor->output.persistent_control_flags);
		}
	}

	return;
}

static void actor_update(
	long actor_index)
{
	struct actor_datum *actor = actor_get(actor_index);

	actor_update_begin(actor_index);
	if (actor_general_update(actor_index))
	{
		actor_get_timeslice(actor_index);
		actor_input_update(actor_index);
		actor_perception_update(actor_index);
		actor_situation_update(actor_index);
		actor_emotion_update(actor_index);
		actor_clear_orders(actor_index);
		actor_decision_loop(actor_index);
		actor_action_update(actor_index);

		if (!actor->meta.dormant)
		{
			if (actor->meta.swarm)
			{
				actor_type_swarm_control(actor_index);
			}
			else
			{
				actor_clear_output(actor_index);
				actor_action_control(actor_index);
				actor_communication_update(actor_index);
				actor_conversation_control(actor_index);
				actor_destination_update(actor_index);
				actor_look_affect_movement(actor_index);
				actor_move_update(actor_index);
				actor_look_update(actor_index);
				actor_combat_update(actor_index);
				actor_unit_control(actor_index);
			}
		}
	}
	actor_update_end();

	return;
}

/* port: an actor whose unit is dead dies with it. unit_died takes the actor
away, but a unit can be marked dead without it (object_deplete_body: a
damage region that forces death, a script's vitality set to 0). Its actor
was then kept, counted by its encounter, so a mission waiting for the
encounter to die waited forever, and its nav point pointed at a body every
client had dead. A feigned death (its timer running) and a player's unit
are left as they are. */
static boolean actor_port_unit_dead(
	long actor_index)
{
	struct actor_datum *actor = actor_get(actor_index);
	struct unit_datum *unit;

	if (actor->meta.swarm || actor->meta.unit_index == NONE)
		return FALSE;
	unit = unit_get(actor->meta.unit_index);
	if (!TEST_FLAG(unit->object.damage_flags, _object_dead_bit) || unit->unit.feign_death_timer > 0 ||
		unit->unit.player_index != NONE || unit->unit.actor_index != actor_index)
	{
		return FALSE;
	}
	unit_died(actor->meta.unit_index, FALSE);

	return TRUE;
}

void actors_update(
	void)
{
	struct actor_iterator iterator;
	struct actor_datum *actor;

	ai_globals->last_highest_service_timer = ai_globals->current_highest_service_timer;
	ai_globals->current_highest_service_timer = 0;
	ai_globals->time_given_this_frame = FALSE;

	actor_iterator_new(&iterator, TRUE);
	while ((actor = actor_iterator_next(&iterator)) != NULL)
	{
		if (actor->meta.erase_next_tick)
		{
			actor_erase(iterator.index, FALSE);
		}
		/* port: (actor_port_unit_dead) */
		else if (actor_port_unit_dead(iterator.index))
		{
		}
		else if (actor->state.mode > _actor_mode_braindead)
		{
			actor_update(iterator.index);
		}
	}

	return;
}

/* ---------- private code */
