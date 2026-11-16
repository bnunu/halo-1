/* MCC wire parameters: script+0x50 block, 36-byte name/type records;
 * syntax primitive/global/local flags 0x15 and zero-based slot in data.
 * Public schema: Invader scenario.json and scenario/pre_compile.cpp.
 * Only signatures/scope are retained here; live values belong to HS stacks. */
#include "cseries.h"
#include "errors.h"
#include "math/real_math.h"
#include "hs/hs.h"
#include "hs/hs_scenario_definitions.h"
#include "memory/data.h"
#include "scenario/scenario_definitions.h"
#include "mcc_cache.h"
#include "mcc_runtime.h"
#include "mcc_script_parameters.h"
#include "mcc_syntax.h"
#include <stdint.h>
#include <string.h>

#define MCC_PARAMETER_SCRIPTS 512
#define MCC_PARAMETER_NODES MCC_SYNTAX_CAPACITY
extern struct data_array *hs_syntax_data;
extern short const hs_external_global_count;
static struct mcc_script_signature mcc_signatures[MCC_PARAMETER_SCRIPTS];
static short mcc_parameter_owners[MCC_PARAMETER_NODES];
static short mcc_parameter_types[MCC_PARAMETER_NODES];
static unsigned short mcc_parameter_seen[MCC_PARAMETER_NODES];
static long mcc_parameter_walk[MCC_PARAMETER_NODES];
static unsigned mcc_signature_count, mcc_parameter_node_count;

/* Preparation precedes active MCC state. Account for MCC's AI conversions
 * and signed short-to-long widening without changing the native cast table. */
static int mcc_parameter_cast(short actual, short desired)
{
    return hs_can_cast(actual, desired) ||
        (actual == _hs_type_short_integer && desired == _hs_type_long_integer) || (actual == _hs_type_ai &&
        (HS_TYPE_IS_OBJECT(desired) || desired == _hs_type_object_list));
}

void mcc_parameters_dispose(void)
{
    mcc_signature_count = mcc_parameter_node_count = 0;
}

struct mcc_script_signature const *mcc_parameters_signature(short script)
{
    return mcc_cache_tags_loaded() && script >= 0 && (unsigned)script < mcc_signature_count ?
        mcc_signatures + script : NULL;
}

int mcc_parameter_type(struct hs_syntax_node const *node, short *owner)
{
    uintptr_t offset;
    unsigned index;
    if (!mcc_cache_tags_loaded() || !node || !(node->flags & 0x10)) return -1;
    if (!hs_syntax_data || !hs_syntax_data->data || (node->flags & 0x17) != 0x15) return -2;
    offset = (uintptr_t)node - (uintptr_t)hs_syntax_data->data;
    index = (unsigned)(offset / sizeof(*node));
    if (offset % sizeof(*node) || offset / sizeof(*node) >= mcc_parameter_node_count ||
        mcc_parameter_owners[index] < 0 || node->data < 0 || node->data >= 16) return -2;
    if (owner) *owner = mcc_parameter_owners[index];
    return mcc_parameter_types[index];
}

int mcc_parameter_compile(struct hs_syntax_node const *node)
{
    int type = mcc_parameter_type(node, NULL);
    if (type == -1) return -1;
    return hs_type_valid(type) && hs_type_valid(node->type) && mcc_parameter_cast((short)type, node->type);
}

int mcc_parameter_set_valid(struct hs_syntax_node const *node, struct hs_syntax_node const *value)
{
    int type = mcc_parameter_type(node, NULL);
    if (type == -1) return -1;
    return hs_type_valid(type) && value && value->type == type && node->type == type;
}

static struct hs_syntax_node *mcc_parameter_node(struct hs_syntax_node *nodes, unsigned count, long handle)
{
    unsigned index = (uint32_t)handle & 0xFFFFu;
    return handle != NONE && index < count && nodes[index].datum_header &&
        (uint16_t)nodes[index].datum_header == (uint16_t)((uint32_t)handle >> 16) ? nodes + index : NULL;
}

static int mcc_parameter_scope(struct mcc_runtime *runtime,struct hs_syntax_node *nodes, unsigned count, long root, short owner, unsigned stamp)
{
    unsigned depth = 0;
    struct hs_syntax_node *node;
    if (root == NONE) return 1;
    if (!(node = mcc_parameter_node(nodes, count, root))) return mcc_diagnostic_fail(runtime,"root expression handle %08lx is invalid",(unsigned long)root);
    mcc_parameter_walk[depth++] = root;
    mcc_parameter_seen[(uint32_t)root & 0xFFFFu] = (unsigned short)stamp;
    while (depth) {
        long handle = mcc_parameter_walk[--depth], links[2];
        unsigned index = (uint32_t)handle & 0xFFFFu, i;
        node = nodes + index;
        if (node->flags & 0x10) {
            if (owner < 0 || (node->flags & 0x17) != 0x15 || node->data < 0 ||
                node->data >= mcc_signatures[owner].count ||
                (mcc_parameter_owners[index] != NONE && mcc_parameter_owners[index] != owner)) return mcc_diagnostic_fail(runtime,"syntax_nodes[%u]: local parameter slot %ld has invalid flags, is outside the signature, or belongs to another script",index,node->data);
            mcc_parameter_owners[index] = owner;
            mcc_parameter_types[index] = mcc_signatures[owner].types[node->data];
            if (!hs_type_valid(node->type) ||
                !mcc_parameter_cast(mcc_parameter_types[index], node->type)) {
                return mcc_diagnostic_fail(runtime,"syntax_nodes[%u]: parameter type %d cannot cast to type %d",
                    index,mcc_parameter_types[index],node->type);
            }
        }
        links[0] = node->next_node_index;
        links[1] = node->flags & 1 ? NONE : node->data;
        for (i = 0; i < 2; ++i) {
            unsigned child = (uint32_t)links[i] & 0xFFFFu;
            if (links[i] == NONE) continue;
            if (!mcc_parameter_node(nodes, count, links[i])) return mcc_diagnostic_fail(runtime,"syntax_nodes[%u].%s has invalid handle %08lx",index,i ? "data" : "next_node_index",(unsigned long)links[i]);
            if (mcc_parameter_seen[child] == stamp) continue;
            if (depth >= count) return mcc_diagnostic_fail(runtime,"syntax_nodes[%u]: expression traversal exceeds the node count",index);
            mcc_parameter_seen[child] = (unsigned short)stamp;
            mcc_parameter_walk[depth++] = links[i];
        }
    }
    return 1;
}

int mcc_parameters_prepare(struct mcc_runtime *runtime, struct scenario *scenario, struct data_array *syntax)
{
    struct hs_script *scripts=NULL;
    struct hs_global *globals=NULL;
    struct hs_syntax_node *nodes;
    unsigned i = 0, j = 0, count, has_parameters = 0;
    char const *stage = "table bounds";
    char const *reason = "syntax, script or global table count exceeds its supported limit";
    int reported=0;
    mcc_parameters_dispose();
    mcc_diagnostic_tag(runtime,runtime->report.scenario_handle);
    mcc_diagnostic_at(runtime,"script parameters");
    if (!syntax || syntax->count < 0 || syntax->count > MCC_PARAMETER_NODES ||
        scenario->hs_scripts.count < 0 || scenario->hs_scripts.count > MCC_PARAMETER_SCRIPTS ||
        scenario->hs_globals.count < 0 || scenario->hs_globals.count > 1024 - hs_external_global_count) goto fail;
    count = (unsigned)syntax->count;
    nodes = (struct hs_syntax_node *)(syntax + 1);
    scripts = mcc_runtime_pointer(runtime, (uint32_t)(uintptr_t)scenario->hs_scripts.address,
        (uint32_t)scenario->hs_scripts.count * sizeof(*scripts));
    globals = mcc_runtime_pointer(runtime, (uint32_t)(uintptr_t)scenario->hs_globals.address,
        (uint32_t)scenario->hs_globals.count * sizeof(*globals));
    reason="scripts or globals table is outside loaded tag data";
    if ((scenario->hs_scripts.count && !scripts) || (scenario->hs_globals.count && !globals)) goto fail;
    memset(mcc_parameter_owners, 0xFF, sizeof(mcc_parameter_owners));
    memset(mcc_parameter_seen, 0, sizeof(mcc_parameter_seen));
    stage = "script signature";
    for (i = 0; i < (unsigned)scenario->hs_scripts.count; ++i) {
        uint32_t fields[3];
        unsigned char const *parameters;
        j=0;
        memcpy(fields, (unsigned char const *)(scripts + i) + 0x50, sizeof(fields));
        reason="only static/stub scripts may have parameters; maximum is 16 parameters";
        if (fields[0] > 16 || (fields[0] && scripts[i].script_type != _hs_script_static &&
            scripts[i].script_type != _hs_script_stub)) goto fail;
        parameters = mcc_runtime_pointer(runtime, fields[1], fields[0] * 36);
        reason="parameter records are outside loaded tag data";
        if (fields[0] && !parameters) goto fail;
        mcc_signatures[i].count = (short)fields[0];
        has_parameters |= fields[0];
        for (j = 0; j < fields[0]; ++j) {
            short type;
            memcpy(&type, parameters + j * 36 + 32, 2);
            reason="parameter name has no terminator within 32 bytes";
            if (!memchr(parameters + j * 36, 0, 32)) goto fail;
            reason="parameter type is invalid or void";
            if (!hs_type_valid(type) || type == _hs_type_void) goto fail;
            mcc_signatures[i].types[j] = type;
        }
    }
    for (i = 0; i < count; ++i) if (nodes[i].datum_header && (nodes[i].flags & 0x10)) has_parameters = 1;
    if (has_parameters) {
        stage = "script scope";
        for (i = 0; i < (unsigned)scenario->hs_scripts.count; ++i) {
            mcc_diagnostic_at(runtime,"scripts[%u] '%.32s'",i,scripts[i].name);
            if (!mcc_parameter_scope(runtime,nodes, count, scripts[i].root_expression_index, (short)i, i + 1)) {reported=1;goto fail;}
        }
        stage = "global scope";
        for (i = 0; i < (unsigned)scenario->hs_globals.count; ++i) {
            mcc_diagnostic_at(runtime,"globals[%u] '%.32s'",i,globals[i].name);
            if (!mcc_parameter_scope(runtime,nodes, count, globals[i].initialization_expression_index, NONE,
                i + MCC_PARAMETER_SCRIPTS + 1)) {reported=1;goto fail;}
        }
        stage = "call arguments";
        for (i = 0; i < count; ++i) {
            struct hs_syntax_node *call = nodes + i, *argument;
            struct mcc_script_signature const *signature;
            long next;
            if (!call->datum_header) continue;
            j=0;reason="local parameter node has no owning script";
            if ((call->flags & 0x10) && mcc_parameter_owners[i] == NONE) goto fail;
            if ((call->flags & 3) != 2) continue;
            reason="script call index or function-name node handle is invalid";
            if (call->script_index < 0 || call->script_index >= scenario->hs_scripts.count ||
                !(argument = mcc_parameter_node(nodes, count, call->data))) goto fail;
            reason="script call must name a static/stub script through a primitive function-name node";
            if (argument->type != _hs_function_name || !(argument->flags & 1) ||
                (scripts[call->script_index].script_type != _hs_script_static &&
                scripts[call->script_index].script_type != _hs_script_stub)) goto fail;
            signature = mcc_signatures + call->script_index;
            next = argument->next_node_index;
            for (j = 0; j < (unsigned)signature->count; ++j) {
                argument = mcc_parameter_node(nodes, count, next);
                reason="argument node is missing or has a different type from the script parameter";
                if (!argument || argument->type != signature->types[j]) goto fail;
                next = argument->next_node_index;
            }
            reason="script call has more arguments than its signature";
            if (next != NONE) goto fail;
        }
    }
    mcc_signature_count = (unsigned)scenario->hs_scripts.count;
    mcc_parameter_node_count = count;
    return 1;
fail:
    mcc_parameters_dispose();
    if (reported) return 0;
    mcc_diagnostic_at(runtime,"%s, entry[%u], argument[%u]",stage,i,j);
    if (!strcmp(stage,"call arguments")) mcc_diagnostic_at(runtime,"syntax_nodes[%u].arguments[%u]",i,j);
    if (scripts && i<(unsigned)scenario->hs_scripts.count && (!strcmp(stage,"script signature") || !strcmp(stage,"script scope")))
        mcc_diagnostic_at(runtime,"scripts[%u] '%.32s', %s, argument[%u]",i,scripts[i].name,stage,j);
    if (globals && i<(unsigned)scenario->hs_globals.count && !strcmp(stage,"global scope"))
        mcc_diagnostic_at(runtime,"globals[%u] '%.32s', argument[%u]",i,globals[i].name,j);
    return mcc_diagnostic_fail(runtime,"%s",reason);
}
