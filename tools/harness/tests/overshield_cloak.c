/*
OVERSHIELD_CLOAK.C (test)

The real overshield_effect_intensity (overshield_effect.c): the overshield,
as display.overshield_effect draws it, lights the body whole and the
first-person arms as far as its rise has reached them, and a unit's weapon
is drawn with its unit's values. Lit whole under active camouflage, it
showed a cloaked player's sniper rifle or rocket launcher where the weapon
reached past the body. It is as faint as the cloak is strong; under it, the
body still shows the shield's own flares (a hit), as the stock game does.
test_overshield_cloak.py takes the code from the sources (under_test.inc).
*/

#include "harness.h"
#include <math.h>

#define MAX(a, b) ((a) > (b) ? (a) : (b))

#include "under_test.inc"

static boolean near(real a, real b)
{
	return fabs(a - b) < 1e-5f;
}

int main(int argc, char **argv)
{
	char const *case_name = argc > 1 ? argv[1] : "";

	/* uncloaked: whole, as before */
	CASE("uncloaked")
	{
		CHECK(near(overshield_effect_intensity(0.f, 1.f, 1.f, FALSE), 1.f), "body not whole");
		CHECK(near(overshield_effect_intensity(0.3f, 1.f, 1.f, FALSE), 1.f), "body under a hit not whole");
		CHECK(near(overshield_effect_intensity(0.f, 0.6f, 1.f, TRUE), 0.6f), "arms not as far as the rise");
		CHECK(near(overshield_effect_intensity(0.9f, 0.6f, 1.f, TRUE), 0.6f), "arms with the shield's own flare");
		return 0;
	}
	/* cloaked whole: none, but the shield's own flare */
	CASE("cloaked")
	{
		CHECK(near(overshield_effect_intensity(0.f, 1.f, 0.f, FALSE), 0.f), "body lit through the cloak: %f",
			overshield_effect_intensity(0.f, 1.f, 0.f, FALSE));
		CHECK(near(overshield_effect_intensity(0.f, 1.f, 0.f, TRUE), 0.f), "arms lit through the cloak");
		CHECK(near(overshield_effect_intensity(0.4f, 1.f, 0.f, FALSE), 0.4f), "a hit not shown through the cloak");
		return 0;
	}
	/* a cloak coming on or broken by fire: as faint as it is strong */
	CASE("partly cloaked")
	{
		CHECK(near(overshield_effect_intensity(0.f, 1.f, 0.25f, FALSE), 0.25f), "body %f",
			overshield_effect_intensity(0.f, 1.f, 0.25f, FALSE));
		CHECK(near(overshield_effect_intensity(0.f, 0.8f, 0.5f, TRUE), 0.4f), "arms %f",
			overshield_effect_intensity(0.f, 0.8f, 0.5f, TRUE));
		CHECK(near(overshield_effect_intensity(0.6f, 1.f, 0.25f, FALSE), 0.6f), "a hit hidden by a weak cloak");
		return 0;
	}

	return 2;
}
