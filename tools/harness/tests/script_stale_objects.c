/*
SCRIPT_STALE_OBJECTS.C

A script holding an object that has been deleted. The script runtime runs here on
the real data arrays (memory/data.c), so a deleted object's index is refused as it
is in the game. datum_get's report in a release build, after which the game reads
through a NULL and crashes, is a failed CHECK here.
*/

/* the real data arrays, not the harness's */
#define data_array harness_data_array
#define datum_header harness_datum_header
#define game_state_data_new harness_game_state_data_new
#define datum_new harness_datum_new
#define datum_get harness_datum_get
#define datum_delete harness_datum_delete
#include "harness.h"
#undef data_array
#undef datum_header
#undef game_state_data_new
#undef datum_new
#undef datum_get
#undef datum_delete

#define HALO_RELEASE
typedef unsigned short word;
#define match_vassert(file, line, condition, message) ((void)0)
#define match_hs_assert(file, line, thread_index, condition, reason) ((void)0)
#define csprintf(buffer, ...) snprintf(buffer, sizeof(buffer), "an index is unused or changed")
#define _error_silent 0
#define error(...) ((void)0)
#define BIT_VECTOR_SET_FLAG(vector, bit, enable) ((void)0)
#define MAXIMUM_TRIGGER_VOLUMES_PER_SCENARIO 256
#define MAX(a, b) ((a)>(b) ? (a) : (b))

static void release_assert_failed(char const *message, char const *file, int line, boolean halt)
{
	CHECK(FALSE, "%s", message);
}

#include "types.inc"

typedef long (*hs_typecasting_procedure)(long value);

/* the fake world: objects with what the code under test reads */
struct object_datum
{
	struct
	{
		short type;
		unsigned long damage_flags;
		real_point3d bounding_sphere_center;
	} object;
};
#define unit_datum object_datum
struct animation { short frame_count; };
struct animation_graph { struct { struct animation elements[1]; } animations; };
struct scenery_datum
{
	struct
	{
		struct
		{
			struct
			{
				long animation_graph_index;
				struct { short index, frame_index; } state;
			} animation;
		} object;
	} object;
	unsigned long flags;
};
union fake_object { struct object_datum object; struct scenery_datum scenery; };
enum { _object_die_act_of_god_bit, _object_die_act_of_god_silent_bit };
enum { MAXIMUM_FAKE_OBJECTS = 16, MAXIMUM_FAKE_NAMES = 4, MAXIMUM_FAKE_FUNCTIONS = 16 };

struct data_array *object_header_data, *object_list_header_data, *object_list_data;
struct data_array *hs_thread_data, *hs_syntax_data, *hs_global_data;
static union fake_object fake_objects[MAXIMUM_FAKE_OBJECTS];
static long fake_names[MAXIMUM_FAKE_NAMES];
static short global_types[16];
static long hs_external_global_count = 0;
static hs_typecasting_procedure typecasting_procedures[NUMBER_OF_HS_TYPES][NUMBER_OF_HS_TYPES];
static long function_storage[MAXIMUM_FAKE_FUNCTIONS][16];
static unsigned long hs_debug_data[8];
static struct animation_graph fake_animation_graph;
static boolean volume_holds_everything = TRUE;
static int volume_tests;

struct hs_script { short return_type; };
static struct { struct { struct hs_script elements[1]; } hs_scripts; } fake_scenario;
#define global_scenario_get() (&fake_scenario)
#define TAG_BLOCK_GET_ELEMENT(block, index, type) (&(block)->elements[index])
#define animation_graph_definition_get(index) (&fake_animation_graph)

#define hs_function_get(index) ((struct hs_function_definition *)function_storage[index])
#define hs_global_get_type(designator) global_types[HS_GLOBAL_DESIGNATOR_TO_INDEX(designator)]
#define hs_global_reconcile_read(designator) ((void)0)
#define hs_thread_format(thread_index) ""
#define hs_runtime_waiting_on_call() FALSE
#define coop_scripts_empty_player_list(list) FALSE
#define coop_scripts_any_player_will_do(list) FALSE
#define coop_scripts_gather_in_volume(volume, list) ((void)0)
#define reference_list_port_full(array) ((void)0)

static long object_index_from_name_index(short name_index)
{
	return name_index>=0 && name_index<MAXIMUM_FAKE_NAMES ? fake_names[name_index] : NONE;
}

static boolean scenario_trigger_volume_test_point(short volume, real_point3d *point)
{
	volume_tests++;
	return point && volume_holds_everything;
}

static void hs_stack_overflow(long thread_index)
{
	CHECK(FALSE, "the thread's stack overflowed");
}

static void hs_syntax_error(long thread_index)
{
	CHECK(FALSE, "a syntax node the runtime refused");
}

static boolean script_error(long thread_index, char const *reason, char const *expression)
{
	CHECK(FALSE, "%s", reason);
	return FALSE;
}

/* the real data arrays' other half: a slot's next identifier always differs */
static long datum_new(struct data_array *data)
{
	short index;

	for (index = 0; index<data->maximum_count; index++)
	{
		struct datum_header *header = (struct datum_header *)((byte *)data->data+data->size*index);

		if (!header->identifier)
		{
			memset(header, 0, (size_t)data->size);
			header->identifier = (short)(data->next_identifier++|0x8000);
			if (index>=data->count)
				data->count = index+1;
			data->actual_count++;
			return ((long)(unsigned short)header->identifier<<16)|index;
		}
	}
	return NONE;
}

static void *hs_stack_allocate(long thread_index, long size);
static boolean hs_stack_push(long thread_index);
static void hs_evaluate(long thread_index, long expression_index, long *destination);
static long hs_global_evaluate(short global_designator);
static long *hs_arguments_evaluate(long thread_index, short formal_parameter_count,
	short const *formal_parameters, boolean initialize);
static long hs_value_still_exists(short type, long value);
static long object_list_skip_deleted(long object_index, long *reference_index);
void *datum_try_and_get(struct data_array *data, long index);
void *datum_get(struct data_array *data, long index);
void *object_try_and_get_and_verify_type(long object_index, unsigned long valid_type_flags);
void *object_get_and_verify_type(long object_index, unsigned long valid_type_flags);
long object_list_new(void);
void object_list_add(long object_list_index, long object_index);
long object_list_get_first(long object_list_index, long *reference_index);
long object_list_get_next(long object_list_index, long *reference_index);
short object_list_count(long object_list_index);
long hs_cast(long thread_index, short actual_type, short desired_type, long value);
void hs_return(long thread_index, long value);
long *hs_macro_function_evaluate(short function_index, long thread_index, boolean initialize);
boolean scenario_trigger_volume_test_object(short trigger_volume_index, long object_index);
boolean hs_trigger_volume_test_objects(short trigger_volume_index, long object_list_index, boolean all);
boolean hs_trigger_volume_test_objects_any(short trigger_volume_index, long object_list_index);
boolean hs_trigger_volume_test_objects_all(short trigger_volume_index, long object_list_index);

#include "under_test.inc"

/* ---------- the fake world's objects, arrays and script nodes */

static struct data_array *array_new(char const *name, short maximum_count, short size)
{
	struct data_array *data = calloc(1, sizeof(*data));

	memcpy(data->name, name, strlen(name)+1);
	data->maximum_count = maximum_count;
	data->size = size;
	data->valid = TRUE;
	data->next_identifier = 0x1000;
	data->data = calloc((size_t)maximum_count, (size_t)size);
	return data;
}

static long object_new(short type)
{
	long object_index = datum_new(object_header_data);
	struct object_header_datum *header = object_header_get(object_index);
	union fake_object *fake = &fake_objects[object_index&0xFFFF];

	memset(fake, 0, sizeof(*fake));
	fake->object.object.type = type;
	header->type = (byte)type;
	header->datum = &fake->object;
	return object_index;
}

static void object_delete(long object_index)
{
	datum_delete(object_header_data, object_index);
}

static long node_new(short type, short index, short flags, long data)
{
	long expression_index = datum_new(hs_syntax_data);
	struct hs_syntax_node *node = hs_syntax_get(expression_index);

	node->type = type;
	node->index = index;
	node->flags = flags;
	node->next_node_index = NONE;
	node->data = data;
	return expression_index;
}

static long constant_node(short type, long value)
{
	return node_new(type, type, FLAG(_hs_syntax_node_primitive_bit), value);
}

/* a global of its type, read as an expression of another (the compiler's cast) */
static long global_node(short global_type, short expression_type, long value)
{
	long global_index = datum_new(hs_global_data);

	((struct hs_global_datum *)datum_get(hs_global_data, global_index))->value = value;
	global_types[global_index&0xFFFF] = global_type;
	return node_new(expression_type, (short)global_type,
		FLAG(_hs_syntax_node_primitive_bit)|FLAG(_hs_syntax_node_global_bit),
		global_index&0xFFFF);
}

static long call_node(short function_index, short type, long first_argument, long second_argument)
{
	long name = node_new(_hs_function_name, function_index, FLAG(_hs_syntax_node_primitive_bit), 0);

	hs_syntax_get(name)->next_node_index = first_argument;
	if (first_argument!=NONE)
		hs_syntax_get(first_argument)->next_node_index = second_argument;
	return node_new(type, function_index, 0, name);
}

/* what hs_thread_main does for a thread, until its expression has a value */
static long evaluate(long expression_index)
{
	long thread_index = datum_new(hs_thread_data);
	struct hs_thread_datum *thread = hs_thread_get(thread_index);
	struct hs_stack_frame *base = (struct hs_stack_frame *)thread->stack_data;
	long *root_result;
	long result;

	thread->stack = base;
	base->previous = NULL;
	base->size = 0;
	root_result = hs_stack_allocate(thread_index, sizeof(long));
	hs_evaluate(thread_index, expression_index, root_result);
	while (thread->stack!=base)
	{
		struct hs_syntax_node *expression = hs_syntax_get(thread->stack->expression_index);
		boolean initialize = TEST_FLAG(thread->flags, _hs_thread_in_function_call_bit);

		thread->stack->size = 0;
		SET_FLAG(thread->flags, _hs_thread_in_function_call_bit, FALSE);
		hs_function_get(expression->index)->evaluate(expression->index, thread_index, initialize);
	}
	result = *root_result;
	datum_delete(hs_thread_data, thread_index);
	return result;
}

/* ---------- script functions to call */

enum
{
	_function_volume_test_object,
	_function_volume_test_objects,
	_function_held_object,
	_function_held_passthrough,
	_function_delete_then_object,
	_function_two_objects,
};

static long held_object;
static long object_to_delete;
static long two_objects_got[2];

static void held_object_evaluate(short function_index, long thread_index, boolean initialize)
{
	hs_return(thread_index, held_object);
}

/* an argument that takes its time: the object goes while it runs, as it can
while a static script sleeps */
static void delete_then_object_evaluate(short function_index, long thread_index, boolean initialize)
{
	object_delete(object_to_delete);
	hs_return(thread_index, held_object);
}

static void two_objects_evaluate(short function_index, long thread_index, boolean initialize)
{
	long const *arguments = hs_macro_function_evaluate(function_index, thread_index, initialize);

	if (arguments)
	{
		two_objects_got[0] = arguments[0];
		two_objects_got[1] = arguments[1];
		hs_return(thread_index, 0);
	}
}

static void function_new(short function_index, short return_type, hs_evaluate_procedure evaluator,
	short parameter_count, short first_type, short second_type)
{
	struct hs_function_definition *function = hs_function_get(function_index);
	short *types = function->parameter_types;

	function->return_type = return_type;
	function->evaluate = evaluator;
	function->parameter_count = parameter_count;
	types[0] = first_type;
	types[1] = second_type;
}

static void world_new(void)
{
	short name;

	object_header_data = array_new("object", MAXIMUM_FAKE_OBJECTS, sizeof(struct object_header_datum));
	object_list_header_data = array_new("object list header", 8, sizeof(struct object_list_header_datum));
	object_list_data = array_new("list object reference", 32, sizeof(struct data_reference));
	hs_thread_data = array_new("hs thread", 4, sizeof(struct hs_thread_datum));
	hs_syntax_data = array_new("hs syntax", 64, sizeof(struct hs_syntax_node));
	hs_global_data = array_new("hs globals", 16, sizeof(struct hs_global_datum));
	for (name = 0; name<MAXIMUM_FAKE_NAMES; name++)
		fake_names[name] = NONE;
	typecasting_procedures[_hs_type_object_list][_hs_type_object] = hs_object_to_object_list;
	typecasting_procedures[_hs_type_object_list][_hs_type_vehicle] = hs_object_to_object_list;
	typecasting_procedures[_hs_type_object_list][_hs_type_object_name] = hs_object_name_to_object_list;
	function_new(_function_volume_test_object, _hs_type_boolean, scenario_trigger_volume_test_object_evaluate,
		2, _hs_type_trigger_volume, _hs_type_object);
	function_new(_function_volume_test_objects, _hs_type_boolean, hs_trigger_volume_test_objects_any_evaluate,
		2, _hs_type_trigger_volume, _hs_type_object_list);
	function_new(_function_held_object, _hs_type_object, held_object_evaluate, 0, 0, 0);
	function_new(_function_held_passthrough, _hs_passthrough, held_object_evaluate, 0, 0, 0);
	function_new(_function_delete_then_object, _hs_type_object, delete_then_object_evaluate, 0, 0, 0);
	function_new(_function_two_objects, _hs_type_void, two_objects_evaluate, 2, _hs_type_object, _hs_type_object);
}

/* (volume_test_object <volume> <expression>) */
static long volume_test_object(long object_expression)
{
	return evaluate(call_node(_function_volume_test_object, _hs_type_boolean,
		constant_node(_hs_type_trigger_volume, 0), object_expression));
}

/* (volume_test_objects <volume> <expression>) */
static long volume_test_objects(long list_expression)
{
	return evaluate(call_node(_function_volume_test_objects, _hs_type_boolean,
		constant_node(_hs_type_trigger_volume, 0), list_expression));
}

/* a list of three objects, walked last added first, as the game's are */
static long three_objects_list(long *first, long *second, long *third)
{
	long list = object_list_new();

	*third = object_new(_object_type_vehicle);
	*second = object_new(_object_type_vehicle);
	*first = object_new(_object_type_vehicle);
	object_list_add(list, *third);
	object_list_add(list, *second);
	object_list_add(list, *first);
	return list;
}

int main(int argc, char **argv)
{
	char const *case_name = argv[1];
	long reference;

	world_new();

	CASE("global-living")
	{
		long vehicle = object_new(_object_type_vehicle);

		CHECK(volume_test_object(global_node(_hs_type_object, _hs_type_object, vehicle))==TRUE, "the living vehicle");
		CHECK(volume_tests==1, "tested %d times", volume_tests);
		return 0;
	}
	/* the crash on the dedicated server: a vehicle a global held, deleted */
	CASE("global-deleted")
	{
		long vehicle = object_new(_object_type_vehicle);

		object_delete(vehicle);
		CHECK(volume_test_object(global_node(_hs_type_object, _hs_type_object, vehicle))==FALSE, "the deleted vehicle");
		CHECK(volume_tests==0, "tested %d times", volume_tests);
		return 0;
	}
	CASE("global-read-as-none")
	{
		long vehicle = object_new(_object_type_vehicle);

		object_delete(vehicle);
		CHECK(evaluate(global_node(_hs_type_vehicle, _hs_type_object, vehicle))==NONE, "the deleted vehicle");
		return 0;
	}
	/* a new object in the deleted one's slot isn't the old index's object */
	CASE("slot-reused")
	{
		long vehicle = object_new(_object_type_vehicle);
		long another;

		object_delete(vehicle);
		another = object_new(_object_type_biped);
		CHECK((another&0xFFFF)==(vehicle&0xFFFF), "the same slot");
		CHECK(evaluate(global_node(_hs_type_object, _hs_type_object, vehicle))==NONE, "the deleted vehicle");
		CHECK(evaluate(global_node(_hs_type_object, _hs_type_object, another))==another, "the new object");
		return 0;
	}
	CASE("returned-deleted")
	{
		held_object = object_new(_object_type_vehicle);
		object_delete(held_object);
		CHECK(evaluate(call_node(_function_held_object, _hs_type_object, NONE, NONE))==NONE, "the result");
		return 0;
	}
	CASE("returned-to-volume-test")
	{
		held_object = object_new(_object_type_vehicle);
		object_delete(held_object);
		CHECK(volume_test_object(call_node(_function_held_object, _hs_type_object, NONE, NONE))==FALSE, "the result");
		return 0;
	}
	/* (if ...) and (begin ...) return as passthrough */
	CASE("passthrough-deleted")
	{
		held_object = object_new(_object_type_vehicle);
		object_delete(held_object);
		CHECK(evaluate(call_node(_function_held_passthrough, _hs_type_object, NONE, NONE))==NONE, "the result");
		return 0;
	}
	CASE("returned-living")
	{
		held_object = object_new(_object_type_vehicle);
		CHECK(evaluate(call_node(_function_held_object, _hs_type_object, NONE, NONE))==held_object, "the result");
		CHECK(evaluate(call_node(_function_held_passthrough, _hs_type_object, NONE, NONE))==held_object, "the result");
		return 0;
	}
	CASE("deleted-during-arguments")
	{
		long vehicle = object_new(_object_type_vehicle);

		object_to_delete = vehicle;
		held_object = object_new(_object_type_biped);
		evaluate(call_node(_function_two_objects, _hs_type_void,
			global_node(_hs_type_object, _hs_type_object, vehicle),
			call_node(_function_delete_then_object, _hs_type_object, NONE, NONE)));
		CHECK(two_objects_got[0]==NONE, "the first argument, deleted while the second ran");
		CHECK(two_objects_got[1]==held_object, "the second argument");
		return 0;
	}
	CASE("cast-to-list")
	{
		long vehicle = object_new(_object_type_vehicle);
		long list = evaluate(global_node(_hs_type_object, _hs_type_object_list, vehicle));

		CHECK(object_list_get_first(list, &reference)==vehicle, "the living vehicle is listed");
		object_delete(vehicle);
		CHECK(evaluate(global_node(_hs_type_object, _hs_type_object_list, vehicle))==NONE, "no list of a deleted vehicle");
		CHECK(object_list_header_data->actual_count==1, "%d lists", object_list_header_data->actual_count);
		return 0;
	}
	/* an object's name: none once the object is gone, and casts as it did */
	CASE("names")
	{
		long vehicle = object_new(_object_type_vehicle);

		fake_names[1] = vehicle;
		CHECK(evaluate(node_new(_hs_type_object, _hs_type_object_name,
			FLAG(_hs_syntax_node_primitive_bit), 1))==vehicle, "the name's object");
		object_delete(vehicle);
		fake_names[1] = NONE;
		CHECK(evaluate(node_new(_hs_type_object, _hs_type_object_name,
			FLAG(_hs_syntax_node_primitive_bit), 1))==NONE, "the name without an object");
		return 0;
	}
	/* only objects and lists are values that can be gone */
	CASE("other-values")
	{
		long vehicle = object_new(_object_type_vehicle);

		object_delete(vehicle);
		CHECK(evaluate(global_node(_hs_type_long_integer, _hs_type_long_integer, vehicle))==vehicle, "a long");
		CHECK(evaluate(constant_node(_hs_type_boolean, TRUE))==TRUE, "a boolean");
		CHECK(evaluate(constant_node(_hs_type_object, NONE))==NONE, "the object none");
		CHECK(evaluate(constant_node(_hs_type_object_list, NONE))==NONE, "the list none");
		return 0;
	}
	CASE("list-walk")
	{
		long first, second, third;
		long list = three_objects_list(&first, &second, &third);

		CHECK(object_list_get_first(list, &reference)==first, "first");
		CHECK(object_list_get_next(list, &reference)==second, "second");
		CHECK(object_list_get_next(list, &reference)==third, "third");
		CHECK(object_list_get_next(list, &reference)==NONE, "the end");
		return 0;
	}
	CASE("list-first-deleted")
	{
		long first, second, third;
		long list = three_objects_list(&first, &second, &third);

		object_delete(first);
		CHECK(object_list_get_first(list, &reference)==second, "second");
		CHECK(object_list_get_next(list, &reference)==third, "third");
		CHECK(object_list_get_next(list, &reference)==NONE, "the end");
		return 0;
	}
	CASE("list-middle-deleted")
	{
		long first, second, third;
		long list = three_objects_list(&first, &second, &third);

		object_delete(second);
		CHECK(object_list_get_first(list, &reference)==first, "first");
		CHECK(object_list_get_next(list, &reference)==third, "third");
		CHECK(object_list_get_next(list, &reference)==NONE, "the end");
		return 0;
	}
	CASE("list-last-deleted")
	{
		long first, second, third;
		long list = three_objects_list(&first, &second, &third);

		object_delete(third);
		CHECK(object_list_get_first(list, &reference)==first, "first");
		CHECK(object_list_get_next(list, &reference)==second, "second");
		CHECK(object_list_get_next(list, &reference)==NONE, "the end");
		return 0;
	}
	CASE("list-all-deleted")
	{
		long first, second, third;
		long list = three_objects_list(&first, &second, &third);

		object_delete(first);
		object_delete(second);
		object_delete(third);
		CHECK(object_list_get_first(list, &reference)==NONE, "nothing left");
		CHECK(hs_object_list_get_element(list, 0)==NONE, "list_get");
		return 0;
	}
	/* list_get gives the objects that are left; list_count counts the entries */
	CASE("list-get-and-count")
	{
		long first, second, third;
		long list = three_objects_list(&first, &second, &third);

		object_delete(second);
		CHECK(hs_object_list_get_element(list, 0)==first, "element 0");
		CHECK(hs_object_list_get_element(list, 1)==third, "element 1");
		CHECK(hs_object_list_get_element(list, 2)==NONE, "element 2");
		CHECK(object_list_count(list)==3, "%d entries", object_list_count(list));
		return 0;
	}
	CASE("list-volume-any")
	{
		long first, second, third;
		long list = three_objects_list(&first, &second, &third);
		long held = global_node(_hs_type_object_list, _hs_type_object_list, list);

		object_delete(first);
		CHECK(volume_test_objects(held)==TRUE, "the two left");
		object_delete(second);
		object_delete(third);
		CHECK(volume_test_objects(global_node(_hs_type_object_list, _hs_type_object_list, list))==FALSE, "none left");
		return 0;
	}
	/* a list with nothing left is as an empty list: all of nothing holds */
	CASE("list-volume-all")
	{
		long first, second, third;
		long list = three_objects_list(&first, &second, &third);

		object_delete(first);
		CHECK(hs_trigger_volume_test_objects_all(0, list)==TRUE, "the two left");
		volume_holds_everything = FALSE;
		CHECK(hs_trigger_volume_test_objects_all(0, list)==FALSE, "the two left, outside");
		object_delete(second);
		object_delete(third);
		CHECK(hs_trigger_volume_test_objects_all(0, list)==TRUE, "none left");
		return 0;
	}
	/* a list collected while a script held it (object_list_gc) */
	CASE("list-collected")
	{
		long list = object_list_new();

		object_list_add(list, object_new(_object_type_vehicle));
		object_list_delete(list);
		CHECK(evaluate(global_node(_hs_type_object_list, _hs_type_object_list, list))==NONE, "the collected list");
		CHECK(volume_test_objects(global_node(_hs_type_object_list, _hs_type_object_list, list))==FALSE, "testing it");
		return 0;
	}
	/* the script functions that didn't take none */
	CASE("kill-none")
	{
		unit_kill(NONE);
		unit_kill_silent(NONE);
		return 0;
	}
	CASE("kill-living")
	{
		long unit = object_new(_object_type_biped);
		struct object_datum *object = object_get(unit);

		unit_kill(unit);
		CHECK(TEST_FLAG(object->object.damage_flags, _object_die_act_of_god_bit), "killed");
		unit_kill_silent(unit);
		CHECK(TEST_FLAG(object->object.damage_flags, _object_die_act_of_god_silent_bit), "killed silently");
		return 0;
	}
	CASE("scenery-none")
	{
		CHECK(scenery_get_animation_time(NONE)==0, "no scenery has no time");
		return 0;
	}
	CASE("scenery-living")
	{
		long object_index = object_new(_object_type_scenery);
		struct scenery_datum *scenery = scenery_get(object_index);

		fake_animation_graph.animations.elements[0].frame_count = 10;
		scenery->flags = FLAG(0);
		scenery->object.object.animation.state.frame_index = 3;
		CHECK(scenery_get_animation_time(object_index)==5, "%d", scenery_get_animation_time(object_index));
		return 0;
	}
	CHECK(FALSE, "no such case: %s", case_name);
	return 0;
}
