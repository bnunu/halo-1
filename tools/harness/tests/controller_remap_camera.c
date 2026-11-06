/* The director's real input routing, with camera movement and devices stubbed. */
#include "harness.h"
#include "actions.inc"

#define MAXIMUM_NUMBER_OF_LOCAL_PLAYERS 4
#define TICKS_PER_SECOND 30
#define csmemset memset
#define SET_FLAG(flags, bit, value) ((flags) = ((flags) & ~FLAG(bit)) | ((value) ? FLAG(bit) : 0))
enum { _gamepad_analog_button_black, _gamepad_binary_button_right_thumb,
    _gamepad_analog_button_right_trigger, _gamepad_analog_button_left_trigger,
    _gamepad_binary_button_dpad_up, _gamepad_binary_button_dpad_down };
enum { _camera_control_up_bit, _camera_control_down_bit, _camera_control_forward_bit,
    _camera_control_reverse_bit, _camera_control_left_bit, _camera_control_right_bit,
    _camera_control_roll_left_bit, _camera_control_roll_right_bit };
enum { _variable_height, _variable_roll, _variable_forward, _variable_right };
enum { _key_backspace, _key_tab, _key_w, _key_s, _key_a, _key_d, _key_r, _key_f, _key_t, _key_g };
typedef struct { real i, j; } real_vector2d;
typedef struct { real yaw, pitch; } real_euler_angles2d;
typedef void (*director_camera_update_proc)(void);
struct camera_control {
    short local_player_index;
    real seconds_elapsed, wheel_delta;
    struct { real yaw, pitch, roll; } facing_delta;
    real_vector3d position_delta;
    boolean active;
};
struct director {
    director_camera_update_proc camera_proc;
    boolean debug_controls;
    real debug_input_scale;
    struct { real delta; } debug_variables[4];
};
struct gamepad_state { byte buttons[6]; };
struct mouse_state { byte buttons[3]; int wheel_delta, x, y; };
static void flying_camera_update(void) {}
static void editor_camera_update(void) {}
static void orbiting_camera_update(void) {}
static void first_person_camera_update(void) {}
static void following_camera_update(void) {}
static struct director camera;
static struct gamepad_state pad;
static struct { short local_player_index; } player = {2};
static struct { real dtime; } director_globals = {0.5f};
static boolean director_camera_switch_fast = TRUE;
static real_vector2d player_control_port_debug_throttle[4] = {{0.25f, -0.5f}};
static real_euler_angles2d player_control_port_debug_facing[4] = {{0.125f, -0.25f}};
static int custom, playing = TRUE, middle, key_up, key_down, inhibited, processed;
static unsigned int held, pressed;
static unsigned long flags_seen;
static float wheel;
static struct director *director_get(short local) { return &camera; }
static long local_player_get_player_index(short local) { return local; }
#define player_get(index) (&player)
static boolean input_has_gamepad(short port) { CHECK(port == 2, "controller port"); return TRUE; }
static struct gamepad_state const *input_get_gamepad_state(short port) { return &pad; }
static struct mouse_state const *input_get_mouse_state(void) { return NULL; }
static boolean input_key_is_down(int key) { return FALSE; }
static boolean passenger_camera_enabled(short local) { return FALSE; }
static int controller_remap_camera(short local, int available, int controlling,
    unsigned int *out_held, unsigned int *out_pressed)
{
    *out_held = held; *out_pressed = pressed;
    return custom;
}
static int controller_remap_camera_cycle(short port, int fast)
{
    CHECK(port == 2 && fast, "camera cycle controller port and mode");
    return (pressed & FLAG(PAD_CAMERA_CYCLE)) != 0;
}
static int controller_remap_playing(short port) { CHECK(port == 2, "remap controller port"); return playing; }
static void halo_linux_debug_camera_input(int *out_middle, int *up, int *down, float *out_wheel)
{
    *out_middle = middle; *up = key_up; *down = key_down; *out_wheel = wheel;
}
static void director_process_variables(short local, unsigned long flags, real speed)
{
    flags_seen = flags; processed++;
    camera.debug_variables[_variable_height].delta =
        TEST_FLAG(flags, _camera_control_up_bit) - TEST_FLAG(flags, _camera_control_down_bit);
}
static void director_inhibit_input(short local) { inhibited++; }
static void director_inhibit_facing(short local) { inhibited++; }

#include "under_test.inc"

static void check_motion(struct camera_control const *controls)
{
    CHECK(controls->active && inhibited == 2 && processed == 1, "camera input was not applied once");
    CHECK(controls->facing_delta.yaw == 0.125f && controls->facing_delta.pitch == -0.25f,
        "processed mouse/stick facing lost");
    CHECK(controls->position_delta.i == 0.25f * 1.638f && controls->position_delta.j == -0.5f * 1.638f,
        "processed keyboard/stick movement lost");
}

int main(int argc, char **argv)
{
    char const *case_name = argv[1];
    struct camera_control controls;
    CHECK(argc == 2, "case");
    camera.camera_proc = flying_camera_update;
    camera.debug_controls = TRUE;
    camera.debug_input_scale = 2.0f;
    CASE("stock")
    {
        pad.buttons[_gamepad_analog_button_right_trigger] = 255;
        pad.buttons[_gamepad_binary_button_dpad_up] = 2;
        CHECK(!director_update_controls(0, &controls), "unexpected camera cycle");
        check_motion(&controls);
        CHECK(flags_seen == FLAG(_camera_control_up_bit) && controls.wheel_delta == 0.4f, "stock up/speed");
        pad.buttons[_gamepad_analog_button_right_trigger] = 0;
        pad.buttons[_gamepad_analog_button_left_trigger] = 255;
        pad.buttons[_gamepad_binary_button_dpad_up] = 0;
        pad.buttons[_gamepad_binary_button_dpad_down] = 2;
        director_update_controls(0, &controls);
        CHECK(flags_seen == FLAG(_camera_control_down_bit) && controls.wheel_delta == -0.4f, "stock down/speed");
    }
    else CASE("custom")
    {
        custom = TRUE;
        held = FLAG(PAD_CAMERA_UP) | FLAG(PAD_CAMERA_FASTER);
        pad.buttons[_gamepad_analog_button_black] = 1;
        pad.buttons[_gamepad_analog_button_left_trigger] = 255;
        CHECK(!director_update_controls(0, &controls), "custom input cycled the camera");
        check_motion(&controls);
        CHECK(flags_seen == FLAG(_camera_control_up_bit) && controls.wheel_delta == 0.4f, "custom up/speed");
        held = FLAG(PAD_CAMERA_DOWN) | FLAG(PAD_CAMERA_SLOWER);
        director_update_controls(0, &controls);
        CHECK(flags_seen == FLAG(_camera_control_down_bit) && controls.wheel_delta == -0.4f, "custom down/speed");
        pressed = FLAG(PAD_CAMERA_SWITCH);
        director_update_controls(0, &controls);
        CHECK(!camera.debug_controls && !controls.active, "custom toggle did not release camera");
    }
    else CASE("keyboard")
    {
        custom = TRUE; key_up = TRUE;
        director_update_controls(0, &controls);
        check_motion(&controls);
        CHECK(flags_seen == FLAG(_camera_control_up_bit), "R lost with custom controller");
        key_up = FALSE; key_down = TRUE;
        director_update_controls(0, &controls);
        CHECK(flags_seen == FLAG(_camera_control_down_bit), "F lost with custom controller");
    }
    else CASE("middle")
    {
        custom = TRUE; middle = TRUE; key_down = TRUE; wheel = 2.0f;
        held = FLAG(PAD_CAMERA_UP) | FLAG(PAD_CAMERA_FASTER);
        director_update_controls(0, &controls);
        check_motion(&controls);
        CHECK(flags_seen == FLAG(_camera_control_down_bit) && controls.wheel_delta == -2.0f,
            "middle mouse did not take priority");
    }
    else if (!strcmp(case_name, "blocked") || !strcmp(case_name, "blocked_middle"))
    {
        custom = TRUE; playing = FALSE; middle = !strcmp(case_name, "blocked_middle");
        key_up = TRUE; wheel = 2.0f; held = FLAG(PAD_CAMERA_UP);
        director_update_controls(0, &controls);
        CHECK(!controls.active && !processed && !inhibited, "menu/chat input moved the custom camera");
    }
    else CASE("normal_camera")
    {
        custom = TRUE; middle = TRUE; key_up = TRUE;
        held = FLAG(PAD_CAMERA_UP); pressed = FLAG(PAD_CAMERA_SWITCH);
        camera.camera_proc = first_person_camera_update;
        director_update_controls(0, &controls);
        CHECK(!controls.active && !processed, "first-person camera accepted debug movement");
        camera.camera_proc = following_camera_update;
        director_update_controls(0, &controls);
        CHECK(!controls.active && !processed, "following camera accepted debug movement");
    }
    else CHECK(FALSE, "unknown case %s", case_name);
    return 0;
}
