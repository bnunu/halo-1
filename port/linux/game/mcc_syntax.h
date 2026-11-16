#ifndef MCC_SYNTAX_H
#define MCC_SYNTAX_H

/* MCC Halo 1's cache syntax array uses signed 16-bit counts. Its final
 * addressable slot is 32766; this does not change the Xbox/CE 19001 limit. */
#define MCC_SYNTAX_CAPACITY 32767
/* Serialized 56-byte data-array header followed by 20-byte syntax nodes. */
#define MCC_SYNTAX_MAXIMUM_DATA_BYTES (56L + 20L * MCC_SYNTAX_CAPACITY)

struct data_array;
struct scenario;
struct hs_syntax_node;

/* Pre-publication validation: the caller owns and bounds the complete blob.
 * This also rejects cycles before the shared compiler follows sibling links. */
boolean mcc_syntax_graph_valid(struct data_array const *syntax, long bytes);
boolean mcc_syntax_graph_check(struct data_array const *syntax, long bytes,
    char const **reason, long *bad_node);

/* -1 for other formats; otherwise whether this active MCC scenario is sound. */
int mcc_syntax_scenario_valid(struct scenario const *scenario);

/* Return TRUE only when the active MCC scenario was handled. The callback
 * applies the engine's existing function allowlist and argument checks. All
 * traversal state belongs to MCC, including when a node is above slot 19000. */
boolean mcc_syntax_functions_check(struct scenario *scenario,
    struct data_array *syntax,
    short (*node_refusal)(struct hs_syntax_node const *, char const **),
    unsigned long *disabled_scripts, unsigned long *disabled_globals);

#endif
