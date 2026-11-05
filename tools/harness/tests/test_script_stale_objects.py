"""A script holding a deleted object: the script runtime gives none, so script functions see no object and don't crash."""
import re
import sys
from pathlib import Path

import pytest

sys.path.insert(0, str(Path(__file__).resolve().parents[2]))
from harness import CHECK_FAILED, build, constant, enum_with, function, inline, mutated, read, run, structure

CASE_CHECK = 'value = hs_value_still_exists(\n\t\tactual_type==_hs_passthrough ? desired_type : actual_type,\n\t\tvalue);\n'
ARGUMENTS_CHECK = 'values[value_index] = hs_value_still_exists(\n\t\t\tformal_parameters[value_index],\n\t\t\tvalues[value_index]);\n'


def define(source, name):
    """A #define, with its continuation lines."""
    match = re.search(r'^#define\s+' + re.escape(name) + r'\b(?:[^\n]*\\\n)*[^\n]*', source, re.M)
    if not match:
        raise LookupError(f'define not found in the sources: {name}')
    return match.group(0) + '\n'


def evaluator(hs, name):
    """A script function's evaluator, as hs.c writes it with its macro."""
    match = re.search(r'^HS_EVALUATE_RETURN_BOOLEAN\(' + re.escape(name) + r'\b.*$', hs, re.M)
    if not match:
        raise LookupError(f'evaluator not found in the sources: {name}')
    return match.group(0) + '\n'


def generated(fault=None):
    runtime = read('source/hs/hs_runtime.c')
    lists = read('source/hs/object_lists.c')
    external = read('source/hs/hs_library_external.c')
    hs = read('source/hs/hs.c')
    hs_h = read('source/hs/hs.h')
    objects_h = read('source/objects/objects.h')
    objects = read('source/objects/objects.c')
    data = read('source/memory/data.c')
    references = read('source/objects/reference_lists.h')
    units = read('source/units/units.c')
    lists_h = read('source/hs/object_lists.h')

    types = f"#define TAG_STRING_LENGTH {constant(read('source/tag_files/tag_files.h'), 'TAG_STRING_LENGTH')}\n"
    types += structure(read('source/memory/data.h'), 'data_array') + '\n'
    types += structure(read('source/memory/data.h'), 'datum_header') + '\n'
    types += enum_with(read('source/objects/object_types.h'), '_object_mask_all') + '\n'
    types += enum_with(hs_h, '_hs_type_object') + '\n'
    for name in ('hs_type_valid', 'HS_TYPE_IS_OBJECT_NAME', 'HS_TYPE_IS_OBJECT'):
        types += define(hs_h, name)
    types += enum_with(runtime, '_hs_syntax_node_primitive_bit') + '\n'
    types += enum_with(runtime, 'HS_THREAD_STACK_SIZE') + '\n'
    types += enum_with(runtime, '_hs_thread_in_function_call_bit') + '\n'
    for name in ('hs_thread_get', 'hs_syntax_get', 'HS_GLOBAL_DESIGNATOR_IS_EXTERNAL', 'HS_GLOBAL_DESIGNATOR_TO_INDEX'):
        types += define(runtime, name)
    types += define(read('source/cseries/cseries.h'), 'SET_FLAG')
    types += define(read('source/cseries/cseries.h'), 'VALID_INDEX')
    types += 'struct object_datum;\n'
    types += structure(objects_h, 'object_header_datum') + '\n'
    for name in ('object_header_get', 'object_header_try_and_get', 'object_get', 'object_try_and_get'):
        types += define(objects_h, name)
    for name in ('unit_get', 'unit_try_and_get'):
        types += define(read('source/units/units.h'), name)
    for name in ('scenery_get', 'scenery_try_and_get'):
        types += define(read('source/objects/scenery.h'), name)
    types += structure(read('source/hs/hs_scenario_definitions.h'), 'hs_syntax_node') + '\n'
    types += 'typedef boolean (*hs_function_parse_proc)(short function_index, long expression_index);\n'
    types += 'typedef void (*hs_evaluate_procedure)(short function_index, long thread_index, boolean initialize);\n'
    types += structure(hs_h, 'hs_function_definition') + '\n'
    for name in ('hs_stack_frame', 'hs_global_datum', 'hs_thread_datum'):
        types += structure(runtime, name) + '\n'
    types += structure(references, 'data_reference') + '\n'
    types += structure(lists_h, 'object_list_header_datum') + '\n'
    for name in ('object_list_header_get', 'object_list_get'):
        types += define(lists_h, name)

    cast = function(runtime, 'hs_cast')
    arguments = function(runtime, 'hs_arguments_evaluate')
    still_exists = function(runtime, 'hs_value_still_exists')
    first = function(lists, 'object_list_get_first')
    following = function(lists, 'object_list_get_next')
    kill = function(units, 'unit_kill')
    kill_silent = function(units, 'unit_kill_silent')
    animation_time = function(read('source/objects/scenery.c'), 'scenery_get_animation_time')
    if fault in ('no-cast', 'no-checks'):
        cast = mutated(cast, CASE_CHECK, '')
    if fault == 'no-passthrough':
        cast = mutated(cast, 'actual_type==_hs_passthrough ? desired_type : actual_type', 'actual_type')
    if fault in ('no-arguments', 'no-checks'):
        arguments = mutated(arguments, ARGUMENTS_CHECK, '')
    if fault == 'no-collected-lists':
        still_exists = mutated(still_exists, 'datum_try_and_get(object_list_header_data, value) ? value : NONE', 'value')
    if fault == 'no-first-skip':
        first = mutated(first, 'index = object_list_skip_deleted(index, reference_index);', '')
    if fault == 'no-next-skip':
        following = mutated(following, 'return object_list_skip_deleted(\n\t\treference_list_get_next_datum_index('
                            'object_list_data, reference_index),\n\t\treference_index);',
                            'return reference_list_get_next_datum_index(object_list_data, reference_index);')
    if fault == 'kill':
        kill = mutated(kill, 'unit_try_and_get(unit_index)', 'unit_get(unit_index)')
    if fault == 'kill-silent':
        kill_silent = mutated(kill_silent, 'unit_try_and_get(unit_index)', 'unit_get(unit_index)')
    if fault == 'scenery':
        animation_time = mutated(animation_time, 'scenery_try_and_get(object_index)', 'scenery_get(object_index)')

    code = function(data, 'datum_try_and_get') + '\n' + function(data, 'datum_get') + '\n'
    code += function(data, 'datum_delete') + '\n'
    code += function(objects, 'object_try_and_get_and_verify_type') + '\n'
    code += function(objects, 'object_get_and_verify_type') + '\n'
    for name in ('reference_list_get_next_datum_index', 'reference_list_add', 'reference_list_delete'):
        code += inline(references, name) + '\n'
    code += define(lists, 'object_list_get_first_internal')
    for name in ('object_list_new', 'object_list_add', 'object_list_delete', 'object_list_count'):
        code += function(lists, name) + '\n'
    code += first + '\n' + following + '\n' + function(lists, 'object_list_skip_deleted') + '\n'
    code += still_exists + '\n' + function(runtime, 'hs_object_name_to_object_list') + '\n'
    code += function(runtime, 'hs_object_to_object_list') + '\n' + cast + '\n'
    for name in ('hs_stack_allocate', 'hs_stack_push', 'hs_return', 'hs_macro_function_evaluate', 'hs_evaluate',
                 'hs_global_evaluate'):
        code += function(runtime, name) + '\n'
    code += arguments + '\n'
    code += function(read('source/scenario/scenario.c'), 'scenario_trigger_volume_test_object') + '\n'
    for name in ('hs_trigger_volume_test_objects', 'hs_trigger_volume_test_objects_all',
                 'hs_trigger_volume_test_objects_any', 'hs_object_list_get_element'):
        code += function(external, name) + '\n'
    code += kill + '\n' + kill_silent + '\n' + animation_time + '\n'
    code += structure(hs, 'hs_arguments_short_long') + '\n'
    code += 'union hs_boolean_result { boolean boolean; long value; };\n'
    code += define(hs, 'HS_EVALUATE_RETURN_BOOLEAN')
    code += evaluator(hs, 'scenario_trigger_volume_test_object_evaluate')
    code += evaluator(hs, 'hs_trigger_volume_test_objects_any_evaluate')
    return (('types.inc', types), ('under_test.inc', code))


@pytest.mark.parametrize('case', [
    'global-living', 'global-deleted', 'global-read-as-none', 'slot-reused', 'returned-deleted',
    'returned-to-volume-test', 'passthrough-deleted', 'returned-living', 'deleted-during-arguments', 'cast-to-list',
    'names', 'other-values', 'list-walk', 'list-first-deleted', 'list-middle-deleted', 'list-last-deleted',
    'list-all-deleted', 'list-get-and-count', 'list-volume-any', 'list-volume-all', 'list-collected', 'kill-none',
    'kill-living', 'scenery-none', 'scenery-living'])
def test_stale_objects(case):
    status, output = run(build('script_stale_objects', generated()), case)
    assert status == 0, output


@pytest.mark.parametrize('fault,case', [
    ('no-checks', 'global-deleted'), ('no-checks', 'returned-to-volume-test'), ('no-cast', 'global-read-as-none'),
    ('no-cast', 'returned-deleted'), ('no-cast', 'slot-reused'), ('no-cast', 'cast-to-list'),
    ('no-passthrough', 'passthrough-deleted'), ('no-arguments', 'deleted-during-arguments'),
    ('no-collected-lists', 'list-collected'), ('no-first-skip', 'list-first-deleted'),
    ('no-first-skip', 'list-all-deleted'), ('no-next-skip', 'list-middle-deleted'),
    ('no-next-skip', 'list-last-deleted'), ('no-next-skip', 'list-get-and-count'), ('kill', 'kill-none'),
    ('kill-silent', 'kill-none'), ('scenery', 'scenery-none')])
def test_negative(fault, case):
    status, output = run(build('script_stale_objects', generated(fault)), case)
    assert status == CHECK_FAILED, output
