/* The real runtime origin checks and map_name evaluator, with the two map
 * change destinations recorded instead of loading game assets. */
#include "harness.h"
#include "types.inc"

struct hs_script { short script_type; };
struct hs_thread_datum { short type, script_index; };
static struct { boolean initialized; long executing_thread_index; } hs_runtime_globals;
static struct hs_thread_datum thread;
static struct hs_script script;
static struct { struct { struct hs_script *address; } hs_scripts; } scenario;
static struct hs_thread_datum *hs_thread_get(long index) { (void)index; return &thread; }
#define global_scenario_get() (&scenario)
#define TAG_BLOCK_GET_ELEMENT(block, index, type) ((type *)(block)->address + (index))

struct hs_arguments_string { char const *value; };
static struct hs_arguments_string arguments = { "levels\\b30\\b30" };
static boolean arguments_ready = TRUE;
static int unrestricted, restricted, refused, returned;
static void *hs_macro_function_evaluate(short function_index, long thread_index, boolean initialize)
{
    (void)function_index; (void)thread_index; (void)initialize;
    return arguments_ready ? &arguments : NULL;
}
static void main_set_map_name(char const *name) { (void)name; unrestricted++; }
static void main_script_map_name(char const *name) { (void)name; restricted++; }
static void hs_return(long index, long value) { (void)index; (void)value; returned++; }
#define _error_silent 0
static void error(int severity, char const *message) { (void)severity; (void)message; refused++; }
#include "under_test.inc"

int main(int argc, char **argv)
{
    char const *name;
    int expected_restricted = 0, expected_unrestricted = 0, expected_refused = 0;
    CHECK(argc == 2, "case required");
    name = argv[1];
    hs_runtime_globals.initialized = TRUE;
    hs_runtime_globals.executing_thread_index = 0;
    scenario.hs_scripts.address = &script;
    script.script_type = _hs_script_startup;
    thread.type = _hs_thread_type_script;
    if (!strcmp(name, "startup")) expected_restricted = 1;
    else if (!strcmp(name, "dormant")) { script.script_type = _hs_script_dormant; expected_restricted = 1; }
    else if (!strcmp(name, "continuous")) { script.script_type = _hs_script_continuous; expected_restricted = 1; }
    else if (!strcmp(name, "global") || !strcmp(name, "global-static")) {
        /* A static call retains its owning global-initializer thread. */
        thread.type = _hs_thread_type_global_initialize;
        thread.script_index = NONE;
        script.script_type = _hs_script_static;
        expected_refused = 1;
    } else if (!strcmp(name, "console") || !strcmp(name, "console-static")) {
        thread.type = _hs_thread_type_console_command;
        thread.script_index = NONE;
        script.script_type = _hs_script_static;
        expected_unrestricted = 1;
    } else if (!strcmp(name, "no-thread")) {
        hs_runtime_globals.executing_thread_index = NONE; expected_refused = 1;
    } else if (!strcmp(name, "uninitialized")) {
        hs_runtime_globals.initialized = FALSE; expected_refused = 1;
    } else if (!strcmp(name, "arguments-pending")) arguments_ready = FALSE;
    else CHECK(FALSE, "unknown case");
    main_set_map_name_evaluate(0, 0, TRUE);
    CHECK(unrestricted == expected_unrestricted, "only console commands may load arbitrary maps");
    CHECK(restricted == expected_restricted, "map scripts use the campaign resolver");
    CHECK(refused == expected_refused, "initializers and absent execution contexts are refused");
    CHECK(returned == arguments_ready, "completed calls return, pending arguments do not");
    return 0;
}
