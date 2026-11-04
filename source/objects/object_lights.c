/*
OBJECT_LIGHTS.C

symbols in this file:
00128730 0030:
	_texture_cache_bitmap_load (0000)
00128760 0020:
	_shade_scalar (0000)
00128780 0040:
	_shade_vector2d (0000)
001287C0 0060:
	_shade_vector3d (0000)
00128820 0100:
	_sample_lightmap (0000)
00128920 00f0:
	_sample_diffuse_texture (0000)
00128A10 0020:
	_should_render_lights (0000)
00128A30 00c0:
	_lights_initialize (0000)
00128AF0 0010:
	_lights_dispose (0000)
00128B00 0030:
	_lights_initialize_for_new_map (0000)
00128B30 0020:
	_lights_dispose_from_old_map (0000)
00128B50 0010:
	_lights_enable (0000)
00128B60 0040:
	_light_delete (0000)
00128BA0 0060:
	_light_build_cluster_array (0000)
00128C00 00d0:
	_object_get_self_illumination (0000)
00128CD0 0260:
	_light_particle (0000)
00128F30 0060:
	_light_disconnect_from_map (0000)
00128F90 00b0:
	_lights_disconnect_from_structure_bsp (0000)
00129040 0020:
	_light_attenuation (0000)
00129060 00a0:
	_brighten_real_rgb_color (0000)
00129100 0020:
	_cluster_get_first_light (0000)
00129120 0020:
	_cluster_get_next_light (0000)
00129140 0040:
	_light_marker_begin (0000)
00129180 0060:
	_light_unmarked (0000)
001291E0 0060:
	_light_mark (0000)
00129240 0040:
	_light_marker_end (0000)
00129280 0110:
	_render_debug_light (0000)
00129390 00e0:
	_lights_queue_lens_flare (0000)
00129470 0230:
	_find_point_lights_for_object_in_cluster (0000)
001296A0 0400:
	_build_distant_lights (0000)
00129AA0 00f0:
	_light_compute_bounding_sphere (0000)
00129B90 00e0:
	_light_get_bounding_sphere (0000)
00129C70 01d0:
	_lights_render_diffuse (0000)
00129E40 0150:
	_lights_render_specular (0000)
00129F90 02d0:
	_lights_illumination_at_point (0000)
0012A260 0110:
	_lights_prepare_for_object_dynamic (0000)
0012A370 03b0:
	_lights_distant_lighting_at_point (0000)
0012A720 0280:
	_light_reconnect_to_map (0000)
0012A9A0 0060:
	_lights_reconnect_to_structure_bsp (0000)
0012AA00 00e0:
	_light_new (0000)
0012AAE0 00f0:
	_light_new_unattached (0000)
0012ABD0 0960:
	_lights_preprocess_scene (0000)
0012B530 0340:
	_lights_prepare_for_object_static (0000)
002896C0 00b0:
	_default_object_lighting (0000)
00289770 0021:
	??_C@_0CB@NDAECIIM@c?3?2halo?2source?2objects?2objects?4h@ (0000)
00289798 00b9:
	??_C@_0LJ@HECDDFBP@material?9?$DOlightmap_vertices?4type@ (0000)
00289854 0027:
	??_C@_0CH@GHJLLMCD@c?3?2halo?2SOURCE?2objects?2object_li@ (0000)
00289880 0095:
	??_C@_0JF@BFHNCHBA@material?9?$DOvertices?4type?$DN?$DN_raster@ (0000)
00289918 002c:
	??_C@_0CM@CCFOPHPG@couldn?8t?5allocate?5memory?5for?5obj@ (0000)
00289944 0014:
	??_C@_0BE@MMBBLMKK@lights_game_globals?$AA@ (0000)
00289958 000b:
	??_C@_0L@PNIBKHKO@light_data?$AA@ (0000)
00289964 000f:
	??_C@_0P@MLOLKFEJ@lights?5globals?$AA@ (0000)
00289974 0007:
	??_C@_06IDBEMGIK@lights?$AA@ (0000)
0028997C 003b:
	??_C@_0DL@OAOFOGFN@TEST_FLAG?$CIlight?9?$DOflags?0?5_point_l@ (0000)
002899B8 0023:
	??_C@_0CD@CEANBCDI@?$CBlights_globals?4marker_initializ@ (0000)
002899DC 0022:
	??_C@_0CC@DMOOLGHL@lights_globals?4marker_initialize@ (0000)
00289A00 0004:
	__real@3f34fdf4 (0000)
00289A04 003c:
	??_C@_0DM@NOKGCMAM@?$CBTEST_FLAG?$CIlight?9?$DOflags?0?5_point_@ (0000)
00289A40 002a:
	??_C@_0CK@JMHDKADE@lens_flare_parameters?4light_iden@ (0000)
00289A6C 0035:
	??_C@_0DF@MMLEIBDB@light?9?$DOcolor?4blue?5?$DO?$DN0?40f?5?$CG?$CG?5ligh@ (0000)
00289AA4 0035:
	??_C@_0DF@KFPEGOCE@light?9?$DOcolor?4green?$DO?$DN0?40f?5?$CG?$CG?5ligh@ (0000)
00289ADC 0033:
	??_C@_0DD@NIPJINKE@light?9?$DOcolor?4red?5?$DO?$DN0?40f?5?$CG?$CG?5light@ (0000)
00289B10 0004:
	__real@bf3504f3 (0000)
0030B388 0605:
	_data_0030b388 (0000)
	_object_light_ambient_base (05f8)
	_object_light_ambient_scale (05fc)
	_object_light_secondary_scale (0600)
	_object_light_interpolate (0604)
00456E94 0004:
	_lights_game_globals (0000)
*/

/* ---------- headers */

/* objects.h lies in this unit's own directory, so January's __FILE__ for it was the absolute
   path (see OBJECTS_H_FILE in objects.h) */
#define OBJECTS_H_FILE "c:\\halo\\source\\objects\\objects.h"

#include "cseries/cseries.h"
#include "cseries/errors.h"
#include "cseries/profile.h"
#include "bitmaps/bitmaps.h"
#include "bitmaps/bitmap_group.h"
#include "bitmaps/bitmap_group_lookup.h"
#include "bitmaps/bitmaps_sampling.h"
#include "bitmaps/bitmap_utilities.h"
#include "effects/decals.h"
#include "math/real_math.h"
#include "cache/texture_cache.h"
#include "game/game.h"
#include "game/game_engine.h"
#include "interface/first_person_weapons.h"
#include "math/periodic_functions.h"
#include "memory/data.h"
#include "objects/light_definitions.h"
#include "objects/object_lights.h"
#include "objects/object_lights_rendering.h"
#include "objects/objects.h"
#include "render/render.h"
#include "render/render_debug.h"
#include "rasterizer/rasterizer.h"
#include "rasterizer/rasterizer_environment.h"
#include "rasterizer/rasterizer_geometry.h"
#include "rasterizer/rasterizer_geometry_environment.h"
#include "rasterizer/rasterizer_lights.h"
#include "saved games/game_state.h"
#include "scenario/scenario.h"
#include "shaders/shader_definitions.h"
#include "structures/cluster_partitions.h"
#include "structures/structure_bsp_definitions.h"
#include "structures/structure_render.h"
#include "structures/structure_render_lights.h"
#include "structures/structures.h"
#include "structures/structure_vector_tests.h"
#include "structures/structure_visibility.h"
#include "tag_files/tag_groups.h"
#include "units/units.h"
#include "cache/cache_files.h" /* port: (tag_iterator: light_port_glow_definition_get) */
void shield_glow_update(void); /* port: port/linux/game/shield_glow.c */

/* port: port/linux/src (halo.log) */
void platform_log(const char *format, ...);

/* ---------- constants */

enum
{
	_light_definition_dynamic_bit = 0,
	_light_definition_no_specular_bit,
	_light_definition_dont_light_own_object_bit,
	_light_definition_supersize_in_first_person_bit,
	_light_definition_is_first_person_flashlight_bit,
	_light_definition_dont_fade_active_camouflage_bit,
};

enum
{
	_point_light_dynamic_bit = 0,
	_point_light_connects_to_map_bit = 1,
	_point_light_connected_to_map_bit = 2,
	_point_light_attached_to_first_person_weapon_bit = 3,
	/* port: a glow the port colors and sizes itself (light_port_glow_set) */
	_point_light_port_glow_bit = 4,
};

enum
{
	MAXIMUM_CLUSTERS_PER_LIGHT = 512,
	LENS_FLARE_FIRST_PERSON_MARKER_FLAG = FLAG(7),
};

enum
{
	_distant_lighting_raycast_sideways_bit = 0,
	_distant_lighting_block_on_textures_bit,
	_distant_lighting_brighten_bit,
};

enum
{
	NUMBER_OF_LIGHTMAP_SAMPLE_SIDEWAYS_RAYCASTS = 4,
};

enum
{
	LENS_FLARE_DEFINITION_TAG = 'lens',
};

enum
{
	_shader_type_environment = 3,
};

/* ---------- macros */

#define light_get(index) \
	((struct light_datum *)datum_get(light_data, (index)))

#define light_definition_get(index) \
	((struct point_light_definition *)tag_get(LIGHT_DEFINITION_TAG, (index)))

#define lens_flare_definition_get(index) \
	((struct lens_flare_definition *)tag_get(LENS_FLARE_DEFINITION_TAG, (index)))

#define shader_environment_definition_get(shader) \
	((struct shader_environment_definition *)shader_get_and_verify_type( \
		(shader), \
		_shader_type_environment))

#define structure_material_get_vertex(material, vertex_index) \
	((struct environment_vertex_compressed const *)(material)->compressed_vertex_data.address \
		+ (vertex_index))

#define structure_material_get_lightmap_vertex(material, vertex_index) \
	((struct environment_lightmap_vertex_compressed const *) \
		((struct environment_vertex_compressed const *)(material)->compressed_vertex_data.address \
			+ (material)->vertices.count) \
		+ (vertex_index))

/* ---------- structures */

struct environment_vertex_compressed
{
	real_point3d position;
	unsigned long normal;
	unsigned long binormal;
	unsigned long tangent;
	real_point2d texcoord;
};

struct environment_lightmap_vertex_compressed
{
	unsigned long incident_radiosity;
	short lightmap_u;
	short lightmap_v;
};

struct shader_environment_definition
{
	struct shader shader;
	byte reserved28[0x60];
	struct tag_reference base_map;
};

struct lights_game_globals
{
	boolean render_lights;
	byte reserved01[3];
};

struct point_light_definition
{
	long flags;
	real radius;
	real radius_modifier_lower_bound;
	real radius_modifier_upper_bound;
	real falloff_angle;
	real cutoff_angle;
	real lens_flare_radius;
	real runtime_cosine_falloff_angle;
	real runtime_cosine_cutoff_angle;
	real specular_radius_multiplier;
	real runtime_sine_cutoff_angle;
	byte reserved2C[0x8];
	unsigned long color_interpolation_flags;
	real_argb_color color_lower_bound;
	real_argb_color color_upper_bound;
	byte reserved58[0x54];
	struct tag_reference lens_flare;
	byte reservedBC[0x38];
	real transition_duration;
	word reservedF8;
	short falloff_function;
	byte reservedFC[0x8];
};

struct light_datum
{
	struct datum_header header;
	word flags;
	long definition_index;
	long rasterizer_light_index;
	long marker;
	long cluster_reference;
	real_rgb_color color;
	byte reserved20[0xC];
	long object_index;
	real_point3d position;
	real_vector3d forward;
	real_vector3d up;
	real radius;
	long parent_light_index;
	short attachment_marker_index;
	short function_index;
	union
	{
		short color_function_index;
		struct
		{
			real_point3d relative_position;
			real_vector3d relative_forward;
		} node;
	};
	real intensity_scale;
};

struct rasterizer_lens_flare_submit_parameters
{
	struct lens_flare_definition *definition;
	real_point3d position;
	unsigned long compressed_direction;
	unsigned long compressed_up;
	unsigned long compressed_light_color;
	short light_identifier;
	short light_index;
	short lens_flare_index;
	byte compressed_window_index;
	byte compressed_light_scale;
	long internal_occlusion_pixels;
};

struct lights_globals
{
	boolean marker_initialized;
	byte pad1[3];
	long marker;
	short scene_point_light_count;
	short padA;
	/* port: the native builds' (render.h: the Xbox's 128) */
	long scene_point_lights[MAXIMUM_RENDERED_LIGHTS];
	struct rasterizer_lens_flare_submit_parameters queued_lens_flares[8];
	short queued_lens_flare_count;
	short pad34E;
};

typedef char verify_lights_game_globals_size[
	sizeof(struct lights_game_globals) == 0x4 ? 1 : -1];
typedef char verify_light_datum_flags_offset[
	offsetof(struct light_datum, flags) == 0x2 ? 1 : -1];
typedef char verify_light_datum_cluster_reference_offset[
	offsetof(struct light_datum, cluster_reference) == 0x10 ? 1 : -1];
typedef char verify_light_definition_lens_flare_offset[
	offsetof(struct point_light_definition, lens_flare) == 0xAC ? 1 : -1];
typedef char verify_light_definition_color_offset[
	offsetof(struct point_light_definition, color_interpolation_flags) == 0x34 ? 1 : -1];
typedef char verify_light_definition_transition_duration_offset[
	offsetof(struct point_light_definition, transition_duration) == 0xF4 ? 1 : -1];
typedef char verify_light_definition_falloff_function_offset[
	offsetof(struct point_light_definition, falloff_function) == 0xFA ? 1 : -1];
typedef char verify_light_datum_size[
	sizeof(struct light_datum) == 0x7C ? 1 : -1];
typedef char verify_rasterizer_light_submit_parameters_size[
	sizeof(struct rasterizer_light_submit_parameters) == 0x38 ? 1 : -1];
/* port: (scene_point_lights' size) */
typedef char verify_lights_globals_size[
	sizeof(struct lights_globals) == 0x350 + 4 * (MAXIMUM_RENDERED_LIGHTS - 128) ? 1 : -1];

/* ---------- prototypes */

static boolean should_render_lights(
	void);
static void light_marker_begin(
	void);
static void light_marker_end(
	void);
static boolean light_unmarked(
	long light_index);
static boolean light_mark(
	long light_index);
static long cluster_get_first_light(
	long *reference_index,
	short cluster_index);
static long cluster_get_next_light(
	long *reference_index);
static void find_point_lights_for_object_in_cluster(
	long object_index,
	short cluster_index,
	real_point3d const *center,
	real radius,
	long *light_indices,
	real *light_intensities,
	real *light_attenuations,
	short *light_count,
	short maximum_light_count);
static void shade_vector3d(
	real_vector3d const *vertex0,
	real_vector3d const *vertex1,
	real_vector3d const *vertex2,
	real b,
	real c,
	real_vector3d *result);
static void brighten_real_rgb_color(
	real_rgb_color *color,
	real fraction);
static void build_distant_lights(
	long flags,
	real_vector3d const *surface_normal,
	real_rgb_color const *diffuse_color,
	real_vector3d const *radiosity_normal,
	real radiosity_accuracy,
	real_rgb_color const *lightmap_color,
	struct render_lighting *lighting);
static short light_build_cluster_array(
	long light_index,
	short maximum_count,
	short *cluster_indices);
static void light_compute_bounding_sphere(
	long light_index,
	boolean maximum,
	boolean specular,
	boolean lens_flare_only,
	real_point3d *position,
	real *radius);
static void render_debug_light(
	long light_index);
static void light_get_bounding_sphere(
	long light_index,
	real_point3d *position,
	real *radius);

/* ---------- globals */

struct render_lighting const default_object_lighting =
{
	{ 0.2f, 0.2f, 0.2f },
	2,
	0,
	{
		{ { 1.0f, 1.0f, 1.0f }, { -0.577f, -0.577f, -0.577f } },
		{ { 0.4f, 0.4f, 0.5f }, { 0.0f, 0.0f, 1.0f } },
	},
	0,
	0,
	{ 0, 0 },
	{ 0.5f, 1.0f, 1.0f, 1.0f },
	{ 0.0f, 0.0f, -1.0f },
	{ 0.0f, 0.0f, 0.0f },
};

static real_vector3d const lightmap_sample_raycast_down = { 0.0f, 0.0f, -10.0f };
static real_vector3d const lightmap_sample_raycast_sideways[NUMBER_OF_LIGHTMAP_SAMPLE_SIDEWAYS_RAYCASTS] =
{
	{ -10.0f, 0.0f, 0.0f },
	{ 10.0f, 0.0f, 0.0f },
	{ 0.0f, -10.0f, 0.0f },
	{ 0.0f, 10.0f, 0.0f },
};

static struct profile_section lights_section = { "render_lights", NONE, TRUE };

real object_light_ambient_base = 0.03f;
real object_light_ambient_scale = 0.4f;
real object_light_secondary_scale = 1.0f;
boolean object_light_interpolate = TRUE;

boolean debug_lights;
boolean debug_object_lights;
struct data_array *light_data;
struct cluster_partition light_cluster_partition;

/* port: the clusters' lights kept for lighting objects (lights_port_cluster):
as many as the light partition's lists can hold */
enum
{
	LIGHTS_PORT_MAXIMUM_ENTRIES = HALO_PORT_MAXIMUM_CLUSTER_REFERENCES,
};
struct lights_port_entry
{
	long light_index;
	real_point3d position;
	real radius;
};
static struct
{
	/* each light's radius or rasterizer light written (lights_preprocess_scene) */
	unsigned long radius_writes;
	/* what the entries were kept for; each cluster's, and where they are */
	unsigned long generation;
	short entry_count;
	unsigned long cluster_generations[MAXIMUM_CLUSTERS_PER_STRUCTURE];
	short cluster_first_entries[MAXIMUM_CLUSTERS_PER_STRUCTURE];
	short cluster_entry_counts[MAXIMUM_CLUSTERS_PER_STRUCTURE];
	struct lights_port_entry entries[LIGHTS_PORT_MAXIMUM_ENTRIES];
} lights_port;
struct lights_game_globals *lights_game_globals = NULL;
short debug_rasterizer_light_count;
struct lights_globals lights_globals;

/* ---------- public code */

void *texture_cache_bitmap_load(
	struct bitmap_data *bitmap)
{
	void *hardware_format;

	profile_texture_start();
	hardware_format = _texture_cache_bitmap_get_hardware_format(
		bitmap,
		TRUE,
		TRUE);
	profile_texture_end();

	return hardware_format;
}

static real shade_scalar(
	real scalar0,
	real scalar1,
	real scalar2,
	real b,
	real c)
{
	return (scalar1 - scalar0) * b + (scalar2 - scalar0) * c + scalar0;
}

static void shade_vector2d(
	real_point2d const *vertex0,
	real_point2d const *vertex1,
	real_point2d const *vertex2,
	real b,
	real c,
	real_point2d *result)
{
	result->x = shade_scalar(vertex0->x, vertex1->x, vertex2->x, b, c);
	result->y = shade_scalar(vertex0->y, vertex1->y, vertex2->y, b, c);

	return;
}

static void shade_vector3d(
	real_vector3d const *vertex0,
	real_vector3d const *vertex1,
	real_vector3d const *vertex2,
	real b,
	real c,
	real_vector3d *result)
{
	result->i = shade_scalar(vertex0->i, vertex1->i, vertex2->i, b, c);
	result->j = shade_scalar(vertex0->j, vertex1->j, vertex2->j, b, c);
	result->k = shade_scalar(vertex0->k, vertex1->k, vertex2->k, b, c);

	return;
}

void sample_lightmap(
	struct structure_material *material,
	struct bitmap_data *bitmap,
	struct structure_surface const *surface,
	real s,
	real t,
	real_rgb_color *color)
{
	real_point2d texcoords[3];
	real_point2d texcoord;

	match_assert(
		"c:\\halo\\SOURCE\\objects\\object_lights.c",
		143,
		material->lightmap_vertices.type==_rasterizer_vertex_type_environment_lightmap_uncompressed || material->lightmap_vertices.type==_rasterizer_vertex_type_environment_lightmap_compressed);
	environment_lightmap_vertex_compressed_get_texcoord(
		structure_material_get_lightmap_vertex(material, surface->vertex_indices[0]),
		&texcoords[0]);
	environment_lightmap_vertex_compressed_get_texcoord(
		structure_material_get_lightmap_vertex(material, surface->vertex_indices[1]),
		&texcoords[1]);
	environment_lightmap_vertex_compressed_get_texcoord(
		structure_material_get_lightmap_vertex(material, surface->vertex_indices[2]),
		&texcoords[2]);
	shade_vector2d(
		&texcoords[0],
		&texcoords[1],
		&texcoords[2],
		s,
		t,
		&texcoord);
	pixel32_to_real_rgb_color(
		bitmap_2d_get_pixel(bitmap, &texcoord, 1.0f),
		color);

	return;
}

void sample_diffuse_texture(
	struct structure_material *material,
	struct bitmap_data *bitmap,
	struct structure_surface const *surface,
	real s,
	real t,
	real_rgb_color *color)
{
	real_point2d texcoords[3];
	real_point2d texcoord;

	match_assert(
		"c:\\halo\\SOURCE\\objects\\object_lights.c",
		167,
		material->vertices.type==_rasterizer_vertex_type_environment_uncompressed || material->vertices.type==_rasterizer_vertex_type_environment_compressed);
	environment_vertex_compressed_get_texcoord(
		structure_material_get_vertex(material, surface->vertex_indices[0]),
		&texcoords[0]);
	environment_vertex_compressed_get_texcoord(
		structure_material_get_vertex(material, surface->vertex_indices[1]),
		&texcoords[1]);
	environment_vertex_compressed_get_texcoord(
		structure_material_get_vertex(material, surface->vertex_indices[2]),
		&texcoords[2]);
	shade_vector2d(
		&texcoords[0],
		&texcoords[1],
		&texcoords[2],
		s,
		t,
		&texcoord);
	pixel32_to_real_rgb_color(
		bitmap_2d_get_pixel(bitmap, &texcoord, 0.3f),
		color);

	return;
}

static boolean should_render_lights(
	void)
{
	if (lights_game_globals->render_lights && game_engine_allow_dynamic_lighting())
	{
		return TRUE;
	}

	return FALSE;
}

void lights_initialize(
	void)
{
	light_data = game_state_data_new(
		"lights",
		MAXIMUM_LIGHTS_PER_MAP,
		sizeof(struct light_datum));
	lights_game_globals = game_state_malloc(
		"lights globals",
		NULL,
		sizeof(struct lights_game_globals));
	match_assert(
		"c:\\halo\\SOURCE\\objects\\object_lights.c",
		0xC2,
		light_data);
	match_assert(
		"c:\\halo\\SOURCE\\objects\\object_lights.c",
		0xC3,
		lights_game_globals);
	lights_game_globals->render_lights = TRUE;
	if (light_data)
	{
		cluster_partition_new(&light_cluster_partition, "light");
	}
	else
	{
		error(_error_silent, "couldn't allocate memory for object lights.");
	}

	return;
}

void lights_dispose(
	void)
{
	cluster_partition_delete(&light_cluster_partition);

	return;
}

/* port: glows the port lights itself (port/linux/game/shield_glow.c): an
unattached light (light_new_unattached) on an object's node, kept alive and
colored and sized each frame by its maker, gone a moment after its maker
stops. Drawn with a light definition the map has (one without a lens flare
or a flashlight's flags, a plasma one where there is one), so no new tags;
NONE if it has none */
static long light_port_glow_definition = NONE - 1;

long light_port_glow_definition_get(
	void)
{
	if (light_port_glow_definition == NONE - 1)
	{
		struct tag_iterator iterator;
		long tag_index;
		long found = NONE;

		tag_iterator_new(&iterator, LIGHT_DEFINITION_TAG);
		for (tag_index = tag_iterator_next(&iterator); tag_index != NONE; tag_index = tag_iterator_next(&iterator))
		{
			struct point_light_definition *definition = light_definition_get(tag_index);

			if (!definition || definition->lens_flare.index != NONE ||
				TEST_FLAG(definition->flags, _light_definition_is_first_person_flashlight_bit))
			{
				continue;
			}
			found = tag_index;
			if (strstr(tag_get_name(tag_index), "plasma"))
				break;
		}
		light_port_glow_definition = found;
	}
	return light_port_glow_definition;
}

/* port: a glow's light (one made with that definition, on that object) its
color and radius for now; FALSE if it is gone */
boolean light_port_glow_set(
	long light_index,
	long object_index,
	real_rgb_color const *color,
	real radius)
{
	struct light_datum *light;

	if (light_index == NONE || !light_data->valid)
		return FALSE;
	light = (struct light_datum *)datum_try_and_get(light_data, light_index);
	if (!light || light->definition_index != light_port_glow_definition || light->object_index != object_index)
		return FALSE;
	SET_FLAG(light->flags, _point_light_port_glow_bit, TRUE);
	light->parent_light_index = game_time_get(); /* (alive another moment) */
	light->color.red = PIN(color->red, 0.0f, 1.0f);
	light->color.green = PIN(color->green, 0.0f, 1.0f);
	light->color.blue = PIN(color->blue, 0.0f, 1.0f);
	light->intensity_scale = radius;
	return TRUE;
}

void lights_initialize_for_new_map(
	void)
{
	light_port_glow_definition = NONE - 1; /* port: (looked for again) */
	data_make_valid(light_data);
	lights_game_globals->render_lights = TRUE;
	cluster_partition_make_valid(&light_cluster_partition);

	return;
}

void lights_dispose_from_old_map(
	void)
{
	data_make_invalid(light_data);
	cluster_partition_make_invalid(&light_cluster_partition);

	return;
}

/* port: the lights array out of order in a map (game_state.c's
game_state_check_data_arrays reports how): Sentry's NATIVE-7 found it made
for no map, every light lost, and the next light made or looked up crashed.
Lights are only seen, so they all go: the array is made again, valid and
empty, its clusters' references with it, and the objects let go of the
lights they had (their attachments, light_delete and
object_get_self_illumination take none for lost). Lights made from then on
are as ever. Whether it did. */
boolean lights_port_recover(
	void)
{
	struct data_array *data = light_data;
	struct object_iterator iterator;
	struct object_datum *object;

	if (data->signature == 'd@t@' && data->data == (void *)(data + 1) && data->valid && data->next_identifier &&
		data->maximum_count == MAXIMUM_LIGHTS_PER_MAP && data->size == sizeof(struct light_datum) &&
		data->count >= 0 && data->count <= data->maximum_count &&
		data->actual_count >= 0 && data->actual_count <= data->count &&
		data->first_free_absolute_index >= 0 && data->first_free_absolute_index <= data->maximum_count)
	{
		return FALSE;
	}
	/* (how, and since when) */
	game_state_check_data_arrays();
	data_initialize(data, "lights", MAXIMUM_LIGHTS_PER_MAP, sizeof(struct light_datum));
	lights_initialize_for_new_map();
	object_iterator_new(&iterator, _object_mask_all, 0);
	while ((object = object_iterator_next(&iterator)) != NULL)
	{
		struct object_definition *definition = object_definition_get(object->definition_index);
		short attachment_count = (short)MIN(definition->object.attachments.count, MAXIMUM_NUMBER_OF_ATTACHMENTS_PER_OBJECT);
		short attachment_index;

		for (attachment_index = 0; attachment_index < attachment_count; attachment_index++)
		{
			if (object->object.attachment_types[attachment_index] == _object_attachment_type_light)
				object->object.attachment_indices[attachment_index] = NONE;
		}
	}
	platform_log("lights: the lights were out of order and all of them were let go");

	return TRUE;
}

boolean lights_enable(
	boolean enable)
{
	lights_game_globals->render_lights = enable;

	return enable;
}

long light_new(
	long definition_index,
	long object_index,
	short object_attachment_index,
	short object_function_index,
	short object_change_color_index)
{
	struct point_light_definition *definition = light_definition_get(definition_index);
	long light_index = NONE;

	if (TEST_FLAG(definition->flags, _light_definition_dynamic_bit)
		|| definition->lens_flare.index != NONE)
	{
		light_index = datum_new(light_data);
		/* port: no light if the new one can't be had (data.c's data_usable) */
		if (light_index != NONE && !datum_try_and_get(light_data, light_index))
			light_index = NONE;
		if (light_index != NONE)
		{
			struct light_datum *light = light_get(light_index);

			light->definition_index = definition_index;
			light->object_index = object_index;
			light->attachment_marker_index = object_attachment_index;
			/* port: a function and a change color the object holds, or none
			(an attachment's references in a map; retail's are none to d for
			the function, none to b for the color) */
			light->function_index = object_function_index;
			light->color_function_index = object_change_color_index;
			if ((object_function_index!=NONE && !VALID_INDEX(object_function_index, NUMBER_OF_OUTGOING_OBJECT_FUNCTIONS)) ||
				(object_change_color_index!=NONE && !VALID_INDEX(object_change_color_index, NUMBER_OF_OBJECT_CHANGE_COLORS)))
			{
				static boolean reference_reported = FALSE;

				if (!reference_reported)
				{
					reference_reported = TRUE;
					error(
						_error_silent,
						"### ERROR light %s is attached with function #%d and change color #%d; the bad one is none",
						tag_get_name(definition_index),
						object_function_index,
						object_change_color_index);
				}
				if (!VALID_INDEX(object_function_index, NUMBER_OF_OUTGOING_OBJECT_FUNCTIONS))
				{
					light->function_index = NONE;
				}
				if (!VALID_INDEX(object_change_color_index, NUMBER_OF_OBJECT_CHANGE_COLORS))
				{
					light->color_function_index = NONE;
				}
			}
			light->flags = 0;
			SET_FLAG(light->flags, _point_light_dynamic_bit,
				TEST_FLAG(definition->flags, _light_definition_dynamic_bit));
			SET_FLAG(light->flags, _point_light_connects_to_map_bit,
				TEST_FLAG(light->flags, _point_light_dynamic_bit)
				|| definition->lens_flare.index != NONE);
			light->cluster_reference = NONE;
			light->parent_light_index = NONE;
			light_reconnect_to_map(light_index);
			light->marker = lights_globals.marker - 1;
		}
	}

	return light_index;
}

long light_new_unattached(
	long definition_index,
	long object_index,
	short object_node_index,
	real_point3d const *position,
	real_vector3d const *direction,
	real scale)
{
	long light_index = datum_new(light_data);

	if (light_index != NONE)
	{
		struct light_datum *light = light_get(light_index);
		struct point_light_definition *definition = light_definition_get(definition_index);

		light->flags = 0;
		light->parent_light_index = game_time_get();
		light->definition_index = definition_index;
		light->object_index = object_index;
		light->intensity_scale = scale;
		light->cluster_reference = NONE;
		SET_FLAG(light->flags, _point_light_dynamic_bit, TRUE);
		SET_FLAG(light->flags, _point_light_connects_to_map_bit, TRUE);
		if (object_index == NONE)
		{
			light->position = *position;
			light->forward = *direction;
		}
		else
		{
			light->attachment_marker_index = object_node_index;
			light->node.relative_position = *position;
			light->node.relative_forward = *direction;
		}
		light_reconnect_to_map(light_index);
		light->marker = lights_globals.marker - 1;
	}

	return light_index;
}

void lights_preprocess_scene(
	void)
{
	long current_time = game_time_get();
	long light_index;
	short rendered_cluster_index;
	short scene_light_index;
	short queued_lens_flare_index;

	profile_enter(lights_section);
	debug_rasterizer_light_count = 0;
	/* port: the lights in order before they are drawn (a frame can come
	between ticks: lights_port_recover) */
	lights_port_recover();
	shield_glow_update(); /* port: port/linux/game/shield_glow.c */
	for (light_index = data_next_index(light_data, NONE);
		light_index != NONE;
		light_index = data_next_index(light_data, light_index))
	{
		struct light_datum *light = light_get(light_index);

		SET_FLAG(
			light->flags,
			_point_light_attached_to_first_person_weapon_bit,
			FALSE);
		light->rasterizer_light_index = NONE;
		lights_port.radius_writes++;
		if (light->parent_light_index != NONE)
		{
			struct point_light_definition *definition = light_definition_get(
				light->definition_index);
			real elapsed = (real)(current_time - light->parent_light_index);

			if (elapsed > definition->transition_duration)
			{
				cluster_partition_disconnect(
					&light_cluster_partition,
					light_index,
					&light_get(light_index)->cluster_reference);
				datum_delete(light_data, light_index);
			}
			else if (object_try_and_get(light->object_index))
			{
				light_disconnect_from_map(light_index);
				light_reconnect_to_map(light_index);
			}
		}
	}

	light_marker_begin();
	lights_globals.scene_point_light_count = structure_visibility_find_objects(
		lights_globals.scene_point_lights,
		MAXIMUM_RENDERED_LIGHTS,
		cluster_get_first_light,
		cluster_get_next_light,
		light_get_bounding_sphere,
		light_unmarked,
		light_mark);
	light_marker_end();
	rasterizer_lights_begin();
	for (rendered_cluster_index = 0;
		rendered_cluster_index < render.rendered_cluster_count;
		rendered_cluster_index++)
	{
		rasterizer_lens_flare_submit_for_cluster(
			rendered_cluster_get(rendered_cluster_index)->cluster_index);
	}

	for (scene_light_index = 0;
		scene_light_index < lights_globals.scene_point_light_count;
		scene_light_index++)
	{
		long scene_light_handle = lights_globals.scene_point_lights[scene_light_index];
		struct light_datum *light = light_get(scene_light_handle);
		struct point_light_definition *definition = light_definition_get(
			light->definition_index);
		struct object_datum *object;
		real intensity;
		real inverse_intensity;
		real lens_flare_scale = 1.0f;

		render_debug_light(scene_light_handle);
		object = light->object_index != NONE
			? object_try_and_get(light->object_index)
			: NULL;
		if (TEST_FLAG(light->flags, _point_light_port_glow_bit))
		{
			/* port: a glow's color is as set (light_port_glow_set) */
			intensity = 1.0f;
		}
		else if (light->parent_light_index == NONE)
		{
			real_rgb_color const *color;

			match_assert(
				"c:\\halo\\SOURCE\\objects\\object_lights.c",
				0x1AC,
				object);
			object_get_function_value(
				light->object_index,
				light->function_index,
				&intensity);
			color = light->color_function_index == NONE
				? global_real_rgb_white
				: &object->object.outgoing_change_colors[light->color_function_index];
			rgb_colors_interpolate_and_scale(
				&light->color,
				definition->color_interpolation_flags,
				&definition->color_lower_bound,
				&definition->color_upper_bound,
				color,
				intensity);
		}
		else
		{
			intensity = (1.0f - transition_function_evaluate(
				definition->falloff_function,
				(real)(current_time - light->parent_light_index)
					/ definition->transition_duration))
				* light->intensity_scale;
			rgb_colors_interpolate(
				&light->color,
				definition->color_interpolation_flags,
				&definition->color_lower_bound.rgb,
				&definition->color_upper_bound.rgb,
				intensity);
		}

		inverse_intensity = 1.0f - intensity;
		match_assert(
			"c:\\halo\\SOURCE\\objects\\object_lights.c",
			0x1BB,
			light->color.red >=0.0f && light->color.red <=1.0f);
		match_assert(
			"c:\\halo\\SOURCE\\objects\\object_lights.c",
			0x1BC,
			light->color.green>=0.0f && light->color.green<=1.0f);
		match_assert(
			"c:\\halo\\SOURCE\\objects\\object_lights.c",
			0x1BD,
			light->color.blue >=0.0f && light->color.blue <=1.0f);

		if (object)
		{
			long ultimate_parent_index = object_get_ultimate_parent(
				light->object_index);
			struct object_datum *ultimate_parent = object_get(
				ultimate_parent_index);

			if (TEST_FLAG(_object_mask_unit, ultimate_parent->object.type))
			{
				struct unit_datum *parent_unit = unit_get(ultimate_parent_index);

				if (parent_unit->unit.active_camouflage > 0.0f
					&& !TEST_FLAG(
						light_definition_get(light->definition_index)->flags,
						_light_definition_dont_fade_active_camouflage_bit))
				{
					lens_flare_scale = 1.0f - parent_unit->unit.active_camouflage;
					light->color.red *= lens_flare_scale;
					light->color.green *= lens_flare_scale;
					light->color.blue *= lens_flare_scale;
					match_assert(
						"c:\\halo\\SOURCE\\objects\\object_lights.c",
						0x1D1,
						light->color.red >=0.0f && light->color.red <=1.0f);
					match_assert(
						"c:\\halo\\SOURCE\\objects\\object_lights.c",
						0x1D2,
						light->color.green>=0.0f && light->color.green<=1.0f);
					match_assert(
						"c:\\halo\\SOURCE\\objects\\object_lights.c",
						0x1D3,
						light->color.blue >=0.0f && light->color.blue <=1.0f);
				}
			}
		}

		if (light->color.red != 0.0f
			|| light->color.green != 0.0f
			|| light->color.blue != 0.0f)
		{
			if (TEST_FLAG(light->flags, _point_light_dynamic_bit))
			{
				light->radius = (definition->radius_modifier_lower_bound * inverse_intensity
					+ definition->radius_modifier_upper_bound * intensity)
					* definition->radius;
				/* port: a glow's radius is as set (light_port_glow_set) */
				if (TEST_FLAG(light->flags, _point_light_port_glow_bit))
					light->radius = light->intensity_scale;
				lights_port.radius_writes++;
				if (light->radius != 0.0f)
				{
					struct rasterizer_light_submit_parameters light_parameters;

					light_parameters.definition = light_definition_get(
						light->definition_index);
					light_parameters.position = light->position;
					light_parameters.forward = light->forward;
					light_parameters.up = light->up;
					light_parameters.radius = light->radius;
					light_parameters.color = light->color;
					match_assert(
						"c:\\halo\\SOURCE\\objects\\object_lights.c",
						0x1EE,
						light->color.red >=0.0f && light->color.red <=1.0f);
					match_assert(
						"c:\\halo\\SOURCE\\objects\\object_lights.c",
						0x1EF,
						light->color.green>=0.0f && light->color.green<=1.0f);
					match_assert(
						"c:\\halo\\SOURCE\\objects\\object_lights.c",
						0x1F0,
						light->color.blue >=0.0f && light->color.blue <=1.0f);

					if (light->parent_light_index == NONE)
					{
						boolean adjusted = FALSE;

						if (TEST_FLAG(
							definition->flags,
							_light_definition_is_first_person_flashlight_bit))
						{
							boolean centered = first_person_weapon_center_flashlight(
								light->object_index,
								&light_parameters.position,
								&light_parameters.forward,
								&light_parameters.up);

							/* port: a beam not on a first-person weapon
							(another player's, or your own with no weapon in
							hand), from the marker on the pose the frame draws
							(render_interpolation.c): the light's own place is
							the tick's, so it stepped at 30 a second while the
							view moved every frame */
							if (!centered)
							{
								struct object_marker markers[1];

								if (object_get_marker_by_name(
									light->object_index,
									object_get_attachment_marker_name(light->object_index,
										light->attachment_marker_index),
									markers,
									1) > 0)
								{
									light_parameters.position = markers[0].matrix.position;
									light_parameters.forward = markers[0].matrix.forward;
									light_parameters.up = markers[0].matrix.up;
								}
							}
							adjusted = TRUE;
						}
						else if (object->object.type == _object_type_weapon
							&& object->object.parent_object_index != NONE)
						{
							adjusted = first_person_weapon_adjust_light(
								light->object_index,
								object_get_attachment_marker_name(
									light->object_index,
									light->attachment_marker_index),
								&light_parameters.position,
								&light_parameters.forward,
								&light_parameters.up);
						}
						if (adjusted)
						{
							SET_FLAG(
								light->flags,
								_point_light_attached_to_first_person_weapon_bit,
								TRUE);
						}
					}

					light->rasterizer_light_index = rasterizer_light_submit(
						&light_parameters);
					/* (port: the lights submitted so far: not a light the
					window had no room for (MAXIMUM_LIGHTS_PER_WINDOW), whose
					index is NONE: that made the count 0, and every object
					lit by the ones before it failed render_objects.c's
					check of their indices against it) */
					if (light->rasterizer_light_index != NONE)
						debug_rasterizer_light_count = (short)(light->rasterizer_light_index + 1);
				}
			}
			else
			{
				light->radius = definition->radius;
				lights_port.radius_writes++;
			}

			if (definition->lens_flare.index != NONE)
			{
				struct rasterizer_lens_flare_submit_parameters lens_flare_parameters;

				lens_flare_parameters.definition = lens_flare_definition_get(
					definition->lens_flare.index);
				lens_flare_parameters.compressed_light_color =
					real_a_rgb_color_to_pixel32(lens_flare_scale, &light->color);
				lens_flare_parameters.compressed_light_scale =
					compress_real_to_int8(intensity);
				lens_flare_parameters.compressed_window_index = (byte)render.window_index;
				lens_flare_parameters.light_index =
					(short)DATUM_INDEX_TO_ABSOLUTE_INDEX(scene_light_handle);
				lens_flare_parameters.light_identifier =
					(short)DATUM_INDEX_TO_IDENTIFIER(scene_light_handle);
				match_assert(
					"c:\\halo\\SOURCE\\objects\\object_lights.c",
					0x21C,
					lens_flare_parameters.light_identifier!=0);
				if (lens_flare_parameters.light_identifier == NONE)
				{
					lens_flare_parameters.light_identifier = 0;
				}

				if (light->parent_light_index == NONE)
				{
					char const *marker_name = object_get_attachment_marker_name(
						light->object_index,
						light->attachment_marker_index);
					struct object_marker markers[MAXIMUM_LENS_FLARES_PER_LIGHT];
					short marker_count = 0;
					short marker_index;

					if (object->object.type == _object_type_weapon
						&& object->object.parent_object_index != NONE)
					{
						marker_count = first_person_weapon_get_marker_by_name_render(
							light->object_index,
							marker_name,
							markers,
							MAXIMUM_LENS_FLARES_PER_LIGHT);
						if (marker_count > 0)
						{
							lens_flare_parameters.compressed_window_index |=
								LENS_FLARE_FIRST_PERSON_MARKER_FLAG;
						}
					}
					if (!marker_count)
					{
						marker_count = object_get_marker_by_name(
							light->object_index,
							marker_name,
							markers,
							MAXIMUM_LENS_FLARES_PER_LIGHT);
					}

					for (marker_index = 0;
						marker_index < marker_count;
						marker_index++)
					{
						lens_flare_parameters.position = markers[marker_index].matrix.position;
						lens_flare_parameters.compressed_direction =
							compress_real_vector3d_to_int32_clamp(
								&markers[marker_index].matrix.forward);
						lens_flare_parameters.compressed_up =
							compress_real_vector3d_to_int32_clamp(
								&markers[marker_index].matrix.up);
						lens_flare_parameters.lens_flare_index = marker_index;
						rasterizer_lens_flare_submit(&lens_flare_parameters);
					}
				}
				else
				{
					lens_flare_parameters.position = light->position;
					lens_flare_parameters.compressed_direction =
						compress_real_vector3d_to_int32_clamp(&light->forward);
					lens_flare_parameters.compressed_up =
						compress_real_vector3d_to_int32_clamp(&light->up);
					lens_flare_parameters.lens_flare_index = 0;
					rasterizer_lens_flare_submit(&lens_flare_parameters);
				}
			}
		}
	}

	for (queued_lens_flare_index = 0;
		queued_lens_flare_index < lights_globals.queued_lens_flare_count;
		queued_lens_flare_index++)
	{
		rasterizer_lens_flare_submit(
			&lights_globals.queued_lens_flares[queued_lens_flare_index]);
	}
	lights_globals.queued_lens_flare_count = 0;
	rasterizer_lights_end();
	profile_exit(lights_section);

	return;
}

void light_delete(
	long light_index)
{
	struct light_datum *light = datum_try_and_get(light_data, light_index);

	/* port: not one let go of (lights_port_recover) */
	if (!light)
		return;
	cluster_partition_disconnect(
		&light_cluster_partition,
		light_index,
		&light->cluster_reference);
	datum_delete(light_data, light_index);

	return;
}

real object_get_self_illumination(
	long object_index)
{
	struct object_datum *object = object_get(object_index);
	struct object_definition *definition = object_definition_get(object->definition_index);
	real illumination = 0.0f;
	short attachment_index = 0;
	/* port: the attachments attachments_new made (a map's count; past them
	the types and indices were the object's other fields; retail has up to
	8) */
	short attachment_count = (short)MIN(definition->object.attachments.count, MAXIMUM_NUMBER_OF_ATTACHMENTS_PER_OBJECT);

	if (attachment_count > 0)
	{
		do
		{
			if (object->object.attachment_types[attachment_index] == _object_attachment_type_light
				&& object->object.attachment_indices[attachment_index] != NONE)
			{
				struct light_datum *light = datum_try_and_get(light_data, object->object.attachment_indices[attachment_index]);

				/* (port: not one let go of: lights_port_recover) */
				if (light)
					illumination += real_rgb_color_brightness(&light->color);
			}
			attachment_index++;
		}
		while (attachment_index < attachment_count);
	}

	if (object->object.first_child_object_index != NONE)
	{
		illumination += object_get_self_illumination(object->object.first_child_object_index);
	}
	if (object->object.next_object_index != NONE)
	{
		illumination += object_get_self_illumination(object->object.next_object_index);
	}

	return illumination;
}

void lights_queue_lens_flare(
	long lens_flare_definition_index,
	real_point3d const *position,
	real_vector3d const *direction,
	real_vector3d const *up,
	real_rgb_color const *color,
	real scale)
{
	if (lights_globals.queued_lens_flare_count < MAXIMUM_QUEUED_LENS_FLARES
		&& (color->red != 0.0f || color->green != 0.0f || color->blue != 0.0f))
	{
		struct rasterizer_lens_flare_submit_parameters *lens_flare_parameters =
			&lights_globals.queued_lens_flares[lights_globals.queued_lens_flare_count];

		lens_flare_parameters->compressed_light_color = real_a_rgb_color_to_pixel32(1.0f, color);
		lens_flare_parameters->compressed_light_scale = compress_real_to_int8(scale);
		lens_flare_parameters->definition = lens_flare_definition_get(lens_flare_definition_index);
		lens_flare_parameters->position = *position;
		lens_flare_parameters->compressed_direction = compress_real_vector3d_to_int32_clamp(direction);
		lens_flare_parameters->compressed_up = compress_real_vector3d_to_int32_clamp(up);
		lens_flare_parameters->compressed_window_index = (byte)render.window_index;
		lens_flare_parameters->light_index = NONE;
		lens_flare_parameters->light_identifier = NONE;
		lens_flare_parameters->lens_flare_index = lights_globals.queued_lens_flare_count;
		lights_globals.queued_lens_flare_count++;
	}

	return;
}

void light_particle(
	real_point3d const *position,
	real_rgb_color *light,
	real_rgb_color *diffuse,
	boolean block)
{
	short lightmap_index;
	short material_index;
	long surface_index;
	real s;
	real t;
	real_point3d collision_point;

	*light = *global_real_rgb_grey;
	*diffuse = *global_real_rgb_grey;
	if (structure_test_vector(
		position,
		&lightmap_sample_raycast_down,
		&collision_point,
		&lightmap_index,
		&material_index,
		&surface_index,
		&s,
		&t))
	{
		struct structure_bsp *structure = global_structure_bsp_get();
		struct structure_lightmap *lightmap = TAG_BLOCK_GET_ELEMENT(
			&structure->lightmaps,
			lightmap_index,
			struct structure_lightmap);
		struct structure_material *material = TAG_BLOCK_GET_ELEMENT(
			&lightmap->materials,
			material_index,
			struct structure_material);
		struct shader *shader = shader_definition_get(material->shader.index);

		if (shader->base.type == _shader_type_environment)
		{
			struct shader_environment_definition *shader_environment =
				shader_environment_definition_get(shader);

			if (structure->lightmap_group.index != NONE
				&& shader_environment->base_map.index != NONE
				&& lightmap->bitmap_index != NONE)
			{
				struct bitmap_data *lightmap_bitmap = bitmap_group_try_and_get_bitmap(
					structure->lightmap_group.index,
					lightmap->bitmap_index);
				struct bitmap_data *diffuse_bitmap = bitmap_group_try_and_get_bitmap(
					shader_environment->base_map.index,
					(short)(material->permutation_index
						% bitmap_group_get(shader_environment->base_map.index)->bitmaps.count));
				struct structure_surface const *surface = NULL;

				if (lightmap_bitmap
					&& ((block && texture_cache_bitmap_load(lightmap_bitmap))
						|| _texture_cache_bitmap_get_hardware_format(lightmap_bitmap, FALSE, FALSE)))
				{
					surface = TAG_BLOCK_GET_ELEMENT(
						&structure->surfaces,
						surface_index,
						struct structure_surface);
					sample_lightmap(material, lightmap_bitmap, surface, s, t, light);
					light->red = MIN(light->red + 0.1f, 1.0f);
					light->green = MIN(light->green + 0.1f, 1.0f);
					light->blue = MIN(light->blue + 0.1f, 1.0f);
				}

				if (diffuse_bitmap
					&& ((block && texture_cache_bitmap_load(diffuse_bitmap))
						|| _texture_cache_bitmap_get_hardware_format(diffuse_bitmap, FALSE, FALSE)))
				{
					if (!surface)
					{
						surface = TAG_BLOCK_GET_ELEMENT(
							&structure->surfaces,
							surface_index,
							struct structure_surface);
					}
					sample_diffuse_texture(material, diffuse_bitmap, surface, s, t, diffuse);
				}
			}
		}
	}

	return;
}

void light_disconnect_from_map(
	long light_index)
{
	struct light_datum *light = light_get(light_index);

	if (TEST_FLAG(light->flags, _point_light_connects_to_map_bit))
	{
		match_assert(
			"c:\\halo\\SOURCE\\objects\\object_lights.c",
			0x4D0,
			TEST_FLAG(light->flags, _point_light_connected_to_map_bit));
		cluster_partition_disconnect(
			&light_cluster_partition,
			light_index,
			&light->cluster_reference);
		SET_FLAG(light->flags, _point_light_connected_to_map_bit, FALSE);
	}

	return;
}

void lights_disconnect_from_structure_bsp(
	void)
{
	long light_index;

	for (light_index = data_next_index(light_data, NONE);
		light_index != NONE;
		light_index = data_next_index(light_data, light_index))
	{
		struct light_datum *light = light_get(light_index);

		if (TEST_FLAG(light->flags, _point_light_connected_to_map_bit))
		{
			light_disconnect_from_map(light_index);
			SET_FLAG(light->flags, _point_light_connected_to_map_bit, TRUE);
		}
	}

	return;
}

void lights_reconnect_to_structure_bsp(
	void)
{
	long light_index;

	for (light_index = data_next_index(light_data, NONE);
		light_index != NONE;
		light_index = data_next_index(light_data, light_index))
	{
		struct light_datum *light = light_get(light_index);

		if (TEST_FLAG(light->flags, _point_light_connected_to_map_bit))
		{
			SET_FLAG(light->flags, _point_light_connected_to_map_bit, FALSE);
			light_reconnect_to_map(light_index);
		}
	}

	return;
}

static void light_marker_begin(
	void)
{
	match_assert(
		"c:\\halo\\SOURCE\\objects\\object_lights.c",
		0x664,
		!lights_globals.marker_initialized);
	lights_globals.marker++;
	lights_globals.marker_initialized = TRUE;

	return;
}

static boolean light_mark(
	long light_index)
{
	struct light_datum *light = light_get(light_index);

	match_assert(
		"c:\\halo\\SOURCE\\objects\\object_lights.c",
		0x67F,
		lights_globals.marker_initialized);
	if (light->marker != lights_globals.marker)
	{
		light->marker = lights_globals.marker;
		return TRUE;
	}

	return FALSE;
}

static boolean light_unmarked(
	long light_index)
{
	struct light_datum *light = light_get(light_index);

	match_assert(
		"c:\\halo\\SOURCE\\objects\\object_lights.c",
		0x66F,
		lights_globals.marker_initialized);
	if (light->marker != lights_globals.marker)
	{
		return TRUE;
	}

	return FALSE;
}

static void light_marker_end(
	void)
{
	match_assert(
		"c:\\halo\\SOURCE\\objects\\object_lights.c",
		0x68E,
		lights_globals.marker_initialized);
	lights_globals.marker_initialized = FALSE;

	return;
}

static void render_debug_light(
	long light_index)
{
	if (debug_lights)
	{
		struct light_datum *light = light_get(light_index);
		struct point_light_definition *definition = light_definition_get(light->definition_index);
		real radius = definition->radius_modifier_upper_bound * definition->radius;
		real_argb_color color = *global_real_argb_orange;

		render_debug_sphere(
			TRUE,
			&light->position,
			definition->lens_flare_radius,
			global_real_argb_white);
		render_debug_sphere(
			TRUE,
			&light->position,
			light->radius,
			&color);
		color.red *= 0.8f;
		color.green *= 0.8f;
		color.blue *= 0.8f;
		if (!TEST_FLAG(definition->flags, _light_definition_no_specular_bit))
		{
			radius *= definition->specular_radius_multiplier;
			render_debug_sphere(
				TRUE,
				&light->position,
				definition->specular_radius_multiplier * light->radius,
				&color);
		}
		color.red *= 0.8f;
		color.green *= 0.8f;
		color.blue *= 0.8f;
		render_debug_sphere(
			TRUE,
			&light->position,
			radius,
			&color);
	}

	return;
}

/* ---------- private code */

static real light_attenuation(
	real radius,
	real distance)
{
	return 1.0f - (distance * distance) / (radius * radius);
}

static void brighten_real_rgb_color(
	real_rgb_color *color,
	real fraction)
{
	real maximum = MAX(color->red, MAX(color->green, color->blue));
	real scale = fraction + 1.0f;
	real scaled_maximum = scale * maximum;

	if (scaled_maximum > 1.0f)
	{
		scale = 1.0f / maximum;
	}
	else if (scaled_maximum < fraction)
	{
		scale = fraction / maximum;
	}

	color->red *= scale;
	color->green *= scale;
	color->blue *= scale;

	return;
}

static long cluster_get_first_light(
	long *reference_index,
	short cluster_index)
{
	return cluster_partition_get_first_datum(
		&light_cluster_partition,
		reference_index,
		cluster_index);
}

static long cluster_get_next_light(
	long *reference_index)
{
	return cluster_partition_get_next_datum(
		&light_cluster_partition,
		reference_index);
}

static void light_compute_bounding_sphere(
	long light_index,
	boolean maximum,
	boolean specular,
	boolean lens_flare_only,
	real_point3d *position,
	real *radius)
{
	struct light_datum *light = light_get(light_index);
	struct point_light_definition *definition = light_definition_get(light->definition_index);
	real light_radius = maximum
		? definition->radius_modifier_upper_bound * definition->radius
		: light->radius;

	if (!TEST_FLAG(definition->flags, _light_definition_no_specular_bit)
		&& (specular || maximum))
	{
		light_radius *= definition->specular_radius_multiplier;
	}

	if (lens_flare_only && light_radius < definition->lens_flare_radius)
	{
		*position = light->position;
		*radius = definition->lens_flare_radius;
	}
	else if (definition->cutoff_angle < _pi / 2)
	{
		if (definition->cutoff_angle < _pi / 4)
		{
			light_radius = *radius = light_radius / definition->runtime_cosine_cutoff_angle;
			position->x = light_radius * light->forward.i + light->position.x;
			position->y = light_radius * light->forward.j + light->position.y;
			position->z = light_radius * light->forward.k + light->position.z;
		}
		else
		{
			*radius = light_radius * definition->runtime_sine_cutoff_angle;
			light_radius *= definition->runtime_cosine_cutoff_angle;
			position->x = light_radius * light->forward.i + light->position.x;
			position->y = light_radius * light->forward.j + light->position.y;
			position->z = light_radius * light->forward.k + light->position.z;
		}
	}
	else
	{
		*position = light->position;
		*radius = light_radius;
	}

	return;
}

static short light_build_cluster_array(
	long light_index,
	short maximum_count,
	short *cluster_indices)
{
	struct light_datum *light = light_get(light_index);
	short cluster_count = 0;
	short cluster_index;
	long reference_index;

	for (cluster_index = (short)cluster_partition_get_first_cluster(
			&light_cluster_partition,
			&reference_index,
			light->cluster_reference);
		cluster_count < maximum_count;
		cluster_index = (short)cluster_partition_get_next_cluster(
			&light_cluster_partition,
			&reference_index))
	{
		if (cluster_index == NONE)
		{
			break;
		}
		cluster_indices[cluster_count++] = cluster_index;
	}

	return cluster_count;
}

static void light_get_bounding_sphere(
	long light_index,
	real_point3d *position,
	real *radius)
{
	light_compute_bounding_sphere(
		light_index,
		TRUE,
		FALSE,
		TRUE,
		position,
		radius);

	return;
}

void lights_render_diffuse(
	void)
{
	short cluster_indices[MAXIMUM_CLUSTERS_PER_LIGHT];
	short light_index;

	rasterizer_environment_diffuse_lights_begin();
	if (should_render_lights())
	{
		for (light_index = 0;
			light_index < lights_globals.scene_point_light_count;
			light_index++)
		{
			struct light_datum *light = light_get(
				lights_globals.scene_point_lights[light_index]);

			if (TEST_FLAG(light->flags, _point_light_dynamic_bit)
				&& light->rasterizer_light_index != NONE)
			{
				boolean skip_clusters = TEST_FLAG(light->flags,
						_point_light_attached_to_first_person_weapon_bit)
					&& TEST_FLAG(light_definition_get(light->definition_index)->flags,
						_light_definition_supersize_in_first_person_bit);
				short cluster_count = 0;
				real_point3d center;
				real radius;

				if (!skip_clusters)
				{
					cluster_count = light_build_cluster_array(
						lights_globals.scene_point_lights[light_index],
						MAXIMUM_CLUSTERS_PER_LIGHT,
						cluster_indices);
				}
				light_compute_bounding_sphere(
					lights_globals.scene_point_lights[light_index],
					FALSE,
					FALSE,
					FALSE,
					&center,
					&radius);
				structure_render_diffuse_light(
					light->rasterizer_light_index,
					&center,
					radius,
					cluster_count,
					skip_clusters ? NULL : cluster_indices);
			}
		}
	}
	rasterizer_environment_diffuse_lights_end();

	return;
}

void lights_render_specular(
	void)
{
	short cluster_indices[MAXIMUM_CLUSTERS_PER_LIGHT];
	short light_index;

	rasterizer_environment_specular_lights_begin();
	if (should_render_lights())
	{
		for (light_index = 0;
			light_index < lights_globals.scene_point_light_count;
			light_index++)
		{
			struct light_datum *light = light_get(
				lights_globals.scene_point_lights[light_index]);

			if (TEST_FLAG(light->flags, _point_light_dynamic_bit)
				&& light->rasterizer_light_index != NONE
				&& !TEST_FLAG(light_definition_get(light->definition_index)->flags,
					_light_definition_no_specular_bit))
			{
				boolean skip_clusters = TEST_FLAG(light->flags,
						_point_light_attached_to_first_person_weapon_bit)
					&& TEST_FLAG(light_definition_get(light->definition_index)->flags,
						_light_definition_supersize_in_first_person_bit);
				short cluster_count = 0;
				real_point3d center;
				real radius;

				if (!skip_clusters)
				{
					cluster_count = light_build_cluster_array(
						lights_globals.scene_point_lights[light_index],
						MAXIMUM_CLUSTERS_PER_LIGHT,
						cluster_indices);
				}
				light_compute_bounding_sphere(
					lights_globals.scene_point_lights[light_index],
					FALSE,
					TRUE,
					FALSE,
					&center,
					&radius);
				structure_render_specular_light(
					light->rasterizer_light_index,
					&center,
					radius,
					cluster_count,
					skip_clusters ? NULL : cluster_indices);
			}
		}
	}
	rasterizer_environment_specular_lights_end();

	return;
}

void light_reconnect_to_map(
	long light_index)
{
	struct light_datum *light = light_get(light_index);
	struct point_light_definition *definition = light_definition_get(light->definition_index);
	struct object_marker markers[1];
	struct location location;
	real_point3d position;
	real radius;

	if (light->parent_light_index == NONE)
	{
		object_get_marker_by_name(
			light->object_index,
			object_get_attachment_marker_name(light->object_index, light->attachment_marker_index),
			markers,
			1);
		light->position = markers[0].matrix.position;
		light->forward = markers[0].matrix.forward;
		light->up = markers[0].matrix.up;
	}
	else if (object_try_and_get(light->object_index))
	{
		real_matrix4x3 *node_matrix = object_get_node_matrix(light->object_index, light->attachment_marker_index);

		matrix4x3_transform_point(node_matrix, &light->node.relative_position, &light->position);
		matrix4x3_transform_normal(node_matrix, &light->node.relative_forward, &light->forward);
		perpendicular3d(&light->forward, &light->up);
		normalize3d(&light->up);
	}

	if (TEST_FLAG(light->flags, _point_light_connects_to_map_bit))
	{
		light_compute_bounding_sphere(
			light_index,
			TRUE,
			FALSE,
			TRUE,
			&position,
			&radius);
		match_assert(
			"c:\\halo\\SOURCE\\objects\\object_lights.c",
			0x4F9,
			!TEST_FLAG(light->flags, _point_light_connected_to_map_bit));
		if (light->object_index != NONE && object_try_and_get(light->object_index))
		{
			object_get_location(light->object_index, &location);
		}
		else
		{
			scenario_location_from_point(&location, &position);
		}
		cluster_partition_reconnect(
			&light_cluster_partition,
			light_index,
			&light->cluster_reference,
			&position,
			radius,
			&location);
		SET_FLAG(light->flags, _point_light_connected_to_map_bit, TRUE);
	}

	return;
}

/* a light of a cluster, for find_point_lights_for_object_in_cluster: as
the Xbox's loop does each, marking it (so its other clusters pass it over) */
static void find_point_light_for_object(
	long object_index,
	long light_index,
	real_point3d const *center,
	real radius,
	long *light_indices,
	real *light_intensities,
	real *light_attenuations,
	short *light_count,
	short maximum_light_count)
{
	if (light_unmarked(light_index))
	{
		struct light_datum *light = light_get(light_index);

		if (light->rasterizer_light_index != NONE
			&& (light->object_index != object_index
				|| !TEST_FLAG(light_definition_get(light->definition_index)->flags,
					_light_definition_dont_light_own_object_bit)))
		{
			real distance = distance3d(&light->position, center);

			if (distance < radius + light->radius)
			{
				real attenuation = light_attenuation(light->radius, distance);
				real intensity = real_rgb_color_brightness(&light->color) * attenuation;
				short index;

				if (*light_count < maximum_light_count)
				{
					index = (*light_count)++;
				}
				else
				{
					real minimum_intensity = REAL_MAX;
					short dimmest_index = NONE;

					for (index = 0; index < *light_count; index++)
					{
						if (minimum_intensity > light_intensities[index])
						{
							minimum_intensity = light_intensities[index];
							dimmest_index = index;
						}
					}

					if (minimum_intensity < intensity)
					{
						index = dimmest_index;
					}
				}

				if (index < maximum_light_count)
				{
					light_indices[index] = light_index;
					light_intensities[index] = intensity;
					light_attenuations[index] = attenuation;
				}
			}
		}
		light_mark(light_index);
	}
}

/* port: a cluster's lights in its list's order, with where each is and how
far it reaches, kept while none of that changes: while the light partition's
lists (cluster_partitions.c's port_modification_count) and the lights' radii
and rasterizer lights (lights_port.radius_writes, written as a frame's lights
are prepared) are as they were; a light only moves as it joins its clusters
again. An object lit (each object drawn, each shadow) then passes over the
lights too far to reach it without their datums or the list's: with hundreds
of lights in a cluster (network co-op's extra enemies), reading them was
most of a frame's lighting. FALSE: not kept (the list walked as before). */
static boolean lights_port_cluster(
	short cluster_index,
	short *first_entry,
	short *entry_count)
{
	unsigned long generation = 1 + light_cluster_partition.port_modification_count + lights_port.radius_writes;
	long reference_index;
	long light_index;
	short first;

	if (cluster_index < 0 || cluster_index >= MAXIMUM_CLUSTERS_PER_STRUCTURE)
		return FALSE;
	if (lights_port.generation != generation)
	{
		lights_port.generation = generation;
		lights_port.entry_count = 0;
	}
	if (lights_port.cluster_generations[cluster_index] != generation)
	{
		first = lights_port.entry_count;
		for (light_index = cluster_partition_get_first_datum(&light_cluster_partition, &reference_index, cluster_index);
			light_index != NONE;
			light_index = cluster_partition_get_next_datum(&light_cluster_partition, &reference_index))
		{
			struct light_datum *light = light_get(light_index);
			struct lights_port_entry *entry;

			/* (one not drawn this frame lights nothing: reaching an object,
			it would only be marked, for that object's other clusters to pass
			over) */
			if (light->rasterizer_light_index == NONE)
				continue;
			if (lights_port.entry_count >= LIGHTS_PORT_MAXIMUM_ENTRIES)
			{
				/* (more than are kept: this cluster's walked) */
				lights_port.entry_count = first;
				first = NONE;
				break;
			}
			entry = &lights_port.entries[lights_port.entry_count++];
			entry->light_index = light_index;
			entry->position = light->position;
			entry->radius = light->radius;
		}
		lights_port.cluster_generations[cluster_index] = generation;
		lights_port.cluster_first_entries[cluster_index] = first;
		lights_port.cluster_entry_counts[cluster_index] = first == NONE ? 0 : (short)(lights_port.entry_count - first);
	}
	if (lights_port.cluster_first_entries[cluster_index] == NONE)
		return FALSE;
	*first_entry = lights_port.cluster_first_entries[cluster_index];
	*entry_count = lights_port.cluster_entry_counts[cluster_index];
	return TRUE;
}

static void find_point_lights_for_object_in_cluster(
	long object_index,
	short cluster_index,
	real_point3d const *center,
	real radius,
	long *light_indices,
	real *light_intensities,
	real *light_attenuations,
	short *light_count,
	short maximum_light_count)
{
	long light_index;
	long reference_index;
	short first_entry, entry_count;

	match_assert(
		"c:\\halo\\SOURCE\\objects\\object_lights.c",
		0x544,
		lights_globals.marker_initialized);
	/* port: the cluster's lights as kept (lights_port_cluster). A light too
	far to reach the object is passed over: the Xbox's loop tested it the
	same, on the same position and radius, and rejected it. Left unmarked, it
	is tested again in the object's other clusters, and rejected again.
	Most are far along one axis, which says so without the distance: a
	distance is never less than one axis of it but for rounding, a few parts
	in 2^24, well inside the margin here. */
	if (lights_port_cluster(cluster_index, &first_entry, &entry_count))
	{
		short entry_index;

		for (entry_index = first_entry; entry_index < first_entry + entry_count; entry_index++)
		{
			struct lights_port_entry const *entry = &lights_port.entries[entry_index];
			real reach = radius + entry->radius;
			real bound = reach * 1.000001f;
			real offset;

			offset = entry->position.x - center->x;
			if (offset > bound || -offset > bound)
				continue;
			offset = entry->position.y - center->y;
			if (offset > bound || -offset > bound)
				continue;
			offset = entry->position.z - center->z;
			if (offset > bound || -offset > bound)
				continue;
			if (!(distance3d(&entry->position, center) < reach))
				continue;
			find_point_light_for_object(object_index, entry->light_index, center, radius, light_indices,
				light_intensities, light_attenuations, light_count, maximum_light_count);
		}
		return;
	}
	for (light_index = cluster_partition_get_first_datum(&light_cluster_partition, &reference_index, cluster_index);
		light_index != NONE;
		light_index = cluster_partition_get_next_datum(&light_cluster_partition, &reference_index))
	{
		find_point_light_for_object(object_index, light_index, center, radius, light_indices, light_intensities,
			light_attenuations, light_count, maximum_light_count);
	}

	return;
}

void lights_illumination_at_point(
	real_point3d const *point,
	struct location const *location,
	real_rgb_color *color)
{
	*color = *global_real_rgb_black;
	{
		short lightmap_index;
		short material_index;
		long surface_index;
		real s;
		real t;
		real_point3d collision_point;

		if (structure_test_vector(
			point,
			&lightmap_sample_raycast_down,
			&collision_point,
			&lightmap_index,
			&material_index,
			&surface_index,
			&s,
			&t))
		{
			struct structure_bsp *structure = global_structure_bsp_get();
			struct structure_lightmap *lightmap = TAG_BLOCK_GET_ELEMENT(
				&structure->lightmaps,
				lightmap_index,
				struct structure_lightmap);
			struct structure_material *material = TAG_BLOCK_GET_ELEMENT(
				&lightmap->materials,
				material_index,
				struct structure_material);

			if (structure->lightmap_group.index != NONE
				&& lightmap->bitmap_index != NONE)
			{
				struct bitmap_data *bitmap = bitmap_group_try_and_get_bitmap(
					structure->lightmap_group.index,
					lightmap->bitmap_index);
				struct structure_surface const *surface = TAG_BLOCK_GET_ELEMENT(
					&structure->surfaces,
					surface_index,
					struct structure_surface);

				match_assert(
					"c:\\halo\\SOURCE\\objects\\object_lights.c",
					854,
					material->lightmap_vertices.type==_rasterizer_vertex_type_environment_lightmap_uncompressed || material->lightmap_vertices.type==_rasterizer_vertex_type_environment_lightmap_compressed);
				if (_texture_cache_bitmap_get_hardware_format(bitmap, FALSE, FALSE))
				{
					sample_lightmap(material, bitmap, surface, s, t, color);
				}
			}
		}
	}

	if (location->cluster_index != NONE)
	{
		long light_indices[MAXIMUM_RENDERED_POINT_LIGHTS];
		real light_intensities[MAXIMUM_RENDERED_POINT_LIGHTS];
		real light_attenuations[MAXIMUM_RENDERED_POINT_LIGHTS];
		short light_count = 0;
		short light_index;

		light_marker_begin();
		find_point_lights_for_object_in_cluster(
			NONE,
			location->cluster_index,
			point,
			0.0f,
			light_indices,
			light_intensities,
			light_attenuations,
			&light_count,
			MAXIMUM_RENDERED_POINT_LIGHTS);
		light_marker_end();
		for (light_index = 0; light_index < light_count; light_index++)
		{
			struct light_datum *light = light_get(light_indices[light_index]);

			if (TEST_FLAG(light->flags, _point_light_dynamic_bit))
			{
				color->red += light->color.red * light_attenuations[light_index];
				color->green += light->color.green * light_attenuations[light_index];
				color->blue += light->color.blue * light_attenuations[light_index];
			}
		}
	}

	color->red = PIN(color->red, 0.0f, 1.0f);
	color->green = PIN(color->green, 0.0f, 1.0f);
	color->blue = PIN(color->blue, 0.0f, 1.0f);

	return;
}

void lights_prepare_for_object_dynamic(
	long object_index,
	struct render_lighting *lighting)
{
	struct object_cluster_iterator iterator;
	real_point3d center;
	real radius;
	real light_intensities[MAXIMUM_RENDERED_POINT_LIGHTS];
	real light_attenuations[MAXIMUM_RENDERED_POINT_LIGHTS];
	short cluster_index;
	short light_index;

	object_get_bounding_sphere(object_index, &center, &radius);
	lighting->point_light_count = 0;
	light_marker_begin();
	for (cluster_index = object_get_first_cluster(&iterator, object_index);
		cluster_index != NONE;
		cluster_index = object_get_next_cluster(&iterator, object_index))
	{
		find_point_lights_for_object_in_cluster(
			object_index,
			cluster_index,
			&center,
			radius,
			lighting->point_light_indices,
			light_intensities,
			light_attenuations,
			&lighting->point_light_count,
			MAXIMUM_RENDERED_POINT_LIGHTS);
	}
	light_marker_end();
	/* (port: and a light the window had no room for, with no rasterizer
	index, lights nothing: left out) */
	{
		short kept = 0;

		for (light_index = 0; light_index < lighting->point_light_count; light_index++)
		{
			long rasterizer_light_index =
				light_get(lighting->point_light_indices[light_index])->rasterizer_light_index;

			if (rasterizer_light_index != NONE)
				lighting->point_light_indices[kept++] = rasterizer_light_index;
		}
		lighting->point_light_count = kept;
	}

	return;
}

static void build_distant_lights(
	long flags,
	real_vector3d const *surface_normal,
	real_rgb_color const *diffuse_color,
	real_vector3d const *radiosity_normal,
	real radiosity_accuracy,
	real_rgb_color const *lightmap_color,
	struct render_lighting *lighting)
{
	real brightness = real_rgb_color_brightness(lightmap_color);
	real accuracy, horizontal_length, ambient;

	lighting->ambient_color.red = lightmap_color->red * object_light_ambient_scale + object_light_ambient_base;
	lighting->ambient_color.green = lightmap_color->green * object_light_ambient_scale + object_light_ambient_base;
	lighting->ambient_color.blue = lightmap_color->blue * object_light_ambient_scale + object_light_ambient_base;
	lighting->distant_light_count = 2;
	lighting->distant_lights[0].color = *lightmap_color;
	lighting->distant_lights[0].direction.i = -radiosity_normal->i;
	lighting->distant_lights[0].direction.j = -radiosity_normal->j;
	lighting->distant_lights[0].direction.k = -radiosity_normal->k;
	lighting->distant_lights[1].color.red = diffuse_color->red * object_light_secondary_scale * brightness;
	lighting->distant_lights[1].color.green = diffuse_color->green * object_light_secondary_scale * brightness;
	lighting->distant_lights[1].color.blue = diffuse_color->blue * object_light_secondary_scale * brightness;
	lighting->distant_lights[1].direction = *surface_normal;
	lighting->reflection_tint_color.alpha = PIN(brightness * 1.5f + 0.25f, 0.0f, 1.0f);
	lighting->reflection_tint_color.red = PIN(diffuse_color->red * 3.0f + 0.5f, 0.0f, 1.0f);
	lighting->reflection_tint_color.green = PIN(diffuse_color->green * 3.0f + 0.5f, 0.0f, 1.0f);
	lighting->reflection_tint_color.blue = PIN(diffuse_color->blue * 3.0f + 0.5f, 0.0f, 1.0f);
	lighting->reflection_tint_color.red *= PIN(lightmap_color->red * 2.0f + 0.25f, 0.0f, 1.0f);
	lighting->reflection_tint_color.green *= PIN(lightmap_color->green * 2.0f + 0.25f, 0.0f, 1.0f);
	lighting->reflection_tint_color.blue *= PIN(lightmap_color->blue * 2.0f + 0.25f, 0.0f, 1.0f);
	accuracy = power(radiosity_accuracy, 0.25f);
	lighting->shadow_vector.i = lighting->distant_lights[0].direction.i * accuracy;
	lighting->shadow_vector.j = lighting->distant_lights[0].direction.j * accuracy;
	horizontal_length = magnitude2d((real_vector2d *)&lighting->shadow_vector);
	if (horizontal_length < 0.707f)
	{
		lighting->shadow_vector.k = -square_root(1.0f - horizontal_length * horizontal_length);
	}
	else
	{
		lighting->shadow_vector.k = -0.707f;
		horizontal_length = 0.707f / horizontal_length;
		lighting->shadow_vector.i *= horizontal_length;
		lighting->shadow_vector.j *= horizontal_length;
	}
	ambient = (1.0f - radiosity_accuracy) * 0.5f;
	lighting->shadow_color.red = PIN(1.0f - lighting->distant_lights[0].color.red * 1.3f + ambient,
		object_light_ambient_base, 1.0f);
	lighting->shadow_color.green = PIN(1.0f - lighting->distant_lights[0].color.green * 1.3f + ambient,
		object_light_ambient_base, 1.0f);
	lighting->shadow_color.blue = PIN(1.0f - lighting->distant_lights[0].color.blue * 1.3f + ambient,
		object_light_ambient_base, 1.0f);
	if (TEST_FLAG(flags, _distant_lighting_brighten_bit))
	{
		brighten_real_rgb_color(&lighting->ambient_color, 0.2f);
		brighten_real_rgb_color(&lighting->distant_lights[0].color, 0.3f);
		brighten_real_rgb_color(&lighting->distant_lights[1].color, 0.2f);
		brighten_real_rgb_color(&lighting->reflection_tint_color.rgb, 0.5f);
		lighting->reflection_tint_color.alpha = 1.0f;
	}

	return;
}

boolean lights_distant_lighting_at_point(
	long flags,
	real_point3d const *position,
	struct render_lighting *lighting)
{
	boolean result = FALSE;
	real s;
	real t;
	short lightmap_index;
	short material_index;
	real lengths[3];
	long surface_index;
	real_rgb_color lightmap_color;
	real_vector3d normals[3];
	real_vector3d radiosity_normal;
	real_vector3d surface_normal;
	real_rgb_color diffuse_color;
	real_point3d collision_point;
	struct structure_bsp *structure_bsp = global_structure_bsp_get();
	real_vector3d const *raycasts;
	short raycast_count;
	short raycast_index;

	if (structure_bsp->default_lighting.ambient_color.red != 0.0f)
	{
		*lighting = structure_bsp->default_lighting;
		lighting->distant_light_count = 2;
	}
	else
	{
		*lighting = default_object_lighting;
	}
	if (TEST_FLAG(flags, _distant_lighting_raycast_sideways_bit))
	{
		raycasts = lightmap_sample_raycast_sideways;
		raycast_count = NUMBER_OF_LIGHTMAP_SAMPLE_SIDEWAYS_RAYCASTS;
	}
	else
	{
		raycasts = &lightmap_sample_raycast_down;
		raycast_count = 1;
	}
	for (raycast_index = 0; raycast_index < raycast_count; raycast_index++)
	{
		if (structure_test_vector(
			position,
			&raycasts[raycast_index],
			&collision_point,
			&lightmap_index,
			&material_index,
			&surface_index,
			&s,
			&t))
		{
			struct structure_lightmap *lightmap;
			struct structure_material *material;
			struct shader *shader;

			structure_bsp = global_structure_bsp_get();
			lightmap = TAG_BLOCK_GET_ELEMENT(
				&structure_bsp->lightmaps,
				lightmap_index,
				struct structure_lightmap);
			material = TAG_BLOCK_GET_ELEMENT(
				&lightmap->materials,
				material_index,
				struct structure_material);
			shader = shader_definition_get(material->shader.index);
			if (shader->base.type == _shader_type_environment)
			{
				struct shader_environment_definition *shader_environment =
					shader_environment_definition_get(shader);

				if (structure_bsp->lightmap_group.index != NONE
					&& lightmap->bitmap_index != NONE
					&& shader_environment->base_map.index != NONE)
				{
					struct structure_surface const *surface = TAG_BLOCK_GET_ELEMENT(
						&structure_bsp->surfaces,
						surface_index,
						struct structure_surface);
					struct bitmap_data *lightmap_bitmap = bitmap_group_try_and_get_bitmap(
						structure_bsp->lightmap_group.index,
						lightmap->bitmap_index);
					struct bitmap_data *diffuse_bitmap = bitmap_group_try_and_get_bitmap(
						shader_environment->base_map.index,
						(short)(material->permutation_index
							% bitmap_group_get(shader_environment->base_map.index)->bitmaps.count));

					if (lightmap_bitmap != NULL
						&& diffuse_bitmap != NULL
						&& texture_cache_bitmap_load(lightmap_bitmap) != NULL
						&& texture_cache_bitmap_load(diffuse_bitmap) != NULL)
					{
						real radiosity_accuracy;

						sample_diffuse_texture(
							material,
							diffuse_bitmap,
							surface,
							s,
							t,
							&diffuse_color);
						sample_lightmap(
							material,
							lightmap_bitmap,
							surface,
							s,
							t,
							&lightmap_color);
						environment_vertex_compressed_get_normal(
							structure_material_get_vertex(material, surface->vertex_indices[0]),
							&normals[0]);
						environment_vertex_compressed_get_normal(
							structure_material_get_vertex(material, surface->vertex_indices[1]),
							&normals[1]);
						environment_vertex_compressed_get_normal(
							structure_material_get_vertex(material, surface->vertex_indices[2]),
							&normals[2]);
						shade_vector3d(
							&normals[0],
							&normals[1],
							&normals[2],
							s,
							t,
							&surface_normal);
						normalize3d(&surface_normal);
						environment_lightmap_vertex_compressed_get_incident_radiosity(
							structure_material_get_lightmap_vertex(material, surface->vertex_indices[0]),
							&normals[0]);
						environment_lightmap_vertex_compressed_get_incident_radiosity(
							structure_material_get_lightmap_vertex(material, surface->vertex_indices[1]),
							&normals[1]);
						environment_lightmap_vertex_compressed_get_incident_radiosity(
							structure_material_get_lightmap_vertex(material, surface->vertex_indices[2]),
							&normals[2]);
						lengths[0] = normalize3d(&normals[0]);
						lengths[1] = normalize3d(&normals[1]);
						lengths[2] = normalize3d(&normals[2]);
						shade_vector3d(
							&normals[0],
							&normals[1],
							&normals[2],
							s,
							t,
							&radiosity_normal);
						radiosity_accuracy = shade_scalar(lengths[0], lengths[1], lengths[2], s, t);
						normalize3d(&radiosity_normal);
						if (debug_object_lights)
						{
							real_argb_color color;

							color.rgb = lightmap_color;
							color.alpha = 1.0f;
							render_debug_point(
								TRUE,
								position,
								0.5f,
								&color);
							render_debug_vector(
								TRUE,
								position,
								&radiosity_normal,
								radiosity_accuracy,
								&color);
						}
						build_distant_lights(
							flags,
							&surface_normal,
							&diffuse_color,
							&radiosity_normal,
							radiosity_accuracy,
							&lightmap_color,
							lighting);
						result = TRUE;
					}
				}
			}

			break;
		}
	}

	return result;
}

void lights_prepare_for_object_static(
	long object_index,
	struct render_lighting *lighting)
{
	struct object_datum *object = object_get(object_index);
	long flags = 0;
	struct render_lighting sample;
	short sample_count;
	short corner_index;
	boolean sampled;

	match_assert(
		"c:\\halo\\SOURCE\\objects\\object_lights.c",
		970,
		lighting);
	if (TEST_FLAG(object->object.flags, _object_static_lighting_raycast_sideways_bit))
	{
		SET_FLAG(flags, _distant_lighting_raycast_sideways_bit, TRUE);
	}
	if (TEST_FLAG(object_definition_get(object->definition_index)->object.flags,
		_object_artificially_bright_bit))
	{
		SET_FLAG(flags, _distant_lighting_brighten_bit, TRUE);
	}
	sampled = lights_distant_lighting_at_point(
		flags,
		&object->object.bounding_sphere_center,
		lighting);
	if (TEST_FLAG(object->object.flags, _object_static_lighting_recompute_bit))
	{
		return;
	}
	if (sampled)
	{
		sample_count = 1;
	}
	else
	{
		sample_count = 0;
		csmemset(lighting, 0, sizeof(struct render_lighting));
		lighting->distant_light_count = 2;
	}
	for (corner_index = 0; corner_index < 4; corner_index++)
	{
		real_point3d point;

		point.x = (TEST_FLAG(corner_index, 0) ? 0.70710678f : -0.70710678f)
			* object->object.bounding_sphere_radius
			+ object->object.bounding_sphere_center.x;
		point.y = (TEST_FLAG(corner_index, 1) ? 0.70710678f : -0.70710678f)
			* object->object.bounding_sphere_radius
			+ object->object.bounding_sphere_center.y;
		point.z = object->object.bounding_sphere_center.z;
		if (lights_distant_lighting_at_point(
			flags,
			&point,
			&sample))
		{
			sample_count++;
			lighting->ambient_color.red += sample.ambient_color.red;
			lighting->ambient_color.green += sample.ambient_color.green;
			lighting->ambient_color.blue += sample.ambient_color.blue;
			lighting->reflection_tint_color.alpha += sample.reflection_tint_color.alpha;
			lighting->reflection_tint_color.red += sample.reflection_tint_color.red;
			lighting->reflection_tint_color.green += sample.reflection_tint_color.green;
			lighting->reflection_tint_color.blue += sample.reflection_tint_color.blue;
			lighting->distant_lights[0].color.red += sample.distant_lights[0].color.red;
			lighting->distant_lights[0].color.green += sample.distant_lights[0].color.green;
			lighting->distant_lights[0].color.blue += sample.distant_lights[0].color.blue;
			lighting->distant_lights[0].direction.i += sample.distant_lights[0].direction.i;
			lighting->distant_lights[0].direction.j += sample.distant_lights[0].direction.j;
			lighting->distant_lights[0].direction.k += sample.distant_lights[0].direction.k;
			lighting->distant_lights[1].color.red += sample.distant_lights[1].color.red;
			lighting->distant_lights[1].color.green += sample.distant_lights[1].color.green;
			lighting->distant_lights[1].color.blue += sample.distant_lights[1].color.blue;
			lighting->distant_lights[1].direction.i += sample.distant_lights[1].direction.i;
			lighting->distant_lights[1].direction.j += sample.distant_lights[1].direction.j;
			lighting->distant_lights[1].direction.k += sample.distant_lights[1].direction.k;
			lighting->shadow_color.red += sample.shadow_color.red;
			lighting->shadow_color.green += sample.shadow_color.green;
			lighting->shadow_color.blue += sample.shadow_color.blue;
			lighting->shadow_vector.i += sample.shadow_vector.i;
			lighting->shadow_vector.j += sample.shadow_vector.j;
			lighting->shadow_vector.k += sample.shadow_vector.k;
		}
	}
	if (sample_count > 1)
	{
		real scale = 1.0f / sample_count;

		lighting->ambient_color.red *= scale;
		lighting->ambient_color.green *= scale;
		lighting->ambient_color.blue *= scale;
		lighting->reflection_tint_color.alpha *= scale;
		lighting->reflection_tint_color.red *= scale;
		lighting->reflection_tint_color.green *= scale;
		lighting->reflection_tint_color.blue *= scale;
		lighting->distant_lights[0].color.red *= scale;
		lighting->distant_lights[0].color.green *= scale;
		lighting->distant_lights[0].color.blue *= scale;
		scale_vector3d(&lighting->distant_lights[0].direction, scale, &lighting->distant_lights[0].direction);
		normalize3d(&lighting->distant_lights[0].direction);
		lighting->distant_lights[1].color.red *= scale;
		lighting->distant_lights[1].color.green *= scale;
		lighting->distant_lights[1].color.blue *= scale;
		scale_vector3d(&lighting->distant_lights[1].direction, scale, &lighting->distant_lights[1].direction);
		normalize3d(&lighting->distant_lights[1].direction);
		lighting->shadow_color.red *= scale;
		lighting->shadow_color.green *= scale;
		lighting->shadow_color.blue *= scale;
		scale_vector3d(&lighting->shadow_vector, scale, &lighting->shadow_vector);
		normalize3d(&lighting->shadow_vector);
	}
	else if (sample_count == 0)
	{
		*lighting = sample;
	}

	return;
}
