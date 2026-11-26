"""The shaders of mods (mod_shaders.c's shader_make): Halo PC model and environment shader tag files made into the
game's shaders over the map's, checked as a map's are."""
import re
import sys
from pathlib import Path

import pytest

sys.path.insert(0, str(Path(__file__).resolve().parents[2]))
from harness import CHECK_FAILED, build, function, mutated, read, run

MOD_SHADERS = 'port/linux/game/mod_shaders.c'
MODS_FUNCTIONS = ('mods_read_be16', 'mods_read_be32', 'mods_read_short', 'mods_read_real', 'mods_in_file',
                  'mods_finite_real', 'mods_tag_file_header_valid')
SHADER_FUNCTIONS = ('write_word', 'write_real', 'read_definition_real', 'reference_read', 'shader_tool_values',
                    'shader_make')


def block(source, pattern):
    """a block of the file, from a line matching pattern to the "};" that ends it"""
    return re.search(r"^" + pattern + r".*?^\};\n", source, re.M | re.S).group(0)


def generated(fault=None):
    shaders = read(MOD_SHADERS)
    mods = read('port/linux/game/mods.c')
    mods_header = read('port/linux/game/mods.h')
    config = ''.join(line + '\n' for line in re.findall(r"^#define MOD_TAG_FILE_\w+ \w+$", mods_header, re.M))
    config += '#define NUMBER_OF_OBJECT_FUNCTION_REFERENCES 5\n'
    config += ''.join(line + '\n' for line in re.findall(r"^#define \w+_GROUP_TAG '\w+'$", shaders, re.M))
    config += ''.join(line + '\n' for line in re.findall(r"^#define (?:WORD|ENUM|REALS|BITMAP|REFERENCE)\(.*$", shaders,
                                                         re.M))
    code = '\n'.join([block(shaders, r"enum\n\{\n\t/\* both groups"), block(shaders, r"enum shader_field_kind"),
                      block(shaders, r"struct shader_field\n"), block(shaders, r"struct shader_group\n"),
                      block(shaders, r"static struct shader_field const shader_model_fields"),
                      block(shaders, r"static struct shader_field const shader_environment_fields"),
                      block(shaders, r"static struct shader_group const shader_groups")] +
                     [function(mods, name) for name in MODS_FUNCTIONS] +
                     [function(shaders, name) for name in SHADER_FUNCTIONS])
    if fault:
        code = mutated(code, *fault)
    return (('config.inc', config), ('under_test.inc', code))


@pytest.mark.parametrize('case', ['model', 'environment', 'refused'])
def test_case(case):
    status, output = run(build('mod_shaders', generated()), case)
    assert status == 0, output


@pytest.mark.parametrize('fault,case', [
    # the file's material (simulated) played over the map's
    (('memcpy(definition + SHADER_PHYSICS_FLAGS_OFFSET, map_definition + SHADER_PHYSICS_FLAGS_OFFSET,',
      'memcpy(definition + SHADER_PHYSICS_FLAGS_OFFSET, bytes + SHADER_PHYSICS_FLAGS_OFFSET,'), 'model'),
    # an enum taken past its values
    (('if (value < 0 || value >= field->maximum)', 'if (value < 0)'), 'refused'),
    # bytes after the last name taken
    (('if (cursor != size)', 'if (cursor > size)'), 'refused'),
    # a bitmap of the wrong kind taken
    (('if (!bitmap || bitmap->type != field->bitmap_type)', 'if (!bitmap)'), 'refused'),
    # the tool's bump map scale for x and y not worked out
    (('write_real(definition, SHADER_ENVIRONMENT_BUMP_MAP_SCALE_XY_OFFSET + 4, scale);', ''), 'environment'),
])
def test_negative_control(fault, case):
    status, output = run(build('mod_shaders', generated(fault)), case)
    assert status == CHECK_FAILED, output
