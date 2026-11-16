/* The shield's flare on the first-person arms (port/linux/game/viewmodel_shield.c) and the
overshield's arms copy (port/linux/game/overshield_effect.c) */
#include "harness.h"
#include <math.h>
#define NUMBEROF(a) (sizeof(a) / sizeof((a)[0]))
#define MIN(a, b) ((a) < (b) ? (a) : (b))
#define MAX(a, b) ((a) > (b) ? (a) : (b))
#define PIN(x, lo, hi) MIN(MAX((x), (lo)), (hi))
#define csmemcpy memcpy
#define MAXIMUM_LOCAL_PLAYERS 4
#define MAXIMUM_NODES_PER_ANIMATION 64
#define NUMBER_OF_OUTGOING_OBJECT_FUNCTIONS 4
#define TAG_BLOCK_GET_ELEMENT(block, index, type) (&((type *)(block)->address)[index])
typedef struct { real red, green, blue; } real_rgb_color;
typedef struct { real scale; real_vector3d forward, left, up; real_point3d position; } real_matrix4x3;
struct tag_block { long count; void *address; };
struct model_node { short parent_node_index; };
struct model { struct tag_block nodes; };
struct shader { struct { short type; } base; };
struct render_animation { real_rgb_color const *colors; real const *values; };
struct render_model_effect { struct shader *modifier_shader; struct render_animation modifier_animation; };
struct object_definition { struct { struct { long index; } modifier_shader; } object; };
struct unit_datum
{
	long definition_index;
	struct { real outgoing_function_values[4]; real_rgb_color outgoing_change_colors[4]; } object;
};

static struct { struct { real_point3d position; } camera; } render;
static struct { struct { real vertical_field_of_view; } camera; } global_window_parameters;
static struct { real first_person_weapon_near_clip_distance; } rasterizer_globals = {0.01f};
static int setting;
static int config_boolean(const char *name) { (void)name; return setting; }
static unsigned long config_changes(void) { static unsigned long changes; return ++changes; }

static struct unit_datum units[2];
static struct object_definition definition = {{{7}}};
static struct shader plasma = {{10}};
static int valid_modifier = 1;
static struct model hands;
static struct unit_datum *unit_get(long index) { return &units[index]; }
static struct object_definition *object_definition_get(long index) { (void)index; return &definition; }
static struct shader *shader_definition_get(long index) { (void)index; return &plasma; }
static boolean shader_type_is_valid_for_modifier(short type) { (void)type; return valid_modifier; }
static struct model *model_definition_get(long index) { (void)index; return &hands; }
static real distance3d(real_point3d const *a, real_point3d const *b)
{
	return sqrtf((a->x - b->x) * (a->x - b->x) + (a->y - b->y) * (a->y - b->y) + (a->z - b->z) * (a->z - b->z));
}

/* the overshield's state, as overshield_effect_get would leave it */
#include "overshield_types.inc"
static struct overshield_effect overshield_effects[OVERSHIELD_EFFECT_MAXIMUM];
static struct overshield_effect *overshield_unit[2];
static int overshield_setting = 1;
static boolean overshield_effect_enabled(void) { return overshield_setting; }
static struct overshield_effect *overshield_effect_get(long unit_index) { return overshield_unit[unit_index]; }
static short overshield_effect_intensity_source(long unit_index) { (void)unit_index; return 1; }
static int color_setting;
static real_rgb_color chosen_colors[4];
static real_rgb_color const *shield_color_colors(long index, real_rgb_color const *colors) { return color_setting ? chosen_colors : colors; }
#include "under_test.inc"

static struct model_node hand_nodes[3] = {{NONE}, {0}, {1}};
static real_matrix4x3 matrices[3];

static void arms_at(real distance)
{
	hands.nodes.count = 3;
	hands.nodes.address = hand_nodes;
	matrices[0].position.x = 0; /* (the root, at the eye) */
	matrices[1].position.x = distance;
	matrices[2].position.x = distance;
}

static struct render_model_effect drawn(short local, long unit)
{
	struct render_model_effect effect = {0};
	viewmodel_shield_apply(local, unit, 3, matrices, units[unit].object.outgoing_function_values, &effect);
	return effect;
}

int main(int argc, char **argv)
{
	char const *case_name = argc > 1 ? argv[1] : "";
	real const half_tan_60 = 0.57735027f; /* tan(30 degrees) */
	if (!strcmp(case_name, "color"))
	{
		struct render_model_effect effect;
		setting = color_setting = 1; arms_at(.5f); effect = drawn(0, 0);
		CHECK(effect.modifier_animation.colors == chosen_colors, "selected shield color reaches arms");
		color_setting = 0; effect = drawn(0, 0);
		CHECK(effect.modifier_animation.colors == units[0].object.outgoing_change_colors, "authored color remains default");
		return 0;
	}
	units[0].object.outgoing_function_values[1] = 0.5f;
	global_window_parameters.camera.vertical_field_of_view = 1.0471976f; /* 60 degrees */
	arms_at(0.2f);
	CASE("off") {
		struct render_model_effect effect;
		setting = 0;
		effect = drawn(0, 0);
		CHECK(!effect.modifier_shader && !effect.modifier_animation.values, "off, the hands' effect is untouched");
		CHECK(viewmodel_shield_offset_amount(units[0].object.outgoing_function_values, 0.5f) == 0.5f,
			"anything's offset is its own");
		CHECK(viewmodel_shield_offset_amount(NULL, 0.25f) == 0.25f, "no values, its own");
	}
	CASE("on") {
		struct render_model_effect effect;
		setting = 1;
		effect = drawn(0, 0);
		CHECK(effect.modifier_shader == &plasma, "the unit's modifier shader on the hands");
		CHECK(effect.modifier_animation.colors == units[0].object.outgoing_change_colors, "the unit's colors");
		CHECK(effect.modifier_animation.values != units[0].object.outgoing_function_values, "a copy of its values");
		CHECK(effect.modifier_animation.values[1] == 0.5f, "the unit's values, copied");
		valid_modifier = 0;
		effect = drawn(1, 0);
		CHECK(!effect.modifier_shader, "a shader that can't modify is not used");
		valid_modifier = 1;
		definition.object.modifier_shader.index = NONE;
		effect = drawn(1, 0);
		CHECK(!effect.modifier_shader, "a unit without a modifier shader has none on its hands");
		definition.object.modifier_shader.index = 7;
		effect = drawn(-1, 0);
		CHECK(!effect.modifier_shader, "no local player");
		effect = drawn(MAXIMUM_LOCAL_PLAYERS, 0);
		CHECK(!effect.modifier_shader, "a local player out of range");
	}
	CASE("others") {
		real body[4] = {0};
		setting = 1;
		(void)drawn(0, 0);
		CHECK(viewmodel_shield_offset_amount(units[0].object.outgoing_function_values, 0.5f) == 0.5f,
			"the body's flare keeps its offset");
		CHECK(viewmodel_shield_offset_amount(body, 0.5f) == 0.5f, "a weapon's plasma keeps its offset");
	}
	CASE("limit") {
		struct render_model_effect effect;
		real limit;
		setting = 1;
		effect = drawn(0, 0);
		limit = VIEWMODEL_SHIELD_STAND_OFF * 0.2f * half_tan_60;
		CHECK(fabsf(viewmodel_shield_offset_amount(effect.modifier_animation.values, 0.5f) - limit) < 1e-6f,
			"at most the share of half the view at the arms' distance (%g)", limit);
		CHECK(viewmodel_shield_offset_amount(effect.modifier_animation.values, limit / 2) == limit / 2,
			"a smaller offset is its own");
		global_window_parameters.camera.vertical_field_of_view = 0.5f;
		CHECK(viewmodel_shield_offset_amount(effect.modifier_animation.values, 0.5f) < limit,
			"a narrower viewmodel angle, a smaller offset");
		global_window_parameters.camera.vertical_field_of_view = 2.0f;
		CHECK(viewmodel_shield_offset_amount(effect.modifier_animation.values, 0.5f) > limit,
			"a wider one, a larger offset");
	}
	CASE("distance") {
		struct render_model_effect effect;
		real limit;
		setting = 1;
		arms_at(0.4f);
		effect = drawn(0, 0);
		limit = VIEWMODEL_SHIELD_STAND_OFF * 0.4f * half_tan_60;
		CHECK(fabsf(viewmodel_shield_offset_amount(effect.modifier_animation.values, 0.5f) - limit) < 1e-6f,
			"the arms' distance, past the root at the eye");
		arms_at(0.f);
		effect = drawn(0, 0);
		limit = VIEWMODEL_SHIELD_STAND_OFF * 0.01f * half_tan_60;
		CHECK(fabsf(viewmodel_shield_offset_amount(effect.modifier_animation.values, 0.5f) - limit) < 1e-7f,
			"no nearer than the first-person near clip");
	}
	CASE("split") {
		struct render_model_effect first, second;
		setting = 1;
		arms_at(0.2f);
		first = drawn(0, 0);
		arms_at(0.4f);
		second = drawn(1, 1);
		CHECK(first.modifier_animation.values != second.modifier_animation.values, "each local player's own copy");
		CHECK(fabsf(viewmodel_shield_offset_amount(first.modifier_animation.values, 0.5f) -
			VIEWMODEL_SHIELD_STAND_OFF * 0.2f * half_tan_60) < 1e-6f, "the first view's arms at their distance");
		CHECK(fabsf(viewmodel_shield_offset_amount(second.modifier_animation.values, 0.5f) -
			VIEWMODEL_SHIELD_STAND_OFF * 0.4f * half_tan_60) < 1e-6f, "the second view's at theirs");
	}
	CASE("overshield") {
		struct render_model_effect effect;
		real speed, softness, height;
		setting = 1;
		overshield_effects[0].unit_index = 0;
		overshield_effects[0].progress = 0.f;
		overshield_effects[0].bottom = 0.f;
		overshield_effects[0].top = 2.f;
		overshield_unit[0] = &overshield_effects[0];
		effect = drawn(0, 0);
		CHECK(effect.modifier_animation.values[1] == 0.f, "the rise has not reached the arms");
		CHECK(overshield_effect_draw(effect.modifier_animation.values, &speed, &softness, &height),
			"the overshield knows the arms' copy");
		CHECK(softness == 0.f && height == 0.f, "the arms come up whole, without the body's fade");
		CHECK(speed == OVERSHIELD_EFFECT_ENERGY_SPEED, "its energy's speed");
		overshield_effects[0].progress = 1.f;
		effect = drawn(0, 0);
		CHECK(effect.modifier_animation.values[1] == 1.f, "risen, the arms are lit whole");
		CHECK(fabsf(viewmodel_shield_offset_amount(effect.modifier_animation.values, 0.5f) -
			VIEWMODEL_SHIELD_STAND_OFF * 0.2f * half_tan_60) < 1e-6f, "and still kept close");
		/* (the unit's overshield gone: the copy is the unit's again, not the overshield's) */
		overshield_unit[0] = NULL;
		effect = drawn(0, 0);
		CHECK(effect.modifier_animation.values[1] == 0.5f, "without an overshield, the unit's own values");
		CHECK(!overshield_effect_draw(effect.modifier_animation.values, &speed, &softness, &height),
			"the overshield no longer draws on these arms");
		overshield_unit[0] = &overshield_effects[0];
		overshield_setting = 0;
		effect = drawn(0, 0);
		CHECK(!overshield_effect_draw(effect.modifier_animation.values, &speed, &softness, &height),
			"nor with display.overshield_effect off");
	}
	CASE("rasterizer-limit") {
		struct render_model_effect effect;
		real maximum = VIEWMODEL_SHIELD_STAND_OFF * 0.2f * half_tan_60;
		setting = 1;
		units[0].object.outgoing_function_values[0] = 2.f;
		effect = drawn(0, 0);
		CHECK(fabsf(plasma_offset(effect.modifier_animation.values, 0.5f, 1.f, 1) - maximum) < 1e-7f,
			"the final displacement is capped after a function value above one");
		CHECK(fabsf(plasma_offset(effect.modifier_animation.values, 0.5f, 2.f, 1) - maximum) < 1e-7f,
			"the exponent cannot multiply the displacement past the cap");
		units[0].object.outgoing_function_values[0] = 0.25f;
		effect = drawn(0, 0);
		CHECK(fabsf(plasma_offset(effect.modifier_animation.values, maximum * 2.f, 1.f, 1) -
			maximum * 0.5f) < 1e-7f, "a final displacement below the cap retains its authored size");
		CHECK(plasma_offset(effect.modifier_animation.values, 0.5f, 1.f, 0) == 0.f,
			"an absent offset source keeps the stock zero displacement");
	}
	CASE("rasterizer-stock") {
		real body[4] = {2.f, 0.f, 0.f, 0.f};
		setting = 1;
		(void)drawn(0, 0);
		CHECK(plasma_offset(body, 0.5f, 2.f, 1) == 2.f, "body and weapon plasma keep their final displacement");
		CHECK(plasma_offset(NULL, 0.5f, 2.f, 1) == 0.f, "no runtime values keep the stock displacement");
		setting = 0;
		CHECK(plasma_offset(body, 0.5f, 2.f, 1) == 2.f, "disabled, the stock calculation is unchanged");
	}
	CASE("toggle") {
		struct render_model_effect effect;
		setting = 1;
		effect = drawn(0, 0);
		CHECK(effect.modifier_shader == &plasma, "enabled");
		setting = 0;
		effect = drawn(0, 0);
		CHECK(!effect.modifier_shader && !effect.modifier_animation.values, "disabling stops submission");
		units[0].object.outgoing_function_values[1] = 0.75f;
		setting = 1;
		effect = drawn(0, 0);
		CHECK(effect.modifier_animation.values[1] == 0.75f, "reenabling copies the current values");
	}
	CASE("replace-player") {
		struct render_model_effect previous, current;
		real speed, softness, height;
		setting = 1;
		overshield_effects[0].unit_index = 0;
		overshield_effects[0].progress = 1.f;
		overshield_unit[0] = &overshield_effects[0];
		previous = drawn(0, 0);
		CHECK(overshield_effect_draw(previous.modifier_animation.values, &speed, &softness, &height),
			"the previous player's overshield owns this view's values");
		units[1].object.outgoing_function_values[1] = 0.25f;
		arms_at(0.4f);
		current = drawn(0, 1);
		CHECK(current.modifier_animation.values[1] == 0.25f, "a replacement unit supplies its own values");
		CHECK(!overshield_effect_draw(current.modifier_animation.values, &speed, &softness, &height),
			"reusing a local slot cannot retain the previous unit's overshield");
		CHECK(fabsf(viewmodel_shield_offset_amount(current.modifier_animation.values, 0.5f) -
			VIEWMODEL_SHIELD_STAND_OFF * 0.4f * half_tan_60) < 1e-6f, "the replacement's pose supplies its distance");
	}
	return 0;
}
