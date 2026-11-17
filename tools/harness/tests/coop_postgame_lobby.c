#include "harness.h"

struct network_game
{
    struct { short game_engine_index; } variant;
    struct { char name[64]; } map;
};
static struct network_game game;
static boolean host, campaign, splitscreen, quickstart, restart, missing_game;
static boolean paused;
static int closed, loaded;
static char const *screen;

static struct network_game *network_game_get_game(void) { return missing_game ? NULL : &game; }
static void *global_network_game_server_get(void) { return host ? &game : NULL; }
static boolean custom_edition_maps_level_campaign(char const *name) { return campaign; }
static boolean network_game_is_splitscreen_local(void) { return splitscreen; }
static boolean network_game_is_quickstart_local(void) { return quickstart; }
static boolean mcc_ui_network_restart_pregame(void) { return restart; }
static char const *pc_menus_screen(char const *name) { return name; }
static void ui_widgets_close_all(void) { closed++; }
static void network_game_server_pause_countdown(void *server, boolean pause) { paused = pause; }
static boolean load_screen(char const *name) { screen = name; loaded++; return TRUE; }
#define ui_widget_load_by_name_or_tag(name, ...) load_screen(name)
#define error(...) CHECK(FALSE, "screen load failed")

#include "under_test.inc"

int main(int argc, char **argv)
{
    char const *case_name = argc == 2 ? argv[1] : "";
    char const *expected = "connected_pregame_screen";
    boolean expect_pause = FALSE;

    host = TRUE;
    campaign = TRUE;
    CASE("coop") {}
    else CASE("multiplayer")
    {
        game.variant.game_engine_index = 1;
        expected = "connected_map_select_postgame_wrapper";
        expect_pause = TRUE;
    }
    else CASE("not-campaign")
    {
        campaign = FALSE;
        expected = "connected_map_select_postgame_wrapper";
        expect_pause = TRUE;
    }
    else CASE("client") { host = FALSE; }
    else CASE("missing-game")
    {
        missing_game = TRUE;
        expected = "connected_map_select_postgame_wrapper";
        expect_pause = TRUE;
    }
    else CASE("splitscreen")
    {
        splitscreen = TRUE;
        expected = "splitscreen_map_select_postgame_wrapper";
    }
    else CASE("quickstart")
    {
        splitscreen = quickstart = TRUE;
        expected = "splitscreen_pregame_wrapper_normal";
    }
    else CASE("mcc-restart") { restart = TRUE; campaign = FALSE; }
    else CASE("mcc-split-restart")
    {
        restart = splitscreen = TRUE;
        campaign = FALSE;
        expected = "splitscreen_pregame_wrapper_normal";
    }
    else return 2;
    network_game_reset_to_pregame_ui();
    CHECK(closed == 1 && loaded == 1, "postgame screen was not replaced once");
    CHECK(screen && strstr(screen, expected), "wrong postgame screen: %s", screen);
    CHECK(paused == expect_pause, "countdown pause %d", paused);
    return 0;
}
