/* Scripted PVS ownership while the real switch commits or recovers. */
#include "harness.h"

struct tag_block { long count; void *address; };
struct scenario_structure_bsp_reference { struct { long index; } structure_bsp; };
struct scenario_datum { struct tag_block structure_bsp_references; };
struct structure_bsp { struct tag_block collision_bsp; };
struct collision_bsp { int unused; };

#define TAG_BLOCK_GET_ELEMENT(block, index, type) (&((type *)(block)->address)[index])
#define MAXIMUM_STRUCTURE_BSPS_PER_SCENARIO 2
#define error(level, ...) ((void)0)
#define main_stop_time() ((void)0)
#define main_start_time() ((void)0)
#define collision_log_enable(enabled) ((void)0)
#define scenario_call_disconnect_from_structure_bsp_procs() ((void)0)
#define scenario_structure_bsp_unload(reference) ((void)0)
#define structure_bsp_port_verify(bsp) TRUE
#define mcc_cache_tags_loaded() FALSE
#define main_goto_main_menu() CHECK(FALSE, "original BSP should recover")
#define structure_culling_structure_bsp_loaded() ((void)0)
#define rasterizer_lights_reset_for_new_structure_bsp() ((void)0)

enum { _pvs_activation_normal, _pvs_activation_object, _pvs_activation_cluster };
static struct { short pvs_activation_type; } activation, *object_globals = &activation;
static struct scenario_structure_bsp_reference references[] = {{{0}}, {{1}}};
static struct scenario_datum scenario = {{2, references}}, *global_scenario = &scenario;
static struct { short structure_bsp_index; } globals, *scenario_globals = &globals;
static short global_structure_bsp_index;
static struct collision_bsp collision, *global_collision_bsp;
static struct structure_bsp bsp = {{1, &collision}}, *global_structure_bsp;
static struct bsp3d *global_bsp3d;
static int refused = NONE;
static int reconnect_activation = NONE;

static boolean scenario_structure_bsp_load(struct scenario_structure_bsp_reference *reference)
{
    return reference->structure_bsp.index != refused;
}

#define structure_bsp_definition_get(index) (&bsp)

static void scenario_call_reconnect_to_structure_bsp_procs(void)
{
    reconnect_activation = activation.pvs_activation_type;
}

#include "under_test.inc"

int main(int argc, char **argv)
{
    char const *case_name = argc > 1 ? argv[1] : "";
    boolean recovering = !strcmp(case_name, "recover");
    short expected = _pvs_activation_normal;

    global_structure_bsp_index = 0;
    activation.pvs_activation_type = _pvs_activation_cluster;
    if (recovering)
    {
        refused = 1;
        expected = _pvs_activation_cluster;
    }
    if (!strcmp(case_name, "object"))
        activation.pvs_activation_type = expected = _pvs_activation_object;
    CHECK(scenario_switch_structure_bsp(1) == !recovering, "switch result");
    CHECK(global_structure_bsp_index == (recovering ? 0 : 1), "installed BSP");
    CHECK(reconnect_activation == expected, "activation at reconnect: %d", reconnect_activation);
    CHECK(activation.pvs_activation_type == expected, "activation after reconnect");
    return 0;
}
