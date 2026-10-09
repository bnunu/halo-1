/*
MATERIAL_EFFECTS.C

symbols in this file:
0008DA20 0080:
	_material_effect_visible (0000)
0008DAA0 0140:
	_material_effect_new (0000)
0008DBE0 0110:
	_material_effect_new_from_point (0000)
0043D589 0001:
	_debug_material_effects (0000)
*/

/* ---------- headers */

#include "effects/material_effects.h"

#include "camera/observer.h"
#include "effects/effects.h"
#include "effects/material_effect_definitions.h"
#include "game/players.h"
#include "networking/network_connection.h"
#include "physics/collisions.h"
#include "render/render_debug.h"
#include "scenario/scenario.h"
#include "sound/game_sound.h"

/* ---------- constants */

enum
{
	_material_effect_collision_flags =
		_collision_test_objects_sight_blocking_flags |
		FLAG(_collision_test_structure_bit) |
		FLAG(_collision_test_objects_bit),
	_material_effect_underwater_material_type = 0x1C,
};

/* ---------- macros */

/* ---------- structures */

/* ---------- prototypes */

/* ---------- globals */

boolean debug_material_effects = FALSE;

/* ---------- public code */

boolean material_effect_visible(
	real_point3d const *position)
{
	boolean visible = FALSE;
	short local_player_index;

	if (local_player_count() > 2)
	{
		visible = TRUE;
	}
	else
	{
		for (local_player_index = 0; local_player_index < MAXIMUM_NUMBER_OF_LOCAL_PLAYERS; local_player_index++)
		{
			if (local_player_get_player_index(local_player_index) != NONE)
			{
				struct observer_result const *camera = observer_get_camera(local_player_index);
				if (distance_squared3d(&camera->position, position) < 100.0f)
				{
					visible = TRUE;
					break;
				}
			}
		}
	}

	return visible;
}

void material_effect_new(
	long definition_index,
	short effect_index,
	short material_index,
	real_point3d const *position,
	real_vector3d const *normal,
	struct location const *location,
	real scale)
{
	struct material_effects_definition *definition;

	definition = material_effects_definition_get(definition_index);
	if (effect_index < definition->effects.count)
	{
		struct material_effect *effect;

		effect = TAG_BLOCK_GET_ELEMENT(&definition->effects, effect_index, struct material_effect);
		if (material_index != NONE && material_index < effect->materials.count)
		{
			struct material_effect_material *material;
			real_point3d effect_position;

			material = TAG_BLOCK_GET_ELEMENT(&effect->materials, material_index, struct material_effect_material);
			effect_position.x = position->x + normal->i * 0.01f;
			effect_position.y = position->y + normal->j * 0.01f;
			effect_position.z = position->z + normal->k * 0.01f;

			if (material->effect.index != NONE)
			{
				effect_new_unattached_from_markers(
					material->effect.index,
					NONE,
					NULL,
					1,
					NULL,
					&effect_position,
					normal,
					scale,
					0.f,
					NULL,
					NULL,
					FALSE);
			}

			if (material->sound.index != NONE)
			{
				struct sound_location sound_location;

				sound_location.position = effect_position;
				sound_location.forward = *normal;
				sound_location.translational_velocity = *global_zero_vector3d;
				sound_location.game_location = *location;
				unattached_impulse_sound_new(material->sound.index, &sound_location, scale);
			}

			if (debug_material_effects)
				render_debug_sphere(FALSE, position, 0.05f, global_real_argb_cyan);
		}
	}

	return;
}

void material_effect_new_from_point(
	long definition_index,
	short effect_index,
	real_point3d const *position,
	real scale)
{
	struct material_effects_definition *definition;

	definition = material_effects_definition_get(definition_index);
	if (effect_index < definition->effects.count)
	{
		real_point3d test_point;
		real_vector3d test_vector;
		struct collision_result collision;

		TAG_BLOCK_GET_ELEMENT(&definition->effects, effect_index, struct material_effect);
		test_point = *position;
		test_point.z += 0.15f;
		test_vector.i = global_down3d->i * 0.3f;
		test_vector.j = global_down3d->j * 0.3f;
		test_vector.k = global_down3d->k * 0.3f;

		if (collision_test_vector(
			_material_effect_collision_flags,
			&test_point,
			&test_vector,
			NONE,
			&collision))
		{
			long material_type;

			/* The January build consumes the complete 32-bit material slot here. */
			material_type = scenario_location_underwater(&collision.location, &collision.point, NULL)
				? _material_effect_underwater_material_type
				: *(long *)&collision.material_type;
			material_effect_new(
				definition_index,
				effect_index,
				(short)material_type,
				&collision.point,
				&collision.plane.n,
				&collision.location,
				scale);
		}
		else if (debug_material_effects)
		{
			render_debug_sphere(FALSE, position, 0.05f, global_real_argb_red);
		}
	}

	return;
}

/* ---------- private code */
