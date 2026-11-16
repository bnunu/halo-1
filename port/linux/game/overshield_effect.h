/* Shared presentation state for the overshield and its existing glow. */
#ifndef OVERSHIELD_EFFECT_H
#define OVERSHIELD_EFFECT_H

void overshield_effect_reset(void);
boolean overshield_effect_light_sample(long unit_index, real_rgb_color *color, real *coverage);

#endif
