/*
VIEW_FOV.C

The first-person view's field of view (display.fov, display.viewmodel_fov)
and whether the weapon is drawn (display.viewmodel_visible), the local
render view only: the observer, its transitions and what the network sends
keep the game's own camera. Each angle is horizontal at 16:9 (the vertical
angle is kept, so other shapes of window widen or narrow as the stock view
does), and 0 keeps the stock view. Called from the game's camera
(main.c's set_window_camera_values), its HUD (hud_draw.c) and the passes
that draw the first-person weapon (rasterizer and render sources).
*/

#include "cseries.h"
#include "view_fov.h"
#include "camera/director.h"
#include "cutscene/cinematics.h"
#include "game/players.h"
#include "items/weapons.h"
#include "objects/objects.h"
#include "rasterizer/rasterizer.h"
#include "rasterizer/xbox/rasterizer_xbox_internal.h"
#include "render/render.h"
#include "render/render_cameras.h"
#include "render/render_cameras_internal.h"
#include "units/unit_definitions.h"
#include "units/units.h"

#include <math.h>

/* (the window's rasterizer target, as the rasterizer's sources number it) */
enum
{
	_rasterizer_target_render_primary = 0
};

double config_real(const char *name);
int config_boolean(const char *name);
unsigned long config_changes(void);

/* ---------- the world's field of view */

static real reticle_scales[MAXIMUM_LOCAL_PLAYERS];
static real authored_vertical[MAXIMUM_LOCAL_PLAYERS];

/* the view's vertical angle for display.fov: the game's own in a vehicle,
dead, in a cinematic, a scripted camera or a view the director holds, and
at a scope's full zoom; between, the zoom's own transition */
static real render_fov_adjust(short local_player_index, real native_vertical_field_of_view)
{
	static boolean initialized;
	static unsigned long read_at;
	static real requested_tangent;
	unsigned long changes = config_changes();
	long unit_index, weapon_index;
	struct unit_datum *unit;
	real unzoomed_angle, unzoomed_tangent, native_tangent, adjusted_tangent;

	if (!initialized || read_at != changes)
	{
		double degrees = config_real("display.fov");

		initialized = TRUE;
		read_at = changes;
		requested_tangent = degrees >= 20.0 && degrees <= 150.0 ?
			(real)(tan(degrees * _pi / 360.0) * (9.0 / 16.0)) : 0.0f;
	}
	if (!requested_tangent || local_player_index < 0 || local_player_index >= MAXIMUM_LOCAL_PLAYERS ||
		cinematic_in_progress() || (director_camera_scripted && *director_camera_scripted) ||
		director_get_perspective(local_player_index) != _director_perspective_first_person ||
		director_inhibited_facing(local_player_index))
	{
		return native_vertical_field_of_view;
	}
	unit_index = player_control_get_unit_index(local_player_index);
	if (unit_index == NONE)
		return native_vertical_field_of_view;
	unit = unit_try_and_get(unit_index);
	if (!unit || unit->object.parent_object_index != NONE || TEST_FLAG(unit->object.damage_flags, _object_dead_bit))
		return native_vertical_field_of_view;

	unzoomed_angle = unit_definition_get(unit->definition_index)->unit.camera_field_of_view;
	if (!(unzoomed_angle >= 0.001f && unzoomed_angle <= _pi / 2.0f) ||
		!(native_vertical_field_of_view > 0.0f && native_vertical_field_of_view < _pi))
	{
		return native_vertical_field_of_view;
	}
	unzoomed_tangent = 0.75f * render_camera_get_adjusted_field_of_view_tangent(unzoomed_angle);
	native_tangent = tanf(native_vertical_field_of_view * 0.5f);
	if (requested_tangent < unzoomed_tangent)
	{
		/* (a view narrower than the stock one: the zoom narrows it further) */
		adjusted_tangent = native_tangent * (requested_tangent / unzoomed_tangent);
	}
	else
	{
		real blend = 1.0f;

		weapon_index = unit_inventory_get_weapon(unit_index, unit->unit.current_weapon_index);
		if (weapon_index != NONE)
		{
			real scoped_angle = weapon_get_field_of_view(weapon_index, unzoomed_angle, 0);

			if (scoped_angle > 0.0f && scoped_angle < unzoomed_angle)
			{
				real scoped_tangent = 0.75f * render_camera_get_adjusted_field_of_view_tangent(scoped_angle);
				/* (a scope's full zoom keeps its stock angle exactly: the
				observer's arctangent and tangent can land just above it) */
				if (native_tangent <= scoped_tangent * 1.000001f)
					return native_vertical_field_of_view;

				/* (the extra width fades out along the scope's own
				transition, so every zoom level reached is the stock view) */
				blend = PIN((native_tangent - scoped_tangent) /
					(unzoomed_tangent - scoped_tangent), 0.0f, 1.0f);
			}
		}
		if (blend == 0.0f)
			return native_vertical_field_of_view;
		adjusted_tangent = native_tangent + (requested_tangent - unzoomed_tangent) * blend;
	}
	return 2.0f * atanf(adjusted_tangent);
}

real render_fov_vertical(short local_player_index, real native_vertical_field_of_view)
{
	real adjusted = render_fov_adjust(local_player_index, native_vertical_field_of_view);

	if (local_player_index >= 0 && local_player_index < MAXIMUM_LOCAL_PLAYERS)
	{
		real scale = 1.0f;

		if (adjusted != native_vertical_field_of_view &&
			native_vertical_field_of_view > 0.0f && native_vertical_field_of_view < _pi &&
			adjusted > 0.0f && adjusted < _pi)
		{
			scale = tanf(native_vertical_field_of_view * 0.5f) / tanf(adjusted * 0.5f);
			if (!(scale > 0.0f && scale < 1000.0f))
				scale = 1.0f;
		}
		/* (the view's projection, the zoom's transition too, for the HUD:
		the weapon's own projection, viewmodel_projection_begin, is not it) */
		reticle_scales[local_player_index] = scale;
		/* (a widened view's own angle, which the weapon keeps by default) */
		authored_vertical[local_player_index] =
			adjusted != native_vertical_field_of_view ? native_vertical_field_of_view : 0.0f;
	}
	return adjusted;
}

real render_fov_authored_vertical(short local_player_index)
{
	if (local_player_index < 0 || local_player_index >= MAXIMUM_LOCAL_PLAYERS)
		return 0.0f;
	return authored_vertical[local_player_index];
}

real render_fov_reticle_scale(short local_player_index)
{
	if (local_player_index < 0 || local_player_index >= MAXIMUM_LOCAL_PLAYERS ||
		reticle_scales[local_player_index] == 0.0f)
	{
		return 1.0f;
	}
	return reticle_scales[local_player_index];
}

/* ---------- the first-person weapon's */

/* how deep the weapon's passes nest, and whether the outermost changed the
view (the cameras and frustums it saved) */
static unsigned long projection_depth;
static boolean projection_applied;
static struct render_camera saved_render_camera, saved_rasterizer_camera;
static struct render_frustum saved_render_frustum, saved_rasterizer_frustum;

static real viewmodel_vertical(void)
{
	static boolean initialized;
	static unsigned long read_at;
	static real angle;
	unsigned long changes = config_changes();

	if (!initialized || read_at != changes)
	{
		double degrees = config_real("display.viewmodel_fov");

		angle = degrees >= 20.0 && degrees <= 150.0 ?
			(real)(2.0 * atan(tan(degrees * _pi / 360.0) * (9.0 / 16.0))) : 0.0f;
		initialized = TRUE;
		read_at = changes;
	}
	return angle;
}

void viewmodel_projection_begin(void)
{
	real angle;
	real_rectangle2d render_bounds, rasterizer_bounds;

	if (projection_depth++ != 0)
		return;
	projection_applied = FALSE;
	/* (display.viewmodel_fov's angle; by default the view's own, from before
	display.fov widened it: arms and a gun right against the camera stretch
	at a wide angle) */
	angle = viewmodel_vertical();
	if (!angle)
		angle = render_fov_authored_vertical(render.local_player_index);
	if (!angle || render.local_player_index < 0 || render.local_player_index >= MAXIMUM_LOCAL_PLAYERS ||
		global_window_parameters.rasterizer_target != _rasterizer_target_render_primary ||
		cinematic_in_progress() || (director_camera_scripted && *director_camera_scripted) ||
		director_get_perspective(render.local_player_index) != _director_perspective_first_person ||
		director_inhibited_facing(render.local_player_index))
	{
		return;
	}

	saved_render_camera = render.camera;
	saved_render_frustum = render.frustum;
	saved_rasterizer_camera = global_window_parameters.camera;
	saved_rasterizer_frustum = global_window_parameters.frustum;
	/* (the frustums' own bounds, the view's crop of a split screen: the
	visibility and the projection change together, the weapon's pose and
	markers do not; get_projection_bounds' slopes are not bounds) */
	render_bounds = saved_render_frustum.frustum_bounds;
	rasterizer_bounds = saved_rasterizer_frustum.frustum_bounds;
	render.camera.vertical_field_of_view = angle;
	global_window_parameters.camera.vertical_field_of_view = angle;
	render_camera_build_frustum(&render.camera, &render_bounds, &render.frustum, TRUE);
	render_camera_build_frustum(&global_window_parameters.camera, &rasterizer_bounds,
		&global_window_parameters.frustum, TRUE);
	projection_applied = TRUE;
	rasterizer_set_frustum_z(0.0f, 0.0f);
}

void viewmodel_projection_end(void)
{
	if (!projection_depth || --projection_depth || !projection_applied)
		return;
	render.camera = saved_render_camera;
	render.frustum = saved_render_frustum;
	global_window_parameters.camera = saved_rasterizer_camera;
	global_window_parameters.frustum = saved_rasterizer_frustum;
	projection_applied = FALSE;
	rasterizer_set_frustum_z(0.0f, 0.0f);
}

/* ---------- whether the first-person weapon is drawn */

boolean viewmodel_is_visible(void)
{
	static boolean initialized, visible;
	static unsigned long read_at;
	unsigned long changes = config_changes();

	if (!initialized || read_at != changes)
	{
		visible = config_boolean("display.viewmodel_visible") != 0;
		initialized = TRUE;
		read_at = changes;
	}
	return visible;
}

boolean viewmodel_draws_geometry(boolean first_person)
{
	return !first_person || viewmodel_is_visible();
}
