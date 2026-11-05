/*
XGPU.H

Internals shared by the OpenGL implementation of the Xbox Direct3D API:
the NV2A shader translators (nv2a_vsh.c, nv2a_psh.c), texture decoding
(xbox_textures.c), guest memory write tracking (memory_watch.c) and the
device itself (d3d8_gl.c).
*/

#ifndef __HALO_LINUX_XGPU_H
#define __HALO_LINUX_XGPU_H

#include "platform.h"
#include "gl.h"

/* display.reversed_depth (desktop GL, taken up as the context is made): the
depth buffers hold 32-bit floats, 1.0 at the near plane and 0.0 at the far
(d3d8_gl.c turns the game's Direct3D depth around: its tests, clears, depth
range and polygon offsets; nv2a_vsh.c and nv2a_psh.c the depth they give).
A float's precision follows its size, so reversed it is about the same at
every distance, where 24 bits and Direct3D's depth spend most of theirs near
the camera: surfaces a few millimetres apart, which 24 bits run together
past a few tens of metres (Blood Gulch's lights on the base), stay apart. */
extern BOOL xgpu_reversed_depth;

#ifdef HALO_GLES
/* OpenGL ES features that are optional (d3d8_gl.c gl_initialize) */
struct xgpu_capabilities
{
	BOOL copy_image;
	BOOL border_clamp;
	BOOL anisotropy;
	BOOL s3tc;
	/* KHR_texture_compression_astc_ldr: DXT1 as ASTC where there is no S3TC
	(display.dxt1_astc) */
	BOOL astc;
	/* ES 3.2, or the EXT/OES_draw_elements_base_vertex extension: glDrawElementsBaseVertex */
	BOOL base_vertex;
	/* ES 3.1 with fragment atomic counters: exact visibility test counts */
	BOOL atomic_counters;
	/* GL_RGBA16F can be drawn into and blended (EXT_color_buffer_half_float
	or EXT_color_buffer_float): display.hdr's back buffer */
	BOOL color_buffer_half_float;
	/* "300 es" or "310 es" */
	const char *shading_language;
};

extern struct xgpu_capabilities xgpu_capabilities;

/* port/android/guest/runtime/guest_host.h */
int host_gl_has_extension(const char *name);
void host_gl_read_buffer(unsigned int buffer, unsigned int offset, unsigned int size, void *data);
void host_gl_buffer_write(unsigned int target, unsigned int offset, unsigned int size, const void *data);
void host_gl_fence_frame(unsigned int slot);
void host_gl_wait_frame(unsigned int slot);
#endif

/* ---------- GL state

The device caches the GL state it sets for draws (d3d8_gl.c); code that
changes GL state behind it (binding a texture to upload it, deleting one)
must call this afterwards. */

void xgpu_gl_state_invalidate(void);
/* whether there is an OpenGL context (not with debug.null_renderer, a
dedicated server's): textures are made only then */
int xgpu_gl_ready(void);
/* a texture about to be deleted: the binding shadows forget it, since GL
may give its name to a new texture (call before glDeleteTextures) */
void xgpu_gl_texture_deleted(unsigned int texture);

/* a compiled shader, or a linked program of two, or 0 with the log
written */
GLuint xgpu_compile_shader(GLenum type, const char *code, const char *what);
GLuint xgpu_link_program(GLuint vertex_shader, GLuint fragment_shader, const char *what);

/* ---------- generated source text */

struct xgpu_text
{
	char *buffer;
	unsigned long length;
	unsigned long capacity;
};

void xgpu_text_append(struct xgpu_text *text, const char *format, ...) __attribute__((format(printf, 2, 3)));

/* ---------- vertex shaders */

#define XGPU_VERTEX_ATTRIBUTE_COUNT 16
#define XGPU_VERTEX_CONSTANT_COUNT 192
/* D3D constant register -96 is hardware register 0 */
#define XGPU_VERTEX_CONSTANT_BIAS 96

/* where one of the game's model lighting programs (d3d8_gl.c
halo_vertex_shader_lighting) has the normal, and the world position, that it
lights the diffuse color by: the temporary register that holds each before
the instruction given */
struct nv2a_vertex_lighting
{
	/* 1 by the ambient and distant lights, 2 by the point lights too */
	int lights;
	unsigned long normal_instruction, normal_register;
	/* (with the point lights only) */
	unsigned long position_instruction, position_register;
};

/* finds them in a model lighting program, checking that it lights its
diffuse color as nv2a_psh.c does for each pixel; FALSE if it does not */
BOOL nv2a_vertex_shader_lighting(const DWORD *instructions, unsigned long instruction_count,
	struct nv2a_vertex_lighting *lighting);

/* GLSL for an NV2A vertex program (the instruction words after the program
header). Attributes whose bit is set in packed_attribute_mask are fed as
NORMPACKED3 32-bit integers and unpacked in the shader. With lighting (else
NULL), the normal and world position go to the pixel shader too, which
lights the diffuse color for each pixel (nv2a_pixel_shader_key
per_pixel_lighting). Returns a malloc'd string. */
char *nv2a_vertex_shader_to_glsl(const DWORD *instructions, unsigned long instruction_count,
	unsigned long packed_attribute_mask, const struct nv2a_vertex_lighting *lighting);
/* the fixed-function pipeline's vertex shader (the loading screen's draws):
positions already in clip space. Returns a malloc'd string. */
char *nv2a_fixed_function_glsl(void);

/* ---------- pixel shaders */

enum
{
	_xgpu_sampler_none = 0,
	_xgpu_sampler_2d,
	_xgpu_sampler_3d,
	_xgpu_sampler_cube,
};

/* everything a translated pixel shader depends on; the GLSL program cache
is keyed by these bytes */
struct nv2a_pixel_shader_key
{
	DWORD combiner_state[D3DRS_PS_MAX];
	/* D3DRS_PSTEXTUREMODES lies past D3DRS_PS_MAX */
	DWORD texture_modes;
	unsigned char sampler_type[4];
	unsigned char alpha_kill[4];
	/* D3DTSS_COLORSIGN: channels (bit 0 alpha ... bit 3 blue, as
	D3DTSIGN_*) that hold signed data in an unsigned texture format */
	unsigned char color_sign[4];
	/* D3DCMP_* function for the alpha test, or 0 when disabled */
	unsigned long alpha_test_function;
	unsigned char fog_enable;
	unsigned char fog_table_mode;
	/* inside a visibility test: count the samples that pass (Android) */
	unsigned char count_samples;
	/* a high-res HUD meter (hud_hires.h) drawn with the meter's blend (the
	destination kept by the source's alpha): that alpha is eased to 1 by the
	coverage texture 0's green holds, so that the meter darkens what is
	behind it only where it covers it (the Xbox's point-sampled meters stop
	at their texels' edges; filtered ones have a fringe of faint texels) */
	unsigned char coverage_alpha;
	/* Discrete meter thresholds in texture 0's red are read at level zero,
	without filtering. Coverage/brightness still use the filtered lookup. */
	unsigned char point_threshold;
	/* A CE meter draw sampling a sheet also used as ordinary artwork. */
	unsigned char ce_meter_channels;
	/* a model lighting program's draw lit for each pixel
	(display.per_pixel_lighting): nv2a_vertex_lighting's lights, or 0 for
	the diffuse color the vertex shader computed */
	unsigned char per_pixel_lighting;
	/* drawn into a multisampled target (display.anti_aliasing's
	multisampling), its samples a pixel: the alpha test covers samples in
	proportion to how far alpha is past the reference, not all of the pixel
	or none of it, so that cut-out edges (foliage, grates) are smoothed too */
	unsigned char alpha_test_samples;
	/* the source's blend factor, applied here instead (1 the blend color, 2
	one less it, 3 its alpha, 4 one less that; 0 none): WebGL refuses a
	blend of a constant color factor with a constant alpha one, as the
	meters' (shader_transparent_meter) */
	unsigned char source_blend;
};

char *nv2a_pixel_shader_to_glsl(const struct nv2a_pixel_shader_key *key);

#ifdef HALO_GLES
/* ES samplers have no LOD bias of their own */
#define XGPU_PIXEL_UNIFORMS_ES "uniform vec4 texture_lod_bias;\n"
#else
#define XGPU_PIXEL_UNIFORMS_ES ""
#endif

/* the combiner registers that live in uniforms rather than in the program:
C0/C1 of each stage and the final combiner, and texture constants */
#define XGPU_PIXEL_UNIFORMS \
	"uniform vec4 ps_c0[8];\n" \
	"uniform vec4 ps_c1[8];\n" \
	"uniform vec4 ps_final_c0;\n" \
	"uniform vec4 ps_final_c1;\n" \
	"uniform vec4 fog_color;\n" \
	"uniform vec4 fog_parameters;\n" \
	"uniform float alpha_reference;\n" \
	"uniform vec4 bump_matrix[4];\n" \
	"uniform vec4 bump_luminance[4];\n" \
	"uniform vec4 texture_scale[4];\n" \
	XGPU_PIXEL_UNIFORMS_ES

/* the vertex constants the per-pixel model lighting reads, in a uniform of
their own (the vertex shader's 192 would pass OpenGL ES's least fragment
uniform space): [0] c[-82] (the translucency in z), [1] to [11] c[-79] to
c[-69] (rasterizer_set_model_lighting's two point lights, two distant
lights and the ambient light) */
#define XGPU_MODEL_LIGHT_COUNT 12

/* ---------- textures */

struct xgpu_texture_description
{
	DWORD format;       /* D3DFMT_* */
	unsigned long width, height, depth, levels;
	BOOL cube_map;
	BOOL linear;        /* not swizzled; addressed with texel coordinates */
	BOOL compressed;
	unsigned long pitch; /* linear textures */
	BOOL hires;         /* a high-res HUD texture drawn in the texture's place (hud_hires.h) */
	BOOL hires_coverage; /* ... whose green is its coverage (a meter's) */
	BOOL hires_point_threshold; /* ... whose red holds discrete segment data */
	BOOL ce_meter_channels;
};

void xgpu_texture_describe(DWORD format_word, DWORD size_word, struct xgpu_texture_description *description);
/* bytes of one face, mip levels included (cube faces are padded) */
unsigned long xgpu_texture_face_size(const struct xgpu_texture_description *description);
unsigned long xgpu_texture_level_offset(const struct xgpu_texture_description *description, unsigned long level);
unsigned long xgpu_texture_level_pitch(const struct xgpu_texture_description *description, unsigned long level);

/* the GL texture for an Xbox texture header, uploading or refreshing it
from guest memory as needed; *target receives GL_TEXTURE_2D etc. */
GLuint xgpu_texture_get(const DWORD *resource, const D3DCOLOR *palette, GLenum *target,
	struct xgpu_texture_description *description);
void xgpu_texture_cache_begin_frame(void);
/* (debug.gpu_stats) a texture of this many bytes uploaded to GL (d3d8_gl.c) */
void xgpu_statistics_texture_upload(unsigned long bytes);

/* ---------- render targets */

struct xgpu_render_target
{
	unsigned long data;  /* physical address */
	unsigned long width, height;
	BOOL depth;
	GLuint texture;
	/* the texture's (and the multisampled renderbuffer's) internal format:
	GL_RGBA8, GL_DEPTH24_STENCIL8, or GL_RGBA16F for the back buffer with
	display.hdr (d3d8_gl.c) */
	GLenum format;
	/* pixels per unit of width and height: more than 1 for the screen's
	targets when the game draws at the display's resolution (d3d8_gl.c) */
	float scale[2];
	unsigned long gl_width, gl_height;
	/* with multisampling, the multisampled renderbuffer draws go to, its
	samples a pixel (0 when it has none), and whether it has been drawn into
	since the texture last had its pixels (d3d8_gl.c,
	render_target_multisample) */
	GLuint multisample;
	int samples;
	BOOL unresolved;
	/* changes whenever the target is drawn into or cleared (d3d8_gl.c,
	bind_targets) */
	unsigned long written;
};

/* the GL texture holding a render target with this physical address, or 0 */
struct xgpu_render_target *xgpu_render_target_find(unsigned long data);

/* ---------- anti-aliasing

display.anti_aliasing's passes (xgpu_post.c): FXAA or SMAA antialias each
window's 3D view in place before the HUD and menus are drawn over it, so
that their text stays sharp. Supersampling and multisampling are the
device's (d3d8_gl.c). */

/* the programs and textures of FXAA, or of SMAA, made now; FALSE if they
cannot be (once FALSE, it stays so) */
BOOL xgpu_post_prepare(BOOL smaa);

/* FXAA, or SMAA, on the corners x0, y0 to x1, y1 (from row 0) of a render
target's framebuffer, width by height, of that internal format (GL_RGBA8, or
GL_RGBA16F with display.hdr); FALSE if its programs do not build */
BOOL xgpu_post_anti_alias(BOOL smaa, GLuint framebuffer, GLenum format, unsigned long width,
	unsigned long height, const GLint corners[4]);

/* ---------- high dynamic range

display.hdr (d3d8_gl.c): the back buffer is GL_RGBA16F, so that what the
game's blending adds up past white is kept rather than clipped, and each
window's 3D view is tone mapped here before the HUD and menus are drawn
over it. The back buffer's values stay the game's, gamma encoded, 1.0 the
display's white; past 1.0 only where an HDR display (display.hdr_output)
can show it. Presentation then draws the back buffer into the window with a
program of its own, which for an HDR display's scRGB framebuffer makes the
values linear. */

enum
{
	_xgpu_tone_mapping_neutral,
	_xgpu_tone_mapping_aces,
	_xgpu_tone_mapping_reinhard,
	_xgpu_tone_mapping_gt7,
	NUMBER_OF_XGPU_TONE_MAPPINGS
};

struct xgpu_tone_mapping
{
	int curve;
	/* the scale of the linear values before the curve */
	float exposure;
	/* the brightest value the curve gives, in the display's whites: 1.0 for
	SDR, an HDR display's headroom */
	float peak;
	/* with an HDR display, paper white in GT7's units (100 nits); 0 for SDR */
	float paper;
};

/* the tone mapping and presentation programs made now; FALSE if they
cannot be (once FALSE, it stays so) */
BOOL xgpu_post_prepare_hdr(void);

/* tone maps the corners (as xgpu_post_anti_alias's) of a GL_RGBA16F target:
reads its pixels from source_texture (source is its framebuffer), writes the
colour (not alpha) into destination (the same, or its multisampled
renderbuffer's) */
BOOL xgpu_post_tone_map(GLuint source, GLuint source_texture, GLuint destination, unsigned long width, unsigned long height,
	const GLint corners[4], const struct xgpu_tone_mapping *tone_mapping);

/* draws a GL_RGBA16F target's texture into the bound draw framebuffer's x,
y, width, height (the window's, row 0 at the bottom), turned the right way
up: as it is (SDR), or, with scale more than 0, made linear and scaled by
it (an HDR display's scRGB framebuffer: scale is its SDR white) */
BOOL xgpu_post_present(GLuint texture, int x, int y, int width, int height, float scale);

/* Android: a vertex (indices 0) or index (1) buffer resource whose size
bytes the game wrote once and will not write again, drawn from a GL buffer
of its own; and the end of one, when the game releases it (d3d8_gl.c) */
void xgpu_static_buffer_add(const void *resource, unsigned long size, int indices);
void xgpu_static_buffer_remove(const void *resource, int indices);
/* Android: an Xbox map's count vertex (indices 0) or index (1) buffers,
stride bytes apart, whose data lie from the first of them to end and are
not written while they are registered; and their end (d3d8_gl.c) */
void xgpu_static_buffers_add(const void *buffers, long count, unsigned long stride, int indices, const void *end);
void xgpu_static_buffers_remove(const void *buffers, long count, unsigned long stride, int indices);
/* Android: size bytes of vertices or indices from start, as above */
void xgpu_static_range_add(const void *start, unsigned long size, int indices);
void xgpu_static_range_remove(const void *start, int indices);
/* the visibility tests ended at an index so far, and the number (from 1) of
the one whose count GetVisibilityTestResult last gave there, 0 for none. A
test's count is read two or more frames after it, and never for one dropped
(no batch free), and until then the index gives an earlier test's: a lens
flare's slot is read only once a count of the flare's own tests has come
(rasterizer_lights.c; d3d8_gl.c) */
unsigned long long xgpu_visibility_tests_ended(unsigned long index);
unsigned long long xgpu_visibility_test_counted(unsigned long index);
/* fills the corners x0, y0 to x1, y1 of the window (row 0 at the bottom)
with color, as it is (an HDR display's scRGB framebuffer: linear, 1.0 at 80
nits), or only the ring ring[0], ring[1] (its centre in the window), from
radius ring[3] to ring[2]; ring[2] 0 for the whole rectangle. FALSE if its
program does not build. */
BOOL xgpu_post_fill(const GLint corners[4], const float color[3], const float ring[4]);

#endif
