"""Same-handle reconstruction, binding and old-life damage through the real gate.

The dispatcher/transport ordering is tested separately; this fixture couples
its production epoch gate to real replica lifecycle and damage handlers.
"""
import re

import pytest

from harness import build, constant, enum_with, function, mutated, read, run, structure


def define_value(source, name):
    """Keep a simple source constant's expression, including its dependencies."""
    match = re.search(r"^#define\s+" + re.escape(name) + r"\s+([^\n]+)", source, re.M)
    if not match:
        match = re.search(r"^\s*" + re.escape(name) + r"\s*=\s*([^,\n]+),", source, re.M)
    if not match:
        raise LookupError(f"constant declaration not found: {name}")
    return f"#define {name} ({match.group(1)})"


def generated(fault=None):
    objects = read("port/linux/game/network_objects.c")
    distributed = read("port/linux/game/network_distributed.c")
    data = read("source/memory/data.c")
    players = read("source/game/players.c")
    units = read("source/units/units.c")
    damage = read("port/linux/game/network_damage.c")
    distributed_h = read("port/linux/game/network_distributed.h")
    units_h = read("source/units/units.h")
    objects_h = read("source/objects/objects.h")
    damage_h = read("source/objects/damage.h")
    config = [f"#define TICKS_PER_SECOND {constant(read('source/cseries/cseries.h'), 'TICKS_PER_SECOND')}",
              f"#define MAXIMUM_MESSAGE_SIZE {constant(read('source/bungie_net/common/message_header.h'), 'MAXIMUM_MESSAGE_SIZE')}",
              f"#define MAXIMUM_ENTRIES_PER_MESSAGE {constant(objects, 'MAXIMUM_ENTRIES_PER_MESSAGE')}",
              f"#define TAG_STRING_LENGTH {constant(read('source/tag_files/tag_files.h'), 'TAG_STRING_LENGTH')}",
              f"#define MAXIMUM_WEAPONS_PER_UNIT {constant(units_h, 'MAXIMUM_WEAPONS_PER_UNIT')}",
              f"#define MAXIMUM_REGIONS_PER_OBJECT {constant(read('source/objects/object_definitions.h'), 'MAXIMUM_REGIONS_PER_OBJECT')}"]
    config.append(re.search(r"^#define RELIABLE_ENTRIES\(type\) \\\n[^\n]+", distributed_h, re.M).group(0))
    for source, names in (
        (objects, ("MAXIMUM_CLIENT_RETRIED_INVENTORIES", "INVENTORY_INTERVAL_TICKS", "INVENTORY_REFRESH_TICKS",
                   "EMPTY_INVENTORY_TICKS", "OWN_INVENTORY_MARGIN_TICKS", "MAXIMUM_INVENTORY_GRENADES",
                   "DEFAULT_OWN_ROUND_TRIP_TICKS", "MAXIMUM_OWN_ROUND_TRIP_TICKS", "AGE_SCALE",
                   "OBJECT_WORLD_BOUND", "HOST_OBJECTS_RESEND_TICKS", "CLIENT_RETRY_TICKS",
                   "CLIENT_RETRY_MAXIMUM_TICKS", "CLIENT_READY_MAXIMUM_INTERVAL_TICKS", "MAXIMUM_CLIENT_NEW_OBJECTS",
                   "CTF_FLAG_TEAMS", "ODDBALL_TEAMS", "NETWORKED_OBJECT_TYPES")),
        (distributed, ("DEATH_BLOW_WAIT_TICKS", "SEAT_DISAGREEMENT_TICKS", "SEAT_DISAGREEMENT_SLACK_TICKS")),
        (distributed_h, ("REMOTE_CORRECTION_TOLERANCE", "REMOTE_BLEND_DISTANCE", "NO_PLAYER")),
        (damage, ("REPORT_WORLD_BOUND", "REPORT_IMPACT_TOLERANCE", "TARGET_HISTORY_TICKS",
                  "MAXIMUM_RECENT_WEAPONS", "MAXIMUM_GRENADE_THROWS", "MAXIMUM_STUCK_PROJECTILES",
                  "RECENT_WEAPON_TICKS", "HIT_REPORT_BURST_SECONDS", "PLAYER_TRAIL_POINTS",
                  "DAMAGE_RATE_CACHE_SIZE", "MAXIMUM_HIT_REPORTS_PER_TICK")),
    ):
        config.extend(define_value(source, name) for name in names)
    config.append(define_value(distributed_h, 'DISTRIBUTED_UNIT_SCALE'))
    types = "\n".join([
        enum_with(distributed_h, "_distributed_message_player_prediction ="),
        enum_with(distributed_h, "_network_vehicle_write_change,"),
        structure(distributed_h, "distributed_message_header"),
        enum_with(read("source/objects/object_types.h"), "_object_mask_all ="),
        enum_with(objects_h, "_object_dead_bit,"),
        enum_with(objects_h, "_object_connected_to_map_bit,"),
        enum_with(objects_h, "NUMBER_OF_OBJECT_CHANGE_COLORS,"),
        enum_with(units_h, "_unit_active_camouflaged_bit,"),
        enum_with(units_h, "_unit_grenade_throw_idle ="),
        enum_with(units_h, "NUMBER_OF_UNIT_GRENADE_TYPES\n"),
        enum_with(read("source/items/weapons.h"), "_magazine_reloading,"),
        enum_with(read("source/game/players.h"), "NUMBER_OF_PLAYER_POWERUPS,"),
        enum_with(damage_h, "NUMBER_OF_DAMAGE_DATA_FLAGS,"),
        enum_with(damage_h, "_object_being_damaged_body_depleted_bit ="),
        enum_with(read("source/game/game_globals.h"), "NUMBER_OF_MATERIAL_TYPES,"),
        structure(read("source/memory/data.h"), "data_array"),
        structure(read("source/memory/data.h"), "datum_header"),
        structure(read("source/memory/data.h"), "data_iterator"),
        enum_with(objects, "_object_change_create,"),
        enum_with(objects, "_send_rank_player_unit,"),
        enum_with(objects, "_distributed_object_carried_bit ="),
        enum_with(distributed, "_distributed_unit_alive_bit ="),
        enum_with(distributed, "_distributed_unit_camouflaged_bit ="),
        enum_with(distributed, "_distributed_pose_facing,"),
        structure(distributed_h, "distributed_vector"),
        structure(objects, "distributed_object_change"),
        structure(objects, "distributed_inventory"),
        structure(objects, "distributed_object_change_message"),
        structure(objects, "distributed_inventory_message"),
        structure(objects.replace("static struct", "struct"), "distributed_own_inventory"),
        structure(objects.replace("static struct", "struct"), "distributed_retried_inventory"),
        structure(read("port/linux/game/vehicle_riding.h"), "vehicle_riding_deck"),
        structure(distributed, "distributed_unit_state"),
        enum_with(damage, "_damage_event_player_effect,"),
        enum_with(damage, "_damage_event_friendly_fire_bit ="),
        structure(damage, "distributed_damage"),
        structure(damage, "distributed_damage_event"),
        structure(damage, "distributed_hit_report"),
        structure(damage, "distributed_hit_report_message"),
    ])
    data_code = "\n".join(function(data, name) for name in (
        "data_initialize", "data_verify", "data_usable", "datum_initialize",
        "datum_try_and_get", "datum_get", "datum_new_at_index", "datum_delete",
        "data_iterator_new", "data_iterator_next"))
    code = "\n".join(function(distributed, name) for name in (
        "distributed_object_index_valid", "distributed_real_valid", "distributed_point_valid",
        "distributed_axes_make_valid", "distributed_player", "distributed_player_from_byte",
        "distributed_player_is_local", "distributed_living_unit"))
    code += "\n" + "\n".join(function(objects, name) for name in (
        "distributed_vector_valid", "distributed_transform_valid", "network_objects_client_has"))
    code += "\n" + "\n".join(function(players, name) for name in (
        "network_player_detach_unit", "network_player_attach_unit"))
    code += "\n" + "\n".join(function(units, name) for name in (
        "unit_kill", "unit_kill_silent", "unit_kill_no_statistics",
        "unit_network_forget_weapon", "unit_network_add_weapon", "unit_network_drop_weapon"))
    code += "\n" + "\n".join(function(objects, name) for name in (
        "distributed_client_release_item", "distributed_client_delete",
        "distributed_client_request_objects", "distributed_client_create_failed",
        "network_objects_expect_live_unit", "distributed_client_own_object",
        "distributed_client_change_valid", "distributed_client_apply_change",
        "distributed_client_reuses_object", "distributed_client_create",
        "network_objects_handle_changes", "network_objects_handle_synchronized",
        "network_objects_discard_client_effects"))
    code += "\n" + "\n".join(function(distributed, name) for name in (
        "distributed_vector_unpack", "distributed_unit_vector_unpack",
        "distributed_scripted_pose_apply", "distributed_handle_unit_state"))
    code += "\n" + "\n".join(function(objects, name) for name in (
        "distributed_client_note_own_inventory", "distributed_client_own_settled",
        "distributed_client_weapon_here", "distributed_client_inventory_whole",
        "distributed_client_retry_inventory", "distributed_client_forget_retried_inventory",
        "distributed_client_apply_inventory", "distributed_client_apply_retried_inventories"))
    code += "\n" + "\n".join(function(damage, name) for name in (
        "distributed_damage_to_data", "distributed_damage_indices_valid",
        "distributed_damage_numbers_valid", "network_damage_handle_events",
        "network_damage_new_game", "network_damage_client_tick"))
    code += "\n" + function(distributed, "distributed_message_epoch_valid")
    code += "\n" + "\n".join(function(objects, name) for name in (
        "distributed_inventory_from_unit", "distributed_host_send_inventories_to_machine",
        "distributed_object_send_rank", "network_objects_client_asked"))
    code += "\n" + function(distributed, "distributed_checkpoint_ready")
    code += "\n" + "\n".join(function(objects, name) for name in (
        "distributed_client_note_own_inventories", "network_objects_client_tick"))
    if fault:
        code = mutated(code, *fault)
    return (("config.inc", "\n".join(config)), ("types.inc", types), ("data.inc", data_code),
            ("engine.inc", function(read("source/objects/objects.c"), "object_delete_recursive")),
            ("under_test.inc", code))


def test_optional_profile_reconstructs_and_rebinds():
    status, output = run(build("coop_checkpoint_recovery", generated()), "profile")
    assert status == 0, output


def test_default_profile_recovers_authoritative_live_unit():
    status, output = run(build("coop_checkpoint_recovery", generated()), "default")
    assert status == 0, output


def test_reconstructed_unit_rejects_old_life_kill():
    status, output = run(build("coop_checkpoint_recovery", generated()), "late-kill")
    assert status == 0, output


@pytest.mark.parametrize("case", [
    "live", "host-dead", "host-kills-live", "bound-corpse", "different-handle", "unknown", "unknown-player", "stale-delete",
    "invalid-tag", "invalid-team", "invalid-color", "invalid-position", "invalid-axes",
    "invalid-identifier", "outside-pool", "allocation-failure", "repeat", "retry",
    "inventory", "hierarchy", "late-inventory", "projectiles", "partial-producers", "same-epoch-create", "snapshot-empty", "snapshot-batches",
    "kill-current", "kill-unknown", "kill-invalid-number", "kill-invalid-tag", "kill-invalid-node",
    "identity-after-boundary",
])
def test_replica_controls(case):
    status, output = run(build("coop_checkpoint_recovery", generated()), case)
    assert status == 0, output


@pytest.mark.parametrize("before,after,case", [
    ("!distributed_client_change_valid(change, &forward, &up)",
     "(forward=change->forward, up=change->up, FALSE)", "invalid-tag"),
    ("objects_client_has[absolute_index] == change->object_index)", "TRUE)", "stale-delete"),
    ("network_player_attach_unit(player_index, state->unit_index);", "(void)player_index;", "profile"),
    ("unit_network_drop_weapon(object_index, weapon_slot);", "(void)weapon_slot;", "hierarchy"),
    ("unit_exit_seat_end(child_index);", "(void)child_index;", "hierarchy"),
    ("distributed_client_apply_inventory(&inventory);", "(void)inventory;", "late-inventory"),
    ("!distributed_damage_numbers_valid(&event->damage)", "FALSE", "kill-invalid-number"),
    ("header->checkpoint_epoch != distributed_checkpoint_epoch", "FALSE", "late-kill"),
    ("checkpoint_pending && network_coop_active()", "network_coop_active()", "same-epoch-create"),
    ("distributed_host_send_inventories_to_machine(machine_index);", "(void)machine_index;", "snapshot-empty"),
    ("if (!distributed_checkpoint_ready())\n\t\treturn;", "(void)0;", "snapshot-empty"),
    ("network_objects_discard_client_effects();", "(void)0;", "partial-producers"),
    ("\tdistributed_client_send_identity();\n\tdistributed_client_send_build();\n}",
     "\tif (objects_client_ready_time != NONE)\n\t{\n\t\tdistributed_client_send_identity();\n"
     "\t\tdistributed_client_send_build();\n\t}\n}", "identity-after-boundary"),
    ("network_damage_new_game();", "(void)0;", "partial-producers"),
    ("if (!distributed_checkpoint_synchronized)\n\t\t\t{", "if (TRUE)\n\t\t\t{", "partial-producers"),
    ("short limit = MIN(MAXIMUM_ENTRIES_PER_MESSAGE, RELIABLE_ENTRIES(struct distributed_inventory));",
     "short limit = MAXIMUM_ENTRIES_PER_MESSAGE;", "snapshot-batches"),
    ("if (!unit || TEST_FLAG(unit->object.damage_flags, _object_dead_bit))\n\t\t\tcontinue;\n\t\tdistributed_inventory_from_unit",
     "if (!unit || !unit->unit.grenade_counts[0] || TEST_FLAG(unit->object.damage_flags, _object_dead_bit))\n\t\t\tcontinue;\n\t\tdistributed_inventory_from_unit", "snapshot-empty"),
    ("if (!network_game_distributed_client())\n\t\treturn;\n\tobject_iterator_new(&iterator, _object_mask_projectile, 0);",
     "object_iterator_new(&iterator, _object_mask_projectile, 0);", "projectiles"),
    ("damage_replay_kill(event->object_index, &damage, event->node_index, event->region_index,\n"
     "\t\t\t\t\tevent->material_index);", "(void)victim_player_index;", "kill-current"),
])
def test_replica_mutations(before, after, case):
    status, output = run(build("coop_checkpoint_recovery", generated((before, after))), case)
    assert status == 1, output
