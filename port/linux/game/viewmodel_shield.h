/*
VIEWMODEL_SHIELD.H

The unit's energy shield on its first-person arms (viewmodel_shield.c).
*/

#ifndef VIEWMODEL_SHIELD_H
#define VIEWMODEL_SHIELD_H

struct render_model_effect;

/* the hands' effect, before they are drawn (not the weapon's): with
display.viewmodel_shield, the unit's own modifier shader, with its colors
and a copy of its interpolated picture values that viewmodel_shield_offset_amount knows again */
void viewmodel_shield_apply(short local_player_index, long unit_index, long hands_model_index,
	real_matrix4x3 const *hands_node_matrices, real const *shown_functions,
	struct render_model_effect *effect);
/* the plasma shader's final displacement for a flare drawn with these values: on
the arms, at most what stands off them by a small share of the view;
anything else's, its own */
real viewmodel_shield_offset_amount(real const *values, real offset_amount);

#endif
