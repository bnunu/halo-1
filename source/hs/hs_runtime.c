/*
HS_RUNTIME.C

symbols in this file:
000B9850 0030:
	_hs_syntax_nth (0000)
000B9880 0050:
	_hs_inspect_boolean (0000)
000B98D0 0050:
	_hs_inspect_real (0000)
000B9920 0050:
	_hs_inspect_short_integer (0000)
000B9970 0050:
	_hs_inspect_long_integer (0000)
000B99C0 0050:
	_hs_inspect_string (0000)
000B9A10 0090:
	_hs_inspect_enum (0000)
000B9AA0 0100:
	_hs_runtime_initialize (0000)
000B9BA0 0080:
	_hs_runtime_dispose_from_old_map (0000)
000B9C20 0010:
	_hs_runtime_dispose (0000)
000B9C30 00b0:
	_expression_get_function_name (0000)
000B9CE0 00f0:
	_hs_thread_new (0000)
000B9DD0 0050:
	_hs_thread_delete (0000)
000B9E20 0080:
	_hs_thread_format (0000)
000B9EA0 0080:
	_hs_stack_push (0000)
000B9F20 0020:
	_hs_stack_pop (0000)
000B9F40 0150:
	_hs_stack_allocate (0000)
000BA090 00b0:
	_hs_wake (0000)
000BA140 0060:
	_hs_find_thread_by_script (0000)
000BA1A0 0080:
	_hs_find_thread_by_name (0000)
000BA220 0020:
	_hs_long_to_boolean (0000)
000BA240 0020:
	_hs_short_to_boolean (0000)
000BA260 0020:
	_hs_string_to_boolean (0000)
000BA280 0010:
	_hs_data_to_void (0000)
000BA290 0020:
	_hs_short_to_real (0000)
000BA2B0 0010:
	_hs_long_to_real (0000)
000BA2C0 0020:
	_hs_enum_to_real (0000)
000BA2E0 0020:
	_hs_real_to_short (0000)
000BA300 0010:
	_hs_real_to_long (0000)
000BA310 0010:
	_hs_long_to_short (0000)
000BA320 0040:
	_hs_object_name_to_object_list (0000)
000BA360 0030:
	_hs_object_to_object_list (0000)
000BA390 0080:
	_hs_object_type_can_cast (0000)
000BA410 0100:
	_hs_can_cast (0000)
000BA510 00c0:
	_hs_cast (0000)
000BA5D0 0580:
	_hs_global_reconcile_read (0000)
000BAB50 0190:
	_hs_global_reconcile_write (0000)
000BACE0 0040:
	_script_error (0000)
000BAD20 0020:
	_hs_runtime_get_executing_thread_name (0000)
000BAD40 0020:
	_hs_wake_by_name (0000)
000BAD60 0180:
	_render_debug_scripting (0000)
000BAEE0 0440:
	_render_debug_trigger_volumes (0000)
000BB320 0120:
	_hs_return (0000)
000BB440 0040:
	_hs_global_evaluate (0000)
000BB480 00f0:
	_hs_evaluate_wake (0000)
000BB570 0170:
	_hs_evaluate (0000)
000BB6E0 0060:
	_hs_script_evaluate (0000)
000BB740 01c0:
	_hs_arguments_evaluate (0000)
000BB900 0030:
	_hs_macro_function_evaluate (0000)
000BB930 00d0:
	_hs_evaluate_begin (0000)
000BBA00 0210:
	_hs_evaluate_begin_random (0000)
000BBC10 0190:
	_hs_evaluate_if (0000)
000BBDA0 0140:
	_hs_evaluate_set (0000)
000BBEE0 0130:
	_hs_evaluate_logical (0000)
000BC010 0180:
	_hs_evaluate_arithmetic (0000)
000BC190 00e0:
	_hs_evaluate_equality (0000)
000BC270 0210:
	_hs_evaluate_inequality (0000)
000BC480 01c0:
	_hs_evaluate_sleep (0000)
000BC640 0200:
	_hs_evaluate_sleep_until (0000)
000BC840 0100:
	_hs_evaluate_inspect (0000)
000BC940 0120:
	_hs_evaluate_object_cast_up (0000)
000BCA60 0180:
	_hs_evaluate_debug_string (0000)
000BCBE0 02f0:
	_hs_thread_main (0000)
000BCED0 02d0:
	_hs_runtime_initialize_for_new_map (0000)
000BD1A0 00b0:
	_hs_runtime_update (0000)
000BD250 00c0:
	_hs_runtime_evaluate (0000)
0026E880 0017:
	??_C@_0BH@JDOAAMFH@type?$DN?$DN_hs_type_boolean?$AA@ (0000)
0026E898 0030:
	??_C@_0DA@OFBKMPEO@c?3?2halo?2source?2hs?2hs_library_int@ (0000)
0026E8C8 0003:
	??_C@_02NJPGOMH@?$CFf?$AA@ (0000)
0026E8CC 0014:
	??_C@_0BE@OILODGFF@type?$DN?$DN_hs_type_real?$AA@ (0000)
0026E8E0 001d:
	??_C@_0BN@EMEJADII@type?$DN?$DN_hs_type_short_integer?$AA@ (0000)
0026E900 0004:
	??_C@_03JALODAI@?$CFld?$AA@ (0000)
0026E904 001c:
	??_C@_0BM@KMJMLGIL@type?$DN?$DN_hs_type_long_integer?$AA@ (0000)
0026E920 0016:
	??_C@_0BG@BOOAAIJG@type?$DN?$DN_hs_type_string?$AA@ (0000)
0026E938 0033:
	??_C@_0DD@LFJLKEGO@enum_value?$DO?$DN0?5?$CG?$CG?5enum_value?$DMenum@ (0000)
0026E96C 0016:
	??_C@_0BG@BHHILIEN@HS_TYPE_IS_ENUM?$CItype?$CJ?$AA@ (0000)
0026E984 0025:
	??_C@_0CF@LGMLOJMN@couldn?8t?5allocate?5scripting?5glob@ (0000)
0026E9AC 000c:
	??_C@_0M@IAKOKLGB@index?$CB?$DNNONE?$AA@ (0000)
0026E9B8 0024:
	??_C@_0CE@JAGJENNE@raise?5MAXIMUM_NUMBER_OF_HS_GLOBA@ (0000)
0026E9DC 001f:
	??_C@_0BP@HNAHKNME@c?3?2halo?2SOURCE?2hs?2hs_runtime?4c?$AA@ (0000)
0026E9FC 000b:
	??_C@_0L@KLMNBLNA@hs?5globals?$AA@ (0000)
0026EA08 000a:
	??_C@_09NFPDIMGO@hs?5thread?$AA@ (0000)
0026EA14 0010:
	??_C@_0BA@DMAAAIAF@?$CIend?5of?5script?$CJ?$AA@ (0000)
0026EA24 0033:
	??_C@_0DD@EECLOJLE@type?$CB?$DN_hs_thread_type_script?5?$HM?$HM?5@ (0000)
0026EA58 002a:
	??_C@_0CK@PILPJPBD@type?$DO?$DN0?5?$CG?$CG?5type?$DMNUMBER_OF_HS_THR@ (0000)
0026EA84 003a:
	??_C@_0DK@IFFNCFDC@hs_thread_get?$CIthread_index?$CJ?9?$DOtyp@ (0000)
0026EAC0 0014:
	??_C@_0BE@FDONEJOD@?$FLglobal?5initialize?$FN?$AA@ (0000)
0026EAD4 0012:
	??_C@_0BC@DIMBPFCM@?$FLconsole?5command?$FN?$AA@ (0000)
0026EAE8 003a:
	??_C@_0DK@FJONKAAK@a?5problem?5occurred?5while?5executi@ (0000)
0026EB24 0010:
	??_C@_0BA@OPEHPPEI@stack?5overflow?4?$AA@ (0000)
0026EB34 003f:
	??_C@_0DP@HONBOJOM@?$CIbyte?5?$CK?$CJ?5?$CInew_frame?$CL1?$CJ?$DMthread?9?$DOs@ (0000)
0026EB78 0046:
	??_C@_0EG@GLGLNCNE@frame?9?$DOdata?$CLframe?9?$DOsize?$CLsize?$DM?$DNth@ (0000)
0026EBC0 002f:
	??_C@_0CP@LGELPKKM@attempt?5to?5allocate?5zero?5space?5f@ (0000)
0026EBF0 0011:
	??_C@_0BB@IJDFEIOE@corrupted?5stack?4?$AA@ (0000)
0026EC04 0015:
	??_C@_0BF@NOEAJPHF@valid_thread?$CIthread?$CJ?$AA@ (0000)
0026EC1C 003a:
	??_C@_0DK@CIPMAIGO@desired_type?$DO?$DN0?5?$CG?$CG?5desired_type?$DM@ (0000)
0026EC58 0038:
	??_C@_0DI@FBLLEJHE@actual_type?$DO?$DN0?5?$CG?$CG?5actual_type?$DMNU@ (0000)
0026EC90 001c:
	??_C@_0BM@LDIPEIGF@hs_type_valid?$CIdesired_type?$CJ?$AA@ (0000)
0026ECAC 003b:
	??_C@_0DL@KFHDEBEB@actual_type?$DN?$DN_hs_passthrough?5?$HM?$HM?5@ (0000)
0026ECE8 000e:
	??_C@_0O@OAGBFBKC@bad?5typecast?4?$AA@ (0000)
0026ECF8 0027:
	??_C@_0CH@CMGFPKPO@hs_can_cast?$CIactual_type?0?5desired@ (0000)
0026ED20 002b:
	??_C@_0CL@BFAIEDCL@script?5?$CFs?5needs?5to?5be?5recompiled@ (0000)
0026ED4C 0011:
	??_C@_0BB@OIKAMJPF@no?5reason?5given?4?$AA@ (0000)
0026ED60 000a:
	??_C@_09DJENBFJK@?$FLunknown?$FN?$AA@ (0000)
0026ED6C 0003:
	??_C@_02BGAGALCL@?$HMt?$AA@ (0000)
0026ED70 0028:
	??_C@_0CI@KMEEEBHP@?$HMn?$HMn?$HMnscript?5name?$HMtsleep?5time?$HMtf@ (0000)
0026ED98 0028:
	??_C@_0CI@PIIEBFIK@script_name_node?9?$DOtype?$DN?$DN_hs_type@ (0000)
0026EDC0 0042:
	??_C@_0EC@EIHIFDEJ@TEST_FLAG?$CIscript_name_node?9?$DOflag@ (0000)
0026EE04 0018:
	??_C@_0BI@DEHGHNBF@?$CKexpression_index?$DN?$DNNONE?$AA@ (0000)
0026EE1C 001e:
	??_C@_0BO@IEBGLNDF@unexpected?5actual?5parameters?4?$AA@ (0000)
0026EE40 004b:
	??_C@_0EL@EPENHJGN@hs_syntax_get?$CI?$CKexpression_index?$CJ@ (0000)
0026EE8C 0017:
	??_C@_0BH@JHPKLPNE@corrupted?5syntax?5tree?4?$AA@ (0000)
0026EEA4 0018:
	??_C@_0BI@JEAIILKP@?$CKexpression_index?$CB?$DNNONE?$AA@ (0000)
0026EEBC 0023:
	??_C@_0CD@LMIAMJPE@function_index?$DN?$DN_hs_function_beg@ (0000)
0026EEE0 001a:
	??_C@_0BK@ICIEGLDA@?$CKargument_count?$DMLONG_BITS?$AA@ (0000)
0026EEFC 002a:
	??_C@_0CK@HFFDNMBO@function_index?$DN?$DN_hs_function_beg@ (0000)
0026EF28 0052:
	??_C@_0FC@PECKPIAM@parameter_types?$FL0?$FN?$DN?$DN_hs_type_sho@ (0000)
0026EF7C 0028:
	??_C@_0CI@EGAEHKHM@attempt?5to?5convert?5object?5?$CFs?5to?5@ (0000)
0026EFA4 0013:
	??_C@_0BD@HGOCLPEG@function?9?$DOevaluate?$AA@ (0000)
0026EFB8 0023:
	??_C@_0CD@JFOOFKBP@found?5a?5static?5script?5at?5topleve@ (0000)
0026EFE0 004f:
	??_C@_0EP@ECAJPAK@script?9?$DOscript_type?$CB?$DN_hs_script_@ (0000)
0026F030 001b:
	??_C@_0BL@LKLEHFKI@ran?5out?5of?5script?5threads?4?$AA@ (0000)
0026F04C 002c:
	??_C@_0CM@EIHAGODH@a?5global?5initialization?5attempte@ (0000)
0026F078 0020:
	??_C@_0CA@COEBOOJE@internal_thread?9?$DOsleep_until?$DN?$DN0?$AA@ (0000)
0026F098 0036:
	??_C@_0DG@NJPPFAFA@there?5are?5not?5enough?5threads?5to?5@ (0000)
002E2370 264c:
	_hs_type_inspectors (0000)
	_typecasting_procedures (00c8)
004535AC 0008:
	_hs_runtime_globals (0000)
*/

/* ---------- headers */

#include "cseries/cseries.h"
#include "ai/ai_debug_scripting.h"
#include "ai/ai_script.h"
#include "hs/hs.h"
#include "hs/hs_library_internal.h"
/* Empty reconstructed header removed; no declarations to include. */
#include "hs/object_lists.h"
#include "hs/hs_scenario_definitions.h"
#include "math/real_math.h"
#include "memory/data.h"
#include "objects/objects.h"
#include "objects/object_types.h"
#include "physics/collisions.h"
#include "render/render.h"
#include "render/render_debug.h"
#include "scenario/scenario.h"
#include "scenario/scenario_definitions.h"
#include "saved games/game_state.h"
#include "text/draw_string.h"
#include "cseries/errors.h"
#include "main/console.h"
#include "game/game.h"

/* ---------- constants */

enum
{
	_hs_thread_type_script = 0,
	_hs_thread_type_global_initialize,
	_hs_thread_type_console_command,
	NUMBER_OF_HS_THREAD_TYPES,
};

enum
{
	_hs_syntax_node_primitive_bit = 0,
	_hs_syntax_node_script_bit,
	_hs_syntax_node_global_bit,
};

enum
{
	MAXIMUM_HS_DEBUG_STRING_ARGUMENTS = 32
};

enum
{
	HS_THREAD_STACK_SIZE = 0x200
};

enum
{
	_hs_thread_in_function_call_bit = 0,
	_hs_thread_sleeping_bit,
	_hs_global_external_bit = 15,
};

/* ---------- macros */

#define hs_thread_get(thread_index) \
	((struct hs_thread_datum *)datum_get(hs_thread_data, (thread_index)))

#define hs_syntax_get(expression_index) \
	((struct hs_syntax_node *)datum_get(hs_syntax_data, (expression_index)))

/* a thread is valid when it lies inside the thread data array and its stack pointer and
   the fill mark of its topmost frame lie inside its own inline stack buffer */
#define valid_thread(thread) \
	((byte *)(thread)>=(byte *)hs_thread_data->data && \
	(byte *)(thread)<(byte *)hs_thread_data->data+hs_thread_data->count*hs_thread_data->size && \
	(byte *)(thread)->stack>=(thread)->stack_data && \
	(byte *)(thread)->stack<(thread)->stack_data+HS_THREAD_STACK_SIZE && \
	(thread)->stack->data+(thread)->stack->size<=(thread)->stack_data+HS_THREAD_STACK_SIZE)

#define HS_GLOBAL_DESIGNATOR_IS_EXTERNAL(designator) \
	TEST_FLAG((designator), _hs_global_external_bit)

#define HS_GLOBAL_DESIGNATOR_TO_INDEX(designator) \
	((designator)&0x7fff)

/* every runtime failure is reported through the executing script's name */
#define match_hs_assert(file, line, thread_index, expr, reason) \
	match_vassert(file, line, expr, csprintf(temporary, \
		"a problem occurred while executing the script %s: %s (%s)", \
		hs_thread_format(thread_index), reason, #expr))

/* the four ordered comparisons of hs_evaluate_inequality, expanded once per operand
   representation; each expansion reports its own case's location */
#define HS_EVALUATE_INEQUALITY(first, second, line) \
	value0 = (first); \
	value1 = (second); \
	switch (function_index) \
	{ \
	case _hs_function_gt: \
		comparison = value0>value1; \
		break; \
	case _hs_function_lt: \
		comparison = value0<value1; \
		break; \
	case _hs_function_gte: \
		comparison = value0>=value1; \
		break; \
	case _hs_function_lte: \
		comparison = value0<=value1; \
		break; \
	default: \
		comparison = FALSE; \
		match_vassert("c:\\halo\\source\\hs\\hs_library_internal_runtime.h", line, FALSE, NULL); \
		break; \
	}

typedef long (*hs_typecasting_procedure)(long value);

typedef void (*hs_debug_string_procedure)(
	long name_count,
	char const **names);

typedef void (*hs_inspection_procedure)(
	short type,
	long value,
	char *buffer);

/* ---------- structures */

struct hs_runtime_globals
{
	boolean initialized;
	short executing_thread_index;
};

struct hs_stack_frame
{
	struct hs_stack_frame *previous;
	long expression_index;
	void *result;
	short size;
	byte data[2];
};

struct hs_global_datum
{
	short identifier;
	short unused;
	long value;
};

struct hs_thread_datum
{
	short identifier;
	byte type;
	byte flags;
	long script_index;
	long sleep_until;
	long previous_sleep_until;
	struct hs_stack_frame *stack;
	long result;
	byte stack_data[0x200];
};

typedef char hs_thread_datum_size_assert[
	sizeof(struct hs_thread_datum) == 0x218 ? 1 : -1];

/* ---------- prototypes */

static void hs_inspect_boolean(
	short type,
	long value,
	char *buffer);
static void hs_inspect_real(
	short type,
	long value,
	char *buffer);
static void hs_inspect_short_integer(
	short type,
	long value,
	char *buffer);
static void hs_inspect_long_integer(
	short type,
	long value,
	char *buffer);
static void hs_inspect_string(
	short type,
	long value,
	char *buffer);
static void hs_inspect_enum(
	short type,
	long value,
	char *buffer);
static long hs_long_to_boolean(
	long n);
static long hs_short_to_boolean(
	long s);
static long hs_string_to_boolean(
	long n);
static long hs_data_to_void(
	long n);
static long hs_short_to_real(
	long s);
static long hs_long_to_real(
	long l);
static long hs_enum_to_real(
	long e);
static long hs_real_to_short(
	long r);
static long hs_real_to_long(
	long r);
static long hs_long_to_short(
	long l);
static long hs_object_name_to_object_list(
	long object_name_index);
static long hs_object_to_object_list(
	long object_index);
static boolean hs_object_type_can_cast(
	short actual_type,
	short desired_type);
static char const *hs_thread_format(
	long thread_index);
static void hs_stack_pop(
	long thread_index);
static void hs_wake(
	long thread_index);
static long hs_find_thread_by_script(
	short script_index);
static long hs_find_thread_by_name(
	char const *name);
static long hs_syntax_nth(
	long expression_index,
	short n);
static void hs_thread_delete(
	long thread_index);
static long hs_thread_new(
	short type,
	long script_index);
static void hs_stack_push(
	long thread_index);
static void *hs_stack_allocate(
	long thread_index,
	long size);
static void hs_evaluate(
	long thread_index,
	long expression_index,
	long *destination);
static long hs_global_evaluate(
	short global_designator);
static long *hs_arguments_evaluate(
	long thread_index,
	short formal_parameter_count,
	short const *formal_parameters,
	boolean initialize);
static boolean script_error(
	long thread_index,
	char const *reason,
	char const *expression);
static void hs_script_evaluate(
	short script_index,
	long thread_index,
	boolean initialize);
static void hs_thread_main(
	long thread_index);
static void hs_global_reconcile_read(
	word global_designator);
static void hs_global_reconcile_write(
	word global_designator);

/* ---------- globals */

struct data_array *hs_global_data;
struct data_array *hs_thread_data;
extern struct data_array *hs_syntax_data;
extern short const hs_external_global_count;
extern short const hs_type_sizes[NUMBER_OF_HS_TYPES];
boolean debug_scripting;
unsigned long hs_debug_data[BIT_VECTOR_SIZE_IN_LONGS(MAXIMUM_TRIGGER_VOLUMES_PER_SCENARIO)];
typedef char verify_hs_debug_data_size[sizeof(hs_debug_data) == 0x20 ? 1 : -1];
static hs_inspection_procedure hs_type_inspectors[NUMBER_OF_HS_TYPES] =
{
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	hs_inspect_boolean,
	hs_inspect_real,
	hs_inspect_short_integer,
	hs_inspect_long_integer,
	hs_inspect_string,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	hs_inspect_enum,
	hs_inspect_enum,
	hs_inspect_enum,
	hs_inspect_enum,
	hs_inspect_enum,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
};
hs_typecasting_procedure typecasting_procedures[NUMBER_OF_HS_TYPES][NUMBER_OF_HS_TYPES] =
{
	{ NULL }, /* unparsed */
	{ NULL }, /* special_form */
	{ NULL }, /* function_name */
	{ NULL }, /* passthrough */
	{ /* void */
		NULL, NULL, NULL, NULL,
		NULL,
		hs_data_to_void, hs_data_to_void, hs_data_to_void, hs_data_to_void,
		hs_data_to_void, hs_data_to_void, hs_data_to_void, hs_data_to_void,
		hs_data_to_void, hs_data_to_void, hs_data_to_void, hs_data_to_void,
		hs_data_to_void, hs_data_to_void, hs_data_to_void, hs_data_to_void,
		hs_data_to_void, hs_data_to_void, hs_data_to_void, hs_data_to_void,
		hs_data_to_void, hs_data_to_void, hs_data_to_void, hs_data_to_void,
		hs_data_to_void, hs_data_to_void, hs_data_to_void, hs_data_to_void,
		hs_data_to_void, hs_data_to_void, hs_data_to_void, hs_data_to_void,
		hs_data_to_void, hs_data_to_void, hs_data_to_void, hs_data_to_void,
		hs_data_to_void, hs_data_to_void, hs_data_to_void, hs_data_to_void,
		hs_data_to_void, hs_data_to_void, hs_data_to_void, hs_data_to_void,
	},
	{ /* boolean */
		NULL, NULL, NULL, NULL,
		NULL, NULL,
		hs_long_to_boolean,
		hs_short_to_boolean,
		hs_long_to_boolean,
		hs_string_to_boolean,
	},
	{ /* real */
		NULL, NULL, NULL, NULL,
		NULL, NULL, NULL,
		hs_short_to_real,
		hs_long_to_real,
		NULL, NULL, NULL, NULL,
		NULL, NULL, NULL, NULL,
		NULL, NULL, NULL, NULL,
		NULL, NULL, NULL, NULL,
		NULL, NULL, NULL, NULL,
		NULL,
		hs_enum_to_real, hs_enum_to_real, hs_enum_to_real, hs_enum_to_real,
	},
	{ /* short_integer */
		NULL, NULL, NULL, NULL,
		NULL, NULL,
		hs_real_to_short,
		NULL,
		hs_long_to_short,
	},
	{ /* long_integer */
		NULL, NULL, NULL, NULL,
		NULL, NULL,
		hs_real_to_long,
		NULL,
		hs_long_to_short,
	},
	{ NULL }, /* string */
	{ NULL }, /* script */
	{ NULL }, /* trigger_volume */
	{ NULL }, /* cutscene_flag */
	{ NULL }, /* cutscene_camera_point */
	{ NULL }, /* cutscene_title */
	{ NULL }, /* cutscene_recording */
	{ NULL }, /* device_group */
	{ NULL }, /* ai */
	{ NULL }, /* ai_command_list */
	{ NULL }, /* starting_profile */
	{ NULL }, /* conversation */
	{ NULL }, /* navpoint */
	{ NULL }, /* hud_message */
	{ /* object_list */
		NULL, NULL, NULL, NULL,
		NULL, NULL, NULL, NULL,
		NULL, NULL, NULL, NULL,
		NULL, NULL, NULL, NULL,
		object_list_from_ai_reference,
		NULL, NULL, NULL, NULL,
		NULL, NULL, NULL, NULL,
		NULL, NULL, NULL, NULL,
		NULL, NULL, NULL, NULL,
		NULL,
		hs_object_to_object_list, hs_object_to_object_list, hs_object_to_object_list, hs_object_to_object_list,
		hs_object_to_object_list,
		hs_object_name_to_object_list, hs_object_name_to_object_list, hs_object_name_to_object_list, hs_object_name_to_object_list,
		hs_object_name_to_object_list, hs_object_name_to_object_list,
	},
};
static struct hs_runtime_globals hs_runtime_globals;

/* per-type fallbacks used when an external global has no backing address */
extern boolean const _hs_type_boolean_default;
extern real const _hs_type_real_default;
extern short const _hs_type_short_integer_default;
extern long const _hs_type_long_integer_default;
extern char const *_hs_type_string_default;
extern short const _hs_type_script_default;
extern short const _hs_type_trigger_volume_default;
extern short const _hs_type_cutscene_flag_default;
extern short const _hs_type_cutscene_camera_point_default;
extern short const _hs_type_cutscene_title_default;
extern short const _hs_type_cutscene_recording_default;
extern short const _hs_type_device_group_default;
extern long const _hs_type_ai_default;
extern short const _hs_type_ai_command_list_default;
extern short const _hs_type_starting_profile_default;
extern short const _hs_type_conversation_default;
extern short const _hs_type_navpoint_default;
extern short const _hs_type_hud_message_default;
extern long const _hs_type_object_list_default;
extern long const _hs_type_sound_default;
extern long const _hs_type_effect_default;
extern long const _hs_type_damage_default;
extern long const _hs_type_looping_sound_default;
extern long const _hs_type_animation_graph_default;
extern long const _hs_type_actor_variant_default;
extern long const _hs_type_damage_effect_default;
extern long const _hs_type_object_definition_default;
extern short const _hs_type_enum_game_difficulty_default;
extern short const _hs_type_enum_team_default;
extern short const _hs_type_enum_ai_default_state_default;
extern short const _hs_type_enum_actor_type_default;
extern short const _hs_type_enum_hud_corner_default;
extern long const _hs_type_object_default;
extern long const _hs_type_unit_default;
extern long const _hs_type_vehicle_default;
extern long const _hs_type_weapon_default;
extern long const _hs_type_device_default;
extern long const _hs_type_scenery_default;
extern short const _hs_type_object_name_default;
boolean debug_trigger_volumes;

/* ---------- public code */

void hs_runtime_initialize(
	void)
{
	short global_index;
	long index;

	hs_thread_data = game_state_data_new("hs thread", 0x100, 0x218);
	hs_global_data = game_state_data_new("hs globals", 0x400, 8);
	if (hs_thread_data && hs_global_data)
	{
		match_vassert("c:\\halo\\SOURCE\\hs\\hs_runtime.c", 0xa9,
			hs_external_global_count*2<0x400,
			"raise MAXIMUM_NUMBER_OF_HS_GLOBALS.");
		data_make_valid(hs_global_data);
		for (global_index = 0;
			global_index<hs_external_global_count;
			global_index++)
		{
			index = datum_new_at_index(hs_global_data,
				DATUM_INDEX_NEW(global_index, 0xaced));
			match_assert("c:\\halo\\SOURCE\\hs\\hs_runtime.c", 0xb1, index!=NONE);
		}
	}
	else
	{
		error(_error_immediate, "couldn't allocate scripting globals.");
	}

	return;
}

void hs_runtime_initialize_for_new_map(
	void)
{
	long internal_thread_index;

	data_make_valid(hs_thread_data);
	hs_runtime_globals.initialized = TRUE;
	hs_runtime_globals.executing_thread_index = NONE;
	internal_thread_index = hs_thread_new(_hs_thread_type_global_initialize, NONE);

	if (global_scenario_index!=NONE)
	{
		struct scenario *scenario = global_scenario_get();
		struct hs_thread_datum *internal_thread = hs_thread_get(internal_thread_index);
		struct hs_global_datum *global_datum;
		long global_datum_index;
		short global_index;
		short script_index;

		for (global_index = 0;
			global_index<scenario->hs_globals.count;
			global_index++)
		{
			struct hs_global *global = TAG_BLOCK_GET_ELEMENT(
				&global_scenario_get()->hs_globals,
				global_index,
				struct hs_global);

			if (HS_GLOBAL_DESIGNATOR_IS_EXTERNAL((word)global_index))
				global_datum_index = HS_GLOBAL_DESIGNATOR_TO_INDEX(global_index);
			else
				global_datum_index = HS_GLOBAL_DESIGNATOR_TO_INDEX(global_index)+
					hs_external_global_count;
			datum_new_at_index(hs_global_data,
				DATUM_INDEX_NEW(global_datum_index, 0xaced));

			if (HS_GLOBAL_DESIGNATOR_IS_EXTERNAL((word)global_index))
				global_datum_index = HS_GLOBAL_DESIGNATOR_TO_INDEX(global_index);
			else
				global_datum_index = HS_GLOBAL_DESIGNATOR_TO_INDEX(global_index)+
					hs_external_global_count;
			global_datum = datum_get(hs_global_data, global_datum_index);
			internal_thread->script_index = NONE;
			internal_thread->stack->size = 0;
			hs_evaluate(
				internal_thread_index,
				global->initialization_expression_index,
				&global_datum->value);

			if (TEST_FLAG(internal_thread->flags, _hs_thread_in_function_call_bit))
			{
				hs_thread_main(internal_thread_index);
				if (global->type==_hs_type_object_list)
					object_list_add_reference(hs_global_evaluate(global_index));
				match_hs_assert("c:\\halo\\SOURCE\\hs\\hs_runtime.c", 0xe7, internal_thread_index,
					internal_thread->sleep_until==0,
					"a global initialization attempted to sleep.");
			}
			hs_global_reconcile_write(global_index);
		}

		hs_thread_delete(internal_thread_index);

		for (script_index = 0;
			script_index<scenario->hs_scripts.count;
			script_index++)
		{
			struct hs_script *script = TAG_BLOCK_GET_ELEMENT(
				&scenario->hs_scripts,
				script_index,
				struct hs_script);

			if (script->script_type!=_hs_script_static &&
				script->script_type!=_hs_script_stub)
			{
				if (hs_thread_new(_hs_thread_type_script, script_index)==NONE)
					error(_error_immediate, "ran out of script threads.");
			}
		}
	}

	csmemset(hs_debug_data, 0,
		BIT_VECTOR_SIZE_IN_BYTES(MAXIMUM_TRIGGER_VOLUMES_PER_SCENARIO));

	return;
}

void hs_runtime_dispose(
	void)
{
	data_make_invalid(hs_global_data);

	return;
}

void hs_runtime_dispose_from_old_map(
	void)
{
	short global_index;

	data_make_invalid(hs_thread_data);
	for (global_index = hs_external_global_count;
		global_index<hs_global_data->count;
		global_index++)
	{
		if (datum_try_and_get(hs_global_data, global_index))
			datum_delete(hs_global_data, global_index);
	}
	hs_runtime_globals.initialized = FALSE;

	return;
}

static char const *expression_get_function_name(
	long thread_index,
	long expression_index)
{
	long next_expression_index;
	struct hs_syntax_node *syntax_node = hs_syntax_get(expression_index);
	struct hs_thread_datum *thread = hs_thread_get(thread_index);

	while (TRUE)
	{
		if (TEST_FLAG(syntax_node->flags, _hs_syntax_node_script_bit))
			break;

		if (syntax_node->index != 0 ||
			expression_index != thread->stack->expression_index)
		{
			return hs_function_get((word)syntax_node->index)->name;
		}

		next_expression_index = *(long *)thread->stack->data;
		if (next_expression_index == NONE)
			return "(end of script)";

		expression_index = next_expression_index;
		syntax_node = hs_syntax_get(expression_index);
		thread = hs_thread_get(thread_index);
	}

	return TAG_BLOCK_GET_ELEMENT(
		&global_scenario_get()->hs_scripts,
		syntax_node->index,
		struct hs_script)->name;
}

char const *hs_runtime_get_executing_thread_name(
	void)
{
	char const *name;

	name = NULL;
	if (hs_runtime_globals.executing_thread_index != NONE)
		name = hs_thread_format(hs_runtime_globals.executing_thread_index);
	if (!name)
		name = "[unknown]";

	return name;
}

boolean hs_wake_by_name(
	char const *name)
{
	long thread_index;
	boolean result;

	thread_index = hs_find_thread_by_name(name);
	result = FALSE;
	if (thread_index != NONE)
	{
		hs_wake(thread_index);
		result = TRUE;
	}

	return result;
}

void render_debug_scripting(
	void)
{
	char string[0x2800];
	short tab_stops[2];

	if (debug_scripting)
	{
		long thread_index;

		tab_stops[0] = 200;
		tab_stops[1] = 300;
		sprintf(string, "|n|n|nscript name|tsleep time|tfunction");

		for (thread_index = data_next_index(hs_thread_data, NONE);
			hs_runtime_globals.initialized && thread_index != NONE;
			thread_index = data_next_index(hs_thread_data, thread_index))
		{
			struct hs_thread_datum *thread = hs_thread_get(thread_index);

			if (thread->sleep_until >= 0)
			{
				sprintf(
					string + csstrlen(string),
					"|n%s|t",
					hs_thread_format(thread_index));
				sprintf(
					string + csstrlen(string),
					"%d",
					thread->sleep_until
						? thread->sleep_until - game_time_get()
						: 0);
				csstrcat(string, "|t");

				if (thread->stack != (struct hs_stack_frame *)thread->stack_data &&
					thread->sleep_until != -2)
				{
					csstrcat(
						string,
						expression_get_function_name(
							thread_index,
							thread->stack->expression_index));
				}
			}
		}

		string[0x400] = 0;
		draw_string_set_tab_stops(tab_stops, NUMBEROF(tab_stops));
		render_debug_string(TRUE, string);
		draw_string_set_tab_stops(tab_stops, 0);
	}

	return;
}

void render_debug_trigger_volumes(
	void)
{
	if (debug_trigger_volumes)
	{
		real_matrix4x3 matrix;
		short volume_index;
		struct scenario *scenario = global_scenario_get();

		for (volume_index = 0;
			volume_index < scenario->trigger_volumes.count;
			volume_index++)
		{
			struct scenario_trigger_volume *volume = TAG_BLOCK_GET_ELEMENT(
				&scenario->trigger_volumes,
				volume_index,
				struct scenario_trigger_volume);
			real_vector3d local_extent;
			real_vector3d world_extent;
			short edge_index;
			real_point3d corner[4];

			switch (volume->type)
			{
			case _scenario_trigger_volume_type_axis_aligned:
				matrix = *global_identity4x3;
				set_real_point3d(
					&matrix.position,
					volume->bounds.x0,
					volume->bounds.y0,
					volume->bounds.z0);
				set_real_vector3d(
					&local_extent,
					volume->bounds.x1 - volume->bounds.x0,
					volume->bounds.y1 - volume->bounds.y0,
					volume->bounds.z1 - volume->bounds.z0);
				world_extent = local_extent;
				break;

			case _scenario_trigger_volume_type_oriented:
				local_extent = volume->extents;
				world_extent = volume->extents;
				matrix4x3_from_point_and_vectors(
					&matrix,
					&volume->position,
					&volume->forward,
					&volume->up);
				matrix4x3_transform_vector(&matrix, &local_extent, &world_extent);
				break;

			/* matrix is left unassigned only by this default arm. Not reached unassigned: the
			 * arm's assertion failure calls system_exit, which does not return in January
			 * (0x47c960 jumps to halt_and_catch_fire 0x4f21c0, which loops or calls exit).
			 * Source-policy approval pending (2026-09-27 audit). */
			default:
				match_assert(
					"c:\\halo\\SOURCE\\hs\\hs_runtime.c",
					0x213,
					!"unreachable");
				break;
			}

			for (edge_index = 0; edge_index < 6; edge_index++)
			{
				real_vector3d sides[2] = { 0 };
				short side = edge_index % 2;
				short axis = edge_index / 2;

				if (side)
				{
					point_from_line3d(&matrix.position, &world_extent, 1.0f, &corner[0]);
					sides[0].n[(axis + 1) % 3] = -local_extent.n[(axis + 1) % 3];
					sides[1].n[(axis + 2) % 3] = -local_extent.n[(axis + 2) % 3];
					matrix4x3_transform_vector(&matrix, &sides[0], &sides[0]);
					matrix4x3_transform_vector(&matrix, &sides[1], &sides[1]);
					point_from_line3d(&corner[0], &sides[0], 1.0f, &corner[1]);
					point_from_line3d(&corner[1], &sides[1], 1.0f, &corner[2]);
					point_from_line3d(&corner[2], &sides[0], -1.0f, &corner[3]);
				}
				else
				{
					corner[0] = matrix.position;
					sides[0].n[(axis + 1) % 3] = local_extent.n[(axis + 1) % 3];
					sides[1].n[(axis + 2) % 3] = local_extent.n[(axis + 2) % 3];
					matrix4x3_transform_vector(&matrix, &sides[0], &sides[0]);
					matrix4x3_transform_vector(&matrix, &sides[1], &sides[1]);
					point_from_line3d(&corner[0], &sides[0], 1.0f, &corner[1]);
					point_from_line3d(&corner[1], &sides[1], 1.0f, &corner[2]);
					point_from_line3d(&corner[2], &sides[0], -1.0f, &corner[3]);
				}

				if (BIT_VECTOR_TEST_FLAG(hs_debug_data, volume_index))
				{
					render_debug_polygon_edges(corner, 4, global_real_argb_blue);
				}
				else
				{
					real_argb_color color = *global_real_argb_blue;

					color.alpha = 0.15f;
					render_debug_polygon_edges(corner, 4, global_real_argb_red);
					render_debug_polygon(corner, 4, &color);
				}
			}

			{
				real_point3d center;
				real_vector3d ray;
				struct collision_result result;

				point_from_line3d(&matrix.position, &world_extent, 0.5f, &center);
				vector_from_points3d(&render.camera.position, &center, &ray);
				scale_vector3d(&ray, 0.95f, &ray);
				if (!collision_test_vector(
					_collision_test_for_line_of_sight_flags,
					&render.camera.position,
					&ray,
					NONE,
					&result))
				{
					if (BIT_VECTOR_TEST_FLAG(hs_debug_data, volume_index))
					{
						render_debug_string_at_point(TRUE, &center, volume->name, global_real_argb_yellow);
					}
					else
					{
						render_debug_string_at_point(TRUE, &center, volume->name, global_real_argb_white);
					}
				}
			}
		}
	}

	return;
}

void hs_runtime_update(
	void)
{
	if (hs_runtime_globals.initialized)
	{
		long time = game_time_get();
		boolean console_command_running = FALSE;
		long thread_index;

		for (thread_index = data_next_index(hs_thread_data, NONE);
			hs_runtime_globals.initialized && thread_index!=NONE;
			thread_index = data_next_index(hs_thread_data, thread_index))
		{
			struct hs_thread_datum *thread = hs_thread_get(thread_index);

			if (thread->type==_hs_thread_type_console_command)
				console_command_running = TRUE;
			if (thread->sleep_until>=0 && thread->sleep_until<=time)
				hs_thread_main(thread_index);
		}

		object_list_gc();
		if (!console_command_running && game_time_get()%16==0)
			hs_node_gc();
	}

	return;
}

long hs_runtime_evaluate(
	long expression_index)
{
	long result = NONE;

	if (hs_runtime_globals.initialized && expression_index!=NONE)
	{
		long thread_index = datum_new(hs_thread_data);

		if (thread_index!=NONE)
		{
			struct hs_thread_datum *thread = hs_thread_get(thread_index);

			thread->stack = (struct hs_stack_frame *)thread->stack_data;
			thread->stack->previous = NULL;
			thread->stack->size = 0;
			thread->stack->expression_index = NONE;
			thread->type = _hs_thread_type_console_command;
			thread->script_index = NONE;
			thread->flags = 0;
			thread->sleep_until = 0;

			thread = hs_thread_get(thread_index);
			hs_evaluate(thread_index, expression_index, &thread->result);
			if (TEST_FLAG(thread->flags, _hs_thread_in_function_call_bit))
			{
				hs_thread_main(thread_index);

				return NONE;
			}

			return thread->result;
		}

		error(_error_silent, "there are not enough threads to execute that command.");
	}

	return result;
}

void hs_evaluate_wake(
	short function_index,
	long thread_index,
	boolean initialize)
{
	long wake_thread_index;
	struct hs_thread_datum *thread;
	struct hs_syntax_node *script_name_node;
	long script_name_node_index;

	thread = datum_get(hs_thread_data, thread_index);
	script_name_node_index = ((struct hs_syntax_node *)datum_get(
		hs_syntax_data,
		((struct hs_syntax_node *)datum_get(
			hs_syntax_data,
			thread->stack->expression_index))->data))->next_node_index;
	script_name_node = datum_get(hs_syntax_data, script_name_node_index);

	match_assert("c:\\halo\\source\\hs\\hs_library_internal_runtime.h", 0x22c,
		function_index==_hs_function_wake);
	match_assert("c:\\halo\\source\\hs\\hs_library_internal_runtime.h", 0x22d,
		TEST_FLAG(script_name_node->flags, _hs_syntax_node_primitive_bit));
	match_assert("c:\\halo\\source\\hs\\hs_library_internal_runtime.h", 0x22e,
		script_name_node->type==_hs_type_script);

	wake_thread_index = hs_find_thread_by_script((short)script_name_node->data);
	if (wake_thread_index != NONE)
		hs_wake(wake_thread_index);
	hs_return(thread_index, 0);

	return;
}

/* ---------- private code */

static long hs_syntax_nth(
	long expression_index,
	short n)
{
	short index;

	for (index = 0; index<n; index++)
		expression_index = hs_syntax_get(expression_index)->next_node_index;

	return expression_index;
}

static long hs_thread_new(
	short type,
	long script_index)
{
	long thread_index = datum_new(hs_thread_data);
	struct hs_thread_datum *thread;

	match_assert("c:\\halo\\SOURCE\\hs\\hs_runtime.c", 0x26f,
		type>=0 && type<NUMBER_OF_HS_THREAD_TYPES);
	match_assert("c:\\halo\\SOURCE\\hs\\hs_runtime.c", 0x270,
		type!=_hs_thread_type_script || script_index!=NONE);

	if (thread_index!=NONE)
	{
		thread = hs_thread_get(thread_index);
		thread->stack = (struct hs_stack_frame *)thread->stack_data;
		thread->stack->previous = NULL;
		thread->stack->size = 0;
		thread->stack->expression_index = NONE;
		thread->type = (byte)type;
		thread->script_index = script_index;
		thread->flags = 0;
		if (script_index!=NONE &&
			TAG_BLOCK_GET_ELEMENT(
				&global_scenario_get()->hs_scripts,
				script_index,
				struct hs_script)->script_type==_hs_script_dormant)
		{
			thread->sleep_until = NONE-1;
		}
		else
		{
			thread->sleep_until = 0;
		}
	}

	return thread_index;
}

static void hs_thread_delete(
	long thread_index)
{
	match_assert("c:\\halo\\SOURCE\\hs\\hs_runtime.c", 0x290,
		hs_thread_get(thread_index)->type!=_hs_thread_type_script);

	datum_delete(hs_thread_data, thread_index);

	return;
}

static void hs_stack_push(
	long thread_index)
{
	struct hs_thread_datum *thread = hs_thread_get(thread_index);
	struct hs_stack_frame *new_frame = (struct hs_stack_frame *)
		((byte *)thread->stack+thread->stack->size+sizeof(struct hs_stack_frame));

	match_hs_assert("c:\\halo\\SOURCE\\hs\\hs_runtime.c", 0x35e, thread_index,
		(byte *) (new_frame+1)<thread->stack_data+HS_THREAD_STACK_SIZE,
		"stack overflow.");

	new_frame->previous = thread->stack;
	thread->stack = new_frame;
	new_frame->size = 0;

	return;
}

static char const *hs_thread_format(
	long thread_index)
{
	struct hs_thread_datum *thread;
	char const *name;

	name = NULL;
	thread = datum_get(hs_thread_data, thread_index);
	switch (thread->type)
	{
	case _hs_thread_type_script:
		name = TAG_BLOCK_GET_ELEMENT(
			&global_scenario_get()->hs_scripts,
			((struct hs_thread_datum *)datum_get(
				hs_thread_data,
				thread_index))->script_index,
			struct hs_script)->name;
		break;
	case _hs_thread_type_global_initialize:
		name = "[global initialize]";
		break;
	case _hs_thread_type_console_command:
		name = "[console command]";
		break;
	default:
		display_assert(NULL, "c:\\halo\\SOURCE\\hs\\hs_runtime.c", 0x2a9, TRUE);
		system_exit(-1);
		break;
	}

	return name;
}

static void hs_stack_pop(
	long thread_index)
{
	struct hs_thread_datum *thread;

	thread = datum_get(hs_thread_data, thread_index);
	thread->stack = thread->stack->previous;

	return;
}

static void hs_wake(
	long thread_index)
{
	struct hs_thread_datum *thread;

	thread = datum_get(hs_thread_data, thread_index);
	if (thread->sleep_until != NONE)
	{
		thread->sleep_until = 0;
		if (TEST_FLAG(thread->flags, 1))
		{
			thread->sleep_until = thread->previous_sleep_until;
			SET_FLAG(thread->flags, 1, FALSE);
		}
		else
		{
			if (thread->stack->expression_index != NONE)
			{
				if (((struct hs_syntax_node *)datum_get(hs_syntax_data, thread->stack->expression_index))->index == 0x14)
				{
					hs_stack_pop(thread_index);
					return;
				}
			}

			if (thread->stack->previous &&
				thread->stack->previous->expression_index != NONE)
			{
				if (((struct hs_syntax_node *)datum_get(
					hs_syntax_data,
					thread->stack->previous->expression_index))->index == 0x14)
				{
					hs_stack_pop(thread_index);
					hs_stack_pop(thread_index);
					SET_FLAG(thread->flags, 0, FALSE);
				}
			}
		}
	}

	return;
}

static long hs_find_thread_by_name(
	char const *name)
{
	long thread_index;

	thread_index = data_next_index(hs_thread_data, NONE);
	while (thread_index != NONE)
	{
		struct hs_thread_datum *thread;

		thread = datum_get(hs_thread_data, thread_index);
		if (thread->script_index != NONE)
		{
			struct hs_script const *script;

			script = TAG_BLOCK_GET_ELEMENT(
				&global_scenario_get()->hs_scripts,
				thread->script_index,
				struct hs_script);
			if (_stricmp(script->name, name) == 0)
				return thread_index;
		}
		thread_index = data_next_index(hs_thread_data, thread_index);
	}

	return NONE;
}

static long hs_find_thread_by_script(
	short script_index)
{
	long thread_index;

	thread_index = data_next_index(hs_thread_data, NONE);
	while (thread_index != NONE)
	{
		struct hs_thread_datum *thread;

		thread = datum_get(hs_thread_data, thread_index);
		if (thread->script_index == script_index)
			return thread_index;
		thread_index = data_next_index(hs_thread_data, thread_index);
	}

	return NONE;
}

static void hs_inspect_boolean(
	short type,
	long value,
	char *buffer)
{
	match_assert("c:\\halo\\source\\hs\\hs_library_internal_runtime.h", 0x241,
		type==_hs_type_boolean);

	sprintf(buffer, "%s", (boolean)value ? "true" : "false");

	return;
}

static void hs_inspect_real(
	short type,
	long value,
	char *buffer)
{
	match_assert("c:\\halo\\source\\hs\\hs_library_internal_runtime.h", 0x24c,
		type==_hs_type_real);

	sprintf(buffer, "%f", *(real *)&value);

	return;
}

static void hs_inspect_short_integer(
	short type,
	long value,
	char *buffer)
{
	match_assert("c:\\halo\\source\\hs\\hs_library_internal_runtime.h", 0x257,
		type==_hs_type_short_integer);

	sprintf(buffer, "%d", (short)value);

	return;
}

static void hs_inspect_long_integer(
	short type,
	long value,
	char *buffer)
{
	match_assert("c:\\halo\\source\\hs\\hs_library_internal_runtime.h", 0x262,
		type==_hs_type_long_integer);

	sprintf(buffer, "%ld", value);

	return;
}

static void hs_inspect_string(
	short type,
	long value,
	char *buffer)
{
	match_assert("c:\\halo\\source\\hs\\hs_library_internal_runtime.h", 0x26d,
		type==_hs_type_string);

	sprintf(buffer, "%s", (char const *)value);

	return;
}

static void hs_inspect_enum(
	short type,
	long value,
	char *buffer)
{
	struct hs_enum_definition const *enum_definition;

	enum_definition = &hs_enum_table[type-_hs_type_enum_game_difficulty];
	match_assert("c:\\halo\\source\\hs\\hs_library_internal_runtime.h", 0x27b,
		HS_TYPE_IS_ENUM(type));
	match_vassert("c:\\halo\\source\\hs\\hs_library_internal_runtime.h", 0x27c,
		(short)value>=0 && (short)value<enum_definition->count,
		"enum_value>=0 && enum_value<enum_definition->count");

	sprintf(buffer, "%s", enum_definition->values[(short)value]);

	return;
}

static long hs_long_to_boolean(
	long n)
{
	long result;

	/* BUG: only the low byte of this uninitialised long is written, and the whole long is returned; its
	   upper three bytes are indeterminate. January homes result in the argument slot, so they are bits 8-31
	   of n; where hs_string_to_boolean inlines this function they come from its 'push ecx' slot, which holds
	   hs_cast's table index 254 (0x000000fe), so they are zero there. No January consumer reads a boolean
	   cell beyond its low byte, but hs_return and the pass-through forms copy the whole long into hs thread
	   stacks and hs globals, so these bytes reach the saved-game CRC and the save files
	   (game_state_write_to_persistent_storage, game_state_write_to_file, game_state_write_core). */
	*(boolean *)&result = n==0;

	return result;
}

static long hs_short_to_boolean(
	long s)
{
	long result;

	/* BUG: only the low byte of this uninitialised long is written, and the whole long is returned; its
	   upper three bytes are indeterminate. January homes result in the argument slot, so they are bits 8-31
	   of s. No January consumer reads a boolean cell beyond its low byte, but hs_return and the pass-through
	   forms copy the whole long into hs thread stacks and hs globals, so these bytes reach the saved-game CRC
	   and the save files (game_state_write_to_persistent_storage, game_state_write_to_file,
	   game_state_write_core). */
	*(boolean *)&result = (short)s==0;

	return result;
}

static long hs_string_to_boolean(
	long n)
{
	return hs_long_to_boolean(csstrlen((char const *)n));
}

static long hs_data_to_void(
	long n)
{
	return 0;
}

static long hs_short_to_real(
	long s)
{
	long result;

	*(real *)&result = (real)(short)s;

	return result;
}

static long hs_long_to_real(
	long l)
{
	long result;

	*(real *)&result = (real)l;

	return result;
}

static long hs_enum_to_real(
	long e)
{
	long result;

	*(real *)&result = (real)((short)e+1);

	return result;
}

static long hs_real_to_short(
	long r)
{
	long result;

	/* BUG: only the low word of this uninitialised long is written, and the whole long is returned; its
	   upper word is indeterminate. January homes result in the argument slot, so it is the upper word of r's
	   bit pattern. No January consumer reads a short cell beyond its low word, but hs_return and the
	   pass-through forms copy the whole long into hs thread stacks and hs globals, so these bytes reach the
	   saved-game CRC and the save files (game_state_write_to_persistent_storage, game_state_write_to_file,
	   game_state_write_core). */
	*(short *)&result = (short)*(real *)&r;

	return result;
}

static long hs_real_to_long(
	long r)
{
	return (long)*(real *)&r;
}

static long hs_long_to_short(
	long l)
{
	long result;

	/* BUG: only the low word of this uninitialised long is written, and the whole long is returned; its
	   upper word is indeterminate. January homes result in the argument slot, so it is the upper word of l
	   (January returns l unchanged). No January consumer reads a short cell beyond its low word, but
	   hs_return and the pass-through forms copy the whole long into hs thread stacks and hs globals, so these
	   bytes reach the saved-game CRC and the save files (game_state_write_to_persistent_storage,
	   game_state_write_to_file, game_state_write_core). */
	*(short *)&result = (short)l;

	return result;
}

static long hs_object_name_to_object_list(
	long object_name_index)
{
	long object_index;
	long object_list_index = NONE;

	object_index = object_index_from_name_index((short)object_name_index);
	if (object_index != NONE)
	{
		object_list_index = object_list_new();
		object_list_add(object_list_index, object_index);
	}

	return object_list_index;
}

static long hs_object_to_object_list(
	long object_index)
{
	long object_list_index = NONE;

	if (object_index != NONE)
	{
		object_list_index = object_list_new();
		object_list_add(object_list_index, object_index);
	}

	return object_list_index;
}

static boolean hs_object_type_can_cast(
	short actual_type,
	short desired_type)
{
	word actual_type_mask;
	word desired_type_mask;

	actual_type_mask = hs_object_type_masks[actual_type];
	desired_type_mask = hs_object_type_masks[desired_type];

	match_assert("c:\\halo\\SOURCE\\hs\\hs_runtime.c", 0x599,
		actual_type>=0 && actual_type<NUMBER_OF_HS_OBJECT_TYPES);
	match_assert("c:\\halo\\SOURCE\\hs\\hs_runtime.c", 0x59a,
		desired_type>=0 && desired_type<NUMBER_OF_HS_OBJECT_TYPES);

	return (actual_type_mask & desired_type_mask)==actual_type_mask;
}

boolean hs_can_cast(
	short actual_type,
	short desired_type)
{
	short object_type;

	match_assert("c:\\halo\\SOURCE\\hs\\hs_runtime.c", 0x5a4,
		actual_type==_hs_passthrough || hs_type_valid(actual_type));
	match_assert("c:\\halo\\SOURCE\\hs\\hs_runtime.c", 0x5a5,
		hs_type_valid(desired_type));

	if (actual_type==_hs_passthrough || actual_type==desired_type)
	{
		return TRUE;
	}

	if (HS_TYPE_IS_OBJECT(desired_type))
	{
		object_type = desired_type - _hs_type_object;
		if (HS_TYPE_IS_OBJECT(actual_type))
		{
			return hs_object_type_can_cast(
				actual_type-_hs_type_object,
				object_type);
		}
		else if (!HS_TYPE_IS_OBJECT_NAME(actual_type))
		{
			return FALSE;
		}

		goto cast_object_type;
	}
	else if (HS_TYPE_IS_OBJECT_NAME(desired_type))
	{
		if (!HS_TYPE_IS_OBJECT_NAME(actual_type))
		{
			return FALSE;
		}

		object_type = desired_type - _hs_type_object_name;

cast_object_type:
		return hs_object_type_can_cast(
			actual_type-_hs_type_object_name,
			object_type);
	}
	else
	{
		return typecasting_procedures[desired_type][actual_type] != NULL;
	}
}

long hs_cast(
	long thread_index,
	short actual_type,
	short desired_type,
	long value)
{
	match_hs_assert("c:\\halo\\SOURCE\\hs\\hs_runtime.c", 0x5d8, thread_index,
		hs_can_cast(actual_type, desired_type), "bad typecast.");

	if (actual_type!=desired_type &&
		actual_type!=_hs_passthrough &&
		!HS_TYPE_IS_OBJECT_NAME(desired_type))
	{
		if (!HS_TYPE_IS_OBJECT(desired_type))
			return typecasting_procedures[desired_type][actual_type](value);
		else if (HS_TYPE_IS_OBJECT_NAME(actual_type))
			return object_index_from_name_index((short)value);
	}

	return value;
}

void hs_return(
	long thread_index,
	long value)
{
	struct hs_thread_datum *thread = hs_thread_get(thread_index);
	struct hs_syntax_node *expression = hs_syntax_get(thread->stack->expression_index);
	short return_type;

	match_hs_assert("c:\\halo\\SOURCE\\hs\\hs_runtime.c", 0x325, thread_index,
		valid_thread(thread), "corrupted stack.");

	if (!TEST_FLAG(expression->flags, _hs_syntax_node_script_bit))
	{
		return_type = hs_function_get(expression->index)->return_type;
	}
	else
	{
		return_type = TAG_BLOCK_GET_ELEMENT(
			&global_scenario_get()->hs_scripts,
			expression->index,
			struct hs_script)->return_type;
	}

	*(long *)thread->stack->previous->result = hs_cast(
		thread_index,
		return_type,
		expression->type,
		value);

	thread = hs_thread_get(thread_index);
	thread->stack = thread->stack->previous;

	return;
}

long *hs_macro_function_evaluate(
	short function_index,
	long thread_index,
	boolean initialize)
{
	struct hs_function_definition *function = hs_function_get(function_index);

	return hs_arguments_evaluate(
		thread_index,
		function->parameter_count,
		function->parameter_types,
		initialize);
}

void hs_evaluate_begin(
	short function_index,
	long thread_index,
	boolean initialize)
{
	struct hs_thread_datum *thread = hs_thread_get(thread_index);
	long *expression_index = hs_stack_allocate(thread_index, sizeof(long));
	long *result = hs_stack_allocate(thread_index, sizeof(long));

	match_assert("c:\\halo\\source\\hs\\hs_library_internal_runtime.h", 0x15,
		function_index==_hs_function_begin);

	if (initialize)
	{
		*expression_index = hs_syntax_get(hs_syntax_get(
			thread->stack->expression_index)->data)->next_node_index;
		*result = 0;
	}

	if (*expression_index!=NONE)
	{
		hs_evaluate(thread_index, *expression_index, result);
		*expression_index = hs_syntax_get(*expression_index)->next_node_index;
	}
	else
	{
		hs_return(thread_index, *result);
	}

	return;
}


void hs_evaluate_equality(
	short function_index,
	long thread_index,
	boolean initialize)
{
	short parameter_types[2];
	long *arguments;
	long result_long;
	short type;

	match_assert("c:\\halo\\source\\hs\\hs_library_internal_runtime.h", 0x131,
		function_index==_hs_function_equal || function_index==_hs_function_not_equal);

	type = hs_syntax_get(hs_syntax_get(hs_syntax_get(
		hs_thread_get(thread_index)->stack->expression_index)->data)->next_node_index)->type;
	parameter_types[0] = parameter_types[1] = type;

	arguments = hs_arguments_evaluate(thread_index, 2, parameter_types, initialize);
	if (arguments)
	{
		boolean equal = csmemcmp(arguments, arguments+1, hs_type_sizes[type])==0;

		if (function_index==_hs_function_not_equal)
			equal = !equal;

		/* BUG: only the low byte of this uninitialised long is written, and the whole long is passed to
		   hs_return; its upper three bytes are indeterminate. January reuses the function_index argument
		   slot, which parameter_types has just filled with the argument type twice, so the long is
		   (type<<16)|equal. No January consumer reads a boolean cell beyond its low byte, but hs_return and
		   the pass-through forms copy the whole long into hs thread stacks and hs globals, so these bytes
		   reach the saved-game CRC and the save files (game_state_write_to_persistent_storage,
		   game_state_write_to_file, game_state_write_core). */
		*(boolean *)&result_long = equal;
		hs_return(thread_index, result_long);
	}

	return;
}

void hs_evaluate_inequality(
	short function_index,
	long thread_index,
	boolean initialize)
{
	static short parameter_types[2];
	long *arguments;

	match_assert("c:\\halo\\source\\hs\\hs_library_internal_runtime.h", 0x15d,
		function_index>=_hs_function_gt && function_index<=_hs_function_lte);

	parameter_types[0] = parameter_types[1] = hs_syntax_get(hs_syntax_get(hs_syntax_get(
		hs_thread_get(thread_index)->stack->expression_index)->data)->next_node_index)->type;

	arguments = hs_arguments_evaluate(thread_index, 2, parameter_types, initialize);
	if (arguments)
	{
		long result_long;
		boolean comparison;
		real value0;
		real value1;

		switch (parameter_types[0])
		{
		case _hs_type_real:
			HS_EVALUATE_INEQUALITY(((real *)arguments)[0], ((real *)arguments)[1], 0x16b);
			break;
		case _hs_type_long_integer:
			HS_EVALUATE_INEQUALITY((real)arguments[0], (real)arguments[1], 0x16e);
			break;
		default:
			match_assert("c:\\halo\\source\\hs\\hs_library_internal_runtime.h", 0x171,
				parameter_types[0]==_hs_type_short_integer || HS_TYPE_IS_ENUM(parameter_types[0]));

			HS_EVALUATE_INEQUALITY((real)(short)arguments[0], (real)(short)arguments[1], 0x172);
			break;
		}

		/* BUG: only the low byte of this uninitialised long is written, and the whole long is passed to
		   hs_return; its upper three bytes are indeterminate. January reuses the function_index argument
		   slot, where it has just spilled value1, so they are bits 8-31 of value1's bit pattern. No January
		   consumer reads a boolean cell beyond its low byte, but hs_return and the pass-through forms copy
		   the whole long into hs thread stacks and hs globals, so these bytes reach the saved-game CRC and
		   the save files (game_state_write_to_persistent_storage, game_state_write_to_file,
		   game_state_write_core). */
		*(boolean *)&result_long = comparison;
		hs_return(thread_index, result_long);
	}

	return;
}

void hs_evaluate_logical(
	short function_index,
	long thread_index,
	boolean initialize)
{
	struct hs_thread_datum *thread = hs_thread_get(thread_index);
	long *expression_index = hs_stack_allocate(thread_index, sizeof(long));
	boolean *value = hs_stack_allocate(thread_index, sizeof(long));
	boolean *result = hs_stack_allocate(thread_index, sizeof(boolean));
	boolean and = function_index==_hs_function_and;
	long result_long;

	match_assert("c:\\halo\\source\\hs\\hs_library_internal_runtime.h", 0xcf,
		function_index==_hs_function_and || function_index==_hs_function_or);

	if (initialize)
	{
		*expression_index = hs_syntax_get(hs_syntax_get(
			thread->stack->expression_index)->data)->next_node_index;
		*result = and;
	}
	else
	{
		boolean argument = *value;

		if (and)
			*result = *result && argument;
		else
			*result = *result || argument;
	}

	if (*expression_index!=NONE && *result==and)
	{
		hs_evaluate(thread_index, *expression_index, (long *)value);
		*expression_index = hs_syntax_get(*expression_index)->next_node_index;
	}
	else
	{
		/* BUG: only the low byte of this uninitialised long is written, and the whole long is passed to
		   hs_return; its upper three bytes are indeterminate. January reuses the initialize argument slot,
		   so they are the upper bytes of the dword hs_thread_main pushed for initialize (stack residue from
		   hs_thread_main's frame). No January consumer reads a boolean cell beyond its low byte, but
		   hs_return and the pass-through forms copy the whole long into hs thread stacks and hs globals, so
		   these bytes reach the saved-game CRC and the save files (game_state_write_to_persistent_storage,
		   game_state_write_to_file, game_state_write_core). */
		*(boolean *)&result_long = *result;

		hs_return(thread_index, result_long);
	}

	return;
}

void hs_evaluate_if(
	short function_index,
	long thread_index,
	boolean initialize)
{
	struct hs_thread_datum *thread = hs_thread_get(thread_index);
	long *condition = hs_stack_allocate(thread_index, sizeof(long));
	long *expression_index = hs_stack_allocate(thread_index, sizeof(long));
	long *result = hs_stack_allocate(thread_index, sizeof(long));

	match_assert("c:\\halo\\source\\hs\\hs_library_internal_runtime.h", 0x77,
		function_index==_hs_function_if);

	if (initialize)
	{
		*condition = 0;
		*expression_index = NONE;
		hs_evaluate(thread_index, hs_syntax_get(hs_syntax_get(
			thread->stack->expression_index)->data)->next_node_index,
			condition);
	}
	else if (*expression_index==NONE)
	{
		if ((boolean)*condition)
		{
			*expression_index = hs_syntax_get(hs_syntax_get(hs_syntax_get(
				thread->stack->expression_index)->data)->next_node_index)->next_node_index;
		}
		else
		{
			*expression_index = hs_syntax_get(hs_syntax_get(hs_syntax_get(hs_syntax_get(
				thread->stack->expression_index)->data)->next_node_index)->
				next_node_index)->next_node_index;
			if (*expression_index==NONE)
			{
				hs_return(thread_index, 0);

				return;
			}
		}

		hs_evaluate(thread_index, *expression_index, result);
	}
	else
	{
		hs_return(thread_index, *result);
	}

	return;
}

void hs_evaluate_set(
	short function_index,
	long thread_index,
	boolean initialize)
{
	struct hs_thread_datum *thread = hs_thread_get(thread_index);
	long variable_expression_index = hs_syntax_get(hs_syntax_get(
		thread->stack->expression_index)->data)->next_node_index;
	struct hs_syntax_node *variable = hs_syntax_get(variable_expression_index);
	short type;
	long global_index;

	hs_stack_allocate(thread_index, sizeof(long));
	type = hs_global_get_type((short)variable->data);

	if (initialize)
	{
		if (type==_hs_type_object_list)
			object_list_remove_reference(hs_global_evaluate((short)variable->data));

		if (HS_GLOBAL_DESIGNATOR_IS_EXTERNAL(variable->data))
			global_index = HS_GLOBAL_DESIGNATOR_TO_INDEX((short)variable->data);
		else
			global_index = HS_GLOBAL_DESIGNATOR_TO_INDEX((short)variable->data)+
				hs_external_global_count;

		hs_evaluate(thread_index,
			hs_syntax_get(variable_expression_index)->next_node_index,
			&((struct hs_global_datum *)datum_get(hs_global_data,
				global_index))->value);
	}
	else
	{
		hs_global_reconcile_write((short)variable->data);
		if (type==_hs_type_object_list)
			object_list_add_reference(hs_global_evaluate((short)variable->data));

		hs_return(thread_index, hs_global_evaluate((short)variable->data));
	}

	return;
}

void hs_evaluate_inspect(
	short function_index,
	long thread_index,
	boolean initialize)
{
	char string[1024];
	struct hs_thread_datum *thread = hs_thread_get(thread_index);
	long *value = hs_stack_allocate(thread_index, sizeof(long));

	match_assert("c:\\halo\\source\\hs\\hs_library_internal_runtime.h", 0x2bc,
		function_index==_hs_function_inspect);

	if (initialize)
	{
		hs_evaluate(thread_index, hs_syntax_get(hs_syntax_get(
			thread->stack->expression_index)->data)->next_node_index,
			value);
	}
	else
	{
		struct hs_syntax_node *expression = hs_syntax_get(hs_syntax_get(hs_syntax_get(
			thread->stack->expression_index)->data)->next_node_index);

		if (hs_type_inspectors[expression->type])
		{
			hs_type_inspectors[expression->type](expression->type, *value, string);
			/* BUG (preserved for exact matching): the inspected value's text is passed as the
			 * format (January 0x4bc840 +0xdd..+0xe6), so inspecting a string that contains '%'
			 * reads arguments that were never passed. A corrected build should print it through
			 * "%s". Source-policy approval pending (2026-09-27 audit). */
			console_printf(FALSE, string);
		}

		hs_return(thread_index, 0);
	}

	return;
}

void hs_evaluate_arithmetic(
	short function_index,
	long thread_index,
	boolean initialize)
{
	struct hs_thread_datum *thread = hs_thread_get(thread_index);
	short *argument_index = hs_stack_allocate(thread_index, sizeof(short));
	long *expression_index = hs_stack_allocate(thread_index, sizeof(long));
	real *value = hs_stack_allocate(thread_index, sizeof(real));
	real *result = hs_stack_allocate(thread_index, sizeof(real));
	long result_long;

	if (initialize)
	{
		*argument_index = 0;
		*expression_index = hs_syntax_get(hs_syntax_get(
			thread->stack->expression_index)->data)->next_node_index;
	}
	else
	{
		real argument = *value;

		if (*argument_index==0)
		{
			*result = argument;
		}
		else
		{
			switch (function_index)
			{
			case _hs_function_plus:
				*result = *result+argument;
				break;
			case _hs_function_minus:
				*result = *result-argument;
				break;
			case _hs_function_times:
				*result = *result*argument;
				break;
			case _hs_function_divide:
				*result = *result/argument;
				break;
			case _hs_function_min:
				*result = MIN(*result, argument);
				break;
			case _hs_function_max:
				*result = MAX(*result, argument);
				break;
			default:
				display_assert(NULL, "c:\\halo\\source\\hs\\hs_library_internal_runtime.h", 0x111, TRUE);
				system_exit(-1);
				break;
			}
		}

		*argument_index += 1;
	}

	if (*expression_index!=NONE)
	{
		hs_evaluate(thread_index, *expression_index, (long *)value);
		*expression_index = hs_syntax_get(*expression_index)->next_node_index;
	}
	else
	{
		*(real *)&result_long = *result;

		hs_return(thread_index, result_long);
	}

	return;
}

void hs_evaluate_object_cast_up(
	short function_index,
	long thread_index,
	boolean initialize)
{
	struct hs_thread_datum *thread = hs_thread_get(thread_index);
	long *object_index = hs_stack_allocate(thread_index, sizeof(long));

	match_assert("c:\\halo\\source\\hs\\hs_library_internal_runtime.h", 0x2dc,
		function_index>=_hs_function_object_to_unit &&
		function_index<=_hs_function_object_to_unit);

	if (initialize)
	{
		hs_evaluate(thread_index, hs_syntax_get(hs_syntax_get(
			thread->stack->expression_index)->data)->next_node_index, object_index);
	}
	else if (*object_index!=NONE)
	{
		struct object_datum *object = object_get(*object_index);
		short object_type = function_index-_hs_function_inspect;

		if (TEST_FLAG((short)hs_object_type_masks[object_type], object->object.type))
		{
			hs_return(thread_index, *object_index);
		}
		else
		{
			error(_error_silent, "attempt to convert object %s to type %s",
				tag_get_name(object->definition_index),
				hs_type_names[object_type+_hs_type_object]);
			hs_return(thread_index, NONE);
		}
	}
	else
	{
		hs_return(thread_index, NONE);
	}

	return;
}

void hs_evaluate_begin_random(
	short function_index,
	long thread_index,
	boolean initialize)
{
	struct hs_thread_datum *thread = hs_thread_get(thread_index);
	short *argument_count = hs_stack_allocate(thread_index, sizeof(short));
	long *evaluated = hs_stack_allocate(thread_index, sizeof(long));
	long *result = hs_stack_allocate(thread_index, sizeof(long));
	short random;
	short index;

	match_assert("c:\\halo\\source\\hs\\hs_library_internal_runtime.h", 0x45,
		function_index==_hs_function_begin_random);

	if (initialize)
	{
		long expression_index = hs_syntax_get(hs_syntax_get(
			thread->stack->expression_index)->data)->next_node_index;

		*argument_count = 0;
		while (expression_index!=NONE)
		{
			expression_index = hs_syntax_get(expression_index)->next_node_index;
			*argument_count += 1;
		}

		match_assert("c:\\halo\\source\\hs\\hs_library_internal_runtime.h", 0x50,
			*argument_count<LONG_BITS);

		csmemset(evaluated, 0, BIT_VECTOR_SIZE_IN_BYTES(*argument_count));
	}

	random = random_range(0, *argument_count);
	for (index = 0; index<*argument_count; index++)
	{
		short choice = (short)((random+index)%*argument_count);

		if (!BIT_VECTOR_TEST_FLAG(evaluated, choice))
		{
			hs_evaluate(thread_index, hs_syntax_nth(hs_syntax_get(hs_syntax_get(
				thread->stack->expression_index)->data)->next_node_index, choice), result);
			BIT_VECTOR_SET_FLAG(evaluated, choice, TRUE);

			break;
		}
	}

	if (index==*argument_count)
		hs_return(thread_index, *result);

	return;
}

void hs_evaluate_debug_string(
	short function_index,
	long thread_index,
	boolean initialize)
{
	struct hs_thread_datum *thread = hs_thread_get(thread_index);
	long *expression_index = hs_stack_allocate(thread_index, sizeof(long));
	long *argument_count = hs_stack_allocate(thread_index, sizeof(long));
	char const **arguments = hs_stack_allocate(thread_index,
		MAXIMUM_HS_DEBUG_STRING_ARGUMENTS*sizeof(char const *));

	match_assert("c:\\halo\\source\\hs\\hs_library_internal_runtime.h", 0x304,
		(function_index>=_hs_function_debug_string__first) &&
		(function_index<=_hs_function_debug_string__last));

	if (initialize)
	{
		*expression_index = hs_syntax_get(hs_syntax_get(
			thread->stack->expression_index)->data)->next_node_index;
		*argument_count = 0;
		csmemset((void *)arguments, 0,
			MAXIMUM_HS_DEBUG_STRING_ARGUMENTS*sizeof(char const *));
	}

	if (*expression_index!=NONE && *argument_count<MAXIMUM_HS_DEBUG_STRING_ARGUMENTS)
	{
		long argument;

		hs_evaluate(thread_index, *expression_index, &argument);
		*expression_index = hs_syntax_get(*expression_index)->next_node_index;
		arguments[*argument_count] = (char const *)argument;
		*argument_count += 1;
	}
	else
	{
		hs_debug_string_procedure procedure;

		switch (function_index)
		{
		case _hs_function_debug_string__ai_debug_communication_suppress:
			procedure = ai_debug_communication_suppress;
			break;
		case _hs_function_debug_string__ai_debug_communication_ignore:
			procedure = ai_debug_communication_ignore;
			break;
		case _hs_function_debug_string__ai_debug_communication_focus:
			procedure = ai_debug_communication_focus;
			break;
		default:
			display_assert(NULL, "c:\\halo\\source\\hs\\hs_library_internal_runtime.h", 0x330, TRUE);
			system_exit(-1);
			procedure = NULL;
			break;
		}

		if (procedure)
			procedure(*argument_count, arguments);

		hs_return(thread_index, NONE);
	}

	return;
}

void hs_evaluate_sleep_until(
	short function_index,
	long thread_index,
	boolean initialize)
{
	struct hs_thread_datum *thread = hs_thread_get(thread_index);
	long *condition = hs_stack_allocate(thread_index, sizeof(long));
	long *ticks = hs_stack_allocate(thread_index, sizeof(long));
	long *timeout = hs_stack_allocate(thread_index, sizeof(long));
	long *start_time = hs_stack_allocate(thread_index, sizeof(long));
	short *argument_index = hs_stack_allocate(thread_index, sizeof(short));
	long expression_index = hs_syntax_get(hs_syntax_get(hs_syntax_get(
		thread->stack->expression_index)->data)->next_node_index)->next_node_index;

	match_assert("c:\\halo\\source\\hs\\hs_library_internal_runtime.h", 0x1e5,
		function_index==_hs_function_sleep_until);

	if (initialize)
	{
		*(boolean *)condition = FALSE;
		*start_time = game_time_get();
		*argument_index = 0;
		*(short *)ticks = 30;
		*timeout = NONE;

		if (expression_index!=NONE)
		{
			hs_evaluate(thread_index, expression_index, ticks);

			return;
		}
	}

	if (*argument_index==0)
	{
		*argument_index = 1;
		if (expression_index!=NONE)
		{
			long timeout_expression_index =
				hs_syntax_get(expression_index)->next_node_index;

			if (timeout_expression_index!=NONE)
			{
				hs_evaluate(thread_index, timeout_expression_index, timeout);

				return;
			}
		}
	}

	if (*argument_index==1)
	{
		if ((boolean)*condition ||
			(*timeout!=NONE && game_time_get()>=*timeout+*start_time))
		{
			hs_return(thread_index, 0);
		}
		else
		{
			hs_evaluate(thread_index, hs_syntax_get(hs_syntax_get(
				thread->stack->expression_index)->data)->next_node_index,
				condition);

			thread->sleep_until = game_time_get()+MAX(1, (short)*ticks);
			if (*timeout!=NONE)
			{
				thread->sleep_until = MIN(*timeout+*start_time, thread->sleep_until);
			}
		}
	}

	return;
}

void hs_evaluate_sleep(
	short function_index,
	long thread_index,
	boolean initialize)
{
	struct hs_thread_datum *thread = hs_thread_get(thread_index);
	long *ticks = hs_stack_allocate(thread_index, sizeof(long));
	long *script_index = hs_stack_allocate(thread_index, sizeof(long));
	short *argument_index = hs_stack_allocate(thread_index, sizeof(short));
	long sleep_thread_index = thread_index;

	match_assert("c:\\halo\\source\\hs\\hs_library_internal_runtime.h", 0x189,
		function_index==_hs_function_sleep);

	if (initialize)
	{
		hs_evaluate(thread_index, hs_syntax_get(hs_syntax_get(
			thread->stack->expression_index)->data)->next_node_index, ticks);
		*argument_index = 0;
	}
	else
	{
		if (*argument_index==0)
		{
			long expression_index = hs_syntax_get(hs_syntax_get(hs_syntax_get(
				thread->stack->expression_index)->data)->next_node_index)->next_node_index;

			*argument_index += 1;
			if (expression_index!=NONE)
			{
				hs_evaluate(thread_index, expression_index, script_index);

				return;
			}

			*script_index = NONE;
		}

		if (*argument_index!=0)
		{
			short sleep_ticks = (short)*ticks;

			if (sleep_ticks!=0)
			{
				if ((short)*script_index!=NONE)
					sleep_thread_index = hs_find_thread_by_script((short)*script_index);

				if (sleep_thread_index!=NONE)
				{
					struct hs_thread_datum *sleep_thread = hs_thread_get(sleep_thread_index);
					long sleep_until;

					if (sleep_ticks<0)
						sleep_until = NONE-1;
					else
						sleep_until = game_time_get()+sleep_ticks;

					if (sleep_thread->sleep_until!=NONE)
					{
						if (sleep_thread_index!=thread_index &&
							!TEST_FLAG(sleep_thread->flags, _hs_thread_sleeping_bit))
						{
							SET_FLAG(sleep_thread->flags, _hs_thread_sleeping_bit, TRUE);
							sleep_thread->previous_sleep_until = sleep_thread->sleep_until;
						}

						hs_thread_get(sleep_thread_index)->sleep_until = sleep_until;
					}
				}
			}

			hs_return(thread_index, 0);
		}
	}

	return;
}

static void hs_thread_main(
	long thread_index)
{
	struct hs_thread_datum *thread = hs_thread_get(thread_index);
	struct hs_script *script = NULL;

	hs_runtime_globals.executing_thread_index = (short)thread_index;
	if (thread->type==_hs_thread_type_script)
	{
		script = TAG_BLOCK_GET_ELEMENT(
			&global_scenario_get()->hs_scripts,
			thread->script_index,
			struct hs_script);
		match_hs_assert("c:\\halo\\SOURCE\\hs\\hs_runtime.c", 0x2ba, thread_index,
			script->script_type!=_hs_script_static &&
			script->script_type!=_hs_script_stub,
			"found a static script at toplevel.");
	}

	match_hs_assert("c:\\halo\\SOURCE\\hs\\hs_runtime.c", 0x2bd, thread_index,
		valid_thread(thread), "corrupted stack.");

	thread->sleep_until = 0;
	if (thread->stack==(struct hs_stack_frame *)thread->stack_data)
	{
		match_assert("c:\\halo\\SOURCE\\hs\\hs_runtime.c", 0x2c3, script);

		thread->stack->size = 0;
		hs_evaluate(thread_index, script->root_expression_index,
			hs_stack_allocate(thread_index, sizeof(long)));
	}

	while (thread->stack!=(struct hs_stack_frame *)thread->stack_data &&
		thread->sleep_until>=0 &&
		(!game_in_progress() || thread->sleep_until<=game_time_get()) &&
		hs_runtime_globals.initialized)
	{
		struct hs_syntax_node *expression = hs_syntax_get(thread->stack->expression_index);
		boolean initialize = TEST_FLAG(thread->flags, _hs_thread_in_function_call_bit);

		thread->stack->size = 0;
		SET_FLAG(thread->flags, _hs_thread_in_function_call_bit, FALSE);

		if (!TEST_FLAG(expression->flags, _hs_syntax_node_script_bit))
		{
			struct hs_function_definition *function = hs_function_get(expression->index);

			match_assert("c:\\halo\\SOURCE\\hs\\hs_runtime.c", 0x2d8, function->evaluate);

			function->evaluate(
				expression->index,
				thread_index,
				initialize);
		}
		else
		{
			hs_script_evaluate(expression->index, thread_index, initialize);
		}
	}

	if (thread->stack==(struct hs_stack_frame *)thread->stack_data)
	{
		if (thread->type==_hs_thread_type_script)
		{
			if (script->script_type==_hs_script_startup ||
				script->script_type==_hs_script_dormant)
			{
				thread->sleep_until = NONE;
			}
		}
		else if (thread->type==_hs_thread_type_console_command)
		{
			hs_thread_delete(thread_index);
		}
	}

	hs_runtime_globals.executing_thread_index = NONE;

	return;
}

static void hs_script_evaluate(
	short script_index,
	long thread_index,
	boolean initialize)
{
	struct hs_script *script = TAG_BLOCK_GET_ELEMENT(
		&global_scenario_get()->hs_scripts,
		script_index,
		struct hs_script);
	struct hs_thread_datum *thread = hs_thread_get(thread_index);
	long *result = hs_stack_allocate(thread_index, sizeof(long));

	if (initialize)
		hs_evaluate(thread_index, script->root_expression_index, result);
	else
		hs_return(thread_index, *result);

	return;
}

static void *hs_stack_allocate(
	long thread_index,
	long size)
{
	struct hs_thread_datum *thread = hs_thread_get(thread_index);
	struct hs_stack_frame *frame = thread->stack;
	void *result;

	match_hs_assert("c:\\halo\\SOURCE\\hs\\hs_runtime.c", 0x37d, thread_index,
		valid_thread(thread), "corrupted stack.");
	match_hs_assert("c:\\halo\\SOURCE\\hs\\hs_runtime.c", 0x37e, thread_index,
		size, "attempt to allocate zero space from the stack.");
	match_hs_assert("c:\\halo\\SOURCE\\hs\\hs_runtime.c", 0x37f, thread_index,
		frame->data+frame->size+size<=thread->stack_data+HS_THREAD_STACK_SIZE,
		"stack overflow.");

	result = frame->data+frame->size;
	frame->size += (short)size;

	return result;
}

static void hs_evaluate(
	long thread_index,
	long expression_index,
	long *destination)
{
	struct hs_thread_datum *thread = hs_thread_get(thread_index);
	struct hs_syntax_node *expression = hs_syntax_get(expression_index);

	match_hs_assert("c:\\halo\\SOURCE\\hs\\hs_runtime.c", 0x2ff, thread_index,
		valid_thread(thread), "corrupted stack.");
	match_assert("c:\\halo\\SOURCE\\hs\\hs_runtime.c", 0x300, destination);

	if (TEST_FLAG(hs_syntax_get(expression_index)->flags, _hs_syntax_node_primitive_bit))
	{
		if (TEST_FLAG(expression->flags, _hs_syntax_node_global_bit))
		{
			*destination = hs_cast(
				thread_index,
				hs_global_get_type((short)expression->data),
				expression->type,
				hs_global_evaluate((short)expression->data));
		}
		else
		{
			*destination = hs_cast(
				thread_index,
				expression->index,
				expression->type,
				expression->data);
		}
	}
	else
	{
		thread->stack->result = destination;
		hs_stack_push(thread_index);
		SET_FLAG(thread->flags, _hs_thread_in_function_call_bit, TRUE);
		thread->stack->expression_index = expression_index;
	}

	return;
}

static void hs_global_reconcile_read(
	word global_designator)
{
	struct hs_global_datum *global;
	struct hs_external_global_definition *external;

	if (HS_GLOBAL_DESIGNATOR_IS_EXTERNAL(global_designator))
	{
		long global_index;

		/* the general external/internal index test, repeated inside the external arm, is original (the debug build keeps it) */
		if (HS_GLOBAL_DESIGNATOR_IS_EXTERNAL(global_designator))
			global_index = HS_GLOBAL_DESIGNATOR_TO_INDEX(global_designator);
		else
			global_index = HS_GLOBAL_DESIGNATOR_TO_INDEX(global_designator)+
				hs_external_global_count;
		global = datum_get(hs_global_data, global_index);
		external = hs_global_external_get(
			HS_GLOBAL_DESIGNATOR_TO_INDEX(global_designator));

		switch (hs_global_get_type(global_designator))
		{
		case _hs_type_boolean:
			*(boolean *)&global->value = external->address
				? *(boolean *)external->address
				: _hs_type_boolean_default;
			break;
		case _hs_type_real:
			*(real *)&global->value = external->address
				? *(real *)external->address
				: _hs_type_real_default;
			break;
		case _hs_type_short_integer:
			*(short *)&global->value = external->address
				? *(short *)external->address
				: _hs_type_short_integer_default;
			break;
		case _hs_type_long_integer:
			global->value = external->address
				? *(long *)external->address
				: _hs_type_long_integer_default;
			break;
		case _hs_type_string:
			*(char const **)&global->value = external->address
				? *(char const * *)external->address
				: _hs_type_string_default;
			break;
		case _hs_type_script:
			*(short *)&global->value = external->address
				? *(short *)external->address
				: _hs_type_script_default;
			break;
		case _hs_type_trigger_volume:
			*(short *)&global->value = external->address
				? *(short *)external->address
				: _hs_type_trigger_volume_default;
			break;
		case _hs_type_cutscene_flag:
			*(short *)&global->value = external->address
				? *(short *)external->address
				: _hs_type_cutscene_flag_default;
			break;
		case _hs_type_cutscene_camera_point:
			*(short *)&global->value = external->address
				? *(short *)external->address
				: _hs_type_cutscene_camera_point_default;
			break;
		case _hs_type_cutscene_title:
			*(short *)&global->value = external->address
				? *(short *)external->address
				: _hs_type_cutscene_title_default;
			break;
		case _hs_type_cutscene_recording:
			*(short *)&global->value = external->address
				? *(short *)external->address
				: _hs_type_cutscene_recording_default;
			break;
		case _hs_type_device_group:
			*(short *)&global->value = external->address
				? *(short *)external->address
				: _hs_type_device_group_default;
			break;
		case _hs_type_ai:
			global->value = external->address
				? *(long *)external->address
				: _hs_type_ai_default;
			break;
		case _hs_type_ai_command_list:
			*(short *)&global->value = external->address
				? *(short *)external->address
				: _hs_type_ai_command_list_default;
			break;
		case _hs_type_starting_profile:
			*(short *)&global->value = external->address
				? *(short *)external->address
				: _hs_type_starting_profile_default;
			break;
		case _hs_type_conversation:
			*(short *)&global->value = external->address
				? *(short *)external->address
				: _hs_type_conversation_default;
			break;
		case _hs_type_navpoint:
			*(short *)&global->value = external->address
				? *(short *)external->address
				: _hs_type_navpoint_default;
			break;
		case _hs_type_hud_message:
			*(short *)&global->value = external->address
				? *(short *)external->address
				: _hs_type_hud_message_default;
			break;
		case _hs_type_object_list:
			global->value = external->address
				? *(long *)external->address
				: _hs_type_object_list_default;
			break;
		case _hs_type_sound:
			global->value = external->address
				? *(long *)external->address
				: _hs_type_sound_default;
			break;
		case _hs_type_effect:
			global->value = external->address
				? *(long *)external->address
				: _hs_type_effect_default;
			break;
		case _hs_type_damage:
			global->value = external->address
				? *(long *)external->address
				: _hs_type_damage_default;
			break;
		case _hs_type_looping_sound:
			global->value = external->address
				? *(long *)external->address
				: _hs_type_looping_sound_default;
			break;
		case _hs_type_animation_graph:
			global->value = external->address
				? *(long *)external->address
				: _hs_type_animation_graph_default;
			break;
		case _hs_type_actor_variant:
			global->value = external->address
				? *(long *)external->address
				: _hs_type_actor_variant_default;
			break;
		case _hs_type_damage_effect:
			global->value = external->address
				? *(long *)external->address
				: _hs_type_damage_effect_default;
			break;
		case _hs_type_object_definition:
			global->value = external->address
				? *(long *)external->address
				: _hs_type_object_definition_default;
			break;
		case _hs_type_enum_game_difficulty:
			*(short *)&global->value = external->address
				? *(short *)external->address
				: _hs_type_enum_game_difficulty_default;
			break;
		case _hs_type_enum_team:
			*(short *)&global->value = external->address
				? *(short *)external->address
				: _hs_type_enum_team_default;
			break;
		case _hs_type_enum_ai_default_state:
			*(short *)&global->value = external->address
				? *(short *)external->address
				: _hs_type_enum_ai_default_state_default;
			break;
		case _hs_type_enum_actor_type:
			*(short *)&global->value = external->address
				? *(short *)external->address
				: _hs_type_enum_actor_type_default;
			break;
		case _hs_type_enum_hud_corner:
			*(short *)&global->value = external->address
				? *(short *)external->address
				: _hs_type_enum_hud_corner_default;
			break;
		case _hs_type_object:
			global->value = external->address
				? *(long *)external->address
				: _hs_type_object_default;
			break;
		case _hs_type_unit:
			global->value = external->address
				? *(long *)external->address
				: _hs_type_unit_default;
			break;
		case _hs_type_vehicle:
			global->value = external->address
				? *(long *)external->address
				: _hs_type_vehicle_default;
			break;
		case _hs_type_weapon:
			global->value = external->address
				? *(long *)external->address
				: _hs_type_weapon_default;
			break;
		case _hs_type_device:
			global->value = external->address
				? *(long *)external->address
				: _hs_type_device_default;
			break;
		case _hs_type_scenery:
			global->value = external->address
				? *(long *)external->address
				: _hs_type_scenery_default;
			break;
		case _hs_type_object_name:
			*(short *)&global->value = external->address
				? *(short *)external->address
				: _hs_type_object_name_default;
			break;
		default:
			display_assert(NULL, "c:\\halo\\SOURCE\\hs\\hs_runtime.c", 0x638, TRUE);
			system_exit(-1);
			break;
		}
	}

	return;
}

static void hs_global_reconcile_write(
	word global_designator)
{
	struct hs_global_datum *global;
	struct hs_external_global_definition *external;
	real value;

	if (HS_GLOBAL_DESIGNATOR_IS_EXTERNAL(global_designator))
	{
		long global_index;

		/* the general external/internal index test, repeated inside the external arm, is original (the debug build keeps it) */
		if (HS_GLOBAL_DESIGNATOR_IS_EXTERNAL(global_designator))
			global_index = HS_GLOBAL_DESIGNATOR_TO_INDEX(global_designator);
		else
			global_index = HS_GLOBAL_DESIGNATOR_TO_INDEX(global_designator)+
				hs_external_global_count;
		global = datum_get(hs_global_data, global_index);
		external = hs_global_external_get(
			HS_GLOBAL_DESIGNATOR_TO_INDEX(global_designator));

		switch (hs_global_get_type(global_designator))
		{
		case _hs_type_boolean:
			if (external->address)
				*(boolean *)external->address = (boolean)global->value;
			break;
		case _hs_type_real:
			value = *(real *)&global->value;
			if (external->address)
				*(real *)external->address = value;
			break;
		case _hs_type_short_integer:
			if (external->address)
				*(short *)external->address = (short)global->value;
			break;
		case _hs_type_long_integer:
			if (external->address)
				*(long *)external->address = global->value;
			break;
		case _hs_type_string:
			if (external->address)
				*(char const **)external->address = (char const *)global->value;
			break;
		case _hs_type_script:
			if (external->address)
				*(short *)external->address = (short)global->value;
			break;
		case _hs_type_trigger_volume:
			if (external->address)
				*(short *)external->address = (short)global->value;
			break;
		case _hs_type_cutscene_flag:
			if (external->address)
				*(short *)external->address = (short)global->value;
			break;
		case _hs_type_cutscene_camera_point:
			if (external->address)
				*(short *)external->address = (short)global->value;
			break;
		case _hs_type_cutscene_title:
			if (external->address)
				*(short *)external->address = (short)global->value;
			break;
		case _hs_type_cutscene_recording:
			if (external->address)
				*(short *)external->address = (short)global->value;
			break;
		case _hs_type_device_group:
			if (external->address)
				*(short *)external->address = (short)global->value;
			break;
		case _hs_type_ai:
			if (external->address)
				*(long *)external->address = global->value;
			break;
		case _hs_type_ai_command_list:
			if (external->address)
				*(short *)external->address = (short)global->value;
			break;
		case _hs_type_starting_profile:
			if (external->address)
				*(short *)external->address = (short)global->value;
			break;
		case _hs_type_conversation:
			if (external->address)
				*(short *)external->address = (short)global->value;
			break;
		case _hs_type_navpoint:
			if (external->address)
				*(short *)external->address = (short)global->value;
			break;
		case _hs_type_hud_message:
			if (external->address)
				*(short *)external->address = (short)global->value;
			break;
		case _hs_type_object_list:
			if (external->address)
				*(long *)external->address = global->value;
			break;
		case _hs_type_sound:
			if (external->address)
				*(long *)external->address = global->value;
			break;
		case _hs_type_effect:
			if (external->address)
				*(long *)external->address = global->value;
			break;
		case _hs_type_damage:
			if (external->address)
				*(long *)external->address = global->value;
			break;
		case _hs_type_looping_sound:
			if (external->address)
				*(long *)external->address = global->value;
			break;
		case _hs_type_animation_graph:
			if (external->address)
				*(long *)external->address = global->value;
			break;
		case _hs_type_actor_variant:
			if (external->address)
				*(long *)external->address = global->value;
			break;
		case _hs_type_damage_effect:
			if (external->address)
				*(long *)external->address = global->value;
			break;
		case _hs_type_object_definition:
			if (external->address)
				*(long *)external->address = global->value;
			break;
		case _hs_type_enum_game_difficulty:
			if (external->address)
				*(short *)external->address = (short)global->value;
			break;
		case _hs_type_enum_team:
			if (external->address)
				*(short *)external->address = (short)global->value;
			break;
		case _hs_type_enum_ai_default_state:
			if (external->address)
				*(short *)external->address = (short)global->value;
			break;
		case _hs_type_enum_actor_type:
			if (external->address)
				*(short *)external->address = (short)global->value;
			break;
		case _hs_type_enum_hud_corner:
			if (external->address)
				*(short *)external->address = (short)global->value;
			break;
		case _hs_type_object:
			if (external->address)
				*(long *)external->address = global->value;
			break;
		case _hs_type_unit:
			if (external->address)
				*(long *)external->address = global->value;
			break;
		case _hs_type_vehicle:
			if (external->address)
				*(long *)external->address = global->value;
			break;
		case _hs_type_weapon:
			if (external->address)
				*(long *)external->address = global->value;
			break;
		case _hs_type_device:
			if (external->address)
				*(long *)external->address = global->value;
			break;
		case _hs_type_scenery:
			if (external->address)
				*(long *)external->address = global->value;
			break;
		case _hs_type_object_name:
			if (external->address)
				*(short *)external->address = (short)global->value;
			break;
		default:
			display_assert(NULL, "c:\\halo\\SOURCE\\hs\\hs_runtime.c", 0x671, TRUE);
			system_exit(-1);
			break;
		}
	}

	return;
}

static long hs_global_evaluate(
	short global_designator)
{
	long global_index;

	hs_global_reconcile_read(global_designator);
	if (HS_GLOBAL_DESIGNATOR_IS_EXTERNAL((word)global_designator))
	{
		global_index = HS_GLOBAL_DESIGNATOR_TO_INDEX(global_designator);
	}
	else
	{
		global_index = HS_GLOBAL_DESIGNATOR_TO_INDEX(global_designator)+
			hs_external_global_count;
	}

	return ((struct hs_global_datum *)datum_get(hs_global_data, global_index))->
		value;
}

static long *hs_arguments_evaluate(
	long thread_index,
	short formal_parameter_count,
	short const *formal_parameters,
	boolean initialize)
{
	struct hs_thread_datum *thread = hs_thread_get(thread_index);
	long *values = hs_stack_allocate(thread_index, formal_parameter_count*sizeof(long));
	short *argument_index = hs_stack_allocate(thread_index, sizeof(short));
	long *expression_index = hs_stack_allocate(thread_index, sizeof(long));

	if (initialize)
	{
		*argument_index = 0;
		*expression_index = hs_syntax_get(hs_syntax_get(
			thread->stack->expression_index)->data)->next_node_index;
	}

	if (*argument_index<formal_parameter_count)
	{
		match_hs_assert("c:\\halo\\SOURCE\\hs\\hs_runtime.c", 0x3c4, thread_index,
			*expression_index!=NONE, "corrupted syntax tree.");

		if (hs_syntax_get(*expression_index)->type!=formal_parameters[*argument_index])
		{
			script_error(thread_index, "unexpected actual parameters.",
				"hs_syntax_get(*expression_index)->type==formal_parameters[*argument_index]");

			return values;
		}

		hs_evaluate(thread_index, *expression_index, &values[*argument_index]);
		*expression_index = hs_syntax_get(*expression_index)->next_node_index;
		*argument_index += 1;

		return NULL;
	}

	match_hs_assert("c:\\halo\\SOURCE\\hs\\hs_runtime.c", 0x3d1, thread_index,
		*expression_index==NONE, "corrupted syntax tree.");

	return values;
}

static boolean script_error(
	long thread_index,
	char const *reason,
	char const *expression)
{
	struct hs_thread_datum *thread = hs_thread_get(thread_index);

	error(_error_silent, "script %s needs to be recompiled. (%s: %s)",
		hs_thread_format(thread_index),
		reason ? reason : "no reason given.",
		expression);

	return FALSE;
}
