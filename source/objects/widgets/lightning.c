/*
LIGHTNING.C

symbols in this file:
00124B70 0040:
	_lightnings_initialize (0000)
00124BB0 0010:
	_lightnings_dispose (0000)
00124BC0 0020:
	_lightnings_initialize_for_new_map (0000)
00124BE0 0020:
	_lightnings_dispose_from_old_map (0000)
00124C00 0040:
	_lightning_new (0000)
00124C40 0020:
	_lightning_delete (0000)
00124C60 0010:
	_lightning_render (0000)
00124C70 00f0:
	_lightning_offset_marker_position (0000)
00124D60 0a10:
	_lightning_submit (0000)
00289208 0021:
	??_C@_0CB@BNDAIAGI@lightning_globals?4lightning_data@ (0000)
0028922C 002b:
	??_C@_0CL@PEKNBFKF@c?3?2halo?2SOURCE?2objects?2widgets?2l@ (0000)
00289258 000b:
	??_C@_0L@PEMAMKLL@lightnings?$AA@ (0000)
00289264 0017:
	??_C@_0BH@PECODGGH@random_position_bounds?$AA@ (0000)
0028927C 001b:
	??_C@_0BL@JLLGKOIH@points?$FLvertex_index?$FN?4valid?$AA@ (0000)
00289298 002f:
	??_C@_0CP@DDEILNHB@?$CBpoints?$FLpoint_count?$CLsegment_poin@ (0000)
002892C8 001c:
	??_C@_0BM@GBNGDLFN@segment_point_start_index?$DO0?$AA@ (0000)
002892E4 002f:
	??_C@_0CP@ODBOIENA@?$CBpoints?$FLpoint_count?$CLsegment_poin@ (0000)
00456E44 0004:
	_lightning_globals (0000)
*/

/* ---------- headers */

#include "objects/widgets/lightning.h"

#include "bitmaps/bitmap_group.h"
#include "cache/texture_cache.h"
#include "cseries/cseries.h"
#include "cseries/errors.h"
#include "game/game.h" /* port: game_time_get (lightning_submit) */
#include "interface/hud_draw.h"
#include "bitmaps/bitmap_color_conversion.h"
#include "math/real_math.h"
#include "objects/objects.h"
#include "rasterizer/rasterizer.h"
#include "rasterizer/rasterizer_geometry.h"
#include "render/render.h"
#include "saved games/game_state.h"
#include "shaders/shader_definitions.h"
#include "shaders/shaders.h"
#include "tag_files/tag_groups.h"

/* ---------- constants */

enum
{
	LIGHTNING_TAG = 'elec',
	MAXIMUM_LIGHTNING_POINTS = 4097,
	/* port: the lightnings (lightnings_initialize's 256) and the windows
	(split screen) a bolt's seed is kept for (lightning_submit) */
	LIGHTNING_TICK_SEED_LIGHTNINGS = 256,
	LIGHTNING_TICK_SEED_WINDOWS = 4,
};

enum
{
	_lightning_marker_not_connected_to_next_marker_bit = 0,
};

/* ---------- macros */

#define lightning_definition_get(definition_index) \
	((struct lightning_definition *)tag_get(LIGHTNING_TAG, (definition_index)))

/* ---------- structures */

struct lightning_marker_definition
{
	char attachment_marker[32];
	word flags;
	short type;
	short octaves_to_next_marker;
	word pad26;
	byte reserved28[0x4C];
	real_vector3d random_position_bounds;
	real random_jitter_offset;
	real thickness;
	real_argb_color tint;
	byte reserved98[0x4C];
};

struct lightning_definition
{
	word flags;
	short count;
	byte reserved04[0x10];
	real near_fade_distance;
	real far_fade_distance;
	byte reserved1C[0x10];
	short jitter_scale_source;
	short thickness_scale_source;
	short tint_modulation_source;
	short brightness_scale_source;
	struct tag_reference map;
	byte reserved44[0x54];
	struct tag_block markers;
	struct tag_block shaders;
	byte reservedB0[0x58];
};

struct intermediate_lightning_point
{
	real_point3d position;
	real width;
	real_argb_color color;
	boolean valid;
	byte pad21[3];
};

struct lightning_vertex
{
	real_point3d point;
	real_point2d texture;
	pixel32 color;
};

typedef char lightning_marker_definition_size_assert[
	sizeof(struct lightning_marker_definition) == 0xE4 ? 1 : -1];
typedef char lightning_definition_size_assert[
	sizeof(struct lightning_definition) == 0x108 ? 1 : -1];
typedef char intermediate_lightning_point_size_assert[
	sizeof(struct intermediate_lightning_point) == 0x24 ? 1 : -1];
typedef char lightning_vertex_size_assert[
	sizeof(struct lightning_vertex) == 0x18 ? 1 : -1];
typedef char lightning_marker_definition_random_position_bounds_offset_assert[
	offsetof(struct lightning_marker_definition, random_position_bounds) == 0x74 ? 1 : -1];
typedef char lightning_marker_definition_thickness_offset_assert[
	offsetof(struct lightning_marker_definition, thickness) == 0x84 ? 1 : -1];
typedef char lightning_definition_jitter_scale_source_offset_assert[
	offsetof(struct lightning_definition, jitter_scale_source) == 0x2C ? 1 : -1];
typedef char lightning_definition_map_offset_assert[
	offsetof(struct lightning_definition, map) == 0x34 ? 1 : -1];
typedef char lightning_definition_markers_offset_assert[
	offsetof(struct lightning_definition, markers) == 0x98 ? 1 : -1];
typedef char lightning_definition_shaders_offset_assert[
	offsetof(struct lightning_definition, shaders) == 0xA4 ? 1 : -1];
typedef char intermediate_lightning_point_valid_offset_assert[
	offsetof(struct intermediate_lightning_point, valid) == 0x20 ? 1 : -1];

/* ---------- prototypes */

static void lightning_offset_marker_position(
	real_point3d *position,
	real_matrix4x3 const *matrix,
	real_vector3d const *random_position_bounds);

/* ---------- globals */

static struct lightning_globals lightning_globals = {0};

/* port: the local random seed each lightning's bolt in each window was first
drawn from in a tick, and the tick plus one, 0 for none (lightning_submit) */
static struct
{
	long tick;
	unsigned long seed;
} lightning_tick_seeds[LIGHTNING_TICK_SEED_LIGHTNINGS][LIGHTNING_TICK_SEED_WINDOWS];

/* ---------- public code */

void lightnings_initialize(
	void)
{
	lightning_globals.lightning_data = game_state_data_new("lightnings", 256, 8);
	match_assert("c:\\halo\\SOURCE\\objects\\widgets\\lightning.c", 50, lightning_globals.lightning_data);

	return;
}

void lightnings_dispose(
	void)
{
	return;
}

void lightnings_initialize_for_new_map(
	void)
{
	if (lightning_globals.lightning_data)
		data_make_valid(lightning_globals.lightning_data);
	/* port: (lightning_submit) */
	csmemset(lightning_tick_seeds, 0, sizeof(lightning_tick_seeds));

	return;
}

void lightnings_dispose_from_old_map(
	void)
{
	if (lightning_globals.lightning_data)
		data_make_invalid(lightning_globals.lightning_data);

	return;
}

long lightning_new(
	long definition_index)
{
	long lightning_index = datum_new(lightning_globals.lightning_data);

	if (lightning_index != NONE)
	{
		lightning_get(lightning_index)->definition_index = definition_index;
		/* port: (lightning_submit) a new bolt, not the one before it there */
		if (DATUM_INDEX_TO_ABSOLUTE_INDEX(lightning_index) < LIGHTNING_TICK_SEED_LIGHTNINGS)
		{
			csmemset(
				lightning_tick_seeds[DATUM_INDEX_TO_ABSOLUTE_INDEX(lightning_index)],
				0,
				sizeof(lightning_tick_seeds[0]));
		}
	}

	return lightning_index;
}

void lightning_delete(
	long lightning_index)
{
	if (lightning_index != NONE)
		datum_delete(lightning_globals.lightning_data, lightning_index);

	return;
}

void lightning_render(
	void)
{
	return;
}

/* ---------- private code */

static void lightning_offset_marker_position(
	real_point3d *position,
	real_matrix4x3 const *matrix,
	real_vector3d const *random_position_bounds)
{
	real random_x;
	real random_y;
	real random_z;
	real_vector3d offset;

	match_assert("c:\\halo\\SOURCE\\objects\\widgets\\lightning.c", 116, position);
	match_assert("c:\\halo\\SOURCE\\objects\\widgets\\lightning.c", 117, matrix);
	match_assert("c:\\halo\\SOURCE\\objects\\widgets\\lightning.c", 118, random_position_bounds);

	random_z = real_seed_random(get_global_local_random_seed_address());
	random_y = real_seed_random(get_global_local_random_seed_address());
	random_x = real_seed_random(get_global_local_random_seed_address());

	set_real_vector3d(
		&offset,
		(2.f*random_x - 1.f)*random_position_bounds->i,
		(2.f*random_y - 1.f)*random_position_bounds->j,
		(2.f*random_z - 1.f)*random_position_bounds->k);
	matrix4x3_transform_vector(matrix, &offset, &offset);

	position->x += offset.i;
	position->y += offset.j;
	position->z += offset.k;

	return;
}

/* ---------- public code */

void lightning_submit(
	long object_index,
	long widget_index,
	struct render_lighting const *lighting,
	struct render_animation const *animation)
{
	(void)lighting;
	if (object_index != NONE && widget_index != NONE)
	{
		struct lightning_datum *lightning = lightning_get(widget_index);
		struct lightning_definition *definition = lightning_definition_get(lightning->definition_index);
		struct object_marker marker;
		/* port: a bolt's shape a tick. The Xbox drew a frame a tick, so a
		bolt's random offsets (its markers', its points' jitter, its
		texture's) were drawn anew 30 times a second. The native builds draw
		every frame (port/linux/game/render_interpolation.c), and a bolt took
		a new shape each frame, flickering several times as fast at high
		frame rates. The first frame of a tick to draw a lightning in a
		window keeps the local random seed it started from; the frames after
		it until the next tick draw the bolt from that seed, the same shape
		between its markers where they are drawn then, and leave the seed as
		they found it. */
		unsigned long *local_seed = get_global_local_random_seed_address();
		unsigned long frame_seed = *local_seed;
		long seed_lightning = DATUM_INDEX_TO_ABSOLUTE_INDEX(widget_index);
		short seed_window = render.window_index;
		boolean tick_drawn = FALSE;

		if (seed_lightning < LIGHTNING_TICK_SEED_LIGHTNINGS &&
			seed_window >= 0 && seed_window < LIGHTNING_TICK_SEED_WINDOWS)
		{
			if (lightning_tick_seeds[seed_lightning][seed_window].tick == game_time_get() + 1)
			{
				*local_seed = lightning_tick_seeds[seed_lightning][seed_window].seed;
				tick_drawn = TRUE;
			}
			else
			{
				lightning_tick_seeds[seed_lightning][seed_window].tick = game_time_get() + 1;
				lightning_tick_seeds[seed_lightning][seed_window].seed = frame_seed;
			}
		}

		if (definition->markers.count > 0 &&
			object_get_marker_by_name(
				object_index,
				TAG_BLOCK_GET_ELEMENT(&definition->markers, 0, struct lightning_marker_definition)->attachment_marker,
				&marker,
				1) > 0)
		{
			struct bitmap_data *bitmap = TAG_BLOCK_GET_ELEMENT(
				&bitmap_group_get(definition->map.index)->bitmaps,
				0,
				struct bitmap_data);

			if (_texture_cache_bitmap_get_hardware_format(bitmap, FALSE, TRUE))
			{
				struct intermediate_lightning_point points[MAXIMUM_LIGHTNING_POINTS];
				short instance_index;

				for (instance_index = 0; instance_index < definition->count; instance_index++)
				{
					short point_count = 0;
					real jitter_scale = 1.f;
					boolean first_marker = TRUE;
					short marker_index;
					/* port: no more markers than the short counter reaches (a
					map's count; retail has up to 12) */
					short marker_count = (short)MIN(definition->markers.count, SHORT_MAX);

					if (animation)
					{
						if (animation->values)
						{
							short source = definition->jitter_scale_source;
							if (source >= _object_function_reference_a && source <= _object_function_reference_d)
								jitter_scale = animation->values[source - 1];
						}
					}

					for (marker_index = 0; marker_index < marker_count; marker_index++)
					{
						struct lightning_marker_definition *marker_definition = TAG_BLOCK_GET_ELEMENT(
							&definition->markers,
							marker_index,
							struct lightning_marker_definition);

						if (first_marker)
						{
							point_count = 0;
							csmemset(points, 0, sizeof(points));
							object_get_marker_by_name(object_index, marker_definition->attachment_marker, &marker, 1);
							points[0].position = marker.matrix.position;
							lightning_offset_marker_position(
								&points[0].position,
								&marker.matrix,
								&marker_definition->random_position_bounds);
							points[0].width = marker_definition->thickness;
							points[0].color = marker_definition->tint;
							points[0].valid = TRUE;
							first_marker = FALSE;
						}

						if (TEST_FLAG(marker_definition->flags, _lightning_marker_not_connected_to_next_marker_bit) ||
							marker_index == marker_count - 1)
						{
							rasterizer_globals.current_lock_operation = _rasterizer_lock_lightning;
							if (point_count > 2)
							{
								long vertex_buffer_index;

								point_count++;
								vertex_buffer_index = rasterizer_dynamic_vertices_new(
									_rasterizer_vertex_type_dynamic_unlit,
									2*point_count);
								if (vertex_buffer_index != NONE)
								{
									struct lightning_vertex *vertices = rasterizer_dynamic_vertices_lock(vertex_buffer_index);
									real one_over_point_count = 1.f/point_count;
									real u_offset = real_seed_random(get_global_local_random_seed_address());
									real thickness_scale = 1.f;
									real_rgb_color const *tint = global_real_rgb_white;
									real brightness_scale = 1.f;
									real_rectangle3d bounds;
									real_point3d centroid;
									struct shader const *shader;
									short vertex_index;

									match_assert("c:\\halo\\SOURCE\\objects\\widgets\\lightning.c", 239, vertices);

									if (animation)
									{
										if (animation->values)
										{
											short source = definition->thickness_scale_source;
											if (source >= _object_function_reference_a && source <= _object_function_reference_d)
												thickness_scale = animation->values[source - 1];
										}
										if (animation->colors)
										{
											short source = definition->tint_modulation_source;
											if (source >= _object_function_reference_a && source <= _object_function_reference_d)
												tint = &animation->colors[source - 1];
										}
										if (animation->values)
										{
											short source = definition->brightness_scale_source;
											if (source >= _object_function_reference_a && source <= _object_function_reference_d)
												brightness_scale = animation->values[source - 1];
										}
									}

									for (vertex_index = 0; vertex_index < point_count; vertex_index++)
									{
										real_point3d *position = &points[vertex_index].position;
										real_point3d *previous_position = vertex_index > 0 ? &points[vertex_index - 1].position : position;
										real_point3d *next_position = vertex_index < point_count - 1 ? &points[vertex_index + 1].position : position;
										real width = points[vertex_index].width*thickness_scale;
										real u = vertex_index*one_over_point_count;
										real_vector3d up;
										real_argb_color color;
										pixel32 pixel;

										match_assert("c:\\halo\\SOURCE\\objects\\widgets\\lightning.c", 280, points[vertex_index].valid);

										vector_from_points3d(previous_position, next_position, &up);
										cross_product3d(&up, &render.camera.forward, &up);
										fast_normalize3d(&up);

										color.alpha = points[vertex_index].color.alpha*brightness_scale;
										color.red = points[vertex_index].color.red*tint->red;
										color.green = points[vertex_index].color.green*tint->green;
										color.blue = points[vertex_index].color.blue*tint->blue;
										pixel = real_argb_color_to_pixel32(&color);

										vertices->point.x = position->x + up.i*width;
										vertices->point.y = position->y + up.j*width;
										vertices->point.z = position->z + up.k*width;
										vertices->texture.x = u + u_offset;
										vertices->color = pixel;
										vertices->texture.y = 0.f;
										vertices++;

										width = -width;
										vertices->point.x = position->x + up.i*width;
										vertices->point.y = position->y + up.j*width;
										vertices->point.z = position->z + up.k*width;
										vertices->texture.x = u + u_offset;
										vertices->color = pixel;
										vertices->texture.y = 1.f;
										vertices++;

										if (vertex_index == 0)
										{
											bounds.x0 = bounds.x1 = position->x;
											bounds.y0 = bounds.y1 = position->y;
											bounds.z0 = bounds.z1 = position->z;
										}
										else
										{
											bounds.x0 = MIN(position->x, bounds.x0);
											bounds.y0 = MIN(position->y, bounds.y0);
											bounds.z0 = MIN(position->z, bounds.z0);
											bounds.x1 = MAX(position->x, bounds.x1);
											bounds.y1 = MAX(position->y, bounds.y1);
											bounds.z1 = MAX(position->z, bounds.z1);
										}
									}

									centroid.x = (bounds.x0 + bounds.x1)*0.5f;
									centroid.y = (bounds.y0 + bounds.y1)*0.5f;
									centroid.z = (bounds.z0 + bounds.z1)*0.5f;

									if (definition->shaders.count > 0)
										shader = &TAG_BLOCK_GET_ELEMENT(&definition->shaders, 0, struct shader_effect_definition)->shader;
									else
										shader = &global_shader_effect_additive.shader;

									rasterizer_dynamic_vertices_unlock(vertex_buffer_index);
									rasterizer_dynamic_unlit_geometry_draw(
										shader,
										bitmap,
										animation,
										-2*point_count,
										vertex_buffer_index,
										2*point_count - 2,
										&centroid,
										0);
									rasterizer_dynamic_vertices_delete(vertex_buffer_index);
								}
								first_marker = TRUE;
							}
							rasterizer_globals.current_lock_operation = _rasterizer_lock_none;
						}
						/* port: a segment whose points fit after the ones so far (a
						map's octaves; past 12 of them, or past the bolt's 4097
						points, they were written past points; retail's bolts have
						up to 3 octaves and 54 points). One that doesn't is left
						out, and the bolt goes on from the point it reached */
						else if (marker_definition->octaves_to_next_marker < 0 ||
							marker_definition->octaves_to_next_marker >= SHORT_BITS-1 ||
							point_count + (1L << marker_definition->octaves_to_next_marker) >= MAXIMUM_LIGHTNING_POINTS)
						{
							static boolean octaves_reported = FALSE;

							if (!octaves_reported)
							{
								octaves_reported = TRUE;
								error(
									_error_silent,
									"### ERROR lightning %s has a marker %d octaves to the next; that segment isn't drawn",
									tag_get_name(lightning->definition_index),
									marker_definition->octaves_to_next_marker);
							}
						}
						else
						{
							struct lightning_marker_definition *next_marker_definition = TAG_BLOCK_GET_ELEMENT(
								&definition->markers,
								marker_index + 1,
								struct lightning_marker_definition);
							short segment_point_count = 1 << marker_definition->octaves_to_next_marker;
							short octaves = marker_definition->octaves_to_next_marker;
							real scale = 1.f;
							real_vector3d up;
							short octave_index;

							match_assert("c:\\halo\\SOURCE\\objects\\widgets\\lightning.c", 383, !points[point_count+segment_point_count].valid);

							object_get_marker_by_name(object_index, next_marker_definition->attachment_marker, &marker, 1);
							points[point_count+segment_point_count].position = marker.matrix.position;
							lightning_offset_marker_position(
								&points[point_count+segment_point_count].position,
								&marker.matrix,
								&next_marker_definition->random_position_bounds);
							points[point_count+segment_point_count].width = next_marker_definition->thickness;
							points[point_count+segment_point_count].color = next_marker_definition->tint;
							points[point_count+segment_point_count].valid = TRUE;

							vector_from_points3d(
								&points[point_count].position,
								&points[point_count+segment_point_count].position,
								&up);
							cross_product3d(&up, &render.camera.forward, &up);
							if (normalize3d(&up) == 0.f)
							{
								up.i = global_z_axis3d->x;
								up.j = global_z_axis3d->y;
								up.k = global_z_axis3d->z;
							}

							for (octave_index = 1; octave_index <= octaves; octave_index++)
							{
								short segment_point_start_index = 1 << (octaves - octave_index);
								short segment_point_index_increment = 2*segment_point_start_index;
								short segment_point_index;

								match_assert("c:\\halo\\SOURCE\\objects\\widgets\\lightning.c", 408, segment_point_start_index>0);

								for (segment_point_index = segment_point_start_index;
									segment_point_index < segment_point_count;
									segment_point_index += segment_point_index_increment)
								{
									struct intermediate_lightning_point *previous_point =
										&points[point_count + segment_point_index - segment_point_start_index];
									struct intermediate_lightning_point *next_point =
										&points[point_count + segment_point_index + segment_point_start_index];
									struct intermediate_lightning_point *point = &points[point_count + segment_point_index];
									real segment_fraction = (real)segment_point_index/segment_point_count;
									real jitter_offset =
										(segment_fraction*
											(next_marker_definition->random_jitter_offset - marker_definition->random_jitter_offset) +
											marker_definition->random_jitter_offset)*scale*jitter_scale;
									real jitter;
									real_vector3d offset;

									match_assert("c:\\halo\\SOURCE\\objects\\widgets\\lightning.c", 426, !points[point_count+segment_point_index].valid);

									jitter = real_seed_random_range(
										get_global_local_random_seed_address(),
										-1.f,
										1.f)*jitter_offset;
									scale_vector3d(&up, jitter, &offset);
									set_real_point3d(
										&point->position,
										(previous_point->position.x + next_point->position.x)*0.5f + offset.i,
										(previous_point->position.y + next_point->position.y)*0.5f + offset.j,
										(previous_point->position.z + next_point->position.z)*0.5f + offset.k);
									point->width = (previous_point->width + next_point->width)*0.5f;
									point->color.alpha = (previous_point->color.alpha + next_point->color.alpha)*0.5f;
									point->color.red = (previous_point->color.red + next_point->color.red)*0.5f;
									point->color.green = (previous_point->color.green + next_point->color.green)*0.5f;
									point->color.blue = (previous_point->color.blue + next_point->color.blue)*0.5f;
									point->valid = TRUE;
								}

								scale *= 0.5f;
							}

							point_count += 1 << marker_definition->octaves_to_next_marker;
						}
					}
				}
			}
		}

		/* port: (above) */
		if (tick_drawn)
			*local_seed = frame_seed;
	}

	return;
}
