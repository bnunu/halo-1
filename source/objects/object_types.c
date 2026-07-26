/*
OBJECT_TYPES.C

symbols in this file:
0012B950 00b0:
	_object_type_definition_get (0000)
0012BA00 00a0:
	_object_type_get_datum_size (0000)
0012BAA0 0090:
	_object_type_get_name (0000)
0012BB30 00c0:
	_object_types_initialize (0000)
0012BBF0 0030:
	_object_types_dispose (0000)
0012BC20 0030:
	_object_types_initialize_for_new_map (0000)
0012BC50 0030:
	_object_types_dispose_from_old_map (0000)
0012BC80 0060:
	_object_type_adjust_placement (0000)
0012BCE0 0070:
	_object_type_new (0000)
0012BD50 0060:
	_object_type_place (0000)
0012BDB0 0060:
	_object_type_delete (0000)
0012BE10 0060:
	_object_type_update (0000)
0012BE70 0060:
	_object_type_export_function_values (0000)
0012BED0 0060:
	_object_type_handle_deleted_object (0000)
0012BF30 0060:
	_object_type_handle_region_destroyed (0000)
0012BF90 0060:
	_object_type_handle_parent_destroyed (0000)
0012BFF0 0060:
	_object_type_preprocess_node_orientations (0000)
0012C050 0060:
	_object_type_postprocess_node_matrices (0000)
0012C0B0 0060:
	_object_type_reset (0000)
0012C110 0060:
	_object_type_disconnect_from_structure_bsp (0000)
0012C170 0060:
	_object_type_render_debug (0000)
0012C1D0 0060:
	_object_type_notify_impulse_sound (0000)
0012C230 0050:
	_object_definition_index_to_object_type (0000)
0012C280 0080:
	_scenario_get_object_type_scenario_datums (0000)
0012C300 0080:
	_scenario_get_object_type_scenario_palette (0000)
0012C380 0050:
	_object_types_disconnect_from_structure_bsp (0000)
0012C3D0 0250:
	_object_types_place_objects (0000)
0012C620 00c0:
	_object_types_place_all (0000)
0012C6E0 00a0:
	_object_names_postprocess (0000)
0012C780 0020:
	_object_types_reconnect_to_structure_bsp (0000)
0012C7A0 0620:
	_object_type_synchronize (0000)
00289B14 000c:
	??_C@_0M@MDAGJHMB@placeholder?$AA@ (0000)
00289B20 000e:
	??_C@_0O@BFAJALOO@light_fixture?$AA@ (0000)
00289B30 000e:
	??_C@_0O@BJKPJAME@sound_scenery?$AA@ (0000)
00289B40 000b:
	??_C@_0L@BOMCPGNF@projectile?$AA@ (0000)
00289B4C 0008:
	??_C@_07BOKCJEKD@garbage?$AA@ (0000)
00289B54 0005:
	??_C@_04NHONDGDE@item?$AA@ (0000)
00289B5C 0006:
	??_C@_05LLJBOCMF@biped?$AA@ (0000)
00289B64 0030:
	??_C@_0DA@OHBPFGNG@object_type_definitions?$FLobject_t@ (0000)
00289B94 0025:
	??_C@_0CF@PNDIAODE@object_type_definitions?$FLobject_t@ (0000)
00289BBC 002a:
	??_C@_0CK@MHJDNGCD@?$CD?$CFd?5isn?8t?5a?5valid?5object?5type?5in@ (0000)
00289BE8 0026:
	??_C@_0CG@OKCLKJ@c?3?2halo?2SOURCE?2objects?2object_ty@ (0000)
00289C10 0012:
	??_C@_0BC@OEEHPIIN@?$CBdefinition?9?$DOnext?$AA@ (0000)
00289C28 0086:
	??_C@_0IG@INCFIBJJ@definition?9?$DOplacement_tag_block_@ (0000)
00289CB0 002d:
	??_C@_0CN@EBIFNKGJ@definition?9?$DOplacement_tag_block_@ (0000)
00289CE0 0082:
	??_C@_0IC@DHNDHHAP@definition?9?$DOpalette_tag_block_of@ (0000)
00289D64 002b:
	??_C@_0CL@LMPCIGEO@definition?9?$DOpalette_tag_block_of@ (0000)
00289D90 0008:
	??_C@_07OBIJJHNE@?$CGmatrix?$AA@ (0000)
0030B990 0a30:
	_object_data_definition (0000)
	_unit_data_definition (00a0)
	_biped_data_definition (0140)
	_vehicle_data_definition (01e0)
	_item_data_definition (0280)
	_weapon_data_definition (0320)
	_equipment_data_definition (03c0)
	_garbage_data_definition (0460)
	_projectile_data_definition (0500)
	_scenery_data_definition (05a0)
	_sound_scenery_data_definition (0640)
	_device_data_definition (06e0)
	_machine_data_definition (0780)
	_control_data_definition (0820)
	_light_fixture_data_definition (08c0)
	_placeholder_data_definition (0960)
	_object_type_definitions (0a00)
00456E98 0002:
	_bss_00456e98 (0000)
*/

/* ---------- headers */

#include "cseries.h"
#include "object_types.h"

#include "cache/cache_files.h"
#include "cutscene/cinematics.h"
#include "editor/editor_stubs.h"
#include "objects.h"
#include "scenario/scenario.h"
#include "scenario/scenario_definitions.h"

/* ---------- constants */

/* ---------- macros */

/* ---------- structures */

/* ---------- prototypes */

void object_types_place_objects(
	boolean reconnecting);

#define DECLARE_OBJECT_TYPE_LIFECYCLE(prefix) \
	void prefix##_initialize( \
		void); \
	void prefix##_dispose( \
		void); \
	void prefix##_initialize_for_new_map( \
		void); \
	void prefix##_dispose_from_old_map( \
		void)

DECLARE_OBJECT_TYPE_LIFECYCLE(units);
DECLARE_OBJECT_TYPE_LIFECYCLE(bipeds);
DECLARE_OBJECT_TYPE_LIFECYCLE(vehicles);
DECLARE_OBJECT_TYPE_LIFECYCLE(items);
DECLARE_OBJECT_TYPE_LIFECYCLE(weapons);
DECLARE_OBJECT_TYPE_LIFECYCLE(projectiles);
DECLARE_OBJECT_TYPE_LIFECYCLE(scenery);
DECLARE_OBJECT_TYPE_LIFECYCLE(devices);
DECLARE_OBJECT_TYPE_LIFECYCLE(machines);
DECLARE_OBJECT_TYPE_LIFECYCLE(controls);
DECLARE_OBJECT_TYPE_LIFECYCLE(light_fixtures);
DECLARE_OBJECT_TYPE_LIFECYCLE(placeholder);

#undef DECLARE_OBJECT_TYPE_LIFECYCLE

void object_export_function_values(
	long object_index);
void object_render_debug(
	long object_index);

boolean unit_new(
	long object_index);
void unit_delete(
	long object_index);
boolean unit_update(
	long object_index);
void unit_export_function_values(
	long object_index);
void unit_handle_deleted_object(
	long object_index,
	long deleted_object_index);
void unit_handle_region_destroyed(
	long object_index,
	short region_index,
	unsigned long damage_flags);
void unit_preprocess_node_orientations(
	long object_index,
	struct real_orientation *node_orientations);
void unit_postprocess_node_matrices(
	long object_index,
	struct real_matrix4x3 *node_matrices);
void unit_notify_impulse_sound(
	long object_index,
	long sound_index,
	long source_object_index);
void unit_render_debug(
	long object_index);

void biped_adjust_placement(
	long object_index,
	struct object_placement_data *data);
boolean biped_new(
	long object_index);
void biped_place(
	long object_index,
	struct scenario_object_datum *scenario_object);
void biped_delete(
	long object_index);
boolean biped_update(
	long object_index);
void biped_export_function_values(
	long object_index);
void biped_preprocess_node_orientations(
	long object_index,
	struct real_orientation *node_orientations);
void biped_reset(
	long object_index);
void biped_disconnect_from_structure_bsp(
	long object_index);
void biped_render_debug(
	long object_index);

boolean vehicle_new(
	long object_index);
void vehicle_place(
	long object_index,
	struct scenario_object_datum *scenario_object);
void vehicle_delete(
	long object_index);
boolean vehicle_update(
	long object_index);
void vehicle_export_function_values(
	long object_index);
void vehicle_preprocess_node_orientations(
	long object_index,
	struct real_orientation *node_orientations);
void vehicle_reset(
	long object_index);
void vehicle_render_debug(
	long object_index);

boolean item_new(
	long object_index);
void item_delete(
	long object_index);
boolean item_update(
	long object_index);

boolean weapon_new(
	long object_index);
void weapon_place(
	long object_index,
	struct scenario_object_datum *scenario_object);
void weapon_delete(
	long object_index);
boolean weapon_update(
	long object_index);
void weapon_export_function_values(
	long object_index);
void weapon_preprocess_node_orientations(
	long object_index,
	struct real_orientation *node_orientations);

void equipment_place(
	long object_index,
	struct scenario_object_datum *scenario_object);
boolean garbage_new(
	long object_index);
boolean garbage_update(
	long object_index);

boolean projectile_new(
	long object_index);
void projectile_delete(
	long object_index);
boolean projectile_update(
	long object_index);
void projectile_export_function_values(
	long object_index);
void projectile_handle_deleted_object(
	long object_index,
	long deleted_object_index);
boolean projectile_handle_parent_destroyed(
	long object_index);

boolean scenery_new(
	long object_index);
void scenery_place(
	long object_index,
	struct scenario_object_datum *scenario_object);
void scenery_delete(
	long object_index);
boolean scenery_update(
	long object_index);

boolean sound_scenery_new(
	long object_index);
void sound_scenery_delete(
	long object_index);

boolean device_new(
	long object_index);
void device_delete(
	long object_index);
boolean device_update(
	long object_index);
void device_export_function_values(
	long object_index);
void device_preprocess_node_orientations(
	long object_index,
	struct real_orientation *node_orientations);
void device_render_debug(
	long object_index);

boolean machine_new(
	long object_index);
void machine_place(
	long object_index,
	struct scenario_object_datum *scenario_object);
void machine_delete(
	long object_index);
boolean machine_update(
	long object_index);

boolean control_new(
	long object_index);
void control_place(
	long object_index,
	struct scenario_object_datum *scenario_object);
void control_delete(
	long object_index);
boolean control_update(
	long object_index);

boolean light_fixture_new(
	long object_index);
void light_fixture_place(
	long object_index,
	struct scenario_object_datum *scenario_object);
void light_fixture_delete(
	long object_index);
boolean light_fixture_update(
	long object_index);

boolean placeholder_new(
	long object_index);
void placeholder_place(
	long object_index,
	struct scenario_object_datum *scenario_object);
void placeholder_delete(
	long object_index);

/* ---------- globals */

/*
The initializer rows follow object_type_definition's field order: identity and
scenario layout, lifecycle callbacks, datum callbacks, hierarchy, then next.
*/
struct object_type_definition object_data_definition =
{
	"object", 'obje', 0x1A4, NONE, NONE, NONE,
	NULL, NULL, NULL, NULL,
	NULL, NULL, NULL, NULL, NULL,
	object_export_function_values,
	NULL, NULL, NULL,
	NULL, NULL, NULL, NULL, NULL,
	object_render_debug,
	{ &object_data_definition },
	NULL
};

struct object_type_definition unit_data_definition =
{
	"unit", 'unit', 0x424, NONE, NONE, NONE,
	units_initialize, units_dispose, units_initialize_for_new_map, units_dispose_from_old_map,
	NULL, unit_new, NULL, unit_delete, unit_update,
	unit_export_function_values,
	unit_handle_deleted_object, unit_handle_region_destroyed, NULL,
	unit_preprocess_node_orientations, unit_postprocess_node_matrices,
	NULL, NULL, unit_notify_impulse_sound,
	unit_render_debug,
	{ &object_data_definition, &unit_data_definition },
	NULL
};

struct object_type_definition biped_data_definition =
{
	"biped", 'bipd', 0x480, 0x228, 0x234, 0x78,
	bipeds_initialize, bipeds_dispose, bipeds_initialize_for_new_map, bipeds_dispose_from_old_map,
	biped_adjust_placement, biped_new, biped_place, biped_delete, biped_update,
	biped_export_function_values,
	NULL, NULL, NULL,
	biped_preprocess_node_orientations, NULL,
	biped_reset, biped_disconnect_from_structure_bsp, NULL,
	biped_render_debug,
	{ &object_data_definition, &unit_data_definition, &biped_data_definition },
	NULL
};

struct object_type_definition vehicle_data_definition =
{
	"vehicle", 'vehi', 0x47C, 0x240, 0x24C, 0x78,
	vehicles_initialize, vehicles_dispose, vehicles_initialize_for_new_map, vehicles_dispose_from_old_map,
	NULL, vehicle_new, vehicle_place, vehicle_delete, vehicle_update,
	vehicle_export_function_values,
	NULL, NULL, NULL,
	vehicle_preprocess_node_orientations, NULL,
	vehicle_reset, NULL, NULL,
	vehicle_render_debug,
	{ &object_data_definition, &unit_data_definition, &vehicle_data_definition },
	NULL
};

struct object_type_definition item_data_definition =
{
	"item", 'item', 0x1DC, NONE, NONE, NONE,
	items_initialize, items_dispose, items_initialize_for_new_map, items_dispose_from_old_map,
	NULL, item_new, NULL, item_delete, item_update,
	NULL,
	NULL, NULL, NULL,
	NULL, NULL,
	NULL, NULL, NULL,
	NULL,
	{ &object_data_definition, &item_data_definition },
	NULL
};

struct object_type_definition weapon_data_definition =
{
	"weapon", 'weap', 0x27C, 0x270, 0x27C, 0x5C,
	weapons_initialize, weapons_dispose, weapons_initialize_for_new_map, weapons_dispose_from_old_map,
	NULL, weapon_new, weapon_place, weapon_delete, weapon_update,
	weapon_export_function_values,
	NULL, NULL, NULL,
	weapon_preprocess_node_orientations, NULL,
	NULL, NULL, NULL,
	NULL,
	{ &object_data_definition, &item_data_definition, &weapon_data_definition },
	NULL
};

struct object_type_definition equipment_data_definition =
{
	"equipment", 'eqip', 0x1F4, 0x258, 0x264, 0x28,
	NULL, NULL, NULL, NULL,
	NULL, NULL, equipment_place, NULL, NULL,
	NULL,
	NULL, NULL, NULL,
	NULL, NULL,
	NULL, NULL, NULL,
	NULL,
	{ &object_data_definition, &item_data_definition, &equipment_data_definition },
	NULL
};

struct object_type_definition garbage_data_definition =
{
	"garbage", 'garb', 0x1F4, NONE, NONE, NONE,
	NULL, NULL, NULL, NULL,
	NULL, garbage_new, NULL, NULL, garbage_update,
	NULL,
	NULL, NULL, NULL,
	NULL, NULL,
	NULL, NULL, NULL,
	NULL,
	{ &object_data_definition, &item_data_definition, &garbage_data_definition },
	NULL
};

struct object_type_definition projectile_data_definition =
{
	"projectile", 'proj', 0x228, NONE, NONE, NONE,
	projectiles_initialize, projectiles_dispose, projectiles_initialize_for_new_map, projectiles_dispose_from_old_map,
	NULL, projectile_new, NULL, projectile_delete, projectile_update,
	projectile_export_function_values,
	projectile_handle_deleted_object, NULL, projectile_handle_parent_destroyed,
	NULL, NULL,
	NULL, NULL, NULL,
	NULL,
	{ &object_data_definition, &projectile_data_definition },
	NULL
};

struct object_type_definition scenery_data_definition =
{
	"scenery", 'scen', 0x1A8, 0x210, 0x21C, 0x48,
	scenery_initialize, scenery_dispose, scenery_initialize_for_new_map, scenery_dispose_from_old_map,
	NULL, scenery_new, scenery_place, scenery_delete, scenery_update,
	NULL,
	NULL, NULL, NULL,
	NULL, NULL,
	NULL, NULL, NULL,
	NULL,
	{ &object_data_definition, &scenery_data_definition },
	NULL
};

struct object_type_definition sound_scenery_data_definition =
{
	"sound_scenery", 'ssce', 0x1A8, 0x2DC, 0x2E8, 0x28,
	NULL, NULL, NULL, NULL,
	NULL, sound_scenery_new, NULL, sound_scenery_delete, NULL,
	NULL,
	NULL, NULL, NULL,
	NULL, NULL,
	NULL, NULL, NULL,
	NULL,
	{ &object_data_definition, &sound_scenery_data_definition },
	NULL
};

struct object_type_definition device_data_definition =
{
	"device", 'devi', 0x1C4, NONE, NONE, NONE,
	devices_initialize, devices_dispose, devices_initialize_for_new_map, devices_dispose_from_old_map,
	NULL, device_new, NULL, device_delete, device_update,
	device_export_function_values,
	NULL, NULL, NULL,
	device_preprocess_node_orientations, NULL,
	NULL, NULL, NULL,
	device_render_debug,
	{ &object_data_definition, &device_data_definition },
	NULL
};

struct object_type_definition machine_data_definition =
{
	"machine", 'mach', 0x1D8, 0x294, 0x2A0, 0x40,
	machines_initialize, machines_dispose, machines_initialize_for_new_map, machines_dispose_from_old_map,
	NULL, machine_new, machine_place, machine_delete, machine_update,
	NULL,
	NULL, NULL, NULL,
	NULL, NULL,
	NULL, NULL, NULL,
	NULL,
	{ &object_data_definition, &device_data_definition, &machine_data_definition },
	NULL
};

struct object_type_definition control_data_definition =
{
	"control", 'ctrl', 0x1CC, 0x2AC, 0x2B8, 0x40,
	controls_initialize, controls_dispose, controls_initialize_for_new_map, controls_dispose_from_old_map,
	NULL, control_new, control_place, control_delete, control_update,
	NULL,
	NULL, NULL, NULL,
	NULL, NULL,
	NULL, NULL, NULL,
	NULL,
	{ &object_data_definition, &device_data_definition, &control_data_definition },
	NULL
};

struct object_type_definition light_fixture_data_definition =
{
	"light_fixture", 'lifi', 0x1DC, 0x2C4, 0x2D0, 0x58,
	light_fixtures_initialize, light_fixtures_dispose, light_fixtures_initialize_for_new_map, light_fixtures_dispose_from_old_map,
	NULL, light_fixture_new, light_fixture_place, light_fixture_delete, light_fixture_update,
	NULL,
	NULL, NULL, NULL,
	NULL, NULL,
	NULL, NULL, NULL,
	NULL,
	{ &object_data_definition, &device_data_definition, &light_fixture_data_definition },
	NULL
};

struct object_type_definition placeholder_data_definition =
{
	"placeholder", 'plac', 0x1FC, NONE, NONE, NONE,
	placeholder_initialize, placeholder_dispose, placeholder_initialize_for_new_map, placeholder_dispose_from_old_map,
	NULL, placeholder_new, placeholder_place, placeholder_delete, NULL,
	NULL,
	NULL, NULL, NULL,
	NULL, NULL,
	NULL, NULL, NULL,
	NULL,
	{ &object_data_definition, &placeholder_data_definition },
	NULL
};

struct object_type_definition *object_type_definitions[NUMBER_OF_OBJECT_TYPES] =
{
	&biped_data_definition,
	&vehicle_data_definition,
	&weapon_data_definition,
	&equipment_data_definition,
	&garbage_data_definition,
	&projectile_data_definition,
	&scenery_data_definition,
	&machine_data_definition,
	&control_data_definition,
	&light_fixture_data_definition,
	&placeholder_data_definition,
	&sound_scenery_data_definition
};

extern struct object_type_definition *first_object_type_definition;
/* VC7 otherwise emits this tentative definition as a common symbol. */
#pragma bss_seg(".bss")
word bss_00456e98;
#pragma bss_seg()

typedef char verify_object_type_definition_size[
	sizeof(struct object_type_definition) == 0xA0 ? 1 : -1];

typedef char verify_object_type_definition_group_tag_offset[
	offsetof(struct object_type_definition, group_tag) == 0x4 ? 1 : -1];

typedef char verify_object_type_definition_game_datum_size_offset[
	offsetof(struct object_type_definition, game_datum_size) == 0x8 ? 1 : -1];

typedef char verify_object_type_definition_part_definitions_offset[
	offsetof(struct object_type_definition, part_definitions) == 0x5C ? 1 : -1];

typedef char verify_object_type_definition_next_offset[
	offsetof(struct object_type_definition, next) == 0x9C ? 1 : -1];

/* ---------- public code */

struct object_type_definition *object_type_definition_get(
	short object_type)
{
	match_vassert(
		"c:\\halo\\SOURCE\\objects\\object_types.c",
		631,
		VALID_INDEX(object_type, NUMBER_OF_OBJECT_TYPES),
		csprintf(
			temporary,
			"#%d isn't a valid object type in [#0,#%d)",
			object_type,
			NUMBER_OF_OBJECT_TYPES));
	match_assert(
		"c:\\halo\\SOURCE\\objects\\object_types.c",
		632,
		object_type_definitions[object_type]);
	match_assert(
		"c:\\halo\\SOURCE\\objects\\object_types.c",
		633,
		object_type_definitions[object_type]->group_tag);

	return object_type_definitions[object_type];
}

short object_type_get_datum_size(
	short object_type)
{
	match_vassert(
		"c:\\halo\\SOURCE\\objects\\object_types.c",
		642,
		VALID_INDEX(object_type, NUMBER_OF_OBJECT_TYPES),
		csprintf(
			temporary,
			"#%d isn't a valid object type in [#0,#%d)",
			object_type,
			NUMBER_OF_OBJECT_TYPES));
	match_assert(
		"c:\\halo\\SOURCE\\objects\\object_types.c",
		643,
		object_type_definitions[object_type]);

	return object_type_definitions[object_type]->game_datum_size;
}

char const *object_type_get_name(
	short object_type)
{
	match_vassert(
		"c:\\halo\\SOURCE\\objects\\object_types.c",
		652,
		VALID_INDEX(object_type, NUMBER_OF_OBJECT_TYPES),
		csprintf(
			temporary,
			"#%d isn't a valid object type in [#0,#%d)",
			object_type,
			NUMBER_OF_OBJECT_TYPES));
	match_assert(
		"c:\\halo\\SOURCE\\objects\\object_types.c",
		653,
		object_type_definitions[object_type]);

	return object_type_definitions[object_type]->name;
}

void object_types_initialize(
	void)
{
	short object_type;
	struct object_type_definition *definition;
	struct object_type_definition **next_definition = &first_object_type_definition;

	for (object_type = 0; object_type < NUMBER_OF_OBJECT_TYPES; object_type++)
	{
		short part_index;

		definition = object_type_definition_get(object_type);
		match_assert(
			"c:\\halo\\SOURCE\\objects\\object_types.c",
			746,
			!definition->next);
		*next_definition = definition;
		next_definition = &definition->next;

		for (part_index = 0; part_index < MAXIMUM_CHILDREN_PER_OBJECT_TYPE_DEFINITION; part_index++)
		{
			struct object_type_definition *part_definition = definition->part_definitions[part_index];

			if (!part_definition)
			{
				break;
			}

			if (!part_definition->next)
			{
				*next_definition = part_definition;
				next_definition = &part_definition->next;
			}
		}
	}

	*next_definition = NULL;
	definition = first_object_type_definition;
	while (definition)
	{
		if (definition->initialize)
		{
			definition->initialize();
		}
		definition = definition->next;
	}

	return;
}

void object_types_dispose(
	void)
{
	struct object_type_definition *definition = first_object_type_definition;

	while (definition)
	{
		if (definition->dispose)
			definition->dispose();
		definition = definition->next;
	}

	return;
}

void object_types_initialize_for_new_map(
	void)
{
	struct object_type_definition *definition = first_object_type_definition;
	bss_00456e98 = 0;

	while (definition)
	{
		if (definition->initialize_for_new_map)
			definition->initialize_for_new_map();
		definition = definition->next;
	}

	return;
}

void object_types_dispose_from_old_map(
	void)
{
	struct object_type_definition *definition = first_object_type_definition;

	while (definition)
	{
		if (definition->dispose_from_old_map)
			definition->dispose_from_old_map();
		definition = definition->next;
	}

	return;
}

short object_definition_index_to_object_type(
	long definition_index)
{
	short object_type;
	short result;
	long group_tag = tag_get_group_tag(definition_index);
	result = NONE;

	for (object_type = 0; object_type < NUMBER_OF_OBJECT_TYPES; object_type++)
	{
		if (object_type_definition_get(object_type)->group_tag == group_tag)
		{
			result = object_type;
			break;
		}
	}

	return result;
}

void object_type_adjust_placement(
	long object_index,
	struct object_placement_data *data)
{
	short i;
	struct object_type_definition *definition = object_type_definition_get(object_get(object_index)->object.type);

	for (i = 0; definition->part_definitions[i]; i++)
	{
		struct object_type_definition *current_definition = definition->part_definitions[i];
		if (current_definition->datum_adjust_placement)
		{
			current_definition->datum_adjust_placement(object_index, data);
		}
	}

	return;
}

boolean object_type_new(
	long object_index)
{
	short i;

	struct object_type_definition* definition = object_type_definition_get(object_get(object_index)->object.type);
	boolean result = TRUE;

	for (i = 0; definition->part_definitions[i]; i++)
	{
		struct object_type_definition *current_definition = definition->part_definitions[i];

		if (current_definition->datum_new && !current_definition->datum_new(object_index))
		{
			result = FALSE;
			break;
		}
	}

	return result;
}

void object_type_place(
	long object_index,
	struct scenario_object_datum *scenario_object)
{
	short i;
	struct object_type_definition *definition = object_type_definition_get(object_get(object_index)->object.type);

	for (i = 0; definition->part_definitions[i]; i++)
	{
		struct object_type_definition *current_definition = definition->part_definitions[i];
		if (current_definition->datum_place)
		{
			current_definition->datum_place(object_index, scenario_object);
		}
	}

	return;
}

void object_type_delete(
	long object_index)
{
	short i;

	struct object_type_definition *definition = object_type_definition_get(object_get(object_index)->object.type);
	boolean result = TRUE;

	for (i = 0; definition->part_definitions[i]; i++)
	{
		struct object_type_definition *current_definition = definition->part_definitions[i];
		if (current_definition->datum_delete)
		{
			current_definition->datum_delete(object_index);
		}
	}

	return;
}

boolean object_type_update(
	long object_index)
{
	short i;
	boolean result;
	struct object_type_definition *definition = object_type_definition_get(object_get(object_index)->object.type);
	result = FALSE;

	for (i = 0; definition->part_definitions[i]; i++)
	{
		struct object_type_definition *current_definition = definition->part_definitions[i];
		if (current_definition->datum_update && current_definition->datum_update(object_index))
		{
			result = TRUE;
		}
	}

	return result;
}

void object_type_export_function_values(
	long object_index)
{
	short i;
	struct object_type_definition *definition = object_type_definition_get(object_get(object_index)->object.type);

	for (i = 0; definition->part_definitions[i]; i++)
	{
		struct object_type_definition *current_definition = definition->part_definitions[i];
		if (current_definition->datum_export_function_values)
		{
			current_definition->datum_export_function_values(object_index);
		}
	}

	return;
}

void object_type_handle_deleted_object(
	long object_index,
	long deleted_object_index)
{
	short i;
	struct object_type_definition *definition = object_type_definition_get(object_get(object_index)->object.type);

	for (i = 0; definition->part_definitions[i]; i++)
	{
		struct object_type_definition *current_definition = definition->part_definitions[i];
		if (current_definition->handle_deleted_object)
		{
			current_definition->handle_deleted_object(object_index, deleted_object_index);
		}
	}

	return;
}

void object_type_handle_region_destroyed(
	long object_index,
	short region_index,
	unsigned long damage_flags)
{
	short i;
	struct object_type_definition *definition = object_type_definition_get(object_get(object_index)->object.type);

	for (i = 0; definition->part_definitions[i]; i++)
	{
		struct object_type_definition *current_definition = definition->part_definitions[i];
		if (current_definition->handle_region_destroyed)
		{
			current_definition->handle_region_destroyed(object_index, region_index, damage_flags);
		}
	}

	return;
}

boolean object_type_handle_parent_destroyed(
	long object_index)
{
	short i;
	boolean result;
	struct object_type_definition *definition = object_type_definition_get(object_get(object_index)->object.type);
	result = FALSE;

	for (i = 0; definition->part_definitions[i]; i++)
	{
		struct object_type_definition *current_definition = definition->part_definitions[i];
		if (current_definition->handle_parent_destroyed &&
			current_definition->handle_parent_destroyed(object_index))
		{
			result = TRUE;
		}
	}

	return result;
}

void object_type_preprocess_node_orientations(
	long object_index,
	struct real_orientation *node_orientations)
{
	short i;
	struct object_type_definition *definition = object_type_definition_get(object_get(object_index)->object.type);

	for (i = 0; definition->part_definitions[i]; i++)
	{
		struct object_type_definition *current_definition = definition->part_definitions[i];
		if (current_definition->datum_preprocess_node_orientations)
		{
			current_definition->datum_preprocess_node_orientations(object_index, node_orientations);
		}
	}

	return;
}

void object_type_postprocess_node_matrices(
	long object_index,
	struct real_matrix4x3 *node_matrices)
{
	short i;
	struct object_type_definition *definition = object_type_definition_get(object_get(object_index)->object.type);

	for (i = 0; definition->part_definitions[i]; i++)
	{
		struct object_type_definition *current_definition = definition->part_definitions[i];
		if (current_definition->datum_postprocess_node_matrices)
		{
			current_definition->datum_postprocess_node_matrices(object_index, node_matrices);
		}
	}

	return;
}

void object_type_disconnect_from_structure_bsp(
	long object_index)
{
	short i;
	struct object_type_definition *definition = object_type_definition_get(object_get(object_index)->object.type);

	for (i = 0; definition->part_definitions[i]; i++)
	{
		struct object_type_definition *current_definition = definition->part_definitions[i];
		if (current_definition->disconnect_from_structure_bsp)
		{
			current_definition->disconnect_from_structure_bsp(object_index);
		}
	}

	return;
}

void object_type_reset(
	long object_index)
{
	short i;
	struct object_type_definition *definition = object_type_definition_get(object_get(object_index)->object.type);

	for (i = 0; definition->part_definitions[i]; i++)
	{
		struct object_type_definition *current_definition = definition->part_definitions[i];
		if (current_definition->reset)
		{
			current_definition->reset(object_index);
		}
	}

	return;
}

void object_type_render_debug(
	long object_index)
{
	short i;
	struct object_type_definition *definition = object_type_definition_get(object_get(object_index)->object.type);

	for (i = 0; definition->part_definitions[i]; i++)
	{
		struct object_type_definition *current_definition = definition->part_definitions[i];
		if (current_definition->render_debug)
		{
			current_definition->render_debug(object_index);
		}
	}

	return;
}

void object_type_notify_impulse_sound(
	long object_index,
	long sound_index,
	long source_object_index)
{
	short i;
	struct object_type_definition *definition = object_type_definition_get(object_get(object_index)->object.type);

	for (i = 0; definition->part_definitions[i]; i++)
	{
		struct object_type_definition *current_definition = definition->part_definitions[i];
		if (current_definition->notify_impulse_sound)
		{
			current_definition->notify_impulse_sound(object_index, sound_index, source_object_index);
		}
	}

	return;
}

struct tag_block *scenario_get_object_type_scenario_datums(
	struct scenario *scenario,
	short object_type,
	long *size)
{
	struct object_type_definition *definition = object_type_definition_get(object_type);

	match_assert(
		"c:\\halo\\SOURCE\\objects\\object_types.c",
		1279,
		definition->placement_tag_block_offset!=NONE);
	match_assert(
		"c:\\halo\\SOURCE\\objects\\object_types.c",
		1280,
		definition->placement_tag_block_offset>=0 && definition->placement_tag_block_offset<=sizeof(struct scenario)+sizeof(struct tag_block));

	if (size)
	{
		*size = definition->placement_tag_block_element_size;
	}

	return (struct tag_block *)((byte *)scenario + definition->placement_tag_block_offset);
}

struct tag_block *scenario_get_object_type_scenario_palette(
	struct scenario *scenario,
	short object_type)
{
	struct object_type_definition *definition = object_type_definition_get(object_type);

	match_assert(
		"c:\\halo\\SOURCE\\objects\\object_types.c",
		1293,
		definition->palette_tag_block_offset!=NONE);
	match_assert(
		"c:\\halo\\SOURCE\\objects\\object_types.c",
		1294,
		definition->palette_tag_block_offset>=0 && definition->palette_tag_block_offset<=sizeof(struct scenario)+sizeof(struct tag_block));

	return (struct tag_block *)((byte *)scenario + definition->palette_tag_block_offset);
}

void object_types_disconnect_from_structure_bsp(
	void)
{
	struct object_iterator iterator;
	struct object_datum *object;

	object_iterator_new(&iterator, _object_mask_remove_on_bsp_switch, 0);
	while ((object = (struct object_datum *)object_iterator_next(&iterator))!=NULL)
	{
		if (object->object.name_index==NONE)
		{
			object_delete(iterator.index);
		}
	}

	return;
}

void object_types_place_objects(
	boolean reconnecting)
{
	if (!game_in_editor() && global_structure_bsp_index!=NONE)
	{
		short object_type;
		struct scenario *scenario = global_scenario_get();

		for (object_type = 0; object_type < NUMBER_OF_OBJECT_TYPES; object_type++)
		{
			struct object_type_definition *definition;

			if (!TEST_FLAG(_object_mask_remove_on_bsp_switch, object_type))
			{
				continue;
			}

			definition = object_type_definition_get(object_type);
			if (definition->placement_tag_block_offset!=NONE &&
				definition->palette_tag_block_offset!=NONE)
			{
				long element_size;
				struct tag_block *scenario_datums = scenario_get_object_type_scenario_datums(
					scenario,
					object_type,
					&element_size);
				struct tag_block *scenario_palette = scenario_get_object_type_scenario_palette(
					scenario,
					object_type);

				if (!TEST_FLAG(bss_00456e98, global_structure_bsp_index))
				{
					short scenario_datum_index;

					for (scenario_datum_index = 0;
						scenario_datum_index < scenario_datums->count;
						scenario_datum_index++)
					{
						struct scenario_object_datum *scenario_object =
							(struct scenario_object_datum *)tag_block_get_element_with_size(
								scenario_datums,
								scenario_datum_index,
								element_size);

						if (scenario_object->palette_entry_index!=NONE)
						{
							struct scenario_object_palette_entry *palette_entry = TAG_BLOCK_GET_ELEMENT(
								scenario_palette,
								scenario_object->palette_entry_index,
								struct scenario_object_palette_entry);
							struct object_definition *object_definition = object_definition_get(
								palette_entry->reference.index);
							real_matrix4x3 matrix;
							real_point3d transformed_bounding_offset;

							matrix4x3_rotation_from_angles(
								&matrix,
								scenario_object->rotation.yaw,
								scenario_object->rotation.pitch,
								scenario_object->rotation.roll);
							matrix.position = scenario_object->position;
							matrix4x3_transform_point(
								&matrix,
								&object_definition->object.bounding_offset,
								&transformed_bounding_offset);

							if (scenario_leaf_index_from_point(&scenario_object->position)==NONE &&
								scenario_leaf_index_from_point(&transformed_bounding_offset)==NONE)
							{
								scenario_object->on_bsp_flags &= ~FLAG(global_structure_bsp_index);
							}
							else
							{
								scenario_object->on_bsp_flags |= FLAG(global_structure_bsp_index);
							}
						}
					}
				}

				if (reconnecting)
				{
					short scenario_datum_index;

					objects_memory_compact();
					for (scenario_datum_index = 0;
						scenario_datum_index < scenario_datums->count;
						scenario_datum_index++)
					{
						struct scenario_object_datum *scenario_object =
							(struct scenario_object_datum *)tag_block_get_element_with_size(
								scenario_datums,
								scenario_datum_index,
								element_size);

						if ((scenario_object->name_index==NONE ||
							object_index_from_name_index(scenario_object->name_index)==NONE) &&
							!TEST_FLAG(scenario_object->placement_flags, _scenario_object_placement_not_automatic_bit) &&
							TEST_FLAG(scenario_object->on_bsp_flags, global_structure_bsp_index))
						{
							object_new_from_scenario(scenario_object, scenario_palette);
							objects_garbage_collection();
						}
					}
				}
			}
		}

		bss_00456e98 |= FLAG(global_structure_bsp_index);
	}

	return;
}

void object_types_reconnect_to_structure_bsp(
	void)
{
	if (!cinematic_in_progress() || !cinematic_globals->suppress_bsp_object_creation)
	{
		object_types_place_objects(TRUE);
	}

	return;
}

void object_types_place_all(
	struct scenario *scenario)
{
	if (!game_in_editor())
	{
		short object_type;

		for (object_type = 0; object_type < NUMBER_OF_OBJECT_TYPES; object_type++)
		{
			struct object_type_definition *definition;

			if (TEST_FLAG(_object_mask_remove_on_bsp_switch, object_type))
			{
				continue;
			}

			definition = object_type_definition_get(object_type);
			if (definition->placement_tag_block_offset!=NONE &&
				definition->palette_tag_block_offset!=NONE)
			{
				long element_size;
				short scenario_datum_index;
				struct tag_block *scenario_datums = scenario_get_object_type_scenario_datums(
					scenario,
					object_type,
					&element_size);
				struct tag_block *scenario_palette = scenario_get_object_type_scenario_palette(
					scenario,
					object_type);

				for (scenario_datum_index = 0;
					scenario_datum_index < scenario_datums->count;
					scenario_datum_index++)
				{
					struct scenario_object_datum *scenario_object =
						(struct scenario_object_datum *)tag_block_get_element_with_size(
							scenario_datums,
							scenario_datum_index,
							element_size);

					object_new_from_scenario(scenario_object, scenario_palette);
					objects_garbage_collection();
				}
			}
		}

		object_types_place_objects(TRUE);
	}

	return;
}

void object_names_postprocess(
	struct scenario *scenario,
	boolean postprocess)
{
	if (!postprocess)
	{
		short object_type;

		for (object_type = 0; object_type < NUMBER_OF_OBJECT_TYPES; object_type++)
		{
			struct object_type_definition *definition = object_type_definition_get(object_type);

			if (definition->placement_tag_block_offset!=NONE &&
				definition->palette_tag_block_offset!=NONE)
			{
				long element_size;
				short scenario_datum_index;
				struct tag_block *scenario_datums = scenario_get_object_type_scenario_datums(
					scenario,
					object_type,
					&element_size);

				for (scenario_datum_index = 0;
					scenario_datum_index < scenario_datums->count;
					scenario_datum_index++)
				{
					struct scenario_object_datum *scenario_object =
						(struct scenario_object_datum *)tag_block_get_element_with_size(
							scenario_datums,
							scenario_datum_index,
							element_size);

					if (scenario_object->name_index!=NONE)
					{
						struct scenario_object_name *object_name = TAG_BLOCK_GET_ELEMENT(
							&scenario->object_names,
							scenario_object->name_index,
							struct scenario_object_name);

						object_name->runtime_object_type = object_type;
						object_name->runtime_scenario_datum_index = scenario_datum_index;
					}
				}
			}
		}
	}

	return;
}


/* ---------- private code */

/* Preserve the original inline expansion used by the matrix validation below. */
#define valid_realcmp(a, b) \
	(valid_real((a) - (b)) && fabs((a) - (b)) < 0.001f)

long object_type_synchronize(
	long object_index,
	struct scenario_object_datum *scenario_object,
	struct tag_block *palette,
	short object_type,
	short scenario_datum_index)
{
	struct object_placement_data placement_data;
	real_matrix4x3 matrix;
	struct object_datum *object;

	if (scenario_object->palette_entry_index==NONE)
	{
		if (object_index!=NONE)
		{
			object_delete(object_index);
			object_index = NONE;
		}
	}
	else
	{
		if (object_index==NONE)
		{
			long definition_index = TAG_BLOCK_GET_ELEMENT(
				palette,
				scenario_object->palette_entry_index,
				struct scenario_object_palette_entry)->reference.index;

			if (definition_index!=NONE)
			{
				object_placement_data_new(
					&placement_data,
					definition_index,
					NONE);
				placement_data.position = scenario_object->position;
				vectors3d_from_euler_angles3d(
					&placement_data.forward,
					&placement_data.up,
					&scenario_object->rotation);
				placement_data.variant_number = scenario_object->variant_number;

				object_index = object_new(&placement_data);
				if (object_index!=NONE)
				{
					object_type_place(object_index, scenario_object);
				}
			}
		}
		else
		{
			struct scenario_object_palette_entry *palette_entry;

			object = object_try_and_get(object_index);
			palette_entry = TAG_BLOCK_GET_ELEMENT(
				palette,
				scenario_object->palette_entry_index,
				struct scenario_object_palette_entry);
			if (!object || object->definition_index!=palette_entry->reference.index)
			{
				if (object)
				{
					object_delete(object_index);
				}
				object_index = NONE;

				if (palette_entry->reference.index!=NONE)
				{
					object_placement_data_new(
						&placement_data,
						palette_entry->reference.index,
						NONE);
					placement_data.position = scenario_object->position;
					vectors3d_from_euler_angles3d(
						&placement_data.forward,
						&placement_data.up,
						&scenario_object->rotation);
					placement_data.variant_number = scenario_object->variant_number;

					object_index = object_new(&placement_data);
					if (object_index!=NONE)
					{
						object_type_place(object_index, scenario_object);
					}
				}
			}
		}

		if (object_index!=NONE)
		{
			struct object_definition *definition;

			object = object_get(object_index);
			object_activate(object_index);
			matrix4x3_rotation_from_angles(
				&matrix,
				scenario_object->rotation.yaw,
				scenario_object->rotation.pitch,
				scenario_object->rotation.roll);
			match_assert_valid_real_matrix4x3(
				"c:\\halo\\SOURCE\\objects\\object_types.c",
				975,
				&matrix);

			definition = object_definition_get(object->definition_index);
			if (definition->object.physics.index!=NONE)
			{
				real_point3d position = scenario_object->position;
				position.z += object->object.bounding_sphere_radius * 0.5f;
				object_set_position(
					object_index,
					&position,
					&matrix.forward,
					&matrix.up);
			}
			else
			{
				object_set_position(
					object_index,
					&scenario_object->position,
					&matrix.forward,
					&matrix.up);
			}
			object->object.name_index = scenario_object->name_index;
		}
	}

	if (scenario_object->name_index!=NONE)
	{
		struct scenario_object_name *object_name = TAG_BLOCK_GET_ELEMENT(
			&global_scenario_get()->object_names,
			scenario_object->name_index,
			struct scenario_object_name);

		object_name->runtime_object_type = object_type;
		object_name->runtime_scenario_datum_index = scenario_datum_index;
		object_set_object_index_for_name_index(
			scenario_object->name_index,
			object_index);
	}

	return object_index;
}

#undef valid_realcmp
