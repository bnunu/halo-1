/*
OBJECT_LISTS.C

symbols in this file:
000BD3F0 0040:
	_reference_list_new (0000)
000BD430 0030:
	_reference_list_get_next_datum_index (0000)
000BD460 0050:
	_reference_list_add (0000)
000BD4B0 0040:
	_reference_list_delete (0000)
000BD4F0 0060:
	_object_lists_initialize (0000)
000BD550 0010:
	_object_lists_dispose (0000)
000BD560 0020:
	_object_lists_initialize_for_new_map (0000)
000BD580 0020:
	_object_lists_dispose_from_old_map (0000)
000BD5A0 0040:
	_object_list_new (0000)
000BD5E0 0070:
	_object_list_delete (0000)
000BD650 0070:
	_object_list_add (0000)
000BD6C0 0030:
	_object_list_get_next (0000)
000BD6F0 0020:
	_object_list_add_reference (0000)
000BD710 0050:
	_object_list_remove_reference (0000)
000BD760 0060:
	_object_list_gc (0000)
000BD7C0 0030:
	_object_list_count (0000)
000BD7F0 0050:
	_object_list_get_first (0000)
0026F2B4 000d:
	??_C@_0N@DOAKPICE@?$CFs?5reference?$AA@ (0000)
0026F2C4 002c:
	??_C@_0CM@PKPEAIHK@WARNING?3?5maximum?5?$CFss?5per?5map?5?$CI?$CFd@ (0000)
0026F2F0 000c:
	??_C@_0M@CNNGENLL@list?5object?$AA@ (0000)
0026F2FC 0013:
	??_C@_0BD@BCDBOOBL@object?5list?5header?$AA@ (0000)
0026F310 0019:
	??_C@_0BJ@LLENHKLF@list?9?$DOreference_count?$DN?$DN0?$AA@ (0000)
0026F32C 0021:
	??_C@_0CB@EEGJCDAK@c?3?2halo?2SOURCE?2hs?2object_lists?4c@ (0000)
0026F350 0018:
	??_C@_0BI@NHAMEGAN@list?9?$DOreference_count?$DO0?$AA@ (0000)
*/

/* ---------- headers */

#include "cseries.h"
#include "object_lists.h"

#include "errors.h"
#include "data.h"
#include "reference_lists.h"
#include "objects/objects.h"

/* ---------- constants */

enum
{
	/* the native builds' more lists, and larger pool of listed objects
	(halo_port_capacity.h): a (players) list alone takes one per living
	player */
	MAXIMUM_OBJECT_LISTS_PER_MAP = HALO_PORT_MAXIMUM_OBJECT_LISTS_PER_MAP,
	MAXIMUM_LISTED_OBJECTS_PER_MAP = HALO_PORT_MAXIMUM_LISTED_OBJECTS_PER_MAP,
	/* port: list provenance; does not change the header's size. */
	_object_list_players_bit = 0,
};

/* ---------- macros */

/* ---------- structures */

/* ---------- prototypes */

static long object_list_skip_deleted(long object_index, long *reference_index);

/* ---------- globals */

struct data_array *object_list_header_data;
struct data_array *object_list_data;

/* ---------- public code */

void object_lists_initialize(
	void)
{
	object_list_header_data = game_state_data_new("object list header", MAXIMUM_OBJECT_LISTS_PER_MAP, sizeof(struct object_list_header_datum));
	object_list_data = reference_list_new("list object", MAXIMUM_LISTED_OBJECTS_PER_MAP);

	return;
}

void object_lists_dispose(
	void)
{
	return;
}

void object_lists_initialize_for_new_map(
	void)
{
	data_make_valid(object_list_header_data);
	data_make_valid(object_list_data);
	/* port: the new map's list warns again when full (cluster_partitions.c) */
	reference_list_port_report(object_list_data);

	return;
}

void object_lists_dispose_from_old_map(
	void)
{
	/* port: how many times the map's list was full past the first */
	reference_list_port_report(object_list_data);
	data_make_invalid(object_list_header_data);
	data_make_invalid(object_list_data);

	return;
}

long object_list_new(
	void)
{
	long list_index = datum_new(object_list_header_data);

	if (list_index != NONE)
	{
		struct object_list_header_datum* list = object_list_header_get(list_index);
		list->count = 0;
		list->flags = 0; /* port: a recycled slot has no previous provenance */
		list->first_reference_index = NONE;
	}

	return list_index;
}

/* port: only hs_players marks its completed result. */
void object_list_mark_players(long list_index)
{
	if (list_index != NONE)
		SET_FLAG(object_list_header_get(list_index)->flags, _object_list_players_bit, TRUE);
}

boolean object_list_is_players(long list_index)
{
	return list_index != NONE &&
		TEST_FLAG(object_list_header_get(list_index)->flags, _object_list_players_bit);
}

void object_list_delete(
	long list_index)
{
	if (list_index != NONE)
	{
		struct object_list_header_datum *list = object_list_header_get(list_index);
		match_assert("c:\\halo\\SOURCE\\hs\\object_lists.c", 100, list->reference_count==0);
		reference_list_delete(object_list_data, list->first_reference_index);
		datum_delete(object_list_header_data, list_index);
	}

	return;
}

void object_list_add(
	long object_list_index,
	long object_index)
{
	struct object_list_header_datum *list;

	/* port: no list (object_list_new found none free): the objects are
	not listed, as when the listed objects run out, rather than the game
	halting */
	if (object_list_index == NONE)
		return;

	list = object_list_header_get(object_list_index);
	/* port: a changed list no longer has hs_players' provenance. */
	list->flags = 0;
	reference_list_add(object_list_data, &list->first_reference_index, object_index);
	list->count++;

	return;
}

long object_list_get_next(
	long object_list_index,
	long *reference_index)
{
	/* port: (object_list_skip_deleted) */
	return object_list_skip_deleted(
		reference_list_get_next_datum_index(object_list_data, reference_index),
		reference_index);
}

void object_list_add_reference(
	long object_list_index)
{
	if (object_list_index!=NONE)
	{
		struct object_list_header_datum *list = object_list_header_get(object_list_index);
		list->reference_count++;
	}

	return;
}

void object_list_remove_reference(
	long object_list_index)
{
	if (object_list_index!=NONE)
	{
		struct object_list_header_datum *list = object_list_header_get(object_list_index);
		match_assert("c:\\halo\\SOURCE\\hs\\object_lists.c", 165, list->reference_count>0);
		list->reference_count--;
	}

	return;
}

void object_list_gc(
	void)
{
	long i;

	for (i = data_next_index(object_list_header_data, NONE);
		i!=NONE; 
		i = data_next_index(object_list_header_data, i))
	{
		if (object_list_header_get(i)->reference_count==0)
		{
			object_list_delete(i);
		}
	}

	return;
}

short object_list_count(
	long object_list_index)
{
	short result = 0;

	if (object_list_index!=NONE)
	{
		result = object_list_header_get(object_list_index)->count;
	}

	return result;
}

#define object_list_get_first_internal(object_list_index, reference_index, index) \
	do \
	{ \
		(index) = object_list_header_get(object_list_index)->first_reference_index; \
		*(reference_index) = (index); \
		if ((index) != NONE) \
		{ \
			struct data_reference *reference = object_list_get(index); \
			*(reference_index) = reference->next_reference_index; \
			(index) = reference->datum_index; \
		} \
		else \
		{ \
			(index) = NONE; \
		} \
	} while (0)

long object_list_get_first(
	long object_list_index,
	long *reference_index)
{
	long index = NONE;

	if (object_list_index != NONE)
	{
		object_list_get_first_internal(object_list_index, reference_index, index);
		/* port: (object_list_skip_deleted) */
		index = object_list_skip_deleted(index, reference_index);
	}

	return index;
}

/* ---------- private code */

/* port: a list in a script's global outlives the objects it holds, and its
entry for a deleted object still has that object's index. Whoever walks the list
gets the objects that still exist (hs_value_still_exists says why). list_count
still counts the entries, as it always has. */
static long object_list_skip_deleted(
	long object_index,
	long *reference_index)
{
	while (object_index!=NONE && !object_try_and_get(object_index))
		object_index = reference_list_get_next_datum_index(object_list_data, reference_index);

	return object_index;
}
