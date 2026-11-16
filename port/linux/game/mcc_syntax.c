/* Independent MCC syntax validation and traversal storage. Runtime datums
 * remain in the map's tag allocation, so handles and checkpoint layout keep
 * their existing representation. The shared interpreter's signed indices
 * support this format's 32767 slots without widening any engine structure. */
#include "cseries.h"
#include "errors.h"
#include "math/real_math.h"
#include "hs/hs.h"
#include "hs/hs_scenario_definitions.h"
#include "memory/data.h"
#include "scenario/scenario_definitions.h"
#include "mcc_cache.h"
#include "mcc_syntax.h"
#include <stdint.h>
#include <string.h>

typedef char mcc_syntax_wire_size_matches_runtime[
    sizeof(struct data_array) + MCC_SYNTAX_CAPACITY * sizeof(struct hs_syntax_node) ==
        MCC_SYNTAX_MAXIMUM_DATA_BYTES ? 1 : -1];

static byte mcc_syntax_colors[MCC_SYNTAX_CAPACITY];
static byte mcc_syntax_edges[MCC_SYNTAX_CAPACITY];
static unsigned short mcc_syntax_stack[MCC_SYNTAX_CAPACITY];
static byte mcc_syntax_seen[MCC_SYNTAX_CAPACITY];

static struct hs_syntax_node const *mcc_syntax_node(
    struct data_array const *syntax, long handle)
{
    unsigned index = (uint32_t)handle & 0xFFFFu;
    struct hs_syntax_node const *nodes = (struct hs_syntax_node const *)(syntax + 1);
    if (handle == NONE || index >= (unsigned)syntax->count || !nodes[index].datum_header ||
        (uint16_t)nodes[index].datum_header != (uint16_t)((uint32_t)handle >> 16)) return NULL;
    return nodes + index;
}

boolean mcc_syntax_graph_check(struct data_array const *syntax, long bytes, char const **reason, long *bad_node)
{
    struct hs_syntax_node const *nodes;
    unsigned start, depth, live = 0;
    *bad_node=NONE;
    *reason="syntax header is missing, truncated or misaligned";
    if (!syntax || bytes < (long)sizeof(*syntax) || ((uintptr_t)syntax & 3u)) return FALSE;
    *reason="syntax header signature is not d@t@ or the valid flag is clear";
    if (syntax->signature != 'd@t@' || !syntax->valid) return FALSE;
    *reason="syntax node size differs from the required 20-byte layout";
    if (syntax->size != sizeof(*nodes)) return FALSE;
    *reason="syntax maximum_count is outside 1..32767";
    if (syntax->maximum_count <= 0 || syntax->maximum_count > MCC_SYNTAX_CAPACITY) return FALSE;
    *reason="syntax count is negative or exceeds maximum_count";
    if (syntax->count < 0 || syntax->count > syntax->maximum_count) return FALSE;
    *reason="syntax actual_count is negative or exceeds count";
    if (syntax->actual_count < 0 || syntax->actual_count > syntax->count) return FALSE;
    *reason="syntax first_free_absolute_index is outside 0..maximum_count";
    if (syntax->first_free_absolute_index < 0 || syntax->first_free_absolute_index > syntax->maximum_count) return FALSE;
    *reason="syntax byte size differs from header plus maximum_count nodes";
    if (bytes != (long)(sizeof(*syntax) + (unsigned)syntax->maximum_count * sizeof(*nodes))) return FALSE;
    nodes = (struct hs_syntax_node const *)(syntax + 1);
    memset(mcc_syntax_colors, 0, sizeof(mcc_syntax_colors));
    for (start = 0; start < (unsigned)syntax->count; ++start) {
        if (!nodes[start].datum_header) continue;
        ++live;
        if (mcc_syntax_colors[start]) continue;
        depth = 0;
        mcc_syntax_stack[depth] = (unsigned short)start;
        mcc_syntax_edges[depth++] = 0;
        mcc_syntax_colors[start] = 1;
        while (depth) {
            unsigned index = mcc_syntax_stack[depth - 1], edge = mcc_syntax_edges[depth - 1]++;
            long link;
            unsigned child;
            if (edge == 2) {
                mcc_syntax_colors[index] = 2;
                --depth;
                continue;
            }
            link = edge == 0 ? nodes[index].next_node_index :
                ((nodes[index].flags & 1) ? NONE : nodes[index].data);
            if (link == NONE) continue;
            *bad_node=(long)index;
            *reason=edge ? "expression child handle is stale or outside the syntax array" : "next_node_index is stale or outside the syntax array";
            if (!mcc_syntax_node(syntax, link)) return FALSE;
            child = (uint32_t)link & 0xFFFFu;
            if (mcc_syntax_colors[child] == 1) {*reason="syntax expression graph contains a cycle";return FALSE;}
            if (mcc_syntax_colors[child] == 2) continue;
            if (depth >= (unsigned)syntax->count) {*reason="syntax graph traversal exceeds its node count";return FALSE;}
            mcc_syntax_colors[child] = 1;
            mcc_syntax_stack[depth] = (unsigned short)child;
            mcc_syntax_edges[depth++] = 0;
        }
    }
    *bad_node=NONE;
    *reason="syntax actual_count differs from the number of live nodes";
    return live == (unsigned)syntax->actual_count;
}

boolean mcc_syntax_graph_valid(struct data_array const *syntax, long bytes)
{
    char const *reason;
    long bad_node;
    return mcc_syntax_graph_check(syntax,bytes,&reason,&bad_node);
}

int mcc_syntax_scenario_valid(struct scenario const *scenario)
{
    if (!mcc_cache_tags_loaded()) return -1;
    if (!scenario || scenario->hs_string_constants.size < 0x400 ||
        !mcc_cache_contains(scenario->hs_string_constants.address, scenario->hs_string_constants.size) ||
        !mcc_cache_contains(scenario->hs_syntax_data.address, scenario->hs_syntax_data.size)) return FALSE;
    return mcc_syntax_graph_valid(scenario->hs_syntax_data.address, scenario->hs_syntax_data.size);
}

static boolean mcc_syntax_expression_valid(struct data_array const *syntax, long root, short type,
    short (*node_refusal)(struct hs_syntax_node const *, char const **), char const **name)
{
    struct hs_syntax_node const *nodes = (struct hs_syntax_node const *)(syntax + 1), *node;
    unsigned depth = 0;
    *name = NULL;
    if (root == NONE) return TRUE;
    if (!(node = mcc_syntax_node(syntax, root)) || node->type != type) return FALSE;
    memset(mcc_syntax_seen, 0, sizeof(mcc_syntax_seen));
    mcc_syntax_seen[(uint32_t)root & 0xFFFFu] = 1;
    mcc_syntax_stack[depth++] = (unsigned short)root;
    while (depth) {
        long child;
        node = nodes + mcc_syntax_stack[--depth];
        if (node_refusal(node, name)) return FALSE;
        if (node->flags & 1) continue;
        child = node->data;
        while (child != NONE) {
            unsigned index = (uint32_t)child & 0xFFFFu;
            if (!(node = mcc_syntax_node(syntax, child)) || mcc_syntax_seen[index] ||
                depth >= (unsigned)syntax->count) {
                *name = NULL;
                return FALSE;
            }
            mcc_syntax_seen[index] = 1;
            mcc_syntax_stack[depth++] = (unsigned short)index;
            child = node->next_node_index;
        }
    }
    return TRUE;
}

boolean mcc_syntax_functions_check(struct scenario *scenario, struct data_array *syntax,
    short (*node_refusal)(struct hs_syntax_node const *, char const **),
    unsigned long *disabled_scripts, unsigned long *disabled_globals)
{
    long i, scripts = 0, globals = 0;
    char const *name, *first_name = NULL, *first_owner = NULL;
    if (!mcc_cache_tags_loaded()) return FALSE;
    for (i = 0; i < scenario->hs_scripts.count; ++i) {
        struct hs_script const *script = (struct hs_script const *)scenario->hs_scripts.address + i;
        if (!mcc_syntax_expression_valid(syntax, script->root_expression_index, script->return_type,
            node_refusal, &name)) {
            BIT_VECTOR_SET_FLAG(disabled_scripts, i, TRUE);
            if (scripts<16) error(_error_silent,"mcc: script %.32s rejected at %s",
                script->name,name ? name : "damaged expression");
            if (!first_owner) { first_owner = script->name; first_name = name; }
            ++scripts;
        }
    }
    for (i = 0; i < scenario->hs_globals.count; ++i) {
        struct hs_global const *global = (struct hs_global const *)scenario->hs_globals.address + i;
        if (!mcc_syntax_expression_valid(syntax, global->initialization_expression_index, global->type,
            node_refusal, &name)) {
            BIT_VECTOR_SET_FLAG(disabled_globals, i, TRUE);
            if (!first_owner) { first_owner = global->name; first_name = name; }
            ++globals;
        }
    }
    if (first_owner) error(_error_silent,
        "mcc: script validation refused %.32s (%s); %ld scripts won't run, %ld globals start at their defaults",
        first_owner, first_name ? first_name : "damaged expression", scripts, globals);
    return TRUE;
}
