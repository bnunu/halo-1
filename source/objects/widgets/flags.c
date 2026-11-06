/*
FLAGS.C

symbols in this file:
00121090 00b0:
	_flag_datum_get_vertex (0000)
00121140 00c0:
	_flag_datum_get_cell (0000)
00121200 0030:
	_flags_initialize (0000)
00121230 0010:
	_flags_initialize_for_new_map (0000)
00121240 0010:
	_flags_dispose_from_old_map (0000)
00121250 0020:
	_flags_dispose (0000)
00121270 0120:
	_flag_tesselate_region (0000)
00121390 0020:
	_flag_delete (0000)
001213B0 02a0:
	_flag_update_attachment (0000)
00121650 00d0:
	_flag_set_attachment_shape (0000)
00121720 00f0:
	_flag_set_trailing_shape (0000)
00121810 04a0:
	_flag_update (0000)
00121CB0 0840:
	_flag_render_proper (0000)
001224F0 0180:
	_flag_new (0000)
00122670 0080:
	_flag_render (0000)
001226F0 00a0:
	_flags_update (0000)
00288F18 001d:
	??_C@_0BN@NBECLHNE@y?$DO?$DN0?5?$CG?$CG?5y?$DMdefinition?9?$DOheight?$AA@ (0000)
00288F38 001c:
	??_C@_0BM@BBBAMOGB@x?$DO?$DN0?5?$CG?$CG?5x?$DMdefinition?9?$DOwidth?$AA@ (0000)
00288F54 0013:
	??_C@_0BD@JPECIJNC@flag?5?$CG?$CG?5definition?$AA@ (0000)
00288F68 0027:
	??_C@_0CH@MIJJCHGH@c?3?2halo?2SOURCE?2objects?2widgets?2f@ (0000)
00288F90 001f:
	??_C@_0BP@EGJJCFHH@y?$DO?$DN0?5?$CG?$CG?5y?$DMdefinition?9?$DOheight?91?$AA@ (0000)
00288FB0 001e:
	??_C@_0BO@PGPCHGEA@x?$DO?$DN0?5?$CG?$CG?5x?$DMdefinition?9?$DOwidth?91?$AA@ (0000)
00288FD0 001f:
	??_C@_0BP@CFMNJLHE@couldn?8t?5allocate?5flag?5globals?$AA@ (0000)
00288FF0 0005:
	??_C@_04DNPGJOMD@flag?$AA@ (0000)
00288FF8 0004:
	__real@3927c5ac (0000)
00288FFC 0004:
	__real@39d1b717 (0000)
00289000 0014:
	??_C@_0BE@KKEACJGN@i?$DMmaximum_triangles?$AA@ (0000)
*/

/* ---------- headers */

#include "objects/widgets/flags.h"

#include "cseries/cseries.h"
#include "cseries/errors.h"
#include "game/game.h"
#include "memory/data.h"
#include "objects/object_definitions.h"
#include "objects/objects.h"
#include "physics/point_physics.h"
#include "rasterizer/rasterizer.h"
#include "rasterizer/rasterizer_geometry.h"
#include "rasterizer/rasterizer_model_types.h"
#include "rasterizer/rasterizer_models.h"
#include "saved games/game_state.h"
#include "scenario/scenario.h"
#include "shaders/shader_definitions.h"
#include "shaders/shaders.h"
#include "tag_files/tag_groups.h"

/* ---------- constants */

enum
{
	MAXIMUM_FLAG_WIDTH = 40,
	MAXIMUM_FLAG_VERTICES = 225,
	MAXIMUM_FLAG_CELLS = 196,
	MAXIMUM_FLAG_HEIGHT = 40,
	MAXIMUM_FLAG_ATTACHMENT_POINTS = 5,
	NUMBER_OF_FLAG_NEIGHBORS = 3,
};

enum trailing_edge_shape
{
	_trailing_edge_shape_flat,
	_trailing_edge_shape_concave_triangular,
	_trailing_edge_shape_convex_triangular,
	_trailing_edge_shape_trapezoid_short_top,
	_trailing_edge_shape_trapezoid_short_bottom
};

enum attached_edge_shape
{
	_attached_edge_shape_flat,
	_attached_edge_shape_triangular_notch
};

enum tesselate
{
	_tesselate_both,
	_tesselate_none,
	_tesselate_top_left,
	_tesselate_bottom_left,
	_tesselate_top_right,
	_tesselate_bottom_right
};

/* ---------- macros */

#define FLAG_TAG 'flag'

#define flag_get(index) \
	((struct flag_datum_prefix *)datum_get(flag_data, (index)))
#define flag_definition_get(index) \
	((struct flag_definition *)tag_get(FLAG_TAG, (index)))

/* ---------- structures */

struct flag_datum_prefix
{
	struct datum_header header;
	boolean noop;
	boolean initialized;
	boolean update_state;
	byte pad5;
	short updates_since_last_render;
	long object_index;
	long definition_index;
	real_point3d first_attachment;
	struct flag_vertex_datum
	{
		real_point3d position;
		real_vector3d velocity;
	} vertices[MAXIMUM_FLAG_VERTICES];
	struct flag_cell_datum
	{
		short tesselation;
	} cells[MAXIMUM_FLAG_CELLS];
};

struct flag_attachment_point
{
	short height_to_next_attachment;
	short pad2;
	long unused[4];
	char marker_name[32];
};

struct flag_definition
{
	unsigned long flags;
	short trailing_edge_shape;
	short trailing_edge_offset;
	short attached_edge_shape;
	short padA;
	short width;
	short height;
	real cell_width_scale;
	real cell_height_scale;
	struct tag_reference shader_red;
	struct tag_reference physics;
	real wind_noise;
	long unused3C[2];
	struct tag_reference shader_blue;
	struct tag_block attachment_points;
};

/* ---------- prototypes */

void flag_update(
	struct flag_datum_prefix *flag,
	struct flag_definition *flag_definition,
	real time);
void flag_render_proper(
	struct flag_datum_prefix *flag,
	struct flag_definition *definition,
	struct render_lighting const *lighting,
	struct render_animation const *animation);
struct flag_vertex_datum *flag_datum_get_vertex(
	struct flag_datum_prefix *flag,
	struct flag_definition *definition,
	short x,
	short y);
struct flag_cell_datum *flag_datum_get_cell(
	struct flag_datum_prefix *flag,
	struct flag_definition *definition,
	short x,
	short y);
void flag_tesselate_region(
	struct flag_definition *definition,
	struct flag_datum_prefix *flag,
	short x,
	short y,
	short size,
	short tesselation);
void flag_set_attachment_shape(
	struct flag_definition *definition,
	struct flag_datum_prefix *flag);
void flag_set_trailing_shape(
	struct flag_definition *definition,
	struct flag_datum_prefix *flag);

/* ---------- globals */

struct data_array *flag_data;

/* ---------- public code */

void flags_initialize(
	void)
{
	flag_data = game_state_data_new("flag", 2, 0x16BC);
	if (!flag_data)
		error(_error_immediate, "couldn't allocate flag globals");

	return;
}

void flags_initialize_for_new_map(
	void)
{
	data_make_valid(flag_data);

	return;
}

void flags_dispose_from_old_map(
	void)
{
	data_make_invalid(flag_data);

	return;
}

void flags_dispose(
	void)
{
	if (flag_data)
		flag_data = NULL;

	return;
}

void flag_delete(
	long flag_index)
{
	datum_delete(flag_data, flag_index);

	return;
}

void flags_update(
	real delta)
{
	long flag_index;
	/* port: the cloth steps a tick at a time, as on the Xbox, which drew a
	frame a tick. Each step moves every vertex to where its neighbours hold
	it (flag_update), and the native builds stepped every frame, several a
	tick (port/linux/game/render_interpolation.c): eight steps a tick at 240
	frames a second held the cloth eight times as stiff, and it hung and
	flapped unlike the Xbox's. Step on the first frame after each tick, for
	all the time since the last step; the frames between draw it where its
	object is drawn (flag_render_proper). */
	static long last_step_game_time = NONE;
	static real pending_delta = 0.0f;

	pending_delta += delta;
	if (last_step_game_time == game_time_get())
		return;
	last_step_game_time = game_time_get();
	delta = pending_delta;
	pending_delta = 0.0f;

	for (flag_index = data_next_index(flag_data, NONE);
		flag_index != NONE;
		flag_index = data_next_index(flag_data, flag_index))
	{
		struct flag_datum_prefix *flag = flag_get(flag_index);
		struct flag_definition *definition =
			flag_definition_get(flag->definition_index);

		flag->updates_since_last_render++;
		if (flag->object_index != NONE &&
			flag->updates_since_last_render < 5 &&
			delta != 0.0f)
		{
			flag_update(flag, definition, delta);
		}
	}

	return;
}

void flag_render(
	long object_index,
	long flag_index,
	struct render_lighting const *lighting,
	struct render_animation const *animation)
{
	struct flag_datum_prefix *flag;
	struct flag_definition *definition;

	object_get(object_index);
	flag = flag_get(flag_index);
	definition = flag_definition_get(flag->definition_index);
	flag->object_index = object_index;

	if (flag->updates_since_last_render > 5 || !flag->initialized)
	{
		flag_update(flag, definition, 5.0f);
		flag->initialized = TRUE;
	}

	flag->updates_since_last_render = 0;
	if (!flag->noop)
		flag_render_proper(flag, definition, lighting, animation);

	return;
}

long flag_new(
	long definition_index)
{
	long flag_index;

	global_scenario_get();

	flag_index = NONE;
	if (definition_index != NONE)
	{
		struct flag_definition *definition = flag_definition_get(definition_index);

		flag_index = datum_new(flag_data);
		if (flag_index != NONE)
		{
			struct flag_datum_prefix *flag = flag_get(flag_index);
			/* port: and a height the update's rows hold, with the one past
			the last that the attachments' spans reach, and a size that isn't
			negative (a map's; a negative width let any height through the
			product; retail's flag is 16 by 13) */
			boolean bad_size = definition->width < 0 ||
				definition->height < 0 ||
				definition->height >= MAXIMUM_FLAG_HEIGHT;

			if (bad_size)
			{
				static boolean size_reported = FALSE;

				if (!size_reported)
				{
					size_reported = TRUE;
					error(
						_error_silent,
						"### ERROR a flag is %d by %d; it isn't simulated",
						definition->width,
						definition->height);
				}
			}

			if (definition->height * definition->width >= MAXIMUM_FLAG_VERTICES ||
				definition->width >= MAXIMUM_FLAG_WIDTH ||
				bad_size ||
				definition->shader_blue.index == NONE)
			{
				flag->noop = TRUE;
				/* port: still the flag's own definition and no object yet,
				which flags_update and flag_render read for it too (they read
				whatever the slot held) */
				flag->initialized = FALSE;
				flag->object_index = NONE;
				flag->definition_index = definition_index;
			}
			else
			{
				short x;
				short y;

				flag->noop = FALSE;
				flag->initialized = FALSE;
				flag->object_index = NONE;
				flag->definition_index = definition_index;
				flag->first_attachment.x = flag->first_attachment.y = flag->first_attachment.z = 0.0f;

				for (x = 0; x < definition->width; x++)
				{
					for (y = 0; y < definition->height; y++)
					{
						struct flag_vertex_datum *vertex = flag_datum_get_vertex(flag, definition, x, y);

						vertex->position = *global_origin3d;
						vertex->velocity = *global_zero_vector3d;
						if (x < definition->width - 1 && y < definition->height - 1)
							flag_datum_get_cell(flag, definition, x, y)->tesselation = _tesselate_both;
					}
				}

				flag_set_attachment_shape(definition, flag);
				flag_set_trailing_shape(definition, flag);
			}
		}
	}

	return flag_index;
}

void flag_update_attachment(
	struct flag_datum_prefix *flag,
	struct flag_definition *definition,
	struct location *location,
	real_point3d *attachment_points,
	real_point3d *attachment_force_points,
	short *attachment_y,
	short *y_attachments)
{
	struct object_marker marker;
	real_vector3d delta;
	short i;
	short y;
	/* port: no more attachment points than the caller's arrays hold (a map's
	count; retail's flag has 2) */
	short attachment_point_count = (short)PIN(definition->attachment_points.count, 0, MAXIMUM_FLAG_ATTACHMENT_POINTS);

	for (i = 0; i < attachment_point_count; i++)
	{
		struct flag_attachment_point *point = TAG_BLOCK_GET_ELEMENT(
			&definition->attachment_points,
			i,
			struct flag_attachment_point);

		object_get_marker_by_name(flag->object_index, point->marker_name, &marker, 1);
		attachment_points[i] = marker.matrix.position;
	}

	scenario_location_from_point(location, attachment_points);

	if (flag->noop)
		return;

	for (y = 0; y < definition->height; y++)
		y_attachments[y] = NONE;

	y = 0;
	for (i = 0; i < attachment_point_count; i++)
	{
		struct flag_attachment_point *point;
		short span;
		short y_end;
		short row_start;

		if (y >= definition->height)
			break;

		point = TAG_BLOCK_GET_ELEMENT(
			&definition->attachment_points,
			i,
			struct flag_attachment_point);
		span = PIN(point->height_to_next_attachment, 0, definition->height - y) & ~1;
		y_end = y + span;

		attachment_y[i] = y;
		y_attachments[y] = i;
		row_start = y;

		for (; y <= y_end; y++)
		{
			real t = ((real)y - row_start) / ((real)y_end - row_start);
			real_point3d *from = &attachment_points[i];
			/* port: the last point the array holds goes to itself (a map's
			span past it read past the array) */
			real_point3d *to = i + 1 < MAXIMUM_FLAG_ATTACHMENT_POINTS ? &attachment_points[i + 1] : from;
			real_point3d *force_point = &attachment_force_points[y];

			force_point->x = from->x * (1.0f - t) + to->x * t;
			force_point->y = from->y * (1.0f - t) + to->y * t;
			force_point->z = from->z * (1.0f - t) + to->z * t;
		}

		y--;
	}

	delta.i = attachment_points->x - flag->first_attachment.x;
	delta.j = attachment_points->y - flag->first_attachment.y;
	delta.k = attachment_points->z - flag->first_attachment.z;

	/* port: the movement compared as it is, as in antenna_update_attachment
	(it was truncated to whole units first, so it took two units of movement
	to move the vertices along) */
	if (fabsf(delta.i) > 1.0f ||
		fabsf(delta.j) > 1.0f ||
		fabsf(delta.k) > 1.0f)
	{
		short x;

		for (x = 0; x < definition->width; x++)
		{
			for (y = 0; y < definition->height; y++)
			{
				struct flag_vertex_datum *vertex = flag_datum_get_vertex(flag, definition, x, y);

				vertex->position.x += delta.i;
				vertex->position.y += delta.j;
				vertex->position.z += delta.k;
			}
		}
	}

	flag->first_attachment = *attachment_points;

	return;
}

void flag_update(
	struct flag_datum_prefix *flag,
	struct flag_definition *flag_definition,
	real time)
{
	short weather_palette_index;
	real_point3d attachment_points[MAXIMUM_FLAG_ATTACHMENT_POINTS];
	short attachment_y[MAXIMUM_FLAG_ATTACHMENT_POINTS];
	struct location attachment_location;
	real_point3d attachment_force_points[MAXIMUM_FLAG_HEIGHT];
	short y_attachments[MAXIMUM_FLAG_HEIGHT];
	boolean under_water;
	short new_update_state = !flag->update_state;

	object_get(flag->object_index);
	flag_update_attachment(
		flag,
		flag_definition,
		&attachment_location,
		attachment_points,
		attachment_force_points,
		attachment_y,
		y_attachments);
	/* port: the last point flag_update_attachment set (a map's count) */
	under_water = scenario_location_underwater(
		&attachment_location,
		&attachment_points[PIN(flag_definition->attachment_points.count, 1, MAXIMUM_FLAG_ATTACHMENT_POINTS) - 1],
		&weather_palette_index);

	if (!flag->noop)
	{
		real parent_distances[NUMBER_OF_FLAG_NEIGHBORS];
		point2d const parent_deltas[NUMBER_OF_FLAG_NEIGHBORS] = {{-1, 0}, {0, 1}, {0, -1}};
		short column;
		short i;

		for (i = 0; i < NUMBER_OF_FLAG_NEIGHBORS; i++)
		{
			real_vector2d parent_vector;

			parent_vector.i = parent_deltas[i].x * flag_definition->cell_width_scale;
			parent_vector.j = parent_deltas[i].y * flag_definition->cell_height_scale;
			parent_distances[i] = magnitude2d(&parent_vector);
		}

		for (column = 0; column < flag_definition->width; column++)
		{
			short delta = new_update_state ? 1 : -1;
			short y;

			for (y = new_update_state ? 0 : flag_definition->height - 1;
				new_update_state ? y < flag_definition->height : y > 0;
				y += delta)
			{
				short attachment_index_for_row = y_attachments[y];
				struct flag_vertex_datum *vertex = flag_datum_get_vertex(flag, flag_definition, column, y);
				short estimated_position_count = 0;
				unsigned long physics_flags = FLAG(_point_physics_ignore_position_bit);
				real_point3d new_position;
				struct location new_location;
				real_vector3d turbulence;
				real wind_scale;

				if (!under_water)
				{
					wind_scale = point_physics_definition_get(flag_definition->physics.index)->air_friction *
						flag_definition->wind_noise * 0.0004f;
				}
				else
				{
					wind_scale = point_physics_definition_get(flag_definition->physics.index)->water_friction *
						flag_definition->wind_noise * 0.00016f;
					SET_FLAG(physics_flags, _point_physics_ignore_position_under_water_bit, TRUE);
				}

				/* The native builds update flags every frame, several frames a
				tick (port/linux/game/render_interpolation.c): a new random push
				every frame would mostly cancel out and leave the cloth stiller
				the higher the frame rate. Keep each vertex's push for the whole
				tick, as long as it lasted on the Xbox. */
				{
					unsigned long seed = (unsigned long)game_time_get() * 0x9E3779B1UL ^
						(unsigned long)(size_t)flag ^
						((unsigned long)column << 16 | (unsigned short)y);

					seed ^= seed >> 16;
					seed *= 0x7FEB352DUL;
					seed ^= seed >> 15;
					seed *= 0x846CA68BUL;
					seed ^= seed >> 16;
					seed_random_direction3d(&seed, &turbulence);
				}
				scale_vector3d(&turbulence, wind_scale, &turbulence);

				new_position = vertex->position;
				new_location = attachment_location;
				point_physics_update(
					physics_flags,
					point_physics_definition_get(flag_definition->physics.index),
					&new_location,
					weather_palette_index,
					&new_position,
					&vertex->velocity,
					&turbulence,
					NULL,
					NULL,
					0.02f,
					time);

				if (column == 0 && attachment_index_for_row != NONE)
				{
					new_position = attachment_points[attachment_index_for_row];
				}
				else
				{
					real_point3d estimated_positions[NUMBER_OF_FLAG_NEIGHBORS];
					real one_over_n;

					for (i = 0; i < NUMBER_OF_FLAG_NEIGHBORS; i++)
					{
						short neighbor_column = column + parent_deltas[i].x;
						short neighbor_row = y + parent_deltas[i].y;

						if (neighbor_column >= 0 && neighbor_column < flag_definition->width &&
							neighbor_row >= 0 && neighbor_row < flag_definition->height)
						{
							struct flag_vertex_datum *neighbor = flag_datum_get_vertex(flag, flag_definition, neighbor_column, neighbor_row);
							real_vector3d parent;

							parent.i = new_position.x - neighbor->position.x;
							parent.j = new_position.y - neighbor->position.y;
							parent.k = new_position.z - neighbor->position.z;
							normalize3d(&parent);
							estimated_positions[estimated_position_count].x = neighbor->position.x + parent.i * parent_distances[i];
							estimated_positions[estimated_position_count].y = neighbor->position.y + parent.j * parent_distances[i];
							estimated_positions[estimated_position_count].z = neighbor->position.z + parent.k * parent_distances[i];
							estimated_position_count++;
						}
					}

					one_over_n = 0.0f;
					new_position.x = new_position.y = new_position.z = 0.0f;
					for (i = 0; i < estimated_position_count; i++)
					{
						real weight = (column != 0 && i == 0) ? 4.0f : 1.0f;

						new_position.x += weight * estimated_positions[i].x;
						new_position.y += weight * estimated_positions[i].y;
						new_position.z += weight * estimated_positions[i].z;
						one_over_n += weight;
					}

					if (column == 0)
					{
						real weight = 4.0f;

						new_position.x += weight * attachment_force_points[y].x;
						new_position.y += weight * attachment_force_points[y].y;
						new_position.z += weight * attachment_force_points[y].z;
						one_over_n += weight;
					}

					one_over_n = 1.0f / one_over_n;
					new_position.x *= one_over_n;
					new_position.y *= one_over_n;
					new_position.z *= one_over_n;
				}

				vertex->velocity.i = (new_position.x - vertex->position.x) / time;
				vertex->velocity.j = (new_position.y - vertex->position.y) / time;
				vertex->velocity.k = (new_position.z - vertex->position.z) / time;
				vertex->position = new_position;
			}
		}
	}

	return;
}

/* ---------- private code */

struct flag_vertex_datum *flag_datum_get_vertex(
	struct flag_datum_prefix *flag,
	struct flag_definition *definition,
	short x,
	short y)
{
	match_assert(
		"c:\\halo\\SOURCE\\objects\\widgets\\flags.c",
		96,
		flag && definition);
	match_assert(
		"c:\\halo\\SOURCE\\objects\\widgets\\flags.c",
		97,
		x>=0 && x<definition->width);
	match_assert(
		"c:\\halo\\SOURCE\\objects\\widgets\\flags.c",
		98,
		y>=0 && y<definition->height);

	return &flag->vertices[x * definition->height + y];
}

struct flag_cell_datum *flag_datum_get_cell(
	struct flag_datum_prefix *flag,
	struct flag_definition *definition,
	short x,
	short y)
{
	match_assert(
		"c:\\halo\\SOURCE\\objects\\widgets\\flags.c",
		109,
		flag && definition);
	match_assert(
		"c:\\halo\\SOURCE\\objects\\widgets\\flags.c",
		110,
		x>=0 && x<definition->width-1);
	match_assert(
		"c:\\halo\\SOURCE\\objects\\widgets\\flags.c",
		111,
		y>=0 && y<definition->height-1);

	return &flag->cells[x * (definition->height - 1) + y];
}

void flag_render_proper(
	struct flag_datum_prefix *flag,
	struct flag_definition *definition,
	struct render_lighting const *lighting,
	struct render_animation const *animation)
{
	long shader_index;
	short maximum_triangles;
	long triangle_buffer_index;
	long vertex_buffer_index;

	if (object_get(flag->object_index)->object.owner_team_index != _team_red)
		shader_index = definition->shader_blue.index;
	else
		shader_index = definition->shader_red.index;
	if (shader_index == NONE)
		shader_index = definition->shader_blue.index;

	rasterizer_globals.current_lock_operation = _rasterizer_lock_flag;
	maximum_triangles = (2 * definition->height - 2) * (definition->width - 1);
	triangle_buffer_index = rasterizer_dynamic_triangles_new(maximum_triangles);
	vertex_buffer_index = rasterizer_dynamic_vertices_new(
		_rasterizer_vertex_type_model_compressed,
		definition->width * definition->height);
	if (triangle_buffer_index != NONE && vertex_buffer_index != NONE)
	{
		struct model_vertex_compressed *vertices;
		short *triangles;
		real one_over_width;
		real one_over_height;
		struct model_vertex_uncompressed vertex;
		struct rasterizer_model_begin_parameters parameters;
		struct shader *shader;
		struct flag_vertex_datum *corner00;
		struct flag_vertex_datum *corner10;
		struct flag_vertex_datum *corner01;
		struct flag_vertex_datum *corner11;
		short vertex_index;
		short i;
		short x;
		short y;
		/* port: the cloth hangs from its first attachment marker where the
		frame that stepped it drew it (flags_update, on the first frame after
		a tick; game_frame reads the drawn pose, render_interpolation.c), and
		later frames draw its object further on: drawn as far from there as
		that marker is now, so that it stays on a running carrier's pole */
		real_vector3d drawn_offset;

		drawn_offset.i = drawn_offset.j = drawn_offset.k = 0.0f;
		if (definition->attachment_points.count > 0)
		{
			struct object_marker marker;

			if (object_get_marker_by_name(
				flag->object_index,
				TAG_BLOCK_GET_ELEMENT(&definition->attachment_points, 0, struct flag_attachment_point)->marker_name,
				&marker,
				1) > 0)
			{
				drawn_offset.i = marker.matrix.position.x - flag->first_attachment.x;
				drawn_offset.j = marker.matrix.position.y - flag->first_attachment.y;
				drawn_offset.k = marker.matrix.position.z - flag->first_attachment.z;
			}
		}

		object_definition_get(object_get(flag->object_index)->definition_index);
		vertices = rasterizer_dynamic_vertices_lock(vertex_buffer_index);
		triangles = rasterizer_dynamic_triangles_lock(triangle_buffer_index);
		one_over_width = 1.0f / (definition->width - 1);
		one_over_height = 1.0f / (definition->height - 1);
		csmemset(&vertex, 0, sizeof(vertex));

		for (x = 0, vertex_index = 0; x < definition->width; x++)
		{
			for (y = 0; y < definition->height; y++, vertex_index++)
			{
				struct flag_vertex_datum *point = flag_datum_get_vertex(flag, definition, x, y);
				short x0 = x < definition->width - 1 ? x : x - 1;
				short y0 = y < definition->height - 1 ? y : y - 1;
				struct flag_vertex_datum *origin = flag_datum_get_vertex(flag, definition, x0, y0);
				struct flag_vertex_datum *across = flag_datum_get_vertex(flag, definition, x0 + 1, y0);
				struct flag_vertex_datum *down = flag_datum_get_vertex(flag, definition, x0, y0 + 1);
				real_vector3d horizontal;
				real_vector3d vertical;

				vector_from_points3d(&origin->position, &across->position, &horizontal);
				vector_from_points3d(&origin->position, &down->position, &vertical);
				cross_product3d(&horizontal, &vertical, &vertex.normal);
				normalize3d(&vertex.normal);

				vertex.position = point->position;
				/* port: (drawn_offset, above) */
				vertex.position.x += drawn_offset.i;
				vertex.position.y += drawn_offset.j;
				vertex.position.z += drawn_offset.k;
				vertex.texcoord.x = x * one_over_width;
				vertex.texcoord.y = y * one_over_height;
				rasterizer_geometry_compress_vertices(
					_rasterizer_vertex_type_model_uncompressed,
					1,
					&vertices[vertex_index],
					sizeof(struct model_vertex_compressed),
					&vertex,
					sizeof(struct model_vertex_uncompressed));
			}
		}

		for (x = 0, i = 0; x < definition->width - 1; x++)
		{
			for (y = 0; y < definition->height - 1; y++)
			{
				switch (flag_datum_get_cell(flag, definition, x, y)->tesselation)
				{
					case _tesselate_both:
						match_assert(
							"c:\\halo\\SOURCE\\objects\\widgets\\flags.c",
							742,
							i<maximum_triangles);
						triangles[3 * i + 0] = definition->height * x + y;
						triangles[3 * i + 1] = definition->height * (x + 1) + y;
						triangles[3 * i + 2] = definition->height * x + y + 1;
						i++;
						match_assert(
							"c:\\halo\\SOURCE\\objects\\widgets\\flags.c",
							747,
							i<maximum_triangles);
						triangles[3 * i + 0] = definition->height * x + y + 1;
						triangles[3 * i + 1] = definition->height * (x + 1) + y;
						triangles[3 * i + 2] = definition->height * (x + 1) + y + 1;
						i++;
						break;

					case _tesselate_none:
						break;

					case _tesselate_top_left:
						match_assert(
							"c:\\halo\\SOURCE\\objects\\widgets\\flags.c",
							754,
							i<maximum_triangles);
						triangles[3 * i + 0] = definition->height * x + y;
						triangles[3 * i + 1] = definition->height * x + y + 1;
						triangles[3 * i + 2] = definition->height * (x + 1) + y;
						i++;
						break;

					case _tesselate_bottom_left:
						match_assert(
							"c:\\halo\\SOURCE\\objects\\widgets\\flags.c",
							761,
							i<maximum_triangles);
						triangles[3 * i + 0] = definition->height * x + y;
						triangles[3 * i + 1] = definition->height * x + y + 1;
						triangles[3 * i + 2] = definition->height * (x + 1) + y + 1;
						i++;
						break;

					case _tesselate_top_right:
						match_assert(
							"c:\\halo\\SOURCE\\objects\\widgets\\flags.c",
							768,
							i<maximum_triangles);
						triangles[3 * i + 0] = definition->height * x + y;
						triangles[3 * i + 1] = definition->height * (x + 1) + y + 1;
						triangles[3 * i + 2] = definition->height * (x + 1) + y;
						i++;
						break;

					case _tesselate_bottom_right:
						match_assert(
							"c:\\halo\\SOURCE\\objects\\widgets\\flags.c",
							775,
							i<maximum_triangles);
						triangles[3 * i + 0] = definition->height * x + y + 1;
						triangles[3 * i + 1] = definition->height * (x + 1) + y + 1;
						triangles[3 * i + 2] = definition->height * (x + 1) + y;
						i++;
						break;

					default:
						match_vassert(
							"c:\\halo\\SOURCE\\objects\\widgets\\flags.c",
							784,
							FALSE,
							NULL);
						break;
				}
			}
		}

		rasterizer_dynamic_triangles_unlock(triangle_buffer_index);
		rasterizer_dynamic_vertices_unlock(vertex_buffer_index);
		shader = shader_definition_get(shader_index);

		{
			real_point3d centroid;

			corner00 = flag_datum_get_vertex(flag, definition, 0, 0);
			corner10 = flag_datum_get_vertex(flag, definition, definition->width - 1, 0);
			corner01 = flag_datum_get_vertex(flag, definition, 0, definition->height - 1);
			corner11 = flag_datum_get_vertex(
				flag,
				definition,
				definition->width - 1,
				definition->height - 1);
			centroid.x = (corner11->position.x + corner01->position.x + corner10->position.x +
				corner00->position.x) * 0.25f;
			centroid.y = (corner11->position.y + corner01->position.y + corner10->position.y +
				corner00->position.y) * 0.25f;
			centroid.z = (corner11->position.z + corner01->position.z + corner10->position.z +
				corner00->position.z) * 0.25f;
			/* port: (drawn_offset, above) */
			centroid.x += drawn_offset.i;
			centroid.y += drawn_offset.j;
			centroid.z += drawn_offset.k;

			csmemset(&parameters, 0, sizeof(parameters));
			parameters.unique_identifier = 1;
			parameters.skinning.node_matrices = global_identity4x3;
			parameters.skinning.node_matrix_count = 1;
			parameters.lighting = *lighting;
			parameters.animation = *animation;
			parameters.centroid = centroid;
			parameters.base_map_scale.i = parameters.base_map_scale.j = 1.0f;

			rasterizer_profile_enable(FALSE);
			rasterizer_models_begin(FALSE);
			rasterizer_model_begin(&parameters, FALSE);
			if (shader_type_is_transparent(shader->base.type))
			{
				rasterizer_model_transparent_geometry_submit(
					shader,
					0,
					NULL,
					triangle_buffer_index,
					i,
					NULL,
					vertex_buffer_index,
					&centroid,
					NULL);
			}
			else
			{
				rasterizer_model_draw(
					shader,
					0,
					NULL,
					triangle_buffer_index,
					i,
					NULL,
					vertex_buffer_index);
			}
			rasterizer_model_end();
			rasterizer_models_end();
			rasterizer_profile_enable(TRUE);
		}
	}

	rasterizer_dynamic_triangles_delete(triangle_buffer_index);
	rasterizer_dynamic_vertices_delete(vertex_buffer_index);
	rasterizer_globals.current_lock_operation = _rasterizer_lock_none;

	return;
}

void flag_tesselate_region(
	struct flag_definition *definition,
	struct flag_datum_prefix *flag,
	short x,
	short y,
	short size,
	short tesselation)
{
	short i;
	short j;

	for (i = x; i < x + size; i++)
	{
		for (j = y; j < y + size; j++)
		{
			struct flag_cell_datum *cell;
			short horizontal;
			short vertical;

			if (i < 0 || j < 0 ||
				i >= definition->width - 1 ||
				j >= definition->height - 1)
			{
				continue;
			}

			horizontal = (tesselation == _tesselate_top_right || tesselation == _tesselate_bottom_right) ?
				i - x : size - i + x - 1;
			vertical = (tesselation == _tesselate_top_right || tesselation == _tesselate_top_left) ?
				j - y : size - j + y - 1;

			cell = flag_datum_get_cell(flag, definition, i, j);
			if (horizontal == vertical)
				cell->tesselation = tesselation;
			else
				cell->tesselation = horizontal <= vertical ? _tesselate_none : _tesselate_both;
		}
	}

	return;
}

void flag_set_attachment_shape(
	struct flag_definition *definition,
	struct flag_datum_prefix *flag)
{
	short point_index;
	short y = 0;

	if (definition->attached_edge_shape == _attached_edge_shape_flat)
		return;

	for (point_index = 0;
		point_index < definition->attachment_points.count;
		point_index++)
	{
		struct flag_attachment_point *point;
		short span;
		short half_span;

		if (y >= definition->height)
			break;

		point = TAG_BLOCK_GET_ELEMENT(
			&definition->attachment_points,
			point_index,
			struct flag_attachment_point);
		span = PIN(point->height_to_next_attachment, 0, definition->height - y) & ~1;
		half_span = span >> 1;

		flag_tesselate_region(
			definition, flag, 0, y, half_span, _tesselate_top_right);
		flag_tesselate_region(
			definition, flag, 0, y + half_span, half_span, _tesselate_bottom_right);

		y += span;
	}

	return;
}

void flag_set_trailing_shape(
	struct flag_definition *definition,
	struct flag_datum_prefix *flag)
{
	short trailing_edge_shape;
	short split;
	short x;

	trailing_edge_shape = definition->trailing_edge_shape;
	if (trailing_edge_shape == _trailing_edge_shape_flat)
		return;

	if (trailing_edge_shape == _trailing_edge_shape_trapezoid_short_top ||
		trailing_edge_shape == _trailing_edge_shape_trapezoid_short_bottom)
	{
		split = definition->height - 1;
	}
	else
	{
		split = definition->height >> 1;
	}

	x = MAX(
		0,
		definition->width + definition->trailing_edge_offset - split - 1);

	if (trailing_edge_shape == _trailing_edge_shape_trapezoid_short_top)
	{
		flag_tesselate_region(
			definition, flag, x, 0, split, _tesselate_bottom_left);
		return;
	}

	if (trailing_edge_shape == _trailing_edge_shape_trapezoid_short_bottom)
	{
		flag_tesselate_region(
			definition, flag, x, 0, split, _tesselate_top_left);
		return;
	}

	if (trailing_edge_shape == _trailing_edge_shape_concave_triangular)
	{
		flag_tesselate_region(
			definition, flag, x, 0, split, _tesselate_top_left);
		flag_tesselate_region(
			definition, flag, x, split, split, _tesselate_bottom_left);
		return;
	}

	if (trailing_edge_shape == _trailing_edge_shape_convex_triangular)
	{
		flag_tesselate_region(
			definition, flag, x, 0, split, _tesselate_bottom_left);
		flag_tesselate_region(
			definition, flag, x, split, split, _tesselate_top_left);
	}

	return;
}
