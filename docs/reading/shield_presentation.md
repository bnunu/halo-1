# Arm shields and the visible overshield

These optional OpenCE effects change presentation. The existing shield charge,
damage, pickup and network simulation remain responsible for shield vitality.
`display.viewmodel_shield` and `display.overshield_effect` default to false.
The [settings table](../../port/linux/README.md) documents their environment
variables, Shield Color, Shield Glow and the host's `network.shield_effects`
policy. A multiplayer test host must choose an overshield policy or `players`
to see the effect; the default `stock` policy overrides the local setting.

## Reading order

1. [viewmodel_shield.c](../../port/linux/game/viewmodel_shield.c):
   `viewmodel_shield_apply` uses the unit's modifier shader and the interpolated
   function values supplied by the first-person renderer. A modifier shader
   is the additional material the engine draws over a model for effects such
   as its shield. Each local player's arms own a separate copy of these values.
2. [first_person_weapons.c](../../source/interface/first_person_weapons.c):
   `first_person_weapon_draw` calls the feature after remapping the hands'
   node matrices, between drawing the weapon and drawing the hands. The weapon
   keeps its existing material. The shared call is necessary because this is
   where the actual hands model, interpolated values and matrices are available.
3. [rasterizer_xbox_plasma_energy.c](../../source/rasterizer/xbox/rasterizer_xbox_plasma_energy.c):
   `rasterizer_plasma_energy_draw` asks `viewmodel_shield_offset_amount`
   to limit the final displacement, after applying the authored exponent.
   The limit is one percent of half the view height at the mean distance of
   the non-root arm nodes, using the first-person vertical field of view.
   Other models retain the authored displacement. This shared hook is needed
   because the final displacement is computed when queued geometry is drawn.
4. [overshield_effect.c](../../port/linux/game/overshield_effect.c):
   `overshield_effect_get` owns the visible charge state;
   `overshield_effect_arms_values` associates the arms' copy with that state;
   `overshield_effect_draw` controls the rise and slower energy animation;
   `overshield_effect_color` selects the remaining layer's color while retaining
   each shader channel's authored brightness.
5. [shield_glow.c](../../port/linux/game/shield_glow.c):
   `shield_glow_flare` samples the same overshield color and progress through
   `overshield_effect_light_sample`. Its existing exponent, camouflage
   attenuation, light allocation, range and host policy remain in place.

## Example and ownership

A player picking up an overshield at full ordinary shields sees a two-second
rise from feet to head. Arms enabled by Arm Shields light up as the rise
reaches the chest. The completed effect stays green while vitality exceeds
200 percent, then becomes reddish-pink for the remaining overshield through
200 percent. Energy moves at 30 percent of its authored speed. These colors
take precedence over Shield Color only while the overshield effect is active;
ordinary shields still use OpenCE's existing Shield Color implementation,
including its faint-gray rendering of black. Shield Glow follows the layer
when both effects are enabled.

The state belongs to this machine's presentation. It is not replicated and
does not change protocol data or gameplay. The existing host policy remains
in [network_shield_effects.c](../../port/linux/game/network_shield_effects.c).
The port retains the existing projectile hit-effect eligibility checks.
There is no Shield Look integration.

Model-height caches and associations with queued values are cleared by
`overshield_effect_reset`, wired to light disposal and new/old-map boundaries
in [object_lights.c](../../source/objects/object_lights.c). A feature module
cannot observe those boundaries on its own. These small calls prevent model
indices or draw identities from surviving into a different map. Disabling
the effect clears its state; death, exhaustion and backwards game time
invalidate active entries. Reassigning a local player's arms unregisters
their previous association. Xbox, Custom Edition and MCC keep their own
loaders and resource namespaces; these presentation hooks consume their
existing model and shader interfaces.

## Checks and limits

[The arm-shield harness](../../tools/harness/tests/test_viewmodel_shield.py)
compiles the production feature and rasterizer calculation with controlled
engine inputs. It covers view distance, field of view, final-exponent clamping,
unrelated geometry, per-view ownership, toggling, reassignment, Shield Color
and the actual hands-hook position; negative controls show that the checks
reject intentionally broken versions.
[Overshield rendering tests](../../tools/test_overshield_rendering.py) compile
the production state, draw, color and glow routines to check layer boundaries,
brightness, color precedence, rise/speed, reset and host-policy behavior.
These are source-level behavioral checks, not GPU or full-game validation.

For runtime validation, use unmodified maps and real pickups. Inspect arms
and bodies while charging, above and below 200 percent, after exhaustion,
and with each option disabled. Check ordinary Shield Color and Shield Glow,
multiple viewmodel fields of view, and Xbox/Custom Edition/MCC map transitions.
Use isolated saves and retain logs. A successful map load alone does not
establish complete map compatibility or multiplayer correctness.
