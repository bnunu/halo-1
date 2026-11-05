/*
D3D8_GL.C

The Xbox Direct3D 8 device, implemented with OpenGL 4.5.

The game drives the device through the XDK's inline functions, which keep
the "simple" render states in D3D__RenderState and call into this file for
everything else. At each draw the full state is read back from there and
translated: the vertex program into GLSL once per shader (nv2a_vsh.c), the
pixel shader - texture stages and register combiners, 57 render states -
into GLSL once per combination (nv2a_psh.c), and the rest into GL state.

Conventions carried over from the Xbox:
- Clip space is D3D's (depth 0..1, y down in window space). glClipControl
  (GL_UPPER_LEFT, GL_ZERO_TO_ONE) makes GL agree, so viewports, scissors and
  texture rows line up with D3D's top-left origin; the window blit at
  Present flips the image back for display.
- Render targets and textures are identified by the physical address in
  their Data field. A texture whose data is a render target samples the GL
  render target directly (render-to-texture).
- Vertex data is read from guest memory at draw time.
*/

#include "xgpu.h"
#include "sdl_platform.h"
#include "halo_ui_pointer.h"
#include "port_config.h"
#include "posix.h"
#include "main/console.h"
#ifdef HALO_ANDROID
#include "../../android/include/android_render_policy.h"
#endif

#include <SDL3/SDL.h>

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

void d3d8_surface_initialize(D3DSurface *surface, D3DFORMAT format, unsigned long width, unsigned long height);
void d3d8_surface_resize(D3DSurface *surface, D3DFORMAT format, unsigned long width, unsigned long height);

/* port/linux/game/stats_log.c: display.stats_log's line names the renderer the
driver reports */
void stats_log_set_renderer(const char *vendor, const char *renderer);

#ifdef HALO_GLES
/* OpenGL ES 3 (port/android/README.md): the desktop formats, enumerants
and entry points used below that ES lacks */
#define GL_BGRA GL_RGBA
#define glDepthRange glDepthRangef
#define glClearDepth glClearDepthf
#ifndef GL_TEXTURE_MAX_ANISOTROPY_EXT
#define GL_TEXTURE_MAX_ANISOTROPY_EXT 0x84fe
#endif
#ifndef GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT
#define GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT 0x84ff
#endif
#ifndef GL_TEXTURE_BORDER_COLOR
#define GL_TEXTURE_BORDER_COLOR 0x1004
#endif
#ifndef GL_CLAMP_TO_BORDER
#define GL_CLAMP_TO_BORDER 0x812d
#endif

/* what the context supports (gl_initialize) */
struct xgpu_capabilities xgpu_capabilities;
#endif

/* display.reversed_depth (xgpu.h), as the context was made; never on
OpenGL ES, which has no glClipControl: its depth from -1 would lose the
precision near 0 that reversing gains */
BOOL xgpu_reversed_depth;

/* ---------- the screen's width

The Xbox screen is 640x480. The native ports can draw a wider one: 480
lines, and as many columns as the display's shape gives. On Android that is
display.screen_width (port_config.c; 640 keeps 4:3); on the desktop, the
shape of the window (of the display while the game is fullscreen, or of
display.resolution), and 640 where display.resolution_scaling is "original".
The game's camera derives its horizontal field of view from the viewport, so
the 3D view simply widens. The menus and full-screen overlays are laid out
for 640 columns; while they draw (halo_screen_ui_offset), everything shifts
right to center them.

The desktop also draws at that resolution (platform_screen_mode), and
Android at the window's where display.resolution_scaling asks for it: render
targets the size of the screen get that many pixels (screen_scale), and
viewports, clears and visibility counts are scaled to match, so the game
still works in its 480 lines; "75%", "50%" and "25%" draw that part of the
resolution each way, and "original" (Android's default) the 480 lines, scaled
up at presentation. The width and the scale change only between frames,
after one is presented (halo_screen_commit). */

#define SCREEN_HEIGHT 480
#define SCREEN_MAXIMUM_WIDTH 1920

/* the width the game draws, 0 until first asked, and how many pixels a
render target the size of the screen has per unit of it */
static long screen_width;
static float screen_scale[2] = { 1.0f, 1.0f };
/* how many times the display's pixels each way supersampling draws (with
the scale screen_mode_choose last chose) */
static float screen_supersampling = 1.0f;
static long ui_offset;
#define UI_OFFSET ((GLint)ui_offset)

/* ---------- anti-aliasing

display.anti_aliasing, off unless it is set: the Xbox drew without any.
"fxaa" and "smaa" are passes over each window's 3D view before the HUD and
menus are drawn over it (halo_screen_anti_alias, xgpu_post.c); "ssaa2x"
draws the screen's targets at twice the resolution each way
(screen_mode_choose), which the display blit scales down; "msaa2x" to
"msaa8x" draw the back buffer and its depth buffer with that many samples a
pixel (render_target_multisample). The setting is read again between frames
(halo_screen_commit), so that a change applies from the next one. */

enum
{
	_anti_aliasing_off,
	_anti_aliasing_fxaa,
	_anti_aliasing_smaa,
	_anti_aliasing_ssaa,
	_anti_aliasing_msaa,
};

static const struct
{
	const char *name;
	int mode;
	int samples;
} anti_aliasing_values[] =
{
	{ "off", _anti_aliasing_off, 0 },
	{ "fxaa", _anti_aliasing_fxaa, 0 },
#ifdef HALO_GLES
	/* (SMAA's three passes and supersampling's four times the pixels are
	more than a phone's GPU has to spare: FXAA in SMAA's place, and none) */
	{ "smaa", _anti_aliasing_fxaa, 0 },
	{ "ssaa2x", _anti_aliasing_off, 0 },
#else
	{ "smaa", _anti_aliasing_smaa, 0 },
	{ "ssaa2x", _anti_aliasing_ssaa, 0 },
#endif
	{ "msaa2x", _anti_aliasing_msaa, 2 },
	{ "msaa4x", _anti_aliasing_msaa, 4 },
	{ "msaa8x", _anti_aliasing_msaa, 8 },
};

#define NUMBER_OF_ANTI_ALIASING_VALUES ((int)(sizeof(anti_aliasing_values) / sizeof(anti_aliasing_values[0])))

/* the value in effect, -1 until the setting is first read; multisampling's
samples a pixel, at most the GPU's (anti_aliasing_prepare); and the GPU's
most samples and largest target (texture and renderbuffer), 0 until the GL
context exists */
static int anti_aliasing_value = -1;
static int anti_aliasing_samples;
static GLint anti_aliasing_maximum_samples, maximum_target_size;

static void anti_aliasing_prepare(void);

/* display.anti_aliasing's value (none of them: the first, off) */
static void anti_aliasing_read(void)
{
	const char *setting = config_string("display.anti_aliasing");
	int value;

	for (value = NUMBER_OF_ANTI_ALIASING_VALUES - 1;
		value > 0 && strcmp(setting, anti_aliasing_values[value].name);
		value--)
	{
	}
	if (value == anti_aliasing_value)
		return;
	anti_aliasing_value = value;
	if (strcmp(setting, anti_aliasing_values[value].name))
		platform_log("anti-aliasing: \"%s\" is unknown, so off", setting);
	else
		platform_log("anti-aliasing: %s", setting);
	anti_aliasing_prepare();
}

static int anti_aliasing(void)
{
	if (anti_aliasing_value < 0)
		anti_aliasing_read();
	return anti_aliasing_values[anti_aliasing_value].mode;
}

/* display.resolution_scaling's part of the resolution each way: 1 for
"native", 0.75, 0.5 or 0.25 for "75%", "50%" or "25%", 0 for "original" (the
Xbox's 480 lines) */
static float resolution_scaling_part(void)
{
	const char *setting = config_string("display.resolution_scaling");

	return !strcmp(setting, "original") ? 0.0f : !strcmp(setting, "75%") ? 0.75f :
		!strcmp(setting, "50%") ? 0.5f : !strcmp(setting, "25%") ? 0.25f : 1.0f;
}

static void screen_mode_choose(long *width, float scale[2])
{
#ifdef HALO_ANDROID
	/* display.screen_width, or 0 for the display's shape, which the app
	passes (port/android/host/host_main.c) */
	const char *display = getenv("HALO_DISPLAY_WIDTH");
	/* (the window's size last given, for a window that has none: before it
	opens, or while the app is away) */
	static int last_width, last_height;
	float part = resolution_scaling_part();
	int display_width = 0, display_height = 0;

	*width = config_integer("display.screen_width");
	if (*width <= 0)
		*width = display ? atol(display) : 640;
	if (*width < 640)
		*width = 640;
	if (*width > 1600)
		*width = 1600;
	*width &= ~1L;
	scale[0] = scale[1] = 1.0f;
	screen_supersampling = 1.0f;
	/* display.resolution_scaling: "original" draws the 480 lines, which the
	display blit scales up; "native" the window's pixels; "75%", "50%" or
	"25%" that part of them each way. The picture keeps its shape: a width
	of another (display.screen_width) is letterboxed. */
	if (part <= 0.0f)
		return;
	platform_video_drawable_size(&display_width, &display_height);
	if (display_width > 0 && display_height > 0)
	{
		last_width = display_width;
		last_height = display_height;
	}
	if (last_width <= 0 || last_height <= 0)
		return;
	scale[0] = (float)last_width / (float)*width;
	scale[1] = (float)last_height / (float)SCREEN_HEIGHT;
	if (*width != ((SCREEN_HEIGHT * last_width / last_height) & ~1L))
		scale[0] = scale[1] = scale[0] < scale[1] ? scale[0] : scale[1];
	scale[0] *= part;
	scale[1] *= part;
	/* (no more than the GPU's largest target, once it is known) */
	if (maximum_target_size > 0 && (*width * scale[0] > (float)maximum_target_size ||
		SCREEN_HEIGHT * scale[1] > (float)maximum_target_size))
	{
		float fit = (float)maximum_target_size / (*width * scale[0]);

		if ((float)maximum_target_size / (SCREEN_HEIGHT * scale[1]) < fit)
			fit = (float)maximum_target_size / (SCREEN_HEIGHT * scale[1]);
		scale[0] *= fit;
		scale[1] *= fit;
	}
#else
	long display_width, display_height;
	float part = resolution_scaling_part();

	*width = 640;
	scale[0] = scale[1] = 1.0f;
	screen_supersampling = 1.0f;
	if (platform_screen_mode(&display_width, &display_height) && display_width > 0 && display_height > 0)
	{
		long wanted = (SCREEN_HEIGHT * display_width + display_height / 2) / display_height;

		*width = wanted < 640 ? 640 : wanted > SCREEN_MAXIMUM_WIDTH ? SCREEN_MAXIMUM_WIDTH : wanted & ~1L;
		scale[0] = (float)display_width / (float)*width;
		scale[1] = (float)display_height / (float)SCREEN_HEIGHT;
		/* a display narrower or wider than the game can be: the picture
		keeps its shape and the display blit letterboxes it */
		if (*width != wanted && *width != (wanted & ~1L))
			scale[0] = scale[1] = scale[0] < scale[1] ? scale[0] : scale[1];
		/* display.resolution_scaling's "75%", "50%" or "25%": that part of
		the resolution each way, which the display blit scales up */
		if (part > 0.0f)
		{
			scale[0] *= part;
			scale[1] *= part;
		}
		/* supersampling: twice the pixels each way, or as many as the GPU's
		largest target has (once it is known), which the display blit
		scales down; not with "original" resolution scaling, which draws
		the Xbox's 640x480 */
		if (anti_aliasing() == _anti_aliasing_ssaa && maximum_target_size > 0)
		{
			float factor = 2.0f;

			if (*width * scale[0] * factor > (float)maximum_target_size)
				factor = (float)maximum_target_size / (*width * scale[0]);
			if (SCREEN_HEIGHT * scale[1] * factor > (float)maximum_target_size)
				factor = (float)maximum_target_size / (SCREEN_HEIGHT * scale[1]);
			if (factor > 1.0f)
			{
				scale[0] *= factor;
				scale[1] *= factor;
				screen_supersampling = factor;
			}
		}
	}
#endif
}

long halo_screen_width(void)
{
	if (!screen_width)
	{
		screen_mode_choose(&screen_width, screen_scale);
		platform_log("screen: %ldx%d drawn at %.0fx%.0f", screen_width, SCREEN_HEIGHT,
			screen_width * screen_scale[0], SCREEN_HEIGHT * screen_scale[1]);
	}
	return screen_width;
}

/* the display's pixels for each of the 480 lines (text_hires.c) */
float halo_screen_pixel_scale(void)
{
	halo_screen_width();
	return screen_scale[1];
}

/* display.model_detail: the size a model is measured at to choose its
level of detail (source/models/models.c).

The game measures a model's bounding sphere in the lines of its 480-line
screen (render_frustum_sphere_diameter_in_pixels), whatever the display has,
and compares that with the pixel sizes each level is meant for: on a display
of 1440 lines a model three times the size the levels were chosen for is
drawn as the Xbox drew it at 480. "stock" keeps that; "native" measures in
the display's lines (not supersampling's, which the display scales down;
"original" resolution scaling draws 480); "highest" measures as "native"
which models are too small to draw, and draws the others at their most
detailed level. Other than "stock", each permutation then draws the level
that port/linux/game/model_detail.c chooses from the model's. level: the
scale for choosing the level, otherwise for whether to draw at all. */
enum
{
	_model_detail_stock,
	_model_detail_native,
	_model_detail_highest,
};

static int model_detail = -1;
static unsigned long model_detail_read_at;

static int model_detail_setting(void)
{
	if (model_detail < 0 || model_detail_read_at != config_changes())
	{
		const char *setting = config_string("display.model_detail");
		int value = !strcmp(setting, "native") ? _model_detail_native :
			!strcmp(setting, "highest") ? _model_detail_highest : _model_detail_stock;

		model_detail_read_at = config_changes();
		if (value != model_detail)
		{
			if (value == _model_detail_stock && strcmp(setting, "stock"))
				platform_log("model detail: \"%s\" is unknown, so stock", setting);
			else
				platform_log("model detail: %s", setting);
		}
		model_detail = value;
	}
	return model_detail;
}

int halo_model_detail_stock(void)
{
	return model_detail_setting() == _model_detail_stock;
}

float halo_model_detail_scale(int level)
{
	float display_scale;

	model_detail_setting();
	if (model_detail == _model_detail_stock)
		return 1.0f;
	if (model_detail == _model_detail_highest && level)
		return 1.0e6f;
	halo_screen_width();
	display_scale = screen_scale[1] / screen_supersampling;
	return display_scale > 1.0f ? display_scale : 1.0f;
}

/* how many pixels the screen's targets draw to the Xbox's one, the larger
of the two ways (the screen effects' convolutions: rasterizer_xbox_screen_effect.c) */
float halo_screen_scale(void)
{
	halo_screen_width();
	return screen_scale[0] > screen_scale[1] ? screen_scale[0] : screen_scale[1];
}

/* display.shadow_resolution: the size the shadow maps are drawn at.

Each object's shadow is drawn from above into a 128x128 map, blurred into
another and projected onto the level under it (rasterizer_xbox_shadows.c).
On a large screen, a shadow's 128 texels show as steps along its edge, which
crawl as the object moves. The two maps, the game's only R5G6B5 render
targets (rasterizer_xbox.c), can be drawn larger as the screen's targets are
(render_target_get): the game's viewports, clears and quads, in its 128 units,
scale up with them. The scale is a power of two up to 8 (1024x1024), which
the blur needs to cover the same part of the map as the Xbox's
(rasterizer_shadow_convolve); 1, the default, draws them as the Xbox did. It
changes only between frames (halo_screen_commit). */
#define SHADOW_MAP_SIZE 128
#define SHADOW_MAP_MAXIMUM_SCALE 8

/* 0 until first asked */
static long shadow_scale;
static unsigned long shadow_scale_read_at;

static long shadow_scale_choose(void)
{
	long resolution = config_integer("display.shadow_resolution");
	long scale = 1;

	while (scale < SHADOW_MAP_MAXIMUM_SCALE && SHADOW_MAP_SIZE * scale * 2 <= resolution)
		scale *= 2;
	return scale;
}

/* the shadow maps' pixels for each of their 128 texels each way */
long halo_shadow_map_scale(void)
{
	if (!shadow_scale)
	{
		shadow_scale_read_at = config_changes();
		shadow_scale = shadow_scale_choose();
		platform_log("shadow maps: %ldx%ld", SHADOW_MAP_SIZE * shadow_scale, SHADOW_MAP_SIZE * shadow_scale);
	}
	return shadow_scale;
}

/* display.hdr (xgpu_post.c): the back buffer in GL_RGBA16F, which keeps
what the game's additive blending adds past white, tone mapped over each
window's 3D view before the HUD is drawn (halo_screen_tone_map) and put in
the window by a shader (D3DDevice_Present). Taken up between frames
(halo_screen_commit) and as the GL context is made (hdr_read). */
static struct
{
	/* config_changes() when the settings were read, if they have been;
	display.hdr as it was then */
	unsigned long read_at;
	BOOL read, wanted;
	/* display.hdr, where the GPU draws into half floats and the programs
	build: the back buffer is GL_RGBA16F */
	BOOL active;
	struct xgpu_tone_mapping tone_mapping;
	/* the window's pixels are floating point (display.hdr_output): its
	pixels are linear, in scRGB's units, the desktop's white sdr_white */
	BOOL float_window;
	float sdr_white;
	/* the float window's display has HDR on (platform_video_hdr), as the
	last frame found */
	BOOL display_on;
	/* display.hdr_paper_white and display.hdr_peak_brightness, in nits: 0
	for Windows' own (its SDR content brightness, and its HDR calibration's
	peak, as SDL reports them) */
	int paper_white, peak_brightness;
#ifdef HALO_GLES
	/* the most samples a pixel of a GL_RGBA16F renderbuffer */
	GLint maximum_samples;
#endif
} hdr;

void halo_screen_ui_offset(unsigned char centered)
{
	ui_offset = centered ? (halo_screen_width() - 640) / 2 : 0;
}

void halo_screen_ui_offset_view(long view_width)
{
	long width = halo_screen_width();

	/* (a quarter's pause menu and dialogs are 320 wide: the Xbox's) */
	ui_offset = view_width > 0 && view_width < width ? (view_width > 320 ? (view_width - 320) / 2 : 0) : (width - 640) / 2;
}

long halo_screen_ui_offset_current(void)
{
	return ui_offset;
}

/* ---------- state the XDK header's inline functions read and write */

DWORD D3D__RenderState[D3DRS_MAX];
DWORD D3D__TextureState[D3DTSS_MAXSTAGES][D3DTSS_MAX];
WORD *D3D__IndexData;
BYTE D3D__StateBlockDirty[1024];

/* ---------- vertex shaders */

#define VERTEX_SHADER_SIGNATURE 0x76736864UL /* 'vshd' */
#define VERTEX_PROGRAM_SLOTS 136

struct vertex_element
{
	unsigned char reg;
	unsigned char stream;
	unsigned char type;
	unsigned char bytes;
	unsigned short offset;
};

struct vertex_shader_object
{
	unsigned long signature;
	unsigned long id;
	DWORD *instructions;
	unsigned long instruction_count;
	struct vertex_element elements[XGPU_VERTEX_ATTRIBUTE_COUNT];
	unsigned long element_count;
	unsigned long packed_mask;
	/* [0] streams per the declaration, [1] immediate mode (all floats) */
	GLuint shader[2];
	/* one of the game's model lighting programs (halo_vertex_shader_lighting),
	whose draws can be lit for each pixel (display.per_pixel_lighting): where
	its lighting's normal and position are (lighting.lights is 0 for the
	others), and its shaders that hand them on, as shader[] */
	struct nv2a_vertex_lighting lighting;
	GLuint lit_shader[2];
	/* a shader lit for each pixel failed to compile or link: lit as the
	vertex shader lights it from then on */
	BOOL lighting_failed;
#ifndef HALO_GLES
	/* the vertex array its draws last used, and the streams they had
	(setup_streams: the layout follows from the two) */
	struct vertex_array_entry *vertex_array;
	unsigned long vertex_array_streams;
#endif
	/* which of shader[] and lit_shader[] have been compiled (bits 0 and 1,
	and 2 and 3): one that failed stays 0 and is not compiled again at each
	draw */
	unsigned char shaders_tried;
};

/* ---------- programs */

struct fragment_entry
{
	struct fragment_entry *next;
	unsigned long hash;
	struct nv2a_pixel_shader_key key;
	GLuint shader;
};

/* the uniforms a draw sets besides the vertex constants */
struct draw_uniforms
{
	float viewport_scale[4];
	float viewport_offset[4];
	float point_size;
	float ps_c0[8][4];
	float ps_c1[8][4];
	float ps_final_c0[4];
	float ps_final_c1[4];
	float fog_color[4];
	float fog_parameters[4];
	float alpha_reference;
	float bump_matrix[4][4];
	float bump_luminance[4][4];
	float texture_scale[4][4];
	float screen_offset;
	float texture_lod_bias[4];
	float height_fade[4];
	/* display.reversed_depth: a z bias's depth units (nv2a_vsh.c) */
	float depth_bias;
};

struct program_entry
{
	struct program_entry *next;
	GLuint vertex_shader;
	GLuint fragment_shader;
	GLuint program;
	GLint constants;
	GLint viewport_scale;
	GLint viewport_offset;
	GLint point_size;
	GLint ps_c0, ps_c1, ps_final_c0, ps_final_c1;
	GLint fog_color, fog_parameters, alpha_reference;
	GLint bump_matrix, bump_luminance, texture_scale;
	GLint texture_lod_bias;
	GLint screen_offset;
	GLint height_fade;
	GLint depth_bias;
	/* nv2a_pixel_shader_key's source_blend's color, and what it last had */
	GLint blend_constant;
	float blend_constant_value[4];
	/* the lights of a draw lit for each pixel (XGPU_MODEL_LIGHT_COUNT), and
	constants_serial at their last upload */
	GLint model_lights;
	unsigned long long model_lights_serial;

	/* the vertex constants c[0..constant_count) the program uses; with
	consecutive locations, a changed range is uploaded by itself */
	unsigned long constant_count;
	BOOL constants_consecutive;
	/* constants_serial at the program's last constant upload (constants_store) */
	unsigned long long constants_serial;
	/* draw_uniforms_serial when the uniforms below were brought up to date */
	unsigned long uniforms_serial;
	/* what the program's other uniforms hold (all ones: unknown) */
	struct draw_uniforms uniforms;
};

#define FRAGMENT_BUCKETS 1024
#define PROGRAM_BUCKETS 1024

/* ---------- the program cache on disk (desktop)

Linking a program is where the drivers do most of their work: on Intel
graphics on Windows up to 70 ms a program, and a large Custom Edition map
makes a hundred or more as new things come into view, each a stalled frame.
A linked program's binary (glGetProgramBinary) is kept in a file of the
driver's (its vendor, renderer and version) under the save root, by the
hashes of its two shaders' GLSL: the next time the program is wanted, in this
run or another, it is loaded from that instead of linked, in under 2 ms
(display.shader_cache). The binaries share one file, opened once: on Windows
opening or making a file of its own for each took 3-110 ms (the antivirus's
look at it), as much as the link it saved. A binary the driver refuses is
forgotten and the program linked again. OpenGL ES keeps its programs to
itself: its drivers' binaries are not all to be trusted. */

/* each compiled shader's GLSL hash, by its name (shader_hash_set) */
static unsigned long long *shader_hashes;
static GLuint shader_hashes_size;

static unsigned long long source_hash(const char *source)
{
	unsigned long long hash = 14695981039346656037ULL;

	while (*source)
		hash = (hash ^ (unsigned char)*source++) * 1099511628211ULL;
	return hash;
}

static void shader_hash_set(GLuint shader, const char *source)
{
	if (!shader)
		return;
	if (shader >= shader_hashes_size)
	{
		GLuint size = shader_hashes_size ? shader_hashes_size : 256;
		unsigned long long *grown;

		while (size <= shader)
			size *= 2;
		grown = realloc(shader_hashes, size * sizeof(*grown));
		if (!grown)
			return;
		memset(grown + shader_hashes_size, 0, (size - shader_hashes_size) * sizeof(*grown));
		shader_hashes = grown;
		shader_hashes_size = size;
	}
	shader_hashes[shader] = source_hash(source);
}

#ifndef HALO_GLES
static unsigned long long shader_hash(GLuint shader)
{
	return shader < shader_hashes_size ? shader_hashes[shader] : 0;
}

/* the cache file: entries one after another, each a header (magic, the two
shaders' GLSL hashes, the binary's format and length, and its data's hash)
then the binary. Entries are appended as programs are linked; at the start
the file is read through once for its index (the place of each entry), and
a program found in it is read from the open file. Reading stops at the first
entry that is not whole (a write a crash cut short): the next entry is
written over it. */
#define PROGRAM_CACHE_MAGIC 0x31435048 /* "HPC1" */
#define PROGRAM_CACHE_BUCKETS 1024
/* a file grown past this starts again empty (each binary is 10-60 KB) */
#define PROGRAM_CACHE_MAXIMUM_SIZE (64UL * 1024 * 1024)

struct program_cache_header
{
	unsigned long magic;
	unsigned long long vertex_hash;
	unsigned long long fragment_hash;
	unsigned long format;
	unsigned long length;
	unsigned long long data_hash;
};

struct program_cache_entry
{
	struct program_cache_entry *next;
	unsigned long long vertex_hash;
	unsigned long long fragment_hash;
	unsigned long format;
	unsigned long length;
	unsigned long long data_hash;
	/* where its binary starts in the file */
	unsigned long offset;
};

static struct
{
	BOOL enabled;
	FILE *file;
	/* the end of the file's last whole entry: the next is written there */
	unsigned long end;
	struct program_cache_entry *buckets[PROGRAM_CACHE_BUCKETS];
	/* this run's: programs loaded from the cache, linked, written to it,
	and binaries the driver refused */
	unsigned long loaded, linked, written, refused;
} program_cache;

static unsigned long long data_hash(const void *data, unsigned long size)
{
	const unsigned char *bytes = data;
	unsigned long long hash = 14695981039346656037ULL;

	while (size--)
		hash = (hash ^ *bytes++) * 1099511628211ULL;
	return hash;
}

static struct program_cache_entry **program_cache_bucket(unsigned long long vertex_hash,
	unsigned long long fragment_hash)
{
	unsigned long long key = vertex_hash * 0x9e3779b97f4a7c15ULL ^ fragment_hash;

	return &program_cache.buckets[(unsigned long)(key >> 32 ^ key) % PROGRAM_CACHE_BUCKETS];
}

static struct program_cache_entry *program_cache_find(unsigned long long vertex_hash,
	unsigned long long fragment_hash)
{
	struct program_cache_entry *entry;

	for (entry = *program_cache_bucket(vertex_hash, fragment_hash); entry; entry = entry->next)
	{
		if (entry->vertex_hash == vertex_hash && entry->fragment_hash == fragment_hash)
			return entry;
	}
	return NULL;
}

/* the file read through once for its entries' places (their data is read
when it is wanted): a later entry for the same program takes the place of
an earlier */
static void program_cache_index(void)
{
	struct program_cache_header header;
	unsigned long offset = 0, size;

	fseek(program_cache.file, 0, SEEK_END);
	size = (unsigned long)ftell(program_cache.file);
	while (offset + sizeof(header) <= size && fseek(program_cache.file, (long)offset, SEEK_SET) == 0 &&
		fread(&header, sizeof(header), 1, program_cache.file) == 1 && header.magic == PROGRAM_CACHE_MAGIC &&
		header.length && header.length <= size - offset - sizeof(header))
	{
		struct program_cache_entry *entry = program_cache_find(header.vertex_hash, header.fragment_hash);

		if (!entry)
		{
			struct program_cache_entry **bucket = program_cache_bucket(header.vertex_hash, header.fragment_hash);

			entry = calloc(1, sizeof(*entry));
			if (!entry)
				break;
			entry->vertex_hash = header.vertex_hash;
			entry->fragment_hash = header.fragment_hash;
			entry->next = *bucket;
			*bucket = entry;
		}
		entry->format = header.format;
		entry->length = header.length;
		entry->data_hash = header.data_hash;
		entry->offset = offset + sizeof(header);
		offset += sizeof(header) + header.length;
	}
	program_cache.end = offset;
}

static void program_cache_initialize(void)
{
	GLint formats = 0;
	char path[512];
	unsigned long long driver;
	const char *strings[3];
	int index;

	program_cache.enabled = FALSE;
	if (!config_boolean("display.shader_cache"))
		return;
	glGetIntegerv(GL_NUM_PROGRAM_BINARY_FORMATS, &formats);
	if (formats <= 0)
	{
		platform_log("shader cache: off (the driver keeps no program binaries)");
		return;
	}
	strings[0] = (const char *)glGetString(GL_VENDOR);
	strings[1] = (const char *)glGetString(GL_RENDERER);
	strings[2] = (const char *)glGetString(GL_VERSION);
	driver = 14695981039346656037ULL;
	for (index = 0; index < 3; index++)
		driver = (driver ^ source_hash(strings[index] ? strings[index] : "")) * 1099511628211ULL;
	snprintf(path, sizeof(path), "%s/shader_cache", platform_save_root());
	posix_make_directory(path);
	snprintf(path, sizeof(path), "%s/shader_cache/%016llx.bin", platform_save_root(), driver);
	program_cache.file = fopen(path, "r+b");
	if (program_cache.file)
	{
		fseek(program_cache.file, 0, SEEK_END);
		if ((unsigned long)ftell(program_cache.file) > PROGRAM_CACHE_MAXIMUM_SIZE)
		{
			fclose(program_cache.file);
			program_cache.file = NULL;
		}
	}
	if (!program_cache.file)
		program_cache.file = fopen(path, "w+b");
	if (!program_cache.file)
	{
		platform_log("shader cache: off (%s cannot be opened)", path);
		return;
	}
	program_cache_index();
	program_cache.enabled = TRUE;
	platform_log("shader cache: %s, %lu KB", path, program_cache.end / 1024);
}

/* the cached binary of the program of these shaders, loaded into a new
program; 0 if there is none or the driver refuses it (then forgotten: the
program is linked again, and its new binary written after) */
static GLuint program_cache_load(unsigned long long vertex_hash, unsigned long long fragment_hash)
{
	struct program_cache_entry *entry;
	void *binary;
	GLuint program = 0;
	GLint status = 0;

	if (!program_cache.enabled || !vertex_hash || !fragment_hash)
		return 0;
	entry = program_cache_find(vertex_hash, fragment_hash);
	if (!entry || !entry->length || !(binary = malloc(entry->length)))
		return 0;
	if (fseek(program_cache.file, (long)entry->offset, SEEK_SET) == 0 &&
		fread(binary, entry->length, 1, program_cache.file) == 1 &&
		data_hash(binary, entry->length) == entry->data_hash)
	{
		program = glCreateProgram();
		glProgramBinary(program, (GLenum)entry->format, binary, (GLsizei)entry->length);
		glGetProgramiv(program, GL_LINK_STATUS, &status);
		if (!status)
		{
			glDeleteProgram(program);
			program = 0;
		}
	}
	free(binary);
	if (!program)
	{
		program_cache.refused++;
		entry->length = 0;
		return 0;
	}
	program_cache.loaded++;
	return program;
}

/* a linked program's binary, written after the file's last whole entry */
static void program_cache_store(GLuint program, unsigned long long vertex_hash, unsigned long long fragment_hash)
{
	struct program_cache_header header;
	struct program_cache_entry *entry;
	GLint length = 0;
	GLenum format = 0;
	void *binary;

	if (!program_cache.enabled || !vertex_hash || !fragment_hash)
		return;
	glGetProgramiv(program, GL_PROGRAM_BINARY_LENGTH, &length);
	if (length <= 0 || program_cache.end + sizeof(header) + (unsigned long)length > PROGRAM_CACHE_MAXIMUM_SIZE ||
		!(binary = malloc((size_t)length)))
	{
		return;
	}
	glGetProgramBinary(program, length, &length, &format, binary);
	memset(&header, 0, sizeof(header));
	header.magic = PROGRAM_CACHE_MAGIC;
	header.vertex_hash = vertex_hash;
	header.fragment_hash = fragment_hash;
	header.format = format;
	header.length = (unsigned long)length;
	header.data_hash = data_hash(binary, header.length);
	if (length > 0 && fseek(program_cache.file, (long)program_cache.end, SEEK_SET) == 0 &&
		fwrite(&header, sizeof(header), 1, program_cache.file) == 1 &&
		fwrite(binary, header.length, 1, program_cache.file) == 1 && fflush(program_cache.file) == 0)
	{
		entry = program_cache_find(vertex_hash, fragment_hash);
		if (!entry)
		{
			struct program_cache_entry **bucket = program_cache_bucket(vertex_hash, fragment_hash);

			entry = calloc(1, sizeof(*entry));
			if (entry)
			{
				entry->vertex_hash = vertex_hash;
				entry->fragment_hash = fragment_hash;
				entry->next = *bucket;
				*bucket = entry;
			}
		}
		if (entry)
		{
			entry->format = header.format;
			entry->length = header.length;
			entry->data_hash = header.data_hash;
			entry->offset = program_cache.end + sizeof(header);
		}
		program_cache.end += sizeof(header) + header.length;
		program_cache.written++;
	}
	free(binary);
}
#endif

static struct fragment_entry *fragment_buckets[FRAGMENT_BUCKETS];
static struct program_entry *program_buckets[PROGRAM_BUCKETS];

/* ---------- render targets */

/* what a render target's scale follows (render_target_get): an entry whose
scale no longer fits is freed (render_targets_evict) */
enum render_target_kind
{
	_render_target_plain,
	/* the screen's targets and the secondary target: screen_scale's */
	_render_target_screen,
	/* a shadow map: halo_shadow_map_scale's */
	_render_target_shadow_map,
};

struct render_target_entry
{
	struct render_target_entry *next;
	/* the next with the same address bucket (render_target_bucket) */
	struct render_target_entry *next_in_bucket;
	struct xgpu_render_target target;
	unsigned long last_rendered;
	/* the back buffer or its depth buffer, which the 3D view is drawn into:
	multisampled with multisampling (render_target_multisample) */
	BOOL screen_buffer;
	enum render_target_kind kind;
};

/* every draw looks up its targets and whether its textures are render
targets, of which there are dozens */
#define RENDER_TARGET_BUCKET_COUNT 256

static struct render_target_entry *render_target_buckets[RENDER_TARGET_BUCKET_COUNT];

static struct render_target_entry **render_target_bucket(unsigned long data)
{
	return &render_target_buckets[((data >> 12) ^ (data >> 20)) % RENDER_TARGET_BUCKET_COUNT];
}

struct framebuffer_entry
{
	struct framebuffer_entry *next;
	GLuint color;
	GLuint depth;
	/* color and depth are multisampled renderbuffers, not textures */
	BOOL renderbuffers;
	GLuint framebuffer;
};

static struct render_target_entry *render_targets;
static struct framebuffer_entry *framebuffers;

static void render_targets_evict(void);

/* ---------- the device */

#ifdef HALO_GLES
/* Mobile drivers (Mali) keep every orphaned copy of a buffer until the GPU
is done with it, so a large buffer orphaned each frame costs its size per
frame in flight and more. Instead each frame streams into the next of a few
smaller buffers, reusing one only once the GPU has finished the frame that
last used it (host_gl_wait_frame). A busy frame streams about 5 MB of
vertices. */
#define STREAM_BUFFER_SIZE (16 * 1024 * 1024)
#define INDEX_BUFFER_SIZE (2 * 1024 * 1024)
#define STREAM_BUFFER_RING 3
#else
#define STREAM_BUFFER_SIZE (32 * 1024 * 1024)
#define INDEX_BUFFER_SIZE (8 * 1024 * 1024)
#endif
#define VISIBILITY_TEST_SLOTS 4096

#ifndef HALO_GLES
#define VISIBILITY_BATCH_COUNT 3
/* what a test's place in the buffer holds until the GPU copies its count
there: more than any test covers (8K with 4x MSAA has 133 million samples) */
#define VISIBILITY_UNCOPIED 0xFFFFFFFFUL
/* a frame's tests: their queries (kept until every count is read), the
slot, number (visibility_ended) and area of each, and (once the frame is
presented) the fence its copies wait on */
struct visibility_batch
{
	GLuint queries[VISIBILITY_TEST_SLOTS];
	unsigned short slots[VISIBILITY_TEST_SLOTS];
	ULONGLONG tests[VISIBILITY_TEST_SLOTS];
	float areas[VISIBILITY_TEST_SLOTS];
	unsigned long count, read, frame;
	ULONGLONG serial;
	BOOL sealed, failed;
	GLsync fence;
};
#endif
#ifdef HALO_GLES
#define VISIBILITY_QUERY GL_ANY_SAMPLES_PASSED
#else
#define VISIBILITY_QUERY GL_SAMPLES_PASSED
#endif
/* a count more than any test covers (the game divides it by the test's
area), well below what would overflow there */
#define VISIBILITY_ALL_SAMPLES 1000000

struct gl_device
{
	D3DPRESENT_PARAMETERS presentation;
	D3DSurface back_buffer;
	/* the frame shown last, as the loading screen reads it (GetBackBuffer
	with -1: front_buffer_ready); the frame it was last asked for in, and
	the frame its picture was last taken in (front_buffer_take), each + 1 */
	D3DSurface front_buffer;
	unsigned long front_buffer_asked;
	unsigned long front_buffer_taken;
	D3DSurface depth_buffer;
	D3DSurface *render_target;
	D3DSurface *depth_stencil;
	D3DVIEWPORT8 viewport;
	D3DMATRIX transforms[D3DTS_MAX];
	D3DBaseTexture *textures[D3DTSS_MAXSTAGES];
	D3DPalette *palettes[D3DTSS_MAXSTAGES];
	D3DSHADERCONSTANTMODE shader_constant_mode;

	struct vertex_shader_object *vertex_shader;
	/* SetVertexShader chose the fixed-function pipeline (with 0): the
	loading screen's draws (fixed_function_program) */
	BOOL fixed_function;
	struct vertex_shader_object *program_slots[VERTEX_PROGRAM_SLOTS];
	unsigned long program_address;
	float constants[XGPU_VERTEX_CONSTANT_COUNT][4];
	float viewport_scale[4];
	float viewport_offset[4];

	struct
	{
		DWORD data;
		UINT stride;
	} streams[16];
	/* SetIndices' base vertex: added to every index of an indexed draw (the
	dynamic vertex buffers keep each buffer's vertices at an offset into one
	vertex buffer, and their triangles count from 0: contrails, lightning) */
	UINT base_vertex_index;

	/* the current value of each input register (SetVertexData) */
	float attributes[XGPU_VERTEX_ATTRIBUTE_COUNT][4];
	BOOL immediate_active;
	D3DPRIMITIVETYPE immediate_type;
	/* the input registers written since Begin, a bit each */
	unsigned long immediate_written;
	float *immediate_vertices;
	unsigned long immediate_count;
	unsigned long immediate_capacity;

	GLuint vertex_array;
	GLuint stream_buffer;
#ifdef HALO_GLES
	GLuint stream_buffers[STREAM_BUFFER_RING];
	GLuint index_buffers[STREAM_BUFFER_RING];
	unsigned long buffer_ring;
#endif
	unsigned long stream_offset;
	GLuint index_buffer;
	unsigned long index_offset;
#ifdef HALO_GLES
	/* (web_chunk_get) the room in the stream and index buffers, the draws
	made so far, and the count at each buffer's latest write */
	unsigned long stream_capacity;
	unsigned long index_capacity;
	unsigned long draw_serial;
	unsigned long stream_serial;
	unsigned long index_serial;
#endif
	GLuint samplers[D3DTSS_MAXSTAGES];

#ifdef HALO_GLES
	/* (without atomic counters) a query for each slot, and the one a test
	runs on until it ends, then swapped into its slot: a slot of its own, so
	that every slot, 0 too, is a test's (the game's lens flares number
	theirs from 0); and whether each slot's query has yet to be read */
	GLuint queries[VISIBILITY_TEST_SLOTS];
	GLuint scratch_query;
	BOOL visibility_unread[VISIBILITY_TEST_SLOTS];
	/* the number of the test each slot's query is (visibility_ended) */
	ULONGLONG query_tests[VISIBILITY_TEST_SLOTS];
#endif
	BOOL query_pending[VISIBILITY_TEST_SLOTS];
	/* the pixels each of the game's pixels covered in the test's target
	(render_target_get), which its count is divided by */
	float query_area[VISIBILITY_TEST_SLOTS];
	GLuint active_query;
	BOOL visibility_test_active;
	/* each slot's latest count known: the game spins on a result it is told
	is incomplete, so until a test's count has arrived the slot's earlier
	count stands. A frame of The Library's lights makes hundreds of tests. */
	GLuint visibility_known[VISIBILITY_TEST_SLOTS];
	/* the tests ended at each slot so far, dropped ones too, and the number
	(from 1) of the one whose count the slot gives, 0 for none: the latest
	read, which can be frames older than the slot's latest test
	(xgpu_visibility_tests_ended) */
	ULONGLONG visibility_ended[VISIBILITY_TEST_SLOTS];
	ULONGLONG visibility_counted[VISIBILITY_TEST_SLOTS];
#ifdef HALO_GLES
	/* with atomic counters: one counter per test, used as a ring */
	GLuint visibility_counters;
	unsigned long counter_next;
	unsigned long counter_active;
	/* Reading the counters waits for the draws that counted, which stops
	the CPU until the GPU has caught up (the game asks at the start of the
	next frame), halving the frame rate on drivers that queue frames (Zink,
	Turnip). Instead, at the end of each frame the GPU copies them into the
	frame's snapshot buffer of the stream ring, and the frame's tests (each
	a result slot and its counter) are listed with it. Once its fence has
	passed (two frames on, D3DDevice_Present) the CPU reads the snapshot
	into counter_values and gives each listed slot its count: a result is
	the latest count known, as the desktop query readback gives. */
	GLuint counter_snapshots[STREAM_BUFFER_RING];
	unsigned short ring_tests[STREAM_BUFFER_RING][VISIBILITY_TEST_SLOTS][2];
	ULONGLONG ring_test_numbers[STREAM_BUFFER_RING][VISIBILITY_TEST_SLOTS];
	unsigned long ring_test_count[STREAM_BUFFER_RING];
	GLuint counter_values[VISIBILITY_TEST_SLOTS];
	GLuint visibility_latest[VISIBILITY_TEST_SLOTS];
#else
	/* the tests of this frame and of the two before, whose counts are yet
	to be read (visibility_read_batch): the game waits for results at the
	start of the next frame, and a query read there would stop the CPU
	until the GPU had caught up. visibility_batch is the batch this frame's
	tests go in (VISIBILITY_BATCH_COUNT with none free: the tests are
	dropped, and their slots keep their earlier counts). */
	struct visibility_batch visibility_batches[VISIBILITY_BATCH_COUNT];
	unsigned long visibility_batch;
	ULONGLONG visibility_serial;
	/* with no query objects to be had: every test reports its slot fully
	visible */
	BOOL visibility_unavailable;
	BOOL visibility_logging;
	/* except with AMD's own driver, the GPU copies each batch's counts into
	its part of this buffer, mapped for the CPU to read */
	GLuint visibility_buffer;
	volatile GLuint *visibility_mapped;
	struct
	{
		unsigned long submitted, completed, samples, nonzero, maximum;
		unsigned long unavailable, dropped, failures;
	} visibility_stats;
	/* a pipeline flush every flush_every draws (draw_flush), 0 never */
	unsigned long flush_every;
	unsigned long flush_draws;
#endif

	unsigned long frame;
	unsigned long next_vertex_shader_id;
	BOOL gl_ready;
	BOOL created;
};

static struct gl_device device;

/* debug.gpu_stats prints these once a second */
static struct
{
	unsigned long draws, immediate_draws, clears, presents;
	unsigned long skipped_no_program, skipped_no_target, skipped_link;
	unsigned long target_changes;
	/* vertex and index bytes drawn from the mirror, and streamed */
	unsigned long mirrored_bytes, streamed_bytes;
	/* the textures uploaded, and the most draws and the longest time (ms)
	of one frame: the averages hide a frame that stalls */
	unsigned long texture_uploads, texture_upload_bytes;
	unsigned long most_draws, slowest_frame;
	/* (draws counted when this frame began) */
	unsigned long frame_first_draw;
} stats;

void xgpu_statistics_texture_upload(unsigned long bytes)
{
	stats.texture_uploads++;
	stats.texture_upload_bytes += bytes;
}

static D3DDevice *device_pointer(void)
{
	return (D3DDevice *)&device;
}

static float dword_to_float(DWORD value)
{
	union { DWORD d; float f; } u;

	u.d = value;
	return u.f;
}

static void color_to_vec4(D3DCOLOR color, float *out)
{
	out[0] = ((color >> 16) & 0xff) / 255.0f;
	out[1] = ((color >> 8) & 0xff) / 255.0f;
	out[2] = (color & 0xff) / 255.0f;
	out[3] = ((color >> 24) & 0xff) / 255.0f;
}

/* ---------- debugging settings, read once (gl_initialize) */

static struct
{
	/* debug.gpu_skip_vertex_shaders "<id>,<id>..." drops draws by vertex
	shader, for finding which pass produces something (port_config.c) */
	const char *skip_vertex_shaders;
	const char *dump_shaders;
	BOOL statistics;
} debug_settings;

#ifndef HALO_GLES
/* display.frames_in_flight: a fence after each frame's swap, and the frame
that many before waited for (frame_pacing_wait). Without it a driver queues
frames until its own limit, then blocks for several at once at whatever
call fills its queue: on Intel graphics on Windows, a frame of 50-60 ms
every sixth frame (some 15 fps felt at 80 fps). OpenGL ES's frames are
paced by its stream buffer ring instead (host_gl_wait_frame). */
#define MAXIMUM_FRAMES_IN_FLIGHT 8

static struct
{
	long limit;
	GLsync fences[MAXIMUM_FRAMES_IN_FLIGHT];
	long next;
} frame_pacing;
#endif

/* ---------- GL state cache

Consecutive draws share most of their state, but each sets all of it: the
setters here skip the call when GL already holds the value. Code that
changes GL state behind the cache's back (clears, presentation, texture
uploads, render target and framebuffer creation) calls
xgpu_gl_state_invalidate, after which every value is set again. Unknown
values are all ones, which no real value matches (floats become NaN, which
compares unequal to everything). */

#ifdef HALO_GLES
struct attribute_pointer
{
	GLuint buffer;
	GLint size;
	GLenum type;
	GLboolean normalized;
	GLboolean integer;
	GLsizei stride;
	unsigned long offset;
};
#else
/* desktop GL (4.3) separates an attribute's format from the buffer it
reads: the attributes of a stream share one binding, so a draw that moves
the stream rebinds it once instead of pointing each attribute again */
struct attribute_format
{
	GLint size;
	GLenum type;
	GLboolean normalized;
	GLboolean integer;
	GLuint relative_offset;
	GLuint binding;
};

struct vertex_binding
{
	GLuint buffer;
	unsigned long offset;
	GLsizei stride;
};

/* a binding per stream (setup_streams); GL has at least 16 */
#define VERTEX_BINDING_COUNT 16

/* a vertex array object for each vertex layout (the attributes a draw
enables, and each one's format and binding), made once: a draw binds its
layout's and points the bindings at its streams, rather than enabling,
formatting and binding each attribute again (setup_streams) */
struct vertex_layout
{
	unsigned long enabled;
	struct attribute_format formats[XGPU_VERTEX_ATTRIBUTE_COUNT];
};

struct vertex_array_entry
{
	struct vertex_array_entry *next;
	unsigned long hash;
	struct vertex_layout layout;
	GLuint vertex_array;
	/* its bindings, as last pointed (nothing else changes them) */
	struct vertex_binding bindings[VERTEX_BINDING_COUNT];
};

#define VERTEX_ARRAY_BUCKET_COUNT 64
static struct vertex_array_entry *vertex_array_buckets[VERTEX_ARRAY_BUCKET_COUNT];
static struct vertex_array_entry *current_vertex_array;
#endif

static struct
{
	GLuint program;
	GLuint framebuffer;
	GLint viewport[4];
	GLint scissor[4];
	float depth_range[2];
	unsigned char depth_test, stencil_test, blend, cull_face, offset_fill, offset_line;
	unsigned char scissor_test;
	GLenum depth_function;
	unsigned char depth_mask;
	GLenum stencil_function;
	GLint stencil_reference;
	GLuint stencil_value_mask;
	GLenum stencil_operations[3];
	GLuint stencil_write_mask;
	GLenum blend_source, blend_destination, blend_equation;
	float blend_color[4];
	unsigned char color_mask;
	GLenum front_face, cull_mode, polygon_mode;
	float polygon_offset[2];
	GLenum active_texture;
	/* per unit: the GL_TEXTURE_2D, GL_TEXTURE_CUBE_MAP and GL_TEXTURE_3D
	bindings */
	GLuint textures[D3DTSS_MAXSTAGES][3];
	GLuint samplers[D3DTSS_MAXSTAGES];
	GLuint array_buffer;
	GLuint element_array_buffer;
	unsigned char attribute_enabled[XGPU_VERTEX_ATTRIBUTE_COUNT];
#ifdef HALO_GLES
	struct attribute_pointer attribute_pointers[XGPU_VERTEX_ATTRIBUTE_COUNT];
#else
	GLuint vertex_array;
#endif
	/* a disabled attribute's value; kind 1 is the integer zero */
	unsigned char attribute_value_kind[XGPU_VERTEX_ATTRIBUTE_COUNT];
	float attribute_values[XGPU_VERTEX_ATTRIBUTE_COUNT][4];
} gl_state;

#ifdef HALO_WEB
static void vertex_array_cache_invalidate(void);
#endif

int xgpu_gl_ready(void)
{
	return device.gl_ready;
}

void xgpu_gl_state_invalidate(void)
{
	memset(&gl_state, 0xff, sizeof(gl_state));
#ifdef HALO_WEB
	vertex_array_cache_invalidate();
#endif
}

/* (a unit the shadows still said had it would never be bound to a new
texture GL gives its name to; and a stage no draw samples keeps what was
bound to it: bind_textures) */
void xgpu_gl_texture_deleted(unsigned int texture)
{
	int unit, slot;

	for (unit = 0; unit < D3DTSS_MAXSTAGES; unit++)
	{
		for (slot = 0; slot < 3; slot++)
		{
			if (gl_state.textures[unit][slot] == texture)
				gl_state.textures[unit][slot] = (GLuint)-1;
		}
	}
}

/* the source blend factor the pixel shader applies (nv2a_pixel_shader_key's
source_blend), when the draw's blend mixes a constant color factor with a
constant alpha one, which WebGL refuses; 0 otherwise */
static unsigned char source_blend_in_shader(void)
{
#ifdef HALO_WEB
	DWORD source = D3D__RenderState[D3DRS_SRCBLEND], destination = D3D__RenderState[D3DRS_DESTBLEND];
	int source_kind = source == D3DBLEND_CONSTANTCOLOR || source == D3DBLEND_INVCONSTANTCOLOR ? 1 :
		source == D3DBLEND_CONSTANTALPHA || source == D3DBLEND_INVCONSTANTALPHA ? 2 : 0;
	int destination_kind = destination == D3DBLEND_CONSTANTCOLOR || destination == D3DBLEND_INVCONSTANTCOLOR ? 1 :
		destination == D3DBLEND_CONSTANTALPHA || destination == D3DBLEND_INVCONSTANTALPHA ? 2 : 0;

	if (D3D__RenderState[D3DRS_ALPHABLENDENABLE] && source_kind && destination_kind && source_kind != destination_kind)
	{
		return source == D3DBLEND_CONSTANTCOLOR ? 1 : source == D3DBLEND_INVCONSTANTCOLOR ? 2 :
			source == D3DBLEND_CONSTANTALPHA ? 3 : 4;
	}
#endif
	return 0;
}

static void state_enable(unsigned char *shadow, GLenum capability, BOOL enabled)
{
	unsigned char value = enabled ? 1 : 0;

	if (*shadow == value)
		return;
	*shadow = value;
	if (value)
		glEnable(capability);
	else
		glDisable(capability);
}

static void state_program(GLuint program)
{
	if (gl_state.program != program)
	{
		gl_state.program = program;
		glUseProgram(program);
	}
}

static void state_framebuffer(GLuint framebuffer)
{
	if (gl_state.framebuffer != framebuffer)
	{
		gl_state.framebuffer = framebuffer;
		glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
	}
}

/* gl_state.textures' slot of a target */
static int texture_slot(GLenum target)
{
	return target == GL_TEXTURE_CUBE_MAP ? 1 : target == GL_TEXTURE_3D ? 2 : 0;
}

#ifdef HALO_GLES
static void state_texture(int unit, GLenum target, GLuint texture)
{
	int slot = texture_slot(target);

	if (gl_state.textures[unit][slot] == texture)
		return;
	if (gl_state.active_texture != GL_TEXTURE0 + (GLenum)unit)
	{
		gl_state.active_texture = GL_TEXTURE0 + (GLenum)unit;
		glActiveTexture(gl_state.active_texture);
	}
	gl_state.textures[unit][slot] = texture;
	glBindTexture(target, texture);
}
#endif

static void state_sampler(int unit, GLuint sampler)
{
	if (gl_state.samplers[unit] != sampler)
	{
		gl_state.samplers[unit] = sampler;
		glBindSampler((GLuint)unit, sampler);
	}
}

static void state_array_buffer(GLuint buffer)
{
	if (gl_state.array_buffer != buffer)
	{
		gl_state.array_buffer = buffer;
		glBindBuffer(GL_ARRAY_BUFFER, buffer);
	}
}

static void state_element_array_buffer(GLuint buffer)
{
	if (gl_state.element_array_buffer != buffer)
	{
		gl_state.element_array_buffer = buffer;
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, buffer);
	}
}

#ifdef HALO_GLES
/* enables the attribute, reading size elements of type from buffer: each
vertex is stride bytes on from the one before it, starting at
buffer_offset, with the attribute relative_offset bytes into it */
static void state_attribute_stream(GLuint index, GLuint binding, GLuint buffer, GLint size, GLenum type,
	GLboolean normalized, BOOL integer, GLsizei stride, unsigned long buffer_offset, unsigned long relative_offset)
{
	struct attribute_pointer *pointer = &gl_state.attribute_pointers[index];
	unsigned long offset = buffer_offset + relative_offset;

	(void)binding;
	if (gl_state.attribute_enabled[index] != 1)
	{
		gl_state.attribute_enabled[index] = 1;
		glEnableVertexAttribArray(index);
	}
	if (pointer->buffer == buffer && pointer->size == size && pointer->type == type &&
		pointer->normalized == normalized && pointer->integer == (integer ? GL_TRUE : GL_FALSE) &&
		pointer->stride == stride && pointer->offset == offset)
	{
		return;
	}
	state_array_buffer(buffer);
	if (integer)
		glVertexAttribIPointer(index, size, type, stride, (const void *)offset);
	else
		glVertexAttribPointer(index, size, type, normalized, stride, (const void *)offset);
	pointer->buffer = buffer;
	pointer->size = size;
	pointer->type = type;
	pointer->normalized = normalized;
	pointer->integer = integer ? GL_TRUE : GL_FALSE;
	pointer->stride = stride;
	pointer->offset = offset;
}
#else
/* the vertex array of a layout, made the first time (its attributes enabled,
formatted and bound to their bindings) */
static struct vertex_array_entry *vertex_array_get(const struct vertex_layout *layout)
{
	const unsigned char *bytes = (const unsigned char *)layout;
	unsigned long hash = 2166136261UL, index;
	struct vertex_array_entry **bucket, *entry;

	for (index = 0; index < sizeof(*layout); index++)
		hash = (hash ^ bytes[index]) * 16777619UL;
	bucket = &vertex_array_buckets[hash % VERTEX_ARRAY_BUCKET_COUNT];
	for (entry = *bucket; entry; entry = entry->next)
	{
		if (entry->hash == hash && !memcmp(&entry->layout, layout, sizeof(*layout)))
			return entry;
	}
	entry = calloc(1, sizeof(*entry));
	entry->hash = hash;
	entry->layout = *layout;
	memset(entry->bindings, 0xff, sizeof(entry->bindings));
	glGenVertexArrays(1, &entry->vertex_array);
	glBindVertexArray(entry->vertex_array);
	gl_state.vertex_array = entry->vertex_array;
	/* (the index buffer's binding is the vertex array's) */
	gl_state.element_array_buffer = (GLuint)-1;
	for (index = 0; index < XGPU_VERTEX_ATTRIBUTE_COUNT; index++)
	{
		const struct attribute_format *format = &layout->formats[index];

		if (!(layout->enabled & (1UL << index)))
			continue;
		glEnableVertexAttribArray((GLuint)index);
		if (format->integer)
			glVertexAttribIFormat((GLuint)index, format->size, format->type, format->relative_offset);
		else
			glVertexAttribFormat((GLuint)index, format->size, format->type, format->normalized, format->relative_offset);
		glVertexAttribBinding((GLuint)index, format->binding);
	}
	entry->next = *bucket;
	*bucket = entry;
	return entry;
}

static void state_vertex_array(struct vertex_array_entry *entry)
{
	if (gl_state.vertex_array != entry->vertex_array)
	{
		gl_state.vertex_array = entry->vertex_array;
		glBindVertexArray(entry->vertex_array);
		/* (the index buffer's binding is the vertex array's) */
		gl_state.element_array_buffer = (GLuint)-1;
	}
	current_vertex_array = entry;
}

/* points the bound vertex array's binding at buffer: each vertex is stride
bytes on from the one before it, starting at offset */
static void state_vertex_buffer(GLuint binding, GLuint buffer, unsigned long offset, GLsizei stride)
{
	struct vertex_binding *vertex_binding = &current_vertex_array->bindings[binding];

	if (vertex_binding->buffer != buffer || vertex_binding->offset != offset || vertex_binding->stride != stride)
	{
		glBindVertexBuffer(binding, buffer, (GLintptr)offset, stride);
		vertex_binding->buffer = buffer;
		vertex_binding->offset = offset;
		vertex_binding->stride = stride;
	}
}

/* an attribute of a layout */
static void layout_attribute(struct vertex_layout *layout, unsigned long index, GLuint binding, GLint size,
	GLenum type, GLboolean normalized, BOOL integer, unsigned long relative_offset)
{
	struct attribute_format *format = &layout->formats[index];

	format->size = size;
	format->type = type;
	format->normalized = normalized;
	format->integer = integer ? GL_TRUE : GL_FALSE;
	format->relative_offset = (GLuint)relative_offset;
	format->binding = binding;
	layout->enabled |= 1UL << index;
}
#endif

/* the value a disabled attribute reads (value, or the integer zero), which
is the context's, not the vertex array's */
static void state_attribute_constant(GLuint index, const float *value)
{
	unsigned char kind = value ? 0 : 1;

	if (gl_state.attribute_value_kind[index] == kind &&
		(!value || !memcmp(gl_state.attribute_values[index], value, sizeof(gl_state.attribute_values[index]))))
	{
		return;
	}
	gl_state.attribute_value_kind[index] = kind;
	if (value)
	{
		memcpy(gl_state.attribute_values[index], value, sizeof(gl_state.attribute_values[index]));
		glVertexAttrib4fv(index, value);
	}
	else
	{
		glVertexAttribI4ui(index, 0, 0, 0, 0);
	}
}

/* disables the attribute, which then reads value, or the integer zero */
static void state_attribute_value(GLuint index, const float *value)
{
	unsigned char kind = value ? 0 : 1;

#ifdef HALO_GLES
	if (gl_state.attribute_enabled[index] != 0)
	{
		gl_state.attribute_enabled[index] = 0;
		glDisableVertexAttribArray(index);
	}
#endif
	if (gl_state.attribute_value_kind[index] == kind &&
		(!value || !memcmp(gl_state.attribute_values[index], value, sizeof(gl_state.attribute_values[index]))))
	{
		return;
	}
	gl_state.attribute_value_kind[index] = kind;
	if (value)
	{
		memcpy(gl_state.attribute_values[index], value, sizeof(gl_state.attribute_values[index]));
		glVertexAttrib4fv(index, value);
	}
	else
	{
		glVertexAttribI4ui(index, 0, 0, 0, 0);
	}
}

/* ---------- vertical blank emulation */

#define VERTICAL_BLANK_NANOSECONDS (1000000000L / 60)

static pthread_mutex_t vertical_blank_lock = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t vertical_blank_condition = PTHREAD_COND_INITIALIZER;
static D3DCALLBACK vertical_blank_callback;
static unsigned long vertical_blank_count;
static volatile unsigned int flip_count;
static unsigned long pending_flips;
static BOOL vertical_blank_thread_started = FALSE;

static void *vertical_blank_thread(void *unused)
{
	struct timespec next;

	(void)unused;
	clock_gettime(CLOCK_MONOTONIC, &next);
	for (;;)
	{
		D3DCALLBACK callback;

		next.tv_nsec += VERTICAL_BLANK_NANOSECONDS;
		if (next.tv_nsec >= 1000000000L)
		{
			next.tv_nsec -= 1000000000L;
			next.tv_sec++;
		}
		clock_nanosleep(CLOCK_MONOTONIC, TIMER_ABSTIME, &next, NULL);

		pthread_mutex_lock(&vertical_blank_lock);
		vertical_blank_count++;
		/* a presented frame becomes visible at the next vertical blank */
		if (pending_flips)
		{
			pending_flips--;
			flip_count++;
		}
		callback = vertical_blank_callback;
		pthread_cond_broadcast(&vertical_blank_condition);
		pthread_mutex_unlock(&vertical_blank_lock);

		if (callback)
			callback(0);
	}
	return NULL;
}

static void vertical_blank_start(void)
{
	pthread_mutex_lock(&vertical_blank_lock);
	if (!vertical_blank_thread_started)
	{
		pthread_t thread;

		if (pthread_create(&thread, NULL, vertical_blank_thread, NULL) == 0)
		{
			pthread_detach(thread);
			vertical_blank_thread_started = TRUE;
		}
		else
		{
			platform_log("cannot start the vertical blank thread");
		}
	}
	pthread_mutex_unlock(&vertical_blank_lock);
}

/* replaces main/d3d_intimacy.cpp, which reads the counter out of the Xbox
Direct3D runtime's private device structure */
volatile unsigned int *d3d_find_flipcount(void)
{
	return &flip_count;
}

void WINAPI D3DDevice_SetVerticalBlankCallback(D3DCALLBACK callback)
{
	pthread_mutex_lock(&vertical_blank_lock);
	vertical_blank_callback = callback;
	pthread_mutex_unlock(&vertical_blank_lock);
	vertical_blank_start();
}

static void front_buffer_take(void);

void WINAPI D3DDevice_BlockUntilVerticalBlank(void)
{
	unsigned long count;

	/* (the loading screen's last call: its frame, for the next frame's
	trails) */
	front_buffer_take();

	vertical_blank_start();
	pthread_mutex_lock(&vertical_blank_lock);
	count = vertical_blank_count;
	while (vertical_blank_count == count)
		pthread_cond_wait(&vertical_blank_condition, &vertical_blank_lock);
	pthread_mutex_unlock(&vertical_blank_lock);
}

/* ---------- GL helpers */

GLuint xgpu_compile_shader(GLenum type, const char *source, const char *what)
{
	GLuint shader = glCreateShader(type);
	GLint status = 0;

	glShaderSource(shader, 1, &source, NULL);
	glCompileShader(shader);
	glGetShaderiv(shader, GL_COMPILE_STATUS, &status);
	if (!status)
	{
		char log[4096];

		glGetShaderInfoLog(shader, sizeof(log), NULL, log);
		platform_log("cannot compile the %s shader:\n%s\n%s", what, log, source);
		glDeleteShader(shader);
		return 0;
	}
	return shader;
}

GLuint xgpu_link_program(GLuint vertex_shader, GLuint fragment_shader, const char *what)
{
	GLuint program = glCreateProgram();
	GLint status = 0;

	glAttachShader(program, vertex_shader);
	glAttachShader(program, fragment_shader);
	glLinkProgram(program);
	glGetProgramiv(program, GL_LINK_STATUS, &status);
	if (!status)
	{
		char log[4096];

		glGetProgramInfoLog(program, sizeof(log), NULL, log);
		platform_log("cannot link the %s program: %s", what, log);
		return 0;
	}
	return program;
}

#ifndef HALO_GLES
static void GLAPIENTRY gl_debug_callback(GLenum source, GLenum type, GLuint id, GLenum severity,
	GLsizei length, const GLchar *message, const void *user)
{
	(void)source; (void)id; (void)length; (void)user;
	if (severity != GL_DEBUG_SEVERITY_NOTIFICATION)
		platform_log("GL %s: %s", type == GL_DEBUG_TYPE_ERROR ? "error" : "debug", message);
}
#endif

/* ---------- render targets */

static void surface_dimensions(const D3DSurface *surface, unsigned long *width, unsigned long *height, BOOL *depth)
{
	struct xgpu_texture_description description;
	DWORD format;

	xgpu_texture_describe(surface->Format, surface->Size, &description);
	*width = description.width;
	*height = description.height;
	format = description.format;
	*depth = format == D3DFMT_D24S8 || format == D3DFMT_F24S8 || format == D3DFMT_D16 || format == D3DFMT_F16 ||
		format == D3DFMT_LIN_D24S8 || format == D3DFMT_LIN_F24S8 || format == D3DFMT_LIN_D16 || format == D3DFMT_LIN_F16;
}

/* the shadow maps: the game's only R5G6B5 render targets, 128x128
(rasterizer_xbox.c) */
static BOOL surface_is_shadow_map(const D3DSurface *surface)
{
	struct xgpu_texture_description description;

	xgpu_texture_describe(surface->Format, surface->Size, &description);
	return description.format == D3DFMT_R5G6B5 && description.width == SHADOW_MAP_SIZE &&
		description.height == SHADOW_MAP_SIZE;
}

/* the secondary render target, the size of the Xbox's (rasterizer_xbox.c) */
#define SECONDARY_TARGET_WIDTH 320
#define SECONDARY_TARGET_HEIGHT 240

/* the targets render_target_get found last, by what it found them from:
each draw asks again for the same two (bind_targets) */
#define RECENT_RENDER_TARGET_COUNT 4

static struct
{
	DWORD data, format, size;
	long screen_width;
	float scale[2];
	struct render_target_entry *entry;
} recent_render_targets[RECENT_RENDER_TARGET_COUNT];
static unsigned long recent_render_target_next;

static struct render_target_entry *render_target_remember(const D3DSurface *surface, long screen,
	struct render_target_entry *entry)
{
	unsigned long slot = recent_render_target_next++ % RECENT_RENDER_TARGET_COUNT;

	recent_render_targets[slot].data = surface->Data;
	recent_render_targets[slot].format = surface->Format;
	recent_render_targets[slot].size = surface->Size;
	recent_render_targets[slot].screen_width = screen;
	recent_render_targets[slot].scale[0] = screen_scale[0];
	recent_render_targets[slot].scale[1] = screen_scale[1];
	recent_render_targets[slot].entry = entry;
	return entry;
}

/* the bound texture's storage, of the target's size and format */
static void render_target_storage(const struct xgpu_render_target *target)
{
	if (target->format == GL_DEPTH24_STENCIL8)
		glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH24_STENCIL8, (GLsizei)target->gl_width, (GLsizei)target->gl_height, 0,
			GL_DEPTH_STENCIL, GL_UNSIGNED_INT_24_8, NULL);
	else if (target->format == GL_DEPTH32F_STENCIL8)
		glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH32F_STENCIL8, (GLsizei)target->gl_width, (GLsizei)target->gl_height, 0,
			GL_DEPTH_STENCIL, GL_FLOAT_32_UNSIGNED_INT_24_8_REV, NULL);
	else if (target->format == GL_RGBA16F)
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, (GLsizei)target->gl_width, (GLsizei)target->gl_height, 0,
			GL_RGBA, GL_HALF_FLOAT, NULL);
	else
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, (GLsizei)target->gl_width, (GLsizei)target->gl_height, 0,
			GL_BGRA, GL_UNSIGNED_BYTE, NULL);
}

static struct render_target_entry *render_target_get(const D3DSurface *surface)
{
	struct render_target_entry *entry;
	unsigned long width, height, slot;
	long screen;
	BOOL depth;

	if (!surface || !surface->Data)
		return NULL;
	float scale[2] = { 1.0f, 1.0f };
	enum render_target_kind kind = _render_target_plain;

	/* (an entry lives until its scale no longer fits: render_targets_evict) */
	screen = halo_screen_width();
	for (slot = 0; slot < RECENT_RENDER_TARGET_COUNT; slot++)
	{
		if (recent_render_targets[slot].entry && recent_render_targets[slot].data == surface->Data &&
			recent_render_targets[slot].format == surface->Format && recent_render_targets[slot].size == surface->Size &&
			recent_render_targets[slot].screen_width == screen && recent_render_targets[slot].scale[0] == screen_scale[0] &&
			recent_render_targets[slot].scale[1] == screen_scale[1])
		{
			return recent_render_targets[slot].entry;
		}
	}
	surface_dimensions(surface, &width, &height, &depth);
	/* the screen's targets are drawn at the screen's scale, and so is the
	secondary target (the reflections' and active camouflage's: its size is
	rasterizer_xbox.c's), which keeps the game's 320x240 units, that its
	shaders' constants are in, and gets the screen's pixels a unit; the
	shadow maps at display.shadow_resolution's (halo_shadow_map_scale) */
	if ((width == (unsigned long)halo_screen_width() && height == SCREEN_HEIGHT) ||
		(!depth && width == SECONDARY_TARGET_WIDTH && height == SECONDARY_TARGET_HEIGHT))
	{
		scale[0] = screen_scale[0];
		scale[1] = screen_scale[1];
		kind = _render_target_screen;
	}
	else if (!depth && surface_is_shadow_map(surface))
	{
		scale[0] = (float)halo_shadow_map_scale();
		scale[1] = scale[0];
		kind = _render_target_shadow_map;
	}
	for (entry = *render_target_bucket(surface->Data); entry; entry = entry->next_in_bucket)
	{
		if (entry->target.data == surface->Data && entry->target.width == width &&
			entry->target.height == height && entry->target.depth == depth &&
			entry->target.scale[0] == scale[0] && entry->target.scale[1] == scale[1])
		{
			return render_target_remember(surface, screen, entry);
		}
	}
	entry = calloc(1, sizeof(*entry));
	entry->target.data = surface->Data;
	entry->target.width = width;
	entry->target.height = height;
	entry->target.depth = depth;
	entry->target.scale[0] = scale[0];
	entry->target.scale[1] = scale[1];
	entry->kind = kind;
	entry->target.gl_width = (unsigned long)(width * scale[0] + 0.5f);
	entry->target.gl_height = (unsigned long)(height * scale[1] + 0.5f);
	entry->screen_buffer = surface->Data == (depth ? device.depth_buffer.Data : device.back_buffer.Data);
	entry->target.format = depth ? (xgpu_reversed_depth ? GL_DEPTH32F_STENCIL8 : GL_DEPTH24_STENCIL8) :
		entry->screen_buffer && hdr.active ? GL_RGBA16F : GL_RGBA8;
	glGenTextures(1, &entry->target.texture);
	glBindTexture(GL_TEXTURE_2D, entry->target.texture);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 0);
	render_target_storage(&entry->target);
	xgpu_gl_state_invalidate();
	entry->next = render_targets;
	render_targets = entry;
	entry->next_in_bucket = *render_target_bucket(entry->target.data);
	*render_target_bucket(entry->target.data) = entry;
	return entry;
}

struct xgpu_render_target *xgpu_render_target_find(unsigned long data)
{
	struct render_target_entry *entry, *best = NULL;

	for (entry = *render_target_bucket(data); entry; entry = entry->next_in_bucket)
	{
		if (entry->target.data == data && !entry->target.depth && (!best || entry->last_rendered > best->last_rendered))
			best = entry;
	}
	return best ? &best->target : NULL;
}

/* the framebuffer of these textures, or with renderbuffers, of these
multisampled renderbuffers (render_target_multisample) */
static GLuint framebuffer_find(GLuint color, GLuint depth, BOOL renderbuffers)
{
	struct framebuffer_entry *entry, **link;
	GLenum draw_buffer = color ? GL_COLOR_ATTACHMENT0 : GL_NONE;

	/* (found, it moves to the front: each draw asks again for the one
	the draw before it did) */
	for (link = &framebuffers; (entry = *link) != NULL; link = &entry->next)
	{
		if (entry->color == color && entry->depth == depth && entry->renderbuffers == renderbuffers)
		{
			*link = entry->next;
			entry->next = framebuffers;
			framebuffers = entry;
			return entry->framebuffer;
		}
	}
	entry = calloc(1, sizeof(*entry));
	entry->color = color;
	entry->depth = depth;
	entry->renderbuffers = renderbuffers;
	glGenFramebuffers(1, &entry->framebuffer);
	glBindFramebuffer(GL_FRAMEBUFFER, entry->framebuffer);
	if (renderbuffers)
	{
		if (color)
			glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, color);
		if (depth)
			glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, depth);
	}
	else
	{
		if (color)
			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, color, 0);
		if (depth)
			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_TEXTURE_2D, depth, 0);
	}
	glDrawBuffers(1, &draw_buffer);
	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
		platform_log("framebuffer %u/%u%s is incomplete", color, depth, renderbuffers ? " (multisampled)" : "");
	xgpu_gl_state_invalidate();
	entry->next = framebuffers;
	framebuffers = entry;
	return entry->framebuffer;
}

static GLuint framebuffer_get(GLuint color, GLuint depth)
{
	return framebuffer_find(color, depth, FALSE);
}

/* ---------- multisampling

With display.anti_aliasing's multisampling, the back buffer and its depth
buffer are drawn into multisampled renderbuffers, and so is any target drawn
together with one of them (the mirror's view goes to the secondary target
with the back buffer's depth buffer): a framebuffer's attachments are all
multisampled or none is, so that of a target's texture and its renderbuffer
only one has pixels drawn since the other had them. The renderbuffer's
pixels are resolved into the texture before anything reads the texture (a
draw's textures, the display blit), and the texture's are put into the
renderbuffer when it is made, or made again for other samples. Nothing
samples a depth buffer as a texture (the game's copy of the depth buffer's
memory is a target of its own), so a depth buffer is resolved only when it
stops being multisampled. */

/* the samples a pixel of the bound targets: their renderbuffers' (or 1) */
static int target_samples = 1;

/* the multisampled pixels of a target drawn into since into its texture */
static void render_target_resolve(struct xgpu_render_target *target)
{
	GLuint read, draw;

	if (!target->unresolved)
		return;
	target->unresolved = FALSE;
	/* (both found first: making a framebuffer binds it) */
	read = target->depth ? framebuffer_find(0, target->multisample, TRUE) :
		framebuffer_find(target->multisample, 0, TRUE);
	draw = target->depth ? framebuffer_get(0, target->texture) : framebuffer_get(target->texture, 0);
	glBindFramebuffer(GL_READ_FRAMEBUFFER, read);
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, draw);
	glDisable(GL_SCISSOR_TEST);
	glBlitFramebuffer(0, 0, (GLint)target->gl_width, (GLint)target->gl_height,
		0, 0, (GLint)target->gl_width, (GLint)target->gl_height,
		target->depth ? GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT : GL_COLOR_BUFFER_BIT, GL_NEAREST);
	xgpu_gl_state_invalidate();
}

/* a target's renderbuffer with these samples a pixel (0: none, its storage
a pixel, the renderbuffer kept for the framebuffers made of it): its pixels
resolved into its texture first, and the texture's put into the new
storage */
static void render_target_multisample(struct xgpu_render_target *target, int samples)
{
	GLuint draw;

	if (target->samples == samples)
		return;
	render_target_resolve(target);
	if (!target->multisample)
		glGenRenderbuffers(1, &target->multisample);
	glBindRenderbuffer(GL_RENDERBUFFER, target->multisample);
	glRenderbufferStorageMultisample(GL_RENDERBUFFER, samples, target->format,
		samples ? (GLsizei)target->gl_width : 1, samples ? (GLsizei)target->gl_height : 1);
	glBindRenderbuffer(GL_RENDERBUFFER, 0);
	target->samples = samples;
	if (!samples)
		return;
	draw = target->depth ? framebuffer_find(0, target->multisample, TRUE) :
		framebuffer_find(target->multisample, 0, TRUE);
#ifdef HALO_GLES
	/* (ES blits into no multisampled framebuffer: it is cleared instead. A
	change of the setting is taken up between frames, and the frame clears
	the screen's targets before it draws) */
	glBindFramebuffer(GL_FRAMEBUFFER, draw);
	glDisable(GL_SCISSOR_TEST);
	glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
	glDepthMask(GL_TRUE);
	glStencilMask(0xff);
	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
	glClearDepth(1.0f);
	glClearStencil(0);
	glClear(target->depth ? GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT : GL_COLOR_BUFFER_BIT);
#else
	{
		GLuint read = target->depth ? framebuffer_get(0, target->texture) : framebuffer_get(target->texture, 0);

		glBindFramebuffer(GL_READ_FRAMEBUFFER, read);
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, draw);
		glDisable(GL_SCISSOR_TEST);
		glBlitFramebuffer(0, 0, (GLint)target->gl_width, (GLint)target->gl_height,
			0, 0, (GLint)target->gl_width, (GLint)target->gl_height,
			target->depth ? GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT : GL_COLOR_BUFFER_BIT, GL_NEAREST);
	}
#endif
	xgpu_gl_state_invalidate();
}

/* counts draws and clears into render targets (xgpu_render_target.written) */
static unsigned long render_target_write_serial;

/* the pixels per unit of the bound targets (render_target_get) */
static float target_scale[2] = { 1.0f, 1.0f };

/* the pixel edge of a coordinate in a target's units, at its scale:
floorf's, without its call (on 32-bit x86 it saves and restores the FPU's
rounding, several times a draw) */
static GLint scaled_pixel(float coordinate, float scale)
{
	float value = coordinate * scale + 0.5f;
	GLint pixel = (GLint)value;

	/* (the conversion is toward zero: a negative value with a fraction
	rounds down one more) */
	if ((float)pixel > value)
		pixel--;
	return pixel;
}

/* ... in the bound targets' units */
static GLint target_pixel(float coordinate, int axis)
{
	return scaled_pixel(coordinate, target_scale[axis]);
}

/* binds the framebuffer for the current targets; returns FALSE if there is
nothing to draw into */
static BOOL bind_targets(BOOL *has_depth)
{
	struct render_target_entry *color = render_target_get(device.render_target);
	struct render_target_entry *depth = render_target_get(device.depth_stencil);
	int samples;

	if (depth && !depth->target.depth)
		depth = NULL;
	if (!color && !depth)
		return FALSE;
	if (color)
	{
		color->last_rendered = device.frame + 1;
		color->target.written = ++render_target_write_serial;
	}
	/* viewports and clears are in the targets' units (render_target_get) */
	target_scale[0] = color ? color->target.scale[0] : depth->target.scale[0];
	target_scale[1] = color ? color->target.scale[1] : depth->target.scale[1];
	/* with multisampling, multisampled where either is a screen buffer or
	is multisampled already */
	samples = anti_aliasing() == _anti_aliasing_msaa ? anti_aliasing_samples : 0;
	if (samples && !((color && (color->screen_buffer || color->target.samples)) ||
		(depth && (depth->screen_buffer || depth->target.samples))))
	{
		samples = 0;
	}
	if (color)
		render_target_multisample(&color->target, samples);
	if (depth)
		render_target_multisample(&depth->target, samples);
	if (samples)
	{
		state_framebuffer(framebuffer_find(color ? color->target.multisample : 0,
			depth ? depth->target.multisample : 0, TRUE));
		if (color)
			color->target.unresolved = TRUE;
		if (depth)
			depth->target.unresolved = TRUE;
		target_samples = samples;
	}
	else
	{
		state_framebuffer(framebuffer_get(color ? color->target.texture : 0, depth ? depth->target.texture : 0));
		target_samples = 1;
	}
	*has_depth = depth != NULL;
	return TRUE;
}

/* ---------- display.hdr */

static const char *const tone_mapping_names[NUMBER_OF_XGPU_TONE_MAPPINGS] =
	{ "neutral", "aces", "reinhard", "gt7" };

/* the HDR calibration screen (port/linux/game/menu_functions.c), while it
is drawn: the pattern of its row chosen (HDR_CALIBRATION_PEAK or
HDR_CALIBRATION_PAPER_WHITE), and the paper white and peak it shows, in
nits (0 for Windows'), which the display takes in place of the settings'
until the screen closes (OK writes them to the settings) */
enum
{
	HDR_CALIBRATION_PEAK,
	HDR_CALIBRATION_PAPER_WHITE
};

static struct
{
	/* device.frame + 1 of the frame the screen was last drawn in; 0 never */
	unsigned long frame;
	int pattern;
	int paper_white, peak_brightness;
} hdr_calibration;

/* a setting in nits, within lowest and highest; 0 (Windows' own) stays */
static int hdr_nits(int value, int lowest, int highest)
{
	return value <= 0 ? 0 : value < lowest ? lowest : value > highest ? highest : value;
}

/* the calibration screen was drawn in this frame, or (or_last_frame) in
this frame or the one before: it says so as it is drawn, after this frame's
halo_screen_commit */
static BOOL hdr_calibration_shown(BOOL or_last_frame)
{
	return hdr.float_window && hdr_calibration.frame &&
		device.frame + 1 - hdr_calibration.frame <= (or_last_frame ? 1UL : 0UL);
}

int halo_hdr_calibration_available(void)
{
	return hdr.float_window && hdr.display_on;
}

void halo_hdr_calibration_show(int pattern, int paper_white, int peak_brightness)
{
	hdr_calibration.frame = device.frame + 1;
	hdr_calibration.pattern = pattern;
	hdr_calibration.paper_white = hdr_nits(paper_white, 80, 1000);
	hdr_calibration.peak_brightness = hdr_nits(peak_brightness, 250, 10000);
}

/* a rectangle of the menus' 640 by 480 units (the screen's middle, as the
menus are drawn) as corners in the window, row 0 at the bottom, where the
back buffer is drawn at window's x, y, width and height */
static void hdr_calibration_place(const struct render_target_entry *back_buffer, const int window[4], float left,
	float top, float right, float bottom, GLint corners[4])
{
	float across = (float)window[2] / (float)back_buffer->target.width;
	float down = (float)window[3] / (float)back_buffer->target.height;
	float offset = (float)(halo_screen_width() - 640) / 2.0f;

	corners[0] = window[0] + (GLint)((left + offset) * across + 0.5f);
	corners[2] = window[0] + (GLint)((right + offset) * across + 0.5f);
	corners[1] = window[1] + window[3] - (GLint)(bottom * down + 0.5f);
	corners[3] = window[1] + window[3] - (GLint)(top * down + 0.5f);
}

/* a square, and a ring in it, of the menus' units: its left and top, the
square's colour and the ring's (scRGB) */
static void hdr_calibration_square(const struct render_target_entry *back_buffer, const int window[4], float left,
	float top, float square_color, float ring_color)
{
	/* (the square's side, and the ring's radii) */
	const float SIDE = 180.0f, OUTER = 62.0f, INNER = 42.0f;
	float down = (float)window[3] / (float)back_buffer->target.height;
	float color[3], ring[4];
	GLint corners[4];

	hdr_calibration_place(back_buffer, window, left, top, left + SIDE, top + SIDE, corners);
	color[0] = color[1] = color[2] = square_color;
	ring[0] = ring[1] = ring[2] = ring[3] = 0.0f;
	xgpu_post_fill(corners, color, ring);
	color[0] = color[1] = color[2] = ring_color;
	ring[0] = (float)(corners[0] + corners[2]) / 2.0f;
	ring[1] = (float)(corners[1] + corners[3]) / 2.0f;
	ring[2] = OUTER * down;
	ring[3] = INNER * down;
	xgpu_post_fill(corners, color, ring);
}

/* the calibration screen's pattern, over its menus, between its rows and
its help. The peak's: a square at the peak shown, and in it a ring at 10000
nits, brighter than any display shows, which disappears into the square
once the square is as bright as the display goes. Paper white's: a grey
scale up to white at the paper white shown, as the menus' white then is,
beside a square of that white with a ring in it at the peak, the light past
white. */
static void hdr_calibration_draw(const struct render_target_entry *back_buffer, const int window[4])
{
	/* (scRGB, 1.0 at 80 nits) */
	const float BRIGHTEST = 10000.0f / 80.0f;
	const int GREYS = 8;
	float paper = hdr.sdr_white, peak = hdr.sdr_white * hdr.tone_mapping.peak;

	if (hdr_calibration.pattern == HDR_CALIBRATION_PEAK)
	{
		hdr_calibration_square(back_buffer, window, 230.0f, 146.0f, peak, BRIGHTEST);
	}
	else
	{
		float ring[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
		int grey;

		for (grey = 0; grey < GREYS; grey++)
		{
			float color[3];
			GLint corners[4];

			/* (the menus' values are gamma encoded, as the presentation's) */
			color[0] = color[1] = color[2] = powf((float)(grey + 1) / (float)GREYS, 2.2f) * paper;
			hdr_calibration_place(back_buffer, window, 100.0f + 26.0f * (float)grey, 166.0f,
				100.0f + 26.0f * (float)(grey + 1), 306.0f, corners);
			xgpu_post_fill(corners, color, ring);
		}
		hdr_calibration_square(back_buffer, window, 360.0f, 146.0f, paper, peak);
	}
}

/* display.hdr's settings, if they have changed: the back buffer's entries
are made again in the format it takes (what they held is lost, and a frame
clears them before it draws) */
static void hdr_read(void)
{
	const char *curve;
	double exposure;
	BOOL wanted, supported = TRUE, active;
	struct render_target_entry *entry;
	int value;

	if (!device.gl_ready || (hdr.read && hdr.read_at == config_changes()))
		return;
	if (!hdr.read)
	{
#ifdef HALO_GLES
		glGetInternalformativ(GL_RENDERBUFFER, GL_RGBA16F, GL_SAMPLES, 1, &hdr.maximum_samples);
#else
		GLint component_type = GL_NONE;

		/* (the window's own framebuffer, as display.hdr_output made it) */
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glGetFramebufferAttachmentParameteriv(GL_FRAMEBUFFER, GL_BACK_LEFT,
			GL_FRAMEBUFFER_ATTACHMENT_COMPONENT_TYPE, &component_type);
		hdr.float_window = component_type == GL_FLOAT;
		xgpu_gl_state_invalidate();
#endif
		hdr.sdr_white = 1.0f;
		hdr.tone_mapping.peak = 1.0f;
		hdr.tone_mapping.paper = 0.0f;
	}
	hdr.read = TRUE;
	hdr.read_at = config_changes();
	curve = config_string("display.hdr_tone_mapping");
	exposure = config_real("display.hdr_exposure");
	wanted = config_boolean("display.hdr");
	hdr.paper_white = hdr_nits((int)config_integer("display.hdr_paper_white"), 80, 1000);
	hdr.peak_brightness = hdr_nits((int)config_integer("display.hdr_peak_brightness"), 250, 10000);
	for (value = NUMBER_OF_XGPU_TONE_MAPPINGS - 1; value > 0 && strcmp(curve, tone_mapping_names[value]); value--)
	{
	}
	exposure = !(exposure >= 0.25) ? 0.25 : exposure > 4.0 ? 4.0 : exposure;
#ifdef HALO_GLES
	supported = xgpu_capabilities.color_buffer_half_float;
#endif
	active = wanted && supported && xgpu_post_prepare_hdr();
	/* (logged as it changes) */
	if (active != hdr.active || wanted != hdr.wanted ||
		(active && (value != hdr.tone_mapping.curve || (float)exposure != hdr.tone_mapping.exposure)))
	{
		if (active)
			platform_log("HDR: on, %s tone mapping%s, exposure %.2f", tone_mapping_names[value],
				strcmp(curve, tone_mapping_names[value]) ? " (the setting's is unknown)" : "", exposure);
		else if (wanted && !supported)
			platform_log("HDR: the GPU cannot draw into half floats, so off");
		else if (wanted)
			platform_log("HDR: its programs do not build, so off");
		else
			platform_log("HDR: off");
	}
	hdr.wanted = wanted;
	hdr.tone_mapping.curve = value;
	hdr.tone_mapping.exposure = (float)exposure;
	if (active == hdr.active)
		return;
	hdr.active = active;
	for (entry = render_targets; entry; entry = entry->next)
	{
		if (!entry->screen_buffer || entry->target.depth)
			continue;
		/* (the multisampled renderbuffer takes the format when it is next
		made: render_target_multisample) */
		render_target_multisample(&entry->target, 0);
		entry->target.format = active ? GL_RGBA16F : GL_RGBA8;
		glBindTexture(GL_TEXTURE_2D, entry->target.texture);
		render_target_storage(&entry->target);
	}
	xgpu_gl_state_invalidate();
	/* (multisampling's samples, which may be fewer for half floats) */
	anti_aliasing_prepare();
}

/* ---------- device creation */

#ifndef HALO_GLES
static BOOL visibility_queries_initialize(void);
static void visibility_results_initialize(void);
static BOOL visibility_readback_gl_ok(const char *phase);
#endif

static void gl_initialize(void)
{
	GLint major = 0, minor = 0;
	int index;

	glGetIntegerv(GL_MAJOR_VERSION, &major);
#ifdef HALO_ANDROID
	platform_log("Android render policy: chip=%d geometry=%s CE textures=%s per-draw refresh=off",
		android_render_chip, android_geometry_fixes ? "on" : "off", android_texture_fixes ? "on" : "off");
#endif
	glGetIntegerv(GL_MINOR_VERSION, &minor);
#ifdef HALO_GLES
	{
		BOOL es32 = major > 3 || (major == 3 && minor >= 2);
		const char *base_vertex_source = es32 ? "core" : "none";

		/* clip control is emulated in the vertex shader (nv2a_vsh.c) */
		xgpu_capabilities.copy_image = es32 || host_gl_has_extension("GL_EXT_copy_image") ||
			host_gl_has_extension("GL_OES_copy_image");
		xgpu_capabilities.border_clamp = es32 || host_gl_has_extension("GL_EXT_texture_border_clamp") ||
			host_gl_has_extension("GL_OES_texture_border_clamp");
		xgpu_capabilities.anisotropy = host_gl_has_extension("GL_EXT_texture_filter_anisotropic");
		/* ANGLE and many Android drivers report ES 3.0 or 3.1 and offer
		glDrawElementsBaseVertex as an extension under another name; the renderer
		calls the pointer it has, so it takes the extension's when the core one is
		not there. One that cannot be loaded leaves the draws without a base vertex */
		xgpu_capabilities.base_vertex = es32 && halo_glDrawElementsBaseVertex != NULL;
		if (!xgpu_capabilities.base_vertex)
		{
			if (host_gl_has_extension("GL_EXT_draw_elements_base_vertex") && halo_glDrawElementsBaseVertexEXT)
			{
				halo_glDrawElementsBaseVertex = halo_glDrawElementsBaseVertexEXT;
				xgpu_capabilities.base_vertex = TRUE;
				base_vertex_source = "GL_EXT_draw_elements_base_vertex";
			}
			else if (host_gl_has_extension("GL_OES_draw_elements_base_vertex") && halo_glDrawElementsBaseVertexOES)
			{
				halo_glDrawElementsBaseVertex = halo_glDrawElementsBaseVertexOES;
				xgpu_capabilities.base_vertex = TRUE;
				base_vertex_source = "GL_OES_draw_elements_base_vertex";
			}
		}
		xgpu_capabilities.color_buffer_half_float = es32 ||
			host_gl_has_extension("GL_EXT_color_buffer_half_float") ||
			host_gl_has_extension("GL_EXT_color_buffer_float");
		xgpu_capabilities.shading_language = major > 3 || (major == 3 && minor >= 1) ? "310 es" : "300 es";
		if (major > 3 || (major == 3 && minor >= 1))
		{
			GLint counters = 0;

			glGetIntegerv(GL_MAX_FRAGMENT_ATOMIC_COUNTERS, &counters);
			xgpu_capabilities.atomic_counters = counters > 0;
		}
		xgpu_capabilities.s3tc = host_gl_has_extension("GL_EXT_texture_compression_s3tc") ||
			(host_gl_has_extension("GL_EXT_texture_compression_dxt1") &&
			host_gl_has_extension("GL_ANGLE_texture_compression_dxt3") &&
			host_gl_has_extension("GL_ANGLE_texture_compression_dxt5"));
		xgpu_capabilities.astc = host_gl_has_extension("GL_KHR_texture_compression_astc_ldr");
		platform_log("OpenGL ES %d.%d: copy image %d, border clamp %d, anisotropy %d, S3TC %d, ASTC %d, "
			"sample counting %d, base vertex %d (%s), half float targets %d",
			(int)major, (int)minor, xgpu_capabilities.copy_image, xgpu_capabilities.border_clamp,
			xgpu_capabilities.anisotropy, xgpu_capabilities.s3tc, xgpu_capabilities.astc, xgpu_capabilities.atomic_counters,
			xgpu_capabilities.base_vertex, base_vertex_source, xgpu_capabilities.color_buffer_half_float);
	}
#else
	if (config_boolean("debug.gl_debug"))
	{
		glEnable(GL_DEBUG_OUTPUT);
		glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
		glDebugMessageCallback(gl_debug_callback, NULL);
	}
	glClipControl(GL_UPPER_LEFT, GL_ZERO_TO_ONE);
	glEnable(GL_PROGRAM_POINT_SIZE);
	/* (read once, at start: the depth buffers' format and the programs
	depend on it) */
	{
		static BOOL read;

		if (!read)
		{
			read = TRUE;
			xgpu_reversed_depth = config_boolean("display.reversed_depth");
			platform_log("depth: %s", xgpu_reversed_depth ? "32-bit float, reversed" : "24 bits, as the Xbox's");
		}
	}
#endif
	glGenVertexArrays(1, &device.vertex_array);
	glBindVertexArray(device.vertex_array);
#ifdef HALO_GLES
	{
		int ring;

		glGenBuffers(STREAM_BUFFER_RING, device.stream_buffers);
		glGenBuffers(STREAM_BUFFER_RING, device.index_buffers);
		for (ring = 0; ring < STREAM_BUFFER_RING; ring++)
		{
			glBindBuffer(GL_ARRAY_BUFFER, device.stream_buffers[ring]);
			glBufferData(GL_ARRAY_BUFFER, STREAM_BUFFER_SIZE, NULL, GL_STREAM_DRAW);
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, device.index_buffers[ring]);
			glBufferData(GL_ELEMENT_ARRAY_BUFFER, INDEX_BUFFER_SIZE, NULL, GL_STREAM_DRAW);
		}
		device.stream_buffer = device.stream_buffers[0];
		device.index_buffer = device.index_buffers[0];
	}
#endif
#ifndef HALO_GLES
	glGenBuffers(1, &device.stream_buffer);
	glBindBuffer(GL_ARRAY_BUFFER, device.stream_buffer);
	glBufferData(GL_ARRAY_BUFFER, STREAM_BUFFER_SIZE, NULL, GL_STREAM_DRAW);
	glGenBuffers(1, &device.index_buffer);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, device.index_buffer);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, INDEX_BUFFER_SIZE, NULL, GL_STREAM_DRAW);
#endif
	glGenSamplers(D3DTSS_MAXSTAGES, device.samplers);
#ifdef HALO_GLES
	glGenQueries(VISIBILITY_TEST_SLOTS, device.queries);
	glGenQueries(1, &device.scratch_query);
#else
	if (visibility_queries_initialize())
		visibility_results_initialize();
	else
	{
		device.visibility_unavailable = TRUE;
		platform_log("GPU: cannot create the visibility test queries; every test reports fully visible");
	}
	{
		long every = config_integer("debug.gpu_flush_draws");
		const char *renderer = (const char *)glGetString(GL_RENDERER);

		if (every < 0)
			every = renderer && strstr(renderer, "Mesa Intel") ? 3 : 0;
		if (every > 0)
		{
			device.flush_every = (unsigned long)every;
			platform_log("GPU: a pipeline flush every %ld draws", every);
		}
	}
#endif
#ifdef HALO_GLES
	if (xgpu_capabilities.atomic_counters)
	{
		int ring;

		glGenBuffers(1, &device.visibility_counters);
		glBindBuffer(GL_ATOMIC_COUNTER_BUFFER, device.visibility_counters);
		glBufferData(GL_ATOMIC_COUNTER_BUFFER, VISIBILITY_TEST_SLOTS * sizeof(GLuint), NULL, GL_DYNAMIC_DRAW);
		glBindBuffer(GL_ATOMIC_COUNTER_BUFFER, 0);
		glGenBuffers(STREAM_BUFFER_RING, device.counter_snapshots);
		for (ring = 0; ring < STREAM_BUFFER_RING; ring++)
		{
			glBindBuffer(GL_COPY_WRITE_BUFFER, device.counter_snapshots[ring]);
			glBufferData(GL_COPY_WRITE_BUFFER, VISIBILITY_TEST_SLOTS * sizeof(GLuint), NULL, GL_STREAM_READ);
		}
		glBindBuffer(GL_COPY_WRITE_BUFFER, 0);
	}
#endif
	for (index = 0; index < XGPU_VERTEX_ATTRIBUTE_COUNT; index++)
	{
		device.attributes[index][3] = 1.0f;
		glVertexAttrib4fv(index, device.attributes[index]);
	}
	memory_watch_initialize();
	debug_settings.skip_vertex_shaders = config_string("debug.gpu_skip_vertex_shaders");
	debug_settings.dump_shaders = *config_string("debug.gpu_dump_shaders") ?
		config_string("debug.gpu_dump_shaders") : NULL;
	debug_settings.statistics = config_boolean("debug.gpu_stats");
#ifndef HALO_GLES
	program_cache_initialize();
	frame_pacing.limit = config_integer("display.frames_in_flight");
	if (frame_pacing.limit < 0)
		frame_pacing.limit = 0;
	if (frame_pacing.limit > MAXIMUM_FRAMES_IN_FLIGHT)
		frame_pacing.limit = MAXIMUM_FRAMES_IN_FLIGHT;
#endif
	{
		GLint renderbuffer_size = 0;

		glGetIntegerv(GL_MAX_SAMPLES, &anti_aliasing_maximum_samples);
		glGetIntegerv(GL_MAX_TEXTURE_SIZE, &maximum_target_size);
		glGetIntegerv(GL_MAX_RENDERBUFFER_SIZE, &renderbuffer_size);
		if (renderbuffer_size < maximum_target_size)
			maximum_target_size = renderbuffer_size;
	}
	xgpu_gl_state_invalidate();
	device.gl_ready = TRUE;
	/* display.stats_log's line: the driver's own text for the renderer */
	stats_log_set_renderer((const char *)glGetString(GL_VENDOR), (const char *)glGetString(GL_RENDERER));
	if (anti_aliasing_value < 0)
		anti_aliasing_read();
	else
		anti_aliasing_prepare();
	hdr_read();
}

/* what display.anti_aliasing's value needs of the GL context, once there is
one: multisampling's samples, at most the GPU's, and the passes' programs,
built as the value is chosen rather than in the middle of a frame (SMAA's
are large) */
static void anti_aliasing_prepare(void)
{
	int mode;

	if (!device.gl_ready || anti_aliasing_value < 0)
		return;
	mode = anti_aliasing_values[anti_aliasing_value].mode;
	anti_aliasing_samples = anti_aliasing_values[anti_aliasing_value].samples;
	if (anti_aliasing_samples > anti_aliasing_maximum_samples)
	{
		platform_log("anti-aliasing: the GPU has at most %d samples a pixel", (int)anti_aliasing_maximum_samples);
		anti_aliasing_samples = anti_aliasing_maximum_samples < 2 ? 0 : anti_aliasing_maximum_samples;
	}
#ifdef HALO_GLES
	if (hdr.active && anti_aliasing_samples > hdr.maximum_samples)
	{
		platform_log("anti-aliasing: the GPU has at most %d samples a pixel of half floats", (int)hdr.maximum_samples);
		anti_aliasing_samples = hdr.maximum_samples < 2 ? 0 : hdr.maximum_samples;
	}
#endif
	if ((mode == _anti_aliasing_fxaa || mode == _anti_aliasing_smaa) && !xgpu_post_prepare(mode == _anti_aliasing_smaa))
		platform_log("anti-aliasing: its programs do not build, so the 3D view is not antialiased");
	xgpu_gl_state_invalidate();
}

Direct3D *WINAPI Direct3DCreate8(UINT sdk_version)
{
	(void)sdk_version;
	return (Direct3D *)1;
}

void WINAPI Direct3D_SetPushBufferSize(DWORD push_buffer_size, DWORD segment_count)
{
	(void)push_buffer_size;
	(void)segment_count;
}

/* each vertex constant register's serial is the value constants_serial took
when the register last changed; a program's registers are current up to
the serial it recorded when it last uploaded them. The serials are 64-bit:
the count rises with every register a draw changes (a skinned model changes
up to 132), and 32 bits wrapped within minutes at a high frame rate, after
which every program's next draw found none of its registers changed and
drew with what it last uploaded (another object's node matrices: vertices
flung across the screen for a frame). */
static unsigned long long constant_serials[XGPU_VERTEX_CONSTANT_COUNT];
static unsigned long long constants_serial;
/* the register each of the latest serials changed, so a program that is
only a little behind finds its changed registers without a full scan */
#define CONSTANT_LOG_SIZE 1024
static unsigned char constant_log[CONSTANT_LOG_SIZE];
/* the registers changed since the last upload (to any program): the
smallest and largest, and the serial that upload was current to. A program
current to that serial needs them, and nothing in the log. The serial is
64-bit as the others are (cut to 32, it matched a program left at an old
serial, which then took only the checkpoint's registers). */
static unsigned long long constants_checkpoint_serial;
static unsigned long constants_checkpoint_first = XGPU_VERTEX_CONSTANT_COUNT, constants_checkpoint_last;

static void constants_store(unsigned long first, const void *data, unsigned long count)
{
	const float (*values)[4] = data;
	unsigned long index;

	for (index = 0; index < count; index++)
	{
		if (memcmp(device.constants[first + index], values[index], sizeof(device.constants[0])))
		{
			memcpy(device.constants[first + index], values[index], sizeof(device.constants[0]));
			constant_serials[first + index] = ++constants_serial;
			constant_log[constants_serial % CONSTANT_LOG_SIZE] = (unsigned char)(first + index);
			if (constants_checkpoint_first > first + index)
				constants_checkpoint_first = first + index;
			if (constants_checkpoint_last < first + index)
				constants_checkpoint_last = first + index;
		}
	}
}

static void viewport_update_constants(void)
{
	/* Direct3D's reserved constants c[-38] and c[-37] map clip space to
	the screen; zscale is the depth buffer's range */
	float zscale = 16777215.0f;
	unsigned long width, height;
	BOOL depth;

	if (device.depth_stencil)
	{
		struct xgpu_texture_description description;

		xgpu_texture_describe(device.depth_stencil->Format, device.depth_stencil->Size, &description);
		if (description.format == D3DFMT_D16 || description.format == D3DFMT_LIN_D16 ||
			description.format == D3DFMT_F16 || description.format == D3DFMT_LIN_F16)
		{
			zscale = 65535.0f;
		}
	}
	(void)width; (void)height; (void)depth;
	device.viewport_scale[0] = device.viewport.Width * 0.5f;
	device.viewport_scale[1] = -(float)device.viewport.Height * 0.5f;
	device.viewport_scale[2] = zscale * (device.viewport.MaxZ - device.viewport.MinZ);
	device.viewport_scale[3] = 0.0f;
	device.viewport_offset[0] = device.viewport.X + device.viewport.Width * 0.5f;
	device.viewport_offset[1] = device.viewport.Y + device.viewport.Height * 0.5f;
	device.viewport_offset[2] = zscale * device.viewport.MinZ;
	device.viewport_offset[3] = 0.0f;
	if (!(device.shader_constant_mode & D3DSCM_NORESERVEDCONSTANTS))
	{
		constants_store(XGPU_VERTEX_CONSTANT_BIAS - 38, device.viewport_scale, 1);
		constants_store(XGPU_VERTEX_CONSTANT_BIAS - 37, device.viewport_offset, 1);
	}
}

HRESULT WINAPI Direct3D_CreateDevice(UINT adapter, D3DDEVTYPE device_type, void *unused, DWORD behavior_flags,
	D3DPRESENT_PARAMETERS *presentation_parameters, D3DDevice **returned_device)
{
	unsigned long width, height;
	int index;

	(void)adapter;
	(void)device_type;
	(void)unused;
	(void)behavior_flags;
	if (!device.created)
	{
		memset(&device, 0, sizeof(device));
		if (presentation_parameters)
			device.presentation = *presentation_parameters;
		width = device.presentation.BackBufferWidth ? device.presentation.BackBufferWidth : 640;
		height = device.presentation.BackBufferHeight ? device.presentation.BackBufferHeight : 480;
#ifdef HALO_ANDROID
		d3d8_surface_initialize(&device.back_buffer, D3DFMT_LIN_A8R8G8B8, width, height);
		d3d8_surface_initialize(&device.depth_buffer, D3DFMT_LIN_D24S8, width, height);
#else
		/* room for the widest screen, which F11 can switch to (the screen's
		width, above) */
		d3d8_surface_initialize(&device.back_buffer, D3DFMT_LIN_A8R8G8B8, SCREEN_MAXIMUM_WIDTH, height);
		d3d8_surface_initialize(&device.depth_buffer, D3DFMT_LIN_D24S8, SCREEN_MAXIMUM_WIDTH, height);
		d3d8_surface_resize(&device.back_buffer, D3DFMT_LIN_A8R8G8B8, width, height);
		d3d8_surface_resize(&device.depth_buffer, D3DFMT_LIN_D24S8, width, height);
#endif
		device.render_target = &device.back_buffer;
		device.depth_stencil = &device.depth_buffer;
		for (index = 0; index < D3DTS_MAX; index++)
		{
			device.transforms[index]._11 = 1.0f;
			device.transforms[index]._22 = 1.0f;
			device.transforms[index]._33 = 1.0f;
			device.transforms[index]._44 = 1.0f;
		}
		device.viewport.Width = width;
		device.viewport.Height = height;
		device.viewport.MaxZ = 1.0f;
		device.next_vertex_shader_id = 1;
		D3D__RenderState[D3DRS_ZENABLE] = TRUE;
		D3D__RenderState[D3DRS_ZWRITEENABLE] = TRUE;
		D3D__RenderState[D3DRS_ZFUNC] = D3DCMP_LESSEQUAL;
		D3D__RenderState[D3DRS_COLORWRITEENABLE] = D3DCOLORWRITEENABLE_ALL;
		D3D__RenderState[D3DRS_SRCBLEND] = D3DBLEND_ONE;
		D3D__RenderState[D3DRS_DESTBLEND] = D3DBLEND_ZERO;
		D3D__RenderState[D3DRS_BLENDOP] = D3DBLENDOP_ADD;
		D3D__RenderState[D3DRS_CULLMODE] = D3DCULL_CCW;
		D3D__RenderState[D3DRS_FRONTFACE] = D3DFRONT_CW;
		D3D__RenderState[D3DRS_FILLMODE] = D3DFILL_SOLID;
		D3D__RenderState[D3DRS_ALPHAFUNC] = D3DCMP_ALWAYS;
		D3D__RenderState[D3DRS_STENCILFUNC] = D3DCMP_ALWAYS;
		D3D__RenderState[D3DRS_STENCILMASK] = 0xff;
		D3D__RenderState[D3DRS_STENCILWRITEMASK] = 0xff;
		D3D__RenderState[D3DRS_STENCILFAIL] = D3DSTENCILOP_KEEP;
		D3D__RenderState[D3DRS_STENCILZFAIL] = D3DSTENCILOP_KEEP;
		D3D__RenderState[D3DRS_STENCILPASS] = D3DSTENCILOP_KEEP;
		for (index = 0; index < D3DTSS_MAXSTAGES; index++)
		{
			D3D__TextureState[index][D3DTSS_ADDRESSU] = D3DTADDRESS_WRAP;
			D3D__TextureState[index][D3DTSS_ADDRESSV] = D3DTADDRESS_WRAP;
			D3D__TextureState[index][D3DTSS_ADDRESSW] = D3DTADDRESS_WRAP;
			D3D__TextureState[index][D3DTSS_MAGFILTER] = D3DTEXF_POINT;
			D3D__TextureState[index][D3DTSS_MINFILTER] = D3DTEXF_POINT;
			D3D__TextureState[index][D3DTSS_MAXANISOTROPY] = 1;
		}
		viewport_update_constants();

		if (!config_boolean("debug.null_renderer") && platform_video_initialize(width, height))
			gl_initialize();
		else
			platform_log("Direct3D: running without a window (nothing is displayed)");
		device.created = TRUE;
	}
	*returned_device = device_pointer();
	return S_OK;
}

/* ---------- the menus' pointer */

/* a point in the window, as SDL reports it, in the menus' coordinates: the
inverse of the letterboxed display blit at presentation, the screen's
width and the menus' centering (halo_screen_ui_offset); or, not centered,
the screen's (the game's drawing: the scoreboard's); x and y are -1 if there
is no back buffer or window yet */
static void ui_point_from_window_on(float window_x, float window_y, int centered, short *x, short *y)
{
	struct render_target_entry *back_buffer = render_target_get(&device.back_buffer);
	int window_width, window_height, pixel_width, pixel_height, width, height, left, top;
	float screen_x, screen_y;

	*x = *y = -1;
	if (!back_buffer)
		return;
	platform_video_window_size(&window_width, &window_height);
	platform_video_drawable_size(&pixel_width, &pixel_height);
	if (window_width <= 0 || window_height <= 0)
		return;
	width = pixel_width;
	height = (int)((long)pixel_width * back_buffer->target.gl_height / back_buffer->target.gl_width);
	if (height > pixel_height)
	{
		height = pixel_height;
		width = (int)((long)pixel_height * back_buffer->target.gl_width / back_buffer->target.gl_height);
	}
	left = (pixel_width - width) / 2;
	top = (pixel_height - height) / 2;
	screen_x = (window_x * pixel_width / window_width - left) * (float)back_buffer->target.width / (float)width;
	screen_y = (window_y * pixel_height / window_height - top) * (float)back_buffer->target.height / (float)height;
	*x = (short)floorf(screen_x - (centered ? (float)(halo_screen_width() - 640) / 2.0f : 0.0f));
	*y = (short)floorf(screen_y);
}

static void ui_point_from_window(float window_x, float window_y, short *x, short *y)
{
	ui_point_from_window_on(window_x, window_y, TRUE, x, y);
}

#ifdef HALO_ANDROID
int halo_scoreboard_pointer_update(int offered, struct halo_ui_pointer *pointer)
{
	(void)offered;
	(void)pointer;
	return -1;
}
#else
int halo_scoreboard_pointer_update(int offered, struct halo_ui_pointer *pointer)
{
	struct platform_ui_pointer state;

	memset(pointer, 0, sizeof(*pointer));
	if (!platform_scoreboard_pointer(offered != 0, &state) || !device.gl_ready)
		return 0;
	ui_point_from_window_on(state.x, state.y, FALSE, &pointer->x, &pointer->y);
	ui_point_from_window_on(state.click_x, state.click_y, FALSE, &pointer->click_x, &pointer->click_y);
	pointer->moved = state.moved != FALSE;
	pointer->left_clicks = (unsigned char)(state.left_clicks < 255 ? state.left_clicks : 255);
	return 1;
}
#endif

int halo_ui_pointer_update(int menus_active, struct halo_ui_pointer *pointer)
{
	struct platform_ui_pointer state;

	platform_menus_set_active(menus_active != 0);
	platform_ui_pointer_set_active(menus_active != 0);
	if (!menus_active || !device.gl_ready || !platform_ui_pointer_read(&state))
		return 0;
	memset(pointer, 0, sizeof(*pointer));
	ui_point_from_window(state.x, state.y, &pointer->x, &pointer->y);
	ui_point_from_window(state.click_x, state.click_y, &pointer->click_x, &pointer->click_y);
	pointer->moved = state.moved != FALSE;
	pointer->left_clicks = (unsigned char)(state.left_clicks < 255 ? state.left_clicks : 255);
	pointer->right_clicks = (unsigned char)(state.right_clicks < 255 ? state.right_clicks : 255);
	pointer->wheel_steps = (signed char)(state.wheel_steps < -8 ? -8 : state.wheel_steps > 8 ? 8 : state.wheel_steps);
	pointer->touch = state.touch != FALSE;
	pointer->downs = (unsigned char)(state.downs < 255 ? state.downs : 255);
	ui_point_from_window(state.down_x, state.down_y, &pointer->down_x, &pointer->down_y);
	return 1;
}

/* takes up the display's shape and resolution, or the window's, if they
have changed; between frames, since the game's layout and the targets must
agree for a whole frame. Returns the width the game draws. */
long halo_screen_commit(void)
{
	long width;
	float scale[2];

	/* (display.anti_aliasing, as Settings or config.toml has it now:
	supersampling changes the scale below) */
	anti_aliasing_read();
	/* display.hdr, and how bright the display goes (an HDR display's
	headroom, with display.hdr_output's floating-point window) */
	hdr_read();
#ifndef HALO_ANDROID
	if (hdr.float_window)
	{
		float sdr_white, headroom, paper, peak;
		BOOL on = platform_video_hdr(&sdr_white, &headroom);
		/* (the calibration screen's, while it is drawn) */
		BOOL calibrating = hdr_calibration_shown(TRUE);
		int paper_white = calibrating ? hdr_calibration.paper_white : hdr.paper_white;
		int peak_brightness = calibrating ? hdr_calibration.peak_brightness : hdr.peak_brightness;

		hdr.display_on = on;
		/* (in nits: scRGB's 1.0 is 80) */
		paper = paper_white ? (float)paper_white : sdr_white * 80.0f;
		peak = peak_brightness ? (float)peak_brightness : headroom * sdr_white * 80.0f;
		if (!on)
		{
			paper = sdr_white * 80.0f;
			peak = paper;
		}
		if (peak < paper)
			peak = paper;
		if (paper != hdr.sdr_white * 80.0f || peak / paper != hdr.tone_mapping.peak)
		{
			if (on && !calibrating)
				platform_log("HDR: the display's paper white %.0f nits%s, peak %.0f nits%s", paper,
					paper_white ? "" : " (Windows')", peak, peak_brightness ? "" : " (Windows')");
		}
		hdr.sdr_white = paper / 80.0f;
		hdr.tone_mapping.peak = peak / paper;
		hdr.tone_mapping.paper = on ? paper / 100.0f : 0.0f;
	}
#endif
	/* display.shadow_resolution, if it has changed: the maps' entries at
	the old scale are freed (render_targets_evict), new ones made at the new */
	if (shadow_scale && shadow_scale_read_at != config_changes())
	{
		long shadow = shadow_scale_choose();

		shadow_scale_read_at = config_changes();
		if (shadow != shadow_scale)
		{
			platform_log("shadow maps: %ldx%ld", SHADOW_MAP_SIZE * shadow, SHADOW_MAP_SIZE * shadow);
			shadow_scale = shadow;
			render_targets_evict();
		}
	}
	if (!screen_width)
		return halo_screen_width();
	screen_mode_choose(&width, scale);
	if (width != screen_width || scale[0] != screen_scale[0] || scale[1] != screen_scale[1])
	{
		platform_log("screen: %ldx%d drawn at %.0fx%.0f", width, SCREEN_HEIGHT,
			width * scale[0], SCREEN_HEIGHT * scale[1]);
		screen_width = width;
		screen_scale[0] = scale[0];
		screen_scale[1] = scale[1];
#ifndef HALO_ANDROID
		if (device.created)
		{
			device.presentation.BackBufferWidth = (UINT)width;
			d3d8_surface_resize(&device.back_buffer, D3DFMT_LIN_A8R8G8B8, (unsigned long)width, SCREEN_HEIGHT);
			d3d8_surface_resize(&device.depth_buffer, D3DFMT_LIN_D24S8, (unsigned long)width, SCREEN_HEIGHT);
		}
#endif
		/* (the entries of the old size and scale, which nothing finds now) */
		render_targets_evict();
	}
	return screen_width;
}

ULONG WINAPI D3DDevice_Release(void)
{
	return 1;
}

void WINAPI D3DDevice_GetDeviceCaps(D3DCAPS8 *caps)
{
	memset(caps, 0, sizeof(*caps));
	caps->DeviceType = D3DDEVTYPE_HAL;
	caps->MaxTextureWidth = 4096;
	caps->MaxTextureHeight = 4096;
	caps->MaxVolumeExtent = 512;
	caps->MaxTextureRepeat = 8192;
	caps->MaxTextureAspectRatio = 4096;
	caps->MaxAnisotropy = 4;
	caps->MaxTextureBlendStages = 4;
	caps->MaxSimultaneousTextures = 4;
	caps->MaxActiveLights = 8;
	caps->MaxVertexBlendMatrices = 4;
	caps->MaxPointSize = 64.0f;
	caps->MaxPrimitiveCount = 0xfffff;
	caps->MaxVertexIndex = 0xffff;
	caps->MaxStreams = 16;
	caps->MaxStreamStride = 255;
	caps->VertexShaderVersion = D3DVS_VERSION(1, 1);
	caps->MaxVertexShaderConst = 192;
	caps->PixelShaderVersion = D3DPS_VERSION(1, 1);
	caps->MaxPixelShaderValue = 1.0f;
}

static BOOL front_buffer_ready(void);

void WINAPI D3DDevice_GetBackBuffer(INT back_buffer, D3DBACKBUFFER_TYPE type, D3DSurface **result)
{
	(void)type;
	/* -1: the front buffer, the frame shown last, which the loading screen
	blurs into its light's trails (progress_bar.c) */
	if (back_buffer == -1 && front_buffer_ready())
	{
		device.front_buffer.Common++;
		*result = &device.front_buffer;
		return;
	}
	/* like Direct3D, the caller gets a reference it must release */
	device.back_buffer.Common++;
	*result = &device.back_buffer;
}

HRESULT WINAPI D3DDevice_GetDepthStencilSurface(D3DSurface **result)
{
	*result = device.depth_stencil;
	if (!*result)
		return D3DERR_NOTFOUND;
	(*result)->Common++;
	return S_OK;
}

static BOOL trace_frame(void);

void WINAPI D3DDevice_SetRenderTarget(D3DSurface *render_target, D3DSurface *depth_stencil)
{
	if (trace_frame())
		platform_log("set render target %08lx depth %08lx", render_target ? (unsigned long)render_target->Data : 0,
			depth_stencil ? (unsigned long)depth_stencil->Data : 0);
	stats.target_changes++;
	if (render_target)
		device.render_target = render_target;
	device.depth_stencil = depth_stencil;
	/* like Direct3D, reset the viewport to the whole new target */
	if (device.render_target)
	{
		unsigned long width, height;
		BOOL depth;

		surface_dimensions(device.render_target, &width, &height, &depth);
		device.viewport.X = 0;
		device.viewport.Y = 0;
		device.viewport.Width = width;
		device.viewport.Height = height;
		device.viewport.MinZ = 0.0f;
		device.viewport.MaxZ = 1.0f;
	}
	viewport_update_constants();
}

void WINAPI D3DDevice_SetViewport(CONST D3DVIEWPORT8 *viewport)
{
	device.viewport = *viewport;
	viewport_update_constants();
}

void WINAPI D3DDevice_SetTransform(D3DTRANSFORMSTATETYPE state, CONST D3DMATRIX *matrix)
{
	if ((unsigned long)state < D3DTS_MAX)
		device.transforms[state] = *matrix;
}

void WINAPI D3DDevice_GetTransform(D3DTRANSFORMSTATETYPE state, D3DMATRIX *matrix)
{
	if ((unsigned long)state < D3DTS_MAX)
		*matrix = device.transforms[state];
}

void WINAPI D3DDevice_SetFlickerFilter(DWORD filter) { (void)filter; }
void WINAPI D3DDevice_SetSoftDisplayFilter(BOOL enable) { (void)enable; }

void WINAPI D3DDevice_SetShaderConstantMode(D3DSHADERCONSTANTMODE mode)
{
	device.shader_constant_mode = mode;
	viewport_update_constants();
}

/* ---------- GPU synchronisation: GL keeps its own ordering */

BOOL WINAPI D3DDevice_IsBusy(void)
{
	return FALSE;
}

void WINAPI D3DDevice_KickPushBuffer(void)
{
	if (device.gl_ready)
		glFlush();
}

void WINAPI D3DDevice_InsertCallback(D3DCALLBACKTYPE type, D3DCALLBACK callback, DWORD context)
{
	(void)type;
	/* the "GPU" reaches the callback immediately */
	if (callback)
		callback(context);
}

/* ---------- visibility (occlusion) tests */

#ifndef HALO_GLES
static int visibility_query_compare(const void *left, const void *right)
{
	GLuint a = *(const GLuint *)left, b = *(const GLuint *)right;

	return (a > b) - (a < b);
}

static BOOL visibility_queries_initialize(void)
{
	GLuint queries[VISIBILITY_TEST_SLOTS * VISIBILITY_BATCH_COUNT] = {0};
	unsigned long count = VISIBILITY_TEST_SLOTS * VISIBILITY_BATCH_COUNT;
	unsigned long i, zeros = 0, duplicates = 0, invalid = 0;
	GLenum error;
	BOOL success = TRUE;

	/* every batch's queries at once, made with glCreateQueries (with
	glGenQueries, AMD's driver gave some names as 0 or twice), and checked
	before any is used */
	while ((error = glGetError()) != GL_NO_ERROR)
		platform_log("GPU: GL error 0x%x before visibility query creation", error);
	glCreateQueries(VISIBILITY_QUERY, (GLsizei)count, queries);
	qsort(queries, count, sizeof(queries[0]), visibility_query_compare);
	for (i = 0; i < count; i++)
	{
		zeros += queries[i] == 0;
		duplicates += i && queries[i] && queries[i] == queries[i - 1];
		invalid += queries[i] && !glIsQuery(queries[i]);
	}
	while ((error = glGetError()) != GL_NO_ERROR)
	{
		success = FALSE;
		platform_log("GPU: GL error 0x%x creating visibility queries", error);
	}
	if (!success || zeros || duplicates || invalid)
	{
		platform_log("GPU: of %lu visibility test queries, %lu are 0, %lu repeated and %lu not queries",
			count, zeros, duplicates, invalid);
		return FALSE;
	}
	for (i = 0; i < VISIBILITY_BATCH_COUNT; i++)
		memcpy(device.visibility_batches[i].queries, queries + VISIBILITY_TEST_SLOTS * i,
			sizeof(device.visibility_batches[i].queries));
	return TRUE;
}

static void visibility_results_initialize(void)
{
	const char *vendor = (const char *)glGetString(GL_VENDOR);
	GLsizeiptr size = VISIBILITY_BATCH_COUNT * VISIBILITY_TEST_SLOTS * sizeof(GLuint);
	GLbitfield flags = GL_MAP_READ_BIT | GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT | GL_MAP_COHERENT_BIT;
	BOOL success;

	device.visibility_logging = config_boolean("debug.gl_debug");
	/* AMD's own driver gives wrong counts copied into a query buffer (lens
	flares and lights go missing or flicker), so there each query is read
	on the CPU once it is available. Elsewhere the GPU copies the counts
	into the buffer (Mesa's GL thread waits for everything before a
	glGetQueryObjectuiv, even one into a bound buffer), each batch into its
	own part, with a fence after the copies. */
	if (!vendor || strcmp(vendor, "ATI Technologies Inc."))
	{
		visibility_readback_gl_ok("before buffer creation");
		glGenBuffers(1, &device.visibility_buffer);
		glBindBuffer(GL_QUERY_BUFFER, device.visibility_buffer);
		glBufferStorage(GL_QUERY_BUFFER, size, NULL, flags);
		device.visibility_mapped = glMapBufferRange(GL_QUERY_BUFFER, 0, size, flags);
		glBindBuffer(GL_QUERY_BUFFER, 0);
		success = visibility_readback_gl_ok("buffer creation");
		if (!success || !device.visibility_buffer || !device.visibility_mapped)
		{
			glDeleteBuffers(1, &device.visibility_buffer);
			device.visibility_buffer = 0;
			device.visibility_mapped = NULL;
			platform_log("GPU: cannot map the visibility test results; queries are read on the CPU");
		}
	}
	else
		platform_log("GPU: AMD's driver; visibility test queries are read on the CPU");
}

static BOOL visibility_readback_gl_ok(const char *phase)
{
	GLenum error;
	BOOL success = TRUE;

	while ((error = glGetError()) != GL_NO_ERROR)
	{
		device.visibility_stats.failures++;
		platform_log("GPU: visibility readback GL error 0x%x at %s", error, phase);
		success = FALSE;
	}
	return success;
}
#endif

void WINAPI D3DDevice_BeginVisibilityTest(void)
{
	if (!device.gl_ready || device.visibility_test_active)
		return;
	/* the test's slot is given only when it ends */
	device.visibility_test_active = TRUE;
#ifdef HALO_GLES
	if (xgpu_capabilities.atomic_counters)
	{
		const GLuint zero = 0;

		device.counter_next = (device.counter_next + 1) % VISIBILITY_TEST_SLOTS;
		device.counter_active = device.counter_next;
		glBindBuffer(GL_ATOMIC_COUNTER_BUFFER, device.visibility_counters);
		host_gl_buffer_write(GL_ATOMIC_COUNTER_BUFFER, (unsigned int)(device.counter_active * sizeof(GLuint)),
			sizeof(zero), &zero);
		glBindBuffer(GL_ATOMIC_COUNTER_BUFFER, 0);
		return;
	}
#else
	{
		device.active_query = 0;
		if (device.visibility_unavailable)
			return;
		if (device.visibility_batch < VISIBILITY_BATCH_COUNT)
		{
			struct visibility_batch *batch = &device.visibility_batches[device.visibility_batch];

			if (!batch->sealed && !batch->failed && batch->count < VISIBILITY_TEST_SLOTS)
				device.active_query = batch->queries[batch->count];
		}
		/* (with every batch waiting to be read, no query: the test is
		dropped, rather than waiting for the GPU or reusing a query whose
		count is unread) */
		if (device.active_query)
			glBeginQuery(VISIBILITY_QUERY, device.active_query);
		else
			device.visibility_stats.dropped++;
		return;
	}
#endif
#ifdef HALO_GLES
	glBeginQuery(VISIBILITY_QUERY, device.scratch_query);
#endif
}

HRESULT WINAPI D3DDevice_EndVisibilityTest(DWORD index)
{
	ULONGLONG test;
#ifdef HALO_GLES
	GLuint scratch;
#endif

	if (!device.gl_ready || !device.visibility_test_active)
		return S_OK;
	device.visibility_test_active = FALSE;
	index %= VISIBILITY_TEST_SLOTS;
	/* (numbered whether its count will come or not) */
	test = ++device.visibility_ended[index];
#ifdef HALO_GLES
	if (xgpu_capabilities.atomic_counters)
	{
		unsigned long *count = &device.ring_test_count[device.buffer_ring];

		/* (a slot tested twice in a frame is listed twice: the later
		counter, resolved after, wins) */
		if (*count < VISIBILITY_TEST_SLOTS)
		{
			device.ring_tests[device.buffer_ring][*count][0] = (unsigned short)index;
			device.ring_tests[device.buffer_ring][*count][1] = (unsigned short)device.counter_active;
			device.ring_test_numbers[device.buffer_ring][*count] = test;
			(*count)++;
		}
		device.query_pending[index] = TRUE;
		return S_OK;
	}
#else
	if (!device.active_query)
	{
		if (device.visibility_unavailable)
		{
			device.visibility_known[index] = VISIBILITY_ALL_SAMPLES;
			device.visibility_counted[index] = test;
			device.query_pending[index] = TRUE;
		}
		return S_OK;
	}
#endif
	glEndQuery(VISIBILITY_QUERY);
	/* the target's samples to a game pixel (its pixels, by its samples a
	pixel with multisampling): the result is a count of the game's pixels
	which the game divides by its own test's area
	(lens flares, rasterizer_lights.c), a split-screen window's or the
	screen's alike */
	device.query_area[index] = target_scale[0] * target_scale[1] * (float)target_samples;
#ifndef HALO_GLES
	{
		struct visibility_batch *batch = &device.visibility_batches[device.visibility_batch];

		batch->slots[batch->count] = (unsigned short)index;
		batch->tests[batch->count] = test;
		batch->areas[batch->count++] = device.query_area[index];
		device.query_pending[index] = TRUE;
		return S_OK;
	}
#else
	/* swap the scratch query into the requested slot */
	scratch = device.scratch_query;
	device.scratch_query = device.queries[index];
	device.queries[index] = scratch;
	device.query_tests[index] = test;
	device.query_pending[index] = TRUE;
	device.visibility_unread[index] = TRUE;
	return S_OK;
#endif
}

HRESULT WINAPI D3DDevice_GetVisibilityTestResult(DWORD index, UINT *result, ULONGLONG *time_stamp)
{
#ifdef HALO_GLES
	GLuint available = 0, samples = 0;
#endif

	if (time_stamp)
		*time_stamp = 0;
	index %= VISIBILITY_TEST_SLOTS;
	if (!device.gl_ready || !device.query_pending[index])
	{
		if (result)
			*result = 0;
		return S_OK;
	}
#ifdef HALO_GLES
	if (xgpu_capabilities.atomic_counters)
	{
		/* the latest count the GPU has finished (counter_snapshots) */
		if (result)
			*result = device.visibility_latest[index];
		return S_OK;
	}
	/* the latest count known: from this test, or while the GPU is still
	behind, from the slot's earlier ones */
	if (device.visibility_unread[index])
	{
		glGetQueryObjectuiv(device.queries[index], GL_QUERY_RESULT_AVAILABLE, &available);
		if (available)
		{
			glGetQueryObjectuiv(device.queries[index], GL_QUERY_RESULT, &samples);
			/* ES only says whether any sample passed. The game divides the
			count by the test's area (lens flare brightness,
			rasterizer_lights.c): report more than any test covers. */
			if (samples)
				samples = VISIBILITY_ALL_SAMPLES;
			device.visibility_known[index] = samples;
			device.visibility_counted[index] = device.query_tests[index];
			device.visibility_unread[index] = FALSE;
		}
	}
#endif
	if (result)
		*result = device.visibility_known[index];
	return S_OK;
}

/* (xgpu.h) */
unsigned long long xgpu_visibility_tests_ended(unsigned long index)
{
	return device.visibility_ended[index % VISIBILITY_TEST_SLOTS];
}

unsigned long long xgpu_visibility_test_counted(unsigned long index)
{
	return device.visibility_counted[index % VISIBILITY_TEST_SLOTS];
}

/* ---------- render and texture stage state */

void D3DFASTCALL D3DDevice_SetRenderState_Simple(DWORD method, DWORD value)
{
	/* callers also store the value in D3D__RenderState themselves */
	(void)method;
	(void)value;
}

void D3DFASTCALL D3DDevice_SetRenderState_Deferred(D3DRENDERSTATETYPE state, DWORD value)
{
	if ((unsigned long)state < D3DRS_MAX)
		D3D__RenderState[state] = value;
}

void WINAPI D3DDevice_SetRenderState_ZBias(DWORD value);

void WINAPI D3DDevice_SetRenderStateNotInline(D3DRENDERSTATETYPE state, DWORD value)
{
	if (state == D3DRS_ZBIAS)
		D3DDevice_SetRenderState_ZBias(value);
	else if ((unsigned long)state < D3DRS_MAX)
		D3D__RenderState[state] = value;
}

/* As the Xbox's D3D8 does it: a z bias is a polygon offset of -bias depth
units plus -bias/4 times the polygon's depth slope, enabled for every fill
mode. Without the slope term, decals (biased by 8) fight with the surface
under them wherever it is seen at an angle. */
void WINAPI D3DDevice_SetRenderState_ZBias(DWORD value)
{
	float offset = -(float)value;
	float slope = offset * 0.25f;
	DWORD enable = value != 0;

	memcpy(&D3D__RenderState[D3DRS_POLYGONOFFSETZSLOPESCALE], &slope, sizeof(slope));
	memcpy(&D3D__RenderState[D3DRS_POLYGONOFFSETZOFFSET], &offset, sizeof(offset));
	D3D__RenderState[D3DRS_POINTOFFSETENABLE] = enable;
	D3D__RenderState[D3DRS_WIREFRAMEOFFSETENABLE] = enable;
	D3D__RenderState[D3DRS_SOLIDOFFSETENABLE] = enable;
	D3D__RenderState[D3DRS_ZBIAS] = value;
}

#define COMPLEX_RENDER_STATE(name, state) \
	void WINAPI D3DDevice_SetRenderState_##name(DWORD value) { D3D__RenderState[state] = value; }

COMPLEX_RENDER_STATE(PSTextureModes, D3DRS_PSTEXTUREMODES)
COMPLEX_RENDER_STATE(VertexBlend, D3DRS_VERTEXBLEND)
COMPLEX_RENDER_STATE(FogColor, D3DRS_FOGCOLOR)
COMPLEX_RENDER_STATE(FillMode, D3DRS_FILLMODE)
COMPLEX_RENDER_STATE(BackFillMode, D3DRS_BACKFILLMODE)
COMPLEX_RENDER_STATE(TwoSidedLighting, D3DRS_TWOSIDEDLIGHTING)
COMPLEX_RENDER_STATE(NormalizeNormals, D3DRS_NORMALIZENORMALS)
COMPLEX_RENDER_STATE(ZEnable, D3DRS_ZENABLE)
COMPLEX_RENDER_STATE(StencilEnable, D3DRS_STENCILENABLE)
COMPLEX_RENDER_STATE(StencilFail, D3DRS_STENCILFAIL)
COMPLEX_RENDER_STATE(FrontFace, D3DRS_FRONTFACE)
COMPLEX_RENDER_STATE(CullMode, D3DRS_CULLMODE)
COMPLEX_RENDER_STATE(TextureFactor, D3DRS_TEXTUREFACTOR)
COMPLEX_RENDER_STATE(LogicOp, D3DRS_LOGICOP)
COMPLEX_RENDER_STATE(EdgeAntiAlias, D3DRS_EDGEANTIALIAS)
COMPLEX_RENDER_STATE(MultiSampleAntiAlias, D3DRS_MULTISAMPLEANTIALIAS)
COMPLEX_RENDER_STATE(MultiSampleMask, D3DRS_MULTISAMPLEMASK)
COMPLEX_RENDER_STATE(MultiSampleType, D3DRS_MULTISAMPLETYPE)
COMPLEX_RENDER_STATE(ShadowFunc, D3DRS_SHADOWFUNC)
COMPLEX_RENDER_STATE(LineWidth, D3DRS_LINEWIDTH)
COMPLEX_RENDER_STATE(Dxt1NoiseEnable, D3DRS_DXT1NOISEENABLE)
COMPLEX_RENDER_STATE(YuvEnable, D3DRS_YUVENABLE)
COMPLEX_RENDER_STATE(OcclusionCullEnable, D3DRS_OCCLUSIONCULLENABLE)
COMPLEX_RENDER_STATE(StencilCullEnable, D3DRS_STENCILCULLENABLE)
COMPLEX_RENDER_STATE(RopZCmpAlwaysRead, D3DRS_ROPZCMPALWAYSREAD)
COMPLEX_RENDER_STATE(RopZRead, D3DRS_ROPZREAD)
COMPLEX_RENDER_STATE(DoNotCullUncompressed, D3DRS_DONOTCULLUNCOMPRESSED)

void D3DFASTCALL D3DDevice_SetTextureState_Deferred(DWORD stage, D3DTEXTURESTAGESTATETYPE type, DWORD value)
{
	if (stage < D3DTSS_MAXSTAGES && (unsigned long)type < D3DTSS_MAX)
		D3D__TextureState[stage][type] = value;
}

void WINAPI D3DDevice_SetTextureState_TexCoordIndex(DWORD stage, DWORD value)
{
	if (stage < D3DTSS_MAXSTAGES)
		D3D__TextureState[stage][D3DTSS_TEXCOORDINDEX] = value;
}

void WINAPI D3DDevice_SetTextureState_BorderColor(DWORD stage, DWORD value)
{
	if (stage < D3DTSS_MAXSTAGES)
		D3D__TextureState[stage][D3DTSS_BORDERCOLOR] = value;
}

void WINAPI D3DDevice_SetTextureState_ColorKeyColor(DWORD stage, DWORD value)
{
	if (stage < D3DTSS_MAXSTAGES)
		D3D__TextureState[stage][D3DTSS_COLORKEYCOLOR] = value;
}

void WINAPI D3DDevice_SetTextureState_BumpEnv(DWORD stage, D3DTEXTURESTAGESTATETYPE type, DWORD value)
{
	if (stage < D3DTSS_MAXSTAGES && (unsigned long)type < D3DTSS_MAX)
		D3D__TextureState[stage][type] = value;
}

void WINAPI D3DDevice_SetTexture(DWORD stage, D3DBaseTexture *texture)
{
	if (stage < D3DTSS_MAXSTAGES)
		device.textures[stage] = texture;
}

#ifdef HALO_ANDROID
/* Remove CE headers before their owner frees or reuses them. */
void android_ce_texture_unbind(const void *resource)
{
	unsigned long stage;
	for (stage = 0; stage < D3DTSS_MAXSTAGES; stage++)
		if ((const void *)device.textures[stage] == resource)
			device.textures[stage] = NULL;
	xgpu_gl_state_invalidate();
}
#endif

void WINAPI D3DDevice_SetPalette(DWORD stage, D3DPalette *palette)
{
	if (stage < D3DTSS_MAXSTAGES)
		device.palettes[stage] = palette;
}

void WINAPI D3DDevice_SetPixelShaderProgram(D3DPIXELSHADERDEF *definition)
{
	/* the definition's members are the pixel shader render states */
	if (!definition)
		return;
	memcpy(&D3D__RenderState[D3DRS_PSALPHAINPUTS0], definition->PSAlphaInputs, sizeof(definition->PSAlphaInputs));
	D3D__RenderState[D3DRS_PSFINALCOMBINERINPUTSABCD] = definition->PSFinalCombinerInputsABCD;
	D3D__RenderState[D3DRS_PSFINALCOMBINERINPUTSEFG] = definition->PSFinalCombinerInputsEFG;
	memcpy(&D3D__RenderState[D3DRS_PSCONSTANT0_0], definition->PSConstant0, sizeof(definition->PSConstant0));
	memcpy(&D3D__RenderState[D3DRS_PSCONSTANT1_0], definition->PSConstant1, sizeof(definition->PSConstant1));
	memcpy(&D3D__RenderState[D3DRS_PSALPHAOUTPUTS0], definition->PSAlphaOutputs, sizeof(definition->PSAlphaOutputs));
	memcpy(&D3D__RenderState[D3DRS_PSRGBINPUTS0], definition->PSRGBInputs, sizeof(definition->PSRGBInputs));
	D3D__RenderState[D3DRS_PSCOMPAREMODE] = definition->PSCompareMode;
	D3D__RenderState[D3DRS_PSFINALCOMBINERCONSTANT0] = definition->PSFinalCombinerConstant0;
	D3D__RenderState[D3DRS_PSFINALCOMBINERCONSTANT1] = definition->PSFinalCombinerConstant1;
	memcpy(&D3D__RenderState[D3DRS_PSRGBOUTPUTS0], definition->PSRGBOutputs, sizeof(definition->PSRGBOutputs));
	D3D__RenderState[D3DRS_PSCOMBINERCOUNT] = definition->PSCombinerCount;
	D3D__RenderState[D3DRS_PSTEXTUREMODES] = definition->PSTextureModes;
	D3D__RenderState[D3DRS_PSDOTMAPPING] = definition->PSDotMapping;
	D3D__RenderState[D3DRS_PSINPUTTEXTURE] = definition->PSInputTexture;
}

/* ---------- vertex shaders */

static unsigned long vertex_type_bytes(unsigned long type)
{
	switch (type)
	{
	case D3DVSDT_FLOAT1: return 4;
	case D3DVSDT_FLOAT2: return 8;
	case D3DVSDT_FLOAT3: return 12;
	case D3DVSDT_FLOAT4: return 16;
	case D3DVSDT_D3DCOLOR: return 4;
	case D3DVSDT_SHORT1: return 2;
	case D3DVSDT_SHORT2: return 4;
	case D3DVSDT_SHORT3: return 6;
	case D3DVSDT_SHORT4: return 8;
	case D3DVSDT_NORMSHORT1: return 2;
	case D3DVSDT_NORMSHORT2: return 4;
	case D3DVSDT_NORMSHORT3: return 6;
	case D3DVSDT_NORMSHORT4: return 8;
	case D3DVSDT_NORMPACKED3: return 4;
	case D3DVSDT_PBYTE1: return 1;
	case D3DVSDT_PBYTE2: return 2;
	case D3DVSDT_PBYTE3: return 3;
	case D3DVSDT_PBYTE4: return 4;
	case D3DVSDT_FLOAT2H: return 12;
	default: return 0;
	}
}

static void parse_declaration(struct vertex_shader_object *object, const DWORD *declaration)
{
	unsigned long stream = 0;
	unsigned long offsets[16] = { 0 };

	for (; declaration && *declaration != D3DVSD_END(); declaration++)
	{
		DWORD token = *declaration;
		unsigned long token_type = (token & D3DVSD_TOKENTYPEMASK) >> D3DVSD_TOKENTYPESHIFT;

		switch (token_type)
		{
		case D3DVSD_TOKEN_STREAM:
			stream = token & D3DVSD_STREAMNUMBERMASK;
			break;
		case D3DVSD_TOKEN_STREAMDATA:
			if (token & D3DVSD_DATALOADTYPEMASK)
			{
				/* skip: the count is in dwords, or in bytes with bit 27 */
				unsigned long count = (token & D3DVSD_SKIPCOUNTMASK) >> D3DVSD_SKIPCOUNTSHIFT;

				offsets[stream] += (token & 0x08000000) ? count : count * 4;
			}
			else if (object->element_count < XGPU_VERTEX_ATTRIBUTE_COUNT)
			{
				struct vertex_element *element = &object->elements[object->element_count++];

				element->reg = (unsigned char)(token & D3DVSD_VERTEXREGMASK);
				element->stream = (unsigned char)stream;
				element->type = (unsigned char)((token & D3DVSD_DATATYPEMASK) >> D3DVSD_DATATYPESHIFT);
				element->bytes = (unsigned char)vertex_type_bytes(element->type);
				element->offset = (unsigned short)offsets[stream];
				offsets[stream] += element->bytes;
				if (element->type == D3DVSDT_NORMPACKED3)
					object->packed_mask |= 1UL << element->reg;
			}
			break;
		case D3DVSD_TOKEN_CONSTMEM:
			declaration += ((token & D3DVSD_CONSTCOUNTMASK) >> D3DVSD_CONSTCOUNTSHIFT) * 4;
			break;
		case D3DVSD_TOKEN_EXT:
			declaration += (token & D3DVSD_EXTCOUNTMASK) >> D3DVSD_EXTCOUNTSHIFT;
			break;
		default:
			break;
		}
	}
}

HRESULT WINAPI D3DDevice_CreateVertexShader(CONST DWORD *declaration, CONST DWORD *function, DWORD *handle, DWORD usage)
{
	struct vertex_shader_object *object = calloc(1, sizeof(*object));

	(void)usage;
	if (!object)
		return E_OUTOFMEMORY;
	object->signature = VERTEX_SHADER_SIGNATURE;
	object->id = device.next_vertex_shader_id++;
	if (function)
	{
		/* header: program type in the low word, instruction count in the high */
		object->instruction_count = function[0] >> 16;
		object->instructions = malloc(object->instruction_count * 4 * sizeof(DWORD));
		memcpy(object->instructions, function + 1, object->instruction_count * 4 * sizeof(DWORD));
	}
	parse_declaration(object, declaration);
	/* odd values are FVF codes; programmable shader handles are even */
	*handle = (DWORD)object;
	return S_OK;
}

static struct vertex_shader_object *vertex_shader_from_handle(DWORD handle)
{
	struct vertex_shader_object *object = (struct vertex_shader_object *)handle;

	if (!handle || (handle & 1) || object->signature != VERTEX_SHADER_SIGNATURE)
		return NULL;
	return object;
}

/* the game names its model lighting programs as it creates them
(rasterizer_xbox_vertex_shaders_initialize.c), so that their draws can be lit
for each pixel (display.per_pixel_lighting); one whose lighting is not as
the pixel shader computes it stays lit for each vertex */
void halo_vertex_shader_lighting(unsigned long handle)
{
	struct vertex_shader_object *object = vertex_shader_from_handle((DWORD)handle);

	if (!object || !object->instructions)
		return;
	if (!nv2a_vertex_shader_lighting(object->instructions, object->instruction_count, &object->lighting))
	{
		memset(&object->lighting, 0, sizeof(object->lighting));
		platform_log("GPU: vertex shader %lu is not lit as the pixel shader would light it: lit for each vertex",
			object->id);
	}
}

void WINAPI D3DDevice_DeleteVertexShader(DWORD handle)
{
	/* programs stay cached; the object is small */
	(void)handle;
}

void WINAPI D3DDevice_SetVertexShader(DWORD handle)
{
	struct vertex_shader_object *object = vertex_shader_from_handle(handle);

	if (object)
	{
		device.vertex_shader = object;
		device.program_address = 0;
		device.program_slots[0] = object;
		device.fixed_function = FALSE;
	}
	/* 0: the fixed-function pipeline, which only the loading screen uses
	(progress_bar.c). Another handle that is no shader (NONE, of a shader
	that could not be made) changes nothing, as before */
	else if (!handle)
		device.fixed_function = TRUE;
}

void WINAPI D3DDevice_LoadVertexShader(DWORD handle, DWORD address)
{
	if (address < VERTEX_PROGRAM_SLOTS)
		device.program_slots[address] = vertex_shader_from_handle(handle);
}

void WINAPI D3DDevice_SelectVertexShader(DWORD handle, DWORD address)
{
	struct vertex_shader_object *object = vertex_shader_from_handle(handle);

	/* (a program, loaded or packed at the address: never the fixed-function
	pipeline, which SetVertexShader with 0 sets) */
	device.fixed_function = FALSE;
	if (object)
		device.vertex_shader = object;
	if (address < VERTEX_PROGRAM_SLOTS)
		device.program_address = address;
}

void WINAPI D3DDevice_GetVertexShaderSize(DWORD handle, UINT *size)
{
	struct vertex_shader_object *object = vertex_shader_from_handle(handle);

	*size = object ? object->instruction_count : 0;
}

void WINAPI D3DDevice_SetVertexShaderConstant(INT reg, CONST void *constant_data, DWORD constant_count)
{
	long first = reg + XGPU_VERTEX_CONSTANT_BIAS;

	if (first < 0 || first >= XGPU_VERTEX_CONSTANT_COUNT)
		return;
	if (first + (long)constant_count > XGPU_VERTEX_CONSTANT_COUNT)
		constant_count = XGPU_VERTEX_CONSTANT_COUNT - first;
	constants_store((unsigned long)first, constant_data, constant_count);
}

/* the program that runs: the one loaded at the selected address, else the
current shader's own */
static struct vertex_shader_object *current_program(void)
{
	struct vertex_shader_object *program = device.program_slots[device.program_address];

	return program ? program : device.vertex_shader;
}

/* the fixed-function pipeline as a program (SetVertexShader with 0): its
immediate-mode draws only, whose positions D3DDevice_End
transforms by the world, view and projection matrices (nv2a_fixed_function_glsl).
Draws from vertex buffers have no declaration to read them by, and are not
drawn (no game draw makes one). */
static struct vertex_shader_object *fixed_function_program(void)
{
	static struct vertex_shader_object program;
	static DWORD no_instructions[4];

	if (!program.instructions)
	{
		char *source = nv2a_fixed_function_glsl();

		program.signature = VERTEX_SHADER_SIGNATURE;
		program.instructions = no_instructions;
		program.shader[1] = xgpu_compile_shader(GL_VERTEX_SHADER, source, "fixed-function vertex");
		/* (vertex_shader_get: neither variant compiled from instructions) */
		program.shaders_tried = 1 | 2;
		free(source);
	}
	return &program;
}

/* a position in immediate mode by the fixed-function pipeline's transforms:
Direct3D's row vectors, times world, view, then projection */
static void fixed_function_transform(float *position)
{
	static const D3DTRANSFORMSTATETYPE order[3] = { D3DTS_WORLD, D3DTS_VIEW, D3DTS_PROJECTION };
	int which, column;

	for (which = 0; which < 3; which++)
	{
		const D3DMATRIX *matrix = &device.transforms[order[which]];
		float in[4];

		memcpy(in, position, sizeof(in));
		for (column = 0; column < 4; column++)
		{
			position[column] = in[0] * matrix->m[0][column] + in[1] * matrix->m[1][column] +
				in[2] * matrix->m[2][column] + in[3] * matrix->m[3][column];
		}
	}
}

/* ---------- program cache */

/* size is a multiple of 4 */
static unsigned long hash_words(const void *data, unsigned long size)
{
	const DWORD *words = data;
	unsigned long hash = 2166136261UL;

	for (size /= 4; size; size--)
		hash = (hash ^ *words++) * 16777619UL;
	return hash;
}

/* size is a multiple of 4. Every draw compares its pixel shader key and its
uniforms' inputs (about 300 bytes each) with recent ones: the loop has no
early exit, so that the compiler compares several words at once, where the
C library's memcmp (the Windows build's) compares them one at a time */
static BOOL words_equal(const void *a, const void *b, unsigned long size)
{
	const DWORD *words_a = a;
	const DWORD *words_b = b;
	DWORD difference = 0;
	unsigned long index;

	for (index = 0; index < size / 4; index++)
		difference |= words_a[index] ^ words_b[index];
	return difference == 0;
}

/* lit: the shader that hands the lighting's normal and position on
(vertex_shader_object lit_shader) */
static GLuint vertex_shader_get(struct vertex_shader_object *program, BOOL immediate, BOOL lit)
{
	int variant = immediate ? 1 : 0;
	GLuint *shader = lit ? &program->lit_shader[variant] : &program->shader[variant];
	unsigned char tried = (unsigned char)(1 << (variant + (lit ? 2 : 0)));

	if (!*shader && !(program->shaders_tried & tried))
	{
		char *source = nv2a_vertex_shader_to_glsl(program->instructions, program->instruction_count,
			immediate ? 0 : device.vertex_shader->packed_mask, lit ? &program->lighting : NULL);

		program->shaders_tried |= tried;
		*shader = xgpu_compile_shader(GL_VERTEX_SHADER, source, "vertex");
		shader_hash_set(*shader, source);
		if (debug_settings.dump_shaders)
		{
			char path[512];
			FILE *file;

			snprintf(path, sizeof(path), "%s/vs%03lu_%d%s.glsl", debug_settings.dump_shaders, program->id, variant,
				lit ? "_lit" : "");
			if ((file = fopen(path, "w")) != NULL)
			{
				fputs(source, file);
				fclose(file);
			}
		}
		free(source);
	}
	return *shader;
}

typedef char pixel_shader_key_size_assert[sizeof(struct nv2a_pixel_shader_key) % 4 == 0 ? 1 : -1];

static GLuint fragment_shader_get(const struct nv2a_pixel_shader_key *key)
{
	/* consecutive draws mostly use one of a few pixel shaders (an object's
	parts take turns) */
#define RECENT_FRAGMENT_COUNT 4
	static struct fragment_entry *recent[RECENT_FRAGMENT_COUNT];
	static unsigned long recent_next;
	unsigned long hash, index;
	struct fragment_entry **bucket;
	struct fragment_entry *entry;
	char *source;

	for (index = 0; index < RECENT_FRAGMENT_COUNT; index++)
	{
		if (recent[index] && words_equal(&recent[index]->key, key, sizeof(*key)))
			return recent[index]->shader;
	}
	hash = hash_words(key, sizeof(*key));
	bucket = &fragment_buckets[hash % FRAGMENT_BUCKETS];
	for (entry = *bucket; entry; entry = entry->next)
	{
		if (entry->hash == hash && words_equal(&entry->key, key, sizeof(*key)))
		{
			recent[recent_next++ % RECENT_FRAGMENT_COUNT] = entry;
			return entry->shader;
		}
	}
	entry = calloc(1, sizeof(*entry));
	entry->hash = hash;
	entry->key = *key;
	source = nv2a_pixel_shader_to_glsl(key);
	entry->shader = xgpu_compile_shader(GL_FRAGMENT_SHADER, source, "pixel");
	shader_hash_set(entry->shader, source);
	if (debug_settings.dump_shaders)
	{
		char path[512];
		FILE *file;

		snprintf(path, sizeof(path), "%s/ps_%08lx.glsl", debug_settings.dump_shaders, hash);
		if ((file = fopen(path, "w")) != NULL)
		{
			fputs(source, file);
			fclose(file);
		}
	}
	free(source);
	entry->next = *bucket;
	*bucket = entry;
	recent[recent_next++ % RECENT_FRAGMENT_COUNT] = entry;
	return entry->shader;
}

static struct program_entry *program_get(GLuint vertex_shader, GLuint fragment_shader)
{
	static struct program_entry *last;
	unsigned long hash = (vertex_shader * 2654435761UL) ^ fragment_shader;
	struct program_entry **bucket = &program_buckets[hash % PROGRAM_BUCKETS];
	struct program_entry *entry;
	int stage;

	if (last && last->vertex_shader == vertex_shader && last->fragment_shader == fragment_shader)
		return last;
	for (entry = *bucket; entry; entry = entry->next)
	{
		if (entry->vertex_shader == vertex_shader && entry->fragment_shader == fragment_shader)
		{
			if (!entry->program)
				return NULL;
			last = entry;
			return entry;
		}
	}
	entry = calloc(1, sizeof(*entry));
	entry->vertex_shader = vertex_shader;
	entry->fragment_shader = fragment_shader;
	memset(&entry->uniforms, 0xff, sizeof(entry->uniforms));
	entry->next = *bucket;
	*bucket = entry;
	if (!vertex_shader || !fragment_shader)
		return NULL;

#ifndef HALO_GLES
	entry->program = program_cache_load(shader_hash(vertex_shader), shader_hash(fragment_shader));
	if (!entry->program)
	{
		/* (linked as xgpu_link_program does, keeping its binary) */
		GLint status = 0;

		entry->program = glCreateProgram();
		glAttachShader(entry->program, vertex_shader);
		glAttachShader(entry->program, fragment_shader);
		if (program_cache.enabled)
			glProgramParameteri(entry->program, GL_PROGRAM_BINARY_RETRIEVABLE_HINT, GL_TRUE);
		glLinkProgram(entry->program);
		glGetProgramiv(entry->program, GL_LINK_STATUS, &status);
		if (!status)
		{
			char log[4096];

			glGetProgramInfoLog(entry->program, sizeof(log), NULL, log);
			platform_log("cannot link the shader program: %s", log);
			glDeleteProgram(entry->program);
			entry->program = 0;
			return NULL;
		}
		program_cache.linked++;
		program_cache_store(entry->program, shader_hash(vertex_shader), shader_hash(fragment_shader));
	}
#else
	entry->program = xgpu_link_program(vertex_shader, fragment_shader, "shader");
	if (!entry->program)
		return NULL;
#endif
	state_program(entry->program);
	entry->constants = glGetUniformLocation(entry->program, "c");
	entry->constant_count = XGPU_VERTEX_CONSTANT_COUNT;
	if (entry->constants >= 0)
	{
		unsigned long index;

		/* c[i] is usually at c's location plus i, and the compiler may
		drop registers past the last one the program reads */
		entry->constants_consecutive = TRUE;
		for (index = 1; index < XGPU_VERTEX_CONSTANT_COUNT; index++)
		{
			char name[16];
			GLint location;

			snprintf(name, sizeof(name), "c[%lu]", index);
			location = glGetUniformLocation(entry->program, name);
			if (location < 0)
			{
				entry->constant_count = index;
				break;
			}
			if (location != entry->constants + (GLint)index)
			{
				entry->constants_consecutive = FALSE;
				entry->constant_count = XGPU_VERTEX_CONSTANT_COUNT;
				break;
			}
		}
	}
	entry->viewport_scale = glGetUniformLocation(entry->program, "viewport_scale");
	entry->viewport_offset = glGetUniformLocation(entry->program, "viewport_offset");
	entry->point_size = glGetUniformLocation(entry->program, "point_size");
	entry->ps_c0 = glGetUniformLocation(entry->program, "ps_c0");
	entry->ps_c1 = glGetUniformLocation(entry->program, "ps_c1");
	entry->ps_final_c0 = glGetUniformLocation(entry->program, "ps_final_c0");
	entry->ps_final_c1 = glGetUniformLocation(entry->program, "ps_final_c1");
	entry->fog_color = glGetUniformLocation(entry->program, "fog_color");
	entry->fog_parameters = glGetUniformLocation(entry->program, "fog_parameters");
	entry->alpha_reference = glGetUniformLocation(entry->program, "alpha_reference");
	entry->bump_matrix = glGetUniformLocation(entry->program, "bump_matrix");
	entry->bump_luminance = glGetUniformLocation(entry->program, "bump_luminance");
	entry->texture_scale = glGetUniformLocation(entry->program, "texture_scale");
	entry->texture_lod_bias = glGetUniformLocation(entry->program, "texture_lod_bias");
	entry->screen_offset = glGetUniformLocation(entry->program, "screen_offset");
	entry->height_fade = glGetUniformLocation(entry->program, "height_fade");
	entry->depth_bias = glGetUniformLocation(entry->program, "depth_bias");
	entry->blend_constant = glGetUniformLocation(entry->program, "blend_constant");
	entry->model_lights = glGetUniformLocation(entry->program, "model_lights");
	for (stage = 0; stage < D3DTSS_MAXSTAGES; stage++)
	{
		char name[8];

		snprintf(name, sizeof(name), "tex%d", stage);
		glUniform1i(glGetUniformLocation(entry->program, name), stage);
	}
	last = entry;
	return entry;
}

/* ---------- per-draw state */

static unsigned long stage_texture_mode(int stage)
{
	return (D3D__RenderState[D3DRS_PSTEXTUREMODES] >> (5 * stage)) & 0x1f;
}

static GLenum address_mode(DWORD mode)
{
	switch (mode)
	{
	case D3DTADDRESS_MIRROR: return GL_MIRRORED_REPEAT;
	case D3DTADDRESS_CLAMP: return GL_CLAMP_TO_EDGE;
#ifdef HALO_GLES
	case D3DTADDRESS_BORDER: return xgpu_capabilities.border_clamp ? GL_CLAMP_TO_BORDER : GL_CLAMP_TO_EDGE;
#else
	case D3DTADDRESS_BORDER: return GL_CLAMP_TO_BORDER;
#endif
	case D3DTADDRESS_CLAMPTOEDGE: return GL_CLAMP_TO_EDGE;
	default: return GL_REPEAT;
	}
}

/* ---------- sampler objects

A sampler object for each sampler state the game uses, made once: a draw
binds the one its state needs, rather than changing a sampler's parameters,
which costs a GL call each, and on Zink a new Vulkan sampler. The game uses
a few dozen states. */

#define SAMPLER_STATE_WORDS 11
#define SAMPLER_CACHE_SIZE 512

static struct
{
	DWORD inputs[SAMPLER_STATE_WORDS];
	GLuint sampler;
} sampler_cache[SAMPLER_CACHE_SIZE];
static unsigned long sampler_cache_count;

/* a sampler's parameters from its state (configure_sampler's inputs) */
static void sampler_parameters(GLuint sampler, const DWORD *inputs)
{
	DWORD min_filter = inputs[0], mip_filter = inputs[1], mag_filter = inputs[2];
	DWORD lod_bias = inputs[6], maximum_mip_level = inputs[7], anisotropy = inputs[8];
	GLenum minification;
	float border[4];

	if (min_filter == D3DTEXF_POINT)
		minification = mip_filter == D3DTEXF_NONE ? GL_NEAREST :
			mip_filter == D3DTEXF_POINT ? GL_NEAREST_MIPMAP_NEAREST : GL_NEAREST_MIPMAP_LINEAR;
	else
		minification = mip_filter == D3DTEXF_NONE ? GL_LINEAR :
			mip_filter == D3DTEXF_POINT ? GL_LINEAR_MIPMAP_NEAREST : GL_LINEAR_MIPMAP_LINEAR;
	glSamplerParameteri(sampler, GL_TEXTURE_MIN_FILTER, (GLint)minification);
	glSamplerParameteri(sampler, GL_TEXTURE_MAG_FILTER, mag_filter == D3DTEXF_POINT ? GL_NEAREST : GL_LINEAR);
	glSamplerParameteri(sampler, GL_TEXTURE_WRAP_S, (GLint)address_mode(inputs[3]));
	glSamplerParameteri(sampler, GL_TEXTURE_WRAP_T, (GLint)address_mode(inputs[4]));
	glSamplerParameteri(sampler, GL_TEXTURE_WRAP_R, (GLint)address_mode(inputs[5]));
#ifdef HALO_GLES
	/* ES has no sampler LOD bias; the pixel shader applies it
	(texture_lod_bias) */
	(void)lod_bias;
	glSamplerParameterf(sampler, GL_TEXTURE_MIN_LOD, (float)maximum_mip_level);
	if (xgpu_capabilities.anisotropy)
		glSamplerParameterf(sampler, GL_TEXTURE_MAX_ANISOTROPY_EXT,
			(min_filter == D3DTEXF_ANISOTROPIC && anisotropy > 1) ? (float)anisotropy : 1.0f);
	if (xgpu_capabilities.border_clamp)
	{
		color_to_vec4(inputs[9], border);
		glSamplerParameterfv(sampler, GL_TEXTURE_BORDER_COLOR, border);
	}
#else
	glSamplerParameterf(sampler, GL_TEXTURE_LOD_BIAS, dword_to_float(lod_bias));
	glSamplerParameterf(sampler, GL_TEXTURE_MIN_LOD, (float)maximum_mip_level);
	glSamplerParameterf(sampler, GL_TEXTURE_MAX_ANISOTROPY,
		(min_filter == D3DTEXF_ANISOTROPIC && anisotropy > 1) ? (float)anisotropy : 1.0f);
	color_to_vec4(inputs[9], border);
	glSamplerParameterfv(sampler, GL_TEXTURE_BORDER_COLOR, border);
#endif
}

/* the sampler object of a sampler state, made the first time; with the
cache full (never seen), the stage's own sampler set to it */
static GLuint sampler_get(int stage, const DWORD *inputs)
{
	unsigned long hash = 2166136261UL, index, probe;
	GLuint sampler;

	for (index = 0; index < SAMPLER_STATE_WORDS; index++)
		hash = (hash ^ inputs[index]) * 16777619UL;
	for (probe = 0; probe < SAMPLER_CACHE_SIZE; probe++)
	{
		index = (hash + probe) % SAMPLER_CACHE_SIZE;
		if (!sampler_cache[index].sampler)
			break;
		if (!memcmp(sampler_cache[index].inputs, inputs, sizeof(sampler_cache[index].inputs)))
			return sampler_cache[index].sampler;
	}
	if (probe == SAMPLER_CACHE_SIZE || sampler_cache_count >= SAMPLER_CACHE_SIZE * 3 / 4)
	{
		sampler_parameters(device.samplers[stage], inputs);
		return device.samplers[stage];
	}
	glGenSamplers(1, &sampler);
	sampler_parameters(sampler, inputs);
	memcpy(sampler_cache[index].inputs, inputs, sizeof(sampler_cache[index].inputs));
	sampler_cache[index].sampler = sampler;
	sampler_cache_count++;
	return sampler;
}

/* display.anisotropic_filtering: the most texels along a pixel's footprint
that a texture with mip levels is sampled with where the game filters it
linearly, 0 or 1 for the game's own filtering. The Xbox game asks for none
(it never sets D3DTEXF_ANISOTROPIC or D3DTSS_MAXANISOTROPY), so the floors,
walls and terrain seen at a grazing angle blur toward their smaller mip
levels. At most the GPU's most (16 on the desktop's), and none on GLES
without GL_EXT_texture_filter_anisotropic. */
static long anisotropic_level = -1;
static unsigned long anisotropic_level_read_at;

static DWORD anisotropic_filtering(void)
{
	if (anisotropic_level < 0 || anisotropic_level_read_at != config_changes())
	{
		long level = config_integer("display.anisotropic_filtering");
		GLint maximum = 1;

		anisotropic_level_read_at = config_changes();
#ifdef HALO_GLES
		if (xgpu_capabilities.anisotropy)
			glGetIntegerv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &maximum);
#else
		glGetIntegerv(GL_MAX_TEXTURE_MAX_ANISOTROPY, &maximum);
#endif
		if (level > (long)maximum)
			level = (long)maximum;
		if (level < 1)
			level = 1;
		if (level != anisotropic_level)
			platform_log("anisotropic filtering: %ldx (the GPU's most %dx)", level, (int)maximum);
		anisotropic_level = level;
	}
	return (DWORD)anisotropic_level;
}

/* hires: a high-res HUD texture (hud_hires.h), drawn smaller than it is, so
filtered and from its mip levels whatever the game asks: the HUD's meters are
point sampled for one player, to keep the Xbox bitmaps' texels sharp.
rendered: a render target the game draws into */
static void configure_sampler(int stage, BOOL mipmapped, BOOL hires, BOOL rendered)
{
	/* the texture stage state each stage's sampler was last chosen by */
	static DWORD configured[D3DTSS_MAXSTAGES][SAMPLER_STATE_WORDS];
	static GLuint configured_sampler[D3DTSS_MAXSTAGES];
	DWORD *state = D3D__TextureState[stage];
	DWORD inputs[SAMPLER_STATE_WORDS];

	inputs[0] = hires ? D3DTEXF_LINEAR : state[D3DTSS_MINFILTER];
	inputs[1] = hires ? D3DTEXF_LINEAR : mipmapped ? state[D3DTSS_MIPFILTER] : D3DTEXF_NONE;
	inputs[2] = hires ? D3DTEXF_LINEAR : state[D3DTSS_MAGFILTER];
	inputs[3] = state[D3DTSS_ADDRESSU];
	inputs[4] = state[D3DTSS_ADDRESSV];
	inputs[5] = state[D3DTSS_ADDRESSW];
	inputs[6] = hires ? 0 : state[D3DTSS_MIPMAPLODBIAS];
	inputs[7] = hires ? 0 : state[D3DTSS_MAXMIPLEVEL];
	inputs[8] = state[D3DTSS_MAXANISOTROPY];
	inputs[9] = state[D3DTSS_BORDERCOLOR];
	inputs[10] = hires;
	/* (display.anisotropic_filtering, over the game's linear filtering of
	the maps' textures with mip levels; not of what the game renders, the
	water's ripples, whose finer levels seen far off are noise) */
	if (!hires && mipmapped && !rendered && inputs[1] != D3DTEXF_NONE &&
		(inputs[0] == D3DTEXF_LINEAR || inputs[0] == D3DTEXF_ANISOTROPIC) && anisotropic_filtering() > 1)
	{
		inputs[0] = D3DTEXF_ANISOTROPIC;
		if (inputs[8] < anisotropic_filtering())
			inputs[8] = anisotropic_filtering();
	}
	if (!configured_sampler[stage] || memcmp(configured[stage], inputs, sizeof(inputs)))
	{
		memcpy(configured[stage], inputs, sizeof(inputs));
		configured_sampler[stage] = sampler_get(stage, inputs);
	}
	state_sampler(stage, configured_sampler[stage]);
}


/* ---------- render targets sampled with their mip chain

The game renders some textures one mip level at a time, each level being a
surface of its own (the water's ripple map). Sampling such a texture needs
every level in one GL texture, so the levels' render targets are copied into
a mipmapped composite. Each draw of the water binds it, some maps (a30) more
than once a frame, so the copy (and the mipmaps of the levels the game did not
render) is redone only once a level's target has been drawn into since the
last one. */

#define MIP_COMPOSITE_LEVELS 16

struct mip_composite
{
	struct mip_composite *next;
	unsigned long data, width, height, levels;
	GLuint texture;
	/* the levels last copied, and each one's target's texture and written
	serial then */
	unsigned long rendered_levels;
	GLuint level_sources[MIP_COMPOSITE_LEVELS];
	unsigned long level_written[MIP_COMPOSITE_LEVELS];
};

static struct mip_composite *mip_composites;

/* an entry render_target_get no longer finds: the screen's targets at
another scale or screen width than now, the shadow maps at another
display.shadow_resolution */
static BOOL render_target_stale(const struct render_target_entry *entry)
{
	switch (entry->kind)
	{
	case _render_target_screen:
		return entry->target.scale[0] != screen_scale[0] || entry->target.scale[1] != screen_scale[1] ||
			(entry->target.height == SCREEN_HEIGHT && entry->target.width != (unsigned long)halo_screen_width());
	case _render_target_shadow_map:
		return entry->target.scale[0] != (float)halo_shadow_map_scale();
	default:
		return FALSE;
	}
}

/* frees the framebuffers made of a texture, or with renderbuffers, of a
multisampled renderbuffer, which is going */
static void framebuffers_forget(GLuint attachment, BOOL renderbuffers)
{
	struct framebuffer_entry *entry, **link = &framebuffers;

	while ((entry = *link) != NULL)
	{
		if (entry->renderbuffers == renderbuffers && (entry->color == attachment || entry->depth == attachment))
		{
			*link = entry->next;
			glDeleteFramebuffers(1, &entry->framebuffer);
			free(entry);
		}
		else
		{
			link = &entry->next;
		}
	}
}

/* frees an entry taken out of render_targets: its GL objects, the
framebuffers made of them, and what remembered them */
static void render_target_delete(struct render_target_entry *entry)
{
	struct render_target_entry **link;
	struct mip_composite *composite;

	for (link = render_target_bucket(entry->target.data); *link != entry; link = &(*link)->next_in_bucket)
		;
	*link = entry->next_in_bucket;
	framebuffers_forget(entry->target.texture, FALSE);
	if (entry->target.multisample)
	{
		framebuffers_forget(entry->target.multisample, TRUE);
		glDeleteRenderbuffers(1, &entry->target.multisample);
	}
	/* (a composite that copied from the texture copies again, rather than
	take a new texture with the same name for it) */
	for (composite = mip_composites; composite; composite = composite->next)
	{
		unsigned long level;

		for (level = 0; level < composite->rendered_levels; level++)
		{
			if (composite->level_sources[level] == entry->target.texture)
			{
				composite->rendered_levels = 0;
				break;
			}
		}
	}
	xgpu_gl_texture_deleted(entry->target.texture);
	glDeleteTextures(1, &entry->target.texture);
	free(entry);
}

/* (between frames, when the screen's or the shadow maps' scale has
changed: halo_screen_commit) frees the entries render_target_get no longer
finds, which a frame at the old scale left */
static void render_targets_evict(void)
{
	struct render_target_entry *entry, **link = &render_targets;
	BOOL any = FALSE;

	while ((entry = *link) != NULL)
	{
		if (render_target_stale(entry))
		{
			*link = entry->next;
			render_target_delete(entry);
			any = TRUE;
		}
		else
		{
			link = &entry->next;
		}
	}
	if (any)
	{
		memset(recent_render_targets, 0, sizeof(recent_render_targets));
		xgpu_gl_state_invalidate();
	}
}

#ifdef HALO_GLES
static GLuint framebuffer_get(GLuint color, GLuint depth);

/* glCopyImageSubData for ES 3.0/3.1 contexts without the extension */
static void copy_level_by_blit(GLuint source, GLuint destination, GLint level, GLsizei width, GLsizei height)
{
	static GLuint draw_framebuffer;

	if (!draw_framebuffer)
		glGenFramebuffers(1, &draw_framebuffer);
	glBindFramebuffer(GL_READ_FRAMEBUFFER, framebuffer_get(source, 0));
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, draw_framebuffer);
	glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, destination, level);
	glDisable(GL_SCISSOR_TEST);
	glBlitFramebuffer(0, 0, width, height, 0, 0, width, height, GL_COLOR_BUFFER_BIT, GL_NEAREST);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	/* the blit bypasses the cached state, so the next draw must re-apply it */
	xgpu_gl_state_invalidate();
}
#endif

static GLuint mip_composite_get(const struct xgpu_texture_description *description, unsigned long data)
{
	struct mip_composite *composite;
	struct xgpu_render_target *targets[MIP_COMPOSITE_LEVELS];
	unsigned long level, rendered_levels = 0;
	BOOL changed;

	for (composite = mip_composites; composite; composite = composite->next)
	{
		if (composite->data == data && composite->width == description->width &&
			composite->height == description->height && composite->levels == description->levels)
		{
			break;
		}
	}
	if (!composite)
	{
		composite = calloc(1, sizeof(*composite));
		composite->data = data;
		composite->width = description->width;
		composite->height = description->height;
		composite->levels = description->levels;
		glGenTextures(1, &composite->texture);
		glBindTexture(GL_TEXTURE_2D, composite->texture);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, 0);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, (GLint)description->levels - 1);
		for (level = 0; level < description->levels; level++)
		{
			GLsizei width = (GLsizei)(description->width >> level ? description->width >> level : 1);
			GLsizei height = (GLsizei)(description->height >> level ? description->height >> level : 1);

			glTexImage2D(GL_TEXTURE_2D, (GLint)level, GL_RGBA8, width, height, 0, GL_BGRA, GL_UNSIGNED_BYTE, NULL);
		}
		composite->rendered_levels = ~0UL;
		composite->next = mip_composites;
		mip_composites = composite;
	}
	for (level = 0; level < description->levels && level < MIP_COMPOSITE_LEVELS; level++)
	{
		unsigned long width = description->width >> level ? description->width >> level : 1;
		unsigned long height = description->height >> level ? description->height >> level : 1;
		struct xgpu_render_target *target =
			xgpu_render_target_find(data + xgpu_texture_level_offset(description, level));

		if (!target || target->width != width || target->height != height ||
			target->gl_width != width || target->gl_height != height)
			break;
		targets[level] = target;
		rendered_levels++;
	}
	changed = rendered_levels != composite->rendered_levels;
	for (level = 0; level < rendered_levels && !changed; level++)
	{
		changed = targets[level]->texture != composite->level_sources[level] ||
			targets[level]->written != composite->level_written[level];
	}
	if (!changed)
		return composite->texture;
	composite->rendered_levels = rendered_levels;
	for (level = 0; level < rendered_levels; level++)
	{
		struct xgpu_render_target *target = targets[level];
		GLsizei width = (GLsizei)target->width, height = (GLsizei)target->height;

		composite->level_sources[level] = target->texture;
		composite->level_written[level] = target->written;
		render_target_resolve(target);
#ifdef HALO_GLES
		if (!xgpu_capabilities.copy_image)
		{
			copy_level_by_blit(target->texture, composite->texture, (GLint)level, width, height);
		}
		else
#endif
		glCopyImageSubData(target->texture, GL_TEXTURE_2D, 0, 0, 0, 0,
			composite->texture, GL_TEXTURE_2D, (GLint)level, 0, 0, 0, width, height, 1);
	}
	glBindTexture(GL_TEXTURE_2D, composite->texture);
	/* levels the game did not render come from the ones it did */
	if (rendered_levels < description->levels)
	{
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, rendered_levels ? (GLint)rendered_levels - 1 : 0);
		glGenerateMipmap(GL_TEXTURE_2D);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, 0);
	}
	xgpu_gl_state_invalidate();
	return composite->texture;
}

#ifdef HALO_WEB
/* WebGL refuses a draw that samples a texture of the framebuffer it draws
into (a feedback loop), which desktop GL and ES drivers draw, sampling the
pixels as they were: a copy of the target is sampled instead, made again
only once the target has been drawn into since */
static GLuint feedback_copy(struct xgpu_render_target *target)
{
	static GLuint copy;
	static GLsizei copy_width, copy_height;
	static unsigned long copied_texture, copied_written;

	if (target->depth)
		return target->texture;
	if (!copy || copy_width != (GLsizei)target->gl_width || copy_height != (GLsizei)target->gl_height)
	{
		if (!copy)
			glGenTextures(1, &copy);
		copy_width = (GLsizei)target->gl_width;
		copy_height = (GLsizei)target->gl_height;
		glBindTexture(GL_TEXTURE_2D, copy);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, copy_width, copy_height, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 0);
		xgpu_gl_state_invalidate();
		copied_texture = 0;
	}
	if (copied_texture != target->texture || copied_written != target->written)
	{
		copy_level_by_blit(target->texture, copy, 0, copy_width, copy_height);
		copied_texture = target->texture;
		copied_written = target->written;
	}
	return copy;
}
#endif

static void bind_textures(struct nv2a_pixel_shader_key *key, float texture_scale[4][4])
{
	/* Bind only after resolving every stage, since texture uploads can
	overwrite the active unit's binding. */
	GLenum gl_targets[D3DTSS_MAXSTAGES];
	GLuint gl_textures[D3DTSS_MAXSTAGES];
	int stage;

	for (stage = 0; stage < D3DTSS_MAXSTAGES; stage++)
	{
		D3DBaseTexture *texture = device.textures[stage];
		unsigned long mode = stage_texture_mode(stage);

		texture_scale[stage][0] = texture_scale[stage][1] = 1.0f;
		texture_scale[stage][2] = texture_scale[stage][3] = 1.0f;
		if (!texture || !texture->Data || mode == 0 || mode == 0x04 || mode == 0x05 || mode == 0x11)
		{
			gl_targets[stage] = GL_TEXTURE_2D;
			gl_textures[stage] = 0;
			key->sampler_type[stage] = mode == 0x11 ? _xgpu_sampler_2d : _xgpu_sampler_none;
			continue;
		}
		{
			struct xgpu_render_target *target = xgpu_render_target_find(texture->Data);
			struct xgpu_texture_description description;
			GLenum gl_target;
			GLuint gl_texture;

			if (target)
			{
				/* (multisampled: its pixels drawn since, resolved) */
				render_target_resolve(target);
				xgpu_texture_describe(texture->Format, texture->Size, &description);
				gl_texture = target->texture;
				gl_target = GL_TEXTURE_2D;
				if (description.linear)
				{
					texture_scale[stage][0] = 1.0f / (float)target->width;
					texture_scale[stage][1] = 1.0f / (float)target->height;
				}
				if (!description.linear && !description.cube_map && description.levels > 1 &&
					target->width == description.width && target->height == description.height)
					gl_texture = mip_composite_get(&description, texture->Data);
				else
					description.levels = 1;
#ifdef HALO_WEB
				{
					struct render_target_entry *drawn = render_target_get(device.render_target);

					if (drawn && gl_texture == drawn->target.texture)
						gl_texture = feedback_copy(target);
				}
#endif
			}
			else
			{
				const D3DCOLOR *palette = device.palettes[stage] && device.palettes[stage]->Data ?
					(const D3DCOLOR *)PLATFORM_PHYSICAL_TO_VIRTUAL(device.palettes[stage]->Data) : NULL;

				gl_texture = xgpu_texture_get((const DWORD *)texture, palette, &gl_target, &description);
				if (description.linear)
				{
					texture_scale[stage][0] = 1.0f / (float)description.width;
					texture_scale[stage][1] = 1.0f / (float)description.height;
				}
			}
			gl_targets[stage] = gl_target;
			gl_textures[stage] = gl_texture;
			configure_sampler(stage, description.levels > 1, description.hires, target != NULL);
			if (stage == 0)
			{
				key->coverage_alpha = description.hires_coverage != FALSE;
				key->point_threshold = description.hires_point_threshold != FALSE;
				key->ce_meter_channels = description.ce_meter_channels != FALSE;
			}
			key->sampler_type[stage] = gl_target == GL_TEXTURE_CUBE_MAP ? _xgpu_sampler_cube :
				gl_target == GL_TEXTURE_3D ? _xgpu_sampler_3d : _xgpu_sampler_2d;
		}
	}
#ifdef HALO_GLES
	/* (a stage the pixel shader does not sample keeps what it had: unbinding
	it, to bind it again at the next draw that has it, is two calls) */
	for (stage = 0; stage < D3DTSS_MAXSTAGES; stage++)
	{
		if (gl_textures[stage] || key->sampler_type[stage] != _xgpu_sampler_none)
			state_texture(stage, gl_targets[stage], gl_textures[stage]);
	}
#else
	{
		/* the units whose texture changes, bound in one call (GL 4.4's
		multi-bind) rather than selecting and binding each unit; binding no
		texture unbinds all of the unit's targets */
		int first = -1, last = -1;

		for (stage = 0; stage < D3DTSS_MAXSTAGES; stage++)
		{
			if (gl_state.textures[stage][texture_slot(gl_targets[stage])] != gl_textures[stage])
			{
				if (first < 0)
					first = stage;
				last = stage;
			}
		}
		if (first >= 0)
		{
			glBindTextures((GLuint)first, (GLsizei)(last - first + 1), &gl_textures[first]);
			for (stage = first; stage <= last; stage++)
			{
				if (gl_textures[stage])
					gl_state.textures[stage][texture_slot(gl_targets[stage])] = gl_textures[stage];
				else
					memset(gl_state.textures[stage], 0, sizeof(gl_state.textures[stage]));
			}
		}
	}
#endif
}

static GLenum stencil_operation(DWORD operation)
{
	/* Xbox stencil operations are the GL enumerants, plus 0 for ZERO */
	return operation ? (GLenum)operation : GL_ZERO;
}

static GLenum blend_equation(DWORD operation)
{
	switch (operation)
	{
	case D3DBLENDOP_SUBTRACT: return GL_FUNC_SUBTRACT;
	case D3DBLENDOP_REVSUBTRACT:
	case D3DBLENDOP_REVSUBTRACTSIGNED: return GL_FUNC_REVERSE_SUBTRACT;
	case D3DBLENDOP_MIN: return GL_MIN;
	case D3DBLENDOP_MAX: return GL_MAX;
	default: return GL_FUNC_ADD;
	}
}

static void apply_raster_state(BOOL has_depth)
{
	DWORD *rs = D3D__RenderState;
	DWORD write = rs[D3DRS_COLORWRITEENABLE];
	GLint viewport[4];
	GLint scissor[4];
	float depth_range[2];
	unsigned char color_mask;
	BOOL depth_test = has_depth && rs[D3DRS_ZENABLE];

	viewport[0] = target_pixel((float)device.viewport.X, 0);
	viewport[1] = target_pixel((float)device.viewport.Y, 1);
	viewport[2] = target_pixel((float)(device.viewport.X + device.viewport.Width), 0) - viewport[0];
	viewport[3] = target_pixel((float)(device.viewport.Y + device.viewport.Height), 1) - viewport[1];
	if (memcmp(gl_state.viewport, viewport, sizeof(viewport)))
	{
		memcpy(gl_state.viewport, viewport, sizeof(viewport));
		glViewport(viewport[0], viewport[1], viewport[2], viewport[3]);
	}
	/* the game never issues a scissor rectangle, and the NV2A scissor register
	defaults to the viewport, so fragment clipping follows the viewport: this is
	what keeps a split-screen window's geometry from bleeding across the divider */
	/* (glScissor takes the corner and the size, as glViewport does) */
	memcpy(scissor, viewport, sizeof(scissor));
	if (memcmp(gl_state.scissor, scissor, sizeof(scissor)))
	{
		memcpy(gl_state.scissor, scissor, sizeof(scissor));
		glScissor(scissor[0], scissor[1], scissor[2], scissor[3]);
	}
	state_enable(&gl_state.scissor_test, GL_SCISSOR_TEST, scissor[2] > 0 && scissor[3] > 0);
	/* (display.reversed_depth: Direct3D's MinZ to MaxZ as 1 - MinZ to
	1 - MaxZ, which glDepthRange takes nearest first) */
	depth_range[0] = xgpu_reversed_depth ? 1.0f - device.viewport.MaxZ : device.viewport.MinZ;
	depth_range[1] = xgpu_reversed_depth ? 1.0f - device.viewport.MinZ : device.viewport.MaxZ;
	if (memcmp(gl_state.depth_range, depth_range, sizeof(depth_range)))
	{
		memcpy(gl_state.depth_range, depth_range, sizeof(depth_range));
		glDepthRange(depth_range[0], depth_range[1]);
	}

	state_enable(&gl_state.depth_test, GL_DEPTH_TEST, depth_test);
	if (depth_test)
	{
		GLenum function = rs[D3DRS_ZFUNC] ? (GLenum)rs[D3DRS_ZFUNC] : GL_NEVER;

		/* (display.reversed_depth: nearer is greater) */
		if (xgpu_reversed_depth)
		{
			function = function == GL_LESS ? GL_GREATER : function == GL_LEQUAL ? GL_GEQUAL :
				function == GL_GREATER ? GL_LESS : function == GL_GEQUAL ? GL_LEQUAL : function;
		}
		if (gl_state.depth_function != function)
		{
			gl_state.depth_function = function;
			glDepthFunc(function);
		}
	}
	{
		unsigned char mask = depth_test && rs[D3DRS_ZWRITEENABLE] ? 1 : 0;

		if (gl_state.depth_mask != mask)
		{
			gl_state.depth_mask = mask;
			glDepthMask(mask ? GL_TRUE : GL_FALSE);
		}
	}

	state_enable(&gl_state.stencil_test, GL_STENCIL_TEST, has_depth && rs[D3DRS_STENCILENABLE]);
	if (has_depth && rs[D3DRS_STENCILENABLE])
	{
		GLenum function = rs[D3DRS_STENCILFUNC] ? (GLenum)rs[D3DRS_STENCILFUNC] : GL_NEVER;
		GLenum operations[3];

		if (gl_state.stencil_function != function || gl_state.stencil_reference != (GLint)rs[D3DRS_STENCILREF] ||
			gl_state.stencil_value_mask != rs[D3DRS_STENCILMASK])
		{
			gl_state.stencil_function = function;
			gl_state.stencil_reference = (GLint)rs[D3DRS_STENCILREF];
			gl_state.stencil_value_mask = rs[D3DRS_STENCILMASK];
			glStencilFunc(function, (GLint)rs[D3DRS_STENCILREF], rs[D3DRS_STENCILMASK]);
		}
		operations[0] = stencil_operation(rs[D3DRS_STENCILFAIL]);
		operations[1] = stencil_operation(rs[D3DRS_STENCILZFAIL]);
		operations[2] = stencil_operation(rs[D3DRS_STENCILPASS]);
		if (memcmp(gl_state.stencil_operations, operations, sizeof(operations)))
		{
			memcpy(gl_state.stencil_operations, operations, sizeof(operations));
			glStencilOp(operations[0], operations[1], operations[2]);
		}
		if (gl_state.stencil_write_mask != rs[D3DRS_STENCILWRITEMASK])
		{
			gl_state.stencil_write_mask = rs[D3DRS_STENCILWRITEMASK];
			glStencilMask(rs[D3DRS_STENCILWRITEMASK]);
		}
	}

	state_enable(&gl_state.blend, GL_BLEND, rs[D3DRS_ALPHABLENDENABLE] != 0);
	if (rs[D3DRS_ALPHABLENDENABLE])
	{
		GLenum equation = blend_equation(rs[D3DRS_BLENDOP]);
		float blend_color[4];

		/* (a constant factor the pixel shader applies: source_blend_in_shader) */
		GLenum source = source_blend_in_shader() ? GL_ONE : (GLenum)rs[D3DRS_SRCBLEND];

		if (gl_state.blend_source != source ||
			gl_state.blend_destination != (GLenum)rs[D3DRS_DESTBLEND])
		{
			gl_state.blend_source = source;
			gl_state.blend_destination = (GLenum)rs[D3DRS_DESTBLEND];
			glBlendFunc(gl_state.blend_source, gl_state.blend_destination);
		}
		if (gl_state.blend_equation != equation)
		{
			gl_state.blend_equation = equation;
			glBlendEquation(equation);
		}
		color_to_vec4(rs[D3DRS_BLENDCOLOR], blend_color);
		if (memcmp(gl_state.blend_color, blend_color, sizeof(blend_color)))
		{
			memcpy(gl_state.blend_color, blend_color, sizeof(blend_color));
			glBlendColor(blend_color[0], blend_color[1], blend_color[2], blend_color[3]);
		}
	}
	color_mask = (unsigned char)(((write & D3DCOLORWRITEENABLE_RED) ? 1 : 0) | ((write & D3DCOLORWRITEENABLE_GREEN) ? 2 : 0) |
		((write & D3DCOLORWRITEENABLE_BLUE) ? 4 : 0) | ((write & D3DCOLORWRITEENABLE_ALPHA) ? 8 : 0));
	if (gl_state.color_mask != color_mask)
	{
		gl_state.color_mask = color_mask;
		glColorMask((color_mask & 1) != 0, (color_mask & 2) != 0, (color_mask & 4) != 0, (color_mask & 8) != 0);
	}

	/* the cull mode names the winding to discard; FRONTFACE names the
	front winding */
	state_enable(&gl_state.cull_face, GL_CULL_FACE, rs[D3DRS_CULLMODE] != D3DCULL_NONE);
	if (rs[D3DRS_CULLMODE] != D3DCULL_NONE)
	{
#ifdef HALO_GLES
		/* the vertex shader flips y in clip space, which (unlike desktop
		GL's upper-left clip origin) also flips the winding */
		GLenum front_face = rs[D3DRS_FRONTFACE] == D3DFRONT_CCW ? GL_CW : GL_CCW;
#else
		GLenum front_face = rs[D3DRS_FRONTFACE] == D3DFRONT_CCW ? GL_CCW : GL_CW;
#endif
		GLenum cull_mode = rs[D3DRS_CULLMODE] == rs[D3DRS_FRONTFACE] ? GL_FRONT : GL_BACK;

		if (gl_state.front_face != front_face)
		{
			gl_state.front_face = front_face;
			glFrontFace(front_face);
		}
		if (gl_state.cull_mode != cull_mode)
		{
			gl_state.cull_mode = cull_mode;
			glCullFace(cull_mode);
		}
	}
#ifndef HALO_GLES
	/* ES draws filled polygons only (wireframe is a debug mode) */
	{
		GLenum polygon_mode = rs[D3DRS_FILLMODE] == D3DFILL_WIREFRAME ? GL_LINE :
			rs[D3DRS_FILLMODE] == D3DFILL_POINT ? GL_POINT : GL_FILL;

		if (gl_state.polygon_mode != polygon_mode)
		{
			gl_state.polygon_mode = polygon_mode;
			glPolygonMode(GL_FRONT_AND_BACK, polygon_mode);
		}
	}
#endif

	/* D3DRS_ZBIAS is expressed in these states (D3DDevice_SetRenderState_ZBias) */
	state_enable(&gl_state.offset_fill, GL_POLYGON_OFFSET_FILL, rs[D3DRS_SOLIDOFFSETENABLE] != 0);
#ifndef HALO_GLES
	state_enable(&gl_state.offset_line, GL_POLYGON_OFFSET_LINE, rs[D3DRS_SOLIDOFFSETENABLE] != 0);
#endif
	if (rs[D3DRS_SOLIDOFFSETENABLE])
	{
		float offset[2];

		offset[0] = dword_to_float(rs[D3DRS_POLYGONOFFSETZSLOPESCALE]);
		offset[1] = dword_to_float(rs[D3DRS_POLYGONOFFSETZOFFSET]);
		/* The slope is the depth's change across a pixel, and a target drawn
		at several pixels to the game's one (render_target_get) has that many
		times less depth across each: the Xbox's slope term, taken across its
		own pixels, is that many times the scale. On a surface seen at an
		angle it is most of the bias (the Pelican's lights on its curved hull
		sank in at 1080 lines with it a 2.25th as large). */
		offset[0] *= target_scale[0] > target_scale[1] ? target_scale[0] : target_scale[1];
		/* (display.reversed_depth: toward the camera is up, and the units,
		which a floating-point depth buffer measures by the depth itself,
		are the vertex program's, in the Xbox's measure: depth_bias) */
		if (xgpu_reversed_depth)
		{
			offset[0] = -offset[0];
			offset[1] = 0.0f;
		}
		if (memcmp(gl_state.polygon_offset, offset, sizeof(offset)))
		{
			memcpy(gl_state.polygon_offset, offset, sizeof(offset));
			glPolygonOffset(offset[0], offset[1]);
		}
	}
}

#ifdef HALO_GLES
/* ES has no debug callback in 3.0; debug.gl_debug polls glGetError around
each draw instead, reporting each distinct error a few times */
static void gl_check_errors(const char *where)
{
	static int enabled = -1;
	static unsigned long reports;
	GLenum error;

	if (enabled < 0)
		enabled = config_boolean("debug.gl_debug");
	if (!enabled)
		return;
	while ((error = glGetError()) != GL_NO_ERROR)
	{
		if (reports++ < 200)
			platform_log("GL error %04x at %s (frame %lu)", (unsigned)error, where, device.frame);
	}
}
#else
#define gl_check_errors(where) ((void)0)
#endif

/* the uniforms of the latest draws, converted from these inputs; the serial
counts the conversions */
#define DRAW_UNIFORM_INPUT_COUNT (4 + 4 + 16 + 1 + 16 + 2 + 4 + 1 + 1 + 7 * D3DTSS_MAXSTAGES + 4 + 2)

/* the height fade of the draws that follow (d3d8_gl_set_height_fade): off
while its scale is zero */
static float height_fade_value[4];

/* The draws that follow fade out above a height on their model (the shield
flare rising up a body: port/linux/game/overshield_effect.c): each vertex's
colors are scaled by clamp((height - z) * inverse_softness, 0, 1), where z,
its height in the model's space, is (oT0.z - offset) / scale (the plasma
shader's noise coordinate, rasterizer_xbox_plasma_energy.c). A scale of zero
turns it off, as it is for every other draw. */
void d3d8_gl_set_height_fade(float inverse_softness, float height, float offset, float scale)
{
	height_fade_value[0] = inverse_softness;
	height_fade_value[1] = height;
	height_fade_value[2] = offset;
	height_fade_value[3] = scale;
}

static DWORD draw_uniform_inputs[DRAW_UNIFORM_INPUT_COUNT];
static struct draw_uniforms draw_uniforms;
static unsigned long draw_uniforms_serial;

/* sets a program's uniform unless it already holds value */
static void uniform_vec4(GLint location, float *shadow, const float *value, int count)
{
	if (location < 0 || !memcmp(shadow, value, (size_t)count * 4 * sizeof(float)))
		return;
	memcpy(shadow, value, (size_t)count * 4 * sizeof(float));
	glUniform4fv(location, count, value);
}

static void uniform_float(GLint location, float *shadow, float value)
{
	if (location < 0 || !memcmp(shadow, &value, sizeof(value)))
		return;
	*shadow = value;
	glUniform1f(location, value);
}

/* Intel's graphics with Mesa's driver can hang the GPU in a long run of
draws with no pipeline flush between them, which the game's effects make
(hundreds of small draws in a row): the command streamer stops at a draw,
and the reset that follows takes the desktop's other programs with it.
Intel's workaround for a hang of this kind on their DG2 graphics
(Wa_16014538804) is a flush at least every 3 draws, which Mesa does not
apply to the others. A memory barrier is one (and only that: nothing
here writes images). */
static void draw_flush(void)
{
#ifndef HALO_GLES
	if (device.flush_every && ++device.flush_draws >= device.flush_every)
	{
		device.flush_draws = 0;
		glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
	}
#endif
}

/* display.per_pixel_lighting: the model lighting programs' draws are lit for
each pixel (nv2a_psh.c model_lighting), from shaders of their own; read
again when a setting changes */
static BOOL per_pixel_lighting(void)
{
	static unsigned long read_at = (unsigned long)-1;
	static BOOL enabled;

	if (read_at != config_changes())
	{
		read_at = config_changes();
		enabled = config_boolean("display.per_pixel_lighting") != 0;
	}
	return enabled;
}

/* the vertex constant register of each of the per-pixel lighting's
(XGPU_MODEL_LIGHT_COUNT) */
static unsigned long model_light_register(int light)
{
	return (unsigned long)(XGPU_VERTEX_CONSTANT_BIAS + (light ? -80 + light : -82));
}

static struct program_entry *prepare_draw(BOOL immediate)
{
	/* (the fixed-function program compiled only with OpenGL there: not by
	the null renderer) */
	struct vertex_shader_object *program = !device.gl_ready ? NULL :
		device.fixed_function ? fixed_function_program() : current_program();
	struct nv2a_pixel_shader_key key;
	struct program_entry *entry;
	struct draw_uniforms uniforms;
	BOOL has_depth = FALSE;
	int stage;

	/* (the fixed-function pipeline's draws: immediate mode's only) */
	if (!device.gl_ready || !program || !(device.vertex_shader || device.fixed_function) || !program->instructions ||
		(device.fixed_function && !immediate))
	{
		stats.skipped_no_program++;
		return NULL;
	}
	{
		const char *skip = debug_settings.skip_vertex_shaders;

		while (skip && *skip)
		{
			if ((unsigned long)atol(skip) == program->id)
				return NULL;
			skip = strchr(skip, ',');
			if (skip)
				skip++;
		}
	}
	memset(&key, 0, sizeof(key));
	memcpy(key.combiner_state, D3D__RenderState, sizeof(key.combiner_state));
	/* constants are uniforms, not part of the program */
	memset(&key.combiner_state[D3DRS_PSCONSTANT0_0], 0, 16 * sizeof(DWORD));
	key.combiner_state[D3DRS_PSFINALCOMBINERCONSTANT0] = 0;
	key.combiner_state[D3DRS_PSFINALCOMBINERCONSTANT1] = 0;
	key.texture_modes = D3D__RenderState[D3DRS_PSTEXTUREMODES];
	/* the textures before the targets: a render target the draw samples
	has its multisampled pixels resolved by a blit (render_target_resolve),
	which binds framebuffers of its own, and the back buffer can be both
	sampled and drawn into */
	bind_textures(&key, uniforms.texture_scale);
	if (!bind_targets(&has_depth))
	{
		stats.skipped_no_target++;
		return NULL;
	}
	apply_raster_state(has_depth);
	for (stage = 0; stage < D3DTSS_MAXSTAGES; stage++)
	{
		key.alpha_kill[stage] = D3D__TextureState[stage][D3DTSS_ALPHAKILL] == D3DTALPHAKILL_ENABLE;
		key.color_sign[stage] = (unsigned char)((D3D__TextureState[stage][D3DTSS_COLORSIGN] >> 28) & 0xf);
	}
	/* (only with the meter's blend: hud_hires.h, nv2a_pixel_shader_key) */
	key.coverage_alpha = key.coverage_alpha && D3D__RenderState[D3DRS_ALPHABLENDENABLE] &&
		D3D__RenderState[D3DRS_SRCBLEND] == D3DBLEND_CONSTANTCOLOR &&
		D3D__RenderState[D3DRS_DESTBLEND] == D3DBLEND_SRCALPHA;
	key.point_threshold = key.point_threshold && key.coverage_alpha;
	key.alpha_test_function = D3D__RenderState[D3DRS_ALPHATESTENABLE] ? D3D__RenderState[D3DRS_ALPHAFUNC] : 0;
#ifndef HALO_GLES
	/* (gl_SampleMask: ES has it only from 3.2) */
	if (target_samples > 1 && key.alpha_test_function && !D3D__RenderState[D3DRS_ALPHABLENDENABLE])
		key.alpha_test_samples = (unsigned char)target_samples;
#endif
	key.fog_enable = D3D__RenderState[D3DRS_FOGENABLE] != 0;
	key.source_blend = source_blend_in_shader();
	key.fog_table_mode = (unsigned char)D3D__RenderState[D3DRS_FOGTABLEMODE];
#ifdef HALO_GLES
	key.count_samples = device.visibility_test_active && xgpu_capabilities.atomic_counters;
#endif

	entry = NULL;
	if (program->lighting.lights && !program->lighting_failed && per_pixel_lighting())
	{
		key.per_pixel_lighting = (unsigned char)program->lighting.lights;
		entry = program_get(vertex_shader_get(program, immediate, TRUE), fragment_shader_get(&key));
		if (!entry)
		{
			/* drawn as the vertex shader lights it instead (not at all,
			were it to fail too) */
			platform_log("GPU: vertex shader %lu cannot be lit for each pixel here (refer to the shader log above): "
				"lit for each vertex", program->id);
			program->lighting_failed = TRUE;
			key.per_pixel_lighting = 0;
		}
	}
	if (!entry)
		entry = program_get(vertex_shader_get(program, immediate, FALSE), fragment_shader_get(&key));
	if (!entry)
	{
		stats.skipped_link++;
		gl_check_errors("program");
		return NULL;
	}
	gl_check_errors("state");
	if (immediate)
		stats.immediate_draws++;
	else
		stats.draws++;
#ifdef HALO_GLES
	device.draw_serial++;
#endif
	draw_flush();
	state_program(entry->program);
	if (key.source_blend && entry->blend_constant >= 0)
	{
		float blend_constant[4];

		color_to_vec4(D3D__RenderState[D3DRS_BLENDCOLOR], blend_constant);
		if (memcmp(entry->blend_constant_value, blend_constant, sizeof(blend_constant)))
		{
			memcpy(entry->blend_constant_value, blend_constant, sizeof(blend_constant));
			glUniform4fv(entry->blend_constant, 1, blend_constant);
		}
	}
#ifdef HALO_GLES
	if (key.count_samples)
		glBindBufferRange(GL_ATOMIC_COUNTER_BUFFER, 0, device.visibility_counters,
			(GLintptr)(device.counter_active * sizeof(GLuint)), sizeof(GLuint));
#endif

	if (entry->constants >= 0 && entry->constants_serial != constants_serial)
	{
		unsigned long first = entry->constant_count, last = 0, index;

		if (entry->constants_serial == constants_checkpoint_serial &&
			constants_checkpoint_last < entry->constant_count)
		{
			if (constants_checkpoint_first <= constants_checkpoint_last)
			{
				first = constants_checkpoint_first;
				last = constants_checkpoint_last;
			}
		}
		else if (constants_serial - entry->constants_serial <= XGPU_VERTEX_CONSTANT_COUNT)
		{
			unsigned long long serial;

			for (serial = entry->constants_serial + 1; serial <= constants_serial; serial++)
			{
				index = constant_log[serial % CONSTANT_LOG_SIZE];
				if (index >= entry->constant_count)
					continue;
				if (first > index)
					first = index;
				if (last < index)
					last = index;
			}
		}
		else
		{
			for (index = 0; index < entry->constant_count; index++)
			{
				if (constant_serials[index] > entry->constants_serial)
				{
					if (first > index)
						first = index;
					last = index;
				}
			}
		}
		if (first < entry->constant_count)
		{
			if (entry->constants_consecutive)
				glUniform4fv(entry->constants + (GLint)first, (GLsizei)(last - first + 1), device.constants[first]);
			else
				glUniform4fv(entry->constants, XGPU_VERTEX_CONSTANT_COUNT, &device.constants[0][0]);
		}
		entry->constants_serial = constants_serial;
		constants_checkpoint_serial = constants_serial;
		constants_checkpoint_first = XGPU_VERTEX_CONSTANT_COUNT;
		constants_checkpoint_last = 0;
	}
	/* the lights of a draw lit for each pixel, from the same registers, when
	any of them changed since the program last had them */
	if (entry->model_lights >= 0 && entry->model_lights_serial != constants_serial)
	{
		int light;

		for (light = 0; light < XGPU_MODEL_LIGHT_COUNT; light++)
		{
			if (constant_serials[model_light_register(light)] > entry->model_lights_serial)
				break;
		}
		if (light < XGPU_MODEL_LIGHT_COUNT)
		{
			float lights[XGPU_MODEL_LIGHT_COUNT][4];

			for (light = 0; light < XGPU_MODEL_LIGHT_COUNT; light++)
				memcpy(lights[light], device.constants[model_light_register(light)], sizeof(lights[0]));
			glUniform4fv(entry->model_lights, XGPU_MODEL_LIGHT_COUNT, lights[0]);
		}
		entry->model_lights_serial = constants_serial;
	}

	/* the state the other uniforms come from: most draws share it with the
	draw before them, and so share its uniforms */
	{
		DWORD inputs[DRAW_UNIFORM_INPUT_COUNT];
		unsigned long count = 0;

		memcpy(&inputs[count], device.viewport_scale, sizeof(device.viewport_scale));
		count += 4;
		memcpy(&inputs[count], device.viewport_offset, sizeof(device.viewport_offset));
		count += 4;
		memcpy(&inputs[count], uniforms.texture_scale, sizeof(uniforms.texture_scale));
		count += 16;
		inputs[count++] = D3D__RenderState[D3DRS_POINTSIZE];
		for (stage = 0; stage < 8; stage++)
		{
			inputs[count++] = D3D__RenderState[D3DRS_PSCONSTANT0_0 + stage];
			inputs[count++] = D3D__RenderState[D3DRS_PSCONSTANT1_0 + stage];
		}
		inputs[count++] = D3D__RenderState[D3DRS_PSFINALCOMBINERCONSTANT0];
		inputs[count++] = D3D__RenderState[D3DRS_PSFINALCOMBINERCONSTANT1];
		inputs[count++] = D3D__RenderState[D3DRS_FOGCOLOR];
		inputs[count++] = D3D__RenderState[D3DRS_FOGSTART];
		inputs[count++] = D3D__RenderState[D3DRS_FOGEND];
		inputs[count++] = D3D__RenderState[D3DRS_FOGDENSITY];
		inputs[count++] = D3D__RenderState[D3DRS_ALPHAREF];
		inputs[count++] = (DWORD)UI_OFFSET;
		for (stage = 0; stage < D3DTSS_MAXSTAGES; stage++)
		{
			DWORD *state = D3D__TextureState[stage];

			inputs[count++] = state[D3DTSS_BUMPENVMAT00];
			inputs[count++] = state[D3DTSS_BUMPENVMAT01];
			inputs[count++] = state[D3DTSS_BUMPENVMAT10];
			inputs[count++] = state[D3DTSS_BUMPENVMAT11];
			inputs[count++] = state[D3DTSS_BUMPENVLSCALE];
			inputs[count++] = state[D3DTSS_BUMPENVLOFFSET];
			inputs[count++] = state[D3DTSS_MIPMAPLODBIAS];
		}
		memcpy(&inputs[count], height_fade_value, sizeof(height_fade_value));
		count += 4;
		inputs[count++] = D3D__RenderState[D3DRS_SOLIDOFFSETENABLE];
		inputs[count++] = D3D__RenderState[D3DRS_POLYGONOFFSETZOFFSET];
		if (!draw_uniforms_serial || !words_equal(inputs, draw_uniform_inputs, sizeof(inputs)))
		{
			struct draw_uniforms *converted = &draw_uniforms;

			memcpy(draw_uniform_inputs, inputs, sizeof(inputs));
			draw_uniforms_serial++;
			memcpy(converted->viewport_scale, device.viewport_scale, sizeof(converted->viewport_scale));
			memcpy(converted->viewport_offset, device.viewport_offset, sizeof(converted->viewport_offset));
			memcpy(converted->texture_scale, uniforms.texture_scale, sizeof(converted->texture_scale));
			converted->point_size = D3D__RenderState[D3DRS_POINTSIZE] ?
				dword_to_float(D3D__RenderState[D3DRS_POINTSIZE]) : 1.0f;
			for (stage = 0; stage < 8; stage++)
			{
				color_to_vec4(D3D__RenderState[D3DRS_PSCONSTANT0_0 + stage], converted->ps_c0[stage]);
				color_to_vec4(D3D__RenderState[D3DRS_PSCONSTANT1_0 + stage], converted->ps_c1[stage]);
			}
			color_to_vec4(D3D__RenderState[D3DRS_PSFINALCOMBINERCONSTANT0], converted->ps_final_c0);
			color_to_vec4(D3D__RenderState[D3DRS_PSFINALCOMBINERCONSTANT1], converted->ps_final_c1);
			color_to_vec4(D3D__RenderState[D3DRS_FOGCOLOR], converted->fog_color);
			converted->fog_parameters[0] = dword_to_float(D3D__RenderState[D3DRS_FOGSTART]);
			converted->fog_parameters[1] = dword_to_float(D3D__RenderState[D3DRS_FOGEND]);
			converted->fog_parameters[2] = dword_to_float(D3D__RenderState[D3DRS_FOGDENSITY]);
			converted->fog_parameters[3] = 0.0f;
			converted->alpha_reference = (float)(D3D__RenderState[D3DRS_ALPHAREF] & 0xff);
			for (stage = 0; stage < D3DTSS_MAXSTAGES; stage++)
			{
				DWORD *state = D3D__TextureState[stage];

				converted->bump_matrix[stage][0] = dword_to_float(state[D3DTSS_BUMPENVMAT00]);
				converted->bump_matrix[stage][1] = dword_to_float(state[D3DTSS_BUMPENVMAT01]);
				converted->bump_matrix[stage][2] = dword_to_float(state[D3DTSS_BUMPENVMAT10]);
				converted->bump_matrix[stage][3] = dword_to_float(state[D3DTSS_BUMPENVMAT11]);
				converted->bump_luminance[stage][0] = dword_to_float(state[D3DTSS_BUMPENVLSCALE]);
				converted->bump_luminance[stage][1] = dword_to_float(state[D3DTSS_BUMPENVLOFFSET]);
				converted->bump_luminance[stage][2] = converted->bump_luminance[stage][3] = 0.0f;
				converted->texture_lod_bias[stage] = dword_to_float(state[D3DTSS_MIPMAPLODBIAS]);
			}
			converted->screen_offset = (float)UI_OFFSET;
			memcpy(converted->height_fade, height_fade_value, sizeof(converted->height_fade));
			/* (display.reversed_depth: the z bias's units, the Xbox's depth
			units toward the camera, in the depth the vertex program writes:
			one is 1 / viewport_scale.z, the depth buffer's range over the
			viewport's) */
			converted->depth_bias = xgpu_reversed_depth && D3D__RenderState[D3DRS_SOLIDOFFSETENABLE] &&
				device.viewport_scale[2] != 0.0f ?
				-dword_to_float(D3D__RenderState[D3DRS_POLYGONOFFSETZOFFSET]) / device.viewport_scale[2] : 0.0f;
		}
	}
	/* and a program that has had them since needs none of them */
	if (entry->uniforms_serial == draw_uniforms_serial)
		return entry;
	entry->uniforms_serial = draw_uniforms_serial;
	uniform_vec4(entry->viewport_scale, entry->uniforms.viewport_scale, draw_uniforms.viewport_scale, 1);
	uniform_vec4(entry->viewport_offset, entry->uniforms.viewport_offset, draw_uniforms.viewport_offset, 1);
	uniform_float(entry->point_size, &entry->uniforms.point_size, draw_uniforms.point_size);
	uniform_vec4(entry->ps_c0, entry->uniforms.ps_c0[0], draw_uniforms.ps_c0[0], 8);
	uniform_vec4(entry->ps_c1, entry->uniforms.ps_c1[0], draw_uniforms.ps_c1[0], 8);
	uniform_vec4(entry->ps_final_c0, entry->uniforms.ps_final_c0, draw_uniforms.ps_final_c0, 1);
	uniform_vec4(entry->ps_final_c1, entry->uniforms.ps_final_c1, draw_uniforms.ps_final_c1, 1);
	uniform_vec4(entry->fog_color, entry->uniforms.fog_color, draw_uniforms.fog_color, 1);
	uniform_vec4(entry->fog_parameters, entry->uniforms.fog_parameters, draw_uniforms.fog_parameters, 1);
	uniform_float(entry->alpha_reference, &entry->uniforms.alpha_reference, draw_uniforms.alpha_reference);
	uniform_vec4(entry->bump_matrix, entry->uniforms.bump_matrix[0], draw_uniforms.bump_matrix[0], 4);
	uniform_vec4(entry->bump_luminance, entry->uniforms.bump_luminance[0], draw_uniforms.bump_luminance[0], 4);
	uniform_vec4(entry->texture_scale, entry->uniforms.texture_scale[0], draw_uniforms.texture_scale[0], 4);
	uniform_float(entry->screen_offset, &entry->uniforms.screen_offset, draw_uniforms.screen_offset);
	uniform_vec4(entry->texture_lod_bias, entry->uniforms.texture_lod_bias, draw_uniforms.texture_lod_bias, 1);
	uniform_vec4(entry->height_fade, entry->uniforms.height_fade, draw_uniforms.height_fade, 1);
	uniform_float(entry->depth_bias, &entry->uniforms.depth_bias, draw_uniforms.depth_bias);
	return entry;
}

/* ---------- tracing (debug.gpu_trace_frame) */

static BOOL trace_frame(void)
{
	static long frame = -2;

	if (frame == -2)
		frame = config_integer("debug.gpu_trace_frame");
	return frame >= 0 && device.frame == (unsigned long)frame;
}

static void trace_draw(const char *kind, D3DPRIMITIVETYPE type, unsigned long count, const float *first_vertex)
{
	struct vertex_shader_object *program = current_program();
	DWORD *rs = D3D__RenderState;

	if (!trace_frame())
		return;
	platform_log("%s type %d count %lu vs %lu (decl %lu) vp %lu,%lu %lux%lu z%.2f-%.2f zen %lu zw %lu zf %lx blend %lu %lx/%lx cull %lx cw %08lx tm %05lx cc %lx fin %08lx/%08lx at %lu/%lx",
		kind, type, count, program ? program->id : 0, device.vertex_shader ? device.vertex_shader->id : 0,
		device.viewport.X, device.viewport.Y, device.viewport.Width, device.viewport.Height,
		device.viewport.MinZ, device.viewport.MaxZ, rs[D3DRS_ZENABLE], rs[D3DRS_ZWRITEENABLE], rs[D3DRS_ZFUNC],
		rs[D3DRS_ALPHABLENDENABLE], rs[D3DRS_SRCBLEND], rs[D3DRS_DESTBLEND], rs[D3DRS_CULLMODE],
		rs[D3DRS_COLORWRITEENABLE], rs[D3DRS_PSTEXTUREMODES], rs[D3DRS_PSCOMBINERCOUNT],
		rs[D3DRS_PSFINALCOMBINERINPUTSABCD], rs[D3DRS_PSFINALCOMBINERINPUTSEFG],
		rs[D3DRS_ALPHATESTENABLE], rs[D3DRS_ALPHAFUNC]);
	{
		int stage;

		for (stage = 0; stage < D3DTSS_MAXSTAGES; stage++)
		{
			D3DBaseTexture *texture = device.textures[stage];
			struct xgpu_texture_description description;

			if (!texture || !((D3D__RenderState[D3DRS_PSTEXTUREMODES] >> (5 * stage)) & 0x1f))
				continue;
			xgpu_texture_describe(texture->Format, texture->Size, &description);
			platform_log("    t%d: data %08lx format %08lx size %08lx -> fmt %02lx %lux%lux%lu levels %lu linear %d cube %d rt %d min %lu mip %lu bias %g maxmip %lu",
				stage, texture->Data, texture->Format, texture->Size, description.format, description.width,
				description.height, description.depth, description.levels, description.linear, description.cube_map,
				xgpu_render_target_find(texture->Data) != NULL, D3D__TextureState[stage][D3DTSS_MINFILTER],
				D3D__TextureState[stage][D3DTSS_MIPFILTER], dword_to_float(D3D__TextureState[stage][D3DTSS_MIPMAPLODBIAS]),
				D3D__TextureState[stage][D3DTSS_MAXMIPLEVEL]);
		}
	}
	platform_log("    offset enable %lu slope %g offset %g zbias %ld stencil %lu func %lx ref %lx mask %lx write %lx ops %lx/%lx/%lx",
		rs[D3DRS_SOLIDOFFSETENABLE], dword_to_float(rs[D3DRS_POLYGONOFFSETZSLOPESCALE]),
		dword_to_float(rs[D3DRS_POLYGONOFFSETZOFFSET]), (long)rs[D3DRS_ZBIAS], rs[D3DRS_STENCILENABLE],
		rs[D3DRS_STENCILFUNC], rs[D3DRS_STENCILREF], rs[D3DRS_STENCILMASK], rs[D3DRS_STENCILWRITEMASK],
		rs[D3DRS_STENCILFAIL], rs[D3DRS_STENCILZFAIL], rs[D3DRS_STENCILPASS]);
	if (config_boolean("debug.gpu_trace_constants"))
	{
		int constant;

		for (constant = 0; constant < XGPU_VERTEX_CONSTANT_COUNT; constant++)
		{
			const float *value = device.constants[constant];

			if (value[0] || value[1] || value[2] || value[3])
				platform_log("    c[%d] = %g %g %g %g", constant, value[0], value[1], value[2], value[3]);
		}
	}
	if (device.vertex_shader)
	{
		unsigned long index;

		for (index = 0; index < device.vertex_shader->element_count; index++)
		{
			const struct vertex_element *element = &device.vertex_shader->elements[index];

			platform_log("    decl v%lu: stream %lu offset %lu type %02lx", (unsigned long)element->reg,
				(unsigned long)element->stream, (unsigned long)element->offset, (unsigned long)element->type);
		}
		for (index = 0; index < XGPU_VERTEX_ATTRIBUTE_COUNT; index++)
		{
			const float *value = device.attributes[index];

			if (value[0] || value[1] || value[2] || value[3] != 1.0f)
				platform_log("    current v%lu = %g %g %g %g", index, value[0], value[1], value[2], value[3]);
		}
	}
	if (first_vertex)
	{
		int reg;

		for (reg = 0; reg < XGPU_VERTEX_ATTRIBUTE_COUNT; reg++)
		{
			const float *v = first_vertex + reg * 4;

			if (v[0] || v[1] || v[2] || v[3] != 1.0f)
				platform_log("    v%d = %g %g %g %g", reg, v[0], v[1], v[2], v[3]);
		}
	}
}


/* Mesa's GL thread queues a glBufferSubData of up to 8 KB; a larger one
first waits for everything queued before it to have run. The same bytes in
pieces are queued (all but the largest, as a map loads, which would be
thousands; Android's GL has no such thread). */
#define BUFFER_UPLOAD_PIECE 4096
#define BUFFER_UPLOAD_PIECES_MAXIMUM 16

static void buffer_upload(GLenum target, unsigned long offset, unsigned long size, const void *data)
{
#ifndef HALO_ANDROID
	if (size <= BUFFER_UPLOAD_PIECE * BUFFER_UPLOAD_PIECES_MAXIMUM)
	{
		const unsigned char *bytes = data;

		while (size)
		{
			unsigned long piece = size < BUFFER_UPLOAD_PIECE ? size : BUFFER_UPLOAD_PIECE;

			glBufferSubData(target, (GLintptr)offset, (GLsizeiptr)piece, bytes);
			offset += piece;
			bytes += piece;
			size -= piece;
		}
		return;
	}
#endif
	glBufferSubData(target, (GLintptr)offset, (GLsizeiptr)size, data);
}

/* ---------- the contiguous window in GL buffers

Vertex and index buffers live in the Xbox's contiguous memory, where most
never change once loaded. The mirror keeps a copy of that memory in GL
buffers (one per segment, created when first needed) and uploads a page
only when it is first drawn from or after the game has written it: pages
are write-protected once uploaded, as cached textures are (memory_watch.c).
Pages the game rewrites frame after frame (dynamic vertices) would fault on
every write; after a few such rewrites a page counts as volatile for a
while, and draws that use it stream their data as before. */

#define MIRROR_SEGMENT_SIZE 0x400000UL
#define MIRROR_SEGMENT_COUNT (PLATFORM_CONTIGUOUS_SIZE / MIRROR_SEGMENT_SIZE)
#define MIRROR_PAGE_SIZE 0x1000UL
#define MIRROR_PAGE_COUNT (PLATFORM_CONTIGUOUS_SIZE / MIRROR_PAGE_SIZE)
/* rewrites no more than this many frames apart ... */
#define MIRROR_REWRITE_FRAMES 2
/* ... this many times in a row make a page volatile ... */
#define MIRROR_VOLATILE_REWRITES 4
/* ... for this many frames */
#define MIRROR_VOLATILE_FRAMES 600

enum
{
	_mirror_page_absent,
	_mirror_page_present,
	_mirror_page_volatile
};

static struct
{
	GLuint buffers[MIRROR_SEGMENT_COUNT];
	unsigned char state[MIRROR_PAGE_COUNT];
	unsigned char rewrites[MIRROR_PAGE_COUNT];
	/* the page's memory_watch generation when it was uploaded */
	unsigned long generation[MIRROR_PAGE_COUNT];
	unsigned long rewritten_frame[MIRROR_PAGE_COUNT];
#ifdef HALO_GLES
	/* device.frame at the segment's latest glBufferSubData */
	unsigned long subdata_frame[MIRROR_SEGMENT_COUNT];
#endif
} mirror;

/* uploads the pages of [first, last) that are absent or stale; FALSE if one
of them turns out to be volatile */
static BOOL mirror_refresh(unsigned long first, unsigned long last)
{
	unsigned long page, run;
	BOOL volatile_page = FALSE;
	unsigned char stale[256];
	unsigned long count = last - first;

	if (count > sizeof(stale))
	{
		/* a range this long is refreshed in pieces */
		for (page = first; page < last; page += sizeof(stale))
		{
			if (!mirror_refresh(page, page + sizeof(stale) < last ? page + sizeof(stale) : last))
				return FALSE;
		}
		return TRUE;
	}
	for (page = first; page < last; page++)
	{
		BOOL written = mirror.state[page] == _mirror_page_present &&
			memory_watch_generation(PLATFORM_CONTIGUOUS_BASE + page * MIRROR_PAGE_SIZE, MIRROR_PAGE_SIZE) >
			mirror.generation[page];

		stale[page - first] = mirror.state[page] != _mirror_page_present || written;
		if (written)
		{
			if (device.frame - mirror.rewritten_frame[page] <= MIRROR_REWRITE_FRAMES)
				mirror.rewrites[page]++;
			else
				mirror.rewrites[page] = 1;
			mirror.rewritten_frame[page] = device.frame;
			if (mirror.rewrites[page] >= MIRROR_VOLATILE_REWRITES)
			{
				mirror.state[page] = _mirror_page_volatile;
				volatile_page = TRUE;
			}
		}
	}
	if (volatile_page)
		return FALSE;
	for (page = first; page < last; page = run)
	{
		unsigned long segment = page * MIRROR_PAGE_SIZE / MIRROR_SEGMENT_SIZE;
		unsigned long address, size;
		/* no queued draw can read pages uploaded for the first time */
		BOOL unused = TRUE;

		if (!stale[page - first])
		{
			run = page + 1;
			continue;
		}
		for (run = page; run < last && stale[run - first]; run++)
		{
			if (mirror.state[run] != _mirror_page_present)
			{
				mirror.rewrites[run] = 0;
				mirror.rewritten_frame[run] = device.frame;
			}
			else
			{
				unused = FALSE;
			}
		}
		address = PLATFORM_CONTIGUOUS_BASE + page * MIRROR_PAGE_SIZE;
		size = (run - page) * MIRROR_PAGE_SIZE;
		/* protect first, so a write racing with the upload is noticed */
		memory_watch_protect(address, size);
		for (; page < run; page++)
		{
			mirror.generation[page] = memory_watch_generation(PLATFORM_CONTIGUOUS_BASE + page * MIRROR_PAGE_SIZE,
				MIRROR_PAGE_SIZE);
			mirror.state[page] = _mirror_page_present;
		}
		if (!mirror.buffers[segment])
		{
			glGenBuffers(1, &mirror.buffers[segment]);
			glBindBuffer(GL_COPY_WRITE_BUFFER, mirror.buffers[segment]);
			glBufferData(GL_COPY_WRITE_BUFFER, MIRROR_SEGMENT_SIZE, NULL, GL_DYNAMIC_DRAW);
#ifdef HALO_GLES
			/* (no glBufferSubData yet: as if long enough ago) */
			mirror.subdata_frame[segment] = device.frame - STREAM_BUFFER_RING;
#endif
		}
		glBindBuffer(GL_COPY_WRITE_BUFFER, mirror.buffers[segment]);
#ifdef HALO_GLES
		/* Mali copies the whole buffer for a glBufferSubData that queued
		draws might read (see STREAM_BUFFER_RING); unused pages can be
		written without waiting for them. Not while such a copy may be
		pending: it gives the buffer new storage at once but fills it on
		the GPU's timeline, over what was written into it meanwhile (a level
		loaded after another drew some of the earlier level's vertices).
		The GPU is done with a frame's work STREAM_BUFFER_RING frames on */
		if (unused && device.frame - mirror.subdata_frame[segment] >= STREAM_BUFFER_RING)
		{
			host_gl_buffer_write(GL_COPY_WRITE_BUFFER,
				(unsigned int)(address - PLATFORM_CONTIGUOUS_BASE - segment * MIRROR_SEGMENT_SIZE),
				(unsigned int)size, (const void *)address);
			continue;
		}
		mirror.subdata_frame[segment] = device.frame;
#else
		(void)unused;
#endif
		buffer_upload(GL_COPY_WRITE_BUFFER, address - PLATFORM_CONTIGUOUS_BASE - segment * MIRROR_SEGMENT_SIZE, size,
			(const void *)address);
	}
	return TRUE;
}

/* makes [address, address + size) current in the mirror, giving the buffer
that holds it, the range's offset in that buffer and the newest upload
generation of its pages (which changes whenever its contents do); FALSE if
the range is outside the window, spans two segments or is volatile */
static BOOL mirror_range(unsigned long address, unsigned long size, GLuint *buffer, unsigned long *offset,
	unsigned long *generation)
{
	unsigned long start = address - PLATFORM_CONTIGUOUS_BASE;
	unsigned long segment, first, last, page, oldest = ~0UL, newest = 0;
	BOOL present = TRUE;

#ifdef HALO_ANDROID
	/* Affected and unknown chips use static buffers or current CPU bytes. */
	if (android_geometry_fixes)
		return FALSE;
#endif
	if (!size || address < PLATFORM_CONTIGUOUS_BASE || start + size > PLATFORM_CONTIGUOUS_SIZE)
		return FALSE;
	segment = start / MIRROR_SEGMENT_SIZE;
	if ((start + size - 1) / MIRROR_SEGMENT_SIZE != segment)
		return FALSE;
	first = start / MIRROR_PAGE_SIZE;
	last = (start + size - 1) / MIRROR_PAGE_SIZE + 1;
	for (page = first; page < last; page++)
	{
		if (mirror.state[page] == _mirror_page_volatile)
		{
			if (device.frame - mirror.rewritten_frame[page] < MIRROR_VOLATILE_FRAMES)
				return FALSE;
			mirror.state[page] = _mirror_page_absent;
		}
		if (mirror.state[page] != _mirror_page_present)
		{
			present = FALSE;
		}
		else
		{
			if (mirror.generation[page] < oldest)
				oldest = mirror.generation[page];
			if (mirror.generation[page] > newest)
				newest = mirror.generation[page];
		}
	}
	if (!present || memory_watch_generation(address, size) > oldest)
	{
		if (!mirror_refresh(first, last))
			return FALSE;
		for (newest = 0, page = first; page < last; page++)
		{
			if (mirror.generation[page] > newest)
				newest = mirror.generation[page];
		}
	}
	*buffer = mirror.buffers[segment];
	*offset = start - segment * MIRROR_SEGMENT_SIZE;
	if (generation)
		*generation = newest;
	stats.mirrored_bytes += size;
	return TRUE;
}

/* the smallest and largest index of an index range the mirror holds: the
same ranges are drawn frame after frame */
#define INDEX_RANGE_SLOTS 4096

static struct
{
	unsigned long address;
	unsigned long count;
	unsigned long generation;
	WORD minimum;
	WORD maximum;
} index_ranges[INDEX_RANGE_SLOTS];

static void index_extent(const WORD *indices, unsigned long count, unsigned long generation, BOOL cached,
	unsigned long *minimum, unsigned long *maximum)
{
	unsigned long slot = (((unsigned long)indices >> 1) ^ (count * 2654435761UL)) % INDEX_RANGE_SLOTS;
	unsigned long index, low = 0xffff, high = 0;

	if (cached && index_ranges[slot].address == (unsigned long)indices && index_ranges[slot].count == count &&
		index_ranges[slot].generation == generation)
	{
		*minimum = index_ranges[slot].minimum;
		*maximum = index_ranges[slot].maximum;
		return;
	}
	for (index = 0; index < count; index++)
	{
		if (indices[index] < low)
			low = indices[index];
		if (indices[index] > high)
			high = indices[index];
	}
	if (cached)
	{
		index_ranges[slot].address = (unsigned long)indices;
		index_ranges[slot].count = count;
		index_ranges[slot].generation = generation;
		index_ranges[slot].minimum = (WORD)low;
		index_ranges[slot].maximum = (WORD)high;
	}
	*minimum = low;
	*maximum = high;
}

#ifdef HALO_ANDROID
/* ---------- static geometry

When Android's geometry fixes are enabled, mirror_range declines the
contiguous window, so draws stream their vertices and indices. Geometry the game writes once and
never again gets GL buffers of its own instead, filled the first time a draw
reads it and never written after, so nothing tracks writes and no queued
draw can read a buffer while it changes:
- the vertex and index buffers rasterizer_vertex_buffer_new and
  rasterizer_triangle_buffer_new make (Custom Edition and mod models, a
  Custom Edition map's BSP), one each (xgpu_static_buffer_add);
- an Xbox map's model geometry in its tags, and its BSP's in the BSP, from
  the first buffer to the end of the tags or the BSP (xgpu_static_buffers_add):
  the cache files hold the buffers there, written only as the map loads.
An entry goes when the game releases or unloads it; its GL buffer is
deleted at the next Present. */

struct static_buffer
{
	unsigned long start;
	unsigned long size;
	GLuint buffer;
	/* (GL had no storage for it: it streams, as before, and is not tried
	again) */
	BOOL failed;
	/* (a value of its own, for index_extent's cache of this range) */
	unsigned long serial;
};

/* sorted by start, one list of vertices (0) and one of indices (1); the
ranges of a list are separate memory */
static struct static_buffer_list
{
	struct static_buffer *entries;
	unsigned long count;
	unsigned long capacity;
} static_buffer_lists[2];

static struct
{
	unsigned long serial;
	GLuint *doomed;
	unsigned long doomed_count;
	unsigned long doomed_capacity;
} static_buffers;

/* the first entry of the list whose start is above address */
static unsigned long static_buffer_upper(const struct static_buffer_list *list, unsigned long address)
{
	unsigned long low = 0, high = list->count;

	while (low < high)
	{
		unsigned long middle = (low + high) / 2;

		if (list->entries[middle].start <= address)
			low = middle + 1;
		else
			high = middle;
	}
	return low;
}

/* a resource's bytes in memory: vertex buffers carry a physical address,
index buffers a virtual one (d3d8_resources.c) */
static unsigned long static_buffer_start(const void *resource, BOOL indices)
{
	DWORD data = ((const DWORD *)resource)[1];

	if (!data)
		return 0;
	return indices ? (unsigned long)data : (unsigned long)PLATFORM_PHYSICAL_TO_VIRTUAL(data);
}

static void static_buffer_forget(struct static_buffer_list *list, unsigned long start)
{
	unsigned long index = static_buffer_upper(list, start);
	struct static_buffer *entry;

	if (!index || list->entries[index - 1].start != start)
		return;
	entry = &list->entries[index - 1];
	if (entry->buffer)
	{
		/* (deleted at Present, from the thread that draws) */
		if (static_buffers.doomed_count == static_buffers.doomed_capacity)
		{
			unsigned long capacity = static_buffers.doomed_capacity ? static_buffers.doomed_capacity * 2 : 64;
			GLuint *doomed = realloc(static_buffers.doomed, capacity * sizeof(*doomed));

			if (doomed)
			{
				static_buffers.doomed = doomed;
				static_buffers.doomed_capacity = capacity;
			}
		}
		if (static_buffers.doomed_count < static_buffers.doomed_capacity)
			static_buffers.doomed[static_buffers.doomed_count++] = entry->buffer;
	}
	memmove(entry, entry + 1, (list->count - index) * sizeof(*entry));
	list->count--;
}

static void static_buffer_note(struct static_buffer_list *list, unsigned long start, unsigned long size)
{
	unsigned long index;
	struct static_buffer *entry;

	if (!android_geometry_fixes || !start || !size)
		return;
	static_buffer_forget(list, start);
	index = static_buffer_upper(list, start);
	/* (a bsp's vertices and lightmap vertices both run to its end: the one
	that starts lower covers the other) */
	if (index && list->entries[index - 1].start + list->entries[index - 1].size == start + size)
		return;
	if (index < list->count && list->entries[index].start + list->entries[index].size == start + size)
		static_buffer_forget(list, list->entries[index].start);
	/* (a range that would overlap another is left to stream) */
	if ((index && list->entries[index - 1].start + list->entries[index - 1].size > start) ||
		(index < list->count && start + size > list->entries[index].start))
	{
		return;
	}
	if (list->count == list->capacity)
	{
		unsigned long capacity = list->capacity ? list->capacity * 2 : 256;
		struct static_buffer *entries = realloc(list->entries, capacity * sizeof(*entries));

		if (!entries)
			return;
		list->entries = entries;
		list->capacity = capacity;
	}
	memmove(&list->entries[index + 1], &list->entries[index], (list->count - index) * sizeof(*entry));
	list->count++;
	entry = &list->entries[index];
	entry->start = start;
	entry->size = size;
	entry->buffer = 0;
	entry->failed = FALSE;
	entry->serial = ++static_buffers.serial;
}

void xgpu_static_buffer_add(const void *resource, unsigned long size, int indices)
{
	if (resource)
		static_buffer_note(&static_buffer_lists[indices != 0], static_buffer_start(resource, indices != 0), size);
}

void xgpu_static_buffer_remove(const void *resource, int indices)
{
	if (resource)
		static_buffer_forget(&static_buffer_lists[indices != 0], static_buffer_start(resource, indices != 0));
}

/* the lowest data of count resources, stride bytes apart; 0 without */
static unsigned long static_buffers_first(const void *buffers, long count, unsigned long stride, BOOL indices)
{
	unsigned long first = 0;
	long index;

	for (index = 0; buffers && index < count; index++)
	{
		unsigned long start = static_buffer_start((const unsigned char *)buffers + index * stride, indices);

		if (start && (!first || start < first))
			first = start;
	}
	return first;
}

void xgpu_static_buffers_add(const void *buffers, long count, unsigned long stride, int indices, const void *end)
{
	unsigned long first = static_buffers_first(buffers, count, stride, indices != 0);

	if (first && (unsigned long)end > first)
		static_buffer_note(&static_buffer_lists[indices != 0], first, (unsigned long)end - first);
}

void xgpu_static_buffers_remove(const void *buffers, long count, unsigned long stride, int indices)
{
	unsigned long first = static_buffers_first(buffers, count, stride, indices != 0);

	if (first)
		static_buffer_forget(&static_buffer_lists[indices != 0], first);
}

void xgpu_static_range_add(const void *start, unsigned long size, int indices)
{
	static_buffer_note(&static_buffer_lists[indices != 0], (unsigned long)start, size);
}

void xgpu_static_range_remove(const void *start, int indices)
{
	static_buffer_forget(&static_buffer_lists[indices != 0], (unsigned long)start);
}

/* the GL buffers of released resources, deleted; the state cache forgets
them, as GL unbinds them and may give their names to new buffers */
static void static_buffers_collect(void)
{
	if (!static_buffers.doomed_count)
		return;
	glDeleteBuffers((GLsizei)static_buffers.doomed_count, static_buffers.doomed);
	static_buffers.doomed_count = 0;
	xgpu_gl_state_invalidate();
}

/* as mirror_range, for [address, address + size) inside one static range
of the kind asked for: the GL buffer that holds it (filled now if no draw
has read it yet), the range's offset in it and the range's serial */
static BOOL static_buffer_range(unsigned long address, unsigned long size, BOOL indices, GLuint *buffer,
	unsigned long *offset, unsigned long *serial)
{
	struct static_buffer_list *list = &static_buffer_lists[indices != 0];
	unsigned long index = static_buffer_upper(list, address);
	struct static_buffer *entry;

	if (!android_geometry_fixes || !index || !size)
		return FALSE;
	entry = &list->entries[index - 1];
	if (address - entry->start > entry->size || size > entry->size - (address - entry->start))
		return FALSE;
	if (!entry->buffer)
	{
		int pending;

		if (entry->failed)
			return FALSE;
		glGenBuffers(1, &entry->buffer);
		if (!entry->buffer)
		{
			entry->failed = TRUE;
			return FALSE;
		}
		/* (the errors already queued, so that the upload's own are read) */
		for (pending = 0; pending < 8 && glGetError() != GL_NO_ERROR; pending++)
			;
		if (indices)
		{
			state_element_array_buffer(entry->buffer);
			glBufferData(GL_ELEMENT_ARRAY_BUFFER, (GLsizeiptr)entry->size, (const void *)entry->start, GL_STATIC_DRAW);
		}
		else
		{
			state_array_buffer(entry->buffer);
			glBufferData(GL_ARRAY_BUFFER, (GLsizeiptr)entry->size, (const void *)entry->start, GL_STATIC_DRAW);
		}
		if (glGetError() != GL_NO_ERROR)
		{
			/* (no storage, GL_OUT_OF_MEMORY: the buffer goes, unbinding
			itself, and the range streams from now on) */
			platform_log("GPU: no storage for %lu bytes of static %s; they are streamed", entry->size,
				indices ? "indices" : "vertices");
			glDeleteBuffers(1, &entry->buffer);
			entry->buffer = 0;
			entry->failed = TRUE;
			xgpu_gl_state_invalidate();
			return FALSE;
		}
	}
	*buffer = entry->buffer;
	*offset = address - entry->start;
	if (serial)
		*serial = entry->serial;
	return TRUE;
}
#endif

/* ---------- vertex data */

#ifdef HALO_GLES
/* port: WebGL on Metal (Chrome's and Safari's ANGLE) waits for the GPU to
finish everything queued before it writes a buffer that a queued draw reads
(without its shadow buffers, which Chrome leaves off): a glBufferSubData
into the stream buffer after each draw was a wait for the GPU each draw,
some 2000 a frame, the frame rate a few frames each second. So each draw's
uploads go into a buffer of their own that no queued draw reads, taken from
pools of a few sizes, one set for each frame of the ring: a buffer is
written again only STREAM_BUFFER_RING frames on, when the GPU is done with
it. Android does the same: on Snapdragon, vertices and indices written
(unsynchronized) into the frame's one stream buffer, which draws earlier
in the frame read, were sometimes drawn wrong, as long spikes, and
glBufferSubData into that buffer made the driver copy all of it each
time. */
#define WEB_CHUNK_SHIFT 12 /* 4 KB */
#define WEB_CHUNK_CLASSES 14 /* to 32 MB */

static BOOL stream_uses_chunks(void)
{
#ifdef HALO_ANDROID
	return android_geometry_fixes;
#else
	return TRUE;
#endif
}

static struct web_chunks
{
	GLuint *buffers;
	unsigned long count;
	unsigned long capacity;
	unsigned long next;
} web_chunks[2][STREAM_BUFFER_RING][WEB_CHUNK_CLASSES];

/* a buffer of at least size bytes for this frame's vertices (index 0) or
indices (index 1), bound, and its size */
static GLuint web_chunk_get(int indices, unsigned long size, unsigned long *capacity)
{
	struct web_chunks *chunks;
	int chunk_class = 0;
	GLuint buffer;

	while (chunk_class < WEB_CHUNK_CLASSES - 1 && (1UL << (WEB_CHUNK_SHIFT + chunk_class)) < size)
		chunk_class++;
	chunks = &web_chunks[indices][device.buffer_ring][chunk_class];
	*capacity = 1UL << (WEB_CHUNK_SHIFT + chunk_class);
	if (chunks->next == chunks->count)
	{
		if (chunks->count == chunks->capacity)
		{
			unsigned long grown_capacity = chunks->capacity ? chunks->capacity * 2 : 64;
			GLuint *grown = realloc(chunks->buffers, grown_capacity * sizeof(*grown));

			if (!grown)
			{
				/* (out of memory: the frame's own buffer, written as before) */
				*capacity = indices ? INDEX_BUFFER_SIZE : STREAM_BUFFER_SIZE;
				return indices ? device.index_buffers[device.buffer_ring] : device.stream_buffers[device.buffer_ring];
			}
			chunks->buffers = grown;
			chunks->capacity = grown_capacity;
		}
		glGenBuffers(1, &chunks->buffers[chunks->count]);
		buffer = chunks->buffers[chunks->count++];
		if (indices)
			state_element_array_buffer(buffer);
		else
			state_array_buffer(buffer);
		glBufferData(indices ? GL_ELEMENT_ARRAY_BUFFER : GL_ARRAY_BUFFER, (GLsizeiptr)*capacity, NULL, GL_STREAM_DRAW);
	}
	return chunks->buffers[chunks->next++];
}

/* the ring's next frame: its buffers are free again */
static void web_chunks_begin_frame(void)
{
	int indices, chunk_class;

	if (!stream_uses_chunks())
		return;
	for (indices = 0; indices < 2; indices++)
	{
		for (chunk_class = 0; chunk_class < WEB_CHUNK_CLASSES; chunk_class++)
			web_chunks[indices][device.buffer_ring][chunk_class].next = 0;
	}
	device.stream_capacity = 0;
	device.index_capacity = 0;
}
#endif

/* makes room for size bytes of uploads, orphaning the stream buffer if it
is full. A draw reserves room for all of its streams at once: orphaning
between two of them would leave the attributes already pointed at the
buffer reading its new, empty storage. */
static void stream_reserve(unsigned long size)
{
#ifdef HALO_GLES
	/* (a new buffer when a draw since the latest write may read this one) */
	if (stream_uses_chunks())
	{
		if (size && (device.stream_offset + size > device.stream_capacity ||
			(device.stream_offset && device.stream_serial != device.draw_serial)))
		{
			device.stream_buffer = web_chunk_get(0, size, &device.stream_capacity);
			device.stream_offset = 0;
		}
		return;
	}
#endif
	if (device.stream_offset + size > STREAM_BUFFER_SIZE)
	{
		/* orphan the buffer and start again */
		state_array_buffer(device.stream_buffer);
		glBufferData(GL_ARRAY_BUFFER, STREAM_BUFFER_SIZE, NULL, GL_STREAM_DRAW);
		device.stream_offset = 0;
	}
}

static unsigned long stream_upload(const void *data, unsigned long size);

#ifdef HALO_GLES
/* stream_upload, at a multiple of alignment into the buffer */
static unsigned long stream_upload_aligned(const void *data, unsigned long size, unsigned long alignment)
{
	stream_reserve(((size + 15) & ~15UL) + alignment);
	device.stream_offset += (alignment - device.stream_offset % alignment) % alignment;
	return stream_upload(data, size);
}
#endif

static unsigned long stream_upload(const void *data, unsigned long size)
{
	unsigned long offset;
	/* (as index_upload: the vertices' own bytes, in room rounded up to 16) */
	unsigned long length = size;

	size = (size + 15) & ~15UL;
	stream_reserve(size);
	offset = device.stream_offset;
	state_array_buffer(device.stream_buffer);
#ifdef HALO_WEB
	host_gl_buffer_write(GL_ARRAY_BUFFER, (unsigned int)offset, (unsigned int)length, data);
#elif defined(HALO_GLES)
	/* (into a pooled buffer that no queued draw reads, web_chunk_get) */
	if (stream_uses_chunks())
		glBufferSubData(GL_ARRAY_BUFFER, (GLintptr)offset, (GLsizeiptr)length, data);
	else
		host_gl_buffer_write(GL_ARRAY_BUFFER, (unsigned int)offset, (unsigned int)length, data);
#else
	buffer_upload(GL_ARRAY_BUFFER, offset, length, data);
#endif
#ifdef HALO_GLES
	device.stream_serial = device.draw_serial;
#endif
	device.stream_offset += size;
	return offset;
}

#ifdef HALO_GLES
/* stream_upload, with the D3DCOLOR elements of the stream turned from BGRA
into the RGBA byte order ES reads */
static unsigned long stream_upload_swizzled(const struct vertex_shader_object *declaration, unsigned long stream,
	const unsigned char *data, unsigned long size, unsigned long stride)
{
	static unsigned char *scratch;
	static unsigned long scratch_size;
	unsigned long offsets[XGPU_VERTEX_ATTRIBUTE_COUNT];
	unsigned long count = 0, index, vertex;

	for (index = 0; index < declaration->element_count; index++)
	{
		const struct vertex_element *element = &declaration->elements[index];

		if (element->stream == stream && element->type == D3DVSDT_D3DCOLOR)
			offsets[count++] = element->offset;
	}
	if (!count || !stride)
		return stream_upload(data, size);
	if (scratch_size < size)
	{
		free(scratch);
		scratch_size = size + 65536;
		scratch = malloc(scratch_size);
	}
	memcpy(scratch, data, size);
	for (vertex = 0; vertex + stride <= size; vertex += stride)
	{
		for (index = 0; index < count; index++)
		{
			unsigned char *color = scratch + vertex + offsets[index];
			unsigned char blue = color[0];

			color[0] = color[2];
			color[2] = blue;
		}
	}
	return stream_upload(scratch, size);
}
#endif

static unsigned long index_upload(const void *data, unsigned long size)
{
	unsigned long offset;
	/* (the indices' own bytes are written; the room they take is rounded up
	to 16, for the next ones' alignment: rounding what was read too read
	past the caller's indices) */
	unsigned long length = size;

	size = (size + 15) & ~15UL;
#ifdef HALO_GLES
	/* (as stream_reserve) */
	if (stream_uses_chunks())
	{
		if (device.index_offset + size > device.index_capacity ||
			(device.index_offset && device.index_serial != device.draw_serial))
		{
			device.index_buffer = web_chunk_get(1, size, &device.index_capacity);
			device.index_offset = 0;
		}
		state_element_array_buffer(device.index_buffer);
	}
	else
#endif
	{
		state_element_array_buffer(device.index_buffer);
		if (device.index_offset + size > INDEX_BUFFER_SIZE)
		{
			glBufferData(GL_ELEMENT_ARRAY_BUFFER, INDEX_BUFFER_SIZE, NULL, GL_STREAM_DRAW);
			device.index_offset = 0;
		}
	}
	offset = device.index_offset;
#ifdef HALO_WEB
	host_gl_buffer_write(GL_ELEMENT_ARRAY_BUFFER, (unsigned int)offset, (unsigned int)length, data);
#elif defined(HALO_GLES)
	/* (into a pooled buffer that no queued draw reads, web_chunk_get) */
	if (stream_uses_chunks())
		glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, (GLintptr)offset, (GLsizeiptr)length, data);
	else
		host_gl_buffer_write(GL_ELEMENT_ARRAY_BUFFER, (unsigned int)offset, (unsigned int)length, data);
#else
	buffer_upload(GL_ELEMENT_ARRAY_BUFFER, offset, length, data);
#endif
#ifdef HALO_GLES
	device.index_serial = device.draw_serial;
#endif
	device.index_offset += size;
	return offset;
}

static void attribute_format(const struct vertex_element *element, GLint *size, GLenum *type, GLboolean *normalized)
{
	*normalized = GL_FALSE;
	switch (element->type)
	{
	case D3DVSDT_FLOAT1: *size = 1; *type = GL_FLOAT; break;
	case D3DVSDT_FLOAT2: *size = 2; *type = GL_FLOAT; break;
	case D3DVSDT_FLOAT3: case D3DVSDT_FLOAT2H: *size = 3; *type = GL_FLOAT; break;
	case D3DVSDT_FLOAT4: *size = 4; *type = GL_FLOAT; break;
#ifdef HALO_GLES
	/* ES has no BGRA attributes: stream_upload_swizzled swaps the bytes */
	case D3DVSDT_D3DCOLOR: *size = 4; *type = GL_UNSIGNED_BYTE; *normalized = GL_TRUE; break;
#else
	case D3DVSDT_D3DCOLOR: *size = GL_BGRA; *type = GL_UNSIGNED_BYTE; *normalized = GL_TRUE; break;
#endif
	case D3DVSDT_SHORT1: *size = 1; *type = GL_SHORT; break;
	case D3DVSDT_SHORT2: *size = 2; *type = GL_SHORT; break;
	case D3DVSDT_SHORT3: *size = 3; *type = GL_SHORT; break;
	case D3DVSDT_SHORT4: *size = 4; *type = GL_SHORT; break;
	case D3DVSDT_NORMSHORT1: *size = 1; *type = GL_SHORT; *normalized = GL_TRUE; break;
	case D3DVSDT_NORMSHORT2: *size = 2; *type = GL_SHORT; *normalized = GL_TRUE; break;
	case D3DVSDT_NORMSHORT3: *size = 3; *type = GL_SHORT; *normalized = GL_TRUE; break;
	case D3DVSDT_NORMSHORT4: *size = 4; *type = GL_SHORT; *normalized = GL_TRUE; break;
	case D3DVSDT_PBYTE1: *size = 1; *type = GL_UNSIGNED_BYTE; *normalized = GL_TRUE; break;
	case D3DVSDT_PBYTE2: *size = 2; *type = GL_UNSIGNED_BYTE; *normalized = GL_TRUE; break;
	case D3DVSDT_PBYTE3: *size = 3; *type = GL_UNSIGNED_BYTE; *normalized = GL_TRUE; break;
	case D3DVSDT_PBYTE4: *size = 4; *type = GL_UNSIGNED_BYTE; *normalized = GL_TRUE; break;
	default: *size = 4; *type = GL_FLOAT; break;
	}
}

/* upload vertices [first, first + count) of every stream the declaration
uses and point the attributes at them; attribute data then starts at
vertex 0 of the uploaded range */
#ifdef HALO_GLES
/* ES has no BGRA attributes, so a stream with colours is swizzled as it is
uploaded (stream_upload_swizzled) and cannot come from the mirror */
static BOOL stream_has_colors(const struct vertex_shader_object *declaration, unsigned long stream)
{
	unsigned long index;

	for (index = 0; index < declaration->element_count; index++)
	{
		if (declaration->elements[index].stream == stream && declaration->elements[index].type == D3DVSDT_D3DCOLOR)
			return TRUE;
	}
	return FALSE;
}
#endif

#ifdef HALO_WEB
/* Each draw's attributes would be pointed again (a call into the browser
for each) whenever it reads its streams from another place than the draw
before it, which is nearly always. The draws whose streams all come from
the mirror read the same places frame after frame (a model's part, the
structure's), so each such set of places has a vertex array of its own,
made the first time, and the draw binds it: one call. Other draws use the
device's own vertex array, pointed as before. A vertex array holds the
element array buffer's binding too, kept here for each. Past
VERTEX_ARRAY_CACHE_MAXIMUM, they are all deleted and made again. */
#define VERTEX_ARRAY_CACHE_SLOTS 8192
#define VERTEX_ARRAY_CACHE_MAXIMUM 4096

struct vertex_array_cache_entry
{
	const struct vertex_shader_object *declaration;
	unsigned long streams;
	GLuint buffers[16];
	unsigned long offsets[16];
	unsigned long strides[16];
	GLuint vertex_array;
	/* its element array buffer binding, when known: kept in the epoch */
	GLuint element_array_buffer;
	unsigned long element_epoch;
};

static struct
{
	struct vertex_array_cache_entry *slots[VERTEX_ARRAY_CACHE_SLOTS];
	unsigned long count;
	/* the vertex array bound: an entry, or NULL for the device's own */
	struct vertex_array_cache_entry *bound;
	BOOL bound_known;
	GLuint device_element_array_buffer;
	/* the entries' element array buffer bindings are known only from the
	latest invalidation on (xgpu_gl_state_invalidate) */
	unsigned long element_epoch;
} vertex_array_cache;

static void vertex_array_cache_invalidate(void)
{
	vertex_array_cache.bound_known = FALSE;
	vertex_array_cache.device_element_array_buffer = (GLuint)-1;
	vertex_array_cache.element_epoch++;
}

/* binds the entry's vertex array (NULL: the device's own) */
static void vertex_array_cache_bind(struct vertex_array_cache_entry *entry)
{
	if (vertex_array_cache.bound_known && vertex_array_cache.bound == entry)
		return;
	if (vertex_array_cache.bound_known)
	{
		if (vertex_array_cache.bound)
		{
			vertex_array_cache.bound->element_array_buffer = gl_state.element_array_buffer;
			vertex_array_cache.bound->element_epoch = vertex_array_cache.element_epoch;
		}
		else
			vertex_array_cache.device_element_array_buffer = gl_state.element_array_buffer;
	}
	else
	{
		/* (what was bound to the device's own is not known) */
		vertex_array_cache.device_element_array_buffer = (GLuint)-1;
	}
	glBindVertexArray(entry ? entry->vertex_array : device.vertex_array);
	vertex_array_cache.bound = entry;
	vertex_array_cache.bound_known = TRUE;
	gl_state.element_array_buffer = !entry ? vertex_array_cache.device_element_array_buffer :
		entry->element_epoch == vertex_array_cache.element_epoch ? entry->element_array_buffer : (GLuint)-1;
}

static void vertex_array_cache_clear(void)
{
	unsigned long slot;

	vertex_array_cache_bind(NULL);
	for (slot = 0; slot < VERTEX_ARRAY_CACHE_SLOTS; slot++)
	{
		struct vertex_array_cache_entry *entry = vertex_array_cache.slots[slot];

		if (entry)
		{
			glDeleteVertexArrays(1, &entry->vertex_array);
			free(entry);
			vertex_array_cache.slots[slot] = NULL;
		}
	}
	vertex_array_cache.count = 0;
}

/* binds the vertex array of the declaration reading its streams (streams,
a bit each) from buffers at offsets; FALSE if there is none and none can be
made */
static BOOL vertex_array_cache_use(const struct vertex_shader_object *declaration, unsigned long streams,
	const GLuint *buffers, const unsigned long *offsets)
{
	unsigned long hash = 2166136261UL ^ (unsigned long)declaration, stream, slot, probe, index;
	struct vertex_array_cache_entry *entry = NULL;

	for (stream = 0; stream < 16; stream++)
	{
		if (streams & (1UL << stream))
		{
			hash = (hash ^ buffers[stream]) * 16777619UL;
			hash = (hash ^ offsets[stream]) * 16777619UL;
			hash = (hash ^ device.streams[stream].stride) * 16777619UL;
		}
	}
	hash = (hash ^ streams) * 16777619UL;
	slot = hash % VERTEX_ARRAY_CACHE_SLOTS;
	for (probe = 0; probe < VERTEX_ARRAY_CACHE_SLOTS; probe++, slot = (slot + 1) % VERTEX_ARRAY_CACHE_SLOTS)
	{
		struct vertex_array_cache_entry *candidate = vertex_array_cache.slots[slot];

		if (!candidate)
			break;
		if (candidate->declaration != declaration || candidate->streams != streams)
			continue;
		for (stream = 0; stream < 16; stream++)
		{
			if ((streams & (1UL << stream)) && (candidate->buffers[stream] != buffers[stream] ||
				candidate->offsets[stream] != offsets[stream] || candidate->strides[stream] != device.streams[stream].stride))
			{
				break;
			}
		}
		if (stream == 16)
		{
			entry = candidate;
			break;
		}
	}
	if (!entry)
	{
		if (vertex_array_cache.count >= VERTEX_ARRAY_CACHE_MAXIMUM)
		{
			vertex_array_cache_clear();
			return vertex_array_cache_use(declaration, streams, buffers, offsets);
		}
		entry = calloc(1, sizeof(*entry));
		if (!entry)
			return FALSE;
		entry->declaration = declaration;
		entry->streams = streams;
		for (stream = 0; stream < 16; stream++)
		{
			if (streams & (1UL << stream))
			{
				entry->buffers[stream] = buffers[stream];
				entry->offsets[stream] = offsets[stream];
				entry->strides[stream] = device.streams[stream].stride;
			}
		}
		glGenVertexArrays(1, &entry->vertex_array);
		entry->element_array_buffer = 0;
		entry->element_epoch = vertex_array_cache.element_epoch;
		vertex_array_cache.slots[slot] = entry;
		vertex_array_cache.count++;
		vertex_array_cache_bind(entry);
		for (index = 0; index < declaration->element_count; index++)
		{
			const struct vertex_element *element = &declaration->elements[index];
			GLint size;
			GLenum type;
			GLboolean normalized;

			stream = element->stream;
			if (!(streams & (1UL << stream)) || element->type == D3DVSDT_NONE)
				continue;
			state_array_buffer(buffers[stream]);
			glEnableVertexAttribArray(element->reg);
			if (element->type == D3DVSDT_NORMPACKED3)
			{
				glVertexAttribIPointer(element->reg, 1, GL_UNSIGNED_INT, (GLsizei)device.streams[stream].stride,
					(const void *)(offsets[stream] + element->offset));
			}
			else
			{
				attribute_format(element, &size, &type, &normalized);
				glVertexAttribPointer(element->reg, size, type, normalized, (GLsizei)device.streams[stream].stride,
					(const void *)(offsets[stream] + element->offset));
			}
		}
		return TRUE;
	}
	vertex_array_cache_bind(entry);
	return TRUE;
}
#endif

static void setup_streams(unsigned long first, unsigned long count)
{
	struct vertex_shader_object *declaration = device.vertex_shader;
	GLuint stream_buffers[16];
	unsigned long stream_offsets[16];
	BOOL placed[16] = { FALSE };
	BOOL enabled[XGPU_VERTEX_ATTRIBUTE_COUNT] = { FALSE };
	unsigned long index, total = 0;
#ifndef HALO_GLES
	struct vertex_layout layout;
	unsigned long streams_used = 0;

	/* (zeroed: layouts are compared and hashed whole) */
	memset(&layout, 0, sizeof(layout));
#endif

	/* the mirror first; then one reservation for everything streamed */
	for (index = 0; index < declaration->element_count; index++)
	{
		const struct vertex_element *element = &declaration->elements[index];
		unsigned long stream = element->stream;
		unsigned long stride = device.streams[stream].stride;
		unsigned long bytes = stride ? stride * count : 64;
		unsigned long base;

		if (!device.streams[stream].data || element->type == D3DVSDT_NONE || placed[stream])
			continue;
		placed[stream] = TRUE;
		stream_buffers[stream] = 0;
		base = (unsigned long)PLATFORM_PHYSICAL_TO_VIRTUAL(device.streams[stream].data) + first * stride;
#ifdef HALO_ANDROID
		/* (a model's or the BSP's vertices, uploaded once) */
		if (!stream_has_colors(declaration, stream) &&
			static_buffer_range(base, bytes, FALSE, &stream_buffers[stream], &stream_offsets[stream], NULL))
		{
			continue;
		}
#endif
#ifdef HALO_GLES
		if (!stream_has_colors(declaration, stream))
#endif
		if (mirror_range(base, bytes, &stream_buffers[stream], &stream_offsets[stream], NULL))
			continue;
		stream_buffers[stream] = 0;
		total += (bytes + 15) & ~15UL;
	}
#ifdef HALO_WEB
	{
		/* every stream from the mirror: a vertex array of their own
		(vertex_array_cache) */
		unsigned long streams = 0;

		for (index = 0; index < 16; index++)
		{
			if (placed[index])
				streams |= 1UL << index;
		}
		if (!total && streams && vertex_array_cache_use(declaration, streams, stream_buffers, stream_offsets))
		{
			for (index = 0; index < declaration->element_count; index++)
			{
				const struct vertex_element *element = &declaration->elements[index];

				if (device.streams[element->stream].data && element->type != D3DVSDT_NONE)
					enabled[element->reg] = TRUE;
			}
			for (index = 0; index < XGPU_VERTEX_ATTRIBUTE_COUNT; index++)
			{
				if (!enabled[index])
					state_attribute_constant(index, declaration->packed_mask & (1UL << index) ? NULL : device.attributes[index]);
			}
			return;
		}
		vertex_array_cache_bind(NULL);
	}
#endif
	stream_reserve(total);
	for (index = 0; index < declaration->element_count; index++)
	{
		const struct vertex_element *element = &declaration->elements[index];
		unsigned long stream = element->stream;
		unsigned long stride = device.streams[stream].stride;
		GLint size;
		GLenum type;
		GLboolean normalized;

		if (!device.streams[stream].data || element->type == D3DVSDT_NONE)
			continue;
		if (!stream_buffers[stream])
		{
			const unsigned char *base = PLATFORM_PHYSICAL_TO_VIRTUAL(device.streams[stream].data);
			unsigned long bytes = stride ? stride * count : 64;

#ifdef HALO_GLES
			stream_offsets[stream] = stream_upload_swizzled(declaration, stream, base + first * stride, bytes, stride);
#else
			stream_offsets[stream] = stream_upload(base + first * stride, bytes);
#endif
			stream_buffers[stream] = device.stream_buffer;
			stats.streamed_bytes += bytes;
		}
#ifdef HALO_GLES
		if (element->type == D3DVSDT_NORMPACKED3)
		{
			state_attribute_stream(element->reg, (GLuint)stream, stream_buffers[stream], 1, GL_UNSIGNED_INT, GL_FALSE,
				TRUE, (GLsizei)stride, stream_offsets[stream], element->offset);
		}
		else
		{
			attribute_format(element, &size, &type, &normalized);
			state_attribute_stream(element->reg, (GLuint)stream, stream_buffers[stream], size, type, normalized,
				FALSE, (GLsizei)stride, stream_offsets[stream], element->offset);
		}
#else
		if (element->type == D3DVSDT_NORMPACKED3)
		{
			layout_attribute(&layout, element->reg, (GLuint)stream, 1, GL_UNSIGNED_INT, GL_FALSE, TRUE, element->offset);
		}
		else
		{
			attribute_format(element, &size, &type, &normalized);
			layout_attribute(&layout, element->reg, (GLuint)stream, size, type, normalized, FALSE, element->offset);
		}
		streams_used |= 1UL << stream;
#endif
		enabled[element->reg] = TRUE;
	}
#ifndef HALO_GLES
	/* (the layout follows from the declaration and the streams it has, so
	the same two have the same vertex array: no need to look it up) */
	if (!declaration->vertex_array || declaration->vertex_array_streams != streams_used)
	{
		declaration->vertex_array = vertex_array_get(&layout);
		declaration->vertex_array_streams = streams_used;
	}
	state_vertex_array(declaration->vertex_array);
	for (index = 0; index < 16; index++)
	{
		if (streams_used & (1UL << index))
			state_vertex_buffer((GLuint)index, stream_buffers[index], stream_offsets[index],
				(GLsizei)device.streams[index].stride);
	}
#endif
	for (index = 0; index < XGPU_VERTEX_ATTRIBUTE_COUNT; index++)
	{
		if (!enabled[index])
			state_attribute_value(index, declaration->packed_mask & (1UL << index) ? NULL : device.attributes[index]);
	}
}

static GLenum primitive_mode(D3DPRIMITIVETYPE type)
{
	switch (type)
	{
	case D3DPT_POINTLIST: return GL_POINTS;
	case D3DPT_LINELIST: return GL_LINES;
	case D3DPT_LINELOOP: return GL_LINE_LOOP;
	case D3DPT_LINESTRIP: return GL_LINE_STRIP;
	case D3DPT_TRIANGLESTRIP:
	case D3DPT_QUADSTRIP: return GL_TRIANGLE_STRIP;
	case D3DPT_TRIANGLEFAN:
	case D3DPT_POLYGON: return GL_TRIANGLE_FAN;
	default: return GL_TRIANGLES;
	}
}

/* quads become two triangles each */
static WORD *quad_indices(const WORD *indices, unsigned long count, unsigned long *out_count)
{
	unsigned long quads = count / 4;
	WORD *result = malloc(quads * 6 * sizeof(WORD) + 2);
	unsigned long quad;

	for (quad = 0; quad < quads; quad++)
	{
		WORD v0 = indices ? indices[quad * 4] : (WORD)(quad * 4);
		WORD v1 = indices ? indices[quad * 4 + 1] : (WORD)(quad * 4 + 1);
		WORD v2 = indices ? indices[quad * 4 + 2] : (WORD)(quad * 4 + 2);
		WORD v3 = indices ? indices[quad * 4 + 3] : (WORD)(quad * 4 + 3);

		result[quad * 6 + 0] = v0;
		result[quad * 6 + 1] = v1;
		result[quad * 6 + 2] = v2;
		result[quad * 6 + 3] = v0;
		result[quad * 6 + 4] = v2;
		result[quad * 6 + 5] = v3;
	}
	*out_count = quads * 6;
	return result;
}

void WINAPI D3DDevice_SetStreamSource(UINT stream_number, D3DVertexBuffer *stream_data, UINT stride)
{
	if (stream_number >= 16)
		return;
	device.streams[stream_number].data = stream_data ? stream_data->Data : 0;
	device.streams[stream_number].stride = stride;
}

void WINAPI D3DDevice_SetIndices(D3DIndexBuffer *index_data, UINT base_vertex_index)
{
	device.base_vertex_index = base_vertex_index;
	D3D__IndexData = index_data ? (WORD *)index_data->Data : NULL;
}

void WINAPI D3DDevice_DrawVertices(D3DPRIMITIVETYPE primitive_type, UINT start_vertex, UINT vertex_count)
{
	if (!vertex_count || !prepare_draw(FALSE))
		return;
	trace_draw("draw", primitive_type, vertex_count, NULL);
	setup_streams(start_vertex, vertex_count);
	if (primitive_type == D3DPT_QUADLIST)
	{
		unsigned long count;
		WORD *indices = quad_indices(NULL, vertex_count, &count);

		glDrawElements(GL_TRIANGLES, (GLsizei)count, GL_UNSIGNED_SHORT,
			(const void *)index_upload(indices, count * sizeof(WORD)));
		free(indices);
	}
	else
	{
		glDrawArrays(primitive_mode(primitive_type), 0, (GLsizei)vertex_count);
	}
	gl_check_errors("draw");
}

#ifdef HALO_WEB
/* WebGL has no base vertex, so the indices of a draw are rebased (less their
smallest) and uploaded with it, a call into the browser each and a check of
their range by it; and the game's index buffers are ordinary memory, which
the mirror does not hold. An index array drawn twice the same is kept
instead, rebased, in a buffer of its own: a draw of it compares its indices
with a copy and draws from the buffer. Arrays that change (the structure's
visible triangles, made each frame) are never the same twice, and are
uploaded as before. When the buffer is full, everything kept is forgotten.

WebGL on Metal waits for the GPU before it writes a buffer that a queued
draw reads (web_chunk_get), and the kept arrays are drawn from every frame:
written into one buffer as they were kept, every few frames while a level
was explored, each was a wait of 20 to 40 ms. So there is a buffer for each
frame of the ring, drawn from only in that frame, and an array kept is
written into a frame's buffer as the frame begins, when the GPU is done with
it (STREAM_BUFFER_RING frames on, as for the chunks). Until its frame's
buffer has it, a draw of it uploads its indices as before. */
#define INDEX_CACHE_BUFFER_SIZE (8 * 1024 * 1024)
#define INDEX_CACHE_SLOTS 8192
#define INDEX_CACHE_PROBES 4
#define INDEX_CACHE_ALL_FRAMES ((1u << STREAM_BUFFER_RING) - 1)

struct index_cache_entry
{
	const WORD *address;
	unsigned long count;
	/* the indices as last drawn from address, and room for them */
	WORD *copy;
	unsigned long capacity;
	WORD minimum, maximum;
	/* in the buffers (when kept is the epoch), or not yet: seen once */
	unsigned long offset;
	unsigned long kept;
	/* the frames of the ring whose buffers have it, a bit each */
	unsigned char written;
};

static struct
{
	GLuint buffers[STREAM_BUFFER_RING];
	unsigned long used;
	/* entries kept in an earlier epoch are not in the buffers */
	unsigned long epoch;
	/* device.frame when its frame's buffer was last brought up to date */
	unsigned long written_frame;
	struct index_cache_entry entries[INDEX_CACHE_SLOTS];
	/* the entries kept that some frame's buffer has not got yet */
	struct index_cache_entry *pending[INDEX_CACHE_SLOTS];
	unsigned long pending_count;
} index_cache = { { 0 }, 0, 1, (unsigned long)-1 };

/* writes the entry's indices, rebased, into the ring frame's buffer; FALSE
if there is no memory to rebase them */
static BOOL index_cache_write(const struct index_cache_entry *entry, unsigned long ring)
{
	WORD *rebased;
	unsigned long index;

	if (!index_cache.buffers[ring])
	{
		glGenBuffers(1, &index_cache.buffers[ring]);
		state_element_array_buffer(index_cache.buffers[ring]);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, INDEX_CACHE_BUFFER_SIZE, NULL, GL_STATIC_DRAW);
	}
	rebased = malloc(entry->count * sizeof(WORD) + 2);
	if (!rebased)
		return FALSE;
	for (index = 0; index < entry->count; index++)
		rebased[index] = (WORD)(entry->copy[index] - entry->minimum);
	state_element_array_buffer(index_cache.buffers[ring]);
	host_gl_buffer_write(GL_ELEMENT_ARRAY_BUFFER, (unsigned int)entry->offset,
		(unsigned int)(entry->count * sizeof(WORD)), rebased);
	free(rebased);
	return TRUE;
}

/* once a frame, before its first draw from the cache: the arrays kept since
its buffer was last drawn from are written into it, which no queued draw
reads now */
static void index_cache_begin_frame(void)
{
	unsigned long ring = device.buffer_ring, index, remaining = 0;

	if (index_cache.written_frame == device.frame)
		return;
	index_cache.written_frame = device.frame;
	for (index = 0; index < index_cache.pending_count; index++)
	{
		struct index_cache_entry *entry = index_cache.pending[index];

		/* (forgotten, or remembered anew, since it was kept) */
		if (entry->kept != index_cache.epoch)
			continue;
		if (!(entry->written & (1u << ring)) && index_cache_write(entry, ring))
			entry->written |= (unsigned char)(1u << ring);
		if (entry->written != INDEX_CACHE_ALL_FRAMES)
			index_cache.pending[remaining++] = entry;
	}
	index_cache.pending_count = remaining;
}

/* the offset in this frame's buffer of the rebased indices kept for the
array, with their extent; FALSE if it is not kept (yet) or not yet in this
frame's buffer */
static BOOL index_cache_get(const WORD *indices, unsigned long count, unsigned long *minimum, unsigned long *maximum,
	unsigned long *offset)
{
	unsigned long slot = (((unsigned long)indices >> 1) ^ (count * 2654435761UL)) % INDEX_CACHE_SLOTS;
	unsigned long probe, size = count * sizeof(WORD);
	struct index_cache_entry *entry = NULL, *oldest = NULL;

	index_cache_begin_frame();
	for (probe = 0; probe < INDEX_CACHE_PROBES; probe++)
	{
		struct index_cache_entry *candidate = &index_cache.entries[(slot + probe) % INDEX_CACHE_SLOTS];

		if (candidate->address == indices && candidate->count == count)
		{
			entry = candidate;
			break;
		}
		if (!oldest || candidate->kept < oldest->kept)
			oldest = candidate;
	}
	if (entry && entry->copy && !memcmp(entry->copy, indices, size))
	{
		if (entry->kept == index_cache.epoch)
		{
			if (!(entry->written & (1u << device.buffer_ring)))
				return FALSE;
			*minimum = entry->minimum;
			*maximum = entry->maximum;
			*offset = entry->offset;
			return TRUE;
		}
		/* the second time the same: kept from now on, in the frames'
		buffers as each of them begins (index_cache_begin_frame) */
		if (index_cache.used + ((size + 15) & ~15UL) > INDEX_CACHE_BUFFER_SIZE)
		{
			index_cache.epoch++;
			index_cache.used = 0;
			index_cache.pending_count = 0;
		}
		if (index_cache.pending_count == INDEX_CACHE_SLOTS)
			return FALSE;
		entry->offset = index_cache.used;
		entry->kept = index_cache.epoch;
		entry->written = 0;
		index_cache.used += (size + 15) & ~15UL;
		index_cache.pending[index_cache.pending_count++] = entry;
		return FALSE;
	}
	/* seen for the first time, or changed: remembered */
	if (!entry)
		entry = oldest;
	if (entry->capacity < count)
	{
		WORD *grown = realloc(entry->copy, size);

		if (!grown)
		{
			entry->address = NULL;
			return FALSE;
		}
		entry->copy = grown;
		entry->capacity = count;
	}
	memcpy(entry->copy, indices, size);
	entry->address = indices;
	entry->count = count;
	entry->kept = 0;
	index_extent(indices, count, 0, FALSE, minimum, maximum);
	entry->minimum = (WORD)*minimum;
	entry->maximum = (WORD)*maximum;
	return FALSE;
}
#endif

void WINAPI D3DDevice_DrawIndexedVertices(D3DPRIMITIVETYPE primitive_type, UINT vertex_count, CONST WORD *index_data)
{
	unsigned long minimum, maximum, index, count, generation = 0, index_offset = 0;
	WORD *indices = NULL;
	const WORD *source = index_data;
	GLuint index_buffer = 0;
	BOOL mirrored;

	if (!vertex_count || !index_data || !prepare_draw(FALSE))
		return;
	/* quads are drawn as triangles, from indices made for the draw */
	mirrored = primitive_type != D3DPT_QUADLIST &&
#ifdef HALO_GLES
		xgpu_capabilities.base_vertex &&
#endif
		mirror_range((unsigned long)index_data, vertex_count * sizeof(WORD), &index_buffer, &index_offset, &generation);
#ifdef HALO_ANDROID
	/* (a model's or the BSP's indices, uploaded once: drawn as the mirror's
	are, their extent cached under the buffer's serial) */
	if (!mirrored && primitive_type != D3DPT_QUADLIST && xgpu_capabilities.base_vertex)
	{
		mirrored = static_buffer_range((unsigned long)index_data, vertex_count * sizeof(WORD), TRUE, &index_buffer,
			&index_offset, &generation);
	}
#endif
#ifdef HALO_WEB
	if (!mirrored && primitive_type != D3DPT_QUADLIST &&
		index_cache_get(index_data, vertex_count, &minimum, &maximum, &index_offset))
	{
		trace_draw("indexed", primitive_type, vertex_count, NULL);
		setup_streams(device.base_vertex_index + minimum, maximum - minimum + 1);
		state_element_array_buffer(index_cache.buffers[device.buffer_ring]);
		glDrawElements(primitive_mode(primitive_type), (GLsizei)vertex_count, GL_UNSIGNED_SHORT, (const void *)index_offset);
		return;
	}
#endif
	index_extent(index_data, vertex_count, generation, mirrored, &minimum, &maximum);
	trace_draw("indexed", primitive_type, vertex_count, NULL);
	/* (the streams from the base vertex on: index i is vertex base + i) */
	setup_streams(device.base_vertex_index + minimum, maximum - minimum + 1);
	if (mirrored)
	{
		/* the attributes start at vertex minimum */
		state_element_array_buffer(index_buffer);
		glDrawElementsBaseVertex(primitive_mode(primitive_type), (GLsizei)vertex_count, GL_UNSIGNED_SHORT,
			(const void *)index_offset, -(GLint)minimum);
		return;
	}
	stats.streamed_bytes += vertex_count * sizeof(WORD);
	count = vertex_count;
	if (primitive_type == D3DPT_QUADLIST)
	{
		indices = quad_indices(index_data, vertex_count, &count);
		source = indices;
	}
#ifdef HALO_GLES
	if (!xgpu_capabilities.base_vertex)
	{
		/* the indices are copied anyway: rebase them */
		WORD *rebased = malloc(count * sizeof(WORD) + 2);

		for (index = 0; index < count; index++)
			rebased[index] = (WORD)(source[index] - minimum);
		glDrawElements(primitive_mode(primitive_type), (GLsizei)count, GL_UNSIGNED_SHORT,
			(const void *)index_upload(rebased, count * sizeof(WORD)));
		free(rebased);
		free(indices);
		return;
	}
#endif
	(void)index;
	glDrawElementsBaseVertex(primitive_mode(primitive_type), (GLsizei)count, GL_UNSIGNED_SHORT,
		(const void *)index_upload(source, count * sizeof(WORD)), -(GLint)minimum);
	free(indices);
}

/* ---------- immediate mode */

void WINAPI D3DDevice_Begin(D3DPRIMITIVETYPE primitive_type)
{
	device.immediate_active = TRUE;
	device.immediate_type = primitive_type;
	device.immediate_count = 0;
	device.immediate_written = 0;
}

static void immediate_emit(void)
{
	unsigned long floats = XGPU_VERTEX_ATTRIBUTE_COUNT * 4;

	if (device.immediate_count == device.immediate_capacity)
	{
		unsigned long capacity = device.immediate_capacity ? device.immediate_capacity * 2 : 256;
		float *vertices = realloc(device.immediate_vertices, capacity * floats * sizeof(float));

		/* (out of memory: the vertex is dropped) */
		if (!vertices)
			return;
		device.immediate_vertices = vertices;
		device.immediate_capacity = capacity;
	}
	memcpy(device.immediate_vertices + device.immediate_count * floats, device.attributes, floats * sizeof(float));
	device.immediate_count++;
}

void WINAPI D3DDevice_End(void)
{
	unsigned long stride = XGPU_VERTEX_ATTRIBUTE_COUNT * 4 * sizeof(float);
	unsigned long offset, index, count = device.immediate_count;
	D3DPRIMITIVETYPE type = device.immediate_type;
	/* (the vertices' place in the streaming buffer, where the attributes'
	pointers start at its beginning) */
	unsigned long first_vertex = 0;

	device.immediate_active = FALSE;
	if (!count || !prepare_draw(TRUE))
		return;
	/* (the fixed-function pipeline's: positions into clip space) */
	if (device.fixed_function)
	{
		for (index = 0; index < count; index++)
			fixed_function_transform(device.immediate_vertices + index * XGPU_VERTEX_ATTRIBUTE_COUNT * 4);
	}
	trace_draw("immediate", type, count, device.immediate_vertices);
#ifdef HALO_WEB
	vertex_array_cache_bind(NULL);
#endif
	{
		/* Only the registers written since Begin are uploaded, one after
		another in each vertex; the others are constants (most draws write 3
		of the 16, and a vertex of all of them is 256 bytes). On ES the
		vertices start a whole number of vertices into the streaming buffer
		and are drawn from there (first_vertex), so that draws of the same
		registers keep the attributes' pointers: each pointer set is a call
		into the driver, or the browser. All sixteen would be 256 bytes, more
		than WebGL's largest stride (255): each attribute then goes in an
		array of its own there. */
		static float *packed;
		static unsigned long packed_capacity;
		unsigned long attribute_size = 4 * sizeof(float);
		unsigned long registers[XGPU_VERTEX_ATTRIBUTE_COUNT];
		unsigned long register_count = 0, vertex_size, vertex, slot;

		for (index = 0; index < XGPU_VERTEX_ATTRIBUTE_COUNT; index++)
		{
			if (device.immediate_written & (1UL << index))
				registers[register_count++] = index;
		}
		if (count > packed_capacity)
		{
			float *grown = realloc(packed, count * stride);

			/* (out of memory: the draw is dropped) */
			if (!grown)
				return;
			packed = grown;
			packed_capacity = count;
		}
		if (register_count < XGPU_VERTEX_ATTRIBUTE_COUNT)
		{
			vertex_size = register_count * attribute_size;
			for (vertex = 0; vertex < count; vertex++)
			{
				for (slot = 0; slot < register_count; slot++)
				{
					memcpy(packed + (vertex * register_count + slot) * 4,
						device.immediate_vertices + (vertex * XGPU_VERTEX_ATTRIBUTE_COUNT + registers[slot]) * 4,
						attribute_size);
				}
			}
#ifdef HALO_GLES
			offset = stream_upload_aligned(packed, count * vertex_size, vertex_size);
			first_vertex = offset / vertex_size;
			for (slot = 0; slot < register_count; slot++)
			{
				state_attribute_stream(registers[slot], 0, device.stream_buffer, 4, GL_FLOAT, GL_FALSE, FALSE,
					(GLsizei)vertex_size, 0, slot * attribute_size);
			}
#else
			offset = stream_upload(packed, count * vertex_size);
			{
				/* (the vertex array of the last few sets of registers) */
				static struct
				{
					unsigned long written;
					struct vertex_array_entry *entry;
				} arrays[8];
				static unsigned long next_array;
				struct vertex_array_entry *entry = NULL;

				for (slot = 0; slot < 8; slot++)
				{
					if (arrays[slot].entry && arrays[slot].written == device.immediate_written)
						entry = arrays[slot].entry;
				}
				if (!entry)
				{
					struct vertex_layout layout;

					memset(&layout, 0, sizeof(layout));
					for (slot = 0; slot < register_count; slot++)
						layout_attribute(&layout, registers[slot], 0, 4, GL_FLOAT, GL_FALSE, FALSE, slot * attribute_size);
					entry = vertex_array_get(&layout);
					arrays[next_array % 8].written = device.immediate_written;
					arrays[next_array % 8].entry = entry;
					next_array++;
				}
				state_vertex_array(entry);
				state_vertex_buffer(0, device.stream_buffer, offset, (GLsizei)vertex_size);
			}
#endif
			for (index = 0; index < XGPU_VERTEX_ATTRIBUTE_COUNT; index++)
			{
				if (!(device.immediate_written & (1UL << index)))
					state_attribute_value(index, device.attributes[index]);
			}
		}
		else
		{
#ifdef HALO_WEB
			for (index = 0; index < XGPU_VERTEX_ATTRIBUTE_COUNT; index++)
			{
				for (vertex = 0; vertex < count; vertex++)
				{
					memcpy(packed + (index * count + vertex) * 4,
						device.immediate_vertices + (vertex * XGPU_VERTEX_ATTRIBUTE_COUNT + index) * 4, attribute_size);
				}
			}
			offset = stream_upload(packed, count * stride);
			for (index = 0; index < XGPU_VERTEX_ATTRIBUTE_COUNT; index++)
			{
				state_attribute_stream(index, 0, device.stream_buffer, 4, GL_FLOAT, GL_FALSE, FALSE,
					(GLsizei)attribute_size, offset, index * count * attribute_size);
			}
#elif defined(HALO_GLES)
			offset = stream_upload(device.immediate_vertices, count * stride);
			for (index = 0; index < XGPU_VERTEX_ATTRIBUTE_COUNT; index++)
			{
				state_attribute_stream(index, 0, device.stream_buffer, 4, GL_FLOAT, GL_FALSE, FALSE, (GLsizei)stride,
					offset, index * 4 * sizeof(float));
			}
#else
			offset = stream_upload(device.immediate_vertices, count * stride);
			{
				/* every attribute four floats, one after another */
				static struct vertex_array_entry *immediate_array;

				if (!immediate_array)
				{
					struct vertex_layout layout;

					memset(&layout, 0, sizeof(layout));
					for (index = 0; index < XGPU_VERTEX_ATTRIBUTE_COUNT; index++)
						layout_attribute(&layout, index, 0, 4, GL_FLOAT, GL_FALSE, FALSE, index * 4 * sizeof(float));
					immediate_array = vertex_array_get(&layout);
				}
				state_vertex_array(immediate_array);
				state_vertex_buffer(0, device.stream_buffer, offset, (GLsizei)stride);
			}
#endif
		}
	}
	if (type == D3DPT_QUADLIST)
	{
		unsigned long index_count;
		WORD *indices = quad_indices(NULL, count, &index_count);

		if (first_vertex)
		{
			/* (the indices from the first vertex on, past what 16 bits hold) */
			GLuint *placed = indices ? malloc(index_count * sizeof(GLuint)) : NULL;

			if (placed)
			{
				for (index = 0; index < index_count; index++)
					placed[index] = (GLuint)(first_vertex + indices[index]);
				glDrawElements(GL_TRIANGLES, (GLsizei)index_count, GL_UNSIGNED_INT,
					(const void *)index_upload(placed, index_count * sizeof(GLuint)));
				free(placed);
			}
		}
		else
		{
			glDrawElements(GL_TRIANGLES, (GLsizei)index_count, GL_UNSIGNED_SHORT,
				(const void *)index_upload(indices, index_count * sizeof(WORD)));
		}
		free(indices);
	}
	else
	{
		glDrawArrays(primitive_mode(type), (GLint)first_vertex, (GLsizei)count);
	}
	gl_check_errors("immediate draw");
}

static void set_attribute(INT reg, float a, float b, float c, float d)
{
	BOOL emit = FALSE;

	if (reg == D3DVSDE_VERTEX)
	{
		reg = 0;
		emit = TRUE;
	}
	if (reg < 0 || reg >= XGPU_VERTEX_ATTRIBUTE_COUNT)
		return;
	device.attributes[reg][0] = a;
	device.attributes[reg][1] = b;
	device.attributes[reg][2] = c;
	device.attributes[reg][3] = d;
	if (device.immediate_active)
		device.immediate_written |= 1UL << reg;
	/* like the hardware, writing register 0 completes a vertex */
	if (device.immediate_active && (emit || reg == 0))
		immediate_emit();
}

void WINAPI D3DDevice_SetVertexData2f(INT reg, FLOAT a, FLOAT b)
{
	set_attribute(reg, a, b, 0.0f, 1.0f);
}

void WINAPI D3DDevice_SetVertexData4f(INT reg, FLOAT a, FLOAT b, FLOAT c, FLOAT d)
{
	set_attribute(reg, a, b, c, d);
}

void WINAPI D3DDevice_SetVertexData2s(INT reg, SHORT a, SHORT b)
{
	set_attribute(reg, (float)a, (float)b, 0.0f, 1.0f);
}

void WINAPI D3DDevice_SetVertexData4ub(INT reg, BYTE a, BYTE b, BYTE c, BYTE d)
{
	set_attribute(reg, a / 255.0f, b / 255.0f, c / 255.0f, d / 255.0f);
}

void WINAPI D3DDevice_SetVertexDataColor(INT reg, D3DCOLOR color)
{
	float value[4];

	color_to_vec4(color, value);
	set_attribute(reg, value[0], value[1], value[2], value[3]);
}

/* ---------- clearing */

/* the state a clear changed, no longer what the shadows say: the masks it
cleared through, the scissor (left disabled) and its rectangle. The rest
of the state is as it was, so the next draw sets only these again (all of
it again, at every clear, was hundreds of calls a frame) */
static void clear_state_forget(void)
{
	gl_state.color_mask = 0xff;
	gl_state.depth_mask = 0xff;
	gl_state.stencil_write_mask = (GLuint)-1;
	gl_state.scissor_test = 0;
	memset(gl_state.scissor, 0xff, sizeof(gl_state.scissor));
}

void WINAPI D3DDevice_Clear(DWORD count, CONST D3DRECT *rectangles, DWORD flags, D3DCOLOR color, float z, DWORD stencil)
{
	float rgba[4];
	GLbitfield mask = 0;
	BOOL has_depth = FALSE;
	DWORD index;

	if (!device.gl_ready || !bind_targets(&has_depth))
		return;
	if (trace_frame())
		platform_log("clear flags %lx color %08lx z %g count %lu target %08lx depth %08lx", (unsigned long)flags,
			(unsigned long)color, z, (unsigned long)count,
			device.render_target ? (unsigned long)device.render_target->Data : 0,
			device.depth_stencil ? (unsigned long)device.depth_stencil->Data : 0);
	stats.clears++;
	color_to_vec4(color, rgba);
	if (flags & D3DCLEAR_TARGET)
	{
		/* the Xbox clears the channels named (D3DCLEAR_TARGET_R, _G, _B, _A):
		the fog screen clears only alpha, leaving the picture under the fog */
		glColorMask((flags & D3DCLEAR_TARGET_R) != 0, (flags & D3DCLEAR_TARGET_G) != 0,
			(flags & D3DCLEAR_TARGET_B) != 0, (flags & D3DCLEAR_TARGET_A) != 0);
		glClearColor(rgba[0], rgba[1], rgba[2], rgba[3]);
		mask |= GL_COLOR_BUFFER_BIT;
	}
	if (has_depth && (flags & D3DCLEAR_ZBUFFER))
	{
		glDepthMask(GL_TRUE);
		glClearDepth(xgpu_reversed_depth ? 1.0f - z : z);
		mask |= GL_DEPTH_BUFFER_BIT;
	}
	if (has_depth && (flags & D3DCLEAR_STENCIL))
	{
		glStencilMask(0xff);
		glClearStencil((GLint)stencil);
		mask |= GL_STENCIL_BUFFER_BIT;
	}
	if (!mask)
		return;
	if (!count || !rectangles)
	{
		/* the NV2A clips a viewport-less clear to the viewport, which is what
		keeps a split-screen window's clear from wiping the other window */
		GLint x0 = target_pixel((float)device.viewport.X, 0);
		GLint y0 = target_pixel((float)device.viewport.Y, 1);

		glEnable(GL_SCISSOR_TEST);
		glScissor(x0, y0, target_pixel((float)(device.viewport.X + device.viewport.Width), 0) - x0,
			target_pixel((float)(device.viewport.Y + device.viewport.Height), 1) - y0);
		glClear(mask);
		glDisable(GL_SCISSOR_TEST);
		clear_state_forget();
		return;
	}
	glEnable(GL_SCISSOR_TEST);
	for (index = 0; index < count; index++)
	{
		INT left = rectangles[index].x1 > device.viewport.X ? rectangles[index].x1 : device.viewport.X;
		INT top = rectangles[index].y1 > device.viewport.Y ? rectangles[index].y1 : device.viewport.Y;
		INT right = rectangles[index].x2 < device.viewport.X + device.viewport.Width ?
			rectangles[index].x2 : device.viewport.X + device.viewport.Width;
		INT bottom = rectangles[index].y2 < device.viewport.Y + device.viewport.Height ?
			rectangles[index].y2 : device.viewport.Y + device.viewport.Height;
		GLint x0, y0;

		if (left >= right || top >= bottom)
			continue;
		x0 = target_pixel((float)(left + UI_OFFSET), 0);
		y0 = target_pixel((float)top, 1);
		glScissor(x0, y0, target_pixel((float)(right + UI_OFFSET), 0) - x0,
			target_pixel((float)bottom, 1) - y0);
		glClear(mask);
	}
	glDisable(GL_SCISSOR_TEST);
	clear_state_forget();
}

/* ---------- the anti-aliasing passes */

/* display.anti_aliasing's pass over a window's 3D view, before the HUD and
menus are drawn over it (source/render/render.c); the window's bounds in the
game's units of the screen */
void halo_screen_anti_alias(short x0, short y0, short x1, short y1)
{
	struct render_target_entry *target;
	GLint corners[4];
	int mode = anti_aliasing();

	if (!device.gl_ready || (mode != _anti_aliasing_fxaa && mode != _anti_aliasing_smaa))
		return;
	/* (the primary target's view: the back buffer's) */
	target = render_target_get(&device.back_buffer);
	if (!target)
		return;
	/* (no longer multisampled, if it was before the setting changed) */
	render_target_multisample(&target->target, 0);
	corners[0] = scaled_pixel(x0, target->target.scale[0]);
	corners[1] = scaled_pixel(y0, target->target.scale[1]);
	corners[2] = scaled_pixel(x1, target->target.scale[0]);
	corners[3] = scaled_pixel(y1, target->target.scale[1]);
	xgpu_post_anti_alias(mode == _anti_aliasing_smaa, framebuffer_get(target->target.texture, 0),
		target->target.format, target->target.gl_width, target->target.gl_height, corners);
	glBindVertexArray(device.vertex_array);
	xgpu_gl_state_invalidate();
}

/* display.hdr's tone mapping of a window's 3D view, before the HUD and
menus are drawn over it, and before display.anti_aliasing's pass (which
then finds the edges the display shows); in place, into the multisampled
renderbuffer with multisampling */
void halo_screen_tone_map(short x0, short y0, short x1, short y1)
{
	struct render_target_entry *target;
	GLuint source, destination;
	GLint corners[4];

	if (!device.gl_ready || !hdr.active)
		return;
	target = render_target_get(&device.back_buffer);
	if (!target || target->target.format != GL_RGBA16F)
		return;
	corners[0] = scaled_pixel(x0, target->target.scale[0]);
	corners[1] = scaled_pixel(y0, target->target.scale[1]);
	corners[2] = scaled_pixel(x1, target->target.scale[0]);
	corners[3] = scaled_pixel(y1, target->target.scale[1]);
	render_target_resolve(&target->target);
	/* (both found first: making a framebuffer binds it) */
	source = framebuffer_get(target->target.texture, 0);
	destination = target->target.samples ? framebuffer_find(target->target.multisample, 0, TRUE) : source;
	if (xgpu_post_tone_map(source, target->target.texture, destination, target->target.gl_width,
		target->target.gl_height, corners, &hdr.tone_mapping) && target->target.samples)
	{
		target->target.unresolved = TRUE;
	}
	glBindVertexArray(device.vertex_array);
	xgpu_gl_state_invalidate();
}

/* ---------- presentation */

#ifndef HALO_GLES
/* the screenshot key's PNG (controls.screenshot; the Android guest has none
of the SDL calls it takes) */
static void write_key_screenshot(struct render_target_entry *target)
{
	char directory[512], path[640], timestamp[32], filename[64];
	SDL_Time now;
	SDL_DateTime date;
	SDL_Surface *surface = NULL;
	unsigned int collision;
	unsigned long width = target->target.gl_width, height = target->target.gl_height;
	GLint framebuffer, draw_framebuffer, pack_buffer, alignment, row_length, skip_rows, skip_pixels;
	GLenum error;
	unsigned long row, column;

	if (snprintf(directory, sizeof(directory), "%s/screenshots", platform_data_root()) >= (int)sizeof(directory))
	{
		SDL_SetError("Screenshot directory path is too long");
		goto failed;
	}
	if (!SDL_CreateDirectory(directory) || !SDL_GetCurrentTime(&now) || !SDL_TimeToDateTime(now, &date, true))
		goto failed;
	snprintf(timestamp, sizeof(timestamp), "%04d-%02d-%02d_%02d.%02d.%02d",
		date.year, date.month, date.day, date.hour, date.minute, date.second);
	for (collision = 0; collision < 1000; collision++)
	{
		if (collision)
			snprintf(filename, sizeof(filename), "%s_%u.png", timestamp, collision + 1);
		else
			snprintf(filename, sizeof(filename), "%s.png", timestamp);
		snprintf(path, sizeof(path), "%s/%s", directory, filename);
		if (!SDL_GetPathInfo(path, NULL))
			break;
	}
	if (collision == 1000)
	{
		SDL_SetError("Too many screenshots with the same timestamp");
		goto failed;
	}
	if (!width || !height || width > INT_MAX / 4 || height > INT_MAX)
	{
		SDL_SetError("Invalid screenshot dimensions");
		goto failed;
	}
	surface = SDL_CreateSurface((int)width, (int)height, SDL_PIXELFORMAT_BGRA32);
	if (!surface)
		goto failed;

	/* Read the render target before the display blit's vertical flip. Its
	row zero is the top of the image. Preserve the caller's readback state. */
	glGetIntegerv(GL_READ_FRAMEBUFFER_BINDING, &framebuffer);
	glGetIntegerv(GL_DRAW_FRAMEBUFFER_BINDING, &draw_framebuffer);
	glGetIntegerv(GL_PIXEL_PACK_BUFFER_BINDING, &pack_buffer);
	glGetIntegerv(GL_PACK_ALIGNMENT, &alignment);
	glGetIntegerv(GL_PACK_ROW_LENGTH, &row_length);
	glGetIntegerv(GL_PACK_SKIP_ROWS, &skip_rows);
	glGetIntegerv(GL_PACK_SKIP_PIXELS, &skip_pixels);
	glBindFramebuffer(GL_READ_FRAMEBUFFER, framebuffer_get(target->target.texture, 0));
	glBindBuffer(GL_PIXEL_PACK_BUFFER, 0);
	glPixelStorei(GL_PACK_ALIGNMENT, 4);
	glPixelStorei(GL_PACK_ROW_LENGTH, surface->pitch / 4);
	glPixelStorei(GL_PACK_SKIP_ROWS, 0);
	glPixelStorei(GL_PACK_SKIP_PIXELS, 0);
	glReadPixels(0, 0, (GLsizei)width, (GLsizei)height, GL_BGRA, GL_UNSIGNED_BYTE, surface->pixels);
	error = glGetError();
	glBindFramebuffer(GL_READ_FRAMEBUFFER, framebuffer);
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, draw_framebuffer);
	glBindBuffer(GL_PIXEL_PACK_BUFFER, pack_buffer);
	glPixelStorei(GL_PACK_ALIGNMENT, alignment);
	glPixelStorei(GL_PACK_ROW_LENGTH, row_length);
	glPixelStorei(GL_PACK_SKIP_ROWS, skip_rows);
	glPixelStorei(GL_PACK_SKIP_PIXELS, skip_pixels);
	if (error != GL_NO_ERROR)
	{
		SDL_SetError("OpenGL screenshot readback failed (%04x)", (unsigned)error);
		goto failed;
	}
	/* Destination alpha is scratch data in the game, not transparency. */
	for (row = 0; row < height; row++)
	{
		unsigned char *pixels = (unsigned char *)surface->pixels + (size_t)row * surface->pitch;

		for (column = 0; column < width; column++)
			pixels[column * 4 + 3] = 0xff;
	}
	if (!SDL_SavePNG(surface, path))
		goto failed;
	SDL_DestroySurface(surface);
	console_printf(FALSE, "Saved screenshot as %s", filename);
	return;

failed:
	console_printf(FALSE, "Screenshot failed: %.180s", SDL_GetError());
	if (surface)
		SDL_DestroySurface(surface);
}

#endif
static void write_screenshot(struct render_target_entry *target)
{
	const char *directory = *config_string("debug.screenshot_directory") ?
		config_string("debug.screenshot_directory") : NULL;
	unsigned long width = target->target.gl_width, height = target->target.gl_height;
	unsigned char *pixels;
	char path[512];
	FILE *file;
	unsigned long row;
	unsigned char header[54] = { 'B', 'M' };
	unsigned long image_size = width * height * 4;

	if (!directory)
		return;
	pixels = malloc(image_size);
	glBindFramebuffer(GL_READ_FRAMEBUFFER, framebuffer_get(target->target.texture, 0));
#ifdef HALO_GLES
	/* (OpenGL ES reads a half float target only as floats) */
	if (target->target.format == GL_RGBA16F)
	{
		float *texels = malloc(image_size * sizeof(float));

		glReadPixels(0, 0, (GLsizei)width, (GLsizei)height, GL_RGBA, GL_FLOAT, texels);
		for (row = 0; row < image_size; row++)
		{
			float texel = texels[row] < 0.0f ? 0.0f : texels[row] > 1.0f ? 1.0f : texels[row];

			pixels[row] = (unsigned char)(texel * 255.0f + 0.5f);
		}
		free(texels);
	}
	else
#endif
	glReadPixels(0, 0, (GLsizei)width, (GLsizei)height, GL_BGRA, GL_UNSIGNED_BYTE, pixels);
	/* the display ignores destination alpha, which the game uses as scratch;
	image viewers would show it as transparency */
	for (row = 0; row < width * height; row++)
	{
#ifdef HALO_GLES
		unsigned char red = pixels[row * 4];

		pixels[row * 4] = pixels[row * 4 + 2];
		pixels[row * 4 + 2] = red;
#endif
		pixels[row * 4 + 3] = 0xff;
	}
	snprintf(path, sizeof(path), "%s/frame%05lu.bmp", directory, device.frame);
	file = fopen(path, "wb");
	if (file)
	{
		*(unsigned int *)(header + 2) = (unsigned int)(54 + image_size);
		*(unsigned int *)(header + 10) = 54;
		*(unsigned int *)(header + 14) = 40;
		*(int *)(header + 18) = (int)width;
		*(int *)(header + 22) = -(int)height; /* rows from the top, as read */
		*(unsigned short *)(header + 26) = 1;
		*(unsigned short *)(header + 28) = 32;
		*(unsigned int *)(header + 34) = (unsigned int)image_size;
		fwrite(header, 1, sizeof(header), file);
		for (row = 0; row < height; row++)
			fwrite(pixels + row * width * 4, 1, width * 4, file);
		fclose(file);
	}
	free(pixels);
}

#ifndef HALO_GLES
/* the counts read from each query, as with AMD's driver, once a batch's
copies into the buffer have failed: a batch that fails is not used again,
and with the buffer the readback would stop once all of them had. The
batches still waiting keep their queries and are read from them. */
static void visibility_fall_back_to_queries(void)
{
	unsigned long i;

	for (i = 0; i < VISIBILITY_BATCH_COUNT; i++)
	{
		if (device.visibility_batches[i].fence)
			glDeleteSync(device.visibility_batches[i].fence);
		device.visibility_batches[i].fence = NULL;
	}
	device.visibility_mapped = NULL;
	platform_log("GPU: visibility test counts read from each query from now on");
}

/* this frame's batch sent, before the swap. A copy into the buffer right
after each test stopped the GPU until that test's draws were through,
hundreds of times a frame in The Library; the GPU copies a batch's counts
when its frame is presented, over the CPU's VISIBILITY_UNCOPIED, with a
fence after the copies. Nothing here waits for an answer from the driver:
with a threaded driver (NVIDIA's) any such call waits until its thread has
carried out the whole frame queued before it, and here, before the swap,
that held the frame back every time. GL errors are asked for with
debug.gl_debug; the swap, next, sends the commands to the GPU. */
static void visibility_send_batch(void)
{
	struct visibility_batch *batch;
	unsigned long i;

	if (device.visibility_unavailable || device.visibility_batch >= VISIBILITY_BATCH_COUNT)
		return;
	batch = &device.visibility_batches[device.visibility_batch];
	if (!batch->count)
		return;
	if (device.visibility_mapped)
	{
		volatile GLuint *counts = device.visibility_mapped + device.visibility_batch * VISIBILITY_TEST_SLOTS;

		if (device.visibility_logging)
			visibility_readback_gl_ok("before query copies");
		/* (the coherent mapping makes these writes visible to the copies
		sent after them; a place that still holds VISIBILITY_UNCOPIED
		once the fence has passed is a copy that failed) */
		for (i = 0; i < batch->count; i++)
			counts[i] = VISIBILITY_UNCOPIED;
		for (i = 0; i < batch->count; i++)
			glGetQueryBufferObjectuiv(batch->queries[i], device.visibility_buffer, GL_QUERY_RESULT,
				(GLintptr)((device.visibility_batch * VISIBILITY_TEST_SLOTS + i) * sizeof(GLuint)));
		batch->fence = glFenceSync(GL_SYNC_GPU_COMMANDS_COMPLETE, 0);
		if (!batch->fence || (device.visibility_logging && !visibility_readback_gl_ok("query copies")))
		{
			batch->failed = TRUE;
			device.visibility_stats.failures++;
			if (batch->fence)
				glDeleteSync(batch->fence);
			batch->fence = NULL;
			platform_log("GPU: visibility test batch %lu dropped: its counts could not be copied",
				device.visibility_batch);
			visibility_fall_back_to_queries();
		}
	}
	batch->sealed = TRUE;
	batch->serial = ++device.visibility_serial;
	batch->frame = device.frame;
	device.visibility_batch = VISIBILITY_BATCH_COUNT;
	device.visibility_stats.submitted++;
}

/* the counts of earlier frames' tests, read into their slots oldest batch
first once the fence after their copies has passed, after the swap, when
the driver has the least queued before the question; with AMD's driver the
CPU reads each query once it says it is available. A batch is not reused
until all its counts have been read; one the GPU is late with is waited for
at the next presents, however long (its slots keep their earlier counts
meanwhile, and with every batch waiting, tests are dropped). */
static void visibility_read_batch(void)
{
	struct visibility_batch *batch;
	unsigned long i, oldest, pending = 0, oldest_age = 0;
	GLint binding = 0;
	BOOL bound = TRUE, fall_back = FALSE;

	if (device.visibility_unavailable)
		return;
	/* (unbound: a query read with a query buffer bound takes its pointer for
	an offset into it) */
	if (!device.visibility_mapped)
	{
		visibility_readback_gl_ok("before readback");
		glGetIntegerv(GL_QUERY_BUFFER_BINDING, &binding);
		if (!visibility_readback_gl_ok("read binding"))
			return;
		glBindBuffer(GL_QUERY_BUFFER, 0);
		bound = visibility_readback_gl_ok("unbind query buffer");
	}
	while (bound)
	{
		oldest = VISIBILITY_BATCH_COUNT;
		for (i = 0; i < VISIBILITY_BATCH_COUNT; i++)
		{
			batch = &device.visibility_batches[i];
			if (batch->sealed && !batch->failed && (oldest == VISIBILITY_BATCH_COUNT ||
				batch->serial < device.visibility_batches[oldest].serial))
				oldest = i;
		}
		if (oldest == VISIBILITY_BATCH_COUNT)
			break;
		batch = &device.visibility_batches[oldest];
		/* (the batch this present sent: looked for from the next one) */
		if (batch->frame == device.frame)
			break;
		if (device.visibility_mapped && batch->fence)
		{
			/* (the flush makes sure the fence reaches the GPU without
			counting on the swap's; after the swap it has nothing to send) */
			GLenum status = glClientWaitSync(batch->fence, GL_SYNC_FLUSH_COMMANDS_BIT, 0);

			if (status == GL_TIMEOUT_EXPIRED)
			{
				device.visibility_stats.unavailable++;
				break;
			}
			if (status != GL_ALREADY_SIGNALED && status != GL_CONDITION_SATISFIED)
			{
				batch->failed = TRUE;
				device.visibility_stats.failures++;
				platform_log("GPU: visibility test batch %lu dropped: its fence failed", oldest);
			}
			glDeleteSync(batch->fence);
			batch->fence = NULL;
			if (batch->failed)
			{
				fall_back = TRUE;
				break;
			}
		}
		while (batch->read < batch->count)
		{
			unsigned long test = batch->read;
			GLuint available = 0, samples = 0;
			float area = batch->areas[test];

			if (device.visibility_mapped)
			{
				samples = device.visibility_mapped[oldest * VISIBILITY_TEST_SLOTS + test];
				if (samples == VISIBILITY_UNCOPIED)
				{
					batch->failed = TRUE;
					device.visibility_stats.failures++;
					break;
				}
			}
			else
			{
				glGetQueryObjectuiv(batch->queries[test], GL_QUERY_RESULT_AVAILABLE, &available);
				if (!visibility_readback_gl_ok("availability"))
				{
					batch->failed = TRUE;
					break;
				}
				if (!available)
				{
					device.visibility_stats.unavailable++;
					break;
				}
				glGetQueryObjectuiv(batch->queries[test], GL_QUERY_RESULT, &samples);
				if (!visibility_readback_gl_ok("result"))
				{
					batch->failed = TRUE;
					break;
				}
			}
			device.visibility_known[batch->slots[test]] = area > 1.0f ?
				(GLuint)(samples / area + 0.5f) : samples;
			device.visibility_counted[batch->slots[test]] = batch->tests[test];
			device.visibility_stats.samples++;
			device.visibility_stats.nonzero += samples != 0;
			if (samples > device.visibility_stats.maximum)
				device.visibility_stats.maximum = samples;
			batch->read++;
		}
		if (batch->failed)
		{
			/* (its slots keep their earlier counts, and its queries, whose
			state is unknown, are not used again) */
			platform_log("GPU: visibility test batch %lu dropped: its counts could not be read", oldest);
			if (device.visibility_mapped)
			{
				fall_back = TRUE;
				break;
			}
			continue;
		}
		if (batch->read < batch->count)
			break;
		device.visibility_stats.completed++;
		batch->count = batch->read = 0;
		batch->sealed = FALSE;
	}
	if (!device.visibility_mapped)
	{
		glBindBuffer(GL_QUERY_BUFFER, (GLuint)binding);
		visibility_readback_gl_ok("restore query buffer");
	}
	/* (after the reads from the buffer: those from each query unbind it
	first, from the next present) */
	if (fall_back)
		visibility_fall_back_to_queries();
	for (i = 0; i < VISIBILITY_BATCH_COUNT; i++)
	{
		batch = &device.visibility_batches[i];
		if (batch->sealed && !batch->failed)
		{
			pending++;
			if (device.frame - batch->frame > oldest_age)
				oldest_age = device.frame - batch->frame;
		}
		if (device.visibility_batch == VISIBILITY_BATCH_COUNT && !batch->sealed && !batch->failed)
			device.visibility_batch = i;
	}
	if (device.visibility_logging && device.frame % 120 == 0)
	{
		platform_log("GPU: visibility %s frame=%lu submitted=%lu completed=%lu pending=%lu oldest_age=%lu samples=%lu nonzero=%lu raw_max=%lu unavailable=%lu dropped=%lu failures=%lu",
			device.visibility_mapped ? "buffer" : "CPU", device.frame, device.visibility_stats.submitted, device.visibility_stats.completed, pending, oldest_age,
			device.visibility_stats.samples, device.visibility_stats.nonzero, device.visibility_stats.maximum,
			device.visibility_stats.unavailable, device.visibility_stats.dropped, device.visibility_stats.failures);
		memset(&device.visibility_stats, 0, sizeof(device.visibility_stats));
	}
}

#endif
/* ---------- the front buffer

The Xbox's front buffer is the frame on the screen, which the loading screen
reads back (GetBackBuffer with -1, progress_bar.c) and draws again blurred,
a little larger and fainter, under each new frame: the trails of its light.
Here it is a render target of the menus' 640x480, which takes the 640
columns in the middle of the frame (where the loading screen draws, as the
menus do on a wide screen) once the loading screen has drawn it, while the
screen asks for it: at its BlockUntilVerticalBlank, its last call, so that
what is drawn over it after (the download screen's words) leaves no trail. */

enum
{
	FRONT_BUFFER_WIDTH = 640,
	FRONT_BUFFER_HEIGHT = 480,
};

/* the front buffer, made the first time it is asked for, and black when the
frame before did not take its picture (a loading screen's first frame) */
static BOOL front_buffer_ready(void)
{
	/* (its memory could not be had: not asked for again each frame) */
	static BOOL failed;
	struct render_target_entry *front;

	if (!device.gl_ready || failed)
		return FALSE;
	if (!device.front_buffer.Data)
	{
		d3d8_surface_initialize(&device.front_buffer, D3DFMT_LIN_A8R8G8B8, FRONT_BUFFER_WIDTH, FRONT_BUFFER_HEIGHT);
		if (!device.front_buffer.Data)
		{
			failed = TRUE;
			return FALSE;
		}
	}
	front = render_target_get(&device.front_buffer);
	if (!front)
		return FALSE;
	if (device.front_buffer_taken != device.frame)
	{
		GLuint framebuffer;

		draw_flush();
		framebuffer = framebuffer_get(front->target.texture, 0);
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, framebuffer);
		glDisable(GL_SCISSOR_TEST);
		glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
		glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);
		gl_check_errors("front buffer clear");
		xgpu_gl_state_invalidate();
		/* (drawn into, as a draw marks a target: the latest of its memory's) */
		front->last_rendered = device.frame + 1;
		front->target.written = ++render_target_write_serial;
	}
	device.front_buffer_asked = device.frame + 1;
	return TRUE;
}

/* the frame as the loading screen has drawn it into the front buffer, when
this frame asked for it (D3DDevice_BlockUntilVerticalBlank) */
static void front_buffer_take(void)
{
	struct render_target_entry *back_buffer, *front;
	GLuint read, draw;
	GLint width, left;

	if (!device.gl_ready || !device.front_buffer.Data || device.front_buffer_asked != device.frame + 1)
		return;
	back_buffer = render_target_get(&device.back_buffer);
	front = render_target_get(&device.front_buffer);
	if (!back_buffer || !front)
		return;
	draw_flush();
	render_target_resolve(&back_buffer->target);
	read = framebuffer_get(back_buffer->target.texture, 0);
	draw = framebuffer_get(front->target.texture, 0);
	width = (GLint)(FRONT_BUFFER_WIDTH * back_buffer->target.scale[0] + 0.5f);
	left = ((GLint)back_buffer->target.gl_width - width) / 2;
	if (left < 0)
	{
		left = 0;
		width = (GLint)back_buffer->target.gl_width;
	}
	glBindFramebuffer(GL_READ_FRAMEBUFFER, read);
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, draw);
	glDisable(GL_SCISSOR_TEST);
	glBlitFramebuffer(left, 0, left + width, (GLint)back_buffer->target.gl_height, 0, 0,
		(GLint)front->target.gl_width, (GLint)front->target.gl_height, GL_COLOR_BUFFER_BIT, GL_LINEAR);
	gl_check_errors("front buffer copy");
	xgpu_gl_state_invalidate();
	front->last_rendered = device.frame + 1;
	front->target.written = ++render_target_write_serial;
	device.front_buffer_taken = device.frame + 1;
}

#ifndef HALO_GLES
/* after the swap: this frame's fence, then a wait for the frame
frame_pacing.limit before it (a second at most: a GPU that slow is not
waited for longer) */
static void frame_pacing_wait(void)
{
	GLsync *slot;

	if (!frame_pacing.limit)
		return;
	frame_pacing.fences[frame_pacing.next] = glFenceSync(GL_SYNC_GPU_COMMANDS_COMPLETE, 0);
	frame_pacing.next = (frame_pacing.next + 1) % frame_pacing.limit;
	slot = &frame_pacing.fences[frame_pacing.next];
	if (*slot)
	{
		glClientWaitSync(*slot, GL_SYNC_FLUSH_COMMANDS_BIT, 1000000000ull);
		glDeleteSync(*slot);
		*slot = NULL;
	}
}
#endif

void WINAPI D3DDevice_Present(CONST RECT *source_rectangle, CONST RECT *destination_rectangle,
	void *unused, void *unused2)
{
	static long screenshot_every = -1;

	(void)source_rectangle;
	(void)destination_rectangle;
	(void)unused;
	(void)unused2;
	if (screenshot_every < 0)
		screenshot_every = config_integer("debug.screenshot_every");

	if (device.gl_ready)
	{
		struct render_target_entry *back_buffer = render_target_get(&device.back_buffer);
		int window_width, window_height, width, height, x, y;

		if (trace_frame())
			platform_log("present back buffer %08lx texture %u", (unsigned long)device.back_buffer.Data,
				back_buffer->target.texture);
		render_target_resolve(&back_buffer->target);
#ifndef HALO_GLES
		visibility_send_batch();
#endif
		if (screenshot_every > 0 && device.frame % (unsigned long)screenshot_every == 0)
			write_screenshot(back_buffer);
#ifndef HALO_GLES
		if (platform_screenshot_take_request())
			write_key_screenshot(back_buffer);
#endif

		platform_video_drawable_size(&window_width, &window_height);
		/* letterbox to the back buffer's aspect ratio */
		width = window_width;
		height = (int)((long)window_width * back_buffer->target.gl_height / back_buffer->target.gl_width);
		if (height > window_height)
		{
			height = window_height;
			width = (int)((long)window_height * back_buffer->target.gl_width / back_buffer->target.gl_height);
		}
		x = (window_width - width) / 2;
		y = (window_height - height) / 2;
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
		glDisable(GL_SCISSOR_TEST);
		glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
		glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);
		/* (display.hdr's back buffer, or display.hdr_output's window, by a
		shader: a blit neither makes the pixels linear nor, on OpenGL ES,
		converts half floats) */
		if (!((back_buffer->target.format == GL_RGBA16F || hdr.float_window) &&
			xgpu_post_present(back_buffer->target.texture, x, y, width, height,
				hdr.float_window ? hdr.sdr_white : 0.0f)))
		{
			glBindFramebuffer(GL_READ_FRAMEBUFFER, framebuffer_get(back_buffer->target.texture, 0));
			/* row 0 of the render target is the top of the picture */
			glBlitFramebuffer(0, 0, (GLint)back_buffer->target.gl_width, (GLint)back_buffer->target.gl_height,
				x, y + height, x + width, y, GL_COLOR_BUFFER_BIT, GL_LINEAR);
		}
		/* (the HDR calibration screen's pattern, brighter than the back
		buffer's white can be) */
		if (hdr_calibration_shown(FALSE) && hdr.display_on)
		{
			int window[4];

			window[0] = x;
			window[1] = y;
			window[2] = width;
			window[3] = height;
			hdr_calibration_draw(back_buffer, window);
		}
		glBindVertexArray(device.vertex_array);
		glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);
		platform_video_swap();
#ifndef HALO_GLES
		frame_pacing_wait();
		visibility_read_batch();
#endif
		xgpu_gl_state_invalidate();
		memory_watch_begin_frame();
		xgpu_texture_cache_begin_frame();
#ifdef HALO_GLES
		if (xgpu_capabilities.atomic_counters)
		{
			/* this frame's counts, for when the GPU is done with it (the
			barrier makes the shaders' counter writes visible to the copy,
			which ES 3.1 does not promise without one) */
			glMemoryBarrier(GL_BUFFER_UPDATE_BARRIER_BIT);
			glBindBuffer(GL_COPY_READ_BUFFER, device.visibility_counters);
			glBindBuffer(GL_COPY_WRITE_BUFFER, device.counter_snapshots[device.buffer_ring]);
			glCopyBufferSubData(GL_COPY_READ_BUFFER, GL_COPY_WRITE_BUFFER, 0, 0,
				VISIBILITY_TEST_SLOTS * sizeof(GLuint));
			glBindBuffer(GL_COPY_READ_BUFFER, 0);
			glBindBuffer(GL_COPY_WRITE_BUFFER, 0);
		}
#ifdef HALO_ANDROID
		static_buffers_collect();
#endif
		host_gl_fence_frame((unsigned int)device.buffer_ring);
		device.buffer_ring = (device.buffer_ring + 1) % STREAM_BUFFER_RING;
		host_gl_wait_frame((unsigned int)device.buffer_ring);
		if (xgpu_capabilities.atomic_counters && device.ring_test_count[device.buffer_ring])
		{
			unsigned long ring = device.buffer_ring;
			unsigned long test;

			/* the GPU has passed that frame's fence: its copy is complete,
			and the slot is free for this frame's tests */
			host_gl_read_buffer(device.counter_snapshots[ring], 0, VISIBILITY_TEST_SLOTS * sizeof(GLuint),
				device.counter_values);
			for (test = 0; test < device.ring_test_count[ring]; test++)
			{
				device.visibility_latest[device.ring_tests[ring][test][0]] =
					device.counter_values[device.ring_tests[ring][test][1]];
				device.visibility_counted[device.ring_tests[ring][test][0]] =
					device.ring_test_numbers[ring][test];
			}
			device.ring_test_count[ring] = 0;
		}
		device.stream_buffer = device.stream_buffers[device.buffer_ring];
		device.index_buffer = device.index_buffers[device.buffer_ring];
		device.stream_offset = 0;
		device.index_offset = 0;
		web_chunks_begin_frame();
#else
		device.stream_offset = STREAM_BUFFER_SIZE; /* orphan next frame */
		device.index_offset = INDEX_BUFFER_SIZE;
#endif
	}
	device.frame++;
	stats.presents++;
	if (debug_settings.statistics)
	{
		static struct timespec last_present;
		struct timespec now;

		clock_gettime(CLOCK_MONOTONIC, &now);
		if (last_present.tv_sec)
		{
			unsigned long milliseconds = (unsigned long)((now.tv_sec - last_present.tv_sec) * 1000 +
				(now.tv_nsec - last_present.tv_nsec) / 1000000);

			if (milliseconds > stats.slowest_frame)
				stats.slowest_frame = milliseconds;
		}
		last_present = now;
		if (stats.draws - stats.frame_first_draw > stats.most_draws)
			stats.most_draws = stats.draws - stats.frame_first_draw;
		stats.frame_first_draw = stats.draws;
	}
	if (debug_settings.statistics && device.frame % 60 == 0)
	{
		platform_log("frame %lu: %lu draws, %lu immediate, %lu clears, %lu target changes; skipped %lu no program, %lu no target, %lu link; "
			"%lu KB mirrored, %lu KB streamed; in 60 frames, %lu textures uploaded (%lu KB), at most %lu draws "
			"and %lu ms in a frame",
			device.frame, stats.draws / stats.presents, stats.immediate_draws / stats.presents, stats.clears / stats.presents,
			stats.target_changes / stats.presents, stats.skipped_no_program, stats.skipped_no_target, stats.skipped_link,
			stats.mirrored_bytes / stats.presents / 1024, stats.streamed_bytes / stats.presents / 1024,
			stats.texture_uploads, stats.texture_upload_bytes / 1024, stats.most_draws, stats.slowest_frame);
		memset(&stats, 0, sizeof(stats));
#ifndef HALO_GLES
		{
			static unsigned long shown = ~0UL;

			if (program_cache.loaded + program_cache.linked != shown)
			{
				shown = program_cache.loaded + program_cache.linked;
				platform_log("programs: %lu from the shader cache, %lu linked (%lu written to it, %lu refused by the driver)",
					program_cache.loaded, program_cache.linked, program_cache.written, program_cache.refused);
			}
		}
#endif
	}
	platform_pump_events();

	pthread_mutex_lock(&vertical_blank_lock);
	/* the Xbox keeps at most two frames queued behind its 60 Hz display;
	with interpolation, frames come at the real display's rate instead,
	paced by vsync (platform_video_swap) */
	if (halo_interpolation_enabled())
	{
		flip_count++;
	}
	else
	{
		while (pending_flips >= 2)
			pthread_cond_wait(&vertical_blank_condition, &vertical_blank_lock);
		pending_flips++;
	}
	pthread_mutex_unlock(&vertical_blank_lock);
}

HRESULT WINAPI D3DDevice_PersistDisplay(void)
{
	return S_OK;
}
