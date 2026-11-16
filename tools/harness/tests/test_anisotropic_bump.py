"""The real configure_sampler (d3d8_gl.c): display.anisotropic_filtering sharpens the maps' textures with mip levels
that the game filters linearly, but not a palettized (P8) texture, which in the Xbox maps is only ever a bump map:
its finer levels at a grazing angle speckled the Covenant and Forerunner floors with single bright pixels that
multisampling cannot smooth. The texture's bind tells it which textures are P8."""

import sys
from pathlib import Path

import pytest

sys.path.insert(0, str(Path(__file__).resolve().parents[2]))
from harness import CHECK_FAILED, build, constant, function, mutated, read, run  # noqa: E402

CASES = ["base map", "bump map", "as before"]
SOURCE = "port/linux/src/d3d8_gl.c"

# a fault in the code and the case that must catch it
NEGATIVE_CONTROLS = {
    # a bump map sharpened as any other texture
    "bump-sharpened": (("!rendered && !palettized && ", "!rendered && "), "bump map"),
    # every texture kept from the setting
    "none-sharpened": (("&& anisotropic_filtering() > 1)", "&& anisotropic_filtering() > 1 && palettized)"),
                       "base map"),
    # a render target sharpened
    "target-sharpened": (("!rendered && !palettized && ", "!palettized && "), "as before"),
}


def generated(fault=None):
    source = read(SOURCE).replace("\r\n", "\n")
    config = "#define SAMPLER_STATE_WORDS %d\n" % constant(source, "SAMPLER_STATE_WORDS")
    code = function(source, "configure_sampler") + "\n"
    if fault:
        code = mutated(code, *fault)
    return (("config.inc", config), ("under_test.inc", code))


def test_wired():
    """A texture's bind tells configure_sampler whether it is P8."""
    source = read(SOURCE).replace("\r\n", "\n")
    assert "configure_sampler(stage, description.levels > 1, description.hires, target != NULL,\n" \
        "\t\t\t\tdescription.format == D3DFMT_P8);" in source


@pytest.mark.parametrize("case", CASES)
def test_case(case):
    status, output = run(build("anisotropic_bump", generated()), case)
    assert status == 0, output


@pytest.mark.parametrize("control", NEGATIVE_CONTROLS)
def test_negative_control(control):
    fault, case = NEGATIVE_CONTROLS[control]
    status, output = run(build("anisotropic_bump", generated(fault)), case)
    assert status == CHECK_FAILED, f"configure_sampler with a fault ({control}) passed '{case}'"
