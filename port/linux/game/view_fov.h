/*
VIEW_FOV.H

The first-person view's field of view and weapon (view_fov.c).
*/

#ifndef VIEW_FOV_H
#define VIEW_FOV_H

/* the vertical angle the local player's world view is drawn at (display.fov,
after the observer and before the projection and the visibility frustum
are built), from the game's own */
real render_fov_vertical(short local_player_index, real native_vertical_field_of_view);
/* how much the reticle shrinks or grows to stay on its aim at that view (the
last local view's own projection against the game's): 1 for the stock view */
real render_fov_reticle_scale(short local_player_index);
/* the view's own vertical angle when display.fov widened it, else 0 */
real render_fov_authored_vertical(short local_player_index);

/* the first-person weapon's projection (display.viewmodel_fov) around its
passes: balanced and nestable, the current local render view only */
void viewmodel_projection_begin(void);
void viewmodel_projection_end(void);

/* whether the first-person weapon, hands and attached visuals are drawn
(display.viewmodel_visible); its firing, sounds and lights go on either way */
boolean viewmodel_is_visible(void);
/* whether a pass draws: not one of the first-person weapon's that is hidden */
boolean viewmodel_draws_geometry(boolean first_person);

#endif
