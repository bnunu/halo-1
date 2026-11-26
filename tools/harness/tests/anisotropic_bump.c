/*
ANISOTROPIC_BUMP.C (test)

The real configure_sampler (d3d8_gl.c): display.anisotropic_filtering is
given to the maps' textures with mip levels that the game filters linearly,
but not to a palettized (P8) texture, which in the Xbox maps is only ever a
bump map: its finer levels, sharpened at a grazing angle, speckled the
Covenant and Forerunner floors with single bright pixels. Nor to a render
target, as before.
test_anisotropic_bump.py takes the code from the sources (under_test.inc).
*/

#include "harness.h"

typedef unsigned int GLuint;
typedef int BOOL;
typedef unsigned long DWORD;

enum
{
	D3DTEXF_NONE = 0,
	D3DTEXF_POINT,
	D3DTEXF_LINEAR,
	D3DTEXF_ANISOTROPIC
};

enum
{
	D3DTSS_ADDRESSU = 0,
	D3DTSS_ADDRESSV,
	D3DTSS_ADDRESSW,
	D3DTSS_MAGFILTER,
	D3DTSS_MINFILTER,
	D3DTSS_MIPFILTER,
	D3DTSS_MIPMAPLODBIAS,
	D3DTSS_MAXMIPLEVEL,
	D3DTSS_MAXANISOTROPY,
	D3DTSS_BORDERCOLOR,
	D3DTSS_MAX
};

#define D3DTSS_MAXSTAGES 4

#include "config.inc"

static DWORD D3D__TextureState[D3DTSS_MAXSTAGES][D3DTSS_MAX];
static DWORD test_anisotropy = 16;
static DWORD test_inputs[SAMPLER_STATE_WORDS];
static GLuint test_next_sampler = 1;

static DWORD anisotropic_filtering(void)
{
	return test_anisotropy;
}

/* a new sampler for each state, its inputs kept */
static GLuint sampler_get(int stage, const DWORD *inputs)
{
	(void)stage;
	memcpy(test_inputs, inputs, sizeof(test_inputs));
	return test_next_sampler++;
}

static void state_sampler(int stage, GLuint sampler)
{
	(void)stage;
	(void)sampler;
}

#include "under_test.inc"

/* the game's state for a map's texture: filtered linearly, with mip levels */
static void stage_reset(int stage)
{
	memset(D3D__TextureState[stage], 0, sizeof(D3D__TextureState[stage]));
	D3D__TextureState[stage][D3DTSS_MINFILTER] = D3DTEXF_LINEAR;
	D3D__TextureState[stage][D3DTSS_MAGFILTER] = D3DTEXF_LINEAR;
	D3D__TextureState[stage][D3DTSS_MIPFILTER] = D3DTEXF_LINEAR;
	memset(test_inputs, 0xff, sizeof(test_inputs));
}

int main(int argc, char **argv)
{
	char const *case_name = argc > 1 ? argv[1] : "";

	/* a base or detail map: sharpened */
	CASE("base map")
	{
		stage_reset(1);
		configure_sampler(1, TRUE, FALSE, FALSE, FALSE);
		CHECK(test_inputs[0] == D3DTEXF_ANISOTROPIC && test_inputs[8] == 16, "filter %lu, anisotropy %lu",
			test_inputs[0], test_inputs[8]);
		return 0;
	}
	/* a bump map (P8): as the game filters it */
	CASE("bump map")
	{
		stage_reset(0);
		configure_sampler(0, TRUE, FALSE, FALSE, TRUE);
		CHECK(test_inputs[0] == D3DTEXF_LINEAR && test_inputs[8] == 0, "filter %lu, anisotropy %lu",
			test_inputs[0], test_inputs[8]);
		/* the same stage given a base map next: sharpened again */
		configure_sampler(0, TRUE, FALSE, FALSE, FALSE);
		CHECK(test_inputs[0] == D3DTEXF_ANISOTROPIC && test_inputs[8] == 16, "after it: filter %lu, anisotropy %lu",
			test_inputs[0], test_inputs[8]);
		return 0;
	}
	/* a render target, a texture with no mip levels, or the setting off:
	as the game filters it */
	CASE("as before")
	{
		stage_reset(2);
		configure_sampler(2, TRUE, FALSE, TRUE, FALSE);
		CHECK(test_inputs[0] == D3DTEXF_LINEAR, "a render target's filter %lu", test_inputs[0]);
		stage_reset(2);
		configure_sampler(2, FALSE, FALSE, FALSE, FALSE);
		CHECK(test_inputs[0] == D3DTEXF_LINEAR, "no mip levels: filter %lu", test_inputs[0]);
		stage_reset(2);
		test_anisotropy = 1;
		configure_sampler(2, TRUE, FALSE, FALSE, FALSE);
		CHECK(test_inputs[0] == D3DTEXF_LINEAR, "the setting off: filter %lu", test_inputs[0]);
		return 0;
	}

	return 2;
}
