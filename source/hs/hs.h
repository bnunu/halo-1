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
	_hs_type_effect,
	_hs_type_damage,
	_hs_type_looping_sound,
	_hs_type_animation_graph,
	_hs_type_actor_variant,
	_hs_type_damage_effect,
	_hs_type_object_definition,
	_hs_type_enum_game_difficulty,
	_hs_type_enum_team,
	_hs_type_enum_ai_default_state,
	_hs_type_enum_actor_type,
	_hs_type_enum_hud_corner,
	_hs_type_object,
	_hs_type_unit,
	_hs_type_vehicle,
	_hs_type_weapon,
	_hs_type_device,
	_hs_type_scenery,
	_hs_type_object_name,
	_hs_type_unit_name,
	_hs_type_vehicle_name,
	_hs_type_weapon_name,
	_hs_type_device_name,
	_hs_type_scenery_name,
	NUMBER_OF_HS_TYPES
};

enum
{
	NUMBER_OF_HS_OBJECT_TYPES = 6
};

enum hs_function_index
{
	_hs_function_begin = 0,
	_hs_function_begin_random,
	_hs_function_if,
	_hs_function_cond,
	_hs_function_set,
	_hs_function_and,
	_hs_function_or,
	_hs_function_plus,
	_hs_function_minus,
	_hs_function_times,
	_hs_function_divide,
	_hs_function_min,
	_hs_function_max,
	_hs_function_equal,
	_hs_function_not_equal,
	_hs_function_gt,
	_hs_function_lt,
	_hs_function_gte,
	_hs_function_lte,
	_hs_function_sleep,
	_hs_function_sleep_until,
	_hs_function_wake,
	_hs_function_inspect,
	_hs_function_object_to_unit,
	_hs_function_debug_string__first,
	_hs_function_debug_string__ai_debug_communication_suppress =
		_hs_function_debug_string__first,
	_hs_function_debug_string__ai_debug_communication_ignore,
	_hs_function_debug_string__ai_debug_communication_focus,
	_hs_function_debug_string__last =
		_hs_function_debug_string__ai_debug_communication_focus,
};

/* ---------- macros */

#define hs_type_valid(type) ((type)>=_hs_type_void && (type)<NUMBER_OF_HS_TYPES)
#define HS_TYPE_IS_TAG_REFERENCE(type) ((type)>=_hs_type_sound && (type)<=_hs_type_object_definition)
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

typedef boolean (*hs_function_parse_proc)(
	short function_index,
	long expression_index);

typedef void (*hs_evaluate_procedure)(
	short function_index,
	long thread_index,
	boolean initialize);

struct hs_function_definition
{
	short return_type;
	short flags;
	char const *name;
	hs_function_parse_proc parse;
	hs_evaluate_procedure evaluate;
	char const *help;
	char const *usage;
	short parameter_count;
	short parameter_types[1];
};

struct hs_external_global_definition
{
	char const *name;
	short type;
	short pad;
	void *address;
};

/* ---------- prototypes/HS.C */

void hs_initialize(
	void);
void hs_initialize_for_new_map(
	void);
void hs_dispose_from_old_map(
	void);
void hs_update(
	void);
void hs_node_gc(
	void);
void hs_dispose(
	void);
boolean hs_evaluate_by_name(
	char const *name);

/* ---------- prototypes/HS_RUNTIME.C */

void hs_runtime_dispose(
	void);

void hs_runtime_dispose_from_old_map(
	void);
void hs_runtime_initialize(
	void);
void hs_runtime_initialize_for_new_map(
	void);
/* port: network co-op (port/linux/game/network_coop.c) */
void hs_runtime_port_shift_sleep_times(
	long ticks);
void hs_runtime_update(
	void);
long hs_runtime_evaluate(
	long expression_index);
/* port: whether the running script waits on the call it evaluates, which
is a sleep_until's condition (hs_runtime.c) */
boolean hs_runtime_waiting_on_call(
	void);
/* port: only an actual console thread may use unrestricted map_name. */
boolean hs_runtime_port_executing_console_command(
	void);
/* port: the map's riding call counts only from a startup script (vehicle_riding.c) */
short hs_runtime_port_executing_script_type(
	void);
/* port: the map's declared boolean, not a disabled initializer's default */
short hs_runtime_port_scenario_boolean(
	char const *name);
char const *hs_runtime_get_executing_thread_name(
	void);
boolean hs_wake_by_name(
	char const *name);
void render_debug_scripting(
	void);
void render_debug_trigger_volumes(
	void);
void hs_evaluate_wake(
	short function_index,
	long thread_index,
	boolean initialize);
void hs_return(
	long thread_index,
	long value);
long *hs_macro_function_evaluate(
	short function_index,
	long thread_index,
	boolean initialize);

/* ---------- prototypes/HS.C */

void hs_recompile(
	void);

boolean hs_compile_and_evaluate(
	char const *expression);

short hs_tokens_enumerate(
	char const *substring,
	long type_flags,
	char const **results,
	short maximum_count);

boolean hs_can_cast(
	short actual_type,
	short desired_type);
struct hs_function_definition *hs_function_get(
	short function_index);
struct hs_external_global_definition *hs_global_external_get(
	short global_index);
short hs_find_function_by_name(
	char const *name);
short hs_find_script_by_name(
	char const *name);
short hs_find_global_by_name(
	char const *name);
short hs_global_get_type(
	short global_index);
char const *hs_global_get_name(
	short global_index);
/* port: the map's scripts and global initializers that call a function a
map's scripts may not (hs_scenario_functions_check) */
boolean hs_scenario_script_disabled(
	short script_index);
boolean hs_scenario_global_initializer_disabled(
	short global_index);

/* ---------- prototypes/HS_GLOBALS_EXTERNAL.C */

/* port: the external globals a map's scripts may set */
boolean hs_external_global_settable_by_maps(
	short global_index);
/* ---------- prototypes/HS_COMPILE.C */

void hs_compile_initialize(
	boolean compiling_scenario);
void hs_compile_dispose(
	void);
long hs_compile_expression(
	long source_size,
	char const *source,
	char const **error_message,
	char const **error_source);
boolean hs_parse(
	long expression_index,
	short expected_type);
void hs_compile(
	long source_size,
	char const *source,
	char const **error_message,
	char const **error_source);
boolean hs_compile_postprocess(
	char const **error_message_pointer,
	char const **error_source_pointer);

/* ---------- globals */

extern word const hs_object_type_masks[NUMBER_OF_HS_OBJECT_TYPES];
extern tag const hs_tag_reference_type_group_tags[];
extern struct hs_enum_definition const hs_enum_table[];
extern char const *hs_type_names[NUMBER_OF_HS_TYPES];
extern char const *hs_script_type_names[];
extern boolean debug_trigger_volumes;

/* ---------- public code */

#endif // __HS_H
