"""The real mixer of dsound_sdl.c on known signals (mixer.c): its resampler against the same low pass worked out in
double precision, sines through it as the ear hears them, voices out of earshot, mono voices and the linear
resampler.

    python tools/harness/tests/test_mixer.py metrics   the figures
    python tools/harness/tests/test_mixer.py bench [voices] [out of earshot] [indoor]   a game's worth of voices, timed
    python tools/harness/tests/test_mixer.py render > out.raw   noise through the mixer, to compare with another's
(MIXER_SOURCE=path: of another dsound_sdl.c; MIXER_SCALAR=1: its resampler adding tap by tap)
"""

import sys
from pathlib import Path

import pytest

sys.path.insert(0, str(Path(__file__).resolve().parents[2]))
from harness import CHECK_FAILED, build, mutated, read, run  # noqa: E402

SOURCE = "port/linux/src/dsound_sdl.c"
CASES = ["reference", "impulse", "sines", "aliases", "silent-voice", "mono", "linear", "cut", "cuts", "doppler"]

# a fault in the mixer, and the case that must catch it
NEGATIVE_CONTROLS = {
    "taps-off-by-one": (("(stream->center + 1 - RESAMPLER_ZERO_CROSSINGS) % RESAMPLER_HISTORY",
                         "(stream->center + 2 - RESAMPLER_ZERO_CROSSINGS) % RESAMPLER_HISTORY"), "reference"),
    "phase-not-blended": (("for (phase = 0; phase <= RESAMPLER_TABLE_STEPS; phase++)",
                           "for (phase = 0; phase <= RESAMPLER_TABLE_STEPS; phase += 2)"), "impulse"),
    "no-stretch": (("#define RESAMPLER_MAXIMUM_STRETCH 4", "#define RESAMPLER_MAXIMUM_STRETCH 1"), "aliases"),
    "cut-dead": (("mix_voice(stream, output, send, CUT_FADE_FRAMES);", "return;"), "cut"),
    "cut-not-faded": (("float fade = 0.5f + 0.5f * cosf(3.14159265f * (float)(frame + 1) / CUT_FADE_FRAMES);",
                       "float fade = 1.0f;"), "cut"),
    "cut-past-the-earlier": (("(frame < left ? stream->cut_send[stream->cut_played + frame] : 0.0f)",
                              "(frame < left ? 1.0f : 0.0f) * stream->cut_send[(stream->cut_played + frame) % CUT_FADE_FRAMES]"),
                             "cuts"),
    "linear-nearest": (("left_frames[0] + (left_frames[1] - left_frames[0]) * fraction", "left_frames[0]"), "linear"),
    "doppler-listener-backwards": (("ratio = (SPEED_OF_SOUND - away) / (SPEED_OF_SOUND - towards);",
                                    "ratio = (SPEED_OF_SOUND + away) / (SPEED_OF_SOUND - towards);"), "doppler"),
    "doppler-in-units": (("scale = listener.distance_factor * listener.doppler_factor / distance;",
                          "scale = listener.doppler_factor / distance;"), "doppler"),
    "doppler-held-to-an-octave": (("return ratio > 0.0f ? ratio : 0.0f;",
                                   "return ratio > 2.0f ? 2.0f : ratio < 0.5f ? 0.5f : ratio;"), "doppler"),
    "doppler-unlimited": (("#define VOICE_FREQUENCY_MAXIMUM 191983.0", "#define VOICE_FREQUENCY_MAXIMUM 1.0e9"),
                          "doppler"),
    "doppler-stereo-still": (("towards = stream->stereo_approach * listener.distance_factor",
                              "towards = 0.0f * listener.distance_factor"), "doppler"),
    "doppler-not-glided": (("\t\tstep += ramp_step;\n", "\t\tstep += 0.0 * ramp_step;\n"), "doppler"),
}


def generated(source=None, fault=None, scalar=False):
    """the mixer: dsound_sdl.c from its constants to the end of the mixing (before the output), its resampler adding
    tap by tap if scalar"""
    text = source if source is not None else read(SOURCE)
    code = text[text.index("#define OUTPUT_RATE"):text.index("/* ---------- output")]
    if fault:
        code = mutated(code, *fault)
    return (("under_test.inc", ("#define RESAMPLER_SCALAR\n" if scalar else "") + code),)


def rendered(scalar):
    """mixer.c's render: voices at each rate, then a scene's whole mix, as floats"""
    import array
    import subprocess

    result = subprocess.run([str(build("mixer", generated(scalar=scalar))), "render"], capture_output=True,
                            timeout=120, check=True)
    return array.array("f", result.stdout)


@pytest.mark.parametrize("case", CASES)
def test_case(case):
    status, output = run(build("mixer", generated()), case)
    assert status == 0, output


def test_vectors_add_as_taps_do():
    """the resampler four taps at a time is the resampler tap by tap but for the order of its additions: the same
    sound to float's rounding, through the whole mix (reverb and limiter)"""
    vector, scalar = rendered(False), rendered(True)
    assert len(vector) == len(scalar) > 0
    worst = max(abs(a - b) for a, b in zip(vector, scalar))
    assert 0 < worst < 1e-5, f"the vector resampler differs by {worst} (none: no vectors)"


@pytest.mark.parametrize("control", NEGATIVE_CONTROLS)
def test_negative_control(control):
    fault, case = NEGATIVE_CONTROLS[control]
    status, output = run(build("mixer", generated(fault=fault)), case)
    assert status == CHECK_FAILED, f"the mixer with a fault ({control}) passed '{case}': the test cannot see it"


if __name__ == "__main__":
    import os
    import subprocess

    # (MIXER_SOURCE: another dsound_sdl.c, to compare; MIXER_SCALAR=1: its resampler tap by tap)
    other = os.environ.get("MIXER_SOURCE")
    executable = build("mixer", generated(Path(other).read_text(encoding="latin-1") if other else None,
                                          scalar=bool(os.environ.get("MIXER_SCALAR"))))
    sys.exit(subprocess.run([str(executable), *(sys.argv[1:] or ["metrics"])]).returncode)
