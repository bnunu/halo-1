/*
FIRST_PERSON_WEAPONS.C

symbols in this file:
000CBC60 0020:
	_animation_update_render_only (0000)
000CBC80 0040:
	_first_person_weapons_initialize (0000)
000CBCC0 0010:
	_first_person_weapons_dispose (0000)
000CBCD0 0050:
	_first_person_weapons_initialize_for_new_map (0000)
000CBD20 0010:
	_first_person_weapons_dispose_from_old_map (0000)
000CBD30 00c0:
	_code_000cbd30 (0000)
000CBDF0 0110:
	_code_000cbdf0 (0000)
000CBF00 0120:
	_code_000cbf00 (0000)
000CC020 0040:
	_code_000cc020 (0000)
000CC060 00a0:
	_code_000cc060 (0000)
000CC100 00b0:
	_code_000cc100 (0000)
000CC1B0 00e0:
	_code_000cc1b0 (0000)
000CC290 0060:
	_code_000cc290 (0000)
000CC2F0 0040:
	_code_000cc2f0 (0000)
000CC330 0080:
	_code_000cc330 (0000)
000CC3B0 0290:
	_first_person_weapon_draw (0000)
000CC640 0080:
	_first_person_weapon_get_local_index (0000)
000CC6C0 00d0:
	_first_person_weapon_get_marker_by_name (0000)
000CC790 00e0:
	_first_person_weapon_center_flashlight (0000)
000CC870 00d0:
	_first_person_weapon_adjust_light (0000)
000CC940 00c0:
	_first_person_weapon_get_node_matrix (0000)
000CCA00 00b0:
	_code_000cca00 (0000)
000CCAB0 0560:
	_code_000ccab0 (0000)
000CD010 00b0:
	_first_person_weapon_render_update (0000)
000CD0C0 0040:
	_first_person_weapon_get_marker_by_name_render (0000)
000CD100 02b0:
	_code_000cd100 (0000)
000CD3B0 0260:
	_code_000cd3b0 (0000)
000CD610 0060:
	_code_000cd610 (0000)
000CD670 0220:
	_code_000cd670 (0000)
000CD890 0050:
	_first_person_weapon_message_from_unit (0000)
000CD8E0 0040:
	_first_person_weapon_message_from_weapon (0000)
000CD920 0170:
	_code_000cd920 (0000)
000CDA90 0600:
	_code_000cda90 (0000)
000CE090 00f0:
	_first_person_weapons_update (0000)
0027076C 0015:
	??_C@_0BF@MEAEHHHF@first_person_weapons?$AA@ (0000)
00270784 0030:
	??_C@_0DA@LGOECHKB@c?3?2halo?2SOURCE?2interface?2first_p@ (0000)
002707B4 0015:
	??_C@_0BF@LKJCKPFC@first?5person?5weapons?$AA@ (0000)
002707CC 0028:
	??_C@_0CI@FFNKKBPO@first_person_weapon?9?$DOweapon_inde@ (0000)
002707F8 0059:
	??_C@_0FJ@DFINBDBK@animation_graph_node_index?$DO?$DN0?5?$CG?$CG@ (0000)
00270854 000b:
	??_C@_0L@HGCOMEEN@flashlight?$AA@ (0000)
00270860 0039:
	??_C@_0DJ@KDPHKICA@node_index?$DO?$DN0?5?$CG?$CG?5node_index?$DManim@ (0000)
0027089C 0035:
	??_C@_0DF@COECBIGL@local?5player?5?$CFd?0?5weapon?5?$CI0x?$CFx?$CJ?0?5@ (0000)
002708D8 00a5:
	??_C@_0KF@OCJNMLNK@first_person_weapon?9?$DOshotgun_rel@ (0000)
00270980 0004:
	__real@c1f00000 (0000)
00453CC8 0004:
	_bss_00453cc8 (0000)
*/

/* ---------- headers */

#define REAL_MATH_EXTERNAL_SIGNED_ANGULAR_DIFFERENCE
#define REAL_MATH_EXTERNAL_REAL_LOCAL_RANDOM
#define REAL_MATH_EXTERNAL_REAL_LOCAL_RANDOM_RANGE
#include "interface/first_person_weapons.h"

#include "cache/predicted_resources.h"
#include "camera/director.h"
#include "cseries/cseries.h"
#include "cseries/errors.h"
#include "effects/effects.h"
#include "effects/particles.h"
#include "game/game_globals.h"
#include "game/players.h"
#include "items/weapon_datum_flags.h"
#include "items/weapon_definitions.h"
#include "items/weapons.h"
#include "models/model_animation_definitions.h"
#include "models/model_definitions.h"
#include "models/models.h"
#include "networking/network_connection.h"
#include "objects/objects.h"
#include "rasterizer/rasterizer.h" /* port: rasterizer_memory_alloc */
#include "rasterizer/rasterizer_model_types.h"
#include "render/render.h"
#include "saved games/game_state.h"
#include "scenario/scenario.h"
#include "sound/game_sound.h"
#include "sound/sound_manager.h"
#include "units/units.h"
#include "view_fov.h" /* port: port/linux/game/view_fov.c */
#include "shaders/shader_definitions.h" /* port: (the shield's flare on the arms) */
#include "shaders/shaders.h"
/* port: port/linux/game/shield_color.c */
real_rgb_color const *shield_color_colors(long unit_index, real_rgb_color const *colors);
/* port: port/linux/game/overshield_effect.c */
real const *overshield_effect_values(long unit_index, real const *values, boolean arms);

/* ---------- constants */

enum
{
	_render_model_first_person_bit = 3,
};

enum
{
	NUMBER_OF_FIRST_PERSON_WEAPON_OVERLAY_FRAMES = 9,
	FIRST_PERSON_WEAPON_RELOAD_SETTLE_TICKS = 44,
};

#define FIRST_PERSON_WEAPON_FIRING_PUSH_BACK_VELOCITY 0.05f

enum first_person_weapon_state
{
	_first_person_weapon_state_idle = 0,
	_first_person_weapon_state_overheating,
	_first_person_weapon_state_overheating_again,
	_first_person_weapon_state_overheated,
	_first_person_weapon_state_charged,
	_first_person_weapon_state_posing,
	_first_person_weapon_state_primary_fire,
	_first_person_weapon_state_secondary_fire,
	_first_person_weapon_state_primary_misfire,
	_first_person_weapon_state_secondary_misfire,
	_first_person_weapon_state_melee,
	_first_person_weapon_state_light_on,
	_first_person_weapon_state_light_off,
	_first_person_weapon_state_reload_while_empty,
	_first_person_weapon_state_reload_while_full,
	_first_person_weapon_state_shotgun_enter_reload,
	_first_person_weapon_state_shotgun_exit_reload_empty,
	_first_person_weapon_state_shotgun_exit_reload_full,
	_first_person_weapon_state_put_away,
	_first_person_weapon_state_ready,
	_first_person_weapon_state_throw_grenade,
	_first_person_weapon_state_throw_grenade_overheated,
	_first_person_weapon_state_overheated_exit,
	_first_person_weapon_state_overheating_super_recoil,
	NUMBER_OF_FIRST_PERSON_WEAPON_STATES,
};

enum
{
	_render_model_effect_type_none = 0,
	_render_model_effect_type_active_camouflage,
};

enum
{
	animation_update_kind_render_only = 0,
};

enum animation_update_result
{
	_animation_no_key_frame = 0,
	_animation_key_frame,
	_animation_will_restart_on_next_frame,
};

/* ---------- macros */

/* ---------- structures */

struct shader;

/* Kept TU-private because moving this recovered layout into a shared header perturbs VC7 output. */
struct animation_state_continuous
{
	short index;
	word pad;
	real frame_index;
};

/* Recovered animation-graph block layouts used only by this translation unit. */
struct animation_graph_sound_reference
{
	struct tag_reference sound;
	long unused;
};

typedef char verify_animation_graph_sound_reference_size[
	sizeof(struct animation_graph_sound_reference) == 0x14 ? 1 : -1];

struct animation_graph_first_person_weapon_animations
{
	long unused[4];
	struct tag_block animations;		// animation_graph_animation_index
};

typedef char verify_animation_graph_first_person_weapon_animations_size[
	sizeof(struct animation_graph_first_person_weapon_animations) == 0x1C ? 1 : -1];

/* Only the leading name is consumed here; the remaining bytes preserve the January layout. */
struct animation_graph_node
{
	char name[TAG_STRING_LENGTH+1];
	byte reserved0020[0x20];
};

typedef char verify_animation_graph_node_size[
	sizeof(struct animation_graph_node) == 0x40 ? 1 : -1];

typedef char verify_render_model_effect_size[
	sizeof(struct render_model_effect) == 0x28 ? 1 : -1];

struct first_person_weapon
{
	boolean visible;
	byte reserved0001[3];
	long unit_index;
	long weapon_index;
	short state;
	short ticks_until_pose;
	short ticks_idle;
	short ticks_until_predict;
	short overlay_animation_index;
	struct animation_state state_animation;
	struct animation_state moving_animation;
	short reserved001e;
	struct animation_state_continuous overcharged_jitter_animation;
	real firing_push_back;
	real firing_push_back_velocity;
	real_vector2d position;
	real_vector2d position_velocity;
	real_vector2d turning;
	real_vector2d turning_velocity;
	boolean rendered;
	byte reserved0051[3];
	real_vector3d render_forward;
	real_euler_angles2d render_facing;
	real_euler_angles2d last_render_facing;
	real_point3d render_position;
	real_point3d last_render_position;
	short interpolation_frame_index;
	short interpolation_frame_count;
	real_orientation node_orientations[MAXIMUM_NODES_PER_ANIMATION];
	real_orientation original_node_orientations[MAXIMUM_NODES_PER_ANIMATION];
	struct real_matrix4x3 node_matrices[MAXIMUM_NODES_PER_ANIMATION];
	boolean weapon_node_remapping_table_valid;
	byte reserved1d8d;
	short weapon_node_remapping_table[MAXIMUM_NODES_PER_ANIMATION];
	boolean hands_node_remapping_table_valid;
	byte reserved1e0f;
	short hands_node_remapping_table[MAXIMUM_NODES_PER_ANIMATION];
	boolean shotgun_empty;
	byte reserved1e91;
	short shotgun_shells_to_reload;
	short shotgun_reload_type;
	short reserved1e96;
	long current_sound_index;
	short current_sound_state;
	byte reserved1e9e[2];
};

typedef char verify_first_person_weapon_size[
	sizeof(struct first_person_weapon) == 0x1EA0 ? 1 : -1];
typedef char verify_first_person_weapon_node_matrices_offset[
	offsetof(struct first_person_weapon, node_matrices) == 0x108C ? 1 : -1];
typedef char verify_first_person_weapon_rendered_offset[
	offsetof(struct first_person_weapon, rendered) == 0x50 ? 1 : -1];
typedef char verify_first_person_weapon_node_orientations_offset[
	offsetof(struct first_person_weapon, node_orientations) == 0x8C ? 1 : -1];
typedef char verify_first_person_weapon_node_remapping_table_offset[
	offsetof(struct first_person_weapon, weapon_node_remapping_table) == 0x1D8E ? 1 : -1];
typedef char verify_first_person_weapon_shotgun_empty_offset[
	offsetof(struct first_person_weapon, shotgun_empty) == 0x1E90 ? 1 : -1];
typedef char verify_first_person_weapon_current_sound_index_offset[
	offsetof(struct first_person_weapon, current_sound_index) == 0x1E98 ? 1 : -1];
typedef char verify_first_person_weapon_current_sound_state_offset[
	offsetof(struct first_person_weapon, current_sound_state) == 0x1E9C ? 1 : -1];
typedef char verify_weapon_first_person_animations_index_offset[
	offsetof(
		struct weapon_definition,
		weapon.interface_definition.first_person_animations.index) == 0x478 ? 1 : -1];
typedef char verify_weapon_flags_offset[
	offsetof(struct weapon_datum, weapon.flags) == 0x1DC ? 1 : -1];
typedef char verify_weapon_type_offset[
	offsetof(struct weapon_definition, weapon.weapon_type) == 0x4E2 ? 1 : -1];
typedef char verify_animation_graph_first_person_weapon_animations_offset[
	offsetof(struct animation_graph, first_person_weapon_animations) == 0x48 ? 1 : -1];
typedef char verify_unit_flags_offset[
	offsetof(struct unit_datum, unit.flags) == 0x1B4 ? 1 : -1];
typedef char verify_unit_active_camouflage_offset[
	offsetof(struct unit_datum, unit.active_camouflage) == 0x32C ? 1 : -1];
typedef char verify_object_outgoing_function_values_offset[
	offsetof(struct object_datum, object.outgoing_function_values) == 0xE4 ? 1 : -1];
typedef char verify_object_outgoing_change_colors_offset[
	offsetof(struct object_datum, object.outgoing_change_colors) == 0x168 ? 1 : -1];
typedef char verify_game_globals_first_person_interface_offset[
	offsetof(struct game_globals, first_person_interface) == 0x17C ? 1 : -1];
typedef char verify_animation_graph_nodes_offset[
	offsetof(struct animation_graph, nodes) == 0x68 ? 1 : -1];

/* ---------- prototypes */

static void first_person_weapon_set_visibility(
	short local_player_index,
	boolean visible);
static void first_person_weapon_build_node_matrices(
	short local_player_index);
static boolean first_person_weapon_animation_fits(
	struct animation_graph const *animation_graph,
	struct animation const *animation);
static void first_person_weapon_predict(
	short local_player_index);
static void first_person_weapon_start_interpolation(
	short local_player_index,
	short frame_count);
static void first_person_weapon_set_state(
	short local_player_index,
	short new_state,
	boolean reset_sounds);
static void first_person_weapon_switch_weapons(
	short local_player_index);
static void first_person_weapon_forget_weapon(
	short local_player_index);
static void first_person_weapon_new_unit(
	short local_player_index,
	long unit_index);
static void first_person_weapon_message(
	short local_player_index,
	short message_type);
static void first_person_weapon_next_state(
	short local_player_index);
static void first_person_weapon_update(
	short local_player_index);
static boolean model_build_remapping_table_for_animation_graph(
	long model_index,
	long animation_graph_index,
	short *node_remapping_table);
static void weapon_play_first_person_weapon_sound(
	long weapon_index,
	short message_type);
static long first_person_weapon_state_from_weapon_message(
	short message_type);
static long first_person_animation_type_from_weapon_state(
	short state);
static void model_remap_node_matrices_to_match_animation_graph(
	long model_index,
	struct real_matrix4x3 *model_node_matrices,
	long animation_graph_index,
	struct real_matrix4x3 const *node_matrices,
	short const *node_remapping_table);
static struct first_person_weapon *first_person_weapon_get(
	short local_player_index);
static short first_person_weapon_index_from_weapon_index(
	long weapon_index);
static short first_person_weapon_index_from_unit_index(
	long unit_index);

/* port/linux/game/pal_tags.c's */
boolean pal_tags_first_person_advance(short local_player_index, long graph_index, short animation_index,
	short frame_index);
real pal_tags_first_person_fraction(short local_player_index, long graph_index, short animation_index,
	short frame_index);

/* ---------- globals */

static struct first_person_weapon *first_person_weapons;

/* ---------- public code */

short animation_update_render_only(
	long animation_graph_index,
	struct animation_state *state,
	long *sound_index)
{
	return animation_update_internal(
		0,
		animation_graph_index,
		state,
		sound_index);
}

void first_person_weapons_initialize(
	void)
{
	first_person_weapons = (struct first_person_weapon *)game_state_malloc(
		"first person weapons",
		NULL,
		sizeof(*first_person_weapons) * MAXIMUM_NUMBER_OF_LOCAL_PLAYERS);
	match_assert(
		"c:\\halo\\SOURCE\\interface\\first_person_weapons.c",
		240,
		first_person_weapons);

	return;
}

void first_person_weapons_dispose(
	void)
{
	return;
}

void first_person_weapons_initialize_for_new_map(
	void)
{
	short local_player_index;

	for (local_player_index=0; local_player_index<MAXIMUM_NUMBER_OF_LOCAL_PLAYERS; local_player_index++)
	{
		csmemset(
			&first_person_weapons[local_player_index],
			0,
			sizeof(first_person_weapons[local_player_index]));
		first_person_weapons[local_player_index].unit_index= NONE;
		first_person_weapons[local_player_index].current_sound_index= NONE;
		first_person_weapons[local_player_index].current_sound_state= NONE;
	}

	return;
}

void first_person_weapons_dispose_from_old_map(
	void)
{
	return;
}

long first_person_weapon_get_local_index(
	long weapon_index)
{
	short local_player_index;

	for (local_player_index=0; local_player_index<MAXIMUM_NUMBER_OF_LOCAL_PLAYERS; local_player_index++)
	{
		struct first_person_weapon *first_person_weapon= first_person_weapon_get(local_player_index);

		if (first_person_weapon->weapon_index==weapon_index && first_person_weapon->visible)
		{
			break;
		}
	}

	return (local_player_index==MAXIMUM_NUMBER_OF_LOCAL_PLAYERS) ? NONE : local_player_index;
}

void first_person_weapon_draw(
	void)
{
	/* port: not drawn with display.viewmodel_visible off (view_fov.c) */
	if (!viewmodel_is_visible())
		return;
	if (render.local_player_index!=NONE)
	{
		struct first_person_weapon *first_person_weapon= first_person_weapon_get(render.local_player_index);

		if (local_player_get_player_index(render.local_player_index)!=NONE)
		{
			long unit_index= player_get(
				local_player_get_player_index(render.local_player_index))->unit_index;

			if (unit_index!=NONE &&
				first_person_weapon->visible &&
				first_person_weapon->unit_index!=NONE &&
				first_person_weapon->weapon_index!=NONE)
			{
				struct unit_datum *unit= unit_get(unit_index);
				struct weapon_datum *weapon= weapon_get(first_person_weapon->weapon_index);
				struct weapon_definition *weapon_definition= weapon_definition_get(weapon->definition_index);

				if (weapon_definition->weapon.interface_definition.first_person_animations.index!=NONE)
				{
					struct real_matrix4x3 model_node_matrices[MAXIMUM_NODES_PER_ANIMATION];
					struct render_model_effect model_effect;
					struct game_globals_first_person_interface *first_person_interface=
						TAG_BLOCK_GET_ELEMENT(
							&scenario_get_game_globals()->first_person_interface,
							0,
							struct game_globals_first_person_interface);
					struct animation_graph *animation_graph= animation_graph_definition_get(
						weapon_definition->weapon.interface_definition.first_person_animations.index);
					struct render_lighting *lighting= object_get_cached_render_lighting(unit_index, FLT_MAX);
					real values[NUMBER_OF_OUTGOING_OBJECT_FUNCTIONS];
					real const *weapon_functions = weapon->object.outgoing_function_values;
					real const *unit_functions = unit->object.outgoing_function_values;
					real *kept;

					/* port: the tick's functions, moved into this frame, so the
					charge clicks with the picture instead of once a tick; kept
					in the rasterizer's memory for the frame, as the groups drawn
					later read them (render_objects.c) */
					object_function_values_for_render(first_person_weapon->weapon_index, values);
					if ((kept = rasterizer_memory_alloc(values, sizeof(values))) != NULL)
						weapon_functions = kept;
					object_function_values_for_render(unit_index, values);
					if ((kept = rasterizer_memory_alloc(values, sizeof(values))) != NULL)
						unit_functions = kept;
					model_effect.modifier_shader= NULL;
					if (TEST_FLAG(unit->unit.flags, _unit_active_camouflaged_bit) ||
						unit->unit.active_camouflage>0.0f)
					{
						model_effect.type= _render_model_effect_type_active_camouflage;
						model_effect.intensity= unit->unit.active_camouflage;
						model_effect.parameter= unit->unit.active_camouflage_super_amount;
						model_effect.source_object_index= unit_index;
						model_effect.source_object_centroid= render.camera.position;
					}
					else
					{
						model_effect.type= _render_model_effect_type_none;
					}

					if (first_person_weapon->weapon_node_remapping_table_valid &&
						weapon_definition->weapon.interface_definition.first_person_model.index!=NONE)
					{
						model_remap_node_matrices_to_match_animation_graph(
							weapon_definition->weapon.interface_definition.first_person_model.index,
							model_node_matrices,
							weapon_definition->weapon.interface_definition.first_person_animations.index,
							first_person_weapon->node_matrices,
							first_person_weapon->weapon_node_remapping_table);
						render_model(
							weapon_definition->weapon.interface_definition.first_person_model.index,
							0.0f,
							model_node_matrices,
							NULL,
							weapon->object.outgoing_change_colors,
							weapon_functions,
							lighting,
							&render.camera.position,
							0.0f,
							&model_effect,
							first_person_weapon->weapon_index,
							0,
							FLAG(_render_model_first_person_bit));
					}

					/* port: the shield's flare on the arms as on the body, the
					unit's own modifier shader (display.viewmodel_shield,
					view_fov.c), in the color chosen for it as on the body
					(display.shield_color, shield_color.c); the weapon above is
					drawn without it */
					if (viewmodel_shield_is_visible())
					{
						struct object_definition *unit_definition= object_definition_get(object_get(unit_index)->definition_index);

						if (unit_definition->object.modifier_shader.index!=NONE)
						{
							struct shader *modifier_shader= shader_definition_get(unit_definition->object.modifier_shader.index);

							if (shader_type_is_valid_for_modifier(modifier_shader->base.type))
							{
								model_effect.modifier_shader= modifier_shader;
								model_effect.modifier_animation.colors= shield_color_colors(unit_index,
									unit->object.outgoing_change_colors);
								model_effect.modifier_animation.values= overshield_effect_values(unit_index,
									unit_functions, TRUE);
							}
						}
					}

					if (first_person_weapon->hands_node_remapping_table_valid &&
						first_person_interface->hands.index!=NONE)
					{
						model_remap_node_matrices_to_match_animation_graph(
							first_person_interface->hands.index,
							model_node_matrices,
							weapon_definition->weapon.interface_definition.first_person_animations.index,
							first_person_weapon->node_matrices,
							first_person_weapon->hands_node_remapping_table);
						render_model(
							first_person_interface->hands.index,
							0.0f,
							model_node_matrices,
							NULL,
							unit->object.outgoing_change_colors,
							unit_functions,
							lighting,
							&render.camera.position,
							0.0f,
							&model_effect,
							first_person_weapon->weapon_index,
							0,
							FLAG(_render_model_first_person_bit));
					}
				}
			}
		}
	}

	return;
}

short first_person_weapon_get_marker_by_name(
	long weapon_index,
	char const *name,
	struct object_marker *markers,
	short maximum_marker_count)
{
	short marker_count= 0;
	struct weapon_datum *weapon= weapon_try_and_get(weapon_index);

	if (weapon)
	{
		short local_player_index= first_person_weapon_index_from_weapon_index(weapon_index);

		if (local_player_index!=NONE &&
			director_get_perspective(local_player_index)==_director_perspective_first_person)
		{
			struct first_person_weapon *first_person_weapon= first_person_weapon_get(local_player_index);
			struct weapon_definition *weapon_definition= weapon_definition_get(weapon->definition_index);

			if (first_person_weapon->weapon_node_remapping_table_valid &&
				weapon_definition->weapon.interface_definition.first_person_model.index!=NONE &&
				weapon_definition->weapon.interface_definition.first_person_animations.index!=NONE)
			{
				marker_count= model_get_marker_by_name(
					weapon_definition->weapon.interface_definition.first_person_model.index,
					name,
					NULL,
					first_person_weapon->weapon_node_remapping_table,
					(short)animation_graph_definition_get(
						weapon_definition->weapon.interface_definition.first_person_animations.index)->nodes.count,
					first_person_weapon->node_matrices,
					FALSE,
					markers,
					maximum_marker_count);
			}
		}
	}

	return marker_count;
}

void first_person_weapon_render_update(
	void)
{
	if (render.local_player_index!=NONE)
	{
		struct first_person_weapon *first_person_weapon= first_person_weapon_get(render.local_player_index);

		if (first_person_weapon->unit_index!=NONE &&
			first_person_weapon->weapon_index!=NONE)
		{
			boolean visible= director_get_perspective(render.local_player_index)==_director_perspective_first_person &&
				player_control_get_zoom_level(render.local_player_index)==NONE;

			first_person_weapon_set_visibility(render.local_player_index, visible);
			if (first_person_weapon->visible)
			{
				first_person_weapon_build_node_matrices(render.local_player_index);
			}
		}
	}

	return;
}

short first_person_weapon_get_marker_by_name_render(
	long weapon_index,
	char const *name,
	struct object_marker *markers,
	short maximum_marker_count)
{
	if (render.local_player_index==first_person_weapon_index_from_weapon_index(weapon_index))
	{
		return first_person_weapon_get_marker_by_name(
			weapon_index,
			name,
			markers,
			maximum_marker_count);
	}

	return 0;
}

boolean first_person_weapon_center_flashlight(
	long unit_index,
	real_point3d *position,
	real_vector3d *forward,
	real_vector3d *up)
{
	short local_player_index= first_person_weapon_index_from_unit_index(unit_index);
	boolean centered= FALSE; /* port: returned */

	if (local_player_index!=NONE)
	{
		struct first_person_weapon *first_person_weapon= first_person_weapon_get(local_player_index);

		if (first_person_weapon->visible)
		{
			struct object_marker marker;

			if (first_person_weapon_get_marker_by_name(
				first_person_weapon->weapon_index,
				"flashlight",
				&marker,
				1)>0)
			{
				position->x= marker.matrix.position.x - marker.matrix.forward.i*0.5f;
				position->y= marker.matrix.position.y - marker.matrix.forward.j*0.5f;
				position->z= marker.matrix.position.z - marker.matrix.forward.k*0.5f;
				*forward= marker.matrix.forward;
				*up= marker.matrix.up;
				centered= TRUE;
			}
		}
	}

	return centered;
}

boolean first_person_weapon_adjust_light(
	long weapon_index,
	char const *marker_name,
	real_point3d *position,
	real_vector3d *forward,
	real_vector3d *up)
{
	struct weapon_datum *weapon= weapon_get(weapon_index);
	struct unit_datum *unit= unit_get(weapon->object.parent_object_index);
	long player_index= unit->unit.player_index;
	boolean adjusted= FALSE;

	if (player_index!=NONE)
	{
		struct player_datum *player= player_get(player_index);
		short local_player_index= player->local_player_index;
		struct object_marker marker;

		if (local_player_index!=NONE &&
			local_player_index==render.local_player_index &&
			first_person_weapon_get(local_player_index)->visible &&
			first_person_weapon_get_marker_by_name(weapon_index, marker_name, &marker, 1)>0)
		{
			*position= marker.matrix.position;
			*forward= marker.matrix.forward;
			*up= marker.matrix.up;
			adjusted= TRUE;
		}
	}

	return adjusted;
}

void first_person_weapon_message_from_unit(
	long unit_index,
	short message_type)
{
	short local_player_index= first_person_weapon_index_from_unit_index(unit_index);

	first_person_weapon_message(local_player_index, message_type);
	if (local_player_index==NONE)
	{
		struct unit_datum *unit= unit_get(unit_index);

		/* port: the weapon in hand's object (not its slot's number, which
		played some other object's sound) */
		if (unit->unit.current_weapon_index>=0 &&
			unit->unit.current_weapon_index<MAXIMUM_WEAPONS_PER_UNIT)
		{
			weapon_play_first_person_weapon_sound(
				unit->unit.weapon_object_indices[unit->unit.current_weapon_index],
				message_type);
		}
	}

	return;
}

void first_person_weapon_message_from_weapon(
	long weapon_index,
	short message_type)
{
	short local_player_index= first_person_weapon_index_from_weapon_index(weapon_index);

	first_person_weapon_message(local_player_index, message_type);
	if (local_player_index==NONE)
	{
		weapon_play_first_person_weapon_sound(weapon_index, message_type);
	}

	return;
}

struct real_matrix4x3 *first_person_weapon_get_node_matrix(
	short local_player_index,
	short node_index)
{
	struct first_person_weapon *first_person_weapon;
	struct weapon_datum *weapon;
	struct weapon_definition *weapon_definition;
	struct animation_graph *animation_graph;

	match_assert(
		"c:\\halo\\SOURCE\\interface\\first_person_weapons.c",
		1433,
		local_player_index>=0 && local_player_index<MAXIMUM_NUMBER_OF_LOCAL_PLAYERS);
	first_person_weapon = &first_person_weapons[local_player_index];
	/* port: the last matrices built when the weapon is gone (an effect or
	particle on it asking before first_person_weapon_forget_weapon stopped
	them was a NULL weapon, and a crash) */
	weapon = first_person_weapon->weapon_index!=NONE ? weapon_try_and_get(first_person_weapon->weapon_index) : NULL;
	if (!weapon)
	{
		return &first_person_weapon->node_matrices[
			node_index>=0 && node_index<MAXIMUM_NODES_PER_ANIMATION ? node_index : 0];
	}
	weapon_definition = weapon_definition_get(weapon->definition_index);
	animation_graph = animation_graph_definition_get(
		weapon_definition->weapon.interface_definition.first_person_animations.index);
	match_assert(
		"c:\\halo\\SOURCE\\interface\\first_person_weapons.c",
		718,
		node_index>=0 && node_index<animation_graph->nodes.count);

	return &first_person_weapon->node_matrices[node_index];
}

void first_person_weapons_update(
	void)
{
	short local_player_index;

	for (local_player_index=0; local_player_index<MAXIMUM_NUMBER_OF_LOCAL_PLAYERS; local_player_index++)
	{
		long player_index= local_player_get_player_index(local_player_index);

		if (player_index!=NONE)
		{
			struct first_person_weapon *first_person_weapon= first_person_weapon_get(local_player_index);
			long unit_index= player_get(player_index)->unit_index;

			if (first_person_weapon->unit_index!=unit_index)
			{
				first_person_weapon_new_unit(local_player_index, unit_index);
			}
			if (first_person_weapon->weapon_index==NONE)
			{
				first_person_weapon_switch_weapons(local_player_index);
			}
			first_person_weapon_update(local_player_index);
		}
	}

	return;
}

/* ---------- private code */

static void first_person_weapon_set_visibility(
	short local_player_index,
	boolean visible)
{
	struct first_person_weapon *first_person_weapon= first_person_weapon_get(local_player_index);

	if (visible!=first_person_weapon->visible)
	{
		if (visible)
		{
			match_assert(
				"c:\\halo\\SOURCE\\interface\\first_person_weapons.c",
				1555,
				first_person_weapon->weapon_index!=NONE);
			effects_start_on_first_person_weapon(local_player_index, first_person_weapon->weapon_index);
		}
		else
		{
			effects_stop_on_first_person_weapon(local_player_index);
			particles_stop_on_first_person_weapon(local_player_index);
		}
		first_person_weapon->visible= visible;
	}

	return;
}

/* port: the weapon dropped or deleted: hidden first, which stops the effects
and particles that follow it in first person (they asked for the weapon's
nodes, and found none, until the next update switched weapons; the
renderer shows the next weapon again) */
static void first_person_weapon_forget_weapon(
	short local_player_index)
{
	first_person_weapon_set_visibility(local_player_index, FALSE);
	first_person_weapon_get(local_player_index)->weapon_index= NONE;

	return;
}

static void first_person_weapon_predict(
	short local_player_index)
{
	struct first_person_weapon *first_person_weapon= first_person_weapon_get(local_player_index);

	if (first_person_weapon->weapon_index!=NONE)
	{
		struct weapon_datum *weapon= weapon_get(first_person_weapon->weapon_index);
		struct weapon_definition *weapon_definition= weapon_definition_get(weapon->definition_index);

		predicted_resources_precache(&weapon_definition->weapon.predicted_resources);
	}
	first_person_weapon->ticks_until_predict= TICKS_PER_SECOND;

	return;
}

static void first_person_weapon_start_interpolation(
	short local_player_index,
	short frame_count)
{
	struct first_person_weapon *first_person_weapon= first_person_weapon_get(local_player_index);
	struct weapon_datum *weapon= weapon_get(first_person_weapon->weapon_index);
	struct weapon_definition *weapon_definition= weapon_definition_get(weapon->definition_index);
	struct animation_graph *animation_graph= animation_graph_definition_get(
		weapon_definition->weapon.interface_definition.first_person_animations.index);

	/* port: no more nodes than the weapon's orientations hold (a map's count) */
	csmemcpy(
		first_person_weapon->original_node_orientations,
		first_person_weapon->node_orientations,
		PIN(animation_graph->nodes.count, 0, MAXIMUM_NODES_PER_ANIMATION)*sizeof(real_orientation));
	if (frame_count>=first_person_weapon->interpolation_frame_count-
		first_person_weapon->interpolation_frame_index)
	{
		first_person_weapon->interpolation_frame_index= 0;
		first_person_weapon->interpolation_frame_count= frame_count;
	}

	return;
}

static void first_person_weapon_set_state(
	short local_player_index,
	short new_state,
	boolean reset_sounds)
{
	struct first_person_weapon *first_person_weapon= first_person_weapon_get(local_player_index);

	if (first_person_weapon->weapon_index!=NONE)
	{
		struct weapon_datum *weapon= weapon_get(first_person_weapon->weapon_index);

		if (TEST_FLAG(weapon->weapon.flags, _weapon_overheated_bit))
		{
			switch (new_state)
			{
				case _first_person_weapon_state_ready:
					new_state= _first_person_weapon_state_overheating_again;
					break;

				case _first_person_weapon_state_throw_grenade:
					new_state= _first_person_weapon_state_throw_grenade_overheated;
					break;
			}
		}
	}

	switch (new_state)
	{
		case _first_person_weapon_state_ready:
			if (first_person_weapon->state==_first_person_weapon_state_ready)
			{
				new_state= NONE;
			}
			break;

		case _first_person_weapon_state_light_off:
		case _first_person_weapon_state_light_on:
			if (first_person_weapon->state!=_first_person_weapon_state_idle &&
				first_person_weapon->state!=_first_person_weapon_state_posing)
			{
				new_state= NONE;
			}
			break;

		case _first_person_weapon_state_primary_fire:
		case _first_person_weapon_state_secondary_fire:
		case _first_person_weapon_state_primary_misfire:
		case _first_person_weapon_state_secondary_misfire:
			if (first_person_weapon->state!=_first_person_weapon_state_idle &&
				first_person_weapon->state!=_first_person_weapon_state_posing &&
				first_person_weapon->state!=_first_person_weapon_state_primary_fire &&
				first_person_weapon->state!=_first_person_weapon_state_charged &&
				first_person_weapon->state!=_first_person_weapon_state_shotgun_enter_reload &&
				first_person_weapon->state!=_first_person_weapon_state_overheated_exit &&
				first_person_weapon->state!=_first_person_weapon_state_shotgun_exit_reload_empty &&
				first_person_weapon->state!=_first_person_weapon_state_shotgun_exit_reload_full &&
				first_person_weapon->state!=_first_person_weapon_state_reload_while_empty &&
				first_person_weapon->state!=_first_person_weapon_state_reload_while_full)
			{
				new_state= NONE;
			}
			break;
	}

	if (new_state!=NONE && first_person_weapon->weapon_index!=NONE)
	{
		struct weapon_datum *weapon= weapon_get(first_person_weapon->weapon_index);
		struct weapon_definition *weapon_definition= weapon_definition_get(weapon->definition_index);
		short animation_type;
		short interpolation_frame_count;

		if (weapon_definition->weapon.weapon_type==_weapon_type_plasma_pistol &&
			new_state==_first_person_weapon_state_overheated &&
			!TEST_FLAG(weapon->weapon.flags, _weapon_overheated_bit))
		{
			new_state= _first_person_weapon_state_idle;
		}
		animation_type= (short)first_person_animation_type_from_weapon_state(new_state);
		if (weapon_definition->weapon.weapon_type==_weapon_type_shotgun &&
			first_person_weapon->state==_first_person_weapon_state_shotgun_exit_reload_empty)
		{
			interpolation_frame_count= 0;
		}
		else
		{
			switch (new_state)
			{
				case _first_person_weapon_state_overheated:
				case _first_person_weapon_state_melee:
				case _first_person_weapon_state_ready:
					interpolation_frame_count= 0;
					break;

				case _first_person_weapon_state_primary_fire:
				case _first_person_weapon_state_secondary_fire:
				case _first_person_weapon_state_primary_misfire:
				case _first_person_weapon_state_secondary_misfire:
					interpolation_frame_count= 3;
					break;

				default:
					interpolation_frame_count= 6;
					break;
			}
		}

		if (first_person_weapon->unit_index!=NONE &&
			first_person_weapon->weapon_index!=NONE)
		{
			struct weapon_datum *weapon= weapon_get(first_person_weapon->weapon_index);
			struct weapon_definition *weapon_definition= weapon_definition_get(weapon->definition_index);
			struct animation_graph *animation_graph= animation_graph_definition_get(
				weapon_definition->weapon.interface_definition.first_person_animations.index);

			if (animation_graph->first_person_weapon_animations.count)
			{
				struct animation_graph_first_person_weapon_animations *first_person_weapon_animations=
					TAG_BLOCK_GET_ELEMENT(
						&animation_graph->first_person_weapon_animations,
						0,
						struct animation_graph_first_person_weapon_animations);

				if (first_person_weapon_animations &&
					animation_type>=0 &&
					animation_type<first_person_weapon_animations->animations.count)
				{
					short animation_index= animation_graph_animation_index_get(
						&first_person_weapon_animations->animations)[animation_type].animation_index;

					if (animation_index!=NONE)
					{
						if (reset_sounds &&
							first_person_weapon->current_sound_index!=NONE &&
							first_person_weapon->current_sound_state!=_first_person_weapon_state_overheating)
						{
							sound_stop_impulse(first_person_weapon->current_sound_index);
							first_person_weapon->current_sound_index= NONE;
							first_person_weapon->current_sound_state= NONE;
						}
						if (interpolation_frame_count>0)
						{
							first_person_weapon_start_interpolation(
								local_player_index,
								interpolation_frame_count);
						}
						first_person_weapon->state= new_state;
						first_person_weapon->state_animation.index= animation_index;
						first_person_weapon->state_animation.frame_index= 0;
					}
				}
			}
		}
	}

	return;
}

/* port: the weapon's orientations are the animation graph's nodes (64 at
most, the weapon's arrays): an animation of more nodes than the graph has,
or than the arrays hold (the map's counts), is not applied, said once. Every
retail first-person animation has the graph's count (up to 54). That is not
the first-person model's count (up to 17, the weapon without the hands),
which every retail animation is above. */
static boolean first_person_weapon_animation_fits(
	struct animation_graph const *animation_graph,
	struct animation const *animation)
{
	static boolean too_many_nodes_reported = FALSE;

	if (animation->node_count>=0 &&
		animation->node_count<=animation_graph->nodes.count &&
		animation->node_count<=MAXIMUM_NODES_PER_ANIMATION)
	{
		return TRUE;
	}

	if (!too_many_nodes_reported)
	{
		too_many_nodes_reported= TRUE;
		error(
			_error_silent,
			"first person animation %.32s of %d nodes (the graph has %d) not applied",
			animation->name,
			animation->node_count,
			animation_graph->nodes.count);
	}

	return FALSE;
}

static void first_person_weapon_build_node_matrices(
	short local_player_index)
{
	struct first_person_weapon *first_person_weapon= first_person_weapon_get(local_player_index);

	if (!first_person_weapon->rendered)
	{
		euler_angles2d_from_vector3d(&first_person_weapon->render_facing, &render.camera.forward);
		first_person_weapon->render_position= render.camera.position;
	}
	/* The native ports draw several frames per tick. The turning sway
	(first_person_weapon_update) takes the facing change since the last
	frame as a tick's worth: move the last facing on once a tick. */
	{
		static long last_render_ticks[MAXIMUM_LOCAL_PLAYERS] = { NONE, NONE, NONE, NONE };

		if (!halo_interpolation_enabled() || last_render_ticks[local_player_index] != game_time_get())
		{
			first_person_weapon->last_render_facing= first_person_weapon->render_facing;
			first_person_weapon->last_render_position= first_person_weapon->render_position;
			last_render_ticks[local_player_index]= game_time_get();
		}
	}
	euler_angles2d_from_vector3d(&first_person_weapon->render_facing, &render.camera.forward);
	first_person_weapon->render_position= render.camera.position;
	first_person_weapon->render_forward= render.camera.forward;
	first_person_weapon->rendered= TRUE;

	if (first_person_weapon->weapon_index!=NONE &&
		!weapon_try_and_get(first_person_weapon->weapon_index))
	{
		error(
			_error_log,
			"local player %d, weapon (0x%x), deleted unexpectedly",
			local_player_index,
			first_person_weapon->weapon_index);
		first_person_weapon_forget_weapon(local_player_index);
	}

	if (first_person_weapon->weapon_index!=NONE)
	{
		struct weapon_datum *weapon= weapon_get(first_person_weapon->weapon_index);
		struct weapon_definition *weapon_definition= weapon_definition_get(weapon->definition_index);
		struct model *model= model_definition_get(
			weapon_definition->weapon.interface_definition.first_person_model.index);
		struct animation_graph *animation_graph= animation_graph_definition_get(
			weapon_definition->weapon.interface_definition.first_person_animations.index);

		if (animation_graph->first_person_weapon_animations.count)
		{
			struct animation_graph_first_person_weapon_animations *first_person_weapon_animations=
				TAG_BLOCK_GET_ELEMENT(
					&animation_graph->first_person_weapon_animations,
					0,
					struct animation_graph_first_person_weapon_animations);

			if (first_person_weapon_animations)
			{
				short firing_animation_index;
				short overlay_animation_index;

				if (first_person_weapon->state_animation.index!=NONE)
				{
					struct animation *state_animation= TAG_BLOCK_GET_ELEMENT(
						&animation_graph->animations,
						first_person_weapon->state_animation.index,
						struct animation);

					animation_get_node_orientations(
						NULL,
						state_animation,
						first_person_weapon->state_animation.frame_index,
						first_person_weapon->node_orientations);
					/* port: a PAL map's animation, slowed to the NTSC maps' pace,
					between the frame it is on and the next: held on the frame, it
					stood still for that tick, and a reload moved in fits and starts
					(port/linux/game/pal_tags.c) */
					{
						real fraction= pal_tags_first_person_fraction(
							local_player_index,
							weapon_definition->weapon.interface_definition.first_person_animations.index,
							first_person_weapon->state_animation.index,
							first_person_weapon->state_animation.frame_index);

						if (fraction>0.0f &&
							first_person_weapon->state_animation.frame_index+1<state_animation->frame_count)
						{
							real_orientation next_node_orientations[MAXIMUM_NODES_PER_ANIMATION];
							short node_index;

							animation_get_node_orientations(
								NULL,
								state_animation,
								(short)(first_person_weapon->state_animation.frame_index+1),
								next_node_orientations);
							/* port: no more nodes than the orientations hold (a
							map's count) */
							for (node_index= 0; node_index<MIN(state_animation->node_count, MAXIMUM_NODES_PER_ANIMATION); node_index++)
							{
								orientations_interpolate(
									&first_person_weapon->node_orientations[node_index],
									&next_node_orientations[node_index],
									fraction,
									&first_person_weapon->node_orientations[node_index]);
							}
						}
					}
				}
				else
				{
					match_vassert(
						"c:\\halo\\SOURCE\\interface\\first_person_weapons.c",
						1294,
						FALSE,
						NULL);
					model_get_node_orientations(model, first_person_weapon->node_orientations);
				}

				firing_animation_index= VALID_INDEX(
					_first_person_weapon_animation_ammunition,
					first_person_weapon_animations->animations.count) ?
					animation_graph_animation_index_get(
						&first_person_weapon_animations->animations)
							[_first_person_weapon_animation_ammunition].animation_index :
					(short)NONE;
				if (firing_animation_index!=NONE)
				{
					struct animation *firing_animation= TAG_BLOCK_GET_ELEMENT(
						&animation_graph->animations,
						firing_animation_index,
						struct animation);

					/* port: and one of the graph's nodes (see first_person_weapon_animation_fits) */
					if (!first_person_weapon_animation_fits(animation_graph, firing_animation))
					{
						/* (not applied) */
					}
					else if (weapon_definition->weapon.weapon_type==_weapon_type_needler &&
						(first_person_weapons->state==_first_person_weapon_state_reload_while_empty ||
						first_person_weapons->state==_first_person_weapon_state_reload_while_full))
					{
						short frame_index= weapon->weapon.magazines[0].rounds_loaded;
						short settle_ticks= weapon->weapon.magazines[0].original_time-
							weapon->weapon.magazines[0].state_timer;

						if (settle_ticks>=FIRST_PERSON_WEAPON_RELOAD_SETTLE_TICKS)
						{
							struct weapon_magazine_definition *magazine_definition= TAG_BLOCK_GET_ELEMENT(
								&weapon_definition->weapon.magazines,
								0,
								struct weapon_magazine_definition);
							real settle_fraction= (settle_ticks-FIRST_PERSON_WEAPON_RELOAD_SETTLE_TICKS)*0.2f;
							short predicted_frame_index;

							if (settle_fraction>1.0f)
							{
								settle_fraction= 1.0f;
							}
							predicted_frame_index= (short)MIN(
								weapon->weapon.magazines[0].rounds_total,
								magazine_definition->rounds_loaded_maximum);
							frame_index= (short)((predicted_frame_index-frame_index)*settle_fraction)+
								frame_index;
						}
						overlay_animation_apply(
							firing_animation,
							frame_index,
							first_person_weapon->node_orientations);
					}
					else if (weapon->weapon.magazines[0].rounds_loaded<firing_animation->frame_count)
					{
						overlay_animation_apply(
							firing_animation,
							weapon->weapon.magazines[0].rounds_loaded,
							first_person_weapon->node_orientations);
					}
				}

				if (first_person_weapon->moving_animation.index!=NONE)
				{
					struct animation *moving_animation= TAG_BLOCK_GET_ELEMENT(
						&animation_graph->animations,
						first_person_weapon->moving_animation.index,
						struct animation);

					/* port: and one of the graph's nodes */
					if (first_person_weapon_animation_fits(animation_graph, moving_animation))
					{
						overlay_animation_apply(
							moving_animation,
							first_person_weapon->moving_animation.frame_index,
							first_person_weapon->node_orientations);
					}
				}

				if (first_person_weapon->overcharged_jitter_animation.index!=NONE)
				{
					struct animation *overcharged_jitter_animation= TAG_BLOCK_GET_ELEMENT(
						&animation_graph->animations,
						first_person_weapon->overcharged_jitter_animation.index,
						struct animation);

					/* port: and one of the graph's nodes */
					if (first_person_weapon_animation_fits(animation_graph, overcharged_jitter_animation))
					{
						overlay_animation_apply_continuous_scaled(
							overcharged_jitter_animation,
							first_person_weapon->overcharged_jitter_animation.frame_index,
							weapon->weapon.overcharged+0.5f,
							first_person_weapon->node_orientations);
					}
				}

				overlay_animation_index= VALID_INDEX(
					_first_person_weapon_animation_overlays,
					first_person_weapon_animations->animations.count) ?
					animation_graph_animation_index_get(
						&first_person_weapon_animations->animations)
							[_first_person_weapon_animation_overlays].animation_index :
					(short)NONE;
				if (overlay_animation_index!=NONE)
				{
					struct animation *overlay_animation= TAG_BLOCK_GET_ELEMENT(
						&animation_graph->animations,
						overlay_animation_index,
						struct animation);

					/* port: and one of the graph's nodes */
					if (overlay_animation->frame_count>=NUMBER_OF_FIRST_PERSON_WEAPON_OVERLAY_FRAMES &&
						first_person_weapon_animation_fits(animation_graph, overlay_animation))
					{
						if (first_person_weapon->position.i>0.0f)
						{
							overlay_animation_apply_scaled(
								overlay_animation,
								0,
								first_person_weapon->position.i,
								first_person_weapon->node_orientations);
						}
						else if (first_person_weapon->position.i<0.0f)
						{
							overlay_animation_apply_scaled(
								overlay_animation,
								1,
								-first_person_weapon->position.i,
								first_person_weapon->node_orientations);
						}

						if (first_person_weapon->position.j>0.0f)
						{
							overlay_animation_apply_scaled(
								overlay_animation,
								3,
								first_person_weapon->position.j,
								first_person_weapon->node_orientations);
						}
						else if (first_person_weapon->position.j<0.0f)
						{
							overlay_animation_apply_scaled(
								overlay_animation,
								2,
								-first_person_weapon->position.j,
								first_person_weapon->node_orientations);
						}

						if (first_person_weapon->turning.i>0.0f)
						{
							overlay_animation_apply_scaled(
								overlay_animation,
								4,
								first_person_weapon->turning.i,
								first_person_weapon->node_orientations);
						}
						else if (first_person_weapon->turning.i<0.0f)
						{
							overlay_animation_apply_scaled(
								overlay_animation,
								5,
								-first_person_weapon->turning.i,
								first_person_weapon->node_orientations);
						}

						if (first_person_weapon->turning.j>0.0f)
						{
							overlay_animation_apply_scaled(
								overlay_animation,
								7,
								first_person_weapon->turning.j,
								first_person_weapon->node_orientations);
						}
						else if (first_person_weapon->turning.j<0.0f)
						{
							overlay_animation_apply_scaled(
								overlay_animation,
								6,
								-first_person_weapon->turning.j,
								first_person_weapon->node_orientations);
						}

						if (first_person_weapon->firing_push_back>0.0f)
						{
							overlay_animation_apply_scaled(
								overlay_animation,
								8,
								first_person_weapon->firing_push_back,
								first_person_weapon->node_orientations);
						}
					}
				}

				if (first_person_weapon->interpolation_frame_count>0)
				{
					interpolate_node_orientations(
						(short)animation_graph->nodes.count,
						first_person_weapon->original_node_orientations,
						first_person_weapon->node_orientations,
						first_person_weapon->interpolation_frame_index,
						first_person_weapon->interpolation_frame_count);
				}
			}
		}

		animation_graph_node_matrices_from_orientations(
			weapon_definition->weapon.interface_definition.first_person_animations.index,
			first_person_weapon->node_matrices,
			first_person_weapon->node_orientations,
			&render.camera.position,
			&render.camera.forward,
			&render.camera.up);
		/* the pose between the last two ticks (render_interpolation.c) */
		/* port: no more nodes than the weapon's matrices hold (a map's count);
		the weapon and its graph tell its poses from another weapon's */
		render_interpolation_first_person(
			local_player_index,
			first_person_weapon->weapon_index,
			weapon_definition->weapon.interface_definition.first_person_animations.index,
			first_person_weapon->node_matrices,
			(short)MIN(animation_graph->nodes.count, MAXIMUM_NODES_PER_ANIMATION),
			&render.camera);
	}

	return;
}

static void first_person_weapon_message(
	short local_player_index,
	short message_type)
{
	if (local_player_index!=NONE)
	{
		struct first_person_weapon *first_person_weapon= first_person_weapon_get(local_player_index);
		short state= NONE;

		switch (message_type)
		{
			case _first_person_weapon_message_drop:
				first_person_weapon_forget_weapon(local_player_index);
				break;

			case _first_person_weapon_message_ready:
				first_person_weapon_switch_weapons(local_player_index);
				break;

			case _first_person_weapon_message_primary_fire:
				first_person_weapon->firing_push_back_velocity+=
					FIRST_PERSON_WEAPON_FIRING_PUSH_BACK_VELOCITY;
				break;

			case _first_person_weapon_message_reload_while_empty:
			case _first_person_weapon_message_reload_while_full:
				player_control_unzoom(first_person_weapon->unit_index);
				break;
		}

		if (first_person_weapon->weapon_index!=NONE)
		{
			struct weapon_datum *weapon= weapon_get(first_person_weapon->weapon_index);

			if (weapon->definition_index!=NONE)
			{
				struct weapon_definition *weapon_definition= weapon_definition_get(weapon->definition_index);

				switch (weapon_definition->weapon.weapon_type)
				{
					case _weapon_type_shotgun:
						if (message_type==_first_person_weapon_message_reload_while_empty ||
							message_type==_first_person_weapon_message_reload_while_full)
						{
							struct weapon_magazine_definition *magazine_definition= TAG_BLOCK_GET_ELEMENT(
								&weapon_definition->weapon.magazines,
								0,
								struct weapon_magazine_definition);
							short rounds_loaded= weapon->weapon.magazines[0].rounds_loaded;
							short rounds_total= weapon->weapon.magazines[0].rounds_total;

							if (first_person_weapon->state==_first_person_weapon_state_shotgun_enter_reload ||
								first_person_weapon->state==_first_person_weapon_state_overheated_exit ||
								first_person_weapon->state==_first_person_weapon_state_shotgun_exit_reload_empty ||
								first_person_weapon->state==_first_person_weapon_state_shotgun_exit_reload_full ||
								first_person_weapon->state==_first_person_weapon_state_reload_while_empty ||
								first_person_weapon->state==_first_person_weapon_state_reload_while_full ||
								weapon->weapon.magazines[0].state)
							{
								if (MIN(magazine_definition->rounds_loaded_maximum-rounds_loaded, rounds_total)==1)
								{
									first_person_weapon->shotgun_reload_type= _shotgun_reload_type_last_round;
								}
								else
								{
									first_person_weapon->shotgun_reload_type= NONE;
								}
							}
							else
							{
								short shells_to_reload= (short)MIN(
									magazine_definition->rounds_loaded_maximum-rounds_loaded,
									rounds_total);

								first_person_weapon->shotgun_shells_to_reload= shells_to_reload;
								first_person_weapon->shotgun_empty= rounds_loaded==0;
								first_person_weapon->shotgun_reload_type= (shells_to_reload!=1) ?
									_shotgun_reload_type_first_round :
									_shotgun_reload_type_first_and_last_round;
							}

							switch (first_person_weapon->shotgun_reload_type)
							{
								case _shotgun_reload_type_first_round:
								case _shotgun_reload_type_first_and_last_round:
									state= _first_person_weapon_state_shotgun_enter_reload;
									break;

								case NONE:
									state= _first_person_weapon_state_reload_while_empty;
									break;
							}
						}
						break;
				}
			}
		}

		if (state==NONE)
		{
			state= (short)first_person_weapon_state_from_weapon_message(message_type);
		}
		if (state!=NONE)
		{
			first_person_weapon_set_state(local_player_index, state, TRUE);
		}
		switch (message_type)
		{
			case _first_person_weapon_message_ready:
				first_person_weapon->interpolation_frame_count= 0;
				break;
		}
	}

	return;
}

static void first_person_weapon_switch_weapons(
	short local_player_index)
{
	struct first_person_weapon *first_person_weapon= first_person_weapon_get(local_player_index);
	boolean visible= first_person_weapon->visible;

	first_person_weapon->weapon_index= NONE;
	if (visible)
	{
		first_person_weapon_set_visibility(local_player_index, FALSE);
	}

	if (first_person_weapon->unit_index!=NONE)
	{
		struct unit_datum *unit= unit_get(first_person_weapon->unit_index);
		long weapon_index= unit_inventory_get_weapon(
			first_person_weapon->unit_index,
			unit->unit.current_weapon_index);

		if (weapon_index!=NONE)
		{
			struct weapon_datum *weapon= weapon_get(weapon_index);
			struct weapon_definition *weapon_definition= weapon_definition_get(weapon->definition_index);

			if (weapon_definition->weapon.interface_definition.first_person_model.index!=NONE &&
				weapon_definition->weapon.interface_definition.first_person_animations.index!=NONE)
			{
				struct animation_graph *animation_graph= animation_graph_definition_get(
					weapon_definition->weapon.interface_definition.first_person_animations.index);

				if (animation_graph->first_person_weapon_animations.count)
				{
					struct animation_graph_first_person_weapon_animations *first_person_weapon_animations=
						TAG_BLOCK_GET_ELEMENT(
							&animation_graph->first_person_weapon_animations,
							0,
							struct animation_graph_first_person_weapon_animations);

					if (first_person_weapon_animations)
					{
						struct game_globals_first_person_interface *first_person_interface;

						first_person_weapon->overlay_animation_index= NONE;
						if (VALID_INDEX(
							_first_person_weapon_animation_overlays,
							first_person_weapon_animations->animations.count))
						{
							short overlay_animation_index= animation_graph_animation_index_get(
								&first_person_weapon_animations->animations)
									[_first_person_weapon_animation_overlays].animation_index;

							if (overlay_animation_index!=NONE &&
								TAG_BLOCK_GET_ELEMENT(
									&animation_graph->animations,
									overlay_animation_index,
									struct animation)->frame_count>=
										NUMBER_OF_FIRST_PERSON_WEAPON_OVERLAY_FRAMES)
							{
								first_person_weapon->overlay_animation_index= overlay_animation_index;
							}
						}

						first_person_interface= TAG_BLOCK_GET_ELEMENT(
							&scenario_get_game_globals()->first_person_interface,
							0,
							struct game_globals_first_person_interface);
						if (first_person_interface->hands.index!=NONE)
						{
							first_person_weapon->hands_node_remapping_table_valid=
								model_build_remapping_table_for_animation_graph(
									first_person_interface->hands.index,
									weapon_definition->weapon.interface_definition.first_person_animations.index,
									first_person_weapon->hands_node_remapping_table);
						}
						first_person_weapon->weapon_node_remapping_table_valid=
							model_build_remapping_table_for_animation_graph(
								weapon_definition->weapon.interface_definition.first_person_model.index,
								weapon_definition->weapon.interface_definition.first_person_animations.index,
								first_person_weapon->weapon_node_remapping_table);

						if (first_person_weapon->weapon_node_remapping_table_valid &&
							first_person_weapon->hands_node_remapping_table_valid)
						{
							first_person_weapon->weapon_index= weapon_index;
							first_person_weapon->state= NONE;
							first_person_weapon->state_animation.index= NONE;
							first_person_weapon->moving_animation.index= NONE;
							first_person_weapon->overcharged_jitter_animation.index= NONE;
							first_person_weapon->firing_push_back= 0.0f;
							first_person_weapon->firing_push_back_velocity= 0.0f;
							first_person_weapon->ticks_idle= 0;
							first_person_weapon->current_sound_index= NONE;
							first_person_weapon->current_sound_state= NONE;
							first_person_weapon_set_state(
								local_player_index,
								_first_person_weapon_state_idle,
								TRUE);
							first_person_weapon->interpolation_frame_count= 0;
							if (visible)
							{
								first_person_weapon_set_visibility(local_player_index, TRUE);
							}
						}
					}
				}
			}
		}
	}
	first_person_weapon_predict(local_player_index);

	return;
}

static void first_person_weapon_new_unit(
	short local_player_index,
	long unit_index)
{
	struct first_person_weapon *first_person_weapon= first_person_weapon_get(local_player_index);

	first_person_weapon->rendered= FALSE;
	first_person_weapon->unit_index= unit_index;
	first_person_weapon_switch_weapons(local_player_index);

	return;
}

static void first_person_weapon_next_state(
	short local_player_index)
{
	struct first_person_weapon *first_person_weapon= first_person_weapon_get(local_player_index);
	short new_state= NONE;

	switch (first_person_weapon->state)
	{
		case _first_person_weapon_state_idle:
		case _first_person_weapon_state_posing:
		case _first_person_weapon_state_primary_fire:
		case _first_person_weapon_state_secondary_fire:
		case _first_person_weapon_state_primary_misfire:
		case _first_person_weapon_state_secondary_misfire:
		case _first_person_weapon_state_melee:
		case _first_person_weapon_state_light_off:
		case _first_person_weapon_state_light_on:
		case _first_person_weapon_state_shotgun_exit_reload_empty:
		case _first_person_weapon_state_shotgun_exit_reload_full:
		case _first_person_weapon_state_ready:
		case _first_person_weapon_state_throw_grenade:
		case _first_person_weapon_state_overheated_exit:
			new_state= _first_person_weapon_state_idle;
			break;

		case _first_person_weapon_state_overheating:
		case _first_person_weapon_state_overheating_again:
		case _first_person_weapon_state_throw_grenade_overheated:
		case _first_person_weapon_state_overheating_super_recoil:
			new_state= _first_person_weapon_state_overheated;
			break;

		case _first_person_weapon_state_put_away:
			first_person_weapon->state_animation.frame_index--;
			break;

		case _first_person_weapon_state_shotgun_enter_reload:
		{
			struct weapon_datum *weapon= weapon_get(first_person_weapon->weapon_index);
			struct weapon_definition *weapon_definition= weapon_definition_get(weapon->definition_index);

			if (weapon_definition->weapon.weapon_type==_weapon_type_shotgun &&
				first_person_weapon->shotgun_reload_type==_shotgun_reload_type_first_and_last_round)
			{
				new_state= first_person_weapon->shotgun_empty ?
					_first_person_weapon_state_shotgun_exit_reload_empty :
					_first_person_weapon_state_shotgun_exit_reload_full;
			}
			else
			{
				new_state= _first_person_weapon_state_idle;
			}
			break;
		}

		case _first_person_weapon_state_reload_while_empty:
		case _first_person_weapon_state_reload_while_full:
		{
			struct weapon_datum *weapon= weapon_get(first_person_weapon->weapon_index);
			struct weapon_definition *weapon_definition= weapon_definition_get(weapon->definition_index);

			if (weapon_definition->weapon.weapon_type!=_weapon_type_shotgun ||
				!first_person_weapon->shotgun_reload_type ||
				first_person_weapon->shotgun_reload_type==NONE)
			{
				new_state= _first_person_weapon_state_idle;
			}
			else
			{
				match_assert(
					"c:\\halo\\SOURCE\\interface\\first_person_weapons.c",
					799,
					first_person_weapon->shotgun_reload_type == _shotgun_reload_type_first_and_last_round ||
						first_person_weapon->shotgun_reload_type == _shotgun_reload_type_last_round);
				new_state= first_person_weapon->shotgun_empty ?
					_first_person_weapon_state_shotgun_exit_reload_empty :
					_first_person_weapon_state_shotgun_exit_reload_full;
			}
			break;
		}
	}

	if (new_state!=NONE)
	{
		first_person_weapon_set_state(local_player_index, new_state, FALSE);
	}

	return;
}

static void first_person_weapon_update(
	short local_player_index)
{
	struct first_person_weapon *first_person_weapon= first_person_weapon_get(local_player_index);

	if (first_person_weapon->weapon_index!=NONE &&
		!weapon_try_and_get(first_person_weapon->weapon_index))
	{
		error(
			_error_log,
			"local player %d, weapon (0x%x), deleted unexpectedly",
			local_player_index,
			first_person_weapon->weapon_index);
		first_person_weapon_forget_weapon(local_player_index);
	}

	if (first_person_weapon->unit_index!=NONE && first_person_weapon->weapon_index!=NONE)
	{
		struct unit_datum *unit= unit_get(first_person_weapon->unit_index);
		struct weapon_datum *weapon= weapon_get(first_person_weapon->weapon_index);
		struct weapon_definition *weapon_definition= weapon_definition_get(weapon->definition_index);
		struct animation_graph *animation_graph;
		long sound_definition_index;
		short animation_update_result;
		boolean moving;

		model_definition_get(weapon_definition->weapon.interface_definition.first_person_model.index);
		animation_graph= animation_graph_definition_get(
			weapon_definition->weapon.interface_definition.first_person_animations.index);

		if (first_person_weapon->state==_first_person_weapon_state_overheated ||
			first_person_weapon->state==_first_person_weapon_state_overheating)
		{
			if (TEST_FLAG(weapon->weapon.flags, _weapon_overheated_exit_bit))
			{
				first_person_weapon_set_state(
					local_player_index,
					_first_person_weapon_state_overheated_exit,
					TRUE);
			}
			if (!TEST_FLAG(weapon->weapon.flags, _weapon_overheated_bit))
			{
				first_person_weapon_set_state(
					local_player_index,
					_first_person_weapon_state_idle,
					TRUE);
			}
		}

		/* port: a PAL map's first-person animation at the NTSC maps' pace,
		which the weapon's timing keeps (port/linux/game/pal_tags.c) */
		if (!pal_tags_first_person_advance(local_player_index,
			weapon_definition->weapon.interface_definition.first_person_animations.index,
			first_person_weapon->state_animation.index, first_person_weapon->state_animation.frame_index))
		{
			animation_update_result= _animation_no_key_frame;
			sound_definition_index= NONE;
		}
		else
		animation_update_result= animation_update_render_only(
			weapon_definition->weapon.interface_definition.first_person_animations.index,
			&first_person_weapon->state_animation,
			&sound_definition_index);
		if (animation_update_result==_animation_key_frame)
		{
			/* a key frame does not advance the first-person weapon state */
		}
		else if (animation_update_result==_animation_will_restart_on_next_frame)
		{
			first_person_weapon_next_state(local_player_index);
		}

		if (sound_definition_index!=NONE &&
			director_get_perspective(local_player_index)==_director_perspective_first_person)
		{
			first_person_weapon->current_sound_index= object_impulse_sound_new(
				first_person_weapon->weapon_index,
				sound_definition_index,
				NONE,
				global_origin3d,
				global_forward3d,
				1.0f);
			first_person_weapon->current_sound_state= first_person_weapon->state;
		}

		moving= magnitude3d(&unit->unit.throttle)>0.1f;
		if (unit_flying_through_air(first_person_weapon->unit_index))
		{
			moving= FALSE;
		}

		if (first_person_weapon->moving_animation.index!=NONE)
		{
			animation_update_render_only(
				weapon_definition->weapon.interface_definition.first_person_animations.index,
				&first_person_weapon->moving_animation,
				NULL);
			if (!moving)
			{
				if (first_person_weapon->state==_first_person_weapon_state_idle)
				{
					first_person_weapon_start_interpolation(local_player_index, 6);
				}
				first_person_weapon->moving_animation.index= NONE;
			}
		}
		else if (moving)
		{
			struct animation_graph_first_person_weapon_animations *first_person_weapon_animations=
				!animation_graph->first_person_weapon_animations.count ?
					NULL :
					TAG_BLOCK_GET_ELEMENT(
						&animation_graph->first_person_weapon_animations,
						0,
						struct animation_graph_first_person_weapon_animations);

			first_person_weapon->moving_animation.frame_index= 0;
			/* no NULL test (January, and the later first-party build at /Od 0x61fcea): a non-NONE
			 * weapon_index is only committed by first_person_weapon_switch_weapons inside
			 * `if (animation_graph->first_person_weapon_animations.count)` for this same graph. The
			 * new-map reset leaves weapon_index 0 but unit_index NONE, and first_person_weapon_new_unit
			 * runs switch_weapons (which stores NONE first) as soon as it sets unit_index, so the NULL
			 * arm above cannot reach this read. */
			first_person_weapon->moving_animation.index=
				VALID_INDEX(
					_first_person_weapon_animation_moving,
					first_person_weapon_animations->animations.count) ?
					animation_graph_animation_index_get(
						&first_person_weapon_animations->animations)
							[_first_person_weapon_animation_moving].animation_index :
					(short)NONE;
		}

		if (first_person_weapon->overcharged_jitter_animation.index==NONE)
		{
			if (first_person_weapon->state==_first_person_weapon_state_charged)
			{
				struct animation_graph_first_person_weapon_animations *first_person_weapon_animations=
					!animation_graph->first_person_weapon_animations.count ?
						NULL :
						TAG_BLOCK_GET_ELEMENT(
							&animation_graph->first_person_weapon_animations,
							0,
							struct animation_graph_first_person_weapon_animations);

				first_person_weapon->overcharged_jitter_animation.frame_index= 0.0f;
				/* no NULL test: the same switch_weapons invariant as the moving animation above
				 * (January and /Od 0x61fd96). */
				first_person_weapon->overcharged_jitter_animation.index=
					VALID_INDEX(
						_first_person_weapon_animation_overcharged_jitter,
						first_person_weapon_animations->animations.count) ?
						animation_graph_animation_index_get(
							&first_person_weapon_animations->animations)
								[_first_person_weapon_animation_overcharged_jitter].animation_index :
						(short)NONE;
			}
		}
		else if (first_person_weapon->state==_first_person_weapon_state_charged)
		{
			struct animation *animation= TAG_BLOCK_GET_ELEMENT(
				&animation_graph->animations,
				first_person_weapon->overcharged_jitter_animation.index,
				struct animation);

			first_person_weapon->overcharged_jitter_animation.frame_index= (real)fmod(
				(weapon->weapon.overcharged+1.0f)*2.0f+
					first_person_weapon->overcharged_jitter_animation.frame_index,
				(real)animation->frame_count);
		}
		else
		{
			first_person_weapon->overcharged_jitter_animation.index= NONE;
		}

		if (first_person_weapon->rendered)
		{
			real_vector2d turning;

			accelerate_to_position(
				&first_person_weapon->position.i,
				&first_person_weapon->position_velocity.i,
				unit->unit.throttle.i,
				0.08f,
				0.5f,
				-1.0f,
				1.0f,
				FALSE);
			accelerate_to_position(
				&first_person_weapon->position.j,
				&first_person_weapon->position_velocity.j,
				unit->unit.throttle.j,
				0.08f,
				0.5f,
				-1.0f,
				1.0f,
				FALSE);

			turning.i= signed_angular_difference(
				first_person_weapon->last_render_facing.yaw,
				first_person_weapon->render_facing.yaw)*30.0f;
			turning.j= signed_angular_difference(
				first_person_weapon->last_render_facing.pitch,
				first_person_weapon->render_facing.pitch)*-30.0f;

			turning.i= PIN(turning.i, -1.0f, 1.0f);
			turning.j= PIN(turning.j, -1.0f, 1.0f);

			accelerate_to_position(
				&first_person_weapon->turning.i,
				&first_person_weapon->turning_velocity.i,
				turning.i,
				0.03f,
				0.2f,
				-1.0f,
				1.0f,
				FALSE);
			accelerate_to_position(
				&first_person_weapon->turning.j,
				&first_person_weapon->turning_velocity.j,
				turning.j,
				0.03f,
				0.2f,
				-1.0f,
				1.0f,
				FALSE);
		}

		accelerate_to_position(
			&first_person_weapon->firing_push_back,
			&first_person_weapon->firing_push_back_velocity,
			0.0f,
			0.01f,
			0.2f,
			0.0f,
			1.0f,
			FALSE);
		if (first_person_weapon->firing_push_back==1.0f)
		{
			first_person_weapon->firing_push_back_velocity= 0.0f;
		}

		if (first_person_weapon->interpolation_frame_count>0)
		{
			first_person_weapon->interpolation_frame_index++;
			if (first_person_weapon->interpolation_frame_index>=
				first_person_weapon->interpolation_frame_count)
			{
				first_person_weapon->interpolation_frame_count= 0;
			}
		}

		if (player_control_get_autoaim_level(local_player_index)!=0.0f ||
			player_control_get_zoom_level(local_player_index)!=NONE ||
			first_person_weapon->firing_push_back!=0.0f ||
			first_person_weapon->position.i!=0.0f ||
			first_person_weapon->position.j!=0.0f ||
			first_person_weapon->turning.i!=0.0f ||
			first_person_weapon->turning.j!=0.0f)
		{
			first_person_weapon->ticks_idle= 0;
			if (first_person_weapon->state==_first_person_weapon_state_posing)
			{
				first_person_weapon_set_state(
					local_player_index,
					_first_person_weapon_state_idle,
					TRUE);
			}
		}
		else if (first_person_weapon->state==_first_person_weapon_state_idle)
		{
			struct game_globals_player_information *player_information= TAG_BLOCK_GET_ELEMENT(
				&scenario_get_game_globals()->player_information,
				0,
				struct game_globals_player_information);

			if (!first_person_weapon->ticks_until_pose)
			{
				first_person_weapon->ticks_until_pose= (short)(real_local_random_range(
					player_information->first_person_idle_time_lower_bound,
					player_information->first_person_idle_time_upper_bound)*TICKS_PER_SECOND);
			}
			first_person_weapon->ticks_idle++;
			if (first_person_weapon->ticks_idle>first_person_weapon->ticks_until_pose)
			{
				first_person_weapon->ticks_until_pose= 0;
				if (real_local_random()>=player_information->first_person_idle_skip_fraction)
				{
					first_person_weapon_set_state(
						local_player_index,
						_first_person_weapon_state_posing,
						TRUE);
				}
			}
		}
		else
		{
			first_person_weapon->ticks_idle= 0;
		}
	}

	first_person_weapon->ticks_until_predict--;
	if (first_person_weapon->ticks_until_predict<=0)
	{
		first_person_weapon_predict(local_player_index);
	}

	return;
}

static boolean model_build_remapping_table_for_animation_graph(
	long model_index,
	long animation_graph_index,
	short *node_remapping_table)
{
	struct model *model= model_definition_get(model_index);
	struct animation_graph *animation_graph= animation_graph_definition_get(animation_graph_index);
	boolean valid= TRUE;
	short model_node_index;

	/* port: no more model nodes than the table holds, and only graph nodes
	the weapon's matrices hold (a map's counts) */
	for (model_node_index=0; model_node_index<MIN(model->nodes.count, MAXIMUM_NODES_PER_ANIMATION); model_node_index++)
	{
		struct model_node *model_node= TAG_BLOCK_GET_ELEMENT(
			&model->nodes,
			model_node_index,
			struct model_node);
		short animation_graph_node_index= NONE;
		short node_index;

		for (node_index=0; node_index<MIN(animation_graph->nodes.count, MAXIMUM_NODES_PER_ANIMATION); node_index++)
		{
			struct animation_graph_node *animation_graph_node= TAG_BLOCK_GET_ELEMENT(
				&animation_graph->nodes,
				node_index,
				struct animation_graph_node);

			if (!csstrcmp(model_node->name, animation_graph_node->name))
			{
				animation_graph_node_index= node_index;
				break;
			}
		}

		if (animation_graph_node_index!=NONE)
		{
			node_remapping_table[model_node_index]= animation_graph_node_index;
		}
		else
		{
			valid= FALSE;
		}
	}

	return valid;
}

static void weapon_play_first_person_weapon_sound(
	long weapon_index,
	short message_type)
{
	if (weapon_index!=NONE && message_type!=NONE && weapon_try_and_get(weapon_index))
	{
		struct weapon_datum *weapon= weapon_get(weapon_index);
		struct weapon_definition *weapon_definition= weapon_definition_get(weapon->definition_index);

		if (weapon_definition->weapon.interface_definition.first_person_animations.index!=NONE)
		{
			short state= (short)first_person_weapon_state_from_weapon_message(message_type);

			if (state!=NONE)
			{
				short animation_type= (short)first_person_animation_type_from_weapon_state(state);

				if (animation_type!=NONE)
				{
					struct animation_graph *animation_graph= animation_graph_definition_get(
						weapon_definition->weapon.interface_definition.first_person_animations.index);
					struct animation_graph_first_person_weapon_animations *first_person_weapon_animations=
						!animation_graph->first_person_weapon_animations.count ?
							NULL :
							TAG_BLOCK_GET_ELEMENT(
								&animation_graph->first_person_weapon_animations,
								0,
								struct animation_graph_first_person_weapon_animations);
					/* port: and the graph has first-person animations (a map's
					graph without them was a NULL read) */
					short animation_index= (first_person_weapon_animations &&
						animation_type>=0 &&
						animation_type<first_person_weapon_animations->animations.count) ?
							animation_graph_animation_index_get(
								&first_person_weapon_animations->animations)
									[animation_type].animation_index :
							(short)NONE;

					if (animation_index!=NONE)
					{
						short sound_index= TAG_BLOCK_GET_ELEMENT(
							&animation_graph->animations,
							animation_index,
							struct animation)->sound_index;

						/* port: a sound the graph has (a map's index) */
						if (VALID_INDEX(sound_index, animation_graph->sound_references.count))
						{
							long definition_index= TAG_BLOCK_GET_ELEMENT(
								&animation_graph->sound_references,
								sound_index,
								struct animation_graph_sound_reference)->sound.index;

							if (definition_index!=NONE)
							{
								object_impulse_sound_new(
									weapon_index,
									definition_index,
									NONE,
									global_origin3d,
									global_forward3d,
									1.0f);
							}
						}
					}
				}
			}
		}
	}

	return;
}

static long first_person_weapon_state_from_weapon_message(
	short message_type)
{
	switch (message_type)
	{
		case _first_person_weapon_message_primary_fire:
			return _first_person_weapon_state_primary_fire;
		case _first_person_weapon_message_secondary_fire:
			return _first_person_weapon_state_secondary_fire;
		case _first_person_weapon_message_primary_misfire:
			return _first_person_weapon_state_primary_misfire;
		case _first_person_weapon_message_secondary_misfire:
			return _first_person_weapon_state_secondary_misfire;
		case _first_person_weapon_message_melee:
			return _first_person_weapon_state_melee;
		case _first_person_weapon_message_light_on:
			return _first_person_weapon_state_light_on;
		case _first_person_weapon_message_light_off:
			return _first_person_weapon_state_light_off;
		case _first_person_weapon_message_reload_while_empty:
			return _first_person_weapon_state_reload_while_empty;
		case _first_person_weapon_message_reload_while_full:
			return _first_person_weapon_state_reload_while_full;
		case _first_person_weapon_message_put_away:
			return _first_person_weapon_state_put_away;
		case _first_person_weapon_message_ready:
			return _first_person_weapon_state_ready;
		case _first_person_weapon_message_charged:
			return _first_person_weapon_state_charged;
		case _first_person_weapon_message_overheating:
			return _first_person_weapon_state_overheating;
		case _first_person_weapon_message_throw_grenade:
			return _first_person_weapon_state_throw_grenade;
		case _first_person_weapon_message_overheating_super_recoil:
			return _first_person_weapon_state_overheating_super_recoil;
	}

	return NONE;
}

static long first_person_animation_type_from_weapon_state(
	short state)
{
	switch (state)
	{
		case _first_person_weapon_state_idle:
			return _first_person_weapon_animation_idle;
		case _first_person_weapon_state_overheated:
			return _first_person_weapon_animation_overheated;
		case _first_person_weapon_state_charged:
			return _first_person_weapon_animation_overcharged;
		case _first_person_weapon_state_posing:
			return _first_person_weapon_animation_posing;
		case _first_person_weapon_state_primary_fire:
			return _first_person_weapon_animation_primary_fire;
		case _first_person_weapon_state_secondary_fire:
			return _first_person_weapon_animation_secondary_fire;
		case _first_person_weapon_state_primary_misfire:
			return _first_person_weapon_animation_primary_misfire;
		case _first_person_weapon_state_secondary_misfire:
			return _first_person_weapon_animation_secondary_misfire;
		case _first_person_weapon_state_melee:
			return _first_person_weapon_animation_melee;
		case _first_person_weapon_state_light_on:
			return _first_person_weapon_animation_light_on;
		case _first_person_weapon_state_light_off:
			return _first_person_weapon_animation_light_off;
		case _first_person_weapon_state_reload_while_empty:
			return _first_person_weapon_animation_reload_while_empty;
		case _first_person_weapon_state_reload_while_full:
			return _first_person_weapon_animation_reload_while_full;
		case _first_person_weapon_state_put_away:
			return _first_person_weapon_animation_put_away;
		case _first_person_weapon_state_ready:
			return _first_person_weapon_animation_ready;
		case _first_person_weapon_state_throw_grenade:
			return _first_person_weapon_animation_throw_grenade;
		case _first_person_weapon_state_throw_grenade_overheated:
			return _first_person_weapon_animation_throw_grenade_overheated;
		case _first_person_weapon_state_overheating:
			return _first_person_weapon_animation_overheating;
		case _first_person_weapon_state_overheating_again:
			return _first_person_weapon_animation_overheating_again;
		case _first_person_weapon_state_shotgun_enter_reload:
			return _first_person_weapon_animation_shotgun_enter;
		case _first_person_weapon_state_shotgun_exit_reload_empty:
			return _first_person_weapon_animation_shotgun_exit_empty;
		case _first_person_weapon_state_shotgun_exit_reload_full:
			return _first_person_weapon_animation_shotgun_exit_full;
		case _first_person_weapon_state_overheated_exit:
			return _first_person_weapon_animation_overheated_exit;
		case _first_person_weapon_state_overheating_super_recoil:
			return _first_person_weapon_animation_overheated_supercharge_enter;
	}

	return NONE;
}

static void model_remap_node_matrices_to_match_animation_graph(
	long model_index,
	struct real_matrix4x3 *model_node_matrices,
	long animation_graph_index,
	struct real_matrix4x3 const *node_matrices,
	short const *node_remapping_table)
{
	struct model *model= model_definition_get(model_index);
	struct animation_graph *animation_graph= animation_graph_definition_get(animation_graph_index);
	short node_index;

	/* port: no more nodes than the table and model_node_matrices hold (a
	map's count) */
	for (node_index=0; node_index<MIN(model->nodes.count, MAXIMUM_NODES_PER_ANIMATION); node_index++)
	{
		short animation_graph_node_index= node_remapping_table[node_index];

		match_assert(
			"c:\\halo\\SOURCE\\interface\\first_person_weapons.c",
			1587,
			animation_graph_node_index>=0 && animation_graph_node_index<animation_graph->nodes.count);
		model_node_matrices[node_index]= node_matrices[animation_graph_node_index];
	}

	return;
}

static struct first_person_weapon *first_person_weapon_get(
	short local_player_index)
{
	match_assert(
		"c:\\halo\\SOURCE\\interface\\first_person_weapons.c",
		1433,
		local_player_index>=0 && local_player_index<MAXIMUM_NUMBER_OF_LOCAL_PLAYERS);

	return &first_person_weapons[local_player_index];
}

static short first_person_weapon_index_from_weapon_index(
	long weapon_index)
{
	short local_player_index;

	for (local_player_index=0; local_player_index<MAXIMUM_NUMBER_OF_LOCAL_PLAYERS; local_player_index++)
	{
		long player_index= local_player_get_player_index(local_player_index);

		if (player_index!=NONE)
		{
			long unit_index= player_get(player_index)->unit_index;

			if (unit_index!=NONE)
			{
				struct unit_datum *unit= unit_get(unit_index);

				if (unit->unit.current_weapon_index!=NONE &&
					weapon_index==unit->unit.weapon_object_indices[unit->unit.current_weapon_index])
				{
					return local_player_index;
				}
			}
		}
	}

	return NONE;
}

static short first_person_weapon_index_from_unit_index(
	long unit_index)
{
	short local_player_index;

	for (local_player_index=0; local_player_index<MAXIMUM_NUMBER_OF_LOCAL_PLAYERS; local_player_index++)
	{
		long player_index= local_player_get_player_index(local_player_index);

		if (player_index!=NONE && player_get(player_index)->unit_index==unit_index)
		{
			return local_player_index;
		}
	}

	return NONE;
}
