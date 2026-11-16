/* MCC script linking and MCC-only native functions. Unknown functions cause
 * a load failure; silently removing campaign logic would hide broken maps. */
#include "cseries.h"
#include "errors.h"
#include "ai/ai_script.h"
#include "hs/hs.h"
#include "hs/hs_scenario_definitions.h"
#include "hs/object_lists.h"
#include "memory/data.h"
#include "objects/objects.h"
#include "scenario/scenario_definitions.h"
#include "scenario/scenario.h"
#include "mcc_runtime.h"
#include "mcc_cache.h"
#include "mcc_scripts.h"
#include "mcc_script_parameters.h"
#include "mcc_syntax.h"
#include "mcc_campaign.h"
#include "mcc_player.h"
#include <string.h>
#include <stdio.h>

boolean hs_macro_function_parse(short index, long expression);

boolean mcc_script_ai_cast(short actual_type, short desired_type)
{
    return mcc_cache_tags_loaded() && actual_type == _hs_type_ai &&
        (HS_TYPE_IS_OBJECT(desired_type) || desired_type == _hs_type_object_list);
}

int mcc_script_parse_ai(struct hs_syntax_node *node, char const *text)
{
    if (!node || (node->flags & 5) != 1 || !(node->flags & 8) ||
        !mcc_script_ai_cast(node->constant_type, node->type)) return -1;
    return text && node->source_offset >= 0 &&
        ai_index_from_string(global_scenario_get(), text + node->source_offset, &node->data);
}

long mcc_script_cast_ai(short desired_type, long reference)
{
    long list = object_list_from_ai_reference(reference), cursor, result = NONE;
    if (desired_type == _hs_type_object_list) return list;
    if (list != NONE) {
        long first = object_list_get_first(list, &cursor);
        struct object_datum *object = object_try_and_get(first);
        if (object && HS_TYPE_IS_OBJECT(desired_type) &&
            TEST_FLAG(hs_object_type_masks[desired_type - _hs_type_object], object->object.type))
            result = first;
        object_list_delete(list);
    }
    return result;
}

boolean mcc_script_can_cast(short actual_type,short desired_type)
{
    return mcc_script_ai_cast(actual_type,desired_type) || (mcc_cache_tags_loaded() &&
        actual_type==_hs_type_short_integer && desired_type==_hs_type_long_integer);
}

long mcc_script_cast(short actual_type,short desired_type,long value)
{
    /* Widen globals as well as parameters; sign extension is required. */
    if (actual_type==_hs_type_short_integer) return (long)(short)value;
    return mcc_script_cast_ai(desired_type,value);
}

static void mcc_distance_evaluate(short function, long thread, boolean initialize)
{
    long *arguments = hs_macro_function_evaluate(function, thread, initialize);
    union { real value; long bits; } result;
    long object, reference;
    real_point3d destination, origin;
    if (!arguments) return;
    result.value = -1.0f;
    if (object_try_and_get(arguments[1]) && arguments[0] != NONE &&
        datum_try_and_get(object_list_header_data, arguments[0])) {
        object_get_origin(arguments[1], &destination);
        object = object_list_get_first(arguments[0], &reference);
        while (object != NONE) {
            if (object_try_and_get(object)) {
                real distance;
                object_get_origin(object, &origin);
                distance = distance3d(&origin, &destination);
                if (result.value < 0 || distance < result.value) result.value = distance;
            }
            object = object_list_get_next(arguments[0], &reference);
        }
    }
    hs_return(thread, result.bits);
}

static struct {
    struct hs_function_definition definition;
    short second_parameter;
} mcc_distance_definition = {
    { _hs_type_real, 0, "objects_distance_to_object", hs_macro_function_parse,
        mcc_distance_evaluate, "Minimum distance to an object; -1 for no objects.",
        NULL, 2, { _hs_type_object_list } }, _hs_type_object
};

static void mcc_mission_segment_evaluate(short function, long thread, boolean initialize)
{
    long *arguments = hs_macro_function_evaluate(function, thread, initialize);
    char const *segment;
    if (!arguments) return;
    segment = (char const *)(uintptr_t)arguments[0];
    /* This port acknowledges a local segment event, without MCC's external
     * telemetry service. Keep the boolean result: shipped scripts use it in
     * conditional expressions before cinematics. MCC also uses the older
     * core_save_name spelling for segments; the Xbox function with that name
     * instead writes a debug save and must not be called by MCC map scripts.
     * Argument expressions still run normally, including any sleep/yield. */
    if (segment) error(_error_log, "mcc: mission segment '%.128s'", segment);
    hs_return(thread, segment != NULL);
}

static struct hs_function_definition mcc_mission_segment_definition = {
    _hs_type_boolean, 0, "mcc_mission_segment", hs_macro_function_parse,
    mcc_mission_segment_evaluate, "Records a local mission segment and acknowledges it.",
    NULL, 1, { _hs_type_string }
};

/* H1A repurposed this name for campaign segments with a boolean result.
 * Keep it outside the legacy table, including its original debug-save API.
 * Format reference: https://c20.reclaimers.net/h1/scripting/#functions */
static struct hs_function_definition mcc_core_segment_definition = {
    _hs_type_boolean, 0, "core_save_name", hs_macro_function_parse,
    mcc_mission_segment_evaluate, "Records a local mission segment and acknowledges it.",
    NULL, 1, { _hs_type_string }
};

static void mcc_equipment_evaluate(short function, long thread, boolean initialize)
{
    long *arguments = hs_macro_function_evaluate(function, thread, initialize);
    if (!arguments) return;
    mcc_player_add_equipment(arguments[0], (short)arguments[1], (boolean)arguments[2]);
    hs_return(thread, 0);
}

static struct {
    struct hs_function_definition definition;
    short other_parameters[2];
} mcc_equipment_definition = {
    { _hs_type_void, 0, "player_add_equipment", hs_macro_function_parse,
        mcc_equipment_evaluate, "Adds a profile, or replaces all player equipment on reset.",
        NULL, 3, { _hs_type_unit } }, { _hs_type_starting_profile, _hs_type_boolean }
};

/* MCC integer arithmetic retains the 32-bit result for LONG_MIN without
 * invoking C signed-overflow undefined behavior. */
static void mcc_abs_integer_evaluate(short function, long thread, boolean initialize)
{
    long *arguments = hs_macro_function_evaluate(function, thread, initialize);
    unsigned long value;
    if (!arguments) return;
    value = (unsigned long)arguments[0];
    if (arguments[0] < 0) value = 0u - value;
    hs_return(thread, (long)value);
}

static struct hs_function_definition mcc_abs_integer_definition = {
    _hs_type_long_integer, 0, "abs_integer", hs_macro_function_parse,
    mcc_abs_integer_evaluate, "Returns the absolute value of a 32-bit integer.",
    NULL, 1, { _hs_type_long_integer }
};

static struct hs_function_definition mcc_executing_thread_definition = {
    _hs_type_long_integer, 0, "get_executing_running_thread", hs_macro_function_parse,
    mcc_executing_thread_evaluate, "Returns the current script thread handle.",
    NULL, 0, { 0 }
};

static struct hs_function_definition mcc_kill_thread_definition = {
    _hs_type_void, 0, "kill_thread", hs_macro_function_parse,
    mcc_kill_thread_evaluate, "Terminates the specified script thread.",
    NULL, 1, { _hs_type_long_integer }
};

/* MCC scenarios may clear the debug selection. Use the existing operation;
 * the Xbox/CE map allowlist still excludes it and other debug controls. */
static void mcc_ai_deselect_evaluate(short function, long thread, boolean initialize)
{
    (void)function;
    (void)initialize;
    ai_scripting_deselect();
    hs_return(thread, 0);
}

static struct hs_function_definition mcc_ai_deselect_definition = {
    _hs_type_void, 0, "ai_deselect", hs_macro_function_parse,
    mcc_ai_deselect_evaluate, "Clears the selected AI encounter.",
    NULL, 0, { 0 }
};

struct hs_function_definition *mcc_script_function(short index)
{
    if (!mcc_cache_tags_loaded()) return NULL;
    if (index == MCC_HS_DISTANCE_TO_OBJECT) return &mcc_distance_definition.definition;
    if (index == MCC_HS_MISSION_SEGMENT) return &mcc_mission_segment_definition;
    if (index == MCC_HS_CORE_SEGMENT) return &mcc_core_segment_definition;
    if (index == MCC_HS_PLAYER_EQUIPMENT) return &mcc_equipment_definition.definition;
    if (index == MCC_HS_ABS_INTEGER) return &mcc_abs_integer_definition;
    if (index == MCC_HS_EXECUTING_THREAD) return &mcc_executing_thread_definition;
    if (index == MCC_HS_KILL_THREAD) return &mcc_kill_thread_definition;
    if (index == MCC_HS_AI_DESELECT) return &mcc_ai_deselect_definition;
    return mcc_campaign_function(index);
}

/* Preparation runs before MCC activation; runtime lookup stays guarded below.
 * A recognized alias returns its target result even when that target is absent. */
static short mcc_script_resolve(char const *name, boolean native_fallback)
{
    short index;
    if (!csstrcasecmp(name, "player_add_equipment")) return MCC_HS_PLAYER_EQUIPMENT;
    if (!csstrcasecmp(name, "objects_distance_to_object")) return MCC_HS_DISTANCE_TO_OBJECT;
    if (!csstrcasecmp(name, "mcc_mission_segment")) return MCC_HS_MISSION_SEGMENT;
    if (!csstrcasecmp(name, "core_save_name")) return MCC_HS_CORE_SEGMENT;
    if (!csstrcasecmp(name, "player_effect_set_max_vibrate"))
        return hs_find_function_by_name("player_effect_set_max_rumble");
    if (!csstrcasecmp(name, "abs_integer")) return MCC_HS_ABS_INTEGER;
    if (!csstrcasecmp(name, "get_executing_running_thread")) return MCC_HS_EXECUTING_THREAD;
    if (!csstrcasecmp(name, "kill_thread")) return MCC_HS_KILL_THREAD;
    if (!csstrcasecmp(name, "ai_deselect")) return MCC_HS_AI_DESELECT;
    index = mcc_campaign_find(name);
    return index == NONE && native_fallback ? hs_find_function_by_name(name) : index;
}

short mcc_script_find(char const *name)
{
    return mcc_cache_tags_loaded() && name ? mcc_script_resolve(name, FALSE) : NONE;
}

boolean mcc_script_call_arguments_valid(struct hs_syntax_node const *call, struct data_array *syntax)
{
    struct hs_function_definition *function = mcc_script_function(call->function_index);
    struct hs_syntax_node *node;
    long next;
    short argument;
    if (!function || (call->flags & 3) ||
        !(node = datum_try_and_get(syntax, call->data))) return FALSE;
    next = node->next_node_index;
    if (call->function_index == MCC_HS_CAMPAIGN_FIRST + 2) {
        if (next == NONE) return TRUE;
        node = datum_try_and_get(syntax, next);
        return node && node->type == _hs_type_script && node->next_node_index == NONE;
    }
    for (argument = 0; argument < function->parameter_count; ++argument) {
        node = datum_try_and_get(syntax, next);
        if (!node || node->type != function->parameter_types[argument]) return FALSE;
        next = node->next_node_index;
    }
    return next == NONE;
}

/* Map admission still requires authored nodes. Saved console expressions use
 * the same argument grammar while their temporary nodes have another owner. */
boolean mcc_script_call_valid(struct hs_syntax_node const *call, struct data_array *syntax)
{
    return (call->flags & 8) && mcc_script_call_arguments_valid(call, syntax);
}

static char const *mcc_script_string(char const *strings, uint32_t bytes, long offset)
{
    return offset >= 0 && (uint32_t)offset < bytes && memchr(strings + offset, 0, bytes - offset) ?
        strings + offset : NULL;
}

/* Give MCC compatibility variables ordinary scenario storage. This preserves
 * script reads/writes and checkpoint lifetime without adding an external
 * engine global or enabling native developer privileges. */
static short mcc_local_global(struct mcc_runtime *runtime, struct scenario *scenario,
    struct data_array *syntax, char const *name, short type)
{
    extern short const hs_external_global_count;
    struct hs_global *old,*globals;
    struct hs_syntax_node *nodes=(struct hs_syntax_node *)(syntax+1),*initializer;
    long count=scenario->hs_globals.count,i,slot;
    if (count<0 || count>=1024-hs_external_global_count) return NONE;
    old=mcc_runtime_pointer(runtime,(uint32_t)(uintptr_t)scenario->hs_globals.address,
        (uint32_t)count*sizeof(*old));
    if (count && !old) return NONE;
    for (i=0;i<count;i++) {
        if (memchr(old[i].name,0,sizeof(old[i].name)) && !csstrcasecmp(old[i].name,name))
            return old[i].type==type ? (short)i : NONE;
    }
    for (slot=0;slot<syntax->count;slot++) if (!nodes[slot].datum_header) break;
    if (slot==syntax->maximum_count) return NONE;
    globals=mcc_runtime_allocate(runtime,(uint32_t)(count+1)*sizeof(*globals));
    if (!globals) return NONE;
    if (count) memcpy(globals,old,count*sizeof(*globals));
    memset(globals+count,0,sizeof(*globals));
    strcpy(globals[count].name,name);
    globals[count].type=type;
    globals[count].initialization_expression_index=0x10000|slot;
    initializer=nodes+slot;
    memset(initializer,0,sizeof(*initializer));
    initializer->datum_header=1;initializer->flags=9;
    initializer->type=initializer->constant_type=type;
    if (!csstrcasecmp(name,"game_speed_value")) initializer->data=0x3f800000;
    initializer->next_node_index=NONE;initializer->source_offset=NONE;
    if (slot>=syntax->count) syntax->count=(short)(slot+1);
    syntax->actual_count++;
    syntax->first_free_absolute_index=(short)(slot+1);
    scenario->hs_globals.address=globals;scenario->hs_globals.count=count+1;
    return (short)count;
}

static boolean mcc_disabled_skull(char const *name)
{
    return name && (!csstrcasecmp(name,"debug_ice_cream_flavor_status_i_would_have_been_your_daddy") ||
        !csstrcasecmp(name,"debug_ice_cream_flavor_status_grunt_birthday_party") ||
        !csstrcasecmp(name,"debug_ice_cream_flavor_status_bandanna") ||
        !csstrcasecmp(name,"debug_ice_cream_flavor_status_sputnik") ||
        !csstrcasecmp(name,"debug_ice_cream_flavor_status_catch") ||
        !csstrcasecmp(name,"debug_ice_cream_flavor_status_boom"));
}

static boolean mcc_boolean_result(struct hs_syntax_node *node,boolean value)
{
    union { real number;long bits; } result;
    switch (node->type) {
    case _hs_type_void: result.bits=0;break;
    case _hs_type_boolean: case _hs_type_short_integer: case _hs_type_long_integer:
        result.bits=value!=FALSE;break;
    case _hs_type_real: result.number=value ? 1.0f : 0.0f;break;
    default: return FALSE;
    }
    /* The native cache recompiler requires primitive constants to have
     * matching stored/desired types, including discarded (void) results. */
    node->flags=9;node->constant_type=node->type;node->data=result.bits;node->source_offset=NONE;
    return TRUE;
}

int mcc_scripts_prepare(struct mcc_runtime *runtime)
{
    uint32_t address;
    struct scenario *scenario;
    struct data_array *syntax;
    struct hs_syntax_node *nodes;
    char const *strings;
    long i, missing = 0;
    short developer=NONE, score=NONE, speed=NONE, scratch[6]={NONE,NONE,NONE,NONE,NONE,NONE};
    long scratch_strings=NONE,bad_node;
    char const *syntax_reason;
    static char const scratch_names[6][17]={"__mcc_scratch_f0","__mcc_scratch_f1",
        "__mcc_scratch_f2","__mcc_scratch_f3","__mcc_scratch_f4","__mcc_scratch_f5"};
    mcc_diagnostic_tag(runtime,runtime->report.scenario_handle);
    mcc_diagnostic_at(runtime,"scripts");
    memcpy(&address, runtime->tag_index + (runtime->report.scenario_handle & 0xFFFFu) * 32 + 20, 4);
    scenario = mcc_runtime_pointer(runtime, address, sizeof(*scenario));
    if (!scenario) return mcc_diagnostic_fail(runtime,"scenario header is outside loaded data");
    if (!scenario->hs_syntax_data.size) return runtime->report.parameterized_script_count == 0 ? TRUE :
        mcc_diagnostic_fail(runtime,"parameterized scripts have no syntax data");
    syntax = mcc_runtime_pointer(runtime, (uint32_t)(uintptr_t)scenario->hs_syntax_data.address,
        (uint32_t)scenario->hs_syntax_data.size);
    strings = mcc_runtime_pointer(runtime, (uint32_t)(uintptr_t)scenario->hs_string_constants.address,
        (uint32_t)scenario->hs_string_constants.size);
    if (!syntax || !strings) return mcc_diagnostic_fail(runtime,"script syntax or string data is outside loaded memory");
    if (!mcc_syntax_graph_check(syntax,scenario->hs_syntax_data.size,&syntax_reason,&bad_node)) {
        if (bad_node!=NONE) {
            struct hs_syntax_node const *bad=(struct hs_syntax_node const *)(syntax+1)+bad_node;
            mcc_diagnostic_at(runtime,"syntax_nodes[%ld] (source offset %ld, string offset %ld)",bad_node,(long)bad->source_offset,(long)bad->string_offset);
        }
        return mcc_diagnostic_fail(runtime,"%s (capacity %d nodes)",syntax_reason,MCC_SYNTAX_CAPACITY);
    }
    nodes = (struct hs_syntax_node *)(syntax + 1);
    syntax->data = nodes;
    for (i = 0; i < syntax->count; ++i) {
        struct hs_syntax_node *node = &nodes[i];
        char const *name = NULL;
        short index = NONE;
        if (!node->datum_header) continue;
        mcc_diagnostic_at(runtime,"syntax_nodes[%ld] (source offset %ld, string offset %ld)",i,(long)node->source_offset,(long)node->string_offset);
        if (!(node->flags & 3)) {
            uint32_t child = (uint32_t)node->data & 0xFFFFu;
            if (child >= (uint32_t)syntax->count || !nodes[child].datum_header ||
                (uint16_t)nodes[child].datum_header != (uint16_t)((uint32_t)node->data >> 16) ||
                nodes[child].type != _hs_function_name)
                return mcc_diagnostic_fail(runtime,"function call references an invalid function-name node (handle %08lx)",(unsigned long)node->data);
            name = mcc_script_string(strings, scenario->hs_string_constants.size, nodes[child].string_offset);
            if (!name) return mcc_diagnostic_fail(runtime,"script name is outside the string table or has no terminator");
            /* These status queries describe disabled, unimplemented skulls.
             * They are read-only constants, not writable legacy globals. */
            if (!csstrcasecmp(name,"set")) {
                struct hs_syntax_node *target = datum_try_and_get(syntax,nodes[child].next_node_index);
                char const *target_name=target ? mcc_script_string(strings,scenario->hs_string_constants.size,target->string_offset) : NULL;
                if (target && (target->flags&5)==5 && target_name && (!csstrcasecmp(target_name,"object_prediction") ||
                    !csstrcasecmp(target_name,"breadcrumbs_navpoints_enabled_override"))) {
                    struct hs_syntax_node *value=datum_try_and_get(syntax,target->next_node_index);
                    /* Prediction and MCC breadcrumb availability are always
                     * enabled. A literal enable agrees with those policies;
                     * never discard a disable request or computed value. */
                    if (value && (value->flags&7)==1 && value->type==_hs_type_boolean &&
                        value->constant_type==_hs_type_boolean && value->boolean_value && value->next_node_index==NONE) {
                        if (!mcc_boolean_result(node,TRUE)) return mcc_diagnostic_fail(runtime,"literal enable cannot be converted to requested result type %d",node->type);
                        continue;
                    }
                    return mcc_diagnostic_fail(runtime,"%s supports only a literal enable; computed values and disabling are unsupported",target_name);
                }
                if (target && (target->flags & 1) && mcc_disabled_skull(
                    mcc_script_string(strings,scenario->hs_string_constants.size,target->string_offset))) {
                    struct hs_syntax_node *value=datum_try_and_get(syntax,target->next_node_index);
                    /* A literal disable is already satisfied. Retain the
                     * expression's requested result type and sibling link;
                     * never discard computed values or an enable request. */
                    if (value && (value->flags&7)==1 && value->type==_hs_type_boolean &&
                        value->constant_type==_hs_type_boolean && !value->boolean_value &&
                        value->next_node_index==NONE) {
                        if (!mcc_boolean_result(node,FALSE)) return mcc_diagnostic_fail(runtime,"literal disable cannot be converted to requested result type %d",node->type);
                        continue;
                    }
                    return mcc_diagnostic_fail(runtime,"unsupported skull assignment to %s; only a literal disable is supported",target_name);
                }
            }
            index = mcc_script_resolve(name, TRUE);
            if (index != NONE) node->function_index = nodes[child].function_index = index;
        } else if ((node->flags & 5) == 5 && ((uint16_t)node->data & 0x8000)) {
            name = mcc_script_string(strings, scenario->hs_string_constants.size, node->string_offset);
            if (!name) return mcc_diagnostic_fail(runtime,"script name is outside the string table or has no terminator");
            if (!csstrcasecmp(name,"object_prediction") || !csstrcasecmp(name,"breadcrumbs_navpoints_enabled_override")) {
                if (node->type!=_hs_type_boolean) return mcc_diagnostic_fail(runtime,"global %s requires boolean type; received type %d",name,node->type);
                node->flags&=~4;node->constant_type=_hs_type_boolean;node->data=TRUE;continue;
            }
            if (mcc_disabled_skull(name)) {
                if (node->type != _hs_type_boolean) return mcc_diagnostic_fail(runtime,"skull global %s requires boolean type; received type %d",name,node->type);
                node->flags &= ~4;
                node->constant_type = _hs_type_boolean;
                node->data = FALSE;
                continue;
            }
            if (!csstrcasecmp(name,"game_speed_value")) {
                if (speed==NONE) speed=mcc_local_global(runtime,scenario,syntax,name,_hs_type_real);
                if (speed==NONE) {
                    return mcc_diagnostic_fail(runtime,"no scenario storage for global game_speed_value (global/node capacity or allocation exhausted)");
                }
                node->data=speed;
                continue;
            }
            /* Debug score is a map-local long, not a multiplayer score or
             * authority control. Preserve its script reads and writes. */
            if (!csstrcasecmp(name,"debug_score")) {
                if (score==NONE) score=mcc_local_global(runtime,scenario,syntax,name,_hs_type_long_integer);
                if (score==NONE) {
                    return mcc_diagnostic_fail(runtime,"no scenario storage for global debug_score (global/node capacity or allocation exhausted)");
                }
                node->data=score;
                continue;
            }
            if (!csstrcasecmp(name,"developer_mode")) {
                if (developer==NONE) developer=mcc_local_global(runtime,scenario,syntax,"developer_mode",_hs_type_short_integer);
                if (developer==NONE) {
                    return mcc_diagnostic_fail(runtime,"no scenario storage for global developer_mode (global/node capacity or allocation exhausted)");
                }
                node->data=developer;
                continue;
            }
            /* Maps use these debug floats as script counters. Give them
             * ordinary scenario storage, not the native renderer's controls.
             * Rename tokens too: the native recompiler resolves them again. */
            if ((name[0]=='f' || name[0]=='F') && name[1]>='0' && name[1]<='5' && !name[2]) {
                unsigned slot=(unsigned)(name[1]-'0');
                if (scratch_strings==NONE) {
                    char *expanded;
                    if (scenario->hs_string_constants.size<1024) return mcc_diagnostic_fail(runtime,"script strings lack the required 1024-byte console source reserve");
                    expanded=mcc_runtime_allocate(runtime,
                        (uint32_t)scenario->hs_string_constants.size+sizeof(scratch_names));
                    if (!expanded) return mcc_diagnostic_fail(runtime,"not enough tag memory to expand scratch-global names");
                    /* The final 1 KiB belongs to console source, not map strings. */
                    scratch_strings=scenario->hs_string_constants.size-1024;
                    memcpy(expanded,strings,(unsigned long)scratch_strings);
                    memcpy(expanded+scratch_strings,scratch_names,sizeof(scratch_names));
                    memset(expanded+scratch_strings+sizeof(scratch_names),0,1024);
                    strings=expanded;scenario->hs_string_constants.address=expanded;
                    scenario->hs_string_constants.size+=sizeof(scratch_names);
                }
                if (scratch[slot]==NONE) scratch[slot]=mcc_local_global(runtime,scenario,syntax,scratch_names[slot],_hs_type_real);
                if (scratch[slot]==NONE) return mcc_diagnostic_fail(runtime,"no scenario storage for scratch global %s",name);
                node->data=scratch[slot];node->string_offset=scratch_strings+slot*sizeof(scratch_names[0]);
                continue;
            }
            index = hs_find_global_by_name(name);
            if (index != NONE && ((uint16_t)index & 0x8000)) node->short_value = index;
            else index = NONE;
        }
        if (name && index == NONE) {
            if (!runtime->failure[0]) snprintf(runtime->failure,sizeof(runtime->failure),
                "Unsupported script name: %.24s. See debug.txt.",name);
            if (missing < 16) mcc_diagnostic_fail(runtime,"unsupported script function or global '%s'",name);
            ++missing;
        }
    }
    /* Retain the MCC arena, including free slots for console expressions.
     * Its separate validator owns the larger traversal workspace. */
    syntax->data = nodes;
    return missing == 0 && mcc_parameters_prepare(runtime, scenario, syntax);
}
