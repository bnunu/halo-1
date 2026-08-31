/*
HS.H

header included in hcex build.
*/

#ifndef __HS_H
#define __HS_H
#pragma once

/* ---------- constants */

enum hs_type
{
	_hs_unparsed = 0,
	_hs_special_form,
	_hs_function_name,
	_hs_passthrough,
	_hs_type_void,
	_hs_type_boolean,
	_hs_type_real,
	_hs_type_short_integer,
	_hs_type_long_integer,
	_hs_type_string,
	_hs_type_script,
	_hs_type_trigger_volume,
	_hs_type_cutscene_flag,
	_hs_type_cutscene_camera_point,
	_hs_type_cutscene_title,
	_hs_type_cutscene_recording,
	_hs_type_device_group,
	_hs_type_ai,
	_hs_type_ai_command_list,
	_hs_type_starting_profile,
	_hs_type_conversation,
	_hs_type_navpoint,
	_hs_type_hud_message,
	_hs_type_object_list,
	_hs_type_sound,
	_hs_type_looping_sound,
	_hs_type_effect,
	_hs_type_damage,
	_hs_type_animation_graph,
	_hs_type_actor_variant,
	_hs_type_damage_effect,
	_hs_type_object_definition,
	_hs_type_enum_game_difficulty,
	_hs_type_enum_team,
	_hs_type_enum_ai_default_state,
	_hs_type_enum_actor_type,
	_hs_type_enum_hud_corner,
	_hs_type_object_name,
	_hs_type_unit_name,
	_hs_type_vehicle_name,
	_hs_type_weapon_name,
	_hs_type_device_name,
	_hs_type_scenery_name,
	_hs_type_object,
	_hs_type_unit,
	_hs_type_vehicle,
	_hs_type_weapon,
	_hs_type_device,
	_hs_type_scenery,
	NUMBER_OF_HS_TYPES
};

enum
{
	NUMBER_OF_HS_OBJECT_TYPES = 6
};

/* ---------- macros */

#define hs_type_valid(type) ((type)>=_hs_type_void && (type)<NUMBER_OF_HS_TYPES)
#define HS_TYPE_IS_ENUM(type) ((type)>=_hs_type_enum_game_difficulty && (type)<=_hs_type_enum_hud_corner)
#define HS_TYPE_IS_OBJECT_NAME(type) ((type)>=_hs_type_object_name && (type)<=_hs_type_scenery_name)
#define HS_TYPE_IS_OBJECT(type) ((type)>=_hs_type_object && (type)<=_hs_type_scenery)

/* ---------- structures */

struct hs_enum_definition
{
	short count;
	short pad;
	char const **values;
};

/* ---------- prototypes/HS.C */

void hs_dispose(
	void);

void hs_runtime_dispose(
	void);

void hs_runtime_dispose_from_old_map(
	void);
char const *hs_runtime_get_executing_thread_name(
	void);
boolean hs_wake_by_name(
	char const *name);
void hs_evaluate_wake(
	short function_index,
	long thread_index,
	boolean initialize);
void hs_return(
	long thread_index,
	long value);

void hs_recompile(
	void);

boolean hs_compile_and_evaluate(const char *expression);

short hs_tokens_enumerate(char const *substring, long type_flags, char const **results, short maximum_count);

boolean hs_can_cast(
	short actual_type,
	short desired_type);

/* ---------- globals */

extern word hs_object_type_masks[NUMBER_OF_HS_OBJECT_TYPES];
extern struct hs_enum_definition hs_enum_table[NUMBER_OF_HS_TYPES];

/* ---------- public code */

#endif // __HS_H
