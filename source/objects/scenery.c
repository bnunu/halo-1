/*
SCENERY.C

symbols in this file:
00134D70 0010:
	_scenery_initialize (0000)
00134D80 0010:
	_scenery_initialize_for_new_map (0000)
00134D90 0010:
	_scenery_dispose_from_old_map (0000)
00134DA0 0010:
	_scenery_dispose (0000)
00134DB0 0020:
	_scenery_place (0000)
00134DD0 0080:
	_scenery_new (0000)
00134E50 0010:
	_scenery_delete (0000)
00134E60 0050:
	_scenery_update (0000)
00134EB0 00e0:
	_scenery_animation_start_private (0000)
00134F90 0070:
	_scenery_get_animation_time (0000)
00135000 0020:
	_scenery_animation_start (0000)
00135020 0020:
	_scenery_animation_start_at_frame (0000)
0028AC4C 0033:
	??_C@_0DD@PMOLPF@the?5animation?5?8?$CFs?8?5doesn?8t?5exist@ (0000)
*/

/* ---------- headers */

#include "cseries.h"
#include "object_types.h"
#include "objects.h"
#include "scenery.h"

#include "main/console.h"
#include "models/model_animation_definitions.h"
#include "objects/object_definitions.h"
#include "scenario/scenario_definitions.h"
#include "tag_files/tag_files.h"
#include "network_coop.h" /* port: port/linux/game/network_coop.c */

/* ---------- constants */

/* ---------- macros */

/* ---------- structures */

/* ---------- prototypes */

static void scenery_animation_start_private(
	long object_index,
	long animation_graph_index,
	char const *animation_name,
	short frame_index);

/* ---------- globals */

/* ---------- public code */

void scenery_initialize(
	void)
{
	return;
}

void scenery_initialize_for_new_map(
	void)
{
	return;
}

void scenery_dispose_from_old_map(
	void)
{
	return;
}

void scenery_dispose(
	void)
{
	return;
}

void scenery_place(
	long object_index,
	struct scenario_scenery_datum *scenario_scenery)
{
	object_add_scenario_permutation(
		object_index,
		(struct scenario_object_permutation *)((byte *)scenario_scenery + sizeof(struct scenario_object_datum)));

	return;
}

boolean scenery_new(
	long object_index)
{
	short animation_index;
	long animation_graph_index;
	unsigned long object_flags;
	struct animation_graph *animation_graph;
	struct object_definition *definition;
	struct scenery_datum *scenery = scenery_get(object_index);

	definition = object_definition_get(scenery->object.definition_index);
	if (definition->object.animation_graph.index != NONE)
	{
		animation_graph = animation_graph_definition_get(definition->object.animation_graph.index);
		if (animation_graph->animations.count > 0)
		{
			animation_graph_index = definition->object.animation_graph.index;
			animation_index = animation_choose_random_permutation_internal(
				_animation_overlay,
				animation_graph_index,
				0);
			if (animation_index != NONE)
			{
				scenery->object.object.animation.state.index = animation_index;
				object_flags = scenery->object.object.flags;
				animation_graph_index = definition->object.animation_graph.index;
				object_flags |= FLAG(7);
				scenery->object.object.animation.animation_graph_index = animation_graph_index;
				scenery->object.object.flags = object_flags;
			}
		}
	}

	scenery->object.object.flags |= FLAG(18);

	return TRUE;
}

void scenery_delete(
	long object_index)
{
	return;
}

boolean scenery_update(
	long object_index)
{
	long animation_graph_index;
	struct scenery_datum *scenery = scenery_get(object_index);

	if (TEST_FLAG(scenery->flags, 0))
	{
		animation_graph_index = scenery->object.object.animation.animation_graph_index;
		switch (animation_update_internal(
			1,
			animation_graph_index,
			&scenery->object.object.animation.state,
			FALSE))
		{
		case 2:
			scenery->object.object.animation.state.frame_index--;
			break;
		}
	}

	return TRUE;
}

short scenery_get_animation_time(
	long object_index)
{
	struct animation *animation;
	struct animation_graph *animation_graph;
	/* port: a script's scenery may be none (hs_value_still_exists) */
	struct scenery_datum *scenery = scenery_try_and_get(object_index);

	if (scenery && TEST_FLAG(scenery->flags, 0))
	{
		animation_graph = animation_graph_definition_get(
			scenery->object.object.animation.animation_graph_index);
		animation = TAG_BLOCK_GET_ELEMENT(
			&animation_graph->animations,
			scenery->object.object.animation.state.index,
			struct animation);

		return MAX(
			animation->frame_count - scenery->object.object.animation.state.frame_index - 2,
			0);
	}

	return 0;
}

void scenery_animation_start(
	long object_index,
	long animation_graph_index,
	char const *animation_name)
{
	scenery_animation_start_private(object_index, animation_graph_index, animation_name, 0);

	return;
}

void scenery_animation_start_at_frame(
	long object_index,
	long animation_graph_index,
	char const *animation_name,
	short frame_index)
{
	scenery_animation_start_private(object_index, animation_graph_index, animation_name, frame_index);

	return;
}

/* ---------- private code */

static void scenery_animation_start_private(
	long object_index,
	long animation_graph_index,
	char const *animation_name,
	short frame_index)
{
	short animation_index;
	struct animation *animation;
	struct animation_graph *animation_graph;
	struct scenery_datum *scenery;

	if (object_index != NONE && animation_graph_index != NONE)
	{
		scenery = scenery_get(object_index);
		animation_graph = animation_graph_definition_get(animation_graph_index);
		animation_index = animation_graph_get_animation_by_name(animation_graph_index, animation_name);
		if (animation_index != NONE)
		{
			animation = TAG_BLOCK_GET_ELEMENT(
				&animation_graph->animations,
				animation_index,
				struct animation);
			scenery->flags |= FLAG(0);
			scenery->object.object.flags &= ~FLAG(7);
			scenery->object.object.animation.state.index = animation_index;
			scenery->object.object.animation.state.frame_index =
				frame_index < 0 ? 0 : MIN(frame_index, animation->frame_count - 1);
			scenery->object.object.animation.animation_graph_index = animation_graph_index;
			network_coop_note_scenery_animation(object_index, animation_graph_index, animation_index,
				scenery->object.object.animation.state.frame_index);
		}
		else
		{
			console_warning(
				"the animation '%s' doesn't exist in the graph '%s'",
				animation_name,
				tag_get_name(animation_graph_index));
		}
	}

	return;
}
