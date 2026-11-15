/*
OVERSHIELD_EFFECT.C

The overshield, as this machine draws it (Video Setup > Extra Effects >
Overshield, display.overshield_effect; off, the game's own):

- As it charges, the shield's flare rises up the body from the feet to the
  top of the head, its edge soft, and stays lit, whole, for as long as the
  overshield lasts. The rise takes 2 seconds, the time the charge takes from
  a full shield, or 3 seconds when the shield was down (the charge's from
  nothing); from any other shield, 2. Meanwhile the shield's own flares
  (its hits, its recharge) are not drawn over it.
- The flare's energy moves at 30% of its speed. Above 200% shield it is
  green; the remaining layer through 200% is reddish-pink.
- On the first-person arms (with display.viewmodel_shield,
  viewmodel_shield.c) it comes up as the rise passes them, and stays.
- On a cloaked unit (active camouflage) it is as faint as the cloak is
  strong, as its glow is (shield_glow.c): a unit cloaked whole shows only
  its shield's own flares, as in the stock game. Lit whole under the cloak,
  it showed on a cloaked player's sniper rifle or rocket launcher where the
  weapon reached past the body, which its cloak hid it on.
- A hit on it plays what a hit on an Elite's shield plays (the hitting
  projectile's own effect for an Elite's energy shield: its sound, sparks
  and light), when the map has it.

Only what is drawn and heard: the overshield is the game's. The rise is
each machine's own, from when it sees the shield over charging (a machine
sent the unit's shields by its host sees it with them).

The rise is drawn by the GPU: the plasma shader's noise coordinate is the
vertex's position on its model, at rest (rasterizer_xbox_plasma_energy.c),
from which the vertex shader fades its colors out above a height
(d3d8_gl_set_height_fade); the height runs over the skeleton's rest pose,
from below its lowest node to above its highest.
*/

#include "cseries.h"
#include "real_math.h"
#include "overshield_effect.h"
#include "objects/objects.h"
#include "objects/object_definitions.h"
#include "models/model_definitions.h"
#include "rasterizer/rasterizer.h"
#include "shaders/shader_definitions.h"
#include "shaders/shaders.h"
#include "units/units.h"
#include "effects/effect_definitions.h"
#include "effects/decal_definitions.h"
#include "effects/contrail_definitions.h"
#include "effects/particle_system_definitions.h"
#include "effects/particles.h"
#include "sound/sound_definitions.h"
#include "objects/light_definitions.h"
#include "network_shield_effects.h"
#include <string.h>

enum
{
	_shader_type_transparent_plasma_glow = 10, /* shader_get_and_verify_type's plasma */
	OVERSHIELD_EFFECT_MAXIMUM = 16,
	OVERSHIELD_EFFECT_MODELS = 8,
};

/* seconds the rise takes: from a full shield (or any but a down one), and
from a down one (the charge's own times: 2 and 3 of shield a second) */
#define OVERSHIELD_EFFECT_RISE 2.f
#define OVERSHIELD_EFFECT_RISE_FROM_DOWN 3.f
/* a shield at most this when the overshield begins is down */
#define OVERSHIELD_EFFECT_DOWN 0.12f
/* the flare's energy's speed */
#define OVERSHIELD_EFFECT_ENERGY_SPEED 0.3f
/* the soft edge, as a share of the body's height */
#define OVERSHIELD_EFFECT_EDGE 0.12f
/* where on the rise the arms come up (the chest and shoulders: the rise
counts from the edge below the feet to the edge over the head), and over
how much of it */
#define OVERSHIELD_EFFECT_ARMS_START 0.55f
#define OVERSHIELD_EFFECT_ARMS_LENGTH 0.3f
/* effects.c's _effect_definition_must_be_deterministic_bit, of an effect
definition's flags (its enum is effects.c's own) */
#define EFFECT_MUST_BE_DETERMINISTIC_BIT 1

/* Remaining overshield layers: inner reddish-pink, outer green. */
static real_rgb_color const overshield_effect_rgb = { 0.992f, 0.306f, 0.455f }; /* (253, 78, 116) */
static real_rgb_color const overshield_effect_outer_rgb = { 0.306f, 0.992f, 0.455f }; /* (78, 253, 116) */

/* the plasma shader's fields after its header (rasterizer_xbox_plasma_energy.c's) */
struct overshield_effect_plasma
{
	byte reserved00[4];
	short intensity_exponent_source;
};

struct overshield_effect
{
	long unit_index; /* NONE: free */
	real start; /* the game's seconds (render_interpolation_game_time_sec) */
	real rise;
	real last_seen;
	real shield_vitality; /* current layer, shared by body and arms */
	/* the values the unit's modifier shader is drawn with, its intensity's
	full */
	real body_values[4];
	/* the first-person arms' copy, which viewmodel_shield.c keeps (NULL:
	none) */
	real const *arms_values;
	/* this frame's: how far up (0..1, eased), and the model's heights */
	real progress;
	real bottom;
	real top;
};

static struct overshield_effect overshield_effects[OVERSHIELD_EFFECT_MAXIMUM];
static boolean overshield_effects_initialized = FALSE;

/* the heights of a model's skeleton at rest, its lowest node's and its
highest's (from each node's default inverse, the model's space to the
node's) */
static struct
{
	long model_index;
	real bottom;
	real top;
} overshield_effect_models[OVERSHIELD_EFFECT_MODELS];
static short overshield_effect_next_model = 0;

/* port/linux/src/port_config.c */
int config_boolean(const char *name);
unsigned long config_changes(void);

/* Model tags and queued draw identities never cross a map or light-array reset. */
void overshield_effect_reset(void)
{
	short i;
	memset(overshield_effects, 0, sizeof(overshield_effects));
	for (i = 0; i < OVERSHIELD_EFFECT_MAXIMUM; i++)
		overshield_effects[i].unit_index = NONE;
	memset(overshield_effect_models, 0, sizeof(overshield_effect_models));
	for (i = 0; i < OVERSHIELD_EFFECT_MODELS; i++)
		overshield_effect_models[i].model_index = NONE;
	overshield_effect_next_model = 0;
	overshield_effects_initialized = TRUE;
}

static real_rgb_color const *overshield_effect_layer_color(struct overshield_effect const *effect)
{
	return effect->shield_vitality > 2.f ? &overshield_effect_outer_rgb : &overshield_effect_rgb;
}

/* display.overshield_effect; in a network game, the host's
(network_shield_effects.c) */
static boolean overshield_effect_enabled(void)
{
	static unsigned long changes = (unsigned long)-1;
	static boolean local_enabled = FALSE;
	short effects = network_shield_effects();
	boolean enabled;
	if (changes != config_changes())
	{
		changes = config_changes();
		local_enabled = config_boolean("display.overshield_effect") != 0;
	}
	enabled = effects != NONE ? effects >= _shield_effects_overshield : local_enabled;
	if (!enabled && overshield_effects_initialized)
	{
		overshield_effect_reset();
		overshield_effects_initialized = FALSE;
	}
	return enabled;
}

static boolean overshield_effect_model_heights(
	long model_index,
	real *bottom,
	real *top)
{
	struct model *model;
	short index;

	if (model_index == NONE)
		return FALSE;
	for (index = 0; index < OVERSHIELD_EFFECT_MODELS; index++)
	{
		if (overshield_effect_models[index].model_index == model_index)
		{
			*bottom = overshield_effect_models[index].bottom;
			*top = overshield_effect_models[index].top;
			return *top > *bottom;
		}
	}
	model = model_definition_get(model_index);
	if (!model || model->nodes.count <= 0)
		return FALSE;
	*bottom = REAL_MAX;
	*top = -REAL_MAX;
	for (index = 0; index < model->nodes.count; index++)
	{
		struct model_node const *node = TAG_BLOCK_GET_ELEMENT(&model->nodes, index, struct model_node);
		real_matrix4x3 const *inverse = &node->runtime_default_inverse_matrix;
		/* (the node's origin, p, has inverse p = 0: p = -R^T t / scale) */
		real z = -(inverse->up.i * inverse->position.x + inverse->up.j * inverse->position.y +
			inverse->up.k * inverse->position.z);

		if (inverse->scale > _real_epsilon)
			z /= inverse->scale;
		if (!valid_real(z))
			continue;
		*bottom = MIN(*bottom, z);
		*top = MAX(*top, z);
	}
	if (!(*top > *bottom))
		return FALSE;
	/* (the skin past the nodes: the soles below the toes, the helmet over
	the head) */
	{
		real height = *top - *bottom;

		*bottom -= 0.08f * height;
		*top += 0.18f * height;
	}
	index = overshield_effect_next_model;
	overshield_effect_next_model = (short)((overshield_effect_next_model + 1) % OVERSHIELD_EFFECT_MODELS);
	overshield_effect_models[index].model_index = model_index;
	overshield_effect_models[index].bottom = *bottom;
	overshield_effect_models[index].top = *top;
	return TRUE;
}

/* the source of the intensity of the unit's modifier shader (a plasma one),
0..3; NONE for none */
static short overshield_effect_intensity_source(
	long unit_index)
{
	struct object_definition *definition = object_definition_get(object_get(unit_index)->definition_index);
	struct shader *shader;
	short source;

	if (!definition || definition->object.modifier_shader.index == NONE)
		return NONE;
	shader = shader_definition_get(definition->object.modifier_shader.index);
	if (!shader || shader->base.type != _shader_type_transparent_plasma_glow)
		return NONE;
	source = ((struct overshield_effect_plasma const *)((byte const *)shader + sizeof(struct shader)))->
		intensity_exponent_source;
	return source >= 1 && source <= 4 ? (short)(source - 1) : NONE;
}

/* a unit's overshield, while it has one and lives (its shield over 1, or
charging past it); NULL for none */
static struct overshield_effect *overshield_effect_get(
	long unit_index)
{
	struct unit_datum *unit = unit_try_and_get(unit_index);
	struct overshield_effect *effect = NULL;
	struct overshield_effect *free_effect = NULL;
	boolean charging;
	real now;
	short index;

	if (!overshield_effect_enabled())
		return NULL;
	if (!overshield_effects_initialized)
		overshield_effect_reset();
	now = global_frame_parameters.game_time_sec;
	for (index = 0; index < OVERSHIELD_EFFECT_MAXIMUM; index++)
	{
		struct overshield_effect *each = &overshield_effects[index];

		/* (one not looked at for a while: its unit gone, or out of sight) */
		if (each->unit_index != NONE && (now - each->last_seen > 1.f || now < each->last_seen))
			each->unit_index = NONE;
		if (each->unit_index == unit_index)
			effect = each;
		else if (each->unit_index == NONE && !free_effect)
			free_effect = each;
	}
	if (!unit)
		return NULL;
	charging = TEST_FLAG(unit->object.damage_flags, _object_shield_over_charging_bit);
	if (TEST_FLAG(unit->object.damage_flags, _object_dead_bit) ||
		!(charging || unit->object.shield_vitality > 1.f))
	{
		if (effect)
			effect->unit_index = NONE;
		return NULL;
	}
	if (!effect)
	{
		real shield = unit->object.shield_vitality;

		if (!free_effect)
			return NULL;
		effect = free_effect;
		effect->unit_index = unit_index;
		effect->arms_values = NULL;
		effect->rise = charging && shield <= OVERSHIELD_EFFECT_DOWN ?
			OVERSHIELD_EFFECT_RISE_FROM_DOWN : OVERSHIELD_EFFECT_RISE;
		/* (seen part of the way through its charge: as far on; seen charged,
		risen) */
		effect->start = now;
		if (!charging)
			effect->start = now - effect->rise;
		else if (shield > 1.f)
			effect->start = now - MIN(shield - 1.f, effect->rise);
	}
	effect->last_seen = now;
	effect->shield_vitality = unit->object.shield_vitality;
	{
		real t = PIN((now - effect->start) / effect->rise, 0.f, 1.f);

		/* (smooth at both ends) */
		effect->progress = t * t * (3.f - 2.f * t);
	}
	return effect;
}

/* ---------- public code */

/* whether the unit has an overshield, as this machine draws it */
boolean overshield_effect_active(
	long object_index)
{
	struct object_datum *object = object_try_and_get(object_index);

	return object && TEST_FLAG(_object_mask_unit, object->object.type) && overshield_effect_get(object_index) != NULL;
}

/* how much of a unit's overshield shows through its cloak: 1 uncloaked,
none cloaked whole */
static real overshield_effect_uncloaked(
	long unit_index)
{
	struct unit_datum *unit = unit_try_and_get(unit_index);

	return unit ? PIN(1.f - unit->unit.active_camouflage, 0.f, 1.f) : 1.f;
}

/* the intensity an overshield's flare is drawn at, lit this far (the body
whole, the arms as far as the rise has reached them): as much of it as
shows through a cloak, and on the body no less than the shield's own flare
(its own intensity), which shows through a cloak as the stock game's does */
static real overshield_effect_intensity(
	real own,
	real lit,
	real uncloaked,
	boolean arms)
{
	return arms ? lit * uncloaked : MAX(own, lit * uncloaked);
}

/* the values a unit's modifier shader is drawn with, given its own: with an
overshield, a copy whose intensity is full on the body (as much as shows
through a cloak, and the shield's own flares under it) that
overshield_effect_draw knows again; otherwise its own */
real const *overshield_effect_values(
	long unit_index,
	real const *values)
{
	struct overshield_effect *effect;
	short source;
	real *copy;

	if (!values || !overshield_effect_enabled())
		return values;
	effect = overshield_effect_get(unit_index);
	source = effect ? overshield_effect_intensity_source(unit_index) : NONE;
	if (!effect || source == NONE)
		return values;
	copy = effect->body_values;
	csmemcpy(copy, values, sizeof(effect->body_values));
	copy[source] = overshield_effect_intensity(values[source], 1.f, overshield_effect_uncloaked(unit_index), FALSE);
	if (!overshield_effect_model_heights(
		object_definition_get(object_get(unit_index)->definition_index)->object.model.index,
		&effect->bottom, &effect->top))
	{
		effect->bottom = effect->top = 0.f;
	}
	return copy;
}

/* the values a unit's first-person arms' modifier shader is drawn with,
given its own, written to arms (viewmodel_shield.c's copy): with an
overshield, its intensity coming up as the rise passes the arms (as much as
shows through a cloak), and TRUE
(overshield_effect_draw then knows arms again); otherwise FALSE, and arms
not written */
boolean overshield_effect_arms_values(
	long unit_index,
	real const *values,
	real *arms)
{
	struct overshield_effect *effect;
	short source;
	short index;

	/* (another unit's overshield no longer draws on these arms) */
	for (index = 0; index < OVERSHIELD_EFFECT_MAXIMUM; index++)
	{
		if (overshield_effects[index].arms_values == arms)
			overshield_effects[index].arms_values = NULL;
	}
	if (!values || !overshield_effect_enabled())
		return FALSE;
	effect = overshield_effect_get(unit_index);
	source = effect ? overshield_effect_intensity_source(unit_index) : NONE;
	if (!effect || source == NONE)
		return FALSE;
	csmemcpy(arms, values, sizeof(effect->body_values));
	arms[source] = overshield_effect_intensity(values[source],
		PIN((effect->progress - OVERSHIELD_EFFECT_ARMS_START) / OVERSHIELD_EFFECT_ARMS_LENGTH, 0.f, 1.f),
		overshield_effect_uncloaked(unit_index), TRUE);
	effect->arms_values = arms;
	return TRUE;
}

/* Queued body and arms draws share the same identity and activation rules. */
static struct overshield_effect const *overshield_effect_find_values(real const *values)
{
	short index;
	if (!values || !overshield_effect_enabled())
		return NULL;
	for (index = 0; index < OVERSHIELD_EFFECT_MAXIMUM; index++)
	{
		struct overshield_effect const *effect = &overshield_effects[index];
		if (effect->unit_index != NONE && (values == effect->body_values || values == effect->arms_values))
			return effect;
	}
	return NULL;
}

/* a plasma shader drawn with values from overshield_effect_values (or
overshield_effect_arms_values, with no fade): the speed its energy moves
at, and the height it fades above on its model (the fade's inverse softness
and height: 0 and 0 for none); FALSE for values not from it */
boolean overshield_effect_draw(
	real const *values,
	real *energy_speed,
	real *inverse_softness,
	real *height)
{
	struct overshield_effect const *effect = overshield_effect_find_values(values);
	if (!effect)
		return FALSE;
	*energy_speed = OVERSHIELD_EFFECT_ENERGY_SPEED;
	*inverse_softness = 0.f;
	*height = 0.f;
	/* (the arms come up whole: they are not on the body's heights) */
	if (values == effect->body_values && effect->top > effect->bottom && effect->progress < 1.f)
	{
		real edge = OVERSHIELD_EFFECT_EDGE * (effect->top - effect->bottom);

		/* (from all of the edge below the feet to all of it above the
		head) */
		*inverse_softness = 1.f / edge;
		*height = effect->bottom + (effect->top - effect->bottom + edge) * effect->progress;
	}
	return TRUE;
}

/* whether an effect a hit on an overshield may play in place of its own:
one that only shows and sounds (sounds, lights, decals, particles,
contrails), so that nothing of the game changes */
boolean overshield_effect_hit_effect_allowed(
	long effect_index)
{
	struct effect_definition const *definition;
	short event_index;

	if (effect_index == NONE)
		return FALSE;
	definition = (struct effect_definition const *)tag_get(EFFECT_DEFINITION_TAG, effect_index);
	/* (nor one that must be deterministic: it draws from the game's random
	seed, which every machine must use alike, as this machine's swap would not) */
	if (!definition || TEST_FLAG(definition->flags, EFFECT_MUST_BE_DETERMINISTIC_BIT))
		return FALSE;
	for (event_index = 0; event_index < definition->events.count; event_index++)
	{
		struct effect_event_definition const *event =
			TAG_BLOCK_GET_ELEMENT(&definition->events, event_index, struct effect_event_definition);
		short part_index;

		for (part_index = 0; part_index < event->parts.count; part_index++)
		{
			struct effect_part_definition const *part =
				TAG_BLOCK_GET_ELEMENT(&event->parts, part_index, struct effect_part_definition);
			unsigned long group = part->reference.group_tag;

			if (part->reference.index != NONE && group != SOUND_DEFINITION_TAG && group != LIGHT_DEFINITION_TAG &&
				group != DECAL_GROUP_TAG && group != PARTICLE_SYSTEM_DEFINITION_TAG && group != PARTICLE_TAG &&
				group != CONTRAIL_DEFINITION_TAG)
			{
				return FALSE;
			}
		}
	}
	return TRUE;
}

/* the color a plasma shader drawn with values from overshield_effect_values
(or overshield_effect_arms_values) is drawn in, the overshield's own; FALSE
for values not from it */
boolean overshield_effect_color(
	real const *values,
	real_rgb_color *perpendicular,
	real_rgb_color *parallel)
{
	struct overshield_effect const *effect = overshield_effect_find_values(values);
	real peak;
	real perpendicular_brightness;
	real parallel_brightness;
	real_rgb_color const *color;

	if (!effect)
		return FALSE;
	color = overshield_effect_layer_color(effect);
	/* Keep the shader's brightness while using the overshield effect's hue. */
	peak = MAX(color->red, MAX(color->green, color->blue));
	perpendicular_brightness = MAX(perpendicular->red, MAX(perpendicular->green, perpendicular->blue));
	parallel_brightness = MAX(parallel->red, MAX(parallel->green, parallel->blue));
	perpendicular->red = color->red / peak * perpendicular_brightness;
	perpendicular->green = color->green / peak * perpendicular_brightness;
	perpendicular->blue = color->blue / peak * perpendicular_brightness;
	parallel->red = color->red / peak * parallel_brightness;
	parallel->green = color->green / peak * parallel_brightness;
	parallel->blue = color->blue / peak * parallel_brightness;
	return TRUE;
}

/* One sample supplies both layer hue and visible charge coverage to lighting. */
boolean overshield_effect_light_sample(long unit_index, real_rgb_color *color, real *coverage)
{
	struct overshield_effect *effect = overshield_effect_get(unit_index);
	if (!effect)
		return FALSE;
	*color = *overshield_effect_layer_color(effect);
	*coverage = effect->progress;
	return TRUE;
}
