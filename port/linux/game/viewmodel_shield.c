/*
VIEWMODEL_SHIELD.C

The unit's energy shield on its first-person arms (Video Setup > FOV and
Viewmodels > Arm Shields, display.viewmodel_shield; off, the stock game's:
on the body only). The hands are drawn with the unit's own modifier shader,
its colors and values, as render_objects.c draws the body; the weapon is
not. With display.overshield_effect, the overshield comes up on the arms as
its rise passes them (overshield_effect.c).

The modifier is queued as transparent geometry and drawn later, in the
first-person pass, so the flare is known again by its values: each local
player's hands are given a copy of them here, which the plasma draw
(rasterizer_xbox_plasma_energy.c) passes back to
viewmodel_shield_offset_amount.

A plasma shield stands off its model by a distance in world units, made
for a body seen across a room; the arms are a few centimetres from the eye,
where that distance fills the view. On the arms it is kept to a share of
the view: at the arms' distance d, drawn at the vertical angle v (the
viewmodel's, display.viewmodel_fov, inside the first-person pass), half the
view's height is d tan(v/2) world units, and the flare stands off no more
than VIEWMODEL_SHIELD_STAND_OFF of that, whatever the field of view or the
resolution.
*/

#include "cseries.h"
#include "real_math.h"
#include "game/players.h"
#include "models/model_animation_definitions.h"
#include "models/model_definitions.h"
#include "objects/objects.h"
#include "objects/object_definitions.h"
#include "rasterizer/rasterizer.h"
#include "rasterizer/rasterizer_model_types.h"
#include "render/render.h"
#include "shaders/shader_definitions.h"
#include "shaders/shaders.h"
#include "units/units.h"

#include "viewmodel_shield.h"

#include <math.h>

/* the largest share of half the view's height the flare stands off the
arms by (tuned by eye) */
#define VIEWMODEL_SHIELD_STAND_OFF 0.01f

/* port/linux/src/port_config.c */
int config_boolean(const char *name);
unsigned long config_changes(void);

/* port/linux/game/overshield_effect.c */
boolean overshield_effect_arms_values(long unit_index, real const *values, real *arms);

/* port/linux/game/shield_color.c */
real_rgb_color const *shield_color_colors(long unit_index, real_rgb_color const *colors);

/* ---------- globals */

/* each local player's arms, rewritten each frame before they are drawn */
static struct
{
	/* the values the arms' modifier is drawn with */
	real values[NUMBER_OF_OUTGOING_OBJECT_FUNCTIONS];
	/* how far from the eye the arms are, world units */
	real distance;
} viewmodel_shields[MAXIMUM_LOCAL_PLAYERS];

/* ---------- private code */

static boolean viewmodel_shield_enabled(
	void)
{
	static unsigned long changes = (unsigned long)-1;
	static boolean enabled = FALSE;

	if (changes != config_changes())
	{
		changes = config_changes();
		enabled = config_boolean("display.viewmodel_shield") != 0;
	}
	return enabled;
}

/* how far the arms are from the eye: their nodes' mean distance, past the
root (the camera's own frame, at the eye), and no nearer than the closest
the first-person pass draws */
static real viewmodel_shield_arms_distance(
	struct model const *model,
	real_matrix4x3 const *node_matrices,
	real_point3d const *eye,
	real nearest)
{
	real sum = 0.f;
	short count = 0;
	short index;

	for (index = 0; index < model->nodes.count && index < MAXIMUM_NODES_PER_ANIMATION; index++)
	{
		struct model_node const *node = TAG_BLOCK_GET_ELEMENT(&model->nodes, index, struct model_node);

		if (node->parent_node_index == NONE)
			continue;
		sum += distance3d(eye, &node_matrices[index].position);
		count++;
	}
	if (count == 0 || !(sum / count > nearest))
		return nearest;
	return sum / count;
}

/* the farthest a flare stands off the arms at that distance and vertical
angle */
static real viewmodel_shield_maximum_offset(
	real distance,
	real vertical_field_of_view)
{
	return VIEWMODEL_SHIELD_STAND_OFF * distance * (real)tan(0.5 * (double)vertical_field_of_view);
}

/* ---------- public code */

void viewmodel_shield_apply(
	short local_player_index,
	long unit_index,
	long hands_model_index,
	real_matrix4x3 const *hands_node_matrices,
	real const *shown_functions,
	struct render_model_effect *effect)
{
	struct unit_datum *unit;
	struct object_definition *definition;
	struct shader *shader;

	if (!viewmodel_shield_enabled() || local_player_index < 0 || local_player_index >= MAXIMUM_LOCAL_PLAYERS)
		return;
	unit = unit_get(unit_index);
	definition = object_definition_get(unit->definition_index);
	if (definition->object.modifier_shader.index == NONE)
		return;
	/* (one that isn't is reported as the body is drawn) */
	shader = shader_definition_get(definition->object.modifier_shader.index);
	if (!shader_type_is_valid_for_modifier(shader->base.type))
		return;

	if (!overshield_effect_arms_values(unit_index, shown_functions,
		viewmodel_shields[local_player_index].values))
	{
		csmemcpy(viewmodel_shields[local_player_index].values, shown_functions,
			sizeof(viewmodel_shields[local_player_index].values));
	}
	viewmodel_shields[local_player_index].distance = viewmodel_shield_arms_distance(
		model_definition_get(hands_model_index), hands_node_matrices, &render.camera.position,
		rasterizer_globals.first_person_weapon_near_clip_distance);
	effect->modifier_shader = shader;
	effect->modifier_animation.colors = shield_color_colors(unit_index, unit->object.outgoing_change_colors);
	effect->modifier_animation.values = viewmodel_shields[local_player_index].values;
}

real viewmodel_shield_offset_amount(
	real const *values,
	real offset_amount)
{
	short index;

	for (index = 0; values && index < MAXIMUM_LOCAL_PLAYERS; index++)
	{
		if (values == viewmodel_shields[index].values)
		{
			real maximum = viewmodel_shield_maximum_offset(viewmodel_shields[index].distance,
				global_window_parameters.camera.vertical_field_of_view);

			return offset_amount > maximum ? maximum : offset_amount;
		}
	}
	return offset_amount;
}
