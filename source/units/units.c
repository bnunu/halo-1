/*
UNITS.C

symbols in this file:
001977E0 0040:
	_units_initialize (0000)
00197820 0020:
	_units_initialize_for_new_map (0000)
00197840 0010:
	_units_dispose_from_old_map (0000)
00197850 0010:
	_units_dispose (0000)
00197860 0020:
	_unit_kill (0000)
00197880 0020:
	_unit_kill_silent (0000)
001978A0 0020:
	_unit_kill_no_statistics (0000)
001978C0 0010:
	_unit_delete (0000)
001978D0 0020:
	_units_update (0000)
001978F0 0180:
	_unit_export_function_values (0000)
00197A70 0070:
	_unit_persistent_control (0000)
00197AE0 01e0:
	_unit_get_seat_entrance_point (0000)
00197CC0 00e0:
	_unit_get_melee_range_and_ticks (0000)
00197DA0 0090:
	_unit_get_animation_frames_remaining (0000)
00197E30 0140:
	_unit_euler_axis_doplan (0000)
00197F70 0020:
	_unit_get_zoom_level (0000)
00197F90 0080:
	_unit_animation_state_interruptable (0000)
00198010 0040:
	_unit_animation_busy (0000)
00198050 0020:
	_unit_animation_overlay_action_loops (0000)
00198070 0060:
	_unit_animation_state_loops (0000)
001980D0 0060:
	_unit_animation_weapon_ik (0000)
00198130 0040:
	_unit_animation_vehicle_ik (0000)
00198170 0020:
	_unit_animation_aiming_screen (0000)
00198190 0060:
	_unit_animation_state_get_aiming_screen_index (0000)
001981F0 0040:
	_unit_animation_state_can_be_entered_without_animation (0000)
00198230 0040:
	_unit_animation_compute_interpolation_frame_count (0000)
00198270 0190:
	_unit_animation_start_action (0000)
00198400 01c0:
	_unit_animation_start_overlay_action (0000)
001985C0 00c0:
	_unit_can_enter_seat (0000)
00198680 0070:
	_unit_impulse (0000)
001986F0 00d0:
	_unit_handle_weapon_state_change (0000)
001987C0 0250:
	_unit_record_damage (0000)
00198A10 00b0:
	_unit_test_spawning (0000)
00198AC0 0020:
	_unit_destroy (0000)
00198AE0 0040:
	_unit_get_head_position (0000)
00198B20 01a0:
	_unit_get_camera_position (0000)
00198CC0 0140:
	_unit_estimate_position (0000)
00198E00 0040:
	_unit_get_center_of_mass (0000)
00198E40 0190:
	_unit_animation_impulse_get_index (0000)
00198FD0 00d0:
	_unit_can_play_animation_impulse (0000)
001990A0 00c0:
	_unit_test_animation_impulse (0000)
00199160 0080:
	_unit_get_aiming_unit_index (0000)
001991E0 0030:
	_unit_get_aiming_vector (0000)
00199210 0030:
	_unit_get_looking_vector (0000)
00199240 0020:
	_unit_get_facing_vector (0000)
00199260 00b0:
	_unit_inventory_next_grenade (0000)
00199310 0020:
	_unit_controllable (0000)
00199330 0040:
	_unit_set_controllable (0000)
00199370 0040:
	_unit_set_possessed (0000)
001993B0 0060:
	_unit_is_busy (0000)
00199410 0050:
	_unit_scripting_set_emotion_animation (0000)
00199460 0080:
	_unit_scripting_suspended (0000)
001994E0 0040:
	_unit_scripting_can_blink (0000)
00199520 0050:
	_unit_scripting_doesnt_drop_items (0000)
00199570 0110:
	_unit_scripting_vehicle_test_seat_list (0000)
00199680 00a0:
	_unit_scripting_vehicle_test_seat (0000)
00199720 0080:
	_unit_scripting_unit_riders (0000)
001997A0 0030:
	_unit_scripting_unit_driver (0000)
001997D0 0030:
	_unit_scripting_unit_gunner (0000)
00199800 0160:
	_unit_shield_sapping_update (0000)
00199960 00f0:
	_units_debug_get_next_unit (0000)
00199A50 00d0:
	_units_debug_get_closest_unit (0000)
00199B20 0120:
	_unit_debug_ninja_rope (0000)
00199C40 0060:
	_unit_set_user_animation (0000)
00199CA0 0070:
	_any_unit_is_dangerous (0000)
00199D10 00a0:
	_unit_can_see_point (0000)
00199DB0 0040:
	_unit_driven_by_ai (0000)
00199DF0 0040:
	_unit_gunned_by_ai (0000)
00199E30 0040:
	_unit_set_desired_flashlight_state (0000)
00199E70 0030:
	_unit_get_current_flashlight_state (0000)
00199EA0 0140:
	_unit_detach_from_parent (0000)
00199FE0 0070:
	_unit_seat_filled (0000)
0019A050 0060:
	_unit_seat_is_driver (0000)
0019A0B0 0060:
	_unit_seat_is_gunner (0000)
0019A110 0060:
	_unit_seat_allow_noncombatants (0000)
0019A170 00e0:
	_unit_update_driver_and_gunner (0000)
0019A250 0020:
	_unit_get_current_equipment (0000)
0019A270 0100:
	_unit_add_grenade_to_inventory (0000)
0019A370 0090:
	_unit_add_grenade_type_to_inventory (0000)
0019A400 0120:
	_unit_add_equipment_to_inventory (0000)
0019A520 0040:
	_unit_delete_current_equipment (0000)
0019A560 0080:
	_unit_delete_all_weapons (0000)
0019A5E0 0060:
	_unit_has_weapon_definition_index (0000)
0019A640 0030:
	_unit_first_free_weapon_index (0000)
0019A670 0070:
	_unit_get_weapon_count (0000)
0019A6E0 0070:
	_unit_approve_weapon_pickup (0000)
0019A750 0070:
	_unit_get_grenade_count (0000)
0019A7C0 0060:
	_unit_get_current_grenade_type (0000)
0019A820 01d0:
	_unit_throw_grenade_move_to_hand (0000)
0019A9F0 0310:
	_unit_throw_grenade_release (0000)
0019AD00 02d0:
	_unit_seat_update (0000)
0019AFD0 0050:
	_base_seat_label_get (0000)
0019B020 0040:
	_seat_label_to_base_seat_index (0000)
0019B060 0050:
	_base_weapon_label_get (0000)
0019B0B0 00b0:
	_unit_set_animation (0000)
0019B160 0050:
	_unit_animation_update (0000)
0019B1B0 0080:
	_unit_refresh_illumination (0000)
0019B230 0050:
	_unit_get_weapon (0000)
0019B280 0190:
	_unit_drop_item (0000)
0019B410 00b0:
	_unit_drop_grenades (0000)
0019B4C0 0100:
	_unit_drop_inventory_weapons (0000)
0019B5C0 0040:
	_unit_handle_region_destroyed (0000)
0019B600 0080:
	_unit_melee_sound (0000)
0019B680 02a0:
	_unit_cause_continuous_melee_damage (0000)
0019B920 0040:
	_unit_set_enterable_by_player (0000)
0019B960 0030:
	_unit_set_emotion (0000)
0019B990 0040:
	_unit_aim_without_turning (0000)
0019B9D0 0070:
	_unit_get_custom_animation_time (0000)
0019BA40 0030:
	_unit_is_playing_custom_animation (0000)
0019BA70 0170:
	_unit_start_user_animation (0000)
0019BBE0 0060:
	_unit_set_mouth_aperture (0000)
0019BC40 0060:
	_unit_inventory_get_must_be_readied_weapon (0000)
0019BCA0 0040:
	_unit_has_weapon (0000)
0019BCE0 0060:
	_unit_has_weapon_with_flag (0000)
0019BD40 00d0:
	_unit_start_running_blindly (0000)
0019BE10 0030:
	_unit_stop_running_blindly (0000)
0019BE40 0100:
	_unit_flame_to_death (0000)
0019BF40 0030:
	_unit_flying_through_air (0000)
0019BF70 04f0:
	_unit_euler_axis_buildplan (0000)
0019C460 0200:
	_unit_euler_axis_couple (0000)
0019C660 0220:
	_unit_set_or_test_seat_and_weapon_label (0000)
0019C880 00d0:
	_unit_adjust_projectile_ray (0000)
0019C950 0200:
	_unit_render_debug (0000)
0019CB50 0580:
	_unit_animation_set_state (0000)
0019D0D0 0020:
	_unit_abort_animation (0000)
0019D0F0 0290:
	_unit_find_nearby_seat (0000)
0019D380 0420:
	_unit_clip_to_aiming_bounds (0000)
0019D7A0 0060:
	_unit_inventory_get_weapon (0000)
0019D800 00b0:
	_unit_set_actively_controlled (0000)
0019D8B0 01a0:
	_vehicle_scripting_find_available_seats (0000)
0019DA50 0020:
	_unit_open (0000)
0019DA70 0020:
	_unit_close (0000)
0019DA90 0040:
	_unit_overcharged (0000)
0019DAD0 0030:
	_unit_set_seat (0000)
0019DB00 0080:
	_units_set_desired_flashlight_state (0000)
0019DB80 00a0:
	_unit_get_seat_label (0000)
0019DC20 0040:
	_unit_drop_current_equipment (0000)
0019DC60 0050:
	_unit_can_use_weapon (0000)
0019DCB0 00d0:
	_unit_approve_weapon_swap (0000)
0019DD80 0170:
	_unit_weapon_next_index (0000)
0019DEF0 0100:
	_unit_drop_current_weapon (0000)
0019DFF0 0030:
	_unit_get_current_weapon_label (0000)
0019E020 0020:
	_scripting_set_magic_base_seat (0000)
0019E040 0030:
	_unit_scripting_set_seat (0000)
0019E070 00c0:
	_unit_handle_deleted_object (0000)
0019E130 0250:
	_unit_cause_melee_damage (0000)
0019E380 0640:
	_unit_cause_player_melee_damage (0000)
0019E9C0 0030:
	_unit_stop_custom_animation (0000)
0019E9F0 0080:
	_unit_custom_animation_at_frame (0000)
0019EA70 0120:
	_unit_align_facing (0000)
0019EB90 00a0:
	_unit_start_flaming_to_death (0000)
0019EC30 02e0:
	_unit_running_blind (0000)
0019EF10 0090:
	_unit_vectors_are_valid (0000)
0019EFA0 02e0:
	_unit_verify_vectors (0000)
0019F280 03a0:
	_unit_control (0000)
0019F620 0780:
	_unit_preprocess_node_orientations (0000)
0019FDA0 0180:
	_unit_postprocess_node_matrices (0000)
0019FF20 06d0:
	_unit_euler_aiming_update (0000)
001A05F0 0090:
	_unit_has_animation_to_enter_seat (0000)
001A0680 0500:
	_unit_update_animation (0000)
001A0B80 00c0:
	_unit_unzoom (0000)
001A0C40 0050:
	_unit_get_zoom_magnification (0000)
001A0C90 0060:
	_unit_integrated_night_vision_is_active (0000)
001A0CF0 0620:
	_unit_ping_animation (0000)
001A1310 0120:
	_unit_start_animation_impulse (0000)
001A1430 0020:
	_unit_inventory_next_weapon (0000)
001A1450 0110:
	_unit_melee_attack_begin (0000)
001A1560 0090:
	_unit_leap_begin (0000)
001A15F0 00b0:
	_unit_unsuspecting (0000)
001A16A0 0130:
	_unit_add_weapon_to_inventory (0000)
001A17D0 01b0:
	_unit_ready_desired_weapon (0000)
001A1980 01d0:
	_unit_throw_grenade_begin (0000)
001A1B50 0030:
	_scripting_magic_melee_attack (0000)
001A1B80 0240:
	_unit_impact_melee_damage (0000)
001A1DC0 0140:
	_unit_place (0000)
001A1F00 0050:
	_unit_solo_player_integrated_night_vision_is_active (0000)
001A1F50 00e0:
	_unit_add_initial_weapons (0000)
001A2030 0040:
	_unit_adjust_for_seat_change (0000)
001A2070 0400:
	_unit_new (0000)
001A2470 0250:
	_unit_enter_seat (0000)
001A26C0 0290:
	_unit_exit_seat_end (0000)
001A2950 0270:
	_unit_died (0000)
001A2BC0 0130:
	_unit_scripting_enter_vehicle (0000)
001A2CF0 0180:
	_vehicle_scripting_load_magic (0000)
001A2E70 0110:
	_unit_try_and_exit_seat (0000)
001A2F80 1730:
	_unit_update (0000)
001A46B0 0640:
	_unit_damage_aftermath (0000)
001A4CF0 0100:
	_vehicle_scripting_unload (0000)
001A4DF0 0040:
	_unit_scripting_exit_vehicle (0000)
002A4DCC 0006:
	??_C@_05PJLLPND@stand?$AA@ (0000)
002A4DD4 000c:
	??_C@_0M@CHPCMCIL@unit_update?$AA@ (0000)
002A4DE0 000d:
	??_C@_0N@KFMOFEIC@unit_globals?$AA@ (0000)
002A4DF0 001d:
	??_C@_0BN@LFBAPBJD@c?3?2halo?2SOURCE?2units?2units?4c?$AA@ (0000)
002A4E10 000d:
	??_C@_0N@IKNEFIIG@unit?5globals?$AA@ (0000)
002A4E20 0044:
	??_C@_0EE@MJKOADBA@VALID_FLAGS?$CIpersistent_control_f@ (0000)
002A4E64 000c:
	??_C@_0M@GBEBNJOA@?5enter?9hint?$AA@ (0000)
002A4E70 0010:
	??_C@_0BA@EKKAICAF@animation_state?$AA@ (0000)
002A4E80 0014:
	??_C@_0BE@DJIPKBF@MISSING?3?5?$CFs?5?8?$CFs?5?$CFs?8?$AA@ (0000)
002A4E94 001e:
	??_C@_0BO@COLKAFDO@best_new_attacker_index?$CB?$DNNONE?$AA@ (0000)
002A4EB8 0051:
	??_C@_0FB@GIBLCPPG@?$CIestimate_mode?5?$DO?$DN?50?$CJ?5?$CG?$CG?5?$CIestimat@ (0000)
002A4F0C 0024:
	??_C@_0CE@EFKICMFK@body_position?5?$CG?$CG?5estimated_posit@ (0000)
002A4F30 0005:
	??_C@_04IEJGKNJ@body?$AA@ (0000)
002A4F38 004c:
	??_C@_0EM@FBFJECKJ@animation_impulse?$DO?$DN0?5?$CG?$CG?5animatio@ (0000)
002A4F84 003f:
	??_C@_0DP@JJACFPIN@current_index?$DO?$DN0?5?$CG?$CG?5current_inde@ (0000)
002A4FC4 0029:
	??_C@_0CJ@EPNCNOAE@couldn?8t?5find?5the?5emotion?5animat@ (0000)
002A4FF0 0031:
	??_C@_0DB@OCHKLFEM@index?$DO?$DN0?5?$CG?$CG?5index?$DMNUMBER_OF_UNIT@ (0000)
002A5024 0004:
	__real@3ca3d70b (0000)
002A5028 0049:
	??_C@_0EJ@NJDBEHGI@equipment_definition?9?$DOequipment?4@ (0000)
002A5078 0045:
	??_C@_0EF@EDBHKOAK@?$CIgrenade_type?5?$DO?$DN?50?$CJ?5?$CG?$CG?5?$CIgrenade_@ (0000)
002A50C0 0011:
	??_C@_0BB@JFCLDGNC@grenade_count?$DO?$DN0?$AA@ (0000)
002A50D8 0049:
	??_C@_0EJ@KGELGNBO@equipment_definition?9?$DOequipment?4@ (0000)
002A5128 0046:
	??_C@_0EG@DMMDGKIO@equipment_definition?9?$DOequipment?4@ (0000)
002A5170 0055:
	??_C@_0FF@BHHPBICM@grenade_type?$DN?$DNNONE?5?$HM?$HM?5?$CIgrenade_t@ (0000)
002A51C8 0091:
	??_C@_0JB@KAHIGLEC@unit?9?$DOunit?4current_grenade_index@ (0000)
002A525C 000a:
	??_C@_09KBFFALMN@left?5hand?$AA@ (0000)
002A5268 003e:
	??_C@_0DO@JIFEEFPA@unit?9?$DOunit?4grenade_counts?$FLunit?9?$DO@ (0000)
002A52A8 0065:
	??_C@_0GF@CHMEGMC@unit?9?$DOunit?4current_grenade_index@ (0000)
002A5310 0040:
	??_C@_0EA@DPJNOHJJ@base_seat_index?$DO?$DN0?5?$CG?$CG?5base_seat_@ (0000)
002A5350 0046:
	??_C@_0EG@PHNFDNJE@base_weapon_index?$DO?$DN0?5?$CG?$CG?5base_wea@ (0000)
002A5398 0008:
	??_C@_07GDJPGPDM@unarmed?$AA@ (0000)
002A53A0 0011:
	??_C@_0BB@FJNDJOGI@?$CFs?3?5animation?5?$CFs?$AA@ (0000)
002A53B4 002b:
	??_C@_0CL@FEKGNBAM@index?$DO?$DN0?5?$CG?$CG?5index?$DMMAXIMUM_WEAPON@ (0000)
002A53E0 002d:
	??_C@_0CN@HHCMEDCD@item?9?$DOobject?4parent_object_index@ (0000)
002A5410 0038:
	??_C@_0DI@FHACCLPK@a?5?$CFs?5tried?5to?5drop?5a?5?$CFs?5which?5wa@ (0000)
002A5448 003c:
	??_C@_0DM@HPMFIBJI@WARNING?3?5?$CFs?5tried?5to?5die?5from?5fl@ (0000)
002A5484 001e:
	??_C@_0BO@JNFPEPKG@plan?9?$DOcoast_t?5?$CL?5actual_t?5?$DO?$DN?5t?$AA@ (0000)
002A54A4 0013:
	??_C@_0BD@PPEPHKIE@plan?9?$DOcoast_t?5?$DO?$DN?50?$AA@ (0000)
002A54B8 0011:
	??_C@_0BB@MPHLKAIB@coasting_vel?5?$DM?50?$AA@ (0000)
002A54CC 0007:
	??_C@_06IGEDIHAH@t?5?$DO?$DN?50?$AA@ (0000)
002A54D4 000a:
	??_C@_09FAMPJHMJ@disc?5?$DO?$DN?50?$AA@ (0000)
002A54E0 001a:
	??_C@_0BK@JAFCIDEL@REAL_MAX?5?$CB?$DN?5plan?9?$DOdecel_t?$AA@ (0000)
002A54FC 001a:
	??_C@_0BK@KHOHGFFP@REAL_MAX?5?$CB?$DN?5plan?9?$DOdecel_a?$AA@ (0000)
002A5518 001a:
	??_C@_0BK@JCCGJEF@REAL_MAX?5?$CB?$DN?5plan?9?$DOcoast_t?$AA@ (0000)
002A5534 001a:
	??_C@_0BK@LLGCLIBF@REAL_MAX?5?$CB?$DN?5plan?9?$DOaccel_t?$AA@ (0000)
002A5550 001a:
	??_C@_0BK@IMNHFOAB@REAL_MAX?5?$CB?$DN?5plan?9?$DOaccel_a?$AA@ (0000)
002A556C 0014:
	??_C@_0BE@OJLFFFPC@plan?9?$DOinitial_v?5?$DM?50?$AA@ (0000)
002A5580 001a:
	??_C@_0BK@KCJNEIPK@plan?9?$DOinitial_p?5?$DO?5?91e?903f?$AA@ (0000)
002A559C 0004:
	__real@ba83126f (0000)
002A55A0 003b:
	??_C@_0DL@NDCNCHBI@?$CIadjust_plan?9?$DOaccel_t?5?$DO?$DN?50?$CJ?5?$CG?$CG?5?$CI@ (0000)
002A55DC 0010:
	??_C@_0BA@JMNOFACH@t_extension?5?$DO?50?$AA@ (0000)
002A55EC 0017:
	??_C@_0BH@KDNEJEKJ@MISSING?3?5?$CFs?5?8?$CFs?5?$CFs?5?$CFs?8?$AA@ (0000)
002A5604 001a:
	??_C@_0BK@GEHICBJO@parent_seat_index?5?$CB?$DN?5NULL?$AA@ (0000)
002A5620 001b:
	??_C@_0BL@MGKMGDBI@relative_aiming_angles?4yaw?$AA@ (0000)
002A563C 001d:
	??_C@_0BN@LENPEPCG@relative_aiming_angles?4pitch?$AA@ (0000)
002A565C 0011:
	??_C@_0BB@DNAJOPNC@?$CGrelative_vector?$AA@ (0000)
002A5670 0074:
	??_C@_0HE@KJEMFFL@?$CIseat_desire_type?5?$DN?$DN?5NONE?$CJ?5?$HM?$HM?5?$CI?$CI@ (0000)
002A56E4 0014:
	??_C@_0BE@BPHFKLIJ@seat_substring_name?$AA@ (0000)
002A56F8 003b:
	??_C@_0DL@HBBDCAFF@current_index?$DO?$DN0?5?$CG?$CG?5current_inde@ (0000)
002A5734 0004:
	__real@3d0f5c29 (0000)
002A5738 0016:
	??_C@_0BG@JEMBMHGH@?$CGunit?9?$DOobject?4forward?$AA@ (0000)
002A5750 0011:
	??_C@_0BB@GELHHNAL@?$CGunit?9?$DOobject?4up?$AA@ (0000)
002A5764 0011:
	??_C@_0BB@LPKMKLOA@alignment_vector?$AA@ (0000)
002A5778 0004:
	__real@3cab92a6 (0000)
002A577C 0004:
	__real@bcab92a6 (0000)
002A5780 0004:
	__real@417ea5dd (0000)
002A5784 0004:
	__real@3e567750 (0000)
002A5788 0004:
	__real@4087cfed (0000)
002A578C 0033:
	??_C@_0DD@FIEJMHNA@unit_verify_vectors?5FAILURE?0?5see@ (0000)
002A57C0 0038:
	??_C@_0DI@OGIFHHM@?5?5looking?5vector?5?$CF08X?5?$CF08X?5?$CF08X?5@ (0000)
002A57F8 0037:
	??_C@_0DH@EJEBMEHF@?5?5aiming?5vector?5?$CF08X?5?$CF08X?5?$CF08X?5v@ (0000)
002A5830 004f:
	??_C@_0EP@KHDNKPNP@?5?5desired?5facing?5?$CF08X?5?$CF08X?5?$CF08X?0@ (0000)
002A5880 0044:
	??_C@_0EE@NPCICDEJ@?5?5object?3?5pos?5?$CF08X?5?$CF08X?5?$CF08X?0?5fw@ (0000)
002A58C4 001f:
	??_C@_0BP@KOGKNNHJ@?5?5warning?0?5hex?5dump?5follows?4?4?4?$AA@ (0000)
002A58E4 002c:
	??_C@_0CM@MJPCCGNM@?5?5looking?5vector?5?$CFf?5?$CFf?5?$CFf?5veloci@ (0000)
002A5910 002b:
	??_C@_0CL@ECJMNHKM@?5?5aiming?5vector?5?$CFf?5?$CFf?5?$CFf?5velocit@ (0000)
002A593C 003d:
	??_C@_0DN@FMBPFOHH@?5?5desired?5facing?5?$CFf?5?$CFf?5?$CFf?0?5aimin@ (0000)
002A597C 003a:
	??_C@_0DK@HCAIDIOB@?$CK?$CK?$CK?$CK?5unit_verify_vectors?3?5proble@ (0000)
002A59B8 000d:
	??_C@_0N@GHHFFCIH@unit?9control?$AA@ (0000)
002A59C8 001e:
	??_C@_0BO@MDANOMAD@control_data?9?$DOprimary_trigger?$AA@ (0000)
002A59E8 0040:
	??_C@_0EA@IBGHKCGO@control_data?9?$DOzoom_level?$DN?$DNNONE?5?$HM@ (0000)
002A5A28 0082:
	??_C@_0IC@CHONDMKL@control_data?9?$DOgrenade_index?$DN?$DNNON@ (0000)
002A5AB0 007b:
	??_C@_0HL@IFOOEEOJ@control_data?9?$DOweapon_index?$DN?$DNNONE@ (0000)
002A5B2C 001e:
	??_C@_0BO@CKDHHENP@?$CGcontrol_data?9?$DOlooking_vector?$AA@ (0000)
002A5B4C 001d:
	??_C@_0BN@CPLNIAKI@?$CGcontrol_data?9?$DOaiming_vector?$AA@ (0000)
002A5B6C 001d:
	??_C@_0BN@PJECCKBL@?$CGcontrol_data?9?$DOfacing_vector?$AA@ (0000)
002A5B90 0047:
	??_C@_0EH@EBEINPCA@VALID_FLAGS?$CIcontrol_data?9?$DOcontro@ (0000)
002A5BD8 0059:
	??_C@_0FJ@GIIHEDDP@control_data?9?$DOaiming_speed?$DO?$DN0?5?$CG?$CG@ (0000)
002A5C38 0062:
	??_C@_0GC@CEOKMDLJ@control_data?9?$DOanimation_state?$DO?$DN0@ (0000)
002A5C9C 002b:
	??_C@_0CL@MKJGLHIO@magnitude3d?$CI?$CGcontrol_data?9?$DOthrot@ (0000)
002A5CC8 001c:
	??_C@_0BM@MNPGDCFG@relative_looking_angles?4yaw?$AA@ (0000)
002A5CE4 001e:
	??_C@_0BO@MGEJEHJF@relative_looking_angles?4pitch?$AA@ (0000)
002A5D04 0019:
	??_C@_0BJ@IONAJDB@?$CGrelative_looking_vector?$AA@ (0000)
002A5D20 0018:
	??_C@_0BI@PHNGNHBO@?$CGrelative_aiming_vector?$AA@ (0000)
002A5D38 001a:
	??_C@_0BK@KJJPMJMN@?$CGunit?9?$DOunit?4aiming_vector?$AA@ (0000)
002A5D54 0016:
	??_C@_0BG@OEBDAJFJ@unit?9preprocess?9nodes?$AA@ (0000)
002A5D6C 000e:
	??_C@_0O@ODHCBCIJ@aiming_vector?$AA@ (0000)
002A5D80 0061:
	??_C@_0GB@FBDPEFM@?$CIend_aiming_angles?4pitch?5?$DO?$DN?5aimi@ (0000)
002A5DE8 005d:
	??_C@_0FN@IHCHGFNM@?$CIend_aiming_angles?4yaw?5?$DO?$DN?5aiming@ (0000)
002A5E48 002b:
	??_C@_0CL@BAPCPLDO@end_aiming_angles?4yaw?5?$DO?$DN?5aiming_@ (0000)
002A5E74 002b:
	??_C@_0CL@ONCNMLAC@end_aiming_angles?4yaw?5?$DM?$DN?5aiming_@ (0000)
002A5EA0 002f:
	??_C@_0CP@FHKMNBOC@desired_aiming_angles?4yaw?5?$DO?$DN?5aim@ (0000)
002A5ED0 002f:
	??_C@_0CP@KKHDOBNO@desired_aiming_angles?4yaw?5?$DM?$DN?5aim@ (0000)
002A5F00 004f:
	??_C@_0EP@KBPKPPJL@?$CIangular_acceleration_limit?5?$DO?50?4@ (0000)
002A5F50 001f:
	??_C@_0BP@DPNLKBBM@angular_velocity_limit?5?$DO?$DN?50?40f?$AA@ (0000)
002A5F70 001e:
	??_C@_0BO@GAMKEDMN@desired_base_seat_index?$CB?$DNNONE?$AA@ (0000)
002A5F90 0038:
	??_C@_0DI@EIIFDJIB@desired_state?$DO?$DN0?5?$CG?$CG?5desired_stat@ (0000)
002A5FC8 0008:
	__real@4001475cd2000000 (0000)
002A5FD0 0027:
	??_C@_0CH@KNBNIKNG@unit?9?$DOobject?4parent_object_index@ (0000)
002A5FF8 0021:
	??_C@_0CB@MFPLMKIO@unit?9?$DOunit?4feign_death_timer?5?$DO?50@ (0000)
002A601C 0010:
	??_C@_0BA@JFPDPOCG@unit?9update?9end?$AA@ (0000)
002A602C 0024:
	??_C@_0CE@EIFKFLM@game_globals_first_person_interf@ (0000)
002A6050 002c:
	??_C@_0CM@JCKFNLKN@player?5illumination?3?5self?5?$CF?42f?5a@ (0000)
002A607C 0017:
	??_C@_0BH@GFOLCKHG@unit?9update?9postvector?$AA@ (0000)
002A6094 001b:
	??_C@_0BL@BHGJMDEB@?$CGunit?9?$DOunit?4looking_vector?$AA@ (0000)
002A60B0 0014:
	??_C@_0BE@MNAOFPAE@unit?9update?9look?93d?$AA@ (0000)
002A60C4 0017:
	??_C@_0BH@HICAMLFB@unit?9update?9look?9euler?$AA@ (0000)
002A60DC 0015:
	??_C@_0BF@NGCFEELO@unit?9update?9look?9set?$AA@ (0000)
002A60F4 0013:
	??_C@_0BD@DPFKBAB@unit?9update?9aim?93d?$AA@ (0000)
002A6108 0016:
	??_C@_0BG@NOLLOOND@unit?9update?9aim?9euler?$AA@ (0000)
002A6120 0014:
	??_C@_0BE@KGIBELMP@unit?9update?9aim?9set?$AA@ (0000)
002A6134 0022:
	??_C@_0CC@HPCCHHEP@?$CGunit?9?$DOunit?4desired_aiming_vecto@ (0000)
002A6158 0017:
	??_C@_0BH@PCBGODII@unit?9update?9prevectors?$AA@ (0000)
002A6170 0029:
	??_C@_0CJ@GMJCHFEH@?$CGgunner_unit?9?$DOunit?4desired_aimin@ (0000)
002A619C 0029:
	??_C@_0CJ@KNNFHIN@?$CGdriver_unit?9?$DOunit?4desired_facin@ (0000)
002A61C8 0012:
	??_C@_0BC@FNMDNBAN@unit?9update?9begin?$AA@ (0000)
002A61DC 0024:
	??_C@_0CE@MIPDNKIJ@p?$CFd?3?5body?5?$CF?42f?5shld?5?$CF?42f?5from?5?$CFs@ (0000)
00315C08 061c:
	_magic_base_animation_seat_index (0000)
	_base_seat_labels (0604)
004CCB18 0004:
	_bss_004ccb18 (0000)
*/

/* ---------- headers */

#include "cseries.h"
#include "infection.h" /* port: port/linux/game/infection.c */
#include "units.h"

#include "bipeds.h"
#include "biped_definitions.h"
#include "dialogue_definitions.h"
#include "unit_definitions.h"

#include "math/real_math.h"
#include "ai/actors.h"
#include "ai/ai_debug.h"
#include "ai/ai.h"
#include "bitmaps/bitmaps.h"
#include "cseries/errors.h"
#include "cseries/profile.h"
#include "devices/device_machines.h"
#include "effects/effects.h"
#include "effects/material_effect_definitions.h"
#include "game/cheats.h"
#include "game/game.h" /* port: game_connection (unit_turns_with_aim) */
#include "game/game_allegiance.h"
#include "game/game_globals.h"
#include "game/game_engine.h"
#include "game/players.h"
#include "hs/object_lists.h"
#include "interface/first_person_weapons.h"
#include "items/equipment.h"
#include "items/equipment_definitions.h"
#include "items/projectiles.h"
#include "items/weapon_definitions.h"
#include "items/weapons.h"
#include "main/console.h"
#include "models/model_animation_definitions.h"
#include "models/model_definitions.h"
#include "models/models.h" /* port: model_find_node (the grenade throw's legs) */
#include "objects/damage.h"
#include "objects/damage_effect_definitions.h"
#include "objects/object_lights.h"
#include "physics/breakable_surfaces.h"
#include "physics/collision_bsp.h"
#include "physics/collision_models.h"
#include "physics/collision_usage.h"
#include "physics/collisions.h"
#include "physics/physics_definitions.h"
#include "render/render.h"
#include "render/render_debug.h"
#include "scenario/scenario.h"
#include "saved games/game_state.h"
#include "sound/game_sound.h"
#include "vehicles.h"
#include "network_coop.h" /* port: port/linux/game/network_coop.c */
#include "coop_scripts.h" /* port: port/linux/game/coop_scripts.c */
#include "coop_enemies.h" /* port: port/linux/game/coop_enemies.c */

/* port: the control and animation impulses the host's actors give their
units go to the clients' copies (port/linux/game/network_actors.c) */
void network_actors_note_control(long unit_index, struct unit_control_data const *control_data);
void network_actors_note_impulse(long unit_index, short animation_impulse, real_vector2d const *alignment_vector);
void network_actors_note_melee(long unit_index, real_vector2d const *alignment_vector);
void network_actors_note_leap(long unit_index, real_vector2d const *alignment_vector);
/* port/linux/game/network_objects.c: the host's pick of a flinch or death animation */
short network_objects_damage_animation(long unit_index, short type, short animation_index);
void network_actors_note_user_animation(long unit_index, long animation_graph_index, short animation_index,
	boolean interpolate);

/* ---------- constants */

enum
{
	_unit_region_destroyed_head_bit = 9,
};

enum
{
	_unit_weapon_overlay_primary_recoil = 1,
	_unit_weapon_overlay_secondary_recoil,
	_unit_weapon_overlay_primary_charged,
	_unit_weapon_overlay_secondary_charged,
	_unit_weapon_overlay_primary_chamber,
	_unit_weapon_overlay_secondary_chamber,
};

enum
{
	NUMBER_OF_UNIT_ANIMATION_IMPULSES = 14,
	_unit_seat_unknown8_bit = 8,
};

enum
{
	_vehicle_seat_desire_not_driver,
	_vehicle_seat_desire_gunner,
	_vehicle_seat_desire_passenger,
	_vehicle_seat_desire_driver,
	_vehicle_seat_desire_any,
	NUMBER_OF_VEHICLE_SEAT_DESIRE_TYPES,
};

enum
{
	_unit_record_damage_driver_seat_type = 9,
};

enum
{
	_unit_debug_function_active_bit = 2,
};

enum
{
	_unit_damage_direction_front = 0,
	_unit_damage_direction_left,
	_unit_damage_direction_right,
	_unit_damage_direction_back,
};

enum
{
	_unit_damage_part_head = 2,
};

/* January computes these direction-cone limits from single-precision pi, then
   promotes the rounded results for the double-precision fabs comparisons. */
#define UNIT_DAMAGE_REAR_CONE_ANGLE 0.7853981852531433
#define UNIT_DAMAGE_FRONT_CONE_ANGLE 2.159845009446144

#define UNIT_DAMAGE_AFTERMATH_ANIMATION_FLAGS_MASK \
	(FLAG(_object_being_damaged_region_destroyed_bit) | \
	 FLAG(_object_being_damaged_shield_depleted_bit) | \
	 FLAG(_object_being_damaged_force_hard_ping_bit))

enum
{
	_damage_definition_pings_resistant_units_bit = 2,
	_damage_definition_does_not_ping_units_bit = 4,
};

/* ---------- macros */

#define unit_get_current_weapon_index(unit_index) unit_inventory_get_weapon((unit_index), unit_get((unit_index))->unit.current_weapon_index)
#define unit_get_desired_weapon_index(unit_index) unit_inventory_get_weapon((unit_index), unit_get((unit_index))->unit.desired_weapon_index)

/* ---------- structures */

struct unit_globals
{
	short next_timer;
	short highest_timer;
	boolean used_time;
	char pad[3];
};

struct unit_acceleration_plan
{
	boolean at_rest;
	char pad[3];
	real initial_position;
	real initial_velocity;
	real acceleration;
	real acceleration_time;
	real coast_time;
	real deceleration;
	real deceleration_time;
};
typedef char unit_acceleration_plan_size_check[
	sizeof(struct unit_acceleration_plan) == 0x20 ? 1 : -1];

struct unit_control_data
{
	char animation_state;
	char aiming_speed;
	word control_flags;
	short weapon_index;
	short grenade_index;
	short zoom_level;
	short pad;
	real_vector3d throttle;
	real primary_trigger;
	real_vector3d facing_vector;
	real_vector3d aiming_vector;
	real_vector3d looking_vector;
};

typedef char unit_control_data_size_assert[
	sizeof(struct unit_control_data) == 0x40 ? 1 : -1];

struct unit_initial_weapon
{
	struct tag_reference weapon;
	long unused[5];
};

typedef char unit_initial_weapon_size_assert[
	sizeof(struct unit_initial_weapon) == 0x24 ? 1 : -1];

/* This tag-block element layout is carried locally because this bounded wave
 * does not own the shared game-globals header. */
struct game_globals_falling_damage
{
	byte unused0[0x48];
	long unknown48;
	byte unused4c[0x20];
	struct tag_reference flaming_death_damage_effect;
	byte unused1[0x1C];
};
typedef char game_globals_falling_damage_size_check[
	sizeof(struct game_globals_falling_damage) == 0x98 ? 1 : -1];
typedef char game_globals_flaming_death_offset_check[
	offsetof(struct game_globals_falling_damage, flaming_death_damage_effect) == 0x6C ? 1 : -1];

/* ---------- prototypes */

void unit_detach_from_parent(
	long unit_index);
void unit_start_flaming_to_death(
	long unit_index,
	long attacker_object_index);
void unit_flame_to_death(
	long unit_index);
boolean unit_unsuspecting(
	long unit_index,
	real_point3d const *point);
void unit_cause_melee_damage(
	long unit_index,
	boolean melee_hit,
	long target_object_index,
	short node_index,
	short region_index,
	short material_index,
	real_vector3d const *object_normal);

static char const *base_seat_label_get(short base_seat_index);
static short seat_label_to_base_seat_index(char const *seat_label);
static char const *base_weapon_label_get(short base_weapon_index);

static void unit_refresh_illumination(long unit_index);

static boolean unit_euler_axis_doplan(
	struct unit_acceleration_plan *plan,
	real delta_time,
	real position,
	real *new_position,
	real velocity,
	real *new_velocity);
static void unit_euler_axis_buildplan(
	real position,
	real velocity,
	real maximum_velocity,
	real maximum_acceleration,
	struct unit_acceleration_plan *plan);
static void unit_euler_axis_couple(
	struct unit_acceleration_plan *first_plan,
	struct unit_acceleration_plan *second_plan,
	real maximum_velocity,
	real maximum_acceleration);

static void unit_melee_sound(
	long unit_index,
	short material_type,
	long damage_effect_index);
static void unit_update_driver_and_gunner(
	long unit_index);
static short unit_first_free_weapon_index(
	long unit_index);

static short unit_animation_impulse_get_index(
	short animation_impulse,
	short *interpolation_frame_count);
static boolean unit_can_play_animation_impulse(
	long unit_index,
	short animation_impulse);

static boolean unit_animation_aiming_screen(
	struct unit_animation *animation);
static boolean unit_animation_state_can_be_entered_without_animation(
	short state);
static short unit_animation_compute_interpolation_frame_count(
	short new_state,
	short old_state);
static boolean unit_animation_overlay_action_loops(
	struct unit_animation *animation);
static boolean unit_animation_state_interruptable(
	struct unit_animation *animation,
	short desired_state);
static boolean unit_animation_state_loops(
	struct unit_animation *animation);
static long unit_animation_state_get_aiming_screen_index(
	short state);
static void unit_set_animation(
	long unit_index,
	long animation_graph_index,
	short animation_index);
static short unit_animation_update(
	long unit_index,
	long animation_graph_index,
	struct animation_state *animation);
static char const *unit_get_current_weapon_label(
	long unit_index);
static void unit_align_facing(
	long unit_index,
	real_vector2d const *alignment_vector);

static short unit_weapon_next_index(long unit_index, short current_index, short delta);
static void unit_ready_desired_weapon(
	long unit_index,
	boolean immediate);

static void unit_throw_grenade_move_to_hand(long unit_index);


static boolean unit_animation_weapon_ik(
	struct unit_animation *animation);
static boolean unit_animation_vehicle_ik(
	struct unit_animation *animation);
static boolean unit_animation_busy(struct unit_animation *animation);

static boolean unit_set_or_test_seat_and_weapon_label(
	long object_index,
	char const *seat_label,
	char const *weapon_label,
	boolean change_flag);

static boolean unit_animation_set_state(
	long unit_index,
	short new_state);

static boolean unit_vectors_are_valid(long unit_index);
static void unit_throw_grenade_release(long unit_index, boolean premature);

static void unit_seat_update(long object_index);

static char const *unit_get_seat_label(long object_index);

static void unit_cause_continuous_melee_damage(long unit_index);

static long unit_get_weapon(struct unit_datum *unit, short index);
static void unit_drop_item(long unit_index, long item_index);
/* port/linux/game/network_objects.c's */
boolean network_objects_creating_host_object(void);
/* port/linux/game/network_damage.c's */
void network_damage_note_grenade(long unit_index, short grenade_type);
/* network_game_globals.c's */
boolean network_game_distributed_client(void);
static void unit_drop_grenades(
	long unit_index);
static void unit_drop_inventory_weapons(
	long unit_index);

static void unit_verify_vectors(long unit_index, char const *debugstring);
static void unit_running_blind(long unit_index, real_vector3d *run_vector);

static boolean unit_integrated_night_vision_is_active(long unit_index);

void unit_detach_from_parent(
	long unit_index);

extern char const *base_seat_labels[NUMBER_OF_UNIT_BASE_SEATS];

/* ---------- globals */

short magic_base_animation_seat_index = NONE;

static struct unit_globals *unit_globals;

static struct profile_section unit_update_section = {"unit_update", NONE, TRUE};

boolean debug_objects_unit_mouth_apeture;
boolean debug_objects_unit_seats;
boolean debug_objects_unit_vectors;
boolean stun_enable;
boolean debug_damage_taken;
boolean debug_unit_illumination;
boolean debug_unit_animations;
boolean debug_unit_all_animations;

/* ---------- public code */

void units_initialize(
	void)
{
	unit_globals = (struct unit_globals *)game_state_malloc("unit globals", NULL, sizeof(*unit_globals));
	match_assert("c:\\halo\\SOURCE\\units\\units.c", 264, unit_globals);

	return;
}

void units_initialize_for_new_map(
	void)
{
	memset(unit_globals, 0, offsetof(struct unit_globals, used_time));

	return;
}

void units_dispose_from_old_map(
	void)
{
	return;
}

void units_dispose(
	void)
{
	return;
}

void unit_kill(
	long unit_index)
{
	/* port: a script's unit may be none (hs_value_still_exists) */
	struct unit_datum *unit = unit_try_and_get(unit_index);

	if (unit)
		SET_FLAG(unit->object.damage_flags, _object_die_act_of_god_bit, TRUE);

	return;
}

void unit_kill_silent(
	long unit_index)
{
	/* port: a script's unit may be none (hs_value_still_exists) */
	struct unit_datum *unit = unit_try_and_get(unit_index);

	if (unit)
		SET_FLAG(unit->object.damage_flags, _object_die_act_of_god_silent_bit, TRUE);

	return;
}

void unit_kill_no_statistics(
	long unit_index)
{
	struct unit_datum *unit = unit_get(unit_index);

	SET_FLAG(unit->object.damage_flags, _object_die_act_of_god_no_statistics_bit, TRUE);

	return;
}

void unit_delete(
	long unit_index)
{
	return;
}

void unit_handle_region_destroyed(
	long object_index,
	short region_index,
	unsigned long damage_flags)
{
	struct unit_datum *unit = unit_get(object_index);

	if (!TEST_FLAG(unit->object.damage_flags, _object_dead_bit))
	{
		unit_scream(
			object_index,
			TEST_FLAG(damage_flags, _unit_region_destroyed_head_bit) ?
				_unit_scream_destroyed_head :
				_unit_scream_destroyed_limb);
	}

	return;
}

void units_update(
	void)
{
	unit_globals->next_timer = unit_globals->highest_timer;
	unit_globals->highest_timer = 0;
	unit_globals->used_time = 0;

	return;
}

enum
{
	_unit_function_none = 0,
	_unit_function_driver_seat_power,
	_unit_function_gunner_seat_power,
	_unit_function_aiming_change,
	_unit_function_mouth_aperture,
	_unit_function_integrated_light_power,
	_unit_function_can_blink,
	_unit_function_shield_sapping,
	NUMBER_OF_UNIT_FUNCTION_MODES,
};

void unit_export_function_values(
	long unit_index)
{
	struct unit_datum *unit = unit_get(unit_index);
	struct unit_definition *unit_definition = unit_definition_get(
		unit->definition_index);
	long function_index;

	for (function_index = 0;
		function_index < NUMBEROF(unit->object.incoming_function_values);
		function_index++)
	{
		if (unit_definition->unit.function_modes[function_index] !=
			_unit_function_none)
		{
			real value = 0.f;

			switch (unit_definition->unit.function_modes[function_index])
			{
			case _unit_function_driver_seat_power:
				value = unit->unit.seat_power[0];
				break;
			case _unit_function_gunner_seat_power:
				value = unit->unit.seat_power[1];
				break;
			case _unit_function_aiming_change:
				value = unit->unit.aiming_change * (1.f / 255.f);
				break;
			case _unit_function_mouth_aperture:
				value = unit->unit.mouth_aperture;
				break;
			case _unit_function_integrated_light_power:
				value = unit->unit.integrated_light_power;
				break;
			case _unit_function_can_blink:
				value =
					!TEST_FLAG(unit->object.damage_flags, _object_dead_bit) &&
					!TEST_FLAG(unit->unit.flags, _unit_cannot_blink_bit) ?
					1.f : 0.f;
				break;
			case _unit_function_shield_sapping:
			{
				struct animation_graph *animation_graph = animation_graph_definition_get(
					unit->object.animation.animation_graph_index);
				struct animation *animation = TAG_BLOCK_GET_ELEMENT(
					&animation_graph->animations,
					unit->object.animation.state.index,
					struct animation);
				short private_loop_frame_index =
					animation->private_loop_frame_index;
				boolean before_private_loop;

				/*
				The January executable compares and divides the animation index here.
				That is almost certainly an original bug: frame_index would measure
				progress through the animation. Preserve index for exact behavior; a
				corrected build may replace both uses below with state.frame_index.
				*/
				before_private_loop =
					unit->object.animation.state.index <
					private_loop_frame_index;
				if (!before_private_loop)
				{
					value = 1.f - unit->unit.shield_sap_timeout * (1.f / 90.f);
				}
				else
				{
					value =
						(real)unit->object.animation.state.index /
						(real)private_loop_frame_index;
				}
				break;
			}
			}

			unit->object.incoming_function_values[function_index] = value;
		}
	}

	return;
}

void unit_persistent_control(
	long unit_index,
	long control_ticks,
	unsigned long persistent_control_flags)
{
	struct unit_datum *unit = unit_get(unit_index);

	match_assert("c:\\halo\\SOURCE\\units\\units.c", 1541, VALID_FLAGS(persistent_control_flags, NUMBER_OF_UNIT_CONTROL_FLAGS));

	unit->unit.persistent_control_flags = persistent_control_flags;
	unit->unit.persistent_control_timer = control_ticks;

	return;
}

boolean unit_get_seat_entrance_point(
	long unit_index,
	long target_unit_index,
	short seat_index,
	real_point3d *entry_position,
	real_point3d *exit_position,
	real_point3d *seat_transform)
{
	struct unit_datum *unit = unit_get(unit_index);
	struct unit_definition *unit_definition =
		unit_definition_get(unit->definition_index);
	struct model *model = model_definition_get(
		unit_definition->object.model.index);
	real_matrix4x3 entrance_matrix;
	struct object_marker seat_marker;
	real_matrix4x3 root_matrix;
	struct object_marker enter_hint_marker;
	char enter_hint_marker_name[256];
	struct animation_graph *animation_graph = animation_graph_definition_get(
		unit_definition->object.animation_graph.index);
	struct unit_datum *target_unit = unit_get(target_unit_index);
	struct unit_definition *target_unit_definition =
		unit_definition_get(target_unit->definition_index);
	struct unit_seat *seat = TAG_BLOCK_GET_ELEMENT(
		&target_unit_definition->unit.seats,
		seat_index,
		struct unit_seat);
	short animation_seat_index;
	boolean result;

	result = FALSE;
	animation_seat_index = 0;
	if (animation_graph->unit_seats.count > 0)
	{
		do
		{
			struct animation_graph_unit_seat *animation_seat = TAG_BLOCK_GET_ELEMENT(
				&animation_graph->unit_seats,
				animation_seat_index,
				struct animation_graph_unit_seat);

			if (!_stricmp(animation_seat->label, seat->label))
			{
				if (animation_seat_index != NONE)
				{
					animation_seat = TAG_BLOCK_GET_ELEMENT(
						&animation_graph->unit_seats,
						animation_seat_index,
						struct animation_graph_unit_seat);

					if (animation_seat->animations.count >
						_unit_seat_animation_seat_enter)
					{
						short animation_index = animation_graph_animation_index_get(
							&animation_seat->animations)
								[_unit_seat_animation_seat_enter].animation_index;

						/* port: an animation the graph has (a map's index) */
						if (VALID_INDEX(animation_index, animation_graph->animations.count))
						{
							struct animation *animation = TAG_BLOCK_GET_ELEMENT(
								&animation_graph->animations,
								animation_index,
								struct animation);

							object_get_marker_by_name(
								target_unit_index,
								seat->marker_name,
								&seat_marker,
								1);
							animation_get_root_matrix(model, animation, 0, &root_matrix);
							matrix4x3_multiply(
								&seat_marker.matrix,
								&root_matrix,
								&entrance_matrix);

							/* port: the marker name is a map's 32 bytes, which
							needn't end; copied up to its length */
							csstrncpy(enter_hint_marker_name, seat->marker_name, TAG_STRING_LENGTH);
							enter_hint_marker_name[TAG_STRING_LENGTH] = 0;
							csstrcat(enter_hint_marker_name, " enter-hint");
							object_get_marker_by_name(
								target_unit_index,
								enter_hint_marker_name,
								&enter_hint_marker,
								1);

							if (exit_position)
							{
								*exit_position = seat_marker.matrix.position;
							}
							if (entry_position)
							{
								*entry_position = entrance_matrix.position;
							}
							if (seat_transform)
							{
								*seat_transform = enter_hint_marker.matrix.position;
							}

							result = TRUE;
						}
					}
				}

				break;
			}

			animation_seat_index++;
		}
		while (animation_seat_index < animation_graph->unit_seats.count);
	}

	return result;
}

boolean unit_get_melee_range_and_ticks(
	long unit_index,
	boolean secondary,
	short *melee_tick,
	real *attack_time,
	short *frame_count,
	real *damage_time)
{
	boolean result;
	struct unit_datum *unit = unit_get(unit_index);
	struct unit_definition *unit_definition =
		unit_definition_get(unit->definition_index);
	struct animation_graph *animation_graph;
	struct animation_graph_unit_seat *animation_seat;
	struct animation_graph_weapon_class *weapon_class;
	long weapon_class_animation_index;
	short animation_index;

	model_definition_get(unit_definition->object.model.index);
	animation_graph = animation_graph_definition_get(
		unit_definition->object.animation_graph.index);
	animation_seat = TAG_BLOCK_GET_ELEMENT(
		&animation_graph->unit_seats,
		unit->unit.animation.seat_index,
		struct animation_graph_unit_seat);
	weapon_class = TAG_BLOCK_GET_ELEMENT(
		&animation_seat->weapon_classes,
		unit->unit.animation.weapon_index,
		struct animation_graph_weapon_class);

	weapon_class_animation_index = secondary ?
		_unit_weapon_class_animation_melee_airborne :
		_unit_weapon_class_animation_melee_attack;
	if (VALID_INDEX(
		weapon_class_animation_index,
		weapon_class->animations.count))
	{
		animation_index = animation_graph_animation_index_get(
			&weapon_class->animations)
				[weapon_class_animation_index].animation_index;
	}
	else
	{
		animation_index = NONE;
	}

	result = FALSE;
	if (animation_index != NONE)
	{
		struct animation *animation = TAG_BLOCK_GET_ELEMENT(
			&animation_graph->animations,
			animation_index,
			struct animation);

		animation_get_x_offsets(animation, attack_time, damage_time);
		if (melee_tick)
		{
			*melee_tick = animation->private_key_frame_index;
		}
		if (frame_count)
		{
			*frame_count = animation->frame_count;
		}

		result = TRUE;
	}

	return result;
}

boolean unit_set_user_animation(
	long unit_index,
	long animation_index,
	short index)
{
	struct unit_datum *unit = unit_get(unit_index);
	struct unit_definition *unit_definition = unit_definition_get(unit->definition_index);

	animation_graph_definition_get(unit_definition->object.animation_graph.index);

	match_assert("c:\\halo\\SOURCE\\units\\units.c", 6689, index>=0 && index<NUMBER_OF_UNIT_USER_ANIMATIONS);

	return FALSE;
}

short unit_get_animation_frames_remaining(
	long unit_index,
	short *animation_state)
{
	struct unit_datum *unit = unit_get(unit_index);
	struct animation_graph *animation_graph = animation_graph_definition_get(unit->object.animation.animation_graph_index);
	struct animation *animation = TAG_BLOCK_GET_ELEMENT(&animation_graph->animations, unit->object.animation.state.index, struct animation);

	match_assert("c:\\halo\\SOURCE\\units\\units.c", 1848, animation_state);

	*animation_state = unit->unit.animation.state;

	return animation->frame_count - unit->object.animation.state.frame_index;
}

short unit_get_zoom_level(
	long unit_index)
{
	return unit_get(unit_index)->unit.current_zoom_level;
}

real unit_get_zoom_magnification(
	long unit_index,
	short zoom_level)
{
	real magnification = 1.f;
	struct unit_datum *unit = unit_get(unit_index);
	long weapon_index = unit_inventory_get_weapon(unit_index, unit->unit.current_weapon_index);

	if (weapon_index!=NONE)
	{
		magnification = weapon_get_zoom_magnification(weapon_index, zoom_level);
	}

	return magnification;
}


boolean unit_controllable(
	long unit_index)
{
	return TEST_FLAG(unit_get(unit_index)->unit.flags, _unit_controllable_bit);
}

boolean unit_is_busy(
	long unit_index)
{
	struct unit_datum *unit = unit_get(unit_index);

	return unit_animation_busy(&unit->unit.animation);
}

void unit_set_enterable_by_player(
	long unit_index,
	boolean enterable_by_player)
{
	if (unit_index!=NONE)
	{
		struct unit_datum *unit = unit_get(unit_index);
		SET_FLAG(unit->unit.flags, _unit_not_enterable_by_player_bit, !enterable_by_player);
	}

	return;
}

void unit_aim_without_turning(
	long unit_index,
	boolean aim_without_turning)
{
	if (unit_index!=NONE)
	{
		struct unit_datum *unit = unit_get(unit_index);
		SET_FLAG(unit->unit.flags, _unit_aim_without_turning_bit, aim_without_turning);
	}

	return;
}

void unit_set_controllable(
	long unit_index,
	boolean controllable)
{
	struct unit_datum *unit = unit_get(unit_index);

	SET_FLAG(unit->unit.flags, _unit_controllable_bit, controllable);

	return;
}

void unit_set_possessed(
	long unit_index,
	boolean possessed)
{
	struct unit_datum *unit = unit_get(unit_index);

	SET_FLAG(unit->unit.flags, _unit_possessed_by_recording_bit, possessed);

	return;
}

void unit_scripting_can_blink(
	long unit_index,
	boolean can_blink)
{
	if (unit_index!=NONE)
	{
		struct unit_datum *unit = unit_get(unit_index);

		SET_FLAG(unit->unit.flags, _unit_cannot_blink_bit, !can_blink);
	}

	return;
}

void unit_set_desired_flashlight_state(
	long unit_index,
	boolean desired_state)
{
	if (unit_index!=NONE)
	{
		struct unit_datum *unit = unit_get(unit_index);

		if (desired_state)
		{
			SET_FLAG(unit->unit.flags, _unit_desired_integrated_light_on_bit, TRUE);
		}
		else
		{
			SET_FLAG(unit->unit.flags, _unit_desired_integrated_light_off_bit, TRUE);
		}
	}

	return;
}

void units_set_desired_flashlight_state(
	long object_list_index,
	boolean desired_state)
{
	long reference_index;
	long unit_index = object_list_get_first(object_list_index, &reference_index);

	while (unit_index!=NONE)
	{
		if (unit_try_and_get(unit_index)!=NULL && unit_index!=NONE)
		{
			struct unit_datum *unit = unit_get(unit_index);

			if (desired_state)
			{
				SET_FLAG(unit->unit.flags, _unit_desired_integrated_light_on_bit, TRUE);
			}
			else
			{
				SET_FLAG(unit->unit.flags, _unit_desired_integrated_light_off_bit, TRUE);
			}
		}

		unit_index = object_list_get_next(object_list_index, &reference_index);
	}

	return;
}

boolean unit_driven_by_ai(
	long unit_index)
{
	struct unit_datum *unit = unit_get(unit_index);

	if (unit->unit.driver_object_index!=NONE)
	{
		unit = unit_get(unit->unit.driver_object_index);
	}

	return unit->unit.actor_index!=NONE;
}

boolean unit_gunned_by_ai(
	long unit_index)
{
	struct unit_datum *unit = unit_get(unit_index);

	if (unit->unit.gunner_object_index!=NONE)
	{
		unit = unit_get(unit->unit.gunner_object_index);
	}

	return unit->unit.actor_index!=NONE;
}

short unit_test_spawning(
	long unit_index)
{
	struct unit_datum *unit = unit_get(unit_index);
	short spawned_actor_count = 0;

	if (!TEST_FLAG(unit->unit.flags, _unit_spawned_actors_bit))
	{
		struct unit_definition *unit_definition = unit_definition_get(unit->definition_index);

		if (unit_definition->unit.spawned_actor_variant.index!=NONE)
		{
			spawned_actor_count = random_range(
				unit_definition->unit.spawn_actor_lower_bound,
				unit_definition->unit.spawn_actor_upper_bound + 1);

			if (spawned_actor_count>0)
			{
				spawned_actor_count = actors_spawn_from_unit(
					unit_index,
					unit_definition->unit.spawned_actor_variant.index,
					spawned_actor_count,
					unit_definition->unit.spawn_throw_velocity * 0.033333335f);
			}

			SET_FLAG(unit->unit.flags, _unit_spawned_actors_bit, TRUE);
		}
	}

	return spawned_actor_count;
}

long unit_scripting_unit_riders(
	long unit_index)
{
	long object_list_index = NONE;

	if (unit_index!=NONE)
	{
		struct unit_datum *unit = unit_get(unit_index);

		object_list_index = object_list_new();

		if (object_list_index!=NONE)
		{
			long child_object_index = unit->object.first_child_object_index;

			while (child_object_index!=NONE)
			{
				struct object_datum *child_object = object_get(child_object_index);

				if (TEST_FLAG(_object_mask_unit, child_object->object.type))
				{
					struct unit_datum *rider = (struct unit_datum *)child_object;

					if (rider->unit.parent_seat_index!=NONE)
					{
						object_list_add(object_list_index, child_object_index);
					}
				}

				child_object_index = child_object->object.next_object_index;
			}
		}
	}

	return object_list_index;
}

boolean unit_scripting_vehicle_test_seat_list(
	long vehicle_index,
	char const *seat_name,
	long object_list_index)
{
	boolean result = FALSE;

	if (vehicle_index!=NONE)
	{
		short seat_index;
		struct unit_datum *vehicle = unit_get(vehicle_index);
		struct unit_definition *vehicle_definition = unit_definition_get(vehicle->definition_index);

		for (seat_index = 0; seat_index<vehicle_definition->unit.seats.count; ++seat_index)
		{
			struct unit_seat *seat = TAG_BLOCK_GET_ELEMENT(&vehicle_definition->unit.seats, seat_index, struct unit_seat);

			if (!_stricmp(seat_name, seat->label))
			{
				struct object_iterator iterator;
				struct unit_datum *unit;

				object_iterator_new(&iterator, _object_mask_unit, 0);
				unit = object_iterator_next(&iterator);

				while (unit)
				{
					if (unit->object.parent_object_index==vehicle_index &&
						unit->unit.parent_seat_index==seat_index)
					{
						long reference_index;
						long object_index = object_list_get_first(object_list_index, &reference_index);

						while (object_index!=NONE && iterator.index!=object_index)
						{
							object_index = object_list_get_next(object_list_index, &reference_index);
						}

						if (iterator.index==object_index)
						{
							result = TRUE;
						}

						break;
					}

					unit = object_iterator_next(&iterator);
				}
			}
		}
	}

	return result;
}

boolean unit_scripting_vehicle_test_seat(
	long vehicle_index,
	char const *seat_name,
	long unit_index)
{
	boolean result = FALSE;

	if (vehicle_index!=NONE && unit_index!=NONE)
	{
		short seat_index;
		struct unit_datum *vehicle = unit_get(vehicle_index);
		struct unit_definition *vehicle_definition = unit_definition_get(vehicle->definition_index);

		for (seat_index = 0; seat_index<vehicle_definition->unit.seats.count; ++seat_index)
		{
			struct unit_seat *seat = TAG_BLOCK_GET_ELEMENT(&vehicle_definition->unit.seats, seat_index, struct unit_seat);

			if (!_stricmp(seat_name, seat->label))
			{
				struct unit_datum *unit = unit_get(unit_index);

				if (unit->object.parent_object_index==vehicle_index && unit->unit.parent_seat_index==seat_index)
				{
					result = TRUE;
					break;
				}
			}
		}
	}

	return result;
}

boolean unit_seat_filled(
	long unit_index,
	short seat_index)
{
	struct object_iterator iterator;
	struct unit_datum *unit;
	boolean filled = FALSE;

	object_iterator_new(&iterator, _object_mask_unit, 0);

	while ((unit = object_iterator_next(&iterator))!=NULL)
	{
		if (unit->object.parent_object_index==unit_index && unit->unit.parent_seat_index==seat_index)
		{
			filled = TRUE;
			break;
		}
	}

	return filled;
}

boolean unit_seat_is_driver(
	long unit_index,
	short seat_index)
{
	struct unit_definition *unit_definition = unit_definition_get(unit_get(unit_index)->definition_index);
	boolean is_driver = FALSE;

	if (seat_index>=0 && seat_index<unit_definition->unit.seats.count)
	{
		struct unit_seat *seat = TAG_BLOCK_GET_ELEMENT(&unit_definition->unit.seats, seat_index, struct unit_seat);
		is_driver = TEST_FLAG(seat->flags, _unit_seat_driver_bit);
	}

	return is_driver;
}

boolean unit_seat_is_gunner(
	long unit_index,
	short seat_index)
{
	struct unit_definition *unit_definition = unit_definition_get(unit_get(unit_index)->definition_index);
	boolean is_gunner = FALSE;

	if (seat_index>=0 && seat_index<unit_definition->unit.seats.count)
	{
		struct unit_seat *seat = TAG_BLOCK_GET_ELEMENT(&unit_definition->unit.seats, seat_index, struct unit_seat);
		is_gunner = TEST_FLAG(seat->flags, _unit_seat_gunner_bit);
	}

	return is_gunner;
}

boolean unit_seat_allow_noncombatants(
	long unit_index,
	short seat_index)
{
	struct unit_definition *unit_definition = unit_definition_get(unit_get(unit_index)->definition_index);
	boolean allow_noncombatants = FALSE;

	if (seat_index>=0 && seat_index<unit_definition->unit.seats.count)
	{
		struct unit_seat *seat = TAG_BLOCK_GET_ELEMENT(&unit_definition->unit.seats, seat_index, struct unit_seat);
		allow_noncombatants = TEST_FLAG(seat->flags, _unit_seat_allows_noncombatants_bit);
	}

	return allow_noncombatants;
}

void unit_delete_current_equipment(
	long unit_index)
{
	struct unit_datum *unit = unit_get(unit_index);

	if (unit->unit.equipment_object_index!=NONE)
	{
		object_delete(unit->unit.equipment_object_index);
		unit->unit.equipment_object_index = NONE;
	}

	return;
}

void unit_delete_all_weapons(
	long unit_index)
{
	struct unit_datum *unit = unit_get(unit_index);
	short weapon_index;

	for (weapon_index = 0; weapon_index<MAXIMUM_WEAPONS_PER_UNIT; ++weapon_index)
	{
		if (unit->unit.weapon_object_indices[weapon_index]!=NONE && weapon_index!=unit->unit.current_weapon_index)
		{
			object_delete(unit->unit.weapon_object_indices[weapon_index]);
			unit->unit.weapon_object_indices[weapon_index] = NONE;

			if (weapon_index==unit->unit.desired_weapon_index)
			{
				unit->unit.desired_weapon_index = NONE;
			}

			if (weapon_index==unit->unit.current_weapon_index)
			{
				unit->unit.current_weapon_index = NONE;
			}
		}
	}

	return;
}

short unit_get_weapon_count(
	long unit_index)
{
	struct unit_datum *unit = unit_get(unit_index);
	long weapon_count = 0;
	short weapon_index;

	for (weapon_index = 0; weapon_index<MAXIMUM_WEAPONS_PER_UNIT; ++weapon_index)
	{
		long weapon_object_index = unit->unit.weapon_object_indices[weapon_index];

		if (weapon_object_index!=NONE)
		{
			struct weapon_datum *weapon = weapon_get(weapon_object_index);
			struct weapon_definition *weapon_definition = weapon_definition_get(weapon->definition_index);

			if (!TEST_FLAG(weapon_definition->weapon.flags, _weapon_doesnt_count_toward_maximum_bit))
			{
				++weapon_count;
			}
		}
	}

	return weapon_count;
}

boolean unit_approve_weapon_pickup(
	long unit_index,
	long weapon_index)
{
	struct unit_datum *unit = unit_get(unit_index);
	struct weapon_datum *weapon = weapon_get(weapon_index);
	long *inventory_weapon_index;
	long remaining_weapon_count;
	boolean approved;

	weapon_definition_get(weapon->definition_index);
	approved = TRUE;
	inventory_weapon_index = unit->unit.weapon_object_indices;
	remaining_weapon_count = MAXIMUM_WEAPONS_PER_UNIT;

	while (remaining_weapon_count--)
	{
		if (*inventory_weapon_index!=NONE &&
			weapon->definition_index==weapon_get(*inventory_weapon_index)->definition_index)
		{
			approved = FALSE;
		}

		++inventory_weapon_index;
	}

	return approved;
}

boolean unit_approve_weapon_swap(
	long unit_index,
	long weapon_index)
{
	struct unit_datum *unit = unit_get(unit_index);
	struct weapon_datum *weapon = weapon_get(weapon_index);
	boolean approved;
	short inventory_index;

	weapon_definition_get(weapon->definition_index);
	approved = TRUE;
	if (unit_get_current_weapon_index(unit_index)!=NONE)
	{
		for (inventory_index = 0;
			inventory_index<MAXIMUM_WEAPONS_PER_UNIT;
			++inventory_index)
		{
			long inventory_weapon_index =
				unit->unit.weapon_object_indices[inventory_index];

			if (inventory_weapon_index!=NONE)
			{
				struct weapon_datum *inventory_weapon =
					weapon_get(inventory_weapon_index);

				if (weapon->definition_index==inventory_weapon->definition_index &&
					(inventory_index!=unit->unit.current_weapon_index ||
					!(inventory_weapon->weapon.age>0.f) ||
					!(weapon->weapon.age<inventory_weapon->weapon.age)))
				{
					approved = FALSE;
				}
			}
		}

		return approved;
	}

	return FALSE;
}

boolean unit_has_weapon_definition_index(
	long unit_index,
	long weapon_definition_index)
{
	struct unit_datum *unit = unit_get(unit_index);
	short weapon_index;

	for (weapon_index = 0; weapon_index<MAXIMUM_WEAPONS_PER_UNIT; ++weapon_index)
	{
		long weapon_object_index = unit->unit.weapon_object_indices[weapon_index];

		if (weapon_object_index!=NONE && weapon_get(weapon_object_index)->definition_index==weapon_definition_index)
		{
			return TRUE;
		}
	}

	return FALSE;
}

boolean unit_has_weapon_with_flag(
	long unit_index,
	long flag_index)
{
	struct unit_datum *unit = unit_get(unit_index);
	boolean has_weapon = FALSE;
	long weapon_index;

	for (weapon_index = 0; weapon_index<MAXIMUM_WEAPONS_PER_UNIT; ++weapon_index)
	{
		long weapon_object_index = unit->unit.weapon_object_indices[weapon_index];

		if (weapon_object_index!=NONE && TEST_FLAG(weapon_get(weapon_object_index)->weapon.flags, flag_index))
		{
			has_weapon = TRUE;
			break;
		}
	}

	return has_weapon;
}

short unit_get_grenade_count(
	long unit_index,
	short grenade_type)
{
	struct unit_datum *unit = unit_get(unit_index);

	if (grenade_type!=NONE)
	{
		match_assert("c:\\halo\\SOURCE\\units\\units.c", 7847, grenade_type==NONE || (grenade_type>=0 && grenade_type<NUMBER_OF_UNIT_GRENADE_TYPES));

		/* port: a unit carries none of a type it doesn't have (a map's type) */
		if (!VALID_INDEX(grenade_type, NUMBER_OF_UNIT_GRENADE_TYPES))
			return 0;

		return unit->unit.grenade_counts[grenade_type];
	}

	return 0;
}

short unit_get_current_grenade_type(
	long unit_index)
{
	struct unit_datum *unit = unit_get(unit_index);

	match_assert("c:\\halo\\SOURCE\\units\\units.c", 7864, unit->unit.current_grenade_index==NONE || (unit->unit.current_grenade_index>=0 && unit->unit.current_grenade_index<NUMBER_OF_UNIT_GRENADE_TYPES));

	return unit->unit.current_grenade_index;
}

short unit_inventory_next_grenade(
	long unit_index,
	short current_index,
	short delta)
{
	struct unit_datum *unit = unit_get(unit_index);
	short next_index = NONE;
	short index;

	if (current_index==NONE)
	{
		current_index = 0;
	}
	else
	{
		match_assert("c:\\halo\\SOURCE\\units\\units.c", 5694, current_index>=0 && current_index<NUMBER_OF_UNIT_GRENADE_TYPES);
	}

	index = current_index;

	do
	{
		if (unit->unit.grenade_counts[index]>0)
		{
			next_index = index;

			if (index!=current_index || delta==0)
			{
				break;
			}
		}

		if (delta<0)
		{
			index = index==0 ? NUMBER_OF_UNIT_GRENADE_TYPES-1 : index-1;
		}
		else
		{
			index = index==NUMBER_OF_UNIT_GRENADE_TYPES-1 ? 0 : index+1;
		}
	} while (index!=current_index);

	return next_index;
}

short unit_inventory_get_must_be_readied_weapon(
	long unit_index)
{
	short must_be_readied_weapon_index = NONE;
	struct unit_datum *unit = unit_get(unit_index);
	short index;

	for (index = 0; index<MAXIMUM_WEAPONS_PER_UNIT; ++index)
	{
		long weapon_index = unit->unit.weapon_object_indices[index];

		if (weapon_index!=NONE && weapon_must_be_readied(weapon_index))
		{
			must_be_readied_weapon_index = index;
			break;
		}
	}

	return must_be_readied_weapon_index;
}

short unit_add_grenade_type_to_inventory(
	long unit_index,
	short grenade_type,
	short grenade_count)
{
	struct unit_datum *unit = unit_get(unit_index);

	match_assert("c:\\halo\\SOURCE\\units\\units.c", 7309, grenade_count>=0);
	match_assert("c:\\halo\\SOURCE\\units\\units.c", 7310, (grenade_type >= 0) && (grenade_type < NUMBER_OF_UNIT_GRENADE_TYPES));

	/* port: only the types a unit carries (an actor variant's type, map
	data; the rest wrote past the unit). Not reported: the released maps'
	warthog gunner variant has type 28786, with no grenades */
	if (!VALID_INDEX(grenade_type, NUMBER_OF_UNIT_GRENADE_TYPES))
		return 0;

	unit->unit.grenade_counts[grenade_type] += grenade_count;
	unit->unit.desired_grenade_index = grenade_type;
	unit->unit.current_grenade_index = grenade_type;

	return unit->unit.grenade_counts[grenade_type];
}

boolean unit_add_grenade_to_inventory(
	long unit_index,
	long equipment_index)
{
	struct item_datum *equipment = equipment_get(equipment_index);
	struct equipment_definition *equipment_definition =
		equipment_definition_get(equipment->definition_index);
	struct unit_datum *unit = unit_get(unit_index);
	struct game_globals_grenade *grenade = TAG_BLOCK_GET_ELEMENT(
		&scenario_get_game_globals()->grenades,
		equipment_definition->equipment.grenade_type,
		struct game_globals_grenade);
	long local_player_index;
	boolean result = FALSE;
	static boolean grenade_type_reported = FALSE;

	/* port: an Infection zombie takes no grenades */
	if (infection_unit_is_zombie(unit_index))
		return FALSE;

	match_assert(
		"c:\\halo\\SOURCE\\units\\units.c",
		7282,
		equipment_definition->equipment.powerup_type==_equipment_powerup_grenade);

	/* port: only the types a unit carries (a map's type) */
	if (!VALID_INDEX(equipment_definition->equipment.grenade_type, NUMBER_OF_UNIT_GRENADE_TYPES) &&
		!grenade_type_reported)
	{
		grenade_type_reported = TRUE;
		error(_error_silent, "### ERROR %s has grenade type %d; it can't be picked up",
			tag_get_name(equipment->definition_index), equipment_definition->equipment.grenade_type);
	}
	if (grenade &&
		VALID_INDEX(equipment_definition->equipment.grenade_type, NUMBER_OF_UNIT_GRENADE_TYPES) &&
		unit->unit.grenade_counts[equipment_definition->equipment.grenade_type] <
			grenade->maximum_count)
	{
		++unit->unit.grenade_counts[equipment_definition->equipment.grenade_type];

		if (player_index_from_unit_index(unit_index)==NONE)
		{
			local_player_index = NONE;
		}
		else
		{
			local_player_index =
				player_get(player_index_from_unit_index(unit_index))->local_player_index;
		}

		if (local_player_index!=NONE)
		{
			equipment_handle_pickup(equipment_index);
		}

		object_delete(equipment_index);
		result = TRUE;
	}

	return result;
}

boolean unit_add_equipment_to_inventory(
	long unit_index,
	long equipment_index,
	short replace)
{
	struct item_datum *equipment = equipment_get(equipment_index);
	struct equipment_definition *equipment_definition =
		equipment_definition_get(equipment->definition_index);
	struct unit_datum *unit = unit_get(unit_index);
	long local_player_index;
	boolean result = FALSE;

	match_assert(
		"c:\\halo\\SOURCE\\units\\units.c",
		7329,
		equipment_definition->equipment.powerup_type!=_equipment_powerup_none);
	match_assert(
		"c:\\halo\\SOURCE\\units\\units.c",
		7330,
		equipment_definition->equipment.powerup_type!=_equipment_powerup_grenade);

	/* port: nor powerups */
	if (infection_unit_is_zombie(unit_index))
		return FALSE;
	if (unit->unit.equipment_object_index!=NONE && replace==TRUE)
	{
		object_delete(unit->unit.equipment_object_index);
		unit->unit.equipment_object_index = NONE;
	}

	if (unit->unit.equipment_object_index==NONE)
	{
		object_disconnect_from_map(equipment_index);
		object_set_visibility(equipment_index, FALSE);

		if (player_index_from_unit_index(unit_index)==NONE)
		{
			local_player_index = NONE;
		}
		else
		{
			local_player_index =
				player_get(player_index_from_unit_index(unit_index))->local_player_index;
		}

		if (local_player_index!=NONE)
		{
			equipment_handle_pickup(equipment_index);
		}

		item_in_unit_inventory(equipment_index, unit_index);
		unit->unit.equipment_object_index = equipment_index;
		result = TRUE;
	}

	return result;
}

void unit_scripting_doesnt_drop_items(
	long object_list_index)
{
	long reference_index;
	long object_index = object_list_get_first(object_list_index, &reference_index);

	while (object_index!=NONE)
	{
		struct unit_datum *unit = unit_try_and_get(object_index);

		if (unit)
		{
			SET_FLAG(unit->unit.flags, _unit_doesnt_drop_items_bit, TRUE);
		}

		object_index = object_list_get_next(object_list_index, &reference_index);
	}

	return;
}

void unit_scripting_set_emotion_animation(
	long unit_index,
	char const *animation_name)
{
	if (unit_index!=NONE)
	{
		struct unit_datum *unit = unit_get(unit_index);
		short animation_index = animation_graph_get_animation_by_name(unit->object.animation.animation_graph_index, animation_name);

		if (animation_index!=NONE)
		{
			unit->unit.override_emotion_animation_index = animation_index;
		}
		else
		{
			console_warning("couldn't find the emotion animation '%s'", animation_name);
		}
	}

	return;
}

void unit_scripting_suspended(
	long unit_index,
	boolean suspended)
{
	/* port: co-op players the scripts can't name go with player0 */
	coop_scripts_suspend_followers(unit_index, suspended);
	if (unit_index!=NONE)
	{
		struct unit_datum *unit = unit_get(unit_index);

		SET_FLAG(unit->unit.flags, _unit_suspended_bit, suspended);
		unit->object.translational_velocity = *global_zero_vector3d;

		if (unit->object.type==_object_type_biped)
		{
			struct biped_datum *biped = biped_get(unit_index);
			SET_FLAG(biped->biped.flags, _biped_airborne_bit, FALSE);
		}
	}

	return;
}

boolean any_unit_is_dangerous(
	void)
{
	struct object_iterator iterator;
	struct unit_datum *unit;

	object_iterator_new(&iterator, _object_mask_unit, 1);

	while ((unit = object_iterator_next(&iterator))!=NULL)
	{
		if ((unit->unit.animation.state==_unit_state_throw_grenade && unit->unit.grenade_throw_state!=_unit_grenade_throw_ending) ||
			((unit->unit.animation.state==_unit_state_dying || unit->unit.animation.state==_unit_state_dying_airborne) &&
			!TEST_FLAG(unit->unit.animation.flags, _unit_animation_ignore_translation_bit)))
		{
			return TRUE;
		}
	}

	return FALSE;
}

boolean unit_custom_animation_at_frame(
	long unit_index,
	long animation_graph_index,
	char const *animation_name,
	boolean interpolate,
	short frame_index)
{
	boolean success = FALSE;

	if (unit_start_user_animation(unit_index, animation_graph_index, animation_name, interpolate))
	{
		struct unit_datum *unit = unit_get(unit_index);
		struct animation_graph *animation_graph = animation_graph_definition_get(unit->object.animation.animation_graph_index);
		struct animation *animation = TAG_BLOCK_GET_ELEMENT(&animation_graph->animations, unit->object.animation.state.index, struct animation);

		if (frame_index>=0 && frame_index<animation->frame_count)
		{
			unit->object.animation.state.frame_index = frame_index;
			success = TRUE;
			network_coop_note_unit_animation_frame(unit_index, frame_index);
		}
	}

	return success;
}

long unit_get_current_equipment(
	long unit_index)
{
	return unit_get(unit_index)->unit.equipment_object_index;
}

void unit_drop_current_equipment(
	long unit_index)
{
	struct unit_datum *unit = unit_get(unit_index);
	long equipment_index = unit->unit.equipment_object_index;

	if (equipment_index!=NONE)
	{
		unit_drop_item(unit_index, equipment_index);
		unit->unit.equipment_object_index = NONE;
	}

	return;
}

boolean unit_can_use_weapon(
	long unit_index,
	long weapon_index)
{
	boolean can_use_weapon;
	char const *seat_label;
	char const *weapon_label;

	unit_get(unit_index);
	weapon_get(weapon_index);

	seat_label = unit_get_seat_label(unit_index);
	weapon_label = weapon_get_label(weapon_index);

	can_use_weapon = unit_set_or_test_seat_and_weapon_label(
		unit_index,
		seat_label,
		weapon_label,
		FALSE);

	if (can_use_weapon)
	{
		can_use_weapon = game_engine_allow_pick_up(unit_index, weapon_index);
	}

	return can_use_weapon;
}

boolean unit_overcharged(
	long unit_index)
{
	struct unit_datum *unit = unit_get(unit_index);
	long weapon_index = unit_inventory_get_weapon(unit_index, unit->unit.current_weapon_index);

	if (weapon_index!=NONE)
	{
		return weapon_overcharged(weapon_index);
	}

	return FALSE;
}

boolean unit_solo_player_integrated_night_vision_is_active(
	void)
{
	boolean active = FALSE;

	if (local_player_count()==1)
	{
		long player_index = local_player_get_player_index(local_player_get_next(NONE));

		if (player_index!=NONE)
		{
			long unit_index = player_get(player_index)->unit_index;

			if (unit_index!=NONE)
			{
				active = unit_integrated_night_vision_is_active(unit_index);
			}
		}
	}

	return active;
}

// HCEX_Release.pdb and the September 2001 map both name this private
// helper unit_add_initial_weapons (file-static; no cachebeta public).
static void unit_add_initial_weapons(
	long unit_index)
{
	short initial_weapon_index;
	struct object_placement_data placement_data;
	struct unit_datum *unit = unit_get(unit_index);
	struct unit_definition *unit_definition = unit_definition_get(unit->definition_index);

	/* (a client of the distributed netcode making the host's unit: its
	weapons are the host's objects, port/linux/game/network_objects.c) */
	if (network_objects_creating_host_object())
		return;
	for (initial_weapon_index = 0;
		initial_weapon_index < unit_definition->unit.initial_weapons.count;
		initial_weapon_index++)
	{
		struct unit_initial_weapon *initial_weapon = TAG_BLOCK_GET_ELEMENT(
			&unit_definition->unit.initial_weapons,
			initial_weapon_index,
			struct unit_initial_weapon);

		if (initial_weapon->weapon.index!=NONE)
		{
			long weapon_index;

			object_placement_data_new(
				&placement_data,
				initial_weapon->weapon.index,
				unit_index);
			weapon_index = object_new(&placement_data);

			if (weapon_index!=NONE)
			{
				struct weapon_datum *weapon = weapon_get(weapon_index);

				if ((game_engine_running() &&
					unit_has_weapon_definition_index(unit_index, weapon->definition_index)) ||
					!unit_add_weapon_to_inventory(unit_index, weapon_index, FALSE))
				{
					object_delete(weapon_index);
				}
			}
		}
	}

	return;
}

boolean unit_new(
	long unit_index)
{
	boolean result;
	short seat_index;
	struct unit_datum *unit = unit_get(unit_index);
	struct unit_definition *unit_definition =
		unit_definition_get(unit->definition_index);
	result = FALSE;

	if (unit_definition->object.animation_graph.index!=NONE)
	{
		unit->unit.equipment_object_index = NONE;
		csmemset(
			unit->unit.weapon_object_indices,
			NONE,
			sizeof(unit->unit.weapon_object_indices));

		unit->unit.current_weapon_index = NONE;
		unit->unit.desired_weapon_index = NONE;
		unit->unit.current_grenade_index = NONE;
		unit->unit.desired_grenade_index = NONE;
		unit->unit.current_zoom_level = NONE;
		unit->unit.desired_zoom_level = NONE;
		unit->unit.player_index = NONE;
		unit->unit.actor_index = NONE;
		unit->unit.swarm_actor_index = NONE;
		unit->unit.swarm_next_unit_index = NONE;
		unit->unit.swarm_prev_unit_index = NONE;
		unit->unit.parent_seat_index = NONE;
		unit->unit.driver_object_index = NONE;
		unit->unit.gunner_object_index = NONE;

		unit->unit.animation.flags = 0;
		unit->unit.animation.seat_index = NONE;
		unit->unit.animation.weapon_index = NONE;
		unit->unit.animation.weapon_type_index = NONE;
		unit->unit.animation.state = NONE;
		unit->unit.animation.action = 0;
		unit->unit.animation.overlay_action = 0;
		unit->unit.animation.aiming_screen_index = NONE;
		unit->unit.animation.looking_screen_index = NONE;
		unit->unit.animation.action_animation.index = NONE;
		unit->unit.animation.overlay_action_animation.index = NONE;
		unit->unit.animation.soft_ping_animation.index = NONE;
		unit->unit.animation.base_seat_index = _unit_base_seat_stand;
		unit->unit.animation.last_ping_animation_index = NONE;
		unit->unit.animation.emotion_index = NONE;
		unit->unit.override_emotion_animation_index = NONE;
		unit->unit.magic_seat_index = NONE;
		unit->unit.animation.aiming_with_euler_screen = FALSE;
		csmemset(
			&unit->unit.animation.aiming_screen_bounds,
			0,
			sizeof(unit->unit.animation.aiming_screen_bounds));
		unit->unit.animation.looking_with_euler_screen = FALSE;
		csmemset(
			&unit->unit.animation.looking_screen_bounds,
			0,
			sizeof(unit->unit.animation.looking_screen_bounds));

		match_assert_valid_real_normal3d(
			"c:\\halo\\SOURCE\\units\\units.c",
			393,
			&unit->object.forward);

		unit->unit.desired_facing_vector =
			unit->unit.desired_aiming_vector =
			unit->unit.aiming_vector =
			unit->unit.desired_looking_vector =
			unit->unit.looking_vector = unit->object.forward;

		unit->unit.persistent_control_timer = 0;
		unit->unit.dialogue_index = NONE;
		SET_FLAG(unit->unit.flags, _unit_must_set_up_dialogue_bit, TRUE);
		csmemset(&unit->unit.speech, 0, sizeof(unit->unit.speech));
		unit->unit.speech.last_speech_finished_time = NONE;
		unit_dialogue_determine_variant(unit_index);
		csmemset(unit->unit.attackers, NONE, sizeof(unit->unit.attackers));

		unit->unit.last_damage_category = 0;
		unit->unit.delayed_damage_timer = 0;
		unit->unit.delayed_damage_peak = 0.f;
		unit->unit.killing_spree_count = 0;
		unit->unit.delayed_damage_attacker_object_index = NONE;
		unit->unit.time_of_death = NONE;
		unit->unit.fake_encounter_index = NONE;
		unit->unit.fake_squad_index = NONE;
		unit->unit.integrated_light_battery = 1.f;
		unit->unit.flaming_death_delay = 0;
		unit->unit.flaming_death_attacker_object_index = NONE;
		unit->unit.killing_spree_last_time = NONE;

		if (unit_definition->unit.grenade_type>=0 &&
			unit_definition->unit.grenade_type<NUMBER_OF_UNIT_GRENADE_TYPES &&
			unit_definition->unit.grenade_count>=0)
		{
			unit->unit.grenade_counts[unit_definition->unit.grenade_type] =
				(char)unit_definition->unit.grenade_count;
		}

		unit->object.flags |=
			FLAG(_object_dynamic_lighting_recompute_bit) |
			FLAG(_object_static_lighting_recompute_bit);

		if (unit_definition->unit.feign_death_threshold>0.f &&
			unit_definition->unit.feign_death_time>0.f &&
			unit_definition->unit.feign_death_chance>0.f)
		{
			SET_FLAG(
				unit->unit.flags,
				_unit_feign_death_allowed_bit,
				real_seed_random(get_global_random_seed_address()) <
					unit_definition->unit.feign_death_chance);
		}

		if (!game_engine_running() &&
			(unit->object.owner_team_index==0 ||
			unit->object.owner_team_index==NONE))
		{
			unit->object.owner_team_index = unit_definition->unit.default_team;
		}

		unit_set_or_test_seat_and_weapon_label(
			unit_index,
			base_seat_labels[_unit_base_seat_stand],
			NULL,
			TRUE);
		unit_add_initial_weapons(unit_index);

		for (seat_index = 0;
			seat_index<unit_definition->unit.seats.count;
			seat_index++)
		{
			struct unit_seat *seat = TAG_BLOCK_GET_ELEMENT(
				&unit_definition->unit.seats,
				seat_index,
				struct unit_seat);

			if (seat->built_in_actor_reference.index!=NONE)
			{
				ai_create_mounted_weapons_for_unit(unit_index);
				break;
			}
		}

		result = TRUE;
	}

	return result;
}

boolean unit_has_weapon(
	long unit_index,
	long weapon_index)
{
	struct unit_datum *unit = unit_get(unit_index);
	long index;
	boolean has_weapon = FALSE;

	for (index = 0; index<MAXIMUM_WEAPONS_PER_UNIT; ++index)
	{
		if (unit->unit.weapon_object_indices[index]==weapon_index)
		{
			has_weapon = TRUE;
			break;
		}
	}

	return has_weapon;
}

void unit_set_mouth_aperture(
	long unit_index,
	real mouth_aperture)
{
	struct unit_datum *unit = unit_get(unit_index);
	real *current_mouth_aperture = &unit->unit.mouth_aperture;

	*current_mouth_aperture += PIN(mouth_aperture - *current_mouth_aperture, -0.3f, 0.3f);

	return;
}

void unit_destroy(
	long unit_index)
{
	object_destroy(unit_index);
	unit_test_spawning(unit_index);

	return;
}

static void unit_drop_inventory_weapons(
	long unit_index)
{
	struct unit_datum *unit;
	struct weapon_datum *weapon;
	long *weapon_object_index;
	long weapon_index;
	short inventory_index;

	unit = unit_get(unit_index);
	inventory_index = 0;
	weapon_object_index = unit->unit.weapon_object_indices;
	do
	{
		weapon_index = *weapon_object_index;
		if (weapon_index != NONE &&
			inventory_index != unit->unit.current_weapon_index)
		{
			weapon = weapon_get(weapon_index);
			match_vassert(
				"c:\\halo\\SOURCE\\units\\units.c",
				8505,
				!TEST_FLAG(weapon->object.flags, _object_connected_to_map_bit),
				csprintf(
					temporary,
					"a %s tried to drop a %s which was connected to the map.",
					tag_get_name(unit->definition_index),
					tag_get_name(weapon->definition_index)));

			unit_drop_item(unit_index, weapon_index);
			if (inventory_index == unit->unit.desired_weapon_index)
				unit->unit.desired_weapon_index = unit->unit.current_weapon_index;

			*weapon_object_index = NONE;
			if (!weapon_can_be_fired(weapon_index))
				object_delete(weapon_index);
		}

		inventory_index++;
		weapon_object_index++;
	} while (inventory_index < MAXIMUM_WEAPONS_PER_UNIT);

	return;
}

void unit_died(
	long unit_index,
	boolean feigned)
{
	struct unit_datum *unit;
	struct unit_datum *fresh_unit;
	struct unit_definition *unit_definition;
	struct actor_datum *actor;
	long equipment_index;

	unit = unit_get(unit_index);
	if (!feigned)
	{
		unit->unit.feign_death_timer = 0;
	}
	else
	{
		match_assert(
			"c:\\halo\\SOURCE\\units\\units.c",
			5099,
			unit->unit.feign_death_timer > 0);
		unit_definition = unit_definition_get(unit->definition_index);
		if (real_seed_random(get_global_random_seed_address()) <
			unit_definition->unit.feign_death_repeat_chance)
		{
			SET_FLAG(unit->unit.flags, _unit_feign_death_allowed_bit, TRUE);
		}
		else
		{
			SET_FLAG(unit->unit.flags, _unit_feign_death_allowed_bit, FALSE);
		}
	}

	if (!feigned)
	{
		object_set_garbage(unit_index, TRUE);
		if (unit->unit.player_index != NONE)
		{
			player_died(unit->unit.player_index);
			unit->unit.player_index = NONE;
		}

		if (unit->unit.actor_index != NONE)
		{
			actor = actor_get(unit->unit.actor_index);
			unit->unit.fake_encounter_index = actor->meta.encounter_index;
			unit->unit.fake_squad_index = actor->meta.squad_index;
			actor_died(unit->unit.actor_index);
			unit->unit.actor_index = NONE;
		}

		if (unit->unit.swarm_actor_index != NONE)
		{
			actor = actor_get(unit->unit.swarm_actor_index);
			unit->unit.fake_encounter_index = actor->meta.encounter_index;
			unit->unit.fake_squad_index = actor->meta.squad_index;
			actor_swarm_unit_died(unit->unit.swarm_actor_index, unit_index);
			unit->unit.swarm_actor_index = NONE;
		}

		unit->unit.time_of_death = game_time_get();
	}

	unit->unit.flags &= ~(
		FLAG(_unit_actively_controlled_bit) |
		FLAG(_unit_active_camouflaged_bit));
	unit->unit.control_flags = 0;
	if (unit->unit.current_weapon_index != NONE)
	{
		weapon_owner_update(
			unit_inventory_get_weapon(
				unit_index,
				(word)unit_get(unit_index)->unit.current_weapon_index),
			0,
			0.0f);
	}

	fresh_unit = unit_get(unit_index);
	fresh_unit->unit.flags &= ~FLAG(_unit_running_blindly_bit);
	if (unit->object.parent_object_index != NONE)
	{
		if (unit->unit.parent_seat_index != NONE)
			unit_exit_seat_end(unit_index);
		else
			unit_detach_from_parent(unit_index);
	}

	unit->unit.speech.queued.priority = 0;
	unit_drop_inventory_weapons(unit_index);
	fresh_unit = unit_get(unit_index);
	if (fresh_unit->unit.equipment_object_index != NONE)
	{
		equipment_index = fresh_unit->unit.equipment_object_index;
		unit_drop_item(unit_index, equipment_index);
		fresh_unit->unit.equipment_object_index = NONE;
	}

	unit_drop_grenades(unit_index);
	if (unit->unit.weapon_drop_delay_ticks == 0)
		unit_drop_current_weapon(unit_index, TRUE);

	unit->unit.animation.overlay_action_animation.index = NONE;
	unit->unit.animation.action_animation.index = NONE;
	unit->unit.melee_attack_state = 0;
	if (unit->unit.grenade_throw_state == 1)
		unit->unit.grenade_throw_state = 0;

	return;
}

static void unit_drop_grenades(
	long unit_index)
{
	struct unit_datum *unit;
	struct object_placement_data placement_data;
	unsigned long grenade_count_negative_base;
	char *grenade_count;
	long grenade_object_index;
	long grenade_type_count;
	struct game_globals_grenade *grenade;

	unit = unit_get(unit_index);
	grenade_count = unit->unit.grenade_counts;
	grenade_count_negative_base =
		0u - (unsigned long)unit->unit.grenade_counts;
	grenade_type_count = NUMBER_OF_UNIT_GRENADE_TYPES;
	do
	{
		grenade = TAG_BLOCK_GET_ELEMENT(
			&scenario_get_game_globals()->grenades,
			grenade_count_negative_base + (unsigned long)grenade_count,
			struct game_globals_grenade);
		while (*grenade_count > 0)
		{
			object_placement_data_new(
				&placement_data,
				grenade->item.index,
				unit_index);
			grenade_object_index = object_new(&placement_data);
			if (grenade_object_index != NONE)
			{
				object_disconnect_from_map(grenade_object_index);
				unit_drop_item(unit_index, grenade_object_index);
			}

			(*grenade_count)--;
		}

		grenade_count++;
	} while (--grenade_type_count);

	return;
}

void unit_get_facing_vector(
	long unit_index,
	real_vector3d *facing_vector)
{
	object_get_orientation(unit_index, facing_vector, NULL);

	return;
}

void unit_get_center_of_mass(
	long unit_index,
	real_point3d *center_of_mass)
{
	struct object_marker body_marker;

	object_get_marker_by_name(unit_index, "body", &body_marker, 1);
	*center_of_mass = body_marker.matrix.position;

	return;
}

void unit_get_aiming_vector(
	long unit_index,
	real_vector3d *aiming_vector)
{
	*aiming_vector = unit_get(unit_index)->unit.aiming_vector;

	return;
}

boolean unit_can_see_point(
	long unit_index,
	real_point3d const *point,
	real field_of_view)
{
	boolean can_see_point = FALSE;

	if (unit_index!=NONE)
	{
		struct unit_datum *unit = unit_get(unit_index);
		struct object_marker head_marker;
		real_vector3d direction;

		object_get_marker_by_name(unit_index, "head", &head_marker, 1);
		vector_from_points3d(&head_marker.matrix.position, point, &direction);
		normalize3d(&direction);

		if (dot_product3d(&direction, &unit->unit.looking_vector)>cosine(field_of_view))
		{
			can_see_point = TRUE;
		}
	}

	return can_see_point;
}

boolean unit_has_animation_to_enter_seat(
	long unit_index,
	long target_unit_index,
	short seat_index)
{
	struct unit_datum *target_unit = unit_get(target_unit_index);
	struct unit_definition *target_unit_definition = unit_definition_get(target_unit->definition_index);
	boolean has_animation = FALSE;

	if (seat_index>=0 && seat_index<target_unit_definition->unit.seats.count)
	{
		struct unit_datum *unit = unit_get(unit_index);

		if (unit->object.type==_object_type_vehicle)
		{
			has_animation = TRUE;
		}
		else
		{
			struct unit_seat *seat = TAG_BLOCK_GET_ELEMENT(&target_unit_definition->unit.seats, seat_index, struct unit_seat);

			if (unit_set_or_test_seat_and_weapon_label(unit_index, seat->label, NULL, FALSE))
			{
				has_animation = TRUE;
			}
		}
	}

	return has_animation;
}

boolean unit_can_enter_seat(
	long unit_index,
	long target_unit_index,
	short seat_index,
	long *occupant_unit_index)
{
	struct unit_datum *unit = unit_get(unit_index);
	struct unit_datum *target_unit = unit_get(target_unit_index);
	long occupant_index = NONE;
	boolean result = TRUE;
	long child_object_index;

	if (unit_index==target_unit_index)
	{
		result = FALSE;
	}

	child_object_index = target_unit->object.first_child_object_index;

	while (child_object_index!=NONE)
	{
		struct object_datum *child_object = object_get(child_object_index);

		if (TEST_FLAG(_object_mask_unit, child_object->object.type))
		{
			struct unit_datum *child_unit = unit_get(child_object_index);

			if (child_unit->unit.parent_seat_index==seat_index)
			{
				occupant_index = child_object_index;
				result = FALSE;
			}
			else if (unit->unit.player_index!=NONE &&
				game_team_is_enemy(
					unit->object.owner_team_index,
					child_object->object.owner_team_index))
			{
				result = FALSE;
			}
		}

		child_object_index = child_object->object.next_object_index;
	}

	if (occupant_unit_index)
	{
		*occupant_unit_index = occupant_index;
	}

	return result;
}

static void unit_adjust_for_seat_change(
	long unit_index)
{
	struct unit_datum *unit = unit_get(unit_index);

	unit->unit.desired_weapon_index = unit_weapon_next_index(
		unit_index,
		unit->unit.current_weapon_index,
		0);
	unit_ready_desired_weapon(unit_index, TRUE);

	return;
}

boolean unit_enter_seat(
	long unit_index,
	long target_unit_index,
	short seat_index)
{
	boolean result = FALSE;

	/* port: and fights on foot */
	if (infection_unit_is_zombie(unit_index))
		return FALSE;
	if (unit_can_enter_seat(unit_index, target_unit_index, seat_index, NULL))
	{
		struct unit_datum *unit = unit_get(unit_index);
		real_vector3d offset;
		struct unit_definition *unit_definition;
		struct animation_graph *animation_graph;
		struct animation_graph_unit_seat *animation_seat;
		long animation_graph_index;

		{
			struct unit_datum *target_unit = unit_get(target_unit_index);
			struct unit_definition *target_unit_definition =
				unit_definition_get(target_unit->definition_index);
			struct unit_seat *seat = TAG_BLOCK_GET_ELEMENT(
				&target_unit_definition->unit.seats,
				seat_index,
				struct unit_seat);
			real_point3d origin;
			struct object_marker seat_marker;
			struct unit_datum *updated_unit;
			long weapon_index;
			char const *weapon_label;

			match_assert(
				"c:\\halo\\SOURCE\\units\\units.c",
				4245,
				unit->object.parent_object_index==NONE);

			object_get_origin(unit_index, &origin);
			object_get_marker_by_name(
				target_unit_index,
				seat->marker_name,
				&seat_marker,
				1);
			vector_from_points3d(
				&seat_marker.matrix.position,
				&origin,
				&offset);
			matrix4x3_inverse_transform_vector(
				&seat_marker.matrix,
				&offset,
				&offset);
			object_attach_to_marker(
				target_unit_index,
				seat->marker_name,
				unit_index,
				"");

			unit->unit.parent_seat_index = seat_index;
			unit->object.parent_object_index = target_unit_index;
			unit_update_driver_and_gunner(target_unit_index);

			updated_unit = unit_get(unit_index);
			updated_unit->unit.desired_weapon_index = unit_weapon_next_index(
				unit_index,
				updated_unit->unit.current_weapon_index,
				0);
			unit_ready_desired_weapon(unit_index, TRUE);
			weapon_index = unit_get_current_weapon_index(unit_index);
			weapon_label = weapon_index==NONE ?
				"unarmed" : weapon_get_label(weapon_index);
			if (!unit_set_or_test_seat_and_weapon_label(
				unit_index,
				seat->label,
				weapon_label,
				TRUE))
			{
				unit_set_or_test_seat_and_weapon_label(
					unit_index,
					seat->label,
					NULL,
					TRUE);
			}
		}

		unit_definition = unit_definition_get(unit->definition_index);
		animation_graph_index = unit_definition->object.animation_graph.index;
		animation_graph = animation_graph_definition_get(
			animation_graph_index);
		animation_seat = TAG_BLOCK_GET_ELEMENT(
			&animation_graph->unit_seats,
			unit->unit.animation.seat_index,
			struct animation_graph_unit_seat);
		if (animation_seat->animations.count>_unit_seat_animation_seat_enter)
		{
			short animation_index = animation_graph_animation_index_get(
				&animation_seat->animations)
					[_unit_seat_animation_seat_enter].animation_index;

			if (animation_index!=NONE)
			{
				object_start_interpolation(unit_index, 6);
				animation_graph_index =
					unit_definition->object.animation_graph.index;
				animation_index = animation_choose_random_permutation_internal(
					TRUE,
					animation_graph_index,
					animation_index);
				animation_graph_index =
					unit_definition->object.animation_graph.index;
				unit_set_animation(
					unit_index,
					animation_graph_index,
					animation_index);
				unit->unit.animation.state = _unit_state_entering_seat;
				object_offset_interpolation(unit_index, &offset);
				object_compute_node_matrices_recursive(unit_index);
			}
		}

		ai_handle_enter_vehicle(unit_index, target_unit_index);
		unit_unzoom(unit_index);
		result = TRUE;
	}

	return result;
}

void unit_exit_seat_end(
	long unit_index)
{
	struct unit_datum *unit = unit_get(unit_index);
	long parent_unit_index = unit->object.parent_object_index;

	if (parent_unit_index!=NONE && unit->unit.parent_seat_index!=NONE)
	{
		struct unit_datum *parent_unit = unit_get(parent_unit_index);
		struct unit_definition *parent_unit_definition =
			unit_definition_get(parent_unit->definition_index);
		struct unit_seat *seat = TAG_BLOCK_GET_ELEMENT(
			&parent_unit_definition->unit.seats,
			unit->unit.parent_seat_index,
			struct unit_seat);
		real_matrix4x3 *unit_node_matrix = object_get_node_matrix(unit_index, 0);
		struct object_marker seat_marker;
		real_vector3d relative_position;
		real_vector3d exit_offset;
		struct unit_definition *unit_definition;
		struct model *model;
		struct model_node *root_node;
		real_point3d root_translation;
		real_matrix4x3 const *root_inverse_matrix;
		real_matrix4x3 const *updated_unit_node_matrix;
		real_point3d position;
		real_matrix4x3 matrix;
		struct real_orientation *node_orientations;

		object_get_marker_by_name(
			parent_unit_index,
			seat->marker_name,
			&seat_marker,
			1);
		vector_from_points3d(
			&seat_marker.matrix.position,
			&unit_node_matrix->position,
			&relative_position);
		matrix4x3_inverse_transform_vector(
			&seat_marker.matrix,
			&relative_position,
			&exit_offset);

		unit_definition = unit_definition_get(unit->definition_index);
		model = model_definition_get(unit_definition->object.model.index);
		root_node = TAG_BLOCK_GET_ELEMENT(&model->nodes, 0, struct model_node);
		root_inverse_matrix = &root_node->runtime_default_inverse_matrix;
		root_translation = root_node->default_translation;

		if (parent_unit->unit.driver_object_index==unit_index &&
			parent_unit->unit.animation.state!=_unit_state_opening &&
			unit->object.parent_object_index!=NONE)
		{
			unit_animation_set_state(
				unit->object.parent_object_index,
				_unit_state_opening);
		}

		unit->unit.last_vehicle_index = parent_unit_index;
		unit->unit.game_time_at_last_vehicle_exit = game_time_get();
		if (unit->unit.driver_object_index==unit_index)
		{
			unit->unit.driver_object_index = NONE;
		}
		if (unit->unit.gunner_object_index==unit_index)
		{
			unit->unit.gunner_object_index = NONE;
		}

		object_detach(unit_index);
		position.x = relative_position.i + unit->object.position.x;
		position.y = relative_position.j + unit->object.position.y;
		position.z = relative_position.k + unit->object.position.z -
			root_translation.z;
		object_set_position(unit_index, &position, NULL, NULL);

		updated_unit_node_matrix = object_get_node_matrix(unit_index, 0);
		matrix4x3_multiply(
			updated_unit_node_matrix,
			root_inverse_matrix,
			&matrix);
		unit->object.forward = matrix.forward;
		unit->object.up = matrix.up;
		object_set_visibility(unit_index, TRUE);
		unit->unit.parent_seat_index = NONE;
		unit->unit.animation.base_seat_index = _unit_base_seat_stand;

		if (parent_unit->unit.driver_object_index==unit_index)
		{
			parent_unit->unit.driver_object_index = NONE;
		}
		if (parent_unit->unit.gunner_object_index==unit_index)
		{
			parent_unit->unit.gunner_object_index = NONE;
		}

		unit_update_driver_and_gunner(parent_unit_index);
		unit_adjust_for_seat_change(unit_index);

		{
			struct unit_animation_update_data animation_update_data;

			animation_update_data.state_desired = _unit_state_airborne;
			animation_update_data.crouching = FALSE;
			unit_update_animation(unit_index, &animation_update_data);
		}
		node_orientations = object_header_block_get(
			unit_index,
			&unit->object.original_node_orientations);
		node_orientations->translation = root_translation;

		if (unit->object.type==_object_type_biped)
		{
			biped_exit_seat_end(unit_index, parent_unit_index);
		}
		object_compute_node_matrices_recursive(unit_index);
	}

	return;
}

void unit_impulse(
	long unit_index,
	long impulse_index,
	real_vector3d const *impulse,
	real magnitude)
{
	struct unit_datum *unit = unit_get(unit_index);
	struct unit_definition *unit_definition = unit_definition_get(unit->definition_index);

	if (unit_definition->object.physics.index!=NONE)
	{
		struct physics_definition *physics_definition = physics_definition_get(unit_definition->object.physics.index);
		real scale = magnitude/physics_definition->mass;

		unit->object.translational_velocity.i += scale*impulse->i;
		unit->object.translational_velocity.j += scale*impulse->j;
		unit->object.translational_velocity.k += scale*impulse->k;
	}

	return;
}

/* the platform layer's (port/linux/src/port_config.c) */
int config_boolean(const char *name);
unsigned long config_changes(void);

/* port: the animation enhancements (game.enhanced_animations, on unless
config.toml turns them off) are for the player bipeds (cyborg and
cyborg_mp), whose animations they were made with; every other unit keeps
the original animations */
static char const *const enhanced_animation_units[] =
{
	"cyborg",
	"cyborg_mp",
};

/* whether the unit is one of the player bipeds the enhancements were made
for */
static boolean unit_animation_enhanced_unit(
	long unit_index)
{
	struct unit_datum *unit = unit_get(unit_index);
	char const *name = tag_name_strip_path(tag_get_name(unit->definition_index));
	long name_index;

	for (name_index = 0; name_index < NUMBEROF(enhanced_animation_units); name_index++)
	{
		if (!_stricmp(name, enhanced_animation_units[name_index]))
		{
			return TRUE;
		}
	}

	return FALSE;
}

boolean unit_animation_enhanced(
	long unit_index)
{
	static boolean enhanced = TRUE;
	static unsigned long read_at = (unsigned long)-1;

	/* (read again when the settings change) */
	if (read_at != config_changes())
	{
		read_at = config_changes();
		enhanced = config_boolean("game.enhanced_animations") != 0;
	}
	if (!enhanced)
	{
		return FALSE;
	}
	return unit_animation_enhanced_unit(unit_index);
}

/* port: whether a player biped turns with the aim where the original left
its body facing (throwing a grenade, in the air: bipeds.c's
biped_update_turning). It moves where a shot goes, not only how the body
looks, so in a network game every machine turns them, whatever its own
game.enhanced_animations says: a player who turned it off would otherwise
aim differently from the others, and a client and its host would turn that
player's body differently. In a game of one machine the setting decides,
as for the other enhancements. */
boolean unit_turns_with_aim(
	long unit_index)
{
	if (game_connection() == _game_connection_network_client ||
		game_connection() == _game_connection_network_server)
	{
		return unit_animation_enhanced_unit(unit_index);
	}
	return unit_animation_enhanced(unit_index);
}

/* port: a seat whose graph has no reload of its own (a Scorpion rider's)
reloads with the standing seat's, the same weapon's: a reload is a
replacement animation, which moves only the arms and the weapon */
static short unit_standing_weapon_type_animation(
	struct animation_graph *animation_graph,
	struct animation_graph_weapon_class *weapon_class,
	struct animation_graph_weapon_type *weapon_type,
	short weapon_type_animation_index)
{
	short seat_index;

	for (seat_index = 0; seat_index < animation_graph->unit_seats.count; seat_index++)
	{
		struct animation_graph_unit_seat *seat = TAG_BLOCK_GET_ELEMENT(&animation_graph->unit_seats, seat_index, struct animation_graph_unit_seat);
		short class_index;

		if (_stricmp(seat->label, "stand"))
		{
			continue;
		}
		for (class_index = 0; class_index < seat->weapon_classes.count; class_index++)
		{
			struct animation_graph_weapon_class *stand_class = TAG_BLOCK_GET_ELEMENT(&seat->weapon_classes, class_index, struct animation_graph_weapon_class);
			short type_index;

			if (_stricmp(stand_class->label, weapon_class->label))
			{
				continue;
			}
			for (type_index = 0; type_index < stand_class->weapon_types.count; type_index++)
			{
				struct animation_graph_weapon_type *stand_type = TAG_BLOCK_GET_ELEMENT(&stand_class->weapon_types, type_index, struct animation_graph_weapon_type);

				if (!_stricmp(stand_type->label, weapon_type->label) &&
					weapon_type_animation_index < stand_type->animations.count)
				{
					return animation_graph_animation_index_get(&stand_type->animations)[weapon_type_animation_index].animation_index;
				}
			}
		}
	}

	return NONE;
}

void unit_animation_start_action(
	long unit_index,
	short action)
{
	struct unit_datum *unit = unit_get(unit_index);

	if (action==0)
	{
		unit->unit.animation.action = 0;
		unit->unit.animation.action_animation.index = NONE;
	}
	else
	{
		struct unit_definition *unit_definition = unit_definition_get(unit->definition_index);
		struct animation_graph *animation_graph = animation_graph_definition_get(unit_definition->object.animation_graph.index);
		struct animation_graph_unit_seat *unit_seat = TAG_BLOCK_GET_ELEMENT(&animation_graph->unit_seats, unit->unit.animation.seat_index, struct animation_graph_unit_seat);
		struct animation_graph_weapon_class *weapon_class = TAG_BLOCK_GET_ELEMENT(&unit_seat->weapon_classes, unit->unit.animation.weapon_index, struct animation_graph_weapon_class);
		struct animation_graph_weapon_type *weapon_type = TAG_BLOCK_GET_ELEMENT(&weapon_class->weapon_types, unit->unit.animation.weapon_type_index, struct animation_graph_weapon_type);
		long action_index = action;
		long animation_graph_index;
		short animation_index = NONE;
		short weapon_class_animation_index = NONE;
		short weapon_type_animation_index = NONE;
		short interpolation_frame_count;

		switch (action_index)
		{
		case 1:
			weapon_class_animation_index = _unit_weapon_class_animation_disarm;
			break;
		case 2:
			weapon_class_animation_index = _unit_weapon_class_animation_drop;
			break;
		case 3:
			weapon_class_animation_index = _unit_weapon_class_animation_ready;
			break;
		case 4:
			weapon_class_animation_index = _unit_weapon_class_animation_put_away;
			break;
		case 5:
			weapon_type_animation_index = 0;
			goto lookup_weapon_type_animation;
		case 6:
			weapon_type_animation_index = 1;
			goto lookup_weapon_type_animation;
		case 7:
			weapon_type_animation_index = 8;
			goto lookup_weapon_type_animation;
		case 8:
			weapon_class_animation_index = _unit_weapon_class_animation_throw_grenade;
			break;
		case 9:
			weapon_type_animation_index = 9;
		lookup_weapon_type_animation:
			if (weapon_type_animation_index<weapon_type->animations.count)
			{
				animation_index = animation_graph_animation_index_get(&weapon_type->animations)[weapon_type_animation_index].animation_index;
			}
			else
			{
				animation_index = NONE;
			}
			break;
		default:
			break;
		}

		if (weapon_class_animation_index!=NONE)
		{
			if (weapon_class_animation_index<weapon_class->animations.count)
			{
				animation_index = animation_graph_animation_index_get(&weapon_class->animations)[weapon_class_animation_index].animation_index;
			}
			else
			{
				animation_index = NONE;
			}
		}
		/* port: a seat with no reload borrows the standing one's */
		if (animation_index == NONE &&
			(weapon_type_animation_index == 0 || weapon_type_animation_index == 1) &&
			unit_animation_enhanced(unit_index))
		{
			animation_index = unit_standing_weapon_type_animation(
				animation_graph,
				weapon_class,
				weapon_type,
				weapon_type_animation_index);
		}
		interpolation_frame_count = action_index==7 ? 0 : 6;

		if (animation_index!=NONE)
		{
			if (interpolation_frame_count>0)
			{
				object_start_interpolation(unit_index, interpolation_frame_count);
			}

			animation_graph_index = unit_definition->object.animation_graph.index;
			unit->unit.animation.action_animation.index = animation_choose_random_permutation_internal(TRUE, animation_graph_index, animation_index);
			unit->unit.animation.action_animation.frame_index = 0;
			unit->unit.animation.action = (char)action;
		}
	}

	return;
}

static void unit_animation_start_overlay_action(
	long unit_index,
	short overlay_action)
{
	struct unit_datum *unit = unit_get(unit_index);

	if (overlay_action >= unit->unit.animation.overlay_action)
	{
		switch (unit->unit.animation.state)
		{
			case _unit_state_hard_ping:
			case _unit_state_dying_airborne:
			case _unit_state_dying:
			case _unit_state_entering_seat:
			case _unit_state_exiting_seat:
			case _unit_state_ai_impulse:
			case _unit_state_melee_attack:
			case _unit_state_melee_airborne:
			case _unit_state_melee_continuous:
			case _unit_state_throw_grenade:
			case _unit_state_resurrect_front:
			case _unit_state_resurrect_back:
			case _unit_state_leap_start:
			case _unit_state_leap_melee:
				break;

			default:
			{
				struct unit_definition *unit_definition = unit_definition_get(unit->definition_index);
				struct animation_graph *animation_graph = animation_graph_definition_get(unit_definition->object.animation_graph.index);
				struct animation_graph_unit_seat *unit_seat = TAG_BLOCK_GET_ELEMENT(&animation_graph->unit_seats, unit->unit.animation.seat_index, struct animation_graph_unit_seat);
				struct animation_graph_weapon_class *weapon_class = TAG_BLOCK_GET_ELEMENT(&unit_seat->weapon_classes, unit->unit.animation.weapon_index, struct animation_graph_weapon_class);
				struct animation_graph_weapon_type *weapon_type = TAG_BLOCK_GET_ELEMENT(&weapon_class->weapon_types, unit->unit.animation.weapon_type_index, struct animation_graph_weapon_type);
				short anim_slot = NONE;
				short animation_index;

				switch (overlay_action)
				{
					case 1: anim_slot = _unit_weapon_class_animation_diving_front; break;
					case 2: anim_slot = _unit_weapon_class_animation_diving_back; break;
					case 3: anim_slot = _unit_weapon_class_animation_diving_left; break;
					case 4: anim_slot = _unit_weapon_class_animation_diving_right; break;
					case 5: anim_slot = _unit_weapon_class_animation_turning_left; break;
					case 6: anim_slot = _unit_weapon_class_animation_turning_right; break;
				}

				animation_index = (anim_slot>=0 && anim_slot<weapon_type->animations.count) ? animation_graph_animation_index_get(&weapon_type->animations)[anim_slot].animation_index : NONE;

				if (animation_index != NONE)
				{
					long graph_index = unit_definition->object.animation_graph.index;

					unit->unit.animation.overlay_action_animation.index = animation_choose_random_permutation_internal(TRUE, graph_index, animation_index);
					unit->unit.animation.overlay_action_animation.frame_index = 0;
					unit->unit.animation.overlay_action = (char)overlay_action;
				}
				else if (debug_unit_animations && unit->object.type==_object_type_biped && unit->unit.animation.aiming_screen_index==NONE)
				{
					console_warning("MISSING: %s '%s %s'",
						tag_name_strip_path(unit_definition->object.animation_graph.name),
						unit_seat->label,
						weapon_class->label,
						weapon_type->label,
						animation_list_get_string(&weapon_type_animation_list, anim_slot));
				}
			}
			break;
		}
	}

	return;
}

void unit_handle_weapon_state_change(
	long unit_index,
	short weapon_state)
{
	short action = NONE;
	short overlay_action = NONE;

	switch (weapon_state)
	{
	case 5:
		action = 5;
		break;
	case 6:
		action = 6;
		break;
	case 3:
		overlay_action = _unit_weapon_overlay_primary_chamber;
		break;
	case 4:
		overlay_action = _unit_weapon_overlay_secondary_chamber;
		break;
	case 1:
		overlay_action = _unit_weapon_overlay_primary_recoil;
		break;
	case 2:
		overlay_action = _unit_weapon_overlay_secondary_recoil;
		break;
	case 7:
		overlay_action = _unit_weapon_overlay_primary_charged;
		break;
	case 8:
		overlay_action = _unit_weapon_overlay_secondary_charged;
		break;
	default:
		break;
	}

	if (action != NONE)
	{
		unit_animation_start_action(unit_index, action);
	}
	else if (overlay_action != NONE)
	{
		unit_animation_start_overlay_action(unit_index, overlay_action);
	}

	return;
}

void unit_scripting_set_seat(
	long unit_index,
	char const *seat_label)
{
	if (unit_index!=NONE)
	{
		struct unit_datum *unit = unit_get(unit_index);
		unit->unit.magic_seat_index = seat_label_to_base_seat_index(seat_label);
	}

	return;
}

void unit_handle_deleted_object(
	long unit_index,
	long deleted_object_index)
{
	struct unit_datum *unit;
	short weapon_index;
	long *weapon_object_index;

	unit = unit_get(unit_index);
	if (unit->unit.grenade_object_index==deleted_object_index)
	{
		unit->unit.grenade_object_index = NONE;
	}

	if (unit->unit.driver_object_index==deleted_object_index)
	{
		unit->unit.driver_object_index = NONE;
	}

	if (unit->unit.gunner_object_index==deleted_object_index)
	{
		unit->unit.gunner_object_index = NONE;
	}

	weapon_index = 0;
	weapon_object_index = unit->unit.weapon_object_indices;
	do
	{
		if (*weapon_object_index==deleted_object_index)
		{
			*weapon_object_index = NONE;
			if (weapon_index==unit->unit.desired_weapon_index)
			{
				unit->unit.desired_weapon_index = NONE;
			}

			if (weapon_index==unit->unit.current_weapon_index)
			{
				unit->unit.current_weapon_index = NONE;
			}
		}

		++weapon_index;
		++weapon_object_index;
	}
	while (weapon_index<MAXIMUM_WEAPONS_PER_UNIT);

	if (unit->unit.current_weapon_index==NONE)
	{
		unit->unit.desired_weapon_index = unit_weapon_next_index(unit_index, NONE, 0);
	}

	if (unit->unit.equipment_object_index==deleted_object_index)
	{
		unit->unit.equipment_object_index = NONE;
	}

	if (unit->unit.delayed_damage_attacker_object_index==deleted_object_index)
	{
		unit->unit.delayed_damage_attacker_object_index = NONE;
	}

	return;
}

void unit_get_looking_vector(
	long unit_index,
	real_vector3d *looking_vector)
{
	*looking_vector = unit_get(unit_index)->unit.looking_vector;

	return;
}

long unit_scripting_unit_driver(
	long unit_index)
{
	struct unit_datum *unit = unit_try_and_get(unit_index);
	long driver_object_index = NONE;

	if (unit)
	{
		driver_object_index = unit->unit.driver_object_index;
	}

	return driver_object_index;
}

long unit_scripting_unit_gunner(
	long unit_index)
{
	struct unit_datum *unit = unit_try_and_get(unit_index);
	long gunner_object_index = NONE;

	if (unit)
	{
		gunner_object_index = unit->unit.gunner_object_index;
	}

	return gunner_object_index;
}

void unit_shield_sapping_update(
	long unit_index)
{
	struct unit_datum *unit = unit_get(unit_index);
	struct unit_definition *unit_definition =
		unit_definition_get(unit->definition_index);

	if (unit->unit.animation.state == _unit_state_shield_sapping)
	{
		struct animation_graph *animation_graph =
			animation_graph_definition_get(
				unit->object.animation.animation_graph_index);
		struct animation *animation = TAG_BLOCK_GET_ELEMENT(
			&animation_graph->animations,
			unit->object.animation.state.index,
			struct animation);

		/*
		 * Original bug: January compares the animation index to the loop
		 * frame. A behavior-corrected build would use state.frame_index.
		 */
		if (unit->object.animation.state.index >=
			animation->private_loop_frame_index)
		{
			struct data_iterator iterator;
			struct player_datum *player;
			boolean shield_sapped = FALSE;

			data_iterator_new(&iterator, player_data);
			while ((player = data_iterator_next(&iterator)) != NULL)
			{
				if (player->unit_index != NONE)
				{
					struct unit_datum *player_unit =
						unit_get(player->unit_index);

					if (distance_squared3d(
						&player_unit->object.bounding_sphere_center,
						&unit->object.bounding_sphere_center) < 16.f)
					{
						struct damage_data damage_data;

						damage_data_new(
							&damage_data,
							unit_definition->unit.melee_damage.index);
						damage_data.owner_object_index = unit_index;
						object_cause_damage(
							&damage_data,
							player->unit_index,
							NONE,
							NONE,
							NONE,
							NULL);
						shield_sapped = TRUE;
					}
				}
			}

			if (!shield_sapped)
			{
				unit->unit.shield_sap_timeout++;
			}
			else
			{
				unit->unit.shield_sap_timeout = 0;
			}
		}
	}

	return;
}

void unit_detach_from_parent(
	long unit_index)
{
	struct unit_datum *unit = unit_get(unit_index);

	if (unit->object.parent_object_index != NONE)
	{
		long parent_index = unit->object.parent_object_index;
		real_point3d parent_origin;
		real_point3d unit_origin;
		real_vector3d velocity;

		unit_get(unit->object.parent_object_index);
		object_get_origin(
			unit->object.parent_object_index,
			&parent_origin);
		object_get_origin(unit_index, &unit_origin);
		vector_from_points3d(&parent_origin, &unit_origin, &velocity);
		if (normalize3d(&velocity) == 0.f)
		{
			velocity = unit->object.forward;
		}
		scale_vector3d(&velocity, 0.020000001f, &velocity);

		object_detach(unit_index);
		unit_origin = unit->object.position;
		scenario_ensure_point_within_world(&unit_origin);
		object_set_position(unit_index, &unit_origin, NULL, NULL);

		SET_FLAG(unit->object.flags, _object_at_rest_bit, FALSE);
		SET_FLAG(unit->unit.flags, _unit_attached_melee_attack_bit, FALSE);
		add_vectors3d(
			&velocity,
			&unit->object.translational_velocity,
			&unit->object.translational_velocity);
		object_set_visibility(unit_index, TRUE);
		object_compute_node_matrices(unit_index);
		/* port: a vehicle dropped by another comes with more in a large
		co-op game (port/linux/game/network_coop.c) */
		if (unit->object.type == _object_type_vehicle && object_get(parent_index)->object.type == _object_type_vehicle)
			network_coop_vehicle_dropped(unit_index, parent_index);
	}

	return;
}

long units_debug_get_next_unit(
	long current_object_index)
{
	struct object_iterator iterator;
	struct unit_datum *unit;
	long result = NONE;

	if (current_object_index != NONE)
	{
		object_iterator_new(&iterator, _object_mask_unit, 0);

		while ((unit = (struct unit_datum *)object_iterator_next(&iterator)) != NULL)
		{
			if (iterator.index == current_object_index)
			{
				break;
			}
		}

		while ((unit = (struct unit_datum *)object_iterator_next(&iterator)) != NULL)
		{
			if (unit->unit.actor_index == NONE &&
				unit->unit.swarm_actor_index == NONE &&
				!TEST_FLAG(unit->object.damage_flags, _object_dead_bit))
			{
				result = iterator.index;
				break;
			}
		}
	}

	if (result == NONE)
	{
		object_iterator_new(&iterator, _object_mask_unit, 0);

		while ((unit = (struct unit_datum *)object_iterator_next(&iterator)) != NULL)
		{
			if (unit->unit.actor_index == NONE &&
				unit->unit.swarm_actor_index == NONE &&
				!TEST_FLAG(unit->object.damage_flags, _object_dead_bit))
			{
				return iterator.index;
			}
		}
	}

	return result;
}

long units_debug_get_closest_unit(
	long reference_object_index)
{
	struct object_iterator iterator;
	struct object_datum *object;
	long closest_index = NONE;
	real closest_distance = REAL_MAX;

	object_iterator_new(&iterator, _object_mask_biped, 0);
	while ((object = (struct object_datum *)object_iterator_next(&iterator)) != NULL)
	{
		if (iterator.index != reference_object_index &&
			!TEST_FLAG(object->object.damage_flags, _object_dead_bit))
		{
			real distance;

			if (reference_object_index != NONE)
			{
				real_point3d reference_origin;
				real_point3d object_origin;

				object_get_origin(reference_object_index, &reference_origin);
				object_get_origin(iterator.index, &object_origin);
				distance = distance3d(&reference_origin, &object_origin);
			}
			else
			{
				distance = 0.f;
			}

			if (distance < closest_distance)
			{
				closest_index = iterator.index;
				closest_distance = distance;
			}
		}
	}

	return closest_index;
}

boolean unit_get_current_flashlight_state(
	long unit_index)
{
	boolean flashlight_on = FALSE;

	if (unit_index!=NONE)
	{
		flashlight_on = TEST_FLAG(unit_get(unit_index)->unit.flags, _unit_integrated_light_on_bit);
	}

	return flashlight_on;
}

void unit_start_running_blindly(
	long unit_index)
{
	struct unit_datum *unit = unit_get(unit_index);

	if (!TEST_FLAG(unit->unit.flags, _unit_running_blindly_bit))
	{
		real angle_range;
		real_vector3d run_vector;

		SET_FLAG(unit->unit.flags, _unit_running_blindly_bit, TRUE);
		if (unit->unit.actor_index != NONE)
		{
			if (actor_get_running_blind_vector(unit->unit.actor_index, &run_vector))
			{
				unit->unit.run_blindly_angle = 0.0f;
				angle_range = DEGREES_TO_RADIANS(25.0f);
				goto randomize_angle;
			}
		}

		{
			real_euler_angles2d facing_angles;

			euler_angles2d_from_vector3d(&facing_angles, &unit->object.forward);
			if (facing_angles.yaw > _pi)
				facing_angles.yaw -= 2.0f * _pi;
			unit->unit.run_blindly_angle = facing_angles.yaw;
			angle_range = DEGREES_TO_RADIANS(100.0f);
		}

randomize_angle:
		unit->unit.run_blindly_angle += real_seed_random_range(
			get_global_random_seed_address(),
			-angle_range,
			angle_range);
	}

	return;
}

void unit_stop_running_blindly(
	long unit_index)
{
	struct unit_datum *unit = unit_get(unit_index);

	SET_FLAG(unit->unit.flags, _unit_running_blindly_bit, FALSE);

	return;
}

long unit_get_aiming_unit_index(
	long unit_index)
{
	long aiming_unit_index = unit_index;

	if (unit_index!=NONE)
	{
		struct unit_datum *unit = unit_get(unit_index);

		if (unit->object.parent_object_index!=NONE && unit->unit.parent_seat_index!=NONE)
		{
			struct unit_datum *parent_unit = unit_get(unit->object.parent_object_index);
			struct unit_definition *parent_unit_definition = unit_definition_get(parent_unit->definition_index);
			struct unit_seat *seat = TAG_BLOCK_GET_ELEMENT(&parent_unit_definition->unit.seats, unit->unit.parent_seat_index, struct unit_seat);

			if (TEST_FLAG(seat->flags, _unit_seat_invisible_bit) || TEST_FLAG(seat->flags, _unit_seat_gunner_bit))
			{
				aiming_unit_index = unit->object.parent_object_index;
			}
		}
	}

	return aiming_unit_index;
}

void unit_set_emotion(
	long unit_index,
	word emotion_index)
{
	if (unit_index!=NONE)
	{
		unit_get(unit_index)->unit.animation.emotion_index = emotion_index;
		object_start_interpolation(unit_index, 6);
	}

	return;
}

boolean unit_is_playing_custom_animation(
	long unit_index)
{
	if (unit_index!=NONE)
	{
		return unit_get(unit_index)->unit.animation.state==_unit_state_user_animation;
	}

	return FALSE;
}

short unit_get_custom_animation_time(
	long unit_index)
{
	if (unit_index!=NONE)
	{
		struct unit_datum *unit = unit_get(unit_index);

		if (unit->unit.animation.state==_unit_state_user_animation)
		{
			struct animation_graph *animation_graph = animation_graph_definition_get(unit->object.animation.animation_graph_index);
			struct animation *animation = TAG_BLOCK_GET_ELEMENT(&animation_graph->animations, unit->object.animation.state.index, struct animation);
			return MAX(animation->frame_count - unit->object.animation.state.frame_index - 2, 0);
		}
	}

	return 0;
}

boolean unit_start_user_animation(
	long unit_index,
	long animation_graph_index,
	char const *animation_name,
	boolean interpolate)
{
	boolean animation_started = FALSE;

	if (unit_index != NONE)
	{
		if (animation_graph_index != NONE)
		{
			struct unit_datum *unit;
			struct animation_graph *animation_graph;
			short base_animation_index;

			unit = unit_get(unit_index);
			(void)unit_definition_get(unit->definition_index);
			animation_graph =
				animation_graph_definition_get(animation_graph_index);
			base_animation_index = animation_graph_get_animation_by_name(
				animation_graph_index,
				animation_name);

			if (base_animation_index != NONE)
			{
				struct animation *animation;
				short animation_index =
					animation_choose_random_permutation_internal(
						TRUE,
						animation_graph_index,
						base_animation_index);

				animation = TAG_BLOCK_GET_ELEMENT(
					&animation_graph->animations,
					animation_index,
					struct animation);

				if (animation->type != _animation_overlay &&
					animation->type == _animation_base)
				{
					boolean animation_allowed = TRUE;

					if (unit->unit.animation.state ==
							_unit_state_user_animation &&
						unit->object.animation.state.index != NONE)
					{
						struct animation *current_animation =
							TAG_BLOCK_GET_ELEMENT(
								&animation_graph->animations,
								unit->object.animation.state.index,
								struct animation);

						if (current_animation->runtime_parent_animation_index ==
							animation->runtime_parent_animation_index)
						{
							if (unit->object.animation.state.frame_index + 2 ==
								current_animation->private_key_frame_index)
							{
								--unit->object.animation.state.frame_index;
								animation_allowed = FALSE;
							}

							else if (unit->object.animation.state.frame_index <
								current_animation->private_key_frame_index)
							{
								animation_allowed = FALSE;
							}
						}
					}

					if (animation_allowed)
					{
						if (interpolate)
						{
							object_start_interpolation(unit_index, 6);
						}

						unit->unit.animation.state =
							_unit_state_user_animation;
						unit_set_animation(
							unit_index,
							animation_graph_index,
							animation_index);
						SET_FLAG(
							unit->unit.animation.flags,
							_unit_animation_postpone_weapon_ik_until_interpolation_ends_bit,
							TRUE);
						object_compute_node_matrices_recursive(unit_index);
						animation_started = TRUE;
						network_coop_note_unit_animation(unit_index, animation_graph_index, animation_index,
							interpolate);
						if (unit->unit.player_index == NONE)
						{
							network_actors_note_user_animation(unit_index, animation_graph_index, animation_index,
								interpolate);
						}
					}
				}
			}
			else
			{
				console_warning(
					"the animation '%s' doesn't exist in the graph '%s'",
					animation_name,
					tag_get_name(animation_graph_index));
			}
		}
	}

	return animation_started;
}

boolean unit_set_seat(
	long unit_index,
	char const *seat_label)
{
	boolean seat_set = FALSE;

	if (unit_set_or_test_seat_and_weapon_label(unit_index, seat_label, NULL, TRUE))
	{
		seat_set = TRUE;
	}

	return seat_set;
}

boolean unit_flying_through_air(
	long unit_index)
{
	struct unit_datum *unit = unit_get(unit_index);
	boolean flying_through_air = FALSE;

	if (unit->object.type==_object_type_biped)
	{
		flying_through_air = biped_flying_through_air(unit_index);
	}

	return flying_through_air;
}

void unit_stop_custom_animation(
	long unit_index)
{
	if (unit_index!=NONE && unit_get(unit_index)->unit.animation.state==_unit_state_user_animation)
	{
		unit_animation_set_state(unit_index, _unit_state_idle);
		network_coop_note_unit_animation(unit_index, NONE, NONE, FALSE);
	}

	return;
}

/* port: a unit that feigned death gets back up (a Flood combat form). The
host's does when its timer runs out (unit_update); a client's copy when the
host's word on it says it is alive again (network_actors.c). */
void unit_port_resurrect(
	long unit_index)
{
	struct unit_datum *unit = unit_get(unit_index);
	short new_state = TEST_FLAG(unit->unit.animation.flags, _unit_animation_fallen_on_front_bit) ?
		_unit_state_resurrect_front : _unit_state_resurrect_back;

	SET_FLAG(unit->object.damage_flags, _object_dead_bit, FALSE);
	unit_set_actively_controlled(unit_index, TRUE);
	unit_set_or_test_seat_and_weapon_label(unit_index, base_seat_label_get(_unit_animation_state_suspicious), NULL,
		TRUE);
	unit_animation_set_state(unit_index, new_state);
	SET_FLAG(unit->unit.animation.flags, _unit_animation_ignore_translation_bit, FALSE);
	if (unit->object.type == _object_type_biped)
		biped_stop_limp_body_physics(unit_index);
	unit_scream(unit_index, _unit_scream_resurrection);
}

/* how far into its own flinch or death animation a client's unit still
switches to the host's pick */
#define DAMAGE_ANIMATION_SWITCH_TICKS 10

/* port: whether the unit plays a flinch or death animation of the type
(the host's pick, port/linux/game/network_objects.c): it is switched to the
host's if its own has only just begun. FALSE if it plays none. */
boolean unit_port_correct_damage_animation(
	long unit_index,
	short type,
	short animation_index)
{
	struct unit_datum *unit = unit_get(unit_index);
	long animation_graph_index = unit_definition_get(unit->definition_index)->object.animation_graph.index;

	if (type == _unit_damage_animation_soft_ping)
	{
		/* (an overlay, beside the unit's animation) */
		if (unit->unit.animation.soft_ping_animation.index == NONE)
			return FALSE;
		if (unit->unit.animation.soft_ping_animation.frame_index < DAMAGE_ANIMATION_SWITCH_TICKS)
			unit->unit.animation.soft_ping_animation.index = animation_index;
		return TRUE;
	}
	if (type == _unit_damage_animation_hard_ping ?
		unit->unit.animation.state != _unit_state_hard_ping :
		unit->unit.animation.state != _unit_state_dying && unit->unit.animation.state != _unit_state_dying_airborne)
	{
		return FALSE;
	}
	if (unit->object.animation.state.index != animation_index &&
		unit->object.animation.state.frame_index < DAMAGE_ANIMATION_SWITCH_TICKS)
	{
		unit_set_animation(unit_index, animation_graph_index, animation_index);
		object_compute_node_matrices_recursive(unit_index);
	}
	return TRUE;
}

/* port: a co-op client plays the custom animation the host's unit started
(network_coop.c): exactly that animation, not another random permutation */
void unit_port_play_user_animation(
	long unit_index,
	long animation_graph_index,
	short animation_index,
	boolean interpolate,
	short frame_index)
{
	struct unit_datum *unit = unit_get(unit_index);
	struct animation_graph *animation_graph = animation_graph_definition_get(animation_graph_index);
	struct animation *animation;

	if (animation_index < 0 || animation_index >= animation_graph->animations.count)
		return;
	animation = TAG_BLOCK_GET_ELEMENT(&animation_graph->animations, animation_index, struct animation);
	if (animation->type != _animation_base)
		return;
	if (interpolate)
		object_start_interpolation(unit_index, 6);
	unit->unit.animation.state = _unit_state_user_animation;
	unit_set_animation(unit_index, animation_graph_index, animation_index);
	SET_FLAG(unit->unit.animation.flags, _unit_animation_postpone_weapon_ik_until_interpolation_ends_bit, TRUE);
	if (frame_index > 0 && frame_index < animation->frame_count)
		unit->object.animation.state.frame_index = frame_index;
	object_compute_node_matrices_recursive(unit_index);
}

boolean unit_melee_attack_begin(
	long unit_index,
	boolean continuous,
	real_vector2d const *alignment_vector)
{
	struct unit_datum *unit = unit_get(unit_index);
	struct unit_definition *unit_definition =
		unit_definition_get(unit->definition_index);
	char previous_animation_state = unit->unit.animation.state;
	boolean result = FALSE;

	switch (previous_animation_state)
	{
	case _unit_state_hard_ping:
	case _unit_state_dying_airborne:
	case _unit_state_dying:
	case _unit_state_entering_seat:
	case _unit_state_exiting_seat:
	case _unit_state_ai_impulse:
	case _unit_state_melee_attack:
	case _unit_state_melee_airborne:
	case _unit_state_melee_continuous:
	case _unit_state_throw_grenade:
	case _unit_state_resurrect_front:
	case _unit_state_resurrect_back:
	case _unit_state_leap_start:
	case _unit_state_leap_melee:
		break;

	default:
		{
			boolean limping = FALSE;
			short new_animation_state;

			if (unit->object.type==_object_type_biped)
			{
				struct biped_datum *biped = (struct biped_datum *)unit;
				limping = TEST_FLAG(
					biped->biped.flags,
					_biped_airborne_bit);
			}

			if (continuous)
			{
				new_animation_state = _unit_state_melee_continuous;
			}
			else if (previous_animation_state==_unit_state_leap_airborne)
			{
				new_animation_state = _unit_state_leap_melee;
			}
			else
			{
				new_animation_state = limping ?
					_unit_state_melee_airborne :
					_unit_state_melee_attack;
			}

			if (unit_animation_set_state(unit_index, new_animation_state) || continuous)
			{
				if (TEST_FLAG(
					unit_definition->unit.flags,
					_unit_melee_attack_is_fatal_bit))
				{
					unit->unit.animation.state = _unit_state_dying;
				}

				if (alignment_vector)
				{
					unit_align_facing(unit_index, alignment_vector);
				}

				if (continuous)
				{
					unit->unit.melee_attack_state = 4;
					unit->unit.melee_continuous_damage_effect_timer = 0;
				}
				else
				{
					unit->unit.melee_attack_state = 1;
					/* port: the AI's swing, played on the clients too */
					if (unit->unit.player_index == NONE)
						network_actors_note_melee(unit_index, alignment_vector);
				}
				result = TRUE;
			}
		}
		break;
	}

	return result;
}

boolean unit_leap_begin(
	long unit_index,
	real_vector2d const *alignment_vector)
{
	struct unit_datum *unit = unit_get(unit_index);
	boolean result = FALSE;

	switch (unit->unit.animation.state)
	{
	case _unit_state_hard_ping:
	case _unit_state_dying_airborne:
	case _unit_state_dying:
	case _unit_state_entering_seat:
	case _unit_state_exiting_seat:
	case _unit_state_ai_impulse:
	case _unit_state_melee_attack:
	case _unit_state_melee_airborne:
	case _unit_state_melee_continuous:
	case _unit_state_throw_grenade:
	case _unit_state_resurrect_front:
	case _unit_state_resurrect_back:
	case _unit_state_leap_start:
	case _unit_state_leap_melee:
		break;

	default:
		{
			boolean biped_limping = FALSE;

			if (unit->object.type==_object_type_biped)
			{
				biped_limping = TEST_FLAG(
					((struct biped_datum *)unit)->biped.flags,
					_biped_airborne_bit);
			}

			if (!biped_limping &&
				unit_animation_set_state(unit_index, _unit_state_leap_start))
			{
				if (alignment_vector)
				{
					unit_align_facing(unit_index, alignment_vector);
				}
				/* port: the AI's leap, played on the clients too */
				if (unit->unit.player_index == NONE)
					network_actors_note_leap(unit_index, alignment_vector);

				result = TRUE;
			}
		}
		break;
	}

	return result;
}

boolean unit_throw_grenade_begin(
	long unit_index,
	real_vector2d const *alignment_vector)
{
	struct unit_datum *unit = unit_get(unit_index);
	struct unit_definition *unit_definition =
		unit_definition_get(unit->definition_index);
	long weapon_index = unit_get_current_weapon_index(unit_index);
	boolean result = FALSE;

	if (unit_get_grenade_count(
		unit_index,
		unit_get_current_grenade_type(unit_index)) > 0)
	{
		switch (unit->unit.animation.state)
		{
		case _unit_state_hard_ping:
		case _unit_state_dying_airborne:
		case _unit_state_dying:
		case _unit_state_entering_seat:
		case _unit_state_exiting_seat:
		case _unit_state_ai_impulse:
		case _unit_state_melee_attack:
		case _unit_state_melee_airborne:
		case _unit_state_melee_continuous:
		case _unit_state_throw_grenade:
		case _unit_state_resurrect_front:
		case _unit_state_resurrect_back:
		case _unit_state_leap_start:
		case _unit_state_leap_melee:
			break;

		default:
			/* port: and with no weapon (a loadout of none), as melee
			(bipeds.c), which weapon_prevents_grenade_throwing(NONE) prevents;
			not from a vehicle's seat, which holds no weapon either */
			if ((weapon_index == NONE && unit->unit.parent_seat_index == NONE) ||
				(weapon_index != NONE && !weapon_prevents_grenade_throwing(weapon_index)))
			{
				struct animation_graph *animation_graph;
				struct animation *animation;
				struct game_globals_grenade *grenade;

				if (weapon_index != NONE)
				{
					weapon_stop_reload(weapon_index);
				}

				biped_stop_melee_attack(unit_index);
				unit->unit.animation.action = 0;
				unit->unit.animation.action_animation.index = NONE;

				if (unit_animation_set_state(
					unit_index,
					_unit_state_throw_grenade))
				{
					unit->unit.grenade_throw_state =
						_unit_grenade_throw_wind_up;
					unit->unit.grenade_throw_ticks = 0;

					animation_graph = animation_graph_definition_get(
						unit_definition->object.animation_graph.index);
					animation = TAG_BLOCK_GET_ELEMENT(
						&animation_graph->animations,
						unit->object.animation.state.index,
						struct animation);
					unit->unit.grenade_throw_full_power_ticks =
						animation->private_key_frame_index -
						unit->object.animation.state.frame_index + 1;

					if (alignment_vector)
					{
						unit_align_facing(unit_index, alignment_vector);
					}
					else
					{
						real_vector2d forward;

						forward.i = unit->unit.aiming_vector.i;
						forward.j = unit->unit.aiming_vector.j;
						if (normalize2d(&forward) > 0.f)
						{
							unit_align_facing(unit_index, &forward);
						}
					}

					first_person_weapon_message_from_unit(unit_index, 0x11);
					player_control_unzoom(unit_index);

					grenade = TAG_BLOCK_GET_ELEMENT(
						&scenario_get_game_globals()->grenades,
						unit->unit.current_grenade_index,
						struct game_globals_grenade);
					if (grenade->throwing_effect.index != NONE)
					{
						effect_new_from_object(
							grenade->throwing_effect.index,
							unit_index,
							unit_index,
							NONE,
							0.f,
							0.f,
							NULL,
							NULL);
					}

					result = TRUE;
				}
			}
			break;
		}
	}

	return result;
}

void scripting_magic_melee_attack(
	void)
{
	unit_melee_attack_begin(player_get(0)->unit_index, FALSE, 0);

	return;
}

void unit_impact_melee_damage(
	long unit_index,
	long target_object_index,
	short node_index,
	short region_index,
	short material_index,
	real_point3d const *position,
	real_vector3d const *object_normal,
	struct location const *location)
{
	struct unit_datum *unit = unit_get(unit_index);
	struct unit_definition *unit_definition =
		unit_definition_get(unit->definition_index);
	struct object_datum *target_object = object_get(target_object_index);

	if (TEST_FLAG(
		unit_definition->unit.flags,
		_unit_impact_melee_die_on_shield_bit) &&
		target_object->object.type==_object_type_biped &&
		target_object->object.shield_vitality>0.f)
	{
		struct unit_definition *target_unit_definition =
			unit_definition_get(target_object->definition_index);

		if (TEST_FLAG(
			target_unit_definition->unit.flags,
			_unit_shields_fry_infection_forms_bit))
		{
			unit_cause_melee_damage(
				unit_index,
				TRUE,
				target_object_index,
				node_index,
				region_index,
				material_index,
				object_normal);
			object_deplete_body(unit_index);
			object_delete(unit_index);

			return;
		}
	}

	if (TEST_FLAG(
		unit_definition->unit.flags,
		_unit_impact_melee_attaches_bit) &&
		TEST_FLAG(
			_object_mask_biped | _object_mask_vehicle,
			target_object->object.type) &&
		!TEST_FLAG(target_object->object.damage_flags, _object_dead_bit))
	{
		long parent_object_index = target_object->object.parent_object_index;

		while (parent_object_index!=NONE)
		{
			struct object_datum *parent_object = object_get(parent_object_index);

			if (parent_object_index==unit_index ||
				parent_object->object.type!=_object_type_vehicle)
			{
				return;
			}

			parent_object_index = parent_object->object.parent_object_index;
		}

		{
			real_vector3d left;
			real_vector3d *forward = &unit->object.forward;

			unit->object.translational_velocity = *global_zero_vector3d;
			unit->object.angular_velocity = *global_zero_vector3d;
			*forward = *object_normal;
			forward->i = -forward->i;
			forward->j = -forward->j;
			forward->k = -forward->k;

			cross_product3d(&unit->object.up, forward, &left);
			if (normalize3d(&left)==0.f)
			{
				cross_product3d(global_up3d, forward, &left);
				if (normalize3d(&left)==0.f)
				{
					left = *global_forward3d;
				}
			}

			cross_product3d(forward, &left, &unit->object.up);
			object_translate(unit_index, position, location);
			object_attach_to_node(target_object_index, unit_index, node_index);
			SET_FLAG(unit->object.flags, _object_at_rest_bit, TRUE);
			SET_FLAG(unit->unit.flags, _unit_attached_melee_attack_bit, TRUE);
			unit_melee_attack_begin(unit_index, TRUE, FALSE);
		}
	}

	return;
}

static void unit_ping_animation(
	long unit_index,
	boolean killed,
	boolean feign_death,
	boolean suppress_random_death_frame,
	boolean suppress_hard_ping,
	boolean force_hard_ping,
	real damage_direction_angle,
	short damage_part,
	real_vector2d const *alignment_vector);

enum
{
	_scenario_unit_dead_bit = 0,
};

struct scenario_unit_datum
{
	real body_vitality;
	unsigned long flags;
};
typedef char scenario_unit_datum_size_check[
	sizeof(struct scenario_unit_datum) == 0x08 ? 1 : -1];

void unit_place(
	long unit_index,
	struct scenario_unit_datum const *scenario_unit)
{
	struct unit_datum *unit = unit_get(unit_index);
	struct unit_definition *unit_definition =
		unit_definition_get(unit->definition_index);

	if (scenario_unit->body_vitality>0.f)
	{
		unit->object.body_vitality = scenario_unit->body_vitality;
	}

	if (TEST_FLAG(scenario_unit->flags, _scenario_unit_dead_bit))
	{
		unit_ping_animation(
			unit_index,
			TRUE,
			FALSE,
			FALSE,
			FALSE,
			FALSE,
			FALSE,
			NONE,
			FALSE);

		if (unit->unit.animation.state==_unit_state_dying)
		{
			unsigned long object_flags;
			unsigned long unit_flags;
			long death_frame_index;
			struct animation_graph *animation_graph;
			struct animation *animation;

			unit_delete_all_weapons(unit_index);
			csmemset(
				unit->unit.grenade_counts,
				0,
				sizeof(unit->unit.grenade_counts));

			if (unit->unit.equipment_object_index!=NONE)
			{
				object_delete(unit->unit.equipment_object_index);
				unit->unit.equipment_object_index = NONE;
			}

			animation_graph = animation_graph_definition_get(
				unit_definition->object.animation_graph.index);
			animation = TAG_BLOCK_GET_ELEMENT(
				&animation_graph->animations,
				unit->object.animation.state.index,
				struct animation);
			object_flags = unit->object.flags;
			death_frame_index = MAX(0, animation->frame_count - 4);
			SET_FLAG(unit->object.damage_flags, _object_dead_bit, TRUE);
			unit_flags = unit->unit.flags | FLAG(_unit_placed_here_dead_bit);
			unit->object.animation.state.frame_index = (short)death_frame_index;
			unit->unit.flags = unit_flags;
			unit->object.flags = object_flags | FLAG(_object_cannot_be_garbage_bit);
			unit->unit.time_of_death = game_time_get();
			unit->object.body_vitality = 0.f;
			unit->object.shield_vitality = 0.f;
			object_deplete_shield(unit_index);
			object_compute_node_matrices_recursive(unit_index);
		}
	}

	return;
}

void unit_scripting_enter_vehicle(
	long unit_index,
	long vehicle_index,
	char const *seat_name)
{
	if (unit_index!=NONE && vehicle_index!=NONE && csstrlen(seat_name)>0)
	{
		struct unit_datum *unit = unit_get(unit_index);

		if (!TEST_FLAG(unit->object.damage_flags, _object_dead_bit))
		{
			if (unit->object.parent_object_index!=NONE)
			{
				if (unit->unit.parent_seat_index!=NONE)
				{
					unit_exit_seat_end(unit_index);
				}

				if (unit->object.parent_object_index!=NONE)
				{
					return;
				}
			}

			{
				struct unit_datum *vehicle = unit_get(vehicle_index);
				struct unit_definition *vehicle_definition = unit_definition_get(vehicle->definition_index);
				short seat_index;

				for (seat_index = 0; seat_index<vehicle_definition->unit.seats.count; ++seat_index)
				{
					struct unit_seat *seat = TAG_BLOCK_GET_ELEMENT(
						&vehicle_definition->unit.seats,
						seat_index,
						struct unit_seat);

					if (!_stricmp(seat_name, seat->label) &&
						!unit_seat_filled(vehicle_index, seat_index) &&
						(unit->object.type==_object_type_vehicle ||
							unit_set_or_test_seat_and_weapon_label(unit_index, seat->label, NULL, FALSE)))
					{
						unit_enter_seat(unit_index, vehicle_index, seat_index);
						break;
					}
				}
			}

			/* port: co-op players the scripts can't name board with player0 */
			if (unit->object.parent_object_index == vehicle_index)
				coop_scripts_board_followers(unit_index, vehicle_index, seat_name);
		}
	}

	return;
}

boolean unit_try_and_exit_seat(
	long unit_index)
{
	struct unit_datum *unit = unit_get(unit_index);
	boolean result = FALSE;

	if (unit->object.parent_object_index!=NONE &&
		unit->unit.parent_seat_index!=NONE)
	{
		if (unit->object.type==_object_type_vehicle)
		{
			unit_exit_seat_end(unit_index);
		}
		else if (!unit_animation_busy(&unit->unit.animation))
		{
			struct unit_definition *unit_definition = unit_definition_get(unit->definition_index);
			struct animation_graph *animation_graph = animation_graph_definition_get(unit_definition->object.animation_graph.index);
			struct animation_graph_unit_seat *unit_seat = TAG_BLOCK_GET_ELEMENT(
				&animation_graph->unit_seats,
				unit->unit.animation.seat_index,
				struct animation_graph_unit_seat);

			if (unit_seat->animations.count>8)
			{
				short animation_index = animation_graph_animation_index_get(&unit_seat->animations)[8].animation_index;

				if (animation_index!=NONE)
				{
					struct unit_datum *parent_unit = unit_get(unit->object.parent_object_index);

					if (parent_unit->unit.driver_object_index==unit_index)
					{
						unit_open(unit->object.parent_object_index);
					}

					animation_index = animation_choose_random_permutation(
						unit_definition->object.animation_graph.index,
						animation_index);

					unit_set_animation(
						unit_index,
						unit_definition->object.animation_graph.index,
						animation_index);
					object_set_visibility(unit_index, TRUE);
					unit->unit.animation.state = _unit_state_exiting_seat;
					ai_handle_exit_vehicle(
						unit_index,
						unit->object.parent_object_index);
					result = TRUE;
				}
			}
		}
	}

	return result;
}

short vehicle_scripting_unload(
	long vehicle_index,
	char const *seat_name)
{
	long unloaded_count = 0;

	if (vehicle_index!=NONE)
	{
		struct unit_datum *vehicle = unit_get(vehicle_index);
		struct unit_definition *vehicle_definition = unit_definition_get(vehicle->definition_index);
		boolean unload_all = !seat_name || csstrlen(seat_name)==0;
		struct object_iterator iterator;
		struct unit_datum *unit;

		object_iterator_new(&iterator, _object_mask_unit, 0);

		while ((unit = object_iterator_next(&iterator))!=NULL)
		{
			if (unit->object.parent_object_index==vehicle_index)
			{
				struct unit_seat *seat = TAG_BLOCK_GET_ELEMENT(
					&vehicle_definition->unit.seats,
					unit->unit.parent_seat_index,
					struct unit_seat);
				char lower_seat_name[256];

				csstrcpy(lower_seat_name, seat->label);
				strlwr(lower_seat_name);

				if (unload_all || strstr(lower_seat_name, seat_name))
				{
					if (unit_try_and_exit_seat(iterator.index))
					{
						++unloaded_count;
					}
				}
			}
		}
	}

	return (short)unloaded_count;
}

void unit_scripting_exit_vehicle(
	long unit_index)
{
	if (unit_index!=NONE)
	{
		struct unit_datum *unit = unit_get(unit_index);
		long vehicle_index = unit->object.parent_object_index;

		if (unit->object.parent_object_index!=NONE && unit->unit.parent_seat_index!=NONE)
		{
			unit_try_and_exit_seat(unit_index);
			/* port: co-op players the scripts can't name get out with player0 */
			coop_scripts_exit_followers(unit_index, vehicle_index);
		}
	}

	return;
}

void unit_adjust_projectile_ray(
	long unit_index,
	real_point3d *origin,
	real_vector3d *direction,
	real *velocity,
	boolean adjust_origin,
	boolean use_aiming_vector)
{
	struct unit_datum *unit;
	real projection;
	real_vector3d relative;

	unit = unit_get(unit_index);
	if (use_aiming_vector)
	{
		*direction = unit->unit.aiming_vector;
	}

	if (adjust_origin)
	{
		real_point3d camera_position;

		unit_get_camera_position(unit_index, &camera_position);
		vector_from_points3d(&camera_position, origin, &relative);
		projection =
			((relative.i*direction->i + relative.k*direction->k) +
			 relative.j*direction->j);
		point_from_line3d(&camera_position, direction, projection, origin);
	}

	{
		real_vector3d object_velocity;

		object_get_velocities(unit_index, &object_velocity, NULL);
		*velocity = dot_product3d(direction, &object_velocity);
	}

	return;
}

void unit_render_debug(
	long unit_index)
{
	struct unit_datum *unit;
	struct unit_definition *unit_definition;
	real_point3d camera_position;
	real_point3d origin;

	unit = unit_get(unit_index);
	unit_definition = unit_definition_get(unit->definition_index);

	if (debug_objects_unit_vectors)
	{
		unit_get_camera_position(unit_index, &camera_position);
		object_get_origin(unit_index, &origin);
		origin.z += 0.1f;

		render_debug_vector(
			TRUE,
			&camera_position,
			&unit->unit.aiming_vector,
			1.f,
			global_real_argb_white);
		render_debug_vector(
			TRUE,
			&camera_position,
			&unit->unit.desired_aiming_vector,
			0.5f,
			global_real_argb_red);
		render_debug_vector(
			TRUE,
			&origin,
			&unit->object.forward,
			1.f,
			global_real_argb_white);
		render_debug_vector(
			TRUE,
			&origin,
			&unit->unit.desired_facing_vector,
			0.5f,
			global_real_argb_red);
	}

	if (debug_objects_unit_seats)
	{
		long player_unit_index =
			player_get(local_player_get_player_index(render.local_player_index))->unit_index;

		if (player_unit_index!=NONE)
		{
			short seat_index;

			for (seat_index = 0;
				seat_index<unit_definition->unit.seats.count;
				seat_index++)
			{
				real_point3d seat_position;

				if (unit_get_seat_entrance_point(
					player_unit_index,
					unit_index,
					seat_index,
					&camera_position,
					&origin,
					&seat_position))
				{
					render_debug_point(
						TRUE,
						&camera_position,
						0.25f,
						global_real_argb_red);
					render_debug_point(
						TRUE,
						&origin,
						0.25f,
						global_real_argb_blue);
					render_debug_point(
						TRUE,
						&seat_position,
						0.25f,
						global_real_argb_yellow);
				}
			}
		}
	}

	if (debug_objects_unit_mouth_apeture)
	{
		struct object_marker marker;
		real mouth_aperture;

		/* port: the Xbox's bug fixed: it ignored the marker count and read
		the marker of a unit with no head marker uninitialised */
		if (object_get_marker_by_name(unit_index, "head", &marker, 1) > 0)
		{
			mouth_aperture = unit->unit.mouth_aperture;
			origin = marker.matrix.position;
			render_debug_string_at_point(
				FALSE,
				&origin,
				csprintf(temporary, "%.2f", mouth_aperture),
				global_real_argb_orange);
		}
	}

	return;
}

void unit_abort_animation(
	long unit_index)
{
	unit_animation_set_state(unit_index, _unit_state_idle);

	return;
}

short unit_find_nearby_seat(
	long unit_index,
	long target_unit_index,
	short *parent_seat_index)
{
	enum
	{
		_unit_seat_requires_driver_bit = 9,
	};
	struct unit_datum *unit = unit_get(unit_index);
	struct unit_datum *target_unit = unit_get(target_unit_index);
	struct unit_definition *target_unit_definition =
		unit_definition_get(target_unit->definition_index);
	short best_state = _unit_nearby_seat_none;
	short best_seat_index = NONE;
	boolean best_seat_is_driver;

	if (!TEST_FLAG(target_unit->object.damage_flags, _object_dead_bit) &&
		!TEST_FLAG(target_unit->unit.flags, _unit_not_enterable_by_player_bit))
	{
		short seat_index;
		real best_distance = REAL_MAX;
		best_seat_is_driver = FALSE;

		for (seat_index = 0;
			seat_index < target_unit_definition->unit.seats.count;
			seat_index++)
		{
			struct unit_seat *seat = TAG_BLOCK_GET_ELEMENT(
				&target_unit_definition->unit.seats,
				seat_index,
				struct unit_seat);
			real_point3d unit_entrance_position;
			real_point3d seat_position;

			if (unit_get_seat_entrance_point(
				unit_index,
				target_unit_index,
				seat_index,
				&unit_entrance_position,
				&seat_position,
				NULL))
			{
				real entrance_distance = distance3d(
					&unit_entrance_position,
					&unit->object.bounding_sphere_center);
				real seat_distance = distance3d(
					&seat_position,
					&unit->object.bounding_sphere_center);
				real distance = MIN(entrance_distance, seat_distance);

				if (distance < 1.f &&
					(!TEST_FLAG(seat->flags, _unit_seat_requires_driver_bit) ||
						target_unit->unit.driver_object_index != NONE) &&
					seat->label[0] != 0 &&
					unit_set_or_test_seat_and_weapon_label(
						unit_index,
						seat->label,
						NULL,
						FALSE))
				{
					long occupant_unit_index = NONE;
					short seat_state = _unit_nearby_seat_none;

					if (unit_can_enter_seat(
						unit_index,
						target_unit_index,
						seat_index,
						&occupant_unit_index))
					{
						seat_state = _unit_nearby_seat_available;
					}
					else if (occupant_unit_index != NONE)
					{
						struct unit_datum *occupant = unit_get(occupant_unit_index);

						if (occupant->unit.actor_index != NONE &&
							ai_try_vehicle_eviction(
								occupant->unit.actor_index,
								unit_index,
								FALSE))
						{
							seat_state = _unit_nearby_seat_occupied;
						}
					}

					if (seat_state != _unit_nearby_seat_none)
					{
						boolean seat_is_driver = TEST_FLAG(
							seat->flags,
							_unit_seat_driver_bit);
						real distance_scale = 1.f;

						if (best_seat_is_driver && !seat_is_driver)
							distance_scale = 1.5f;

						if (best_seat_index == NONE ||
							seat_state > best_state ||
							distance * distance_scale < best_distance)
						{
							best_state = seat_state;
							best_seat_index = seat_index;
							best_seat_is_driver = seat_is_driver;
							best_distance = distance;
						}
					}
				}
			}
		}
	}

	match_assert(
		"c:\\halo\\SOURCE\\units\\units.c",
		4088,
		parent_seat_index != NULL);
	*parent_seat_index = best_seat_index;

	return best_state;
}

short vehicle_scripting_find_available_seats(
	long vehicle_index,
	char const *seat_substring_name,
	short seat_desire_type,
	short *seat_indices,
	short maximum_seat_count)
{
	struct unit_datum *vehicle = unit_get(vehicle_index);
	struct unit_definition *vehicle_definition = unit_definition_get(vehicle->definition_index);
	boolean match_all_seats;
	short available_seat_count;
	short seat_index;

	match_assert("c:\\halo\\SOURCE\\units\\units.c", 6031, seat_substring_name);
	match_assert(
		"c:\\halo\\SOURCE\\units\\units.c",
		6032,
		(seat_desire_type == NONE) || ((seat_desire_type >= 0) && (seat_desire_type < NUMBER_OF_VEHICLE_SEAT_DESIRE_TYPES)));

	match_all_seats = !seat_substring_name || csstrlen(seat_substring_name)==0;
	available_seat_count = 0;

	for (seat_index = 0; seat_index<vehicle_definition->unit.seats.count; ++seat_index)
	{
		struct unit_seat *seat = TAG_BLOCK_GET_ELEMENT(
			&vehicle_definition->unit.seats,
			seat_index,
			struct unit_seat);
		boolean seat_matches_desire = TRUE;
		char lower_seat_name[256];

		if (available_seat_count>=maximum_seat_count)
		{
			break;
		}

		csstrcpy(lower_seat_name, seat->label);
		strlwr(lower_seat_name);

		if (!match_all_seats && !strstr(lower_seat_name, seat_substring_name))
		{
			continue;
		}

		switch (seat_desire_type)
		{
		case _vehicle_seat_desire_not_driver:
			seat_matches_desire = !TEST_FLAG(seat->flags, _unit_seat_driver_bit);
			break;

		case _vehicle_seat_desire_gunner:
			seat_matches_desire = TEST_FLAG(seat->flags, _unit_seat_gunner_bit);
			break;

		case _vehicle_seat_desire_passenger:
			seat_matches_desire =
				!TEST_FLAG(seat->flags, _unit_seat_driver_bit) &&
				!TEST_FLAG(seat->flags, _unit_seat_gunner_bit);
			break;

		case _vehicle_seat_desire_driver:
			seat_matches_desire = TEST_FLAG(seat->flags, _unit_seat_driver_bit);
			break;

		default:
			break;
		}

		if (seat_matches_desire && !unit_seat_filled(vehicle_index, seat_index))
		{
			seat_indices[available_seat_count++] = seat_index;
		}
	}

	return available_seat_count;
}

short vehicle_scripting_load_magic(
	long vehicle_index,
	char const *seat_name,
	long object_list_index)
{
	long loaded_count = 0;

	if (vehicle_index!=NONE)
	{
		struct unit_datum *vehicle = unit_get(vehicle_index);
		struct unit_definition *vehicle_definition = unit_definition_get(vehicle->definition_index);
		short available_seat_indices[16];
		short available_seat_count;
		long reference_index;
		long unit_index;
		/* port: the riders left without a seat that network co-op keeps
		(coop_enemies.c), erased once the list is gone through */
		long unseated_actor_indices[64];
		short unseated_count = 0;
		short unseated_number;

		available_seat_count = vehicle_scripting_find_available_seats(
			vehicle_index,
			seat_name,
			NONE,
			available_seat_indices,
			NUMBEROF(available_seat_indices));
		unit_index = object_list_get_first(object_list_index, &reference_index);

		while (unit_index!=NONE)
		{
			struct object_datum *object = object_get(unit_index);

			if (TEST_FLAG(_object_mask_unit, object->object.type) &&
				!TEST_FLAG(vehicle->object.damage_flags, _object_dead_bit))
			{
				struct unit_datum *unit = (struct unit_datum *)object;
				short available_seat_index;
				long loaded_before = loaded_count;

				for (available_seat_index = 0;
					available_seat_index<available_seat_count;
					++available_seat_index)
				{
					short seat_index = available_seat_indices[available_seat_index];

					if (seat_index!=NONE)
					{
						struct unit_seat *seat = TAG_BLOCK_GET_ELEMENT(
							&vehicle_definition->unit.seats,
							seat_index,
							struct unit_seat);

						if ((unit->object.type==_object_type_vehicle ||
							unit_set_or_test_seat_and_weapon_label(unit_index, seat->label, NULL, FALSE)))
						{
							if (unit->object.parent_object_index!=NONE)
							{
								if (unit->unit.parent_seat_index!=NONE)
								{
									unit_exit_seat_end(unit_index);
								}
							}

							if (unit->object.parent_object_index==NONE &&
								unit_enter_seat(unit_index, vehicle_index, seat_index))
							{
								available_seat_indices[available_seat_index] = NONE;
								++loaded_count;
								break;
							}
						}
					}
				}

				/* port: network co-op's extra enemies (coop_enemies.c): a
				rider seated, or one left without a seat, kept until the
				seated get out */
				if (loaded_count > loaded_before)
					coop_enemies_rider_seated(vehicle_index, unit_index);
				else if (unit->object.parent_object_index == NONE && unit->unit.actor_index != NONE &&
					unseated_count < (short)NUMBEROF(unseated_actor_indices) &&
					coop_enemies_rider_unseated(vehicle_index, unit_index))
				{
					unseated_actor_indices[unseated_count++] = unit->unit.actor_index;
				}
			}

			unit_index = object_list_get_next(object_list_index, &reference_index);
		}
		for (unseated_number = 0; unseated_number < unseated_count; unseated_number++)
			actor_erase(unseated_actor_indices[unseated_number], FALSE);

		/* port: co-op players the scripts can't name board with player0,
		once this load's seats are taken */
		for (unit_index = object_list_get_first(object_list_index, &reference_index);
			unit_index != NONE;
			unit_index = object_list_get_next(object_list_index, &reference_index))
		{
			if (object_get(unit_index)->object.parent_object_index == vehicle_index)
				coop_scripts_board_followers(unit_index, vehicle_index, seat_name);
		}
	}

	return (short)loaded_count;
}

void unit_open(
	long unit_index)
{
	if (unit_index!=NONE)
	{
		network_coop_note_unit_open(unit_index, TRUE);
		unit_animation_set_state(unit_index, _unit_state_opening);
	}

	return;
}

void unit_close(
	long unit_index)
{
	if (unit_index!=NONE)
	{
		network_coop_note_unit_open(unit_index, FALSE);
		unit_animation_set_state(unit_index, _unit_state_closing);
	}

	return;
}

void scripting_set_magic_base_seat(
	char const *seat_name)
{
	magic_base_animation_seat_index = seat_label_to_base_seat_index(seat_name);

	return;
}

boolean unit_update(
	long unit_index)
{
	struct unit_datum *unit = unit_get(unit_index);
	struct unit_definition *unit_definition = unit_definition_get(unit->definition_index);
	boolean used_time = FALSE;
	boolean jetpack_cheat_active = FALSE;
	boolean light_change_state = FALSE;
	boolean allow_integrated_lights = game_engine_allow_integrated_lights(unit_index);

	profile_enter(unit_update_section);

	unit_verify_vectors(unit_index, "unit-update-begin");

	++unit->unit.timer;

	if (unit_globals->used_time!=0 || unit->unit.timer <= unit_globals->next_timer)
	{
		long timer;

		if (unit_globals->highest_timer>unit->unit.timer)
		{
			timer = unit_globals->highest_timer;
		}
		else
		{
			timer = unit->unit.timer;
		}

		unit_globals->highest_timer = timer;
	}
	else
	{
		used_time = TRUE;
		unit_globals->used_time = TRUE;
		unit->unit.timer = 0;
	}

	if (TEST_FLAG(unit->unit.flags, _unit_running_blindly_bit))
	{
		unit_running_blind(unit_index, &unit->unit.desired_facing_vector);
		unit->unit.desired_aiming_vector = unit->unit.desired_facing_vector;
		unit->unit.desired_looking_vector = unit->unit.desired_facing_vector;
		unit->unit.throttle = *global_forward3d;
		unit->unit.control_flags = 0;
	}
	else if (!TEST_FLAG(unit->unit.flags, _unit_actively_controlled_bit))
	{
		unit->unit.desired_looking_vector = unit->object.forward;
		unit->unit.desired_aiming_vector = unit->object.forward;
		unit->unit.desired_facing_vector = unit->object.forward;
		unit->unit.throttle = *global_zero_vector3d;
		unit->unit.control_flags = 0;
	}

	if (!TEST_FLAG(unit_definition->unit.flags, _unit_simple_creature_bit))
	{
		boolean v4 = FALSE;

		if (unit->unit.persistent_control_timer>0)
		{
			unit->unit.control_flags = unit->unit.persistent_control_flags|unit->unit.control_flags;
			if (TEST_FLAG(unit->unit.persistent_control_flags, _unit_control_weapon_primary_trigger_bit))
			{
				SET_FLAG(unit->unit.control_flags, _unit_control_weapon_primary_trigger_bit, (unit->unit.persistent_control_timer%7)==0);
				unit->unit.primary_trigger = 1.f;
			}
			else
			{
				unit->unit.primary_trigger = 0.f;
			}

			if (--unit->unit.persistent_control_timer==0)
			{
				unit->unit.persistent_control_flags = 0;
			}
		}

		if (!TEST_FLAG(unit->unit.flags, _unit_possessed_by_recording_bit))
		{
			if (unit->unit.driver_object_index!=NONE &&
				!TEST_FLAG(unit->object.damage_flags, _object_dead_bit))
			{
				struct unit_datum *driver_unit = unit_get(unit->unit.driver_object_index);

				unit->object.owner_team_index = driver_unit->object.owner_team_index;
				v4 = TRUE;

				if (driver_unit->unit.player_index!=NONE ||
					driver_unit->unit.animation.state!=_unit_state_exiting_seat &&
					driver_unit->unit.animation.state!=_unit_state_entering_seat)
				{
					match_assert_valid_real_normal3d(
						"c:\\halo\\SOURCE\\units\\units.c",
						696,
						&driver_unit->unit.desired_facing_vector
					);
					unit->unit.control_flags |= driver_unit->unit.control_flags&UNIT_CONTROL_DRIVER_MASK;
					unit->unit.desired_facing_vector = driver_unit->unit.desired_facing_vector;
					unit->unit.throttle = driver_unit->unit.throttle;
				}
			}

			if (unit->unit.gunner_object_index!=NONE &&
				!TEST_FLAG(unit->object.damage_flags, _object_dead_bit))
			{
				struct unit_datum *gunner_unit = unit_get(unit->unit.gunner_object_index);

				if (!v4)
				{
					unit->object.owner_team_index = gunner_unit->object.owner_team_index;
				}

				if (gunner_unit->unit.player_index!=NONE ||
					gunner_unit->unit.animation.state!=_unit_state_exiting_seat &&
					gunner_unit->unit.animation.state!=_unit_state_entering_seat)
				{
					match_assert_valid_real_normal3d(
						"c:\\halo\\SOURCE\\units\\units.c",
						720,
						&gunner_unit->unit.desired_aiming_vector
					);

					unit->unit.desired_aiming_vector = gunner_unit->unit.desired_aiming_vector;
					unit->unit.desired_looking_vector = gunner_unit->unit.desired_aiming_vector;
					unit->unit.control_flags |= gunner_unit->unit.control_flags&UNIT_CONTROL_GUNNER_MASK;
					unit->unit.primary_trigger = gunner_unit->unit.primary_trigger;
				}
			}

			if ((unit->unit.control_flags&UNIT_CONTROL_GUNNER_MASK)!=0)
			{
				unit->unit.gunner_inactive_ticks = 0;
			}
			else if (unit->unit.gunner_inactive_ticks<CHAR_MAX)
			{
				++unit->unit.gunner_inactive_ticks;
			}
		}

		/*
		* Jetpack cheat combo:
		* Press the following keys while the jetpack cheat is active
		* Grenade throw button
		* Primary weapon fire button
		*/
		if (cheat.jetpack &&
			TEST_FLAG(unit->unit.control_flags, _unit_control_weapon_primary_trigger_bit) &&
			TEST_FLAG(unit->unit.control_flags, _unit_control_throw_grenade_bit))
		{
			jetpack_cheat_active = TRUE;
		}

		if (TEST_FLAG(unit->unit.flags, _unit_active_camouflaged_bit))
		{
			real active_camo_change_this_tick;

			if (game_engine_running() &&
				unit->unit.cause_for_camo_regrowth)
			{
				if (unit->unit.cause_for_camo_regrowth==cause_for_camo_regrowth_shot_fired)
				{
					boolean set = FALSE;
					long weapon_index = unit_get_current_weapon_index(unit_index);

					if (weapon_index!=NONE)
					{
						struct weapon_datum *weapon = weapon_get(weapon_index);
						struct weapon_definition *weapon_definition = weapon_definition_get(weapon->definition_index);

						if (weapon_definition->weapon.active_camo_regrowth_rate!=0.f)
						{
							active_camo_change_this_tick = weapon_definition->weapon.active_camo_regrowth_rate;
							set = TRUE;
						}
					}

					if (!set)
						active_camo_change_this_tick = 1.f/120.f;
				}
				else
					active_camo_change_this_tick = 1.f/120.f;
			}
			else
				active_camo_change_this_tick = 1.f/120.f;

			unit->unit.active_camouflage += active_camo_change_this_tick;

			if (unit->unit.active_camouflage>1.f)
			{
				unit->unit.active_camouflage = 1.f;
				unit->unit.cause_for_camo_regrowth = cause_for_camo_regrowth_default;
			}
		}
		else
		{
			unit->unit.active_camouflage -= 1.f/120.f;

			if (unit->unit.active_camouflage<0.f)
			{
				unit->unit.active_camouflage = 0.f;
			}
		}

		if (TEST_FLAG(unit->unit.flags, _unit_super_camouflaged_bit))
		{
			unit->unit.active_camouflage_super_amount += 1.f/90.f;

			if (unit->unit.active_camouflage_super_amount>1.f)
			{
				unit->unit.active_camouflage_super_amount = 1.f;
			}
		}
		else
		{
			unit->unit.active_camouflage_super_amount -= 1.f/90.f;

			if (unit->unit.active_camouflage_super_amount<0.f)
			{
				unit->unit.active_camouflage_super_amount = 0.f;
			}
		}

		// Update stun timer
		if (unit->unit.body_stun_ticks>0)
		{
			unit->unit.body_stun_ticks--;
			// Stop stunning when tick timer is done
			if (unit->unit.body_stun_ticks==0)
			{
				unit->unit.body_stun = 0.f;
			}
		}

		// Drop weapon when drop delay is done
		if (unit->unit.weapon_drop_delay_ticks>0 && --unit->unit.weapon_drop_delay_ticks==0)
		{
			unit_drop_current_weapon(unit_index, TRUE);
		}

		// Update feign death timer
		if (unit->unit.feign_death_timer>0 &&
			TEST_FLAG(unit->object.flags, _object_at_rest_bit))
		{
			unit->unit.feign_death_timer--;

			// Die when the unit has met it's end
			if (!unit->unit.feign_death_timer)
			{
				if (unit->object.body_vitality>0.f)
				{
					unit_port_resurrect(unit_index);
				}
				else
				{
					unit_died(unit_index, FALSE);
				}
			}
		}
	}

	if (!TEST_FLAG(unit_definition->unit.flags, _unit_has_no_aiming_bit))
	{
		real aim_scale;
		real aiming_velocity_limit;
		real aiming_angular_acceleration_limit;
		real_vector3d last_aiming_vector;

		if (!TEST_FLAG(unit->object.damage_flags, _object_dead_bit))
		{
			if (TEST_FLAG(unit->object.damage_flags, _object_cannot_hold_weapon_bit))
			{
				unit_drop_current_weapon(unit_index, TRUE);
			}
			else if (unit->unit.desired_weapon_index!=unit->unit.current_weapon_index &&
				!unit_animation_busy(&unit->unit.animation))
			{
				long weapon_index = unit_get_desired_weapon_index(unit_index);

				if (weapon_index!=NONE && unit_can_use_weapon(unit_index, weapon_index))
				{
					unit_ready_desired_weapon(unit_index, TRUE);
				}
			}

			if (unit->unit.desired_grenade_index!=unit->unit.current_grenade_index &&
				!unit_animation_busy(&unit->unit.animation))
			{
				short grenade_index = unit_inventory_next_grenade(unit_index, unit->unit.desired_grenade_index, 0);

				if (grenade_index!=NONE)
				{
					unit->unit.current_grenade_index = grenade_index;
				}
			}

			if (cheat.infinite_ammo && unit->unit.player_index!=NONE)
			{
				short grenade_index;

				for (grenade_index = 0; grenade_index<NUMBEROF(unit->unit.grenade_counts); ++grenade_index)
				{
					unit->unit.grenade_counts[grenade_index] = unit->unit.grenade_counts[grenade_index]>1 ? unit->unit.grenade_counts[grenade_index] : 1;
				}

				if (unit->unit.desired_grenade_index==NONE)
				{
					unit->unit.desired_grenade_index = 0;
				}
			}

			if (unit->unit.desired_zoom_level!=unit->unit.current_zoom_level)
			{
				long local_player_index;

				unit->unit.current_zoom_level = unit->unit.desired_zoom_level;

				if (unit->unit.desired_zoom_level==NONE)
				{
					unit->unit.integrated_night_vision_power = 0.f;
				}

				if (player_index_from_unit_index(unit_index)!=NONE)
				{
					local_player_index = player_get(player_index_from_unit_index(unit_index))->local_player_index;
				}
				else
				{
					local_player_index = NONE;
				}

				if (local_player_index!=NONE)
				{
					long weapon_index = unit_get_current_weapon_index(unit_index);

					if (weapon_index!=NONE)
					{
						struct weapon_datum *weapon = weapon_get(weapon_index);
						struct weapon_definition *weapon_definition = weapon_definition_get(weapon->definition_index);
						long sound_index = unit->unit.current_zoom_level==NONE ? weapon_definition->weapon.zoom_out_sound.index : weapon_definition->weapon.zoom_in_sound.index;
						real scale = 1.f;

						if (unit->unit.current_zoom_level!=NONE && weapon_definition->weapon.zoom_level_count>1)
						{
							scale = (real)unit->unit.current_zoom_level/(real)(weapon_definition->weapon.zoom_level_count-1);
						}

						if (sound_index!=NONE)
						{
							unspatialized_impulse_sound_new(sound_index, scale);
						}
					}
				}
			}
		}

		unit_verify_vectors(unit_index, "unit-update-prevectors");

		if (unit->unit.aiming_speed==_unit_aiming_speed_casual)
		{
			aim_scale = unit_definition->unit.casual_aiming_modifier;
		}
		else
		{
			aim_scale = 1.f;
		}

		aiming_velocity_limit = aim_scale*unit_definition->unit.aiming_velocity_maximum/TICKS_PER_SECOND;
		aiming_angular_acceleration_limit = aim_scale*unit_definition->unit.aiming_acceleration_maximum/(TICKS_PER_SECOND*TICKS_PER_SECOND);
		last_aiming_vector = unit->unit.aiming_vector;

		if (aiming_velocity_limit==0.f && aiming_angular_acceleration_limit==0.f)
		{
			match_assert_valid_real_normal3d("c:\\halo\\SOURCE\\units\\units.c", 993, &unit->unit.desired_aiming_vector);
			
			unit->unit.aiming_vector = unit->unit.desired_aiming_vector;
			unit_clip_to_aiming_bounds(unit_index, &unit->unit.aiming_vector, TRUE);
			unit->unit.aiming_velocity = *global_zero_vector3d;
			unit_verify_vectors(unit_index, "unit-update-aim-set");
		}
		else if (unit->unit.animation.aiming_with_euler_screen)
		{
			real_matrix4x3 object_matrix;

			object_matrix.scale = 1.f;
			object_get_orientation(unit_index, &object_matrix.forward, &object_matrix.up);
			cross_product3d(&object_matrix.up, &object_matrix.forward, &object_matrix.left);
			object_matrix.position = *global_origin3d;

			unit_euler_aiming_update(
				&object_matrix,
				&unit->unit.aiming_vector,
				&unit->unit.desired_aiming_vector,
				&unit->unit.aiming_velocity,
				&unit->unit.animation.aiming_screen_bounds,
				aiming_velocity_limit,
				aiming_angular_acceleration_limit);

			unit_verify_vectors(unit_index, "unit-update-aim-euler");
		}
		else
		{
			angular_accelerate_to_position(
				&unit->unit.aiming_vector,
				&unit->unit.desired_aiming_vector,
				&unit->unit.aiming_velocity,
				aiming_velocity_limit,
				aiming_angular_acceleration_limit);
			unit_verify_vectors(unit_index, "unit-update-aim-3d");
		}

		{
			real angle = angle_between_vectors3d(&unit->unit.aiming_vector, &last_aiming_vector);
			real change = angle / (unit_definition->unit.aiming_velocity_maximum/TICKS_PER_SECOND);
				
			unit->unit.aiming_change = (byte)(PIN(change, 0.f, 1.f) * 255.f);
		}

		match_assert_valid_real_normal3d("c:\\halo\\SOURCE\\units\\units.c", 1032, &unit->unit.aiming_vector);

		{
			real looking_velocity_limit = (aim_scale*unit_definition->unit.looking_velocity_maximum)/((real)TICKS_PER_SECOND);
			real looking_angular_acceleration_limit = (aim_scale*unit_definition->unit.looking_acceleration_maximum)/900.f;

			if (looking_velocity_limit==0.f && looking_angular_acceleration_limit==0.f)
			{
				unit->unit.looking_vector = unit->unit.desired_looking_vector;
				unit_clip_to_aiming_bounds(unit_index, &unit->unit.looking_vector, FALSE);
				unit->unit.looking_velocity = *global_zero_vector3d;

				unit_verify_vectors(unit_index, "unit-update-look-set");
			}
			else if (unit->unit.animation.looking_with_euler_screen)
			{
				real_matrix4x3 object_matrix;

				object_matrix.scale = 1.f;
				object_get_orientation(unit_index, &object_matrix.forward, &object_matrix.up);
				cross_product3d(&object_matrix.up, &object_matrix.forward, &object_matrix.left);
				object_matrix.position = *global_origin3d;

				unit_euler_aiming_update(
					&object_matrix,
					&unit->unit.looking_vector,
					&unit->unit.desired_looking_vector,
					&unit->unit.looking_velocity,
					&unit->unit.animation.looking_screen_bounds,
					looking_velocity_limit,
					looking_angular_acceleration_limit);

				unit_verify_vectors(unit_index, "unit-update-look-euler");
			}
			else
			{
				angular_accelerate_to_position(
					&unit->unit.looking_vector,
					&unit->unit.desired_looking_vector,
					&unit->unit.looking_velocity,
					looking_velocity_limit,
					looking_angular_acceleration_limit);
				unit_verify_vectors(unit_index, "unit-update-look-3d");
			}
		}


		match_assert_valid_real_normal3d("c:\\halo\\SOURCE\\units\\units.c", 1076, &unit->unit.looking_vector);

		unit_verify_vectors(unit_index, "unit-update-postvector");

		if (!jetpack_cheat_active)
		{
			boolean throw_grenade = TEST_FLAG(unit->unit.control_flags, _unit_control_throw_grenade_bit);

			switch (unit->unit.grenade_throw_state)
			{
			case _unit_grenade_throw_idle:
				if (throw_grenade)
				{
					unit_throw_grenade_begin(unit_index, NULL);
				}
				break;
			case _unit_grenade_throw_wind_up:
				if (unit->object.animation.state.frame_index>=2)
				{
					unit_throw_grenade_move_to_hand(unit_index);
				}
				break;
			case _unit_grenade_throw_ending:
				if (unit->unit.animation.state!=_unit_state_throw_grenade && !throw_grenade)
				{
					unit->unit.grenade_throw_state = _unit_grenade_throw_idle;
				}
				break;
			case _unit_grenade_throw_in_hand:
				++unit->unit.grenade_throw_ticks;
				if (unit->unit.animation.state!=_unit_state_throw_grenade)
				{
					unit_throw_grenade_release(unit_index, TRUE);
				}
				break;
			default:
				break;
			}
		}

		if (unit->unit.current_weapon_index!=NONE)
		{
			long flags = 0;
			real primary_trigger = unit->unit.primary_trigger;

			if (unit->unit.current_weapon_index==unit->unit.desired_weapon_index)
			{
				boolean const time_remaining = unit->unit.persistent_control_timer>0 && TEST_FLAG(unit->unit.persistent_control_flags, _unit_control_weapon_primary_trigger_bit);

				if (!jetpack_cheat_active)
				{
					if (allow_integrated_lights && TEST_FLAG(unit->unit.control_flags, _unit_control_integrated_light_bit))
					{
						SET_FLAG(flags, _weapon_control_integrated_light_bit, TRUE);
					}
					if (TEST_FLAG(unit->unit.control_flags, _unit_control_weapon_primary_trigger_bit))
					{
						SET_FLAG(flags, _weapon_control_primary_trigger_bit, TRUE);
					}
					if (TEST_FLAG(unit->unit.control_flags, _unit_control_weapon_secondary_trigger_bit))
					{
						SET_FLAG(flags, _weapon_control_secondary_trigger_bit, TRUE);
					}
				}

				if (TEST_FLAG(unit_definition_get(unit->definition_index)->unit.flags, _unit_integrated_light_controls_weapon_directly_bit))
				{
					weapon_set_integrated_light_power(unit_get_current_weapon_index(unit_index), unit->unit.integrated_light_power);
				}

				if (TEST_FLAG(unit->unit.control_flags, _unit_control_weapon_reload_bit))
				{
					SET_FLAG(flags, _weapon_control_reload_bit, TRUE);
				}
				if (unit_animation_busy(&unit->unit.animation) && !time_remaining)
				{
					SET_FLAG(flags, _weapon_control_user_busy_bit, TRUE);
				}
				if (unit->object.type==_object_type_biped && ((struct biped_datum *)unit)->biped.player_melee_ticks > 0)
				{
					SET_FLAG(flags, _weapon_control_user_busy_bit, TRUE);
				}
				if (unit->unit.current_zoom_level!=NONE)
				{
					SET_FLAG(flags, _weapon_control_zoomed_bit, TRUE);
				}
			}
			else
			{
				SET_FLAG(flags, _weapon_control_user_switching_weapons_bit, TRUE);
			}

			weapon_owner_update(unit_get_current_weapon_index(unit_index), flags, primary_trigger);
		}
	}


	match_assert_valid_real_vector3d_axes2("c:\\halo\\SOURCE\\units\\units.c", 1155, &unit->object.forward, &unit->object.up);
	match_assert_valid_real_normal3d("c:\\halo\\SOURCE\\units\\units.c", 1156, &unit->unit.aiming_vector);
	match_assert_valid_real_normal3d("c:\\halo\\SOURCE\\units\\units.c", 1157, &unit->unit.looking_vector);

	if (!TEST_FLAG(unit_definition->unit.flags, _unit_simple_creature_bit))
	{
		static boolean powered_seats_reported = FALSE;
		short seat_index;

		if (unit_definition->unit.powered_seats.count>(long)NUMBEROF(unit->unit.seat_power) &&
			!powered_seats_reported)
		{
			powered_seats_reported = TRUE;
			error(_error_silent, "### ERROR %s has %ld powered seats; only the first %d get power",
				tag_get_name(unit->definition_index), unit_definition->unit.powered_seats.count,
				(int)NUMBEROF(unit->unit.seat_power));
		}

		if (TEST_FLAG(unit->unit.animation.flags, _unit_animation_showing_acceleration_bit))
		{
			unit_seat_update(unit_index);
			unit->unit.seat_acceleration.i = unit->unit.seat_desired_acceleration.i*0.3f + unit->unit.seat_acceleration.i*0.7f;
			unit->unit.seat_acceleration.j = unit->unit.seat_desired_acceleration.j*0.3f + unit->unit.seat_acceleration.j*0.7f;
			unit->unit.seat_acceleration.k = unit->unit.seat_desired_acceleration.k*0.3f + unit->unit.seat_acceleration.k*0.7f;
		}

		/* port: no more powered seats than the unit keeps power for (a map's
		count; the rest wrote past the unit) */
		for (seat_index = 0; seat_index<MIN(unit_definition->unit.powered_seats.count, (long)NUMBEROF(unit->unit.seat_power)); ++seat_index)
		{
			boolean v96 = FALSE;
			struct powered_seat_definition *powered_seat = TAG_BLOCK_GET_ELEMENT(&unit_definition->unit.powered_seats, seat_index, struct powered_seat_definition);

			if (seat_index==0)
			{
				if (unit->unit.driver_object_index!=NONE || TEST_FLAG(unit->unit.flags, _unit_actively_controlled_bit))
				{
					v96 = TRUE;
				}
				else
				{
					v96 = FALSE;
				}
			}
			else
			{
				v96 = unit->unit.gunner_object_index!=NONE && unit->unit.gunner_object_index!=unit->unit.driver_object_index;
			}

			if (TEST_FLAG(unit->object.damage_flags, _object_dead_bit) || !v96)
			{
				if (unit->unit.seat_power[seat_index] != 0.f)
				{
					unit->unit.seat_power[seat_index] -= 1.f / (powered_seat->powerdown_time * TICKS_PER_SECOND);

					if (unit->unit.seat_power[seat_index]<0.f)
					{
						unit->unit.seat_power[seat_index] = 0.f;
					}
				}
			}
			else if (unit->unit.seat_power[seat_index] != 1.f)
			{
				unit->unit.seat_power[seat_index] += 1.f / (powered_seat->powerup_time * TICKS_PER_SECOND);

				if (unit->unit.seat_power[seat_index]>1.f)
				{
					unit->unit.seat_power[seat_index] = 1.f;
				}
			}
		}
	}

	if (unit->unit.delayed_damage_timer>0)
	{
		unit->unit.delayed_damage_timer--;

		if (unit->unit.delayed_damage_timer==0)
		{
			ai_handle_damage(
				unit_index,
				unit->unit.delayed_damage_attacker_object_index,
				unit->unit.last_damage_category,
				unit->unit.delayed_damage_peak,
				NULL,
				TRUE);

			unit->unit.last_damage_category = 0;
			unit->unit.delayed_damage_attacker_object_index = NONE;
			unit->unit.delayed_damage_peak = 0.f;
		}
	}

	unit_cause_continuous_melee_damage(unit_index);
	unit_dialogue_update(unit_index);

	if (used_time || unit->unit.player_index!=NONE)
	{
		unit_refresh_illumination(unit_index);

		if (debug_unit_illumination)
		{
			if (unit->unit.player_index !=NONE)
			{
				long current_time = game_time_get();
				static long last_time = NONE;

				if (current_time >= last_time+TICKS_PER_SECOND)
				{
					error(
						_error_silent,
						"player illumination: self %.2f ambient %.2f",
						unit->unit.self_illumination,
						unit->unit.ambient_illumination);
					last_time = current_time;
				}
			}
		}
	}

	if (unit->unit.flaming_death_delay>0)
	{
		if (--unit->unit.flaming_death_delay==0)
		{
			unit_flame_to_death(unit_index);
		}
	}

	interpolate_scalar(&unit->unit.mouth_aperture, 0.f, 0.1f);

	if (TEST_FLAG(unit->unit.flags, _unit_desired_integrated_light_on_bit))
	{
		if (!TEST_FLAG(unit->unit.flags, _unit_integrated_light_on_bit))
		{
			light_change_state = TRUE;
		}

		SET_FLAG(unit->unit.flags, _unit_desired_integrated_light_on_bit, FALSE);
	}
	

	if (TEST_FLAG(unit->unit.flags, _unit_desired_integrated_light_off_bit))
	{
		if (TEST_FLAG(unit->unit.flags, _unit_integrated_light_on_bit))
		{
			light_change_state = TRUE;
		}

		SET_FLAG(unit->unit.flags, _unit_desired_integrated_light_off_bit, FALSE);
	}

	if (allow_integrated_lights &&
		TEST_FLAG(unit->unit.control_flags, _unit_control_integrated_light_bit) ||
		unit->unit.integrated_light_battery <= 0.f ||
		light_change_state)
	{
		boolean const active = unit_integrated_night_vision_is_active(unit_index);

		if (active && TEST_FLAG(unit->unit.control_flags, _unit_control_integrated_light_bit))
		{
			long effect_index;
			struct game_globals *game_globals;
			struct game_globals_first_person_interface *game_globals_first_person_interface;

			game_globals = scenario_get_game_globals();

			match_assert("c:\\halo\\SOURCE\\units\\units.c", 1302, game_globals);
			game_globals_first_person_interface = TAG_BLOCK_GET_ELEMENT(&game_globals->first_person_interface, 0, struct game_globals_first_person_interface);
			match_assert("c:\\halo\\SOURCE\\units\\units.c", 1304, game_globals_first_person_interface);

			if (TEST_FLAG(unit->unit.flags, _unit_integrated_night_vision_on_bit))
			{
				effect_index = game_globals_first_person_interface->night_vision_on_off_effect.index;
			}
			else
			{
				effect_index = game_globals_first_person_interface->night_vision_off_on_effect.index;
			}

			if (effect_index!=NONE)
			{
				effect_new_from_object(effect_index, unit_index, unit_index, NONE, 0.f, 0.f, NULL, NULL);
			}

			unit->unit.flags ^= FLAG(_unit_integrated_night_vision_on_bit);
		}

		if (!(active && TEST_FLAG(unit->unit.control_flags, _unit_control_integrated_light_bit)) &&
			(TEST_FLAG(unit->unit.flags, _unit_integrated_light_on_bit) || unit->unit.integrated_light_battery > 0.2f) &&
			unit->object.parent_object_index==NONE)
		{
			effect_new_from_object(unit_definition->unit.integrated_light_toggle_effect.index, unit_index, unit_index, NONE, 0.f, 0.f, NULL, NULL);
			unit->unit.flags ^= FLAG(_unit_integrated_light_on_bit);
		}
	}

	if (TEST_FLAG(unit->unit.flags, _unit_integrated_light_on_bit))
	{
		if (!TEST_FLAG(unit_definition->unit.flags, _unit_integrated_light_fucking_lasts_forever_bit))
		{
			unit->unit.integrated_light_battery = unit->unit.integrated_light_battery - 1.f/(2.f*TICKS_PER_MINUTE);
		}

		if (unit->object.parent_object_index!=NONE || TEST_FLAG(unit->object.damage_flags, _object_dead_bit))
		{
			SET_FLAG(unit->unit.flags, _unit_integrated_light_on_bit, FALSE);
		}

		if (unit->unit.integrated_light_power!=1.f)
		{
			unit->unit.integrated_light_power += 1.f/6.f;

			if (unit->unit.integrated_light_power>1.f)
			{
				unit->unit.integrated_light_power = 1.f;
			}
		}
	}
	else
	{
		if (unit->unit.integrated_light_battery < 1.f)
		{
			unit->unit.integrated_light_battery += 1.f/900.f;
		}

		if (unit->unit.integrated_light_power != 0.f)
		{
			unit->unit.integrated_light_power -= 1.f/24.f;

			if (unit->unit.integrated_light_power<0.f)
			{
				unit->unit.integrated_light_power = 0.f;
			}
		}
	}
	

	
	if (unit_integrated_night_vision_is_active(unit_index))
	{
		if (TEST_FLAG(unit->unit.flags, _unit_integrated_night_vision_on_bit))
		{
			if (unit->unit.integrated_night_vision_power!=1.f)
			{
				unit->unit.integrated_night_vision_power += 1.f/12.f;

				if (unit->unit.integrated_night_vision_power>1.f)
				{
					unit->unit.integrated_night_vision_power = 1.f;
				}
			}
		}
		else if (unit->unit.integrated_night_vision_power!=0.f)
		{
			unit->unit.integrated_night_vision_power -= 1.f/24.f;

			if (unit->unit.integrated_night_vision_power<0.f)
			{
				unit->unit.integrated_night_vision_power = 0.f;
			}
		}
	}

	unit_verify_vectors(unit_index, "unit-update-end");
	
	profile_exit(unit_update_section);
	
	return TRUE;
}


void unit_unzoom(
	long unit_index)
{
	boolean play_sound;
	boolean current_weapon_zooms;
	long current_weapon_index;
	long local_player_index;

	struct unit_datum *unit = unit_get(unit_index);

	if (player_index_from_unit_index(unit_index)==NONE)
	{
		local_player_index = NONE;
	}
	else
	{
		local_player_index = player_get(player_index_from_unit_index(unit_index))->local_player_index;
	}

	current_weapon_zooms = unit->unit.current_zoom_level!=NONE;

	if (local_player_index!=NONE && current_weapon_zooms)
	{
		current_weapon_index = unit_get_current_weapon_index(unit_index);

		if (current_weapon_index!=NONE)
		{
			struct weapon_datum *weapon = weapon_get(current_weapon_index);
			struct weapon_definition *weapon_definition = weapon_definition_get(weapon->definition_index);
			long zoom_out_sound_index = weapon_definition->weapon.zoom_out_sound.index;

			play_sound = zoom_out_sound_index!=NONE;

			if (play_sound)
			{
				unspatialized_impulse_sound_new(zoom_out_sound_index, 1.f);
			}
		}
	}

	unit->unit.current_zoom_level = NONE;
	unit->unit.desired_zoom_level = NONE;
	unit->unit.integrated_night_vision_power = 0.f;
	player_control_unzoom(unit_index);

	return;
}

void unit_record_damage(
	long unit_index,
	real damage_amount,
	short damage_type,
	boolean notify_ai,
	long attacker_player_index,
	short attacker_team,
	long killing_object_index)
{
	struct unit_datum *unit;
	struct unit_datum *attacker_unit;
	long attacker_unit_index;
	long player_unit_index;
	long controlling_unit_index;
	long game_time;
	short attacker_index;
	short preserved_attacker_index;
	short best_new_attacker_index;
	boolean found_attacker;
	struct player_datum *attacker_player = attacker_player_index != NONE ?
		player_try_and_get(attacker_player_index) : NULL;

	/* port: damage can outlive its player (an older checkpoint's): a handle
	that is no player's now is neither recorded nor read */
	if (!attacker_player)
		attacker_player_index = NONE;

	unit = unit_get(unit_index);
	found_attacker = FALSE;
	game_time = game_time_get();
	for (attacker_index = 0;
		attacker_index < MAXIMUM_ATTACKERS_PER_UNIT;
		attacker_index++)
	{
		struct unit_attacker *attacker =
			&unit->unit.attackers[attacker_index];

		if ((attacker_player_index != NONE &&
			attacker->player_index == attacker_player_index) ||
			attacker->object_index == killing_object_index)
		{
			attacker->game_time_stamp = game_time;
			found_attacker = TRUE;
			attacker->damage_inflicted += damage_amount;
		}
	}

	if (!found_attacker)
	{
		best_new_attacker_index = NONE;
		for (attacker_index = 0;
			attacker_index < MAXIMUM_ATTACKERS_PER_UNIT;
			attacker_index++)
		{
			if (unit->unit.attackers[attacker_index].game_time_stamp == NONE)
			{
				best_new_attacker_index = attacker_index;
				break;
			}
		}

		if (best_new_attacker_index == NONE)
		{
			preserved_attacker_index = 0;
			for (attacker_index = 1;
				attacker_index < MAXIMUM_ATTACKERS_PER_UNIT;
				attacker_index++)
			{
				if (unit->unit.attackers[attacker_index].damage_inflicted >
					unit->unit.attackers[preserved_attacker_index].damage_inflicted)
				{
					preserved_attacker_index = attacker_index;
				}
			}

			best_new_attacker_index = NONE;
			for (attacker_index = 0;
				attacker_index < MAXIMUM_ATTACKERS_PER_UNIT;
				attacker_index++)
			{
				if (attacker_index != preserved_attacker_index &&
					(best_new_attacker_index == NONE ||
					unit->unit.attackers[attacker_index].game_time_stamp <
						unit->unit.attackers[best_new_attacker_index].game_time_stamp))
				{
					best_new_attacker_index = attacker_index;
				}
			}

			match_assert(
				"c:\\halo\\SOURCE\\units\\units.c",
				4972,
				best_new_attacker_index!=NONE);
		}

		unit->unit.attackers[best_new_attacker_index].player_index = attacker_player_index;
		unit->unit.attackers[best_new_attacker_index].object_index = killing_object_index;
		unit->unit.attackers[best_new_attacker_index].damage_inflicted = damage_amount;
		unit->unit.attackers[best_new_attacker_index].game_time_stamp = game_time;
	}

	attacker_unit = NULL;
	attacker_unit_index = killing_object_index;
	if (notify_ai && attacker_team != NONE &&
		game_team_is_enemy(unit->object.owner_team_index, attacker_team))
	{
		/* port: (the player looked up above) */
		if (attacker_player)
		{
			player_unit_index = attacker_player->unit_index;
			if (player_unit_index != NONE)
			{
				attacker_unit_index = player_unit_index;
				attacker_unit = unit_get(player_unit_index);
			}
		}

		if (attacker_unit == NULL)
		{
			attacker_unit_index = killing_object_index;
			attacker_unit = unit_try_and_get(killing_object_index);
		}

		if (attacker_unit != NULL)
		{
			if (damage_type == _unit_record_damage_driver_seat_type)
				controlling_unit_index = attacker_unit->unit.driver_object_index;
			else
				controlling_unit_index = attacker_unit->unit.gunner_object_index;

			if (controlling_unit_index != NONE)
			{
				attacker_unit_index = controlling_unit_index;
				attacker_unit = unit_get(controlling_unit_index);
			}

			if (!TEST_FLAG(attacker_unit->object.damage_flags, _object_dead_bit))
			{
				game_time = game_time_get();
				if (attacker_unit->unit.killing_spree_last_time == NONE ||
					attacker_unit->unit.killing_spree_last_time + 120 < game_time)
				{
					attacker_unit->unit.killing_spree_count = 0;
				}

				attacker_unit->unit.killing_spree_count++;
				attacker_unit->unit.killing_spree_last_time = game_time;
				if (ai_handle_killing_spree(
					attacker_unit_index,
					attacker_unit->unit.killing_spree_count))
				{
					attacker_unit->unit.killing_spree_count = 0;
				}
			}
		}
	}

	return;
}

static void unit_ping_animation(
	long unit_index,
	boolean killed,
	boolean feign_death,
	boolean suppress_random_death_frame,
	boolean suppress_hard_ping,
	boolean force_hard_ping,
	real damage_direction_angle,
	short damage_part,
	real_vector2d const *alignment_vector)
{
	struct unit_datum *unit;
	struct unit_definition *unit_definition;
	struct animation_graph *animation_graph;
	boolean ping;
	boolean hard_ping;
	short damage_direction;
	short damage_animation_index;
	short animation_index;
	short damage_animation_type;
	short animation_state;
	boolean apply_animation;
	struct biped_datum *biped;
	struct biped_definition *biped_definition;
	struct animation *animation;
	short quarter_frame_count;
	short random_frame;
	long front_animation_index;
	word animation_flags;

	unit = unit_get(unit_index);
	unit_definition = unit_definition_get(unit->definition_index);

	if (killed)
	{
		feign_death = FALSE;
		ping = TRUE;

		if (unit_definition->unit.hard_death_threshold>0.f &&
			unit->object.current_body_damage>
				unit_definition->unit.hard_death_threshold)
		{
			hard_ping = TRUE;
		}
		else
		{
			hard_ping = FALSE;
		}
	}
	else if (feign_death)
	{
		killed = TRUE;
		ping = TRUE;
		hard_ping = FALSE;
	}
	else
	{
		ping =
			unit->object.current_body_damage>
				unit_definition->unit.soft_ping_threshold ||
			unit->object.current_shield_damage>
				unit_definition->unit.soft_ping_threshold;
		hard_ping =
			unit->object.current_body_damage>
				unit_definition->unit.hard_ping_threshold;

		if (suppress_hard_ping ||
			TEST_FLAG(unit->unit.flags, _unit_ignore_hard_pings_bit))
		{
			hard_ping = FALSE;
		}
	}

	if (force_hard_ping)
	{
		hard_ping = TRUE;
		ping = TRUE;
	}

	if (damage_part==NONE)
	{
		damage_part = 0;
	}

	if (fabs(damage_direction_angle)<UNIT_DAMAGE_REAR_CONE_ANGLE)
	{
		damage_direction = _unit_damage_direction_back;
	}
	else if (fabs(damage_direction_angle)>UNIT_DAMAGE_FRONT_CONE_ANGLE)
	{
		damage_direction = _unit_damage_direction_front;
	}
	else if (damage_direction_angle>0.f)
	{
		damage_direction = _unit_damage_direction_left;
	}
	else
	{
		damage_direction = _unit_damage_direction_right;
	}

	if (game_engine_running() &&
		damage_part==_unit_damage_part_head &&
		hard_ping &&
		killed)
	{
		damage_direction = _unit_damage_direction_left;
	}

	if (!ping && !killed)
	{
		return;
	}

	{
		unit_definition = unit_definition_get(unit->definition_index);
		animation_graph = animation_graph_definition_get(
			unit_definition->object.animation_graph.index);

		if (!hard_ping && !killed)
		{
			if (unit->unit.animation.soft_ping_animation.index!=NONE &&
				unit->unit.animation.soft_ping_animation.frame_index<=
					unit_definition->unit.runtime_soft_ping_minimum_interrupt_ticks)
			{
				return;
			}

			damage_animation_index = build_damage_animation_index(
				_unit_damage_animation_soft_ping,
				damage_direction,
				damage_part);

			{
				long animation_graph_index;
				long selected_damage_animation_index;

				if (damage_animation_index>=0 &&
					damage_animation_index<
						animation_graph->unit_damage_animations.count)
				{
					selected_damage_animation_index =
						animation_graph_animation_index_get(
							&animation_graph->unit_damage_animations)
							[damage_animation_index].animation_index;
				}
				else
				{
					selected_damage_animation_index = NONE;
				}
				animation_graph_index =
					unit_definition->object.animation_graph.index;

				animation_index = animation_choose_random_permutation_internal(
					TRUE,
					animation_graph_index,
					selected_damage_animation_index);
			}
			/* port: the host's pick on every machine (port/linux/game/network_objects.c) */
			animation_index = network_objects_damage_animation(unit_index, _unit_damage_animation_soft_ping,
				animation_index);

			if (animation_index==NONE)
			{
				return;
			}

			unit->unit.animation.soft_ping_animation.index = animation_index;
			unit->unit.animation.soft_ping_animation.frame_index = 0;

			return;
		}
		else
		{
			animation_state = killed ? _unit_state_dying : _unit_state_hard_ping;

			if (killed)
			{
				damage_animation_type =
					hard_ping ?
						_unit_damage_animation_hard_kill :
						_unit_damage_animation_soft_kill;
			}
			else
			{
				damage_animation_type = _unit_damage_animation_hard_ping;
			}

			apply_animation =
				killed ||
				unit_animation_state_interruptable(
					&unit->unit.animation,
					animation_state);

			if (unit->unit.animation.state==_unit_state_hard_ping &&
				unit->object.animation.state.frame_index>
					unit_definition->unit.runtime_hard_ping_minimum_interrupt_ticks)
			{
				apply_animation = TRUE;
			}

			if (!killed)
			{
				if (TEST_FLAG(
					unit->object.damage_flags,
					_object_dead_bit))
				{
					apply_animation = FALSE;
				}

				if (unit->object.parent_object_index!=NONE)
				{
					return;
				}
			}

			if (!apply_animation)
			{
				return;
			}

			if (killed)
			{
				char const *seat_label =
					base_seat_labels[_unit_base_seat_stand];

				unit_set_or_test_seat_and_weapon_label(
					unit_index,
					seat_label,
					unit_get_current_weapon_label(unit_index),
					TRUE);
			}

			if (animation_state==_unit_state_dying &&
				unit->object.type==_object_type_biped)
			{
				biped = biped_get(unit_index);
				biped_definition = biped_definition_get(biped->definition_index);

				if (TEST_FLAG(biped->biped.flags, _biped_airborne_bit) &&
					!TEST_FLAG(
						biped_definition->biped.flags,
						_biped_has_no_dying_airborne_bit))
				{
					animation_state = _unit_state_dying_airborne;

					if (unit_animation_set_state(
						unit_index,
						_unit_state_dying_airborne))
					{
						goto apply_alignment;
					}
				}
			}

			damage_animation_index = build_damage_animation_index(
				damage_animation_type,
				damage_direction,
				damage_part);

			{
				long animation_graph_index;
				long selected_damage_animation_index;

				if (damage_animation_index>=0 &&
					damage_animation_index<
						animation_graph->unit_damage_animations.count)
				{
					selected_damage_animation_index =
						animation_graph_animation_index_get(
							&animation_graph->unit_damage_animations)
							[damage_animation_index].animation_index;
				}
				else
				{
					selected_damage_animation_index = NONE;
				}
				animation_graph_index =
					unit_definition->object.animation_graph.index;

				animation_index = animation_choose_random_permutation_internal(
					TRUE,
					animation_graph_index,
					selected_damage_animation_index);
			}
			/* port: the host's pick on every machine (port/linux/game/network_objects.c) */
			animation_index = network_objects_damage_animation(unit_index, damage_animation_type, animation_index);

			if (animation_index!=NONE)
			{
				if (unit->unit.animation.state==_unit_state_throw_grenade)
				{
					unit_throw_grenade_release(unit_index, TRUE);
				}

				object_start_interpolation(unit_index, 3);
				unit->unit.animation.state = (char)animation_state;
				unit_set_animation(
					unit_index,
					unit_definition->object.animation_graph.index,
					animation_index);
				SET_FLAG(
					unit->unit.animation.flags,
					_unit_animation_postpone_weapon_ik_until_interpolation_ends_bit,
					TRUE);

				if (killed)
				{
					if (!suppress_random_death_frame && !feign_death)
					{
						animation = TAG_BLOCK_GET_ELEMENT(
							&animation_graph->animations,
							animation_index,
							struct animation);
						quarter_frame_count = animation->frame_count>>2;
						random_frame = random_range(
							quarter_frame_count,
							(animation->frame_count>>1) + quarter_frame_count);
						unit->unit.weapon_drop_delay_ticks = (char)random_frame;
						unit->unit.weapon_drop_delay_ticks =
							MAX((char)random_frame, 1);
					}
					else
					{
						unit->unit.weapon_drop_delay_ticks = 0;
					}
				}

				if (damage_direction!=_unit_damage_direction_front)
				{
					animation = TAG_BLOCK_GET_ELEMENT(
						&animation_graph->animations,
						animation_index,
						struct animation);

					if (build_damage_animation_index(
						damage_animation_type,
						_unit_damage_direction_front,
						damage_part)>=0 &&
						build_damage_animation_index(
							damage_animation_type,
							_unit_damage_direction_front,
							damage_part)<
							animation_graph->unit_damage_animations.count)
					{
						front_animation_index =
							animation_graph_animation_index_get(
								&animation_graph->unit_damage_animations)
								[build_damage_animation_index(
									damage_animation_type,
									_unit_damage_direction_front,
									damage_part)].animation_index;
					}
					else
					{
						front_animation_index = NONE;
					}

					if (animation->runtime_parent_animation_index==
						front_animation_index)
					{
						damage_direction = _unit_damage_direction_front;
					}
				}

				if (killed)
				{
					SET_FLAG(
						unit->unit.animation.flags,
						_unit_animation_fallen_on_front_bit,
						damage_direction==_unit_damage_direction_back);
				}
			}
			else if (killed)
			{
				animation_flags = unit->unit.animation.flags;

				animation_flags &=
					~FLAG(_unit_animation_fallen_on_front_bit);
				animation_flags |=
					FLAG(_unit_animation_ignore_translation_bit);
				unit->unit.animation.flags = animation_flags;

				if (TEST_FLAG(
					unit_definition->unit.flags,
					_unit_is_destroyed_after_dying_bit))
				{
					unit_destroy(unit_index);
				}
			}
		}
	}

apply_alignment:
	if (alignment_vector!=NULL &&
		!TEST_FLAG(
			unit_definition->unit.flags,
			_unit_does_not_reorient_during_pings_bit) &&
		unit->object.type==_object_type_biped &&
		unit->object.parent_object_index==NONE &&
		(hard_ping || killed))
	{
		real_vector2d transformed_alignment;

		switch (damage_direction)
		{
		case _unit_damage_direction_back:
			transformed_alignment = *alignment_vector;
			break;

		case _unit_damage_direction_front:
			transformed_alignment.i = -alignment_vector->i;
			transformed_alignment.j = -alignment_vector->j;
			break;

		case _unit_damage_direction_left:
			transformed_alignment.i = -alignment_vector->j;
			transformed_alignment.j = alignment_vector->i;
			break;

		case _unit_damage_direction_right:
		{
			real negated_i;

			negated_i = -alignment_vector->i;
			transformed_alignment.i = alignment_vector->j;
			transformed_alignment.j = negated_i;
			break;
		}

		default:
			display_assert(
				NULL,
				"c:\\halo\\SOURCE\\units\\units.c",
				4562,
				TRUE);
			system_exit(-1);
			break;
		}

		unit_align_facing(unit_index, &transformed_alignment);
	}

	return;
}

void unit_damage_aftermath(
	long unit_index,
	struct damage_data *damage_data,
	unsigned long damage_flags,
	real shield_damage,
	real body_damage,
	real body_damage_multiplier,
	short body_part)
{
	struct unit_datum *unit;
	struct unit_definition *unit_definition;
	struct damage_definition *damage_definition;
	real total_damage;
	boolean lethal;
	boolean feigned;
	boolean instantaneous;

	unit = unit_get(unit_index);
	unit_definition = unit_definition_get(unit->definition_index);
	damage_definition = &damage_effect_definition_get(
		damage_data->definition_index)->damage;
	total_damage = shield_damage + body_damage;
	lethal = TEST_FLAG(
		damage_flags,
		_object_being_damaged_body_depleted_bit);
	feigned = FALSE;

	(void)body_damage_multiplier;

	if (debug_damage_taken && unit->unit.player_index != NONE)
	{
		char const *owner_name = "<unknown>";
		char const *damage_name = "<unknown>";

		if (damage_data->owner_object_index != NONE)
		{
			struct object_datum *owner_object =
				object_get(damage_data->owner_object_index);
			struct object_definition *owner_definition =
				object_definition_get(owner_object->definition_index);
			char const *separator = strrchr(
				owner_definition->object.model.name,
				'\\');

			if (separator)
			{
				owner_name = separator + 1;
			}
		}

		if (damage_data->definition_index != NONE)
		{
			char const *tag_name =
				tag_get_name(damage_data->definition_index);
			char const *separator = strrchr(
				tag_name,
				'\\');

			if (separator)
			{
				damage_name = separator + 1;
			}
		}

		console_printf(
			FALSE,
			"p%d: body %.2f shld %.2f from %s %s",
			DATUM_INDEX_TO_ABSOLUTE_INDEX(unit->unit.player_index),
			body_damage,
			shield_damage,
			owner_name,
			damage_name);
	}

	{
		real vitality =
			unit->object.recent_shield_damage +
			unit->object.recent_body_damage;

		if (vitality > 0.f)
		{
			unit->unit.last_damage_category = damage_definition->category;
			unit->unit.delayed_damage_timer = 45;
			if (vitality < unit->unit.delayed_damage_peak)
			{
				vitality = unit->unit.delayed_damage_peak;
			}
			unit->unit.delayed_damage_peak = vitality;

			if (damage_data->owner_object_index != NONE)
			{
				unit->unit.delayed_damage_attacker_object_index =
					damage_data->owner_object_index;
			}
		}
	}

	if (TEST_FLAG(unit->unit.flags, _unit_active_camouflaged_bit))
	{
		unit->unit.active_camouflage -=
			damage_definition->active_camouflage_damage;

		if (unit->unit.active_camouflage < 0.f)
		{
			unit->unit.active_camouflage = 0.f;
		}
	}

	instantaneous =
		lethal &&
		damage_definition->instantaneous_acceleration >= 2.f;

	if (!lethal &&
		TEST_FLAG(unit->unit.flags, _unit_feign_death_allowed_bit) &&
		unit_definition->unit.feign_death_threshold > 0.f &&
		unit_definition->unit.feign_death_time > 0.f &&
		unit->object.body_vitality > 0.f &&
		unit->object.recent_body_damage >
			unit_definition->unit.feign_death_threshold)
	{
		real feign_death_ticks =
			(real_random_range(0.f, 1.f) +
				unit_definition->unit.feign_death_time) * TICKS_PER_SECOND;

		SET_FLAG(unit->object.damage_flags, _object_dead_bit, TRUE);
		feigned = TRUE;
		unit->unit.feign_death_timer = (short)MAX(
			1.f,
			feign_death_ticks);
		match_assert(
			"c:\\halo\\SOURCE\\units\\units.c",
			0x1284,
			unit->unit.feign_death_timer > 0);
	}

	if (!TEST_FLAG(
			damage_data->flags,
			_damage_silent_bit) &&
		(lethal ||
			feigned ||
			!TEST_FLAG(unit->object.damage_flags, _object_dead_bit)) &&
		!TEST_FLAG(unit->unit.flags, _unit_impervious_bit) &&
		!TEST_FLAG(
			damage_definition->flags,
			_damage_definition_does_not_ping_units_bit))
	{
		real_vector2d damage_direction;
		real_vector2d unit_forward;
		boolean resists_pings;
		boolean animation_flag;
		boolean direction_valid;
		real angle;

		damage_direction.i = damage_data->direction.i;
		damage_direction.j = damage_data->direction.j;
		unit_forward.i = unit->object.forward.i;
		unit_forward.j = unit->object.forward.j;
		resists_pings = FALSE;
		animation_flag = FALSE;
		direction_valid = FALSE;
		angle = 0.f;

		if (normalize2d(&damage_direction) > 0.f &&
			normalize2d(&unit_forward) > 0.f)
		{
			angle = signed_angle_between_vectors2d(
				&damage_direction,
				&unit_forward);
			direction_valid = TRUE;
		}

		if (TEST_FLAG(
				unit_definition->unit.flags,
				_unit_resists_pings_bit) &&
			!TEST_FLAG(
				damage_definition->flags,
				_damage_definition_pings_resistant_units_bit))
		{
			resists_pings = TRUE;
		}

		if (unit->unit.flaming_death_delay > 0)
		{
			resists_pings = TRUE;
		}

		if (damage_flags & UNIT_DAMAGE_AFTERMATH_ANIMATION_FLAGS_MASK)
		{
			animation_flag = TRUE;
		}

		unit_ping_animation(
			unit_index,
			lethal,
			feigned,
			instantaneous,
			resists_pings,
			animation_flag,
			angle,
			body_part,
			direction_valid ? &damage_direction : NULL);
	}

	{
		long owner_player_index = damage_data->owner_player_index;
		long player_index = unit->unit.player_index;

		if (owner_player_index != NONE && player_index != NONE)
		{
			game_engine_player_damaged_player(
				owner_player_index,
				player_index,
				TEST_FLAG(
					damage_flags,
					_object_being_damaged_by_friendly_bit));
		}
	}

	if (damage_data->owner_player_index != NONE ||
		damage_data->owner_object_index != NONE)
	{
		unit_record_damage(
			unit_index,
			total_damage,
			damage_definition->category,
			lethal,
			damage_data->owner_player_index,
			damage_data->owner_team_index,
			damage_data->owner_object_index);
	}

	if (!TEST_FLAG(
			damage_data->flags,
			_damage_silent_bit) &&
		(TEST_FLAG(
				damage_flags,
				_object_being_damaged_body_depleted_bit) ||
			body_damage > 0.f ||
			shield_damage > 0.f))
	{
		unit_make_damage_sound(
			unit_index,
			damage_data,
			lethal | feigned,
			TEST_FLAG(
				damage_flags,
				_object_being_damaged_killed_instantly_bit),
			body_damage,
			shield_damage);
	}

	if (body_damage > 0.f || shield_damage > 0.f)
	{
		unit_unzoom(unit_index);
	}

	if (unit->object.type == _object_type_biped)
	{
		if (lethal)
		{
			ai_handle_death(
				unit_index,
				damage_data->owner_object_index,
				damage_definition->category);
		}
		else if (!TEST_FLAG(unit->object.damage_flags, _object_dead_bit))
		{
			ai_handle_damage(
				unit_index,
				damage_data->owner_object_index,
				damage_definition->category,
				total_damage,
				&damage_data->direction,
				FALSE);
		}
	}

	if (unit->unit.player_index != NONE &&
		damage_definition->stun > 0.f &&
		(game_engine_running() || stun_enable))
	{
		struct game_globals_player_information *player_information =
			TAG_BLOCK_GET_ELEMENT(
				&scenario_get_game_globals()->player_information,
				0,
				struct game_globals_player_information);
		real stun_increment =
			damage_definition->stun * damage_data->scale;
		real maximum_stun =
			damage_definition->maximum_stun * damage_data->scale;
		short stun_ticks;
		short minimum_stun_ticks;
		short maximum_stun_ticks;

		if (stun_increment < 0.f)
		{
			stun_increment = 0.f;
		}
		if (maximum_stun < 0.f)
		{
			maximum_stun = 0.f;
		}
		else if (maximum_stun >= 1.f)
		{
			maximum_stun = 1.f;
		}

		if (unit->unit.body_stun < maximum_stun)
		{
			unit->unit.body_stun += stun_increment;
			if (unit->unit.body_stun > maximum_stun)
			{
				unit->unit.body_stun = maximum_stun;
			}
		}

		stun_ticks = (short)(
			damage_definition->stun_time * TICKS_PER_SECOND);
		minimum_stun_ticks = (short)(
			player_information->minimum_stun_time * TICKS_PER_SECOND);
		maximum_stun_ticks = (short)(
			player_information->maximum_stun_time * TICKS_PER_SECOND);

		if (unit->unit.body_stun_ticks < minimum_stun_ticks)
		{
			unit->unit.body_stun_ticks = minimum_stun_ticks;
		}
		unit->unit.body_stun_ticks += stun_ticks;
		if (unit->unit.body_stun_ticks > maximum_stun_ticks)
		{
			unit->unit.body_stun_ticks = maximum_stun_ticks;
		}
	}

	if (lethal || feigned)
	{
		unit_died(unit_index, feigned);
	}

	return;
}

void unit_flame_to_death(
	long unit_index)
{
	struct game_globals_falling_damage *falling_damage;
	struct unit_datum *unit;

	unit = unit_get(unit_index);
	falling_damage = TAG_BLOCK_GET_ELEMENT(
		&scenario_get_game_globals()->falling_damage,
		0,
		struct game_globals_falling_damage);

	{
		struct unit_datum *running_unit = unit_get(unit_index);

		running_unit->unit.flags &= ~FLAG(_unit_running_blindly_bit);
	}
	unit->unit.flags &= ~FLAG(_unit_ignore_hard_pings_bit);
	unit->object.damage_flags &= ~FLAG(_object_cannot_take_damage_bit);

	if (falling_damage &&
		falling_damage->flaming_death_damage_effect.index != NONE)
	{
		struct object_datum *attacker = object_try_and_get(
			unit->unit.flaming_death_attacker_object_index);
		struct damage_data damage_data;

		damage_data_new(
			&damage_data,
			falling_damage->flaming_death_damage_effect.index);

		if (attacker)
		{
			damage_data.owner_player_index = attacker->object.owner_player_index;
			if (attacker->object.owner_object_index == NONE)
			{
				damage_data.owner_object_index =
					unit->unit.flaming_death_attacker_object_index;
			}
			else
			{
				damage_data.owner_object_index =
					attacker->object.owner_object_index;
			}
			damage_data.owner_team_index = attacker->object.owner_team_index;
		}

		object_cause_damage(
			&damage_data,
			unit_index,
			NONE,
			NONE,
			NONE,
			NULL);
	}

	if (!TEST_FLAG(unit->object.damage_flags, _object_dead_bit))
	{
		error(
			_error_silent,
			"WARNING: %s tried to die from flaming to death but couldn't",
			tag_name_strip_path(tag_get_name(unit->definition_index)));
		SET_FLAG(unit->object.damage_flags, _object_die_act_of_god_bit, TRUE);
	}

	return;
}

void unit_get_head_position(
	long unit_index,
	union real_point3d *head_position)
{
	struct object_marker head_marker;

	object_get_marker_by_name(unit_index, "head", &head_marker, 1);
	*head_position = head_marker.matrix.position;

	return;
}

void unit_get_camera_position(
	long unit_index,
	real_point3d *camera_position)
{
	struct object_marker marker;

	struct unit_datum *unit = unit_get(unit_index);
	struct unit_definition *unit_definition = unit_definition_get(unit->definition_index);

	if (unit->object.parent_object_index!=NONE || TEST_FLAG(unit->object.damage_flags, _object_dead_bit) || unit->object.type!=_object_type_biped)
	{
		if (unit->object.parent_object_index==NONE)
		{
			if (unit->unit.gunner_object_index==NONE)
			{
				object_get_marker_by_name(unit_index, "head", &marker, 1);
				*camera_position = marker.matrix.position;
			}
			else
			{
				struct unit_datum *gunner_unit = unit_get(unit->unit.gunner_object_index);
				struct unit_seat *parent_seat = TAG_BLOCK_GET_ELEMENT(&unit_definition->unit.seats, gunner_unit->unit.parent_seat_index, struct unit_seat);

				object_get_marker_by_name(unit_index, parent_seat->marker_name, &marker, 1);
				*camera_position = marker.matrix.position;
			}
		}
		else
		{
			struct object_datum *object = object_get(unit->object.parent_object_index);

			*camera_position = object->object.position;

			if (TEST_FLAG(_object_mask_unit, object->object.type) && unit->unit.parent_seat_index!=NONE)
			{
				struct unit_definition *parent_unit_definition = unit_definition_get(object->definition_index);
				struct unit_seat *parent_seat = TAG_BLOCK_GET_ELEMENT(&parent_unit_definition->unit.seats, unit->unit.parent_seat_index, struct unit_seat);

				if (object->object.type!=_object_type_vehicle || parent_seat->camera.marker_name[0])
				{
					object_get_marker_by_name(unit->object.parent_object_index, parent_seat->camera.marker_name, &marker, 1);
					*camera_position = marker.matrix.position;
				}
			}
		}
	}
	else
	{
		biped_get_sight_position(unit_index, 0.f, NULL, NULL, NULL, camera_position);
	}

	return;
}

void unit_estimate_position(
	long unit_index,
	short estimate_mode,
	real_point3d const *body_position,
	real_vector3d *desired_facing,
	real_vector3d *desired_gun_offset,
	real_point3d *estimated_position)
{
	struct unit_datum *unit;
	real_point3d reference_position;

	unit = unit_get(unit_index);

	match_assert(
		"c:\\halo\\SOURCE\\units\\units.c",
		5297,
		body_position && estimated_position);
	match_assert(
		"c:\\halo\\SOURCE\\units\\units.c",
		5298,
		(estimate_mode >= 0) &&
		(estimate_mode < NUMBER_OF_UNIT_ESTIMATE_POSITION_MODES));

	if (unit->object.parent_object_index == NONE &&
		!TEST_FLAG(unit->object.damage_flags, _object_dead_bit))
	{
		if (unit->object.type == _object_type_biped)
		{
			biped_get_sight_position(
				unit_index,
				estimate_mode,
				body_position,
				desired_facing,
				desired_gun_offset,
				estimated_position);

			return;
		}
	}
	else if (unit->object.type == _object_type_biped &&
		unit->object.parent_object_index != NONE)
	{
		struct object_datum *parent_object =
			object_get(unit->object.parent_object_index);

		if (parent_object->object.type == _object_type_vehicle &&
			vehicle_find_pathfinding_surface_index(
				unit->object.parent_object_index,
				&reference_position) != NONE)
		{
			goto apply_delta;
		}
	}

	object_get_origin(unit_index, &reference_position);

apply_delta:
	unit_get_camera_position(unit_index, estimated_position);
	add_vectors3d(
		vector_from_points3d(
			&reference_position,
			body_position,
			(real_vector3d *)&reference_position),
		(real_vector3d *)estimated_position,
		(real_vector3d *)estimated_position);

	return;
}

void unit_debug_ninja_rope(
	long unit_index)
{
	struct collision_result collision;
	real_point3d origin;
	real_vector3d direction;
	struct unit_datum *unit;

	unit = unit_get(unit_index);
	unit_get_camera_position(unit_index, &origin);
	scale_vector3d(&unit->unit.aiming_vector, 25.f, &direction);

	match_assert(
		"c:\\halo\\SOURCE\\units\\units.c",
		6648,
		global_current_collision_user_depth <
			MAXIMUM_COLLISION_USER_STACK_DEPTH);
	global_current_collision_users[global_current_collision_user_depth++] = 21;

	if (collision_test_vector(
		FLAG(_collision_test_back_facing_surfaces_bit) |
			FLAG(_collision_test_structure_bit),
		&origin,
		&direction,
		unit_index,
		&collision))
	{
		if (collision.plane.n.k > 0.95f)
		{
			collision.point.z += 0.25f;
			object_translate(unit_index, &collision.point, NULL);
		}
	}

	match_assert(
		"c:\\halo\\SOURCE\\units\\units.c",
		6657,
		global_current_collision_user_depth > 1);
	--global_current_collision_user_depth;

	return;
}

boolean unit_clip_to_aiming_bounds(
	long unit_index,
	real_vector3d *vector,
	boolean use_aiming_screen)
{

	boolean aiming;
	real_rectangle2d *bounds;

	struct unit_datum *unit = unit_get(unit_index);
	boolean result = FALSE;

	if (use_aiming_screen)
	{
		aiming = unit->unit.animation.aiming_with_euler_screen;
		bounds = &unit->unit.animation.aiming_screen_bounds;
	}
	else
	{
		aiming = unit->unit.animation.looking_with_euler_screen;
		bounds = &unit->unit.animation.looking_screen_bounds;
	}

	match_assert_valid_real_normal3d("c:\\halo\\SOURCE\\units\\units.c", 5608, vector);

	if (aiming)
	{
		real_matrix4x3 matrix;
		real_euler_angles2d relative_aiming_angles;
		real_vector3d relative_vector;

		matrix.scale = 1.f;
		object_get_orientation(unit_index, &matrix.forward, &matrix.up);
		cross_product3d(&matrix.up, &matrix.forward, &matrix.left);
		matrix.position = *global_origin3d;
		matrix4x3_inverse_transform_normal(&matrix, vector, &relative_vector);

		match_assert_valid_real_vector3d("c:\\halo\\SOURCE\\units\\units.c", 5626, &relative_vector);

		euler_angles2d_from_vector3d(&relative_aiming_angles, &relative_vector);

		match_assert_valid_real("c:\\halo\\SOURCE\\units\\units.c", 5629, relative_aiming_angles.pitch);
		match_assert_valid_real("c:\\halo\\SOURCE\\units\\units.c", 5630, relative_aiming_angles.yaw);

		if (relative_aiming_angles.yaw<bounds->x0)
		{
			relative_aiming_angles.yaw = bounds->x0;
			result = TRUE;
		}
		else
		{
			if (relative_aiming_angles.yaw>bounds->x1)
			{
				relative_aiming_angles.yaw = bounds->x1;
				result = TRUE;
			}
		}

		if (relative_aiming_angles.pitch<bounds->y0)
		{
			result = TRUE;
			relative_aiming_angles.pitch = bounds->y0;
		}
		else
		{
			if (relative_aiming_angles.pitch>bounds->y1)
			{
				result = TRUE;
				relative_aiming_angles.pitch = bounds->y1;
			}
		}

		if (result)
		{
			match_assert_valid_real("c:\\halo\\SOURCE\\units\\units.c", 5661, relative_aiming_angles.pitch);
			match_assert_valid_real("c:\\halo\\SOURCE\\units\\units.c", 5662, relative_aiming_angles.yaw);
			vector3d_from_euler_angles2d(&relative_vector, &relative_aiming_angles);

			match_assert_valid_real_vector3d("c:\\halo\\SOURCE\\units\\units.c", 5665, &relative_vector);
			matrix4x3_transform_normal(&matrix, &relative_vector, vector);
			match_assert_valid_real_vector3d("c:\\halo\\SOURCE\\units\\units.c", 5667, vector);
		}
	}

	return result;
}

long unit_inventory_get_weapon(
	long unit_index, 
	short index)
{
	struct unit_datum *unit = unit_get(unit_index);

	return unit_get_weapon(unit, index);
}

static void unit_ready_desired_weapon(
	long unit_index,
	boolean immediate)
{
	struct unit_datum *unit = unit_get(unit_index);
	struct unit_definition *unit_definition = unit_definition_get(unit->definition_index);
	long desired_weapon_index = unit_get_desired_weapon_index(unit_index);
	long current_weapon_index = unit_get_current_weapon_index(unit_index);

	if (current_weapon_index!=NONE && weapon_put_away(current_weapon_index, immediate))
	{
		object_detach(current_weapon_index);
		object_disconnect_from_map(current_weapon_index);
		object_activate(current_weapon_index);
		object_set_visibility(current_weapon_index, FALSE);
		item_in_unit_inventory(current_weapon_index, unit_index);
		unit->unit.current_weapon_index = NONE;
	}

	if (unit->unit.current_weapon_index==NONE)
	{
		if (desired_weapon_index!=NONE)
		{
			struct animation_graph *animation_graph;
			struct animation_graph_unit_seat *unit_seat;
			struct animation_graph_weapon_class *weapon_class;

			unit_set_or_test_seat_and_weapon_label(unit_index, unit_get_seat_label(unit_index), weapon_get_label(desired_weapon_index), TRUE);

			animation_graph = animation_graph_definition_get(unit_definition->object.animation_graph.index);
			unit_seat = TAG_BLOCK_GET_ELEMENT(&animation_graph->unit_seats, unit->unit.animation.seat_index, struct animation_graph_unit_seat);
			weapon_class = TAG_BLOCK_GET_ELEMENT(&unit_seat->weapon_classes, unit->unit.animation.weapon_index, struct animation_graph_weapon_class);

			object_reconnect_to_map(desired_weapon_index, NULL);
			object_set_visibility(desired_weapon_index, TRUE);
			object_attach_to_marker(unit_index, weapon_class->hand_marker_name, desired_weapon_index, weapon_class->grip_marker_name);
			unit->unit.current_weapon_index = unit->unit.desired_weapon_index;
			
			if (unit->unit.desired_weapon_index!=NONE)
			{
				unit->unit.weapon_last_used_at_game_time[unit->unit.current_weapon_index] = game_time_get();
			}

			weapon_ready(desired_weapon_index);
		}
		else
		{
			unit_set_or_test_seat_and_weapon_label(unit_index, unit_get_seat_label(unit_index), base_weapon_label_get(_unit_base_weapon_none), TRUE);
			unit->unit.current_weapon_index = NONE;
		}
	}

	unit_unzoom(unit_index);

	return;
}

short unit_inventory_next_weapon(
	long unit_index,
	short current_index,
	short delta)
{
	return unit_weapon_next_index(
		unit_index,
		current_index,
		delta);
}

boolean unit_drop_current_weapon(
	long unit_index,
	boolean immediate)
{
	struct unit_datum *unit = unit_get(unit_index);
	struct unit_definition *unit_definition = unit_definition_get(unit->definition_index);
	long current_weapon_index = unit_get_current_weapon_index(unit_index);
	short next_index = unit_weapon_next_index(unit_index, unit->unit.current_weapon_index, 1);

	boolean result = FALSE;

	if (current_weapon_index != NONE &&
		(next_index != unit->unit.current_weapon_index || immediate) &&
		!TEST_FLAG(object_get(current_weapon_index)->object.flags, _object_invisible_bit) &&
		weapon_put_away(current_weapon_index, immediate))
	{
		first_person_weapon_message_from_unit(unit_index, 13);
		unit_drop_item(unit_index, current_weapon_index);
		unit->unit.weapon_object_indices[unit->unit.current_weapon_index] = NONE;
		unit->unit.current_weapon_index = NONE;
		unit->unit.desired_weapon_index = unit_weapon_next_index(unit_index, NONE, 0);
		
		if (!weapon_can_be_fired(current_weapon_index))
		{
			object_delete(current_weapon_index);
		}

		result = TRUE;
	}

	return result;
}

/* port: a swap's weapon out: the one the player chose, though the weapon
in hand is still being put away for it (a network player's choice gets to
the host before the switch is done), not that one, their backup */
boolean unit_drop_selected_weapon(
	long unit_index)
{
	struct unit_datum *unit = unit_get(unit_index);
	short slot = unit->unit.desired_weapon_index;
	long weapon_index = unit_inventory_get_weapon(unit_index, slot);

	if (weapon_index == NONE || slot == unit->unit.current_weapon_index ||
		TEST_FLAG(weapon_definition_get(weapon_get(weapon_index)->definition_index)->weapon.flags,
			_weapon_doesnt_count_toward_maximum_bit))
	{
		/* port: (an Infection zombie keeps its skull: infection.c) */
		if (!infection_allow_weapon_drop(unit_index, unit_get_current_weapon_index(unit_index)))
			return FALSE;
		return unit_drop_current_weapon(unit_index, TRUE);
	}
	if (!infection_allow_weapon_drop(unit_index, weapon_index))
		return FALSE;
	unit_drop_item(unit_index, weapon_index);
	unit->unit.weapon_object_indices[slot] = NONE;
	unit->unit.desired_weapon_index = unit->unit.current_weapon_index!=NONE ?
		unit->unit.current_weapon_index : unit_weapon_next_index(unit_index, NONE, 0);
	if (!weapon_can_be_fired(weapon_index))
	{
		object_delete(weapon_index);
	}

	return TRUE;
}

/* ---------- private code */

char const *base_seat_labels[NUMBER_OF_UNIT_BASE_SEATS] = {"asleep", "alert", "stand", "crouch", "flee", "flaming"};

static char const *base_seat_label_get(
	short base_seat_index)
{
	match_assert("c:\\halo\\SOURCE\\units\\units.c", 8214, base_seat_index>=0 && base_seat_index<NUMBER_OF_UNIT_BASE_SEATS);
	return base_seat_labels[base_seat_index];
}

static short seat_label_to_base_seat_index(
	char const *seat_label)
{
	short seat_index;
	short result = NONE;

	for (seat_index = 0; seat_index<NUMBER_OF_UNIT_BASE_SEATS; ++seat_index)
	{
		if (!_stricmp(seat_label, base_seat_labels[seat_index]))
		{
			result = seat_index;
			break;
		}
	}

	return result;
}

static char const *base_weapon_label_get(
	short base_weapon_index)
{
	char const *base_weapon_labels[NUMBER_OF_UNIT_BASE_WEAPONS] = {"unarmed"};

	match_assert("c:\\halo\\SOURCE\\units\\units.c", 8266, base_weapon_index>=0 && base_weapon_index<NUMBER_OF_UNIT_BASE_WEAPONS);

	return base_weapon_labels[base_weapon_index];
}

static void unit_refresh_illumination(
	long unit_index)
{
	struct unit_datum *unit = unit_get(unit_index);
	struct unit_datum *parent_unit = unit_try_and_get(unit->object.parent_object_index);

	if (parent_unit==NULL)
	{
		real_rgb_color color;
		lights_illumination_at_point(&unit->object.position, &unit->object.location, &color);
		unit->unit.ambient_illumination = real_rgb_color_brightness(&color);
		unit->unit.self_illumination = object_get_self_illumination(unit_index);
	}
	else
	{
		unit->unit.ambient_illumination = parent_unit->unit.ambient_illumination;
		unit->unit.self_illumination = parent_unit->unit.self_illumination;
	}

	return;
}

static short unit_weapon_next_index(
	long unit_index,
	short current_index,
	short delta)
{
	short inventory_index;
	
	struct unit_datum *unit = unit_get(unit_index);
	short selected_weapon_index = NONE;

	if (current_index==NONE)
	{
		current_index = 0;
	}

	match_assert("c:\\halo\\SOURCE\\units\\units.c", 7744, current_index>=0 && current_index<MAXIMUM_WEAPONS_PER_UNIT);
	
	inventory_index = current_index;
	
	do
	{
		long current_weapon_index = unit->unit.weapon_object_indices[inventory_index];

		if (current_weapon_index != NONE && unit_can_use_weapon(unit_index, current_weapon_index))
		{
			if (delta ||
				selected_weapon_index == NONE ||
				unit->unit.weapon_last_used_at_game_time[selected_weapon_index] <
					unit->unit.weapon_last_used_at_game_time[inventory_index])
			{
				selected_weapon_index = inventory_index;
			}

			if (weapon_must_be_readied(unit->unit.weapon_object_indices[inventory_index]) ||
				inventory_index != current_index)
			{
				break;
			}
		}

		if (delta<0)
		{
			inventory_index =
				inventory_index == 0 ?
					MAXIMUM_WEAPONS_PER_UNIT-1 :
					inventory_index-1;
		}
		else
		{
			inventory_index = inventory_index == (MAXIMUM_WEAPONS_PER_UNIT-1) ? 0 : inventory_index+1;
		}

	}
	while (inventory_index != current_index);

	return selected_weapon_index;
}

static void unit_throw_grenade_move_to_hand(
	long unit_index)
{
	long object_index;

	struct object_placement_data placement_data;
	struct object_marker marker;

	struct unit_datum *unit = unit_get(unit_index);
	struct unit_definition *unit_definition = unit_definition_get(unit->definition_index);
	struct game_globals_grenade *grenades = TAG_BLOCK_GET_ELEMENT(
		&scenario_get_game_globals()->grenades,
		unit->unit.current_grenade_index,
		struct game_globals_grenade);

	if ((
		unit->unit.player_index==NONE ||
		!cheat.infinite_ammo && !game_engine_infinite_grenades(unit->unit.player_index)) &&
		(unit->unit.actor_index==NONE ||
		!actor_has_unlimited_grenades(unit->unit.actor_index)))
	{
		match_assert("c:\\halo\\SOURCE\\units\\units.c", 7966, unit->unit.current_grenade_index>=0 && unit->unit.current_grenade_index<NUMBER_OF_UNIT_GRENADE_TYPES);
		/* port: a distributed client's grenade counts are the host's (what
		every unit carries, network_objects.c), which can arrive between a
		throw starting and the grenade reaching the hand, the throw already
		counted there: the host's count stands */
		if (network_game_distributed_client())
		{
			if (unit->unit.grenade_counts[unit->unit.current_grenade_index] > 0)
				--unit->unit.grenade_counts[unit->unit.current_grenade_index];
		}
		else
		{
		match_assert("c:\\halo\\SOURCE\\units\\units.c", 7967, unit->unit.grenade_counts[unit->unit.current_grenade_index]>0);
		--unit->unit.grenade_counts[unit->unit.current_grenade_index];
		}
	}

	object_get_marker_by_name(unit_index, "left hand", &marker, 1);
	object_placement_data_new(&placement_data, grenades->projectile.index, unit_index);
	SET_FLAG(placement_data.flags, _new_object_never_automatically_delete_bit, TRUE);
	placement_data.forward= unit_get(unit_index)->unit.aiming_vector;
	normalize3d(perpendicular3d(&placement_data.forward, &placement_data.up));

	placement_data.position = marker.matrix.position;
	object_index = object_new(&placement_data);

	if (object_index!=NONE)
	{
		object_attach_to_node(unit_index, object_index, marker.node_index);
		unit->unit.grenade_object_index = object_index;
		unit->unit.grenade_throw_state = _unit_grenade_throw_in_hand;
		/* port: the host takes a client's report of the grenade's damage only
		from a grenade thrown (network_damage.c) */
		network_damage_note_grenade(unit_index, unit->unit.current_grenade_index);
	}
	else
	{
		unit->unit.grenade_throw_state = _unit_grenade_throw_ending;
	}

	return;
}

static boolean unit_animation_overlay_action_loops(
	struct unit_animation *animation)
{
	boolean result = FALSE;

	if (animation->state>=_unit_state_turn_right &&
		animation->state<=_unit_state_move_front)
	{
		result = TRUE;
	}

	return result;
}

static boolean unit_animation_state_interruptable(
	struct unit_animation *animation,
	short desired_state)
{
	boolean result = TRUE;

	switch (animation->state)
	{
	case _unit_state_ai_impulse:
	case _unit_state_melee_attack:
	case _unit_state_melee_airborne:
	case _unit_state_throw_grenade:
	case _unit_state_resurrect_front:
	case _unit_state_resurrect_back:
	case _unit_state_leap_start:
	case _unit_state_leap_melee:
		if (desired_state!=_unit_state_hard_ping)
		{
			return FALSE;
		}
		break;

	case _unit_state_dying_airborne:
	case _unit_state_dying:
		{
			long state = desired_state;

			if (state<_unit_state_dying_airborne ||
				state>_unit_state_dying)
			{
				goto state_not_allowed;
			}

			return TRUE;
		}

	case _unit_state_turn_left:
	case _unit_state_turn_right:
	case _unit_state_opening:
	case _unit_state_closing:
		if (desired_state!=_unit_state_idle)
		{
			break;
		}
		/* fall through */
	case _unit_state_hard_ping:
	case _unit_state_entering_seat:
	case _unit_state_exiting_seat:
	case _unit_state_user_animation:
	state_not_allowed:
		result = FALSE;
		break;

	default:
		break;
	}

	return result;
}

static boolean unit_animation_state_loops(
	struct unit_animation *animation)
{
	boolean result = TRUE;

	switch (animation->state)
	{
	case _unit_state_gesture:
	case _unit_state_turn_left:
	case _unit_state_turn_right:
	case _unit_state_hard_ping:
	case _unit_state_entering_seat:
	case _unit_state_exiting_seat:
	case _unit_state_user_animation:
	case _unit_state_ai_impulse:
	case _unit_state_melee_attack:
	case _unit_state_melee_airborne:
	case _unit_state_throw_grenade:
	case _unit_state_resurrect_front:
	case _unit_state_resurrect_back:
	case _unit_state_leap_start:
	case _unit_state_leap_melee:
		result = FALSE;
		break;
	default:
		break;
	}

	return result;
}

static boolean unit_animation_weapon_ik(
	struct unit_animation *animation)
{
	boolean result = animation->soft_ping_animation.index==NONE;

	if (animation->action)
	{
		result = FALSE;
	}

	switch (animation->state)
	{
	case _unit_state_flying_front:
	case _unit_state_flying_back:
	case _unit_state_flying_left:
	case _unit_state_flying_right:
	case _unit_state_hard_ping:
	case _unit_state_dying_airborne:
	case _unit_state_dying:
	case _unit_state_entering_seat:
	case _unit_state_exiting_seat:
	case _unit_state_user_animation:
	case _unit_state_ai_impulse:
	case _unit_state_melee_attack:
	case _unit_state_melee_airborne:
	case _unit_state_melee_continuous:
	case _unit_state_throw_grenade:
	case _unit_state_resurrect_front:
	case _unit_state_resurrect_back:
	case _unit_state_leap_start:
	case _unit_state_leap_airborne:
	case _unit_state_leap_melee:
		result = FALSE;
		break;
	default:
		break;
	}

	return result;
}

static boolean unit_animation_vehicle_ik(
	struct unit_animation *animation)
{
	boolean result = TRUE;

	switch (animation->state)
	{
	case _unit_state_hard_ping:
	case _unit_state_dying_airborne:
	case _unit_state_dying:
	case _unit_state_entering_seat:
	case _unit_state_exiting_seat:
	case _unit_state_ai_impulse:
	case _unit_state_resurrect_front:
	case _unit_state_resurrect_back:
		result = FALSE;
		break;
	default:
		break;
	}

	return result;
}

static boolean unit_animation_busy(
	struct unit_animation *animation)
{
	boolean result = FALSE;

	switch (animation->state)
	{
	case _unit_state_hard_ping:
	case _unit_state_dying_airborne:
	case _unit_state_dying:
	case _unit_state_entering_seat:
	case _unit_state_exiting_seat:
	case _unit_state_ai_impulse:
	case _unit_state_melee_attack:
	case _unit_state_melee_airborne:
	case _unit_state_melee_continuous:
	case _unit_state_throw_grenade:
	case _unit_state_resurrect_front:
	case _unit_state_resurrect_back:
	case _unit_state_leap_start:
	case _unit_state_leap_melee:
		result = TRUE;
		break;
	default:
		break;
	}

	return result;
}

static boolean unit_set_or_test_seat_and_weapon_label(
	long object_index,
	char const *seat_label,
	char const *weapon_label,
	boolean change_flag)
{
	short seat_index;

	struct unit_datum *unit = unit_get(object_index);
	struct unit_definition *unit_definition = unit_definition_get(unit->definition_index);
	struct animation_graph *animation_graph = animation_graph_definition_get(unit_definition->object.animation_graph.index);
	boolean result = FALSE;

	for (seat_index = 0; seat_index<animation_graph->unit_seats.count; ++seat_index)
	{
		struct animation_graph_unit_seat *unit_seat = TAG_BLOCK_GET_ELEMENT(&animation_graph->unit_seats, seat_index, struct animation_graph_unit_seat);

		if (!seat_label || !_stricmp(seat_label, unit_seat->label))
		{
			short weapon_class_index;

			for (weapon_class_index = 0; weapon_class_index<unit_seat->weapon_classes.count; ++weapon_class_index)
			{
				short weapon_type_index;

				struct animation_graph_weapon_class *weapon_class = TAG_BLOCK_GET_ELEMENT(&unit_seat->weapon_classes, weapon_class_index, struct animation_graph_weapon_class);

				for (weapon_type_index = 0; weapon_type_index<weapon_class->weapon_types.count; ++weapon_type_index)
				{
					struct animation_graph_weapon_type *weapon_type = TAG_BLOCK_GET_ELEMENT(&weapon_class->weapon_types, weapon_type_index, struct animation_graph_weapon_type);
					
					if (!weapon_label ||
						!strcmp(weapon_label, "unarmed") &&
						weapon_type->label[0]=='\0'||
						!_stricmp(weapon_label, weapon_type->label))
					{
						if (change_flag)
						{
							long anim_2 =
								unit_seat->animations.count <= 2 ?
								NONE :
								animation_graph_animation_index_get(&unit_seat->animations)[2].animation_index;
							boolean showing_acceleration;
							
							if (anim_2==NONE)
							{
								long anim_3 =
									unit_seat->animations.count <= 3 ?
									NONE :
									animation_graph_animation_index_get(&unit_seat->animations)[3].animation_index;
								
								if (anim_3==NONE)
								{
									long anim_4 =
										unit_seat->animations.count <= 4 ?
										NONE :
										animation_graph_animation_index_get(&unit_seat->animations)[4].animation_index;

									if (anim_4==NONE)
									{
										showing_acceleration = FALSE;
										goto acceleration_determined;
									}
								}
							}
							showing_acceleration = TRUE;
							acceleration_determined:

							if (unit->unit.animation.state!=_unit_state_user_animation)
							{
								unit->unit.animation.state = NONE;
							}

							unit->unit.animation.seat_index = seat_index;
							unit->unit.animation.base_seat_index = seat_label_to_base_seat_index(seat_label);
							unit->unit.animation.weapon_index = weapon_class_index;
							unit->unit.animation.weapon_type_index = weapon_type_index;

							SET_FLAG(unit->unit.animation.flags, _unit_animation_showing_acceleration_bit, showing_acceleration);
						}

						result = TRUE;
						break;
					}
				}
			}
		}
	}

	return result;
}

static boolean unit_euler_axis_doplan(
	struct unit_acceleration_plan *plan,
	real delta_time,
	real position,
	real *new_position,
	real velocity,
	real *new_velocity)
{
	boolean result = plan->at_rest;
	real current_position = position;
	real current_velocity = velocity;
	real time;
	real remaining_time;

	if (result)
	{
		goto store_result;
	}
	remaining_time = delta_time;
	if (!(remaining_time > 0.f))
	{
		goto store_result;
	}

	if (plan->acceleration_time>0.f)
	{
		time = MIN(remaining_time, plan->acceleration_time);
		current_position +=
			(time*plan->acceleration*0.5f + current_velocity)*time;
		current_velocity += time*plan->acceleration;
		remaining_time -= time;
	}

	if (!(remaining_time > 0.f))
	{
		goto store_result;
	}

	if (plan->coast_time>0.f)
	{
		time = MIN(remaining_time, plan->coast_time);
		current_position += current_velocity*time;
		remaining_time -= time;
	}

	if (!(remaining_time > 0.f))
	{
		goto store_result;
	}

	if (plan->deceleration_time>0.f)
	{
		time = MIN(remaining_time, plan->deceleration_time);
		current_position +=
			(0.5f*time*plan->deceleration + current_velocity)*time;
		current_velocity += time*plan->deceleration;
		remaining_time -= time;
	}

	if (remaining_time > 0.f)
	{
		result = TRUE;
	}

store_result:
	*new_position = current_position;
	*new_velocity = current_velocity;

	return result;
}
void unit_set_actively_controlled(
	long unit_index,
	boolean actively_controlled)
{
	struct unit_datum *unit = unit_get(unit_index);
	short weapon_index;

	if (unit->unit.actor_index!=NONE ||
		unit->unit.swarm_actor_index!=NONE ||
		unit->unit.player_index!=NONE)
	{
		actively_controlled = TRUE;
	}

	actively_controlled =
		!TEST_FLAG(unit->object.damage_flags, _object_dead_bit) &&
		actively_controlled;

	SET_FLAG(unit->unit.flags, _unit_actively_controlled_bit, actively_controlled);
	SET_FLAG(unit->unit.flags, _unit_controllable_bit, actively_controlled);

	for (weapon_index = 0; weapon_index<MAXIMUM_WEAPONS_PER_UNIT; ++weapon_index)
	{
		long weapon_object_index = unit->unit.weapon_object_indices[weapon_index];

		if (weapon_object_index!=NONE)
		{
			item_in_unit_inventory(weapon_object_index, unit_index);
		}
	}

	unit_update_driver_and_gunner(unit_index);

	return;
}
static void unit_update_driver_and_gunner(
	long unit_index)
{
	struct unit_datum *unit = unit_get(unit_index);
	struct unit_definition *unit_definition = unit_definition_get(unit->definition_index);
	long child_object_index = unit->object.first_child_object_index;

	while (child_object_index!=NONE)
	{
		struct object_datum *child_object = object_get(child_object_index);

		if (TEST_FLAG(_object_mask_unit, child_object->object.type))
		{
			struct unit_datum *child_unit = (struct unit_datum *)child_object;

			if (child_unit->unit.parent_seat_index!=NONE)
			{
				struct unit_seat *seat = TAG_BLOCK_GET_ELEMENT(
					&unit_definition->unit.seats,
					child_unit->unit.parent_seat_index,
					struct unit_seat);

				if (TEST_FLAG(seat->flags, _unit_seat_driver_bit) &&
					!TEST_FLAG(unit->unit.flags, _unit_actively_controlled_bit) &&
					unit->unit.driver_object_index==NONE)
				{
					unit->unit.driver_object_index = child_object_index;

					if (TEST_FLAG(seat->flags, _unit_seat_gunner_bit) &&
						unit->unit.gunner_object_index==NONE)
					{
						unit->unit.gunner_object_index = child_object_index;
					}
				}
				else if (TEST_FLAG(seat->flags, _unit_seat_gunner_bit) &&
					(unit->unit.gunner_object_index==NONE ||
					unit->unit.gunner_object_index==unit->unit.driver_object_index))
				{
					unit->unit.gunner_object_index = child_object_index;
				}
			}
		}

		child_object_index = child_object->object.next_object_index;
	}

	return;
}
static short unit_first_free_weapon_index(
	long unit_index)
{
	struct unit_datum *unit = unit_get(unit_index);
	short result = NONE;
	short weapon_index;

	for (weapon_index = 0; weapon_index<MAXIMUM_WEAPONS_PER_UNIT; ++weapon_index)
	{
		if (unit->unit.weapon_object_indices[weapon_index]==NONE)
		{
			result = weapon_index;
			break;
		}
	}

	return result;
}
boolean unit_add_weapon_to_inventory(
	long unit_index,
	long weapon_index,
	long is_starting_weapon)
{
	struct weapon_datum *weapon = weapon_get(weapon_index);
	struct unit_datum *unit = unit_get(unit_index);
	short inventory_index;
	boolean added = FALSE;
	short mode = (short)is_starting_weapon;

	/* port: only a weapon goes in a unit's weapons. A map's tag reference
	(a starting or initial weapon, an item collection) can name any object,
	which was then used as a weapon. In the released maps they all name
	weapons (224 initial weapons, 32 item collection entries, every
	starting profile) */
	if (weapon->object.type!=_object_type_weapon)
	{
		static boolean reported = FALSE;

		if (!reported)
		{
			reported = TRUE;
			error(_error_silent, "### ERROR %s isn't a weapon; it can't be carried",
				tag_get_name(weapon->definition_index));
		}
		return FALSE;
	}

	if (TEST_FLAG(weapon->object.flags, _object_connected_to_map_bit) &&
		weapon->object.parent_object_index==NONE &&
		unit_can_use_weapon(unit_index, weapon_index) &&
		game_engine_picking_up(unit_index, weapon_index))
	{
		if (mode==_unit_add_weapon_replace)
		{
			unit_delete_all_weapons(unit_index);
		}

		inventory_index = unit_first_free_weapon_index(unit_index);

		if (inventory_index!=NONE)
		{
			object_disconnect_from_map(weapon_index);
			object_set_visibility(weapon_index, FALSE);
			item_in_unit_inventory(weapon_index, unit_index);

			unit->unit.weapon_object_indices[inventory_index] = weapon_index;
			unit->unit.weapon_last_used_at_game_time[inventory_index] = 0;

			switch (mode)
			{
			case _unit_add_weapon_normal:
				unit->unit.desired_weapon_index =
					unit_weapon_next_index(unit_index, unit->unit.current_weapon_index, 0);
				break;

			case _unit_add_weapon_starting:
				/* port: a second weapon, into the empty slot of a one-weapon
				loadout, is the one in hand. While the trigger was down this
				left the choice on the weapon already held, and the next
				action put that choice back, so the new one stayed holstered. */
				{
					boolean holding_another = FALSE;
					short slot;

					for (slot = 0; slot < MAXIMUM_WEAPONS_PER_UNIT; slot++)
					{
						if (slot != inventory_index &&
							unit->unit.weapon_object_indices[slot] != NONE)
						{
							holding_another = TRUE;
						}
					}
					if (holding_another ||
						!TEST_FLAG(unit->unit.control_flags, _unit_control_weapon_primary_trigger_bit))
					{
						player_control_set_desired_weapon(unit_index, inventory_index);
					}
				}

			case _unit_add_weapon_replace:
				unit->unit.desired_weapon_index = inventory_index;
				break;
			}

			added = TRUE;
		}
	}

	return added;
}
static short unit_animation_impulse_get_index(
	short animation_impulse,
	short *interpolation_frame_count)
{
	short index = NONE;

	match_assert(
		"c:\\halo\\SOURCE\\units\\units.c",
		5364,
		animation_impulse>=0 && animation_impulse<NUMBER_OF_UNIT_ANIMATION_IMPULSES);

	switch (animation_impulse)
	{
	case 0:
		index = _unit_weapon_class_animation_berserk;
		break;
	case 1:
		index = _unit_weapon_class_animation_signal_move;
		break;
	case 2:
		index = _unit_weapon_class_animation_signal_attack;
		break;
	case 3:
		index = _unit_weapon_class_animation_signal_warn;
		break;
	case 4:
		index = _unit_weapon_class_animation_surprise_front;
		break;
	case 5:
		index = _unit_weapon_class_animation_surprise_back;
		break;
	case 6:
		index = _unit_weapon_class_animation_evade_left;
		break;
	case 7:
		index = _unit_weapon_class_animation_evade_right;
		break;
	case 8:
		index = _unit_weapon_class_animation_diving_front;
		break;
	case 9:
		index = _unit_weapon_class_animation_diving_back;
		break;
	case 10:
		index = _unit_weapon_class_animation_diving_left;
		break;
	case 11:
		index = _unit_weapon_class_animation_diving_right;
		break;
	case 12:
		index = _unit_weapon_class_animation_celebrate;
		break;
	case 13:
		index = _unit_weapon_class_animation_panic;
		break;
	default:
		display_assert(NULL, "c:\\halo\\SOURCE\\units\\units.c", 5383, TRUE);
		system_exit(-1);
		break;
	}

	match_assert(
		"c:\\halo\\SOURCE\\units\\units.c",
		5385,
		index!=NONE);

	if (interpolation_frame_count!=NULL)
	{
		switch (animation_impulse)
		{
		case 4:
		case 5:
		case 8:
		case 9:
		case 10:
		case 11:
			*interpolation_frame_count = 3;
			break;
		case 0:
		case 1:
		case 2:
		case 3:
		case 6:
		case 7:
		case 12:
		case 13:
			*interpolation_frame_count = 6;
			break;
		default:
			display_assert(NULL, "c:\\halo\\SOURCE\\units\\units.c", 5412, TRUE);
			system_exit(-1);
			break;
		}
	}

	return index;
}
static boolean unit_can_play_animation_impulse(
	long unit_index,
	short animation_impulse)
{
	struct unit_datum *unit = unit_get(unit_index);
	boolean result = FALSE;

	switch (unit->unit.animation.state)
	{
	case _unit_state_hard_ping:
	case _unit_state_dying_airborne:
	case _unit_state_dying:
	case _unit_state_entering_seat:
	case _unit_state_exiting_seat:
	case _unit_state_ai_impulse:
	case _unit_state_melee_attack:
	case _unit_state_melee_airborne:
	case _unit_state_melee_continuous:
	case _unit_state_throw_grenade:
	case _unit_state_resurrect_front:
	case _unit_state_resurrect_back:
	case _unit_state_leap_start:
	case _unit_state_leap_melee:
		result = FALSE;
		break;

	default:
		if (unit->object.parent_object_index!=NONE)
		{
			struct unit_datum *parent_unit;
			struct unit_definition *parent_unit_definition;
			struct unit_seat *seat;

			if (unit->unit.parent_seat_index!=NONE)
			{
				parent_unit = unit_try_and_get(unit->object.parent_object_index);
				if (parent_unit!=NULL)
				{
					long impulse_index;

					parent_unit_definition = unit_definition_get(parent_unit->definition_index);
					seat = TAG_BLOCK_GET_ELEMENT(
						&parent_unit_definition->unit.seats,
						unit->unit.parent_seat_index,
						struct unit_seat);
					impulse_index = animation_impulse;

					if (impulse_index>=12 &&
						impulse_index<=13)
					{
						result = (boolean)TEST_FLAG(seat->flags, _unit_seat_unknown8_bit);
					}
				}
			}
		}
		else
		{
			long impulse_index = animation_impulse;

			if (impulse_index>=12 &&
				impulse_index<=13)
			{
				result = FALSE;
			}
			else
			{
				result = TRUE;
			}
		}
		break;
	}

	return result;
}

boolean unit_test_animation_impulse(
	long unit_index,
	short animation_impulse)
{
	struct unit_datum *unit = unit_get(unit_index);
	struct unit_definition *unit_definition;
	struct animation_graph *animation_graph;
	struct animation_graph_unit_seat *unit_seat;
	struct animation_graph_weapon_class *weapon_class;
	short interpolation_frame_count;
	short animation_type;

	if (unit_can_play_animation_impulse(unit_index, animation_impulse))
	{
		unit_definition = unit_definition_get(unit->definition_index);
		animation_graph = animation_graph_definition_get(
			unit_definition->object.animation_graph.index);
		unit_seat = TAG_BLOCK_GET_ELEMENT(
			&animation_graph->unit_seats,
			unit->unit.animation.seat_index,
			struct animation_graph_unit_seat);
		weapon_class = TAG_BLOCK_GET_ELEMENT(
			&unit_seat->weapon_classes,
			unit->unit.animation.weapon_index,
			struct animation_graph_weapon_class);
		animation_type = unit_animation_impulse_get_index(
			animation_impulse,
			&interpolation_frame_count);

		if (animation_type>=0 && animation_type<weapon_class->animations.count)
		{
			animation_type =
				animation_graph_animation_index_get(&weapon_class->animations)
					[animation_type].animation_index;
		}
		else
		{
			animation_type = NONE;
		}

		return (boolean)(animation_type!=NONE);
	}

	return FALSE;
}

boolean unit_start_animation_impulse(
	long unit_index,
	short animation_impulse,
	real_vector2d *alignment_vector)
{
	struct unit_datum *unit = unit_get(unit_index);
	struct unit_definition *unit_definition;
	struct animation_graph *animation_graph;
	struct animation_graph_unit_seat *unit_seat;
	struct animation_graph_weapon_class *weapon_class;
	short interpolation_frame_count;
	short animation_type;
	short animation_index;
	long animation_graph_index;
	boolean result = FALSE;

	if (unit_can_play_animation_impulse(unit_index, animation_impulse))
	{
		unit_definition = unit_definition_get(unit->definition_index);
		animation_graph = animation_graph_definition_get(
			unit_definition->object.animation_graph.index);
		unit_seat = TAG_BLOCK_GET_ELEMENT(
			&animation_graph->unit_seats,
			unit->unit.animation.seat_index,
			struct animation_graph_unit_seat);
		weapon_class = TAG_BLOCK_GET_ELEMENT(
			&unit_seat->weapon_classes,
			unit->unit.animation.weapon_index,
			struct animation_graph_weapon_class);
		animation_type = unit_animation_impulse_get_index(
			animation_impulse,
			&interpolation_frame_count);

		if (animation_type>=0 &&
			animation_type<weapon_class->animations.count)
		{
			animation_index =
				animation_graph_animation_index_get(&weapon_class->animations)
					[animation_type].animation_index;

			if (animation_index!=NONE)
			{
				object_start_interpolation(
					unit_index,
					interpolation_frame_count);
				animation_graph_index =
					unit_definition->object.animation_graph.index;
				animation_index =
					animation_choose_random_permutation_internal(
						TRUE,
						animation_graph_index,
						animation_index);
				unit_set_animation(
					unit_index,
					unit_definition->object.animation_graph.index,
					animation_index);
				SET_FLAG(
					unit->unit.animation.flags,
					_unit_animation_postpone_weapon_ik_until_interpolation_ends_bit,
					TRUE);
				unit->unit.animation.state = _unit_state_ai_impulse;

				if (alignment_vector!=NULL &&
					unit->object.type==_object_type_biped &&
					unit->object.parent_object_index==NONE)
				{
					unit_align_facing(unit_index, alignment_vector);
				}
				if (unit->unit.player_index == NONE)
					network_actors_note_impulse(unit_index, animation_impulse, alignment_vector);

				result = TRUE;
			}
		}
	}

	return result;
}
static void unit_melee_sound(
	long unit_index,
	short material_type,
	long damage_effect_index)
{
	struct material_definition *material_definition =
		scenario_material_definition_get(material_type);

	if (material_definition->melee_hit_sound.index!=NONE)
	{
		object_impulse_sound_new(
			unit_index,
			material_definition->melee_hit_sound.index,
			NONE,
			global_origin3d,
			global_forward3d,
			1.f);
	}

	if (damage_effect_index!=NONE)
	{
		struct damage_effect_definition *damage_effect_definition =
			damage_effect_definition_get(damage_effect_index);

		if (damage_effect_definition->sound.index!=NONE)
		{
			object_impulse_sound_new(
				unit_index,
				damage_effect_definition->sound.index,
				NONE,
				global_origin3d,
				global_forward3d,
				1.f);
		}
	}

	return;
}
void unit_cause_melee_damage(
	long unit_index,
	boolean melee_hit,
	long target_object_index,
	short node_index,
	short region_index,
	short material_index,
	real_vector3d const *object_normal)
{
	struct unit_datum *unit = unit_get(unit_index);
	struct unit_definition *unit_definition =
		unit_definition_get(unit->definition_index);

	if (unit_definition->unit.melee_damage.index!=NONE)
	{
		struct object_marker marker;
		real_point3d melee_position;
		long damage_effect_index;

		if (object_get_marker_by_name(unit_index, "melee", &marker, 1)==1)
		{
			struct collision_result collision;
			real_vector3d collision_vector;
			real_point3d const *collision_origin;

			melee_position = marker.matrix.position;
			match_assert(
				"c:\\halo\\SOURCE\\units\\units.c",
				8640,
				global_current_collision_user_depth < MAXIMUM_COLLISION_USER_STACK_DEPTH);

			global_current_collision_users[global_current_collision_user_depth++] = 7;
			collision_origin = &unit->object.bounding_sphere_center;
			vector_from_points3d(collision_origin, &melee_position, &collision_vector);
			if (collision_test_vector(
				_collision_test_for_projectiles_flags,
				collision_origin,
				&collision_vector,
				NONE,
				&collision))
			{
				melee_position = *collision_origin;
			}

			match_assert(
				"c:\\halo\\SOURCE\\units\\units.c",
				8648,
				global_current_collision_user_depth > 1);
			--global_current_collision_user_depth;
		}
		else
		{
			melee_position = unit->object.bounding_sphere_center;
		}

		damage_effect_index = unit_definition->unit.melee_damage.index;
		{
			long weapon_index = unit_inventory_get_weapon(
				unit_index,
				unit_get(unit_index)->unit.current_weapon_index);

			if (weapon_index!=NONE)
			{
				struct weapon_definition *weapon_definition =
					weapon_definition_get(weapon_get(weapon_index)->definition_index);

				if (TEST_FLAG(
					weapon_definition->weapon.flags,
					_weapon_non_players_use_melee_damage_bit))
				{
					damage_effect_index =
						weapon_definition->weapon.melee_attack_damage.index;
				}
			}
		}

		{
			struct damage_data damage_data;

			damage_data_new(&damage_data, damage_effect_index);
			damage_data.location = unit->object.location;
			damage_data.owner_object_index = unit_index;
			damage_data.owner_team_index = unit->object.owner_team_index;
			damage_data.owner_player_index = unit->unit.player_index;
			damage_data.origin = melee_position;
			damage_data.epicenter = unit->object.bounding_sphere_center;

			if (target_object_index==NONE)
			{
				area_of_effect_cause_damage(&damage_data, NONE);
			}
			else
			{
				object_cause_damage(
					&damage_data,
					target_object_index,
					node_index,
					region_index,
					material_index,
					object_normal);
			}

			if (!melee_hit && damage_data.material_type!=NONE)
			{
				long hit_material_type = damage_data.material_type;

				unit_melee_sound(
					unit_index,
					hit_material_type,
					damage_effect_index);
			}
		}
	}

	unit->unit.melee_attack_state = 0;

	return;
}

/* insert verbatim into source/units/units.c immediately after the closing
   brace of unit_cause_melee_damage (line 8403) and before
   'static void unit_align_facing(' */
enum
{
	_collision_result_breakable_surface_bit = 3,
};

/* the globals' first multiplayer weapon, NONE where there is none: a
campaign map's globals list no multiplayer weapons (and its player starts
some levels unarmed, a10's), so its unarmed blow stays the game's, none */
static long unarmed_melee_weapon_definition_index(
	void)
{
	struct game_globals *game_globals = scenario_get_game_globals();

	return game_globals && game_globals->weapon_list.count>0 ?
		list_index_to_weapon_definition_index(0) : NONE;
}

/* port: the melee damage of a unit with no weapon (a gametype's loadout of
none): its own, else (a player's biped has none: players always had a
weapon) the blow of the globals' first multiplayer weapon, the assault
rifle's. network_damage.c takes it as the player's. */
long unit_unarmed_melee_damage(
	long unit_index)
{
	struct unit_definition *unit_definition = unit_definition_get(unit_get(unit_index)->definition_index);
	long weapon_definition_index;

	if (unit_definition->unit.melee_damage.index!=NONE)
		return unit_definition->unit.melee_damage.index;
	weapon_definition_index = unarmed_melee_weapon_definition_index();
	return weapon_definition_index!=NONE ?
		weapon_definition_get(weapon_definition_index)->weapon.melee_attack_damage.index : NONE;
}

void unit_cause_player_melee_damage(
	long unit_index)
{
	struct unit_datum *unit;
	struct unit_definition *unit_definition;
	real_vector3d *facing;
	long best_object_index;
	short hit_material_type;
	short breakable_surface_index;
	long breakable_surface_seed_surface_index;
	long melee_response_effect_index;
	real_point3d ray_origin;
	short best_object_type;
	real best_object_fraction;
	long outer_index;
	long outer_count;

	unit = unit_get(unit_index);
	unit_definition = unit_definition_get(unit->definition_index);
	best_object_index = NONE;
	hit_material_type = NONE;
	breakable_surface_index = NONE;
	melee_response_effect_index = NONE;

	{
		struct object_marker marker;

		object_get_marker_by_name(unit_index, "head", &marker, 1);
		ray_origin = marker.matrix.position;
	}

	match_assert(
		"c:\\halo\\SOURCE\\units\\units.c",
		8729,
		global_current_collision_user_depth < MAXIMUM_COLLISION_USER_STACK_DEPTH);
	global_current_collision_users[global_current_collision_user_depth++] = 8;
	facing = &unit->unit.aiming_vector;

	{
		real_vector3d perpendicular;
		real_vector3d cross;
		struct collision_result collision;
		real_vector3d ray;

		normalize3d(perpendicular3d(facing, &perpendicular));
		cross_product3d(facing, &perpendicular, &cross);

		outer_index = -2;
		outer_count = 5;
		do
		{
			long inner_index = -2;
			long inner_count = 5;

			do
			{
				double inner_component_i = cross.i * (real)inner_index;

				ray.i = facing->i * 0.8f +
					((real)inner_component_i + (real)outer_index * perpendicular.i) * 0.1f;
				inner_component_i = cross.j * (real)inner_index;
				ray.j = unit->unit.aiming_vector.j * 0.8f +
					((real)inner_component_i + (real)outer_index * perpendicular.j) * 0.1f;
				ray.k = unit->unit.aiming_vector.k * 0.8f +
					((real)outer_index * perpendicular.k + (real)inner_index * cross.k) * 0.1f;

				if (collision_test_vector(
					_collision_test_for_projectiles_flags,
					&ray_origin,
					&ray,
					unit_index,
					&collision))
				{
					switch (collision.type)
					{
					case _collision_result_structure:
						if (best_object_index==NONE)
						{
							hit_material_type = collision.material_type;
							if (TEST_FLAG(
								collision.flags,
								_collision_result_breakable_surface_bit))
							{
								breakable_surface_index =
									collision.breakable_surface_index;
								breakable_surface_seed_surface_index =
									collision.surface_index;
							}
						}
						break;

					case _collision_result_object:
					{
						long hit_object_index = collision.object_index;
						struct object_datum *hit_object = object_get(hit_object_index);

						/* port: (an Infection zombie's skull hits a rider in an open
						seat itself: infection.c) */
						if (hit_object->object.type!=_object_type_weapon &&
							hit_object->object.parent_object_index!=NONE &&
							!infection_melee_rider_target(unit_index, hit_object_index))
						{
							hit_object_index = hit_object->object.parent_object_index;
							hit_object = object_get(hit_object_index);
						}

						if (best_object_index==NONE ||
							(hit_object->object.type==_object_type_biped &&
							best_object_type==_object_type_biped &&
							best_object_fraction>collision.t) ||
							(hit_object->object.type==_object_type_biped &&
							best_object_type!=_object_type_biped))
						{
							best_object_index = hit_object_index;
							best_object_type = hit_object->object.type;
							hit_material_type = collision.material_type;
							best_object_fraction = collision.t;
						}
					}
						break;
					}
				}

				++inner_index;
				--inner_count;
			}
			while (inner_count!=0);

			++outer_index;
			--outer_count;
		}
		while (outer_count!=0);
	}

	match_assert(
		"c:\\halo\\SOURCE\\units\\units.c",
		8797,
		global_current_collision_user_depth > 1);
	--global_current_collision_user_depth;

	{
		short index = unit_get(unit_index)->unit.current_weapon_index;
		struct unit_datum *current_unit;
		long melee_damage_effect_index = NONE;

		current_unit = unit_get(unit_index);
		if (index!=NONE)
		{
			long weapon_index;

			match_assert(
				"c:\\halo\\SOURCE\\units\\units.c",
				8371,
				index>=0 && index<MAXIMUM_WEAPONS_PER_UNIT);
			weapon_index = current_unit->unit.weapon_object_indices[index];
			if (weapon_index!=NONE)
			{
				struct weapon_definition *weapon_definition =
					weapon_definition_get(weapon_get(weapon_index)->definition_index);

				melee_damage_effect_index =
					weapon_definition->weapon.melee_attack_damage.index;
				melee_response_effect_index =
					weapon_definition->weapon.melee_attack_response.index;
			}
		}

		if (melee_damage_effect_index==NONE)
		{
			melee_damage_effect_index = unit_definition->unit.melee_damage.index;
		}
		/* port: a player with no weapon (a gametype's loadout of none), whose
		biped has no blow of its own: unit_unarmed_melee_damage's, and its
		response */
		if (melee_damage_effect_index==NONE)
		{
			long weapon_definition_index = unarmed_melee_weapon_definition_index();

			melee_damage_effect_index = unit_unarmed_melee_damage(unit_index);
			if (weapon_definition_index!=NONE && melee_response_effect_index==NONE)
			{
				melee_response_effect_index =
					weapon_definition_get(weapon_definition_index)->weapon.melee_attack_response.index;
			}
		}

		if (best_object_index!=NONE)
		{
			struct object_datum *best_object = object_get(best_object_index);

			if (best_object->object.type==_object_type_vehicle)
			{
				struct object_definition *best_object_definition =
					object_definition_get(best_object->definition_index);
				real_vector3d acceleration;
				real acceleration_scale =
					best_object_definition->object.acceleration_scale * 0.035f;

				acceleration.i = acceleration_scale * unit->unit.aiming_vector.i;
				acceleration.j = acceleration_scale * unit->unit.aiming_vector.j;
				acceleration.k = acceleration_scale * unit->unit.aiming_vector.k;
				vehicle_accelerate(best_object_index, &acceleration);
			}
		}

		if (melee_damage_effect_index!=NONE)
		{
			struct damage_data damage_data;

			damage_data_new(&damage_data, melee_damage_effect_index);
			/* port: an Infection zombie's skull's (a client's report the host
			works out again: network_damage.c) */
			damage_data.multiplier = infection_melee_multiplier(unit_index);
			SET_FLAG(damage_data.flags, _damage_area_of_effect_bit, TRUE);
			damage_data.location = unit->object.location;
			damage_data.owner_object_index = unit_index;
			damage_data.owner_team_index = unit->object.owner_team_index;
			damage_data.owner_player_index = unit->unit.player_index;
			damage_data.origin = ray_origin;
			damage_data.epicenter = unit->object.bounding_sphere_center;
			damage_data.direction = unit->unit.aiming_vector;
			damage_data.material_type = hit_material_type;

			if (best_object_index==NONE)
			{
				if (breakable_surface_index!=NONE)
				{
					breakable_surface_damage(
						breakable_surface_index,
						&damage_data,
						breakable_surface_seed_surface_index);
				}
			}
			else
			{
				struct object_datum *best_object = object_get(best_object_index);
				struct game_globals_player_information *player_information;

				if (best_object->object.type==_object_type_machine)
				{
					machine_try_to_open_with_damage(best_object_index);
				}

				player_information = TAG_BLOCK_GET_ELEMENT(
					&scenario_get_game_globals()->player_information,
					0,
					struct game_globals_player_information);
				if (player_information->run_forward_speed>0.f)
				{
					damage_data.scale =
						dot_product3d(
							&unit->object.translational_velocity,
							&unit->object.forward) * 30.f /
						player_information->run_forward_speed;
					if (damage_data.scale<0.f)
					{
						damage_data.scale = 0.f;
					}
					else if (damage_data.scale>1.f)
					{
						damage_data.scale = 1.f;
					}
				}

				if (unit->object.type==_object_type_biped &&
					biped_get(unit_index)->biped.airborne_ticks>15)
				{
					damage_data.scale = 1.5f;
				}

				if (object_get(best_object_index)->object.type==_object_type_biped)
				{
					object_cause_damage(
						&damage_data,
						best_object_index,
						NONE,
						NONE,
						NONE,
						NULL);
				}
			}
		}

		if (hit_material_type!=NONE)
		{
			unit_melee_sound(
				unit_index,
				hit_material_type,
				melee_damage_effect_index);
			if (melee_response_effect_index!=NONE)
			{
				struct damage_data damage_data;

				damage_data_new(&damage_data, melee_response_effect_index);
				damage_data.direction.i = -unit->unit.aiming_vector.i;
				damage_data.epicenter = unit->object.bounding_sphere_center;
				damage_data.direction.j = -unit->unit.aiming_vector.j;
				damage_data.direction.k = -unit->unit.aiming_vector.k;
				SET_FLAG(
					damage_data.flags,
					_damage_from_weapon_bit,
					TRUE);
				damage_data.origin = unit->object.bounding_sphere_center;
				object_cause_damage(
					&damage_data,
					unit_index,
					NONE,
					NONE,
					NONE,
					NULL);
			}
		}
	}

	unit->unit.melee_attack_state = 0;

	return;
}

static void unit_align_facing(
	long unit_index,
	real_vector2d const *alignment_vector)
{
	struct unit_datum *unit = unit_get(unit_index);

	if (unit->object.parent_object_index==NONE)
	{
		match_vassert(
			"c:\\halo\\SOURCE\\units\\units.c",
			9353,
			valid_real_normal2d(alignment_vector),
			csprintf(
				temporary,
				"%s: assert_valid_real_normal2d(%f, %f)",
				"alignment_vector",
				alignment_vector->i,
				alignment_vector->j));

		set_real_vector3d(
			&unit->object.forward,
			alignment_vector->i,
			alignment_vector->j,
			0.f);
		unit->object.up = *global_up3d;

		match_assert_valid_real_vector3d_axes2(
			"c:\\halo\\SOURCE\\units\\units.c",
			9357,
			&unit->object.forward,
			&unit->object.up);
	}

	return;
}
void unit_start_flaming_to_death(
	long unit_index,
	long attacker_object_index)
{
	struct unit_datum *unit = unit_get(unit_index);

	unit_drop_current_weapon(unit_index, TRUE);

	SET_FLAG(unit->unit.flags, _unit_ignore_hard_pings_bit, TRUE);
	SET_FLAG(unit->object.damage_flags, _object_dead_bit, FALSE);
	SET_FLAG(unit->object.damage_flags, _object_cannot_take_damage_bit, TRUE);

	if (unit->unit.flaming_death_delay==0)
	{
		short random_delay =
			seed_random_range(get_global_random_seed_address(), 60, 150);
		long delay;

		if (random_delay<1)
		{
			delay = 1;
		}
		else if (random_delay>255)
		{
			delay = 255;
		}
		else
		{
			delay = random_delay;
		}

		unit->unit.flaming_death_delay = (byte)delay;
		unit->unit.flaming_death_attacker_object_index = attacker_object_index;

		unit_start_running_blindly(unit_index);
	}

	return;
}

static void unit_euler_axis_buildplan(
	real position,
	real velocity,
	real maximum_velocity,
	real maximum_acceleration,
	struct unit_acceleration_plan *plan)
{
	boolean moving_positive;
	real acceleration_time;
	real coasting_velocity;
	real discriminant;
	real first_root;
	real half_time;
	real half_velocity;
	real maximum_velocity_time;
	real negative_acceleration;
	real second_root;
	real second_time;
	real stopping_position;
	real time;

	plan->acceleration = REAL_MAX;
	plan->acceleration_time = REAL_MAX;
	plan->coast_time = REAL_MAX;
	plan->deceleration = REAL_MAX;
	plan->deceleration_time = REAL_MAX;
	plan->initial_position = position;
	plan->initial_velocity = velocity;
	plan->at_rest = fabs(position)<0.001f && fabs(velocity)<0.001f;

	if (plan->at_rest)
	{
		plan->acceleration = 0.f;
		plan->acceleration_time = 0.f;
		plan->coast_time = 0.f;
		plan->deceleration = 0.f;
		plan->deceleration_time = 0.f;
		goto plan_complete;
	}

	time = fabs(velocity) / maximum_acceleration;
	moving_positive = velocity>0.f;
	half_time = time*0.5f;
	if (half_time*velocity*0.5f + position<0.f)
	{
		unit_euler_axis_buildplan(
			-position,
			-velocity,
			maximum_velocity,
			maximum_acceleration,
			plan);
		plan->initial_position *= -1.f;
		plan->initial_velocity *= -1.f;
		plan->acceleration *= -1.f;
		plan->deceleration *= -1.f;
		goto validate_plan;
	}

	half_velocity = velocity*0.5f;
	stopping_position = half_velocity*time + position;
	if (stopping_position<0.f)
	{
		match_vassert(
			"c:\\halo\\SOURCE\\units\\units.c",
			0x7b7,
			plan->initial_position > -0.001f,
			"plan->initial_p > -1e-03f");
		match_vassert(
			"c:\\halo\\SOURCE\\units\\units.c",
			0x7b8,
			plan->initial_velocity < 0.f,
			"plan->initial_v < 0");
		plan->acceleration = 0.f;
		plan->acceleration_time = 0.f;
		plan->deceleration =
			plan->initial_velocity*plan->initial_velocity /
			(plan->initial_position + plan->initial_position);
		plan->deceleration_time =
			-(plan->initial_velocity / plan->deceleration);
		plan->coast_time = 0.f;
		goto validate_plan;
	}

	if (moving_positive)
	{
		acceleration_time = square_root(stopping_position / maximum_acceleration);
	}
	else
	{
		negative_acceleration = -maximum_acceleration;
		velocity += velocity;
		position =
			velocity*velocity - negative_acceleration*stopping_position*4.f;
		match_vassert(
			"c:\\halo\\SOURCE\\units\\units.c",
			0x7eb,
			position >= 0.f,
			"disc >= 0");
		second_root = square_root(position);
		first_root =
			(-velocity - second_root) /
			(negative_acceleration + negative_acceleration);
		second_time =
			(second_root - velocity) /
			(negative_acceleration + negative_acceleration);
		if (first_root>=0.f &&
			(second_time<0.f || first_root<second_time))
		{
			acceleration_time = first_root;
		}
		else if (0.f>second_time)
		{
			acceleration_time = 0.f;
			goto apply_velocity_limit;
		}
		else
		{
			acceleration_time = second_time;
		}
	}

	match_vassert(
		"c:\\halo\\SOURCE\\units\\units.c",
		0x7fa,
		acceleration_time >= 0.f,
		"t >= 0");

apply_velocity_limit:
	if (maximum_velocity>0.f)
	{
		maximum_velocity_time =
			(moving_positive ?
				maximum_velocity :
				maximum_velocity + plan->initial_velocity) /
			maximum_acceleration;
		maximum_velocity_time = MAX(0.f, maximum_velocity_time);
		first_root = MIN(acceleration_time, maximum_velocity_time);
	}
	else
	{
		first_root = acceleration_time;
	}

	plan->acceleration = -maximum_acceleration;
	plan->deceleration = maximum_acceleration;
	if (moving_positive)
	{
		plan->acceleration_time = first_root + time;
		plan->deceleration_time = first_root;
	}
	else
	{
		plan->acceleration_time = first_root;
		plan->deceleration_time = first_root + time;
	}

	if (first_root<acceleration_time)
	{
		coasting_velocity =
			plan->acceleration*plan->acceleration_time +
			plan->initial_velocity;
		discriminant =
			((acceleration_time - first_root)*coasting_velocity)*2.f -
			(acceleration_time - first_root)*
			(acceleration_time - first_root)*maximum_acceleration;
		match_vassert(
			"c:\\halo\\SOURCE\\units\\units.c",
			0x850,
			coasting_velocity < 0.f,
			"coasting_vel < 0");
		plan->coast_time = discriminant / coasting_velocity;
		match_vassert(
			"c:\\halo\\SOURCE\\units\\units.c",
			0x852,
			plan->coast_time >= 0.f,
			"plan->coast_t >= 0");
		match_vassert(
			"c:\\halo\\SOURCE\\units\\units.c",
			0x853,
			plan->coast_time + first_root >= acceleration_time,
			"plan->coast_t + actual_t >= t");
	}
	else
	{
		plan->coast_time = 0.f;
	}

validate_plan:
	match_vassert(
		"c:\\halo\\SOURCE\\units\\units.c",
		0x85c,
		REAL_MAX != plan->acceleration,
		"REAL_MAX != plan->accel_a");
	match_vassert(
		"c:\\halo\\SOURCE\\units\\units.c",
		0x85d,
		REAL_MAX != plan->acceleration_time,
		"REAL_MAX != plan->accel_t");
	match_vassert(
		"c:\\halo\\SOURCE\\units\\units.c",
		0x85e,
		REAL_MAX != plan->coast_time,
		"REAL_MAX != plan->coast_t");
	match_vassert(
		"c:\\halo\\SOURCE\\units\\units.c",
		0x85f,
		REAL_MAX != plan->deceleration,
		"REAL_MAX != plan->decel_a");
	match_vassert(
		"c:\\halo\\SOURCE\\units\\units.c",
		0x860,
		REAL_MAX != plan->deceleration_time,
		"REAL_MAX != plan->decel_t");

plan_complete:
	return;
}
static void unit_euler_axis_couple(
	struct unit_acceleration_plan *first_plan,
	struct unit_acceleration_plan *second_plan,
	real maximum_velocity,
	real maximum_acceleration)
{
	real acceleration_time;
	real adjustment;
	real discriminant;
	real first_total_time;
	real peak_velocity;
	real second_total_time;
	real time_extension;
	real time_factor;
	struct unit_acceleration_plan *adjust_plan = NULL;

	if (first_plan->at_rest || second_plan->at_rest)
	{
		goto overlap_complete;
	}

	first_total_time =
		first_plan->deceleration_time +
		first_plan->coast_time +
		first_plan->acceleration_time;
	second_total_time =
		second_plan->deceleration_time +
		second_plan->coast_time +
		second_plan->acceleration_time;
	if (first_plan->acceleration_time>0.f &&
		first_total_time<second_total_time)
	{
		time_extension = second_total_time - first_total_time;
		adjust_plan = first_plan;
	}
	else if (second_plan->acceleration_time>0.f &&
		second_total_time<first_total_time)
	{
		time_extension = first_total_time - second_total_time;
		adjust_plan = second_plan;
	}

	if (!adjust_plan)
	{
		goto overlap_complete;
	}

	match_vassert(
		"c:\\halo\\SOURCE\\units\\units.c",
		0x8ae,
		time_extension > 0.f,
		"t_extension > 0");
	time_factor =
		(time_extension + adjust_plan->coast_time) *
		maximum_acceleration;
	peak_velocity =
		adjust_plan->acceleration_time*adjust_plan->acceleration +
		adjust_plan->initial_velocity;
	discriminant =
		time_factor*time_factor -
		(real)((-time_extension)*fabs(peak_velocity)) *
		maximum_acceleration*4.f;
	match_vassert(
		"c:\\halo\\SOURCE\\units\\units.c",
		0x8c4,
		discriminant >= 0.f,
		"disc >= 0");
	adjustment =
		(square_root(discriminant) - time_factor) /
		(maximum_acceleration + maximum_acceleration);
	adjustment = MIN(
		adjustment,
		MIN(adjust_plan->acceleration_time, adjust_plan->deceleration_time));
	if (!(adjustment>0.f))
	{
		goto overlap_complete;
	}

	acceleration_time = adjust_plan->acceleration_time - adjustment;
	peak_velocity =
		acceleration_time*adjust_plan->acceleration +
		adjust_plan->initial_velocity;
	adjust_plan->acceleration_time = acceleration_time;
	adjust_plan->deceleration_time -= adjustment;
	adjust_plan->coast_time =
		((peak_velocity + peak_velocity +
			adjustment*adjust_plan->acceleration)*adjustment) /
		peak_velocity;
	match_vassert(
		"c:\\halo\\SOURCE\\units\\units.c",
		0x8d8,
		(acceleration_time >= 0.f) &&
		(adjust_plan->deceleration_time >= 0.f),
		"(adjust_plan->accel_t >= 0) && (adjust_plan->decel_t >= 0)");

overlap_complete:
	return;
}
void unit_euler_aiming_update(
	real_matrix4x3 const *orientation,
	real_vector3d *aiming_vector,
	real_vector3d const *desired_aiming_vector,
	real_vector3d *aiming_velocity,
	real_rectangle2d const *aiming_bounds,
	real angular_velocity_limit,
	real angular_acceleration_limit)
{
	boolean aiming_yaw_in_bounds;
	boolean pitch_complete;
	boolean yaw_complete;
	boolean yaw_wraps;
	real angle;
	real dot_product;
	real_euler_angles2d aiming_error;
	real_euler_angles2d aiming_angular_velocity;
	real_euler_angles2d aiming_angles;
	real_euler_angles2d end_aiming_angles;
	real_euler_angles2d end_angular_velocity;
	real_euler_angles2d desired_aiming_angles;
	real_vector3d clamped_desired_aiming_vector;
	real_vector3d local_aiming_vector;
	real_vector3d local_desired_aiming_vector;
	real_vector3d end_aiming_vector;
	struct unit_acceleration_plan pitch_plan;
	struct unit_acceleration_plan yaw_plan;

	match_assert(
		"c:\\halo\\SOURCE\\units\\units.c",
		2402,
		angular_velocity_limit >= 0.0f);
	match_assert(
		"c:\\halo\\SOURCE\\units\\units.c",
		2403,
		(angular_acceleration_limit > 0.0f) &&
		(angular_acceleration_limit < 10000.0f));

	if (orientation)
	{
		matrix4x3_inverse_transform_normal(
			orientation,
			aiming_vector,
			&local_aiming_vector);
		matrix4x3_inverse_transform_normal(
			orientation,
			desired_aiming_vector,
			&local_desired_aiming_vector);
	}
	else
	{
		local_aiming_vector = *aiming_vector;
		local_desired_aiming_vector = *desired_aiming_vector;
	}

	yaw_wraps =
		(aiming_bounds->x1 - aiming_bounds->x0) - 2.f * _pi >
		-_real_epsilon;
	euler_angles2d_from_vector3d(&aiming_angles, &local_aiming_vector);
	euler_angles2d_from_vector3d(
		&desired_aiming_angles,
		&local_desired_aiming_vector);

	aiming_yaw_in_bounds = TRUE;
	if (yaw_wraps)
	{
		if (desired_aiming_angles.yaw < aiming_bounds->x0)
		{
			desired_aiming_angles.yaw += 2.f * _pi;
			match_assert(
				"c:\\halo\\SOURCE\\units\\units.c",
				2433,
				desired_aiming_angles.yaw <= aiming_bounds->x1);
		}
		else if (desired_aiming_angles.yaw > aiming_bounds->x1)
		{
			desired_aiming_angles.yaw -= 2.f * _pi;
			match_assert(
				"c:\\halo\\SOURCE\\units\\units.c",
				2438,
				desired_aiming_angles.yaw >= aiming_bounds->x0);
		}
	}
	else if (desired_aiming_angles.yaw < aiming_bounds->x0)
	{
		desired_aiming_angles.yaw = aiming_bounds->x0;
		aiming_yaw_in_bounds = FALSE;
	}
	else if (desired_aiming_angles.yaw > aiming_bounds->x1)
	{
		desired_aiming_angles.yaw = aiming_bounds->x1;
		aiming_yaw_in_bounds = FALSE;
	}

	if (desired_aiming_angles.pitch < aiming_bounds->y0)
	{
		desired_aiming_angles.pitch = aiming_bounds->y0;
	}
	else if (desired_aiming_angles.pitch > aiming_bounds->y1)
	{
		desired_aiming_angles.pitch = aiming_bounds->y1;
	}
	else if (aiming_yaw_in_bounds)
	{
		clamped_desired_aiming_vector = *desired_aiming_vector;
		goto desired_aiming_vector_ready;
	}

	vector3d_from_euler_angles2d(
		&clamped_desired_aiming_vector,
		&desired_aiming_angles);
	if (orientation)
	{
		matrix4x3_transform_normal(
			orientation,
			&clamped_desired_aiming_vector,
			&clamped_desired_aiming_vector);
		normalize3d(&clamped_desired_aiming_vector);
	}

desired_aiming_vector_ready:
	{
		real angular_speed;
		real_vector3d velocity_axis;

		velocity_axis = *aiming_velocity;
		angular_speed = normalize3d(&velocity_axis);
		if (angular_speed != 0.f)
		{
			real_euler_angles2d rotated_angles;
			real_vector3d rotated_aiming_vector;

			rotated_aiming_vector = local_aiming_vector;
			rotate_vector_about_axis(
				&rotated_aiming_vector,
				&velocity_axis,
				sine(angular_speed),
				cosine(angular_speed));
			euler_angles2d_from_vector3d(&rotated_angles, &rotated_aiming_vector);
			aiming_angular_velocity.yaw = rotated_angles.yaw - aiming_angles.yaw;
			aiming_angular_velocity.pitch = rotated_angles.pitch - aiming_angles.pitch;
		}
		else
		{
			aiming_angular_velocity.yaw = 0.f;
			aiming_angular_velocity.pitch = 0.f;
		}
	}

	aiming_error.yaw = aiming_angles.yaw - desired_aiming_angles.yaw;
	aiming_error.pitch = aiming_angles.pitch - desired_aiming_angles.pitch;
	if (yaw_wraps)
	{
		if (aiming_error.yaw > _pi)
		{
			aiming_error.yaw -= 2.f * _pi;
		}
		else if (aiming_error.yaw < -_pi)
		{
			aiming_error.yaw += 2.f * _pi;
		}
	}

	unit_euler_axis_buildplan(
		aiming_error.yaw,
		aiming_angular_velocity.yaw,
		angular_velocity_limit,
		angular_acceleration_limit,
		&yaw_plan);
	unit_euler_axis_buildplan(
		aiming_error.pitch,
		aiming_angular_velocity.pitch,
		angular_velocity_limit,
		angular_acceleration_limit,
		&pitch_plan);
	unit_euler_axis_couple(
		&yaw_plan,
		&pitch_plan,
		angular_velocity_limit,
		angular_acceleration_limit);

	yaw_complete = unit_euler_axis_doplan(
		&yaw_plan,
		1.f,
		aiming_error.yaw,
		&aiming_error.yaw,
		aiming_angular_velocity.yaw,
		&end_angular_velocity.yaw);
	pitch_complete = unit_euler_axis_doplan(
		&pitch_plan,
		1.f,
		aiming_error.pitch,
		&aiming_error.pitch,
		aiming_angular_velocity.pitch,
		&end_angular_velocity.pitch);

	if (yaw_complete && pitch_complete)
	{
		*aiming_vector = clamped_desired_aiming_vector;
		*aiming_velocity = *global_zero_vector3d;
	}
	else
	{
		end_aiming_angles.yaw = aiming_error.yaw + desired_aiming_angles.yaw;
		end_aiming_angles.pitch = aiming_error.pitch + desired_aiming_angles.pitch;

		if (yaw_wraps)
		{
			if (end_aiming_angles.yaw < aiming_bounds->x0)
			{
				end_aiming_angles.yaw += 2.f * _pi;
				match_assert(
					"c:\\halo\\SOURCE\\units\\units.c",
					2725,
					end_aiming_angles.yaw <= aiming_bounds->x1);
			}
			else if (end_aiming_angles.yaw > aiming_bounds->x1)
			{
				end_aiming_angles.yaw -= 2.f * _pi;
				match_assert(
					"c:\\halo\\SOURCE\\units\\units.c",
					2730,
					end_aiming_angles.yaw >= aiming_bounds->x0);
			}
		}
		else
		{
			end_aiming_angles.yaw = PIN(
				end_aiming_angles.yaw,
				aiming_bounds->x0,
				aiming_bounds->x1);
		}

		end_aiming_angles.pitch = PIN(
			end_aiming_angles.pitch,
			aiming_bounds->y0,
			aiming_bounds->y1);
		match_assert(
			"c:\\halo\\SOURCE\\units\\units.c",
			2739,
			(end_aiming_angles.yaw >= aiming_bounds->x0) &&
			(end_aiming_angles.yaw <= aiming_bounds->x1));
		match_assert(
			"c:\\halo\\SOURCE\\units\\units.c",
			2740,
			(end_aiming_angles.pitch >= aiming_bounds->y0) &&
			(end_aiming_angles.pitch <= aiming_bounds->y1));

		vector3d_from_euler_angles2d(&end_aiming_vector, &end_aiming_angles);
		{
			real_euler_angles2d further_angles;
			real_vector3d further_vector;

			further_angles.yaw =
				end_angular_velocity.yaw + end_aiming_angles.yaw;
			further_angles.pitch =
				end_angular_velocity.pitch + end_aiming_angles.pitch;
			vector3d_from_euler_angles2d(&further_vector, &further_angles);

			dot_product = dot_product3d(&end_aiming_vector, &further_vector);
			dot_product = PIN(dot_product, -1.f, 1.f);
			cross_product3d(
				&end_aiming_vector,
				&further_vector,
				aiming_velocity);
		}
		normalize3d(aiming_velocity);
		angle = MIN(arccosine(dot_product), angular_velocity_limit);
		scale_vector3d(aiming_velocity, angle, aiming_velocity);

		if (orientation)
		{
			matrix4x3_transform_normal(
				orientation,
				&end_aiming_vector,
				aiming_vector);
			normalize3d(aiming_vector);
		}
		else
		{
			*aiming_vector = end_aiming_vector;
		}
	}

	match_assert_valid_real_normal3d(
		"c:\\halo\\SOURCE\\units\\units.c",
		2793,
		aiming_vector);

	return;
}
static boolean unit_animation_aiming_screen(
	struct unit_animation *animation)
{
	boolean result = !animation->action;

	if (animation->state>=_unit_state_hard_ping)
	{
		if (animation->state<=_unit_state_resurrect_back ||
			animation->state==_unit_state_leap_melee)
		{
			result = FALSE;
		}
	}

	return result;
}
static boolean unit_animation_state_can_be_entered_without_animation(
	short state)
{
	boolean result = TRUE;

	switch (state)
	{
	case _unit_state_melee_attack:
	case _unit_state_melee_airborne:
	case _unit_state_melee_continuous:
	case _unit_state_throw_grenade:
	case _unit_state_leap_start:
	case _unit_state_leap_melee:
		result = FALSE;
		break;
	default:
		break;
	}

	return result;
}
static short unit_animation_compute_interpolation_frame_count(
	short new_state,
	short old_state)
{
	short result = 6;

	if ((new_state==_unit_state_idle ||
		new_state==_unit_state_turn_left ||
		new_state==_unit_state_turn_right) &&
		(old_state==_unit_state_idle ||
		old_state==_unit_state_turn_left ||
		old_state==_unit_state_turn_right))
	{
		result = 1;
	}

	if (new_state==_unit_state_land_hard ||
		new_state==_unit_state_land_soft)
	{
		result = 2;
	}

	return result;
}
static long unit_animation_state_get_aiming_screen_index(
	short state)
{
	long result = NONE;

	switch (state)
	{
	case _unit_state_idle:
	case _unit_state_turn_left:
	case _unit_state_turn_right:
	case _unit_state_flying_front:
	case _unit_state_flying_back:
	case _unit_state_flying_left:
	case _unit_state_flying_right:
	case _unit_state_airborne:
	case _unit_state_land_soft:
	case _unit_state_land_hard:
	case _unit_state_opening:
	case _unit_state_closing:
		result = _unit_weapon_class_animation_aiming_still;
		break;
	case _unit_state_move_front:
	case _unit_state_move_back:
	case _unit_state_move_left:
	case _unit_state_move_right:
	case _unit_state_stunned_move_front:
	case _unit_state_stunned_move_back:
	case _unit_state_stunned_move_left:
	case _unit_state_stunned_move_right:
	case _unit_state_slide_front:
	case _unit_state_slide_back:
	case _unit_state_slide_left:
	case _unit_state_slide_right:
		result = _unit_weapon_class_animation_aiming_moving;
		break;
	default:
		break;
	}

	return result;
}
static void unit_set_animation(
	long unit_index,
	long animation_graph_index,
	short animation_index)
{
	struct unit_datum *unit = unit_get(unit_index);

	unit->object.animation.animation_graph_index = animation_graph_index;
	unit->object.animation.state.index = animation_index;
	unit->object.animation.state.frame_index = 0;

	if (debug_unit_all_animations)
	{
		char const *animation_name = "<none>";

		if (animation_graph_index != NONE)
		{
			struct animation_graph *animation_graph =
				animation_graph_definition_get(animation_graph_index);

			if (animation_index != NONE)
			{
				struct animation *animation = TAG_BLOCK_GET_ELEMENT(
					&animation_graph->animations,
					animation_index,
					struct animation);
				animation_name = animation->name;
			}
		}

		if (ai_debug.selected_actor_index == NONE ||
			unit->unit.actor_index == ai_debug.selected_actor_index ||
			unit->unit.swarm_actor_index == ai_debug.selected_actor_index)
		{
			console_printf(
				FALSE,
				"%s: animation %s",
				tag_name_strip_path(tag_get_name(unit->definition_index)),
				animation_name);
		}
	}

	return;
}

static short unit_animation_update(
	long unit_index,
	long animation_graph_index,
	struct animation_state *animation)
{
	long sound_index;
	short result = animation_update_internal(
		1,
		animation_graph_index,
		animation,
		&sound_index);

	if (sound_index!=NONE)
	{
		object_impulse_sound_new(
			unit_index,
			sound_index,
			0,
			global_origin3d,
			global_forward3d,
			1.f);
	}

	return result;
}

static char const *unit_get_current_weapon_label(
	long unit_index)
{
	long weapon_index = unit_inventory_get_weapon(
		unit_index,
		unit_get(unit_index)->unit.current_weapon_index);
	char const *weapon_label;

	if (weapon_index==NONE)
	{
		weapon_label = "unarmed";
	}
	else
	{
		weapon_label = weapon_get_label(weapon_index);
	}

	return weapon_label;
}

short unit_update_animation(
	long unit_index,
	struct unit_animation_update_data *data)
{
	struct unit_datum *unit;
	struct unit_definition *unit_definition;
	short desired_state;
	word result;
	boolean apply_state;
	short animation_update_result;
	boolean can_change_animation;
	struct biped_datum *biped;
	struct biped_definition *biped_definition;
	struct model *model;
	struct animation_graph *animation_graph;
	struct animation *animation;
	struct unit_datum *parent_unit;
	struct unit_definition *parent_definition;
	struct unit_seat *seat;
	real_vector3d exit_velocity;
	real_matrix4x3 old_world_matrix;

	unit = unit_get(unit_index);
	unit_definition = unit_definition_get(unit->definition_index);
	desired_state = data->state_desired;
	result = 0;
	apply_state = FALSE;

	match_assert(
		"c:\\halo\\SOURCE\\units\\units.c",
		0xb61,
		desired_state>=0 && desired_state<NUMBER_OF_UNIT_STATES);

	if (unit->object.parent_object_index==NONE &&
		!TEST_FLAG(
			unit->object.damage_flags,
			_object_dead_bit))
	{
		short desired_base_seat_index = NONE;

		switch (unit->unit.animation.desired_state)
		{
		case _unit_animation_state_asleep:
			desired_base_seat_index = _unit_base_seat_asleep;
			break;
		case _unit_animation_state_alert:
			desired_base_seat_index = _unit_base_seat_alert;
			break;
		case _unit_animation_state_suspicious:
			desired_base_seat_index = _unit_base_seat_alert;
			break;
		case _unit_animation_state_in_combat:
			desired_base_seat_index =
				(data->crouching != FALSE) + _unit_base_seat_stand;
			break;
		case _unit_animation_state_wary:
			desired_base_seat_index = _unit_base_seat_stand;
			break;
		case _unit_animation_state_flee:
			desired_base_seat_index = _unit_base_seat_flee;
			break;
		case _unit_animation_state_flaming:
			desired_base_seat_index = _unit_base_seat_flaming;
			break;
		}

		match_assert(
			"c:\\halo\\SOURCE\\units\\units.c",
			0xb73,
			desired_base_seat_index!=NONE);

		if (unit->unit.player_index!=NONE &&
			magic_base_animation_seat_index!=NONE)
		{
			if (magic_base_animation_seat_index<0)
			{
				desired_base_seat_index = _unit_base_seat_asleep;
			}
			else
			{
				desired_base_seat_index = MIN(
					magic_base_animation_seat_index,
					NUMBER_OF_UNIT_ANIMATION_STATES-1);
			}
		}

		if (unit->unit.magic_seat_index!=NONE)
		{
			desired_base_seat_index = unit->unit.magic_seat_index;
		}
		if (TEST_FLAG(
			unit->unit.control_flags,
			_unit_control_force_alert_bit))
		{
			desired_base_seat_index = _unit_base_seat_alert;
		}
		if (unit->unit.flaming_death_delay>0)
		{
			desired_base_seat_index = _unit_base_seat_flaming;
		}

		if (unit->unit.animation.base_seat_index!=desired_base_seat_index)
		{
			can_change_animation = unit_animation_state_interruptable(
				&unit->unit.animation,
				desired_state);
			if (can_change_animation)
			{
				unit_set_or_test_seat_and_weapon_label(
					unit_index,
					base_seat_label_get(desired_base_seat_index),
					unit_get_current_weapon_label(unit_index),
					TRUE);
			}
		}
	}

	if (unit->unit.animation.soft_ping_animation.index!=NONE)
	{
		animation_update_result = unit_animation_update(
			unit_index,
			unit_definition->object.animation_graph.index,
			&unit->unit.animation.soft_ping_animation);
		if (animation_update_result==2)
		{
			unit->unit.animation.soft_ping_animation.index = NONE;
		}
	}

	if (unit->object.animation.state.index!=NONE)
	{
		animation_update_result = unit_animation_update(
			unit_index,
			unit->object.animation.animation_graph_index,
			&unit->object.animation.state);

		if (animation_update_result==1)
		{
			switch (unit->unit.animation.state)
			{
			case _unit_state_throw_grenade:
				unit_throw_grenade_release(unit_index, FALSE);
				break;
			case _unit_state_melee_attack:
			case _unit_state_melee_airborne:
			case _unit_state_leap_melee:
				unit_cause_melee_damage(
					unit_index,
					FALSE,
					NONE,
					NONE,
					NONE,
					NONE,
					FALSE);
				break;
			default:
				break;
			}
		}
		else if (animation_update_result==2)
		{
			switch (unit->unit.animation.state)
			{
			case _unit_state_leap_start:
				result = TRUE;
				desired_state = _unit_state_leap_airborne;
				break;

			case _unit_state_dying:
				if (TEST_FLAG(
						unit_definition->unit.flags,
						_unit_is_destroyed_after_dying_bit))
				{
					if (TEST_FLAG(unit->object.flags, _object_at_rest_bit))
					{
						goto destroy_unit;
					}
					if (unit->object.type!=_object_type_biped)
					{
						goto finish_dying;
					}

					biped = biped_get(unit_index);
					biped_definition =
						biped_definition_get(biped->definition_index);
					if (!TEST_FLAG(biped->biped.flags, _biped_airborne_bit) ||
						TEST_FLAG(
							biped_definition->biped.flags,
							_biped_has_no_dying_airborne_bit))
					{
	destroy_unit:
						unit_destroy(unit_index);
						goto finish_dying_animation;
					}
				}

				if (unit->object.type==_object_type_biped)
				{
					biped_start_limp_body_physics(unit_index);
				}

	finish_dying:
				SET_FLAG(
					unit->unit.animation.flags,
					_unit_animation_ignore_translation_bit,
					TRUE);
				--unit->object.animation.state.frame_index;
	finish_dying_animation:
				break;

			case _unit_state_exiting_seat:
				model = model_definition_get(
					unit_definition->object.model.index);
				animation_graph = animation_graph_definition_get(
					unit->object.animation.animation_graph_index);
				animation = TAG_BLOCK_GET_ELEMENT(
					&animation_graph->animations,
					unit->object.animation.state.index,
					struct animation);

				animation_get_root_velocity(
					model,
					animation,
					unit->object.animation.state.frame_index,
					&exit_velocity);
				matrix4x3_transform_vector(
					object_get_world_matrix(unit_index, &old_world_matrix),
					&exit_velocity,
					&exit_velocity);
				unit_exit_seat_end(unit_index);
				add_vectors3d(
					&unit->object.translational_velocity,
					&exit_velocity,
					&unit->object.translational_velocity);
				break;

			case _unit_state_opening:
			case _unit_state_closing:
				--unit->object.animation.state.frame_index;
				break;

			case _unit_state_entering_seat:
				parent_unit = unit_get(unit->object.parent_object_index);
				parent_definition = unit_definition_get(parent_unit->definition_index);
				seat = TAG_BLOCK_GET_ELEMENT(
					&parent_definition->unit.seats,
					unit->unit.parent_seat_index,
					struct unit_seat);

				object_set_visibility(
					unit_index,
					!TEST_FLAG(seat->flags, _unit_seat_invisible_bit));
				if (parent_unit->unit.driver_object_index==unit_index)
				{
					unit_close(unit->object.parent_object_index);
				}
				break;

			default:
				break;
			}

			if (!unit_animation_state_loops(&unit->unit.animation))
			{
				apply_state = TRUE;
			}
		}
	}

	if (unit->unit.animation.action_animation.index!=NONE)
	{
		animation_update_result = unit_animation_update(
			unit_index,
			unit_definition->object.animation_graph.index,
			&unit->unit.animation.action_animation);
		if (animation_update_result==2)
		{
			struct unit_datum *interpolated_unit;

			object_start_interpolation(unit_index, 6);
			interpolated_unit = unit_get(unit_index);
			interpolated_unit->unit.animation.action = FALSE;
			interpolated_unit->unit.animation.action_animation.index = NONE;
		}
	}

	if (unit->unit.animation.overlay_action_animation.index!=NONE)
	{
		animation_update_result = unit_animation_update(
			unit_index,
			unit_definition->object.animation_graph.index,
			&unit->unit.animation.overlay_action_animation);

		switch (animation_update_result)
		{
		case 2:
		case 4:
			if (!unit_animation_overlay_action_loops(&unit->unit.animation))
			{
				unit->unit.animation.overlay_action = FALSE;
				unit->unit.animation.overlay_action_animation.index = NONE;
			}
			break;
		default:
			break;
		}
	}

	if (apply_state ||
		(desired_state!=unit->unit.animation.state &&
		unit_animation_state_interruptable(&unit->unit.animation, desired_state)))
	{
		unit_animation_set_state(unit_index, desired_state);
	}

	return result;
}

static boolean unit_animation_set_state(
	long unit_index,
	short new_state)
{
	struct unit_datum *unit = unit_get(unit_index);
	struct unit_definition *unit_definition =
		unit_definition_get(unit->definition_index);
	struct animation_graph *animation_graph =
		animation_graph_definition_get(
			unit_definition->object.animation_graph.index);
	struct animation_graph_unit_seat *unit_seat =
		TAG_BLOCK_GET_ELEMENT(
			&animation_graph->unit_seats,
			unit->unit.animation.seat_index,
			struct animation_graph_unit_seat);
	struct animation_graph_weapon_class *weapon_class =
		TAG_BLOCK_GET_ELEMENT(
			&unit_seat->weapon_classes,
			unit->unit.animation.weapon_index,
			struct animation_graph_weapon_class);
	long animation_graph_index;
	short interpolation_frame_count;
	boolean old_state_is_none;
	boolean changed_state;
	boolean result;

	TAG_BLOCK_GET_ELEMENT(
		&weapon_class->weapon_types,
		unit->unit.animation.weapon_type_index,
		struct animation_graph_weapon_type);

	old_state_is_none = unit->unit.animation.state == NONE;
	changed_state = FALSE;
	result = TRUE;

	if (old_state_is_none ||
		new_state != unit->unit.animation.state)
	{
		short animation_index;
		short weapon_class_animation_index = NONE;
		short unit_seat_animation_index = NONE;

		if (unit->unit.animation.state ==
			_unit_state_throw_grenade)
		{
			unit_throw_grenade_release(unit_index, TRUE);
		}

		switch (new_state)
		{
		case _unit_state_idle:
			weapon_class_animation_index = _unit_weapon_class_animation_idle;
			break;
		case _unit_state_gesture:
			weapon_class_animation_index = _unit_weapon_class_animation_gesture;
			break;
		case _unit_state_turn_left:
			weapon_class_animation_index = _unit_weapon_class_animation_turning_left;
			break;
		case _unit_state_turn_right:
			weapon_class_animation_index = _unit_weapon_class_animation_turning_right;
			break;
		case _unit_state_move_front:
			weapon_class_animation_index = _unit_weapon_class_animation_moving_front;
			break;
		case _unit_state_move_back:
			weapon_class_animation_index = _unit_weapon_class_animation_moving_back;
			break;
		case _unit_state_move_left:
			weapon_class_animation_index = _unit_weapon_class_animation_moving_left;
			break;
		case _unit_state_move_right:
			weapon_class_animation_index = _unit_weapon_class_animation_moving_right;
			break;
		case _unit_state_stunned_move_front:
			weapon_class_animation_index = _unit_weapon_class_animation_moving_wounded_front;
			break;
		case _unit_state_stunned_move_back:
			weapon_class_animation_index = _unit_weapon_class_animation_moving_wounded_back;
			break;
		case _unit_state_stunned_move_left:
			weapon_class_animation_index = _unit_weapon_class_animation_moving_wounded_left;
			break;
		case _unit_state_stunned_move_right:
			weapon_class_animation_index = _unit_weapon_class_animation_moving_wounded_right;
			break;
		case _unit_state_slide_front:
			weapon_class_animation_index = _unit_weapon_class_animation_sliding_front;
			break;
		case _unit_state_slide_back:
			weapon_class_animation_index = _unit_weapon_class_animation_sliding_back;
			break;
		case _unit_state_slide_left:
			weapon_class_animation_index = _unit_weapon_class_animation_sliding_left;
			break;
		case _unit_state_slide_right:
			weapon_class_animation_index = _unit_weapon_class_animation_sliding_right;
			break;
		case _unit_state_airborne:
			weapon_class_animation_index = _unit_weapon_class_animation_airborne;
			break;
		case _unit_state_land_soft:
			weapon_class_animation_index = _unit_weapon_class_animation_land_soft;
			break;
		case _unit_state_land_hard:
			weapon_class_animation_index = _unit_weapon_class_animation_land_hard;
			break;
		case _unit_state_throw_grenade:
			weapon_class_animation_index = _unit_weapon_class_animation_throw_grenade;
			break;
		case _unit_state_melee_attack:
			weapon_class_animation_index = _unit_weapon_class_animation_melee_attack;
			break;
		case _unit_state_melee_airborne:
			weapon_class_animation_index = _unit_weapon_class_animation_melee_airborne;
			break;
		case _unit_state_melee_continuous:
			weapon_class_animation_index = _unit_weapon_class_animation_melee_continuous;
			break;
		case _unit_state_resurrect_front:
			weapon_class_animation_index = _unit_weapon_class_animation_resurrect_front;
			break;
		case _unit_state_resurrect_back:
			weapon_class_animation_index = _unit_weapon_class_animation_resurrect_back;
			break;
		case _unit_state_feeding:
			weapon_class_animation_index = _unit_weapon_class_animation_feeding;
			break;
		case _unit_state_leap_start:
			weapon_class_animation_index = _unit_weapon_class_animation_leap_start;
			break;
		case _unit_state_leap_airborne:
			weapon_class_animation_index = _unit_weapon_class_animation_leap_airborne;
			break;
		case _unit_state_leap_melee:
			weapon_class_animation_index = _unit_weapon_class_animation_leap_melee;
			break;
		case _unit_state_flying_front:
			unit_seat_animation_index = _unit_seat_animation_flying_front;
			break;
		case _unit_state_flying_back:
			unit_seat_animation_index = _unit_seat_animation_flying_back;
			break;
		case _unit_state_flying_left:
			unit_seat_animation_index = _unit_seat_animation_flying_left;
			break;
		case _unit_state_flying_right:
			unit_seat_animation_index = _unit_seat_animation_flying_right;
			break;
		case _unit_state_dying_airborne:
			unit_seat_animation_index = _unit_seat_animation_airborne_dead;
			break;
		case _unit_state_dying:
			unit_seat_animation_index = _unit_seat_animation_landing_dead;
			break;
		case _unit_state_opening:
			unit_seat_animation_index = _unit_seat_animation_opening;
			break;
		case _unit_state_closing:
			unit_seat_animation_index = _unit_seat_animation_closing;
			break;
		case _unit_state_hovering:
			unit_seat_animation_index = _unit_seat_animation_hovering;
			break;
		default:
			break;
		}

		// Get weapon class animation
		if (weapon_class_animation_index!=NONE)
		{
			if (weapon_class_animation_index>=0 && weapon_class_animation_index<weapon_class->animations.count)
			{
				animation_index = animation_graph_animation_index_get(&weapon_class->animations)[weapon_class_animation_index].animation_index;
			}
			else
			{
				animation_index = NONE;
			}
		}
		// Get unit seat animation
		else
		{
			if (unit_seat_animation_index!=NONE)
			{
				if (unit_seat_animation_index>=0 && unit_seat_animation_index<unit_seat->animations.count)
				{
					animation_index = animation_graph_animation_index_get(&unit_seat->animations)[unit_seat_animation_index].animation_index;
				}
				else
				{
					animation_index = NONE;
				}
			}
			else
			{
				animation_index = NONE;
			}
		}

		if (debug_unit_animations &&
			unit->object.type == _object_type_biped &&
			animation_index == NONE)
		{
			char const *weapon_animation_name;
			char const *seat_or_weapon_class_name;

			if (weapon_class_animation_index != NONE)
			{
				weapon_animation_name = animation_list_get_string(
						&weapon_class_animation_list,
						weapon_class_animation_index);
			}
			else
			{
				weapon_animation_name = "";
			}

			if (unit_seat_animation_index != NONE)
			{
				seat_or_weapon_class_name = animation_list_get_string(
						&unit_seat_animation_list,
						unit_seat_animation_index);
			}
			else
			{
				seat_or_weapon_class_name = weapon_class->label;
			}

			console_warning(
				"MISSING: %s '%s %s %s'",
				tag_name_strip_path(
					unit_definition->object.animation_graph.name),
				unit_seat->label,
				seat_or_weapon_class_name,
				weapon_animation_name);
		}

		if (animation_index == NONE)
		{
			if (!unit_animation_state_can_be_entered_without_animation(new_state))
			{
				result = FALSE;

				goto done;
			}
		}

		{
			animation_graph_index =
				unit_definition->object.animation_graph.index;
			animation_index =
				animation_choose_random_permutation_internal(
					TRUE,
					animation_graph_index,
					animation_index);
			unit_set_animation(
				unit_index,
				unit_definition->object.animation_graph.index,
				animation_index);

			interpolation_frame_count =
				unit_animation_compute_interpolation_frame_count(
					new_state,
					unit->unit.animation.state);

			changed_state = TRUE;
		}
	}

	{
		short aiming_screen_animation_index =
			(short)unit_animation_state_get_aiming_screen_index(new_state);

		if (old_state_is_none ||
			aiming_screen_animation_index !=
				(short)unit_animation_state_get_aiming_screen_index(
					unit->unit.animation.state))
		{
			long animation_index;

			if (aiming_screen_animation_index >= 0 &&
				aiming_screen_animation_index <
					weapon_class->animations.count)
			{
				animation_index =
					animation_graph_animation_index_get(
						&weapon_class->animations)
						[aiming_screen_animation_index].animation_index;
			}
			else
			{
				animation_index = NONE;
			}

			animation_graph_index =
				unit_definition->object.animation_graph.index;
			unit->unit.animation.aiming_screen_index =
				animation_choose_random_permutation_internal(
					TRUE,
					animation_graph_index,
					animation_index);

			if (debug_unit_animations &&
				unit->object.type == _object_type_biped &&
				unit->unit.animation.aiming_screen_index == NONE &&
				aiming_screen_animation_index != NONE)
			{
				console_warning(
					"MISSING: %s '%s %s %s'",
					tag_name_strip_path(
						unit_definition->object.animation_graph.name),
					unit_seat->label,
					weapon_class->label,
					animation_list_get_string(
						&weapon_class_animation_list,
						aiming_screen_animation_index));
			}

			interpolation_frame_count = 6;
			changed_state = TRUE;

			if (old_state_is_none)
			{
			long animation_index;

			if (_unit_seat_animation_looking >= 0 &&
				_unit_seat_animation_looking <
					unit_seat->animations.count)
			{
				animation_index =
					animation_graph_animation_index_get(
						&unit_seat->animations)
						[_unit_seat_animation_looking].animation_index;
			}
			else
			{
				animation_index = NONE;
			}

			animation_graph_index =
				unit_definition->object.animation_graph.index;
			unit->unit.animation.looking_screen_index =
				animation_choose_random_permutation_internal(
					TRUE,
					animation_graph_index,
					animation_index);

			if (debug_unit_animations &&
				unit->object.type == _object_type_biped &&
				unit->unit.animation.looking_screen_index == NONE)
			{
				console_warning(
					"MISSING: %s '%s %s'",
					tag_name_strip_path(
						unit_definition->object.animation_graph.name),
					unit_seat->label,
					animation_list_get_string(
						&unit_seat_animation_list,
						_unit_seat_animation_looking));
			}
			}
		}

		if (changed_state)
		{
			object_start_interpolation(
				unit_index,
				interpolation_frame_count);
		}

		unit->unit.animation.state = (char)new_state;
	}

done:
	return result;
}

/* port: a unit throwing a grenade on the move keeps its lower body (the
pelvis and the legs) in the movement animation; the throw keeps the spine
and everything above it. The throw is a base animation, which sets every
node, so the legs of the original game stop while the unit slides. */
static boolean unit_node_is_at_or_below(
	struct model *model,
	short node_index,
	short ancestor_node_index)
{
	while (node_index != NONE)
	{
		if (node_index == ancestor_node_index)
		{
			return TRUE;
		}
		node_index = TAG_BLOCK_GET_ELEMENT(&model->nodes, node_index, struct model_node)->parent_node_index;
	}

	return FALSE;
}

/* the speed, in world units per second, at and above which the legs move
fully; below it they blend toward the rest pose */
#define GRENADE_THROW_FULL_MOVE_SPEED 0.75f
/* the ticks the legs take to blend fully in or out, as the engine's own
transitions between animation states */
#define GRENADE_THROW_BLEND_TICKS 6
#define GRENADE_THROW_MAXIMUM_TRACKED_UNITS 2048

static real grenade_throw_leg_weights[GRENADE_THROW_MAXIMUM_TRACKED_UNITS];
static long grenade_throw_leg_weight_times[GRENADE_THROW_MAXIMUM_TRACKED_UNITS];
static real grenade_throw_air_weights[GRENADE_THROW_MAXIMUM_TRACKED_UNITS];
static long grenade_throw_air_weight_times[GRENADE_THROW_MAXIMUM_TRACKED_UNITS];
static long grenade_throw_ik_release_times[GRENADE_THROW_MAXIMUM_TRACKED_UNITS];

/* a weight eased toward its target by a step a tick; a new throw starts
at the target */
static real unit_grenade_throw_ease(
	real *weights,
	long *times,
	long unit_index,
	real target)
{
	long absolute_index = DATUM_INDEX_TO_ABSOLUTE_INDEX(unit_index);
	long now = game_time_get();
	real weight = target;

	if (absolute_index >= 0 && absolute_index < GRENADE_THROW_MAXIMUM_TRACKED_UNITS)
	{
		if (times[absolute_index] == now)
		{
			weight = weights[absolute_index];
		}
		else
		{
			if (times[absolute_index] == now - 1)
			{
				real step = 1.f / GRENADE_THROW_BLEND_TICKS;

				weight = weights[absolute_index] + PIN(target - weights[absolute_index], -step, step);
			}
			weights[absolute_index] = weight;
			times[absolute_index] = now;
		}
	}

	return weight;
}

/* port: a rider's hands leave the vehicle's grips while throwing a grenade
or playing an action (a reload, readying or putting away a weapon, a
melee), and through the interpolation back to the seat after it, which the
grips would otherwise snap */
static boolean unit_grenade_throw_releases_vehicle_ik(
	long unit_index)
{
	struct unit_datum *unit = unit_get(unit_index);
	long absolute_index = DATUM_INDEX_TO_ABSOLUTE_INDEX(unit_index);
	long now = game_time_get();
	long elapsed;
	boolean releasing =
		unit->unit.animation.state == _unit_state_throw_grenade ||
		unit->unit.animation.action != 0;

	if (!unit_animation_enhanced(unit_index))
	{
		return FALSE;
	}
	if (absolute_index < 0 || absolute_index >= GRENADE_THROW_MAXIMUM_TRACKED_UNITS)
	{
		return releasing;
	}
	if (releasing)
	{
		grenade_throw_ik_release_times[absolute_index] = now + 1;
		return TRUE;
	}
	if (grenade_throw_ik_release_times[absolute_index] == 0)
	{
		return FALSE;
	}
	elapsed = now + 1 - grenade_throw_ik_release_times[absolute_index];

	return elapsed >= 0 && elapsed <= GRENADE_THROW_BLEND_TICKS;
}

/* one of the weapon class's base animations, NULL where the graph has none
that fits the model */
static struct animation *unit_weapon_class_base_animation(
	struct model *model,
	struct animation_graph *animation_graph,
	struct animation_graph_weapon_class *weapon_class,
	short slot)
{
	short animation_index;
	struct animation *animation;

	if (slot < 0 || slot >= weapon_class->animations.count)
	{
		return NULL;
	}
	animation_index = animation_graph_animation_index_get(&weapon_class->animations)[slot].animation_index;
	if (animation_index == NONE)
	{
		return NULL;
	}

	animation = TAG_BLOCK_GET_ELEMENT(&animation_graph->animations, animation_index, struct animation);
	if (animation->type != _animation_base ||
		animation->frame_count < 1 ||
		animation->node_count != model->nodes.count)
	{
		return NULL;
	}

	return animation;
}

/* port: the nodes a unit's orientations hold: its model's, no more than a
model can have (object_new sizes them from the model) */
short unit_animation_model_node_count(
	long unit_index)
{
	struct object_datum *object = object_get(unit_index);
	struct object_definition *object_definition = object_definition_get(object->definition_index);
	short model_node_count = 0;

	if (object_definition->object.model.index!=NONE)
	{
		model_node_count = (short)PIN(
			model_definition_get(object_definition->object.model.index)->nodes.count,
			0, MAXIMUM_NODES_PER_MODEL);
	}

	return model_node_count;
}

/* port: the graph's animation, or NULL for one the graph doesn't have or
one with more nodes than the model, which is all the orientations hold (a
map's index and animation; the apply wrote past them). In the released
maps every unit and vehicle animation has its model's node count */
struct animation *unit_animation_get_fitting(
	struct animation_graph *animation_graph,
	short animation_index,
	short model_node_count)
{
	static boolean reported = FALSE;
	struct animation *animation = NULL;

	if (VALID_INDEX(animation_index, animation_graph->animations.count))
	{
		animation = TAG_BLOCK_GET_ELEMENT(
			&animation_graph->animations,
			animation_index,
			struct animation);

		if (animation->node_count>model_node_count)
		{
			if (!reported)
			{
				reported = TRUE;
				error(_error_silent, "### ERROR animation '%s' has %d nodes, more than its model's %d; it is skipped",
					animation->name, animation->node_count, model_node_count);
			}
			animation = NULL;
		}
	}

	return animation;
}

/* the animation's pose at a point (0 to 1) through its cycle */
static void unit_animation_pose_at_phase(
	struct model *model,
	struct animation *animation,
	real phase,
	struct real_orientation *orientations)
{
	long frame_index = (long)(phase * animation->frame_count);

	frame_index = PIN(frame_index, 0, animation->frame_count - 1);
	animation_get_node_orientations(model, animation, (short)frame_index, orientations);

	return;
}

static void unit_node_orientations_blend(
	short node_count,
	struct real_orientation const *from,
	struct real_orientation const *to,
	real fraction,
	struct real_orientation *result)
{
	short node_index;

	for (node_index = 0; node_index < node_count; node_index++)
	{
		quaternions_interpolate_and_normalize(
			&from[node_index].rotation,
			&to[node_index].rotation,
			fraction,
			&result[node_index].rotation);
		points_interpolate(
			&from[node_index].translation,
			&to[node_index].translation,
			fraction,
			&result[node_index].translation);
		scalars_interpolate(
			from[node_index].scale,
			to[node_index].scale,
			fraction,
			&result[node_index].scale);
	}

	return;
}

static void unit_grenade_throw_keep_legs_moving(
	long unit_index,
	struct real_orientation *node_orientations)
{
	struct unit_datum *unit = unit_get(unit_index);
	struct unit_definition *unit_definition = unit_definition_get(unit->definition_index);
	struct animation_graph *animation_graph;
	struct animation_graph_unit_seat *unit_seat;
	struct animation_graph_weapon_class *weapon_class;
	struct model *model;
	struct animation *idle_animation;
	struct animation *air_animation;
	struct animation *front_animation;
	struct animation *forward_animation;
	struct animation *side_animation;
	struct real_orientation rest_orientations[MAXIMUM_NODES_PER_MODEL];
	struct real_orientation forward_orientations[MAXIMUM_NODES_PER_MODEL];
	struct real_orientation side_orientations[MAXIMUM_NODES_PER_MODEL];
	struct real_orientation move_orientations[MAXIMUM_NODES_PER_MODEL];
	struct real_orientation legs_orientations[MAXIMUM_NODES_PER_MODEL];
	struct real_orientation air_orientations[MAXIMUM_NODES_PER_MODEL];
	struct real_orientation *legs;
	real_vector3d left;
	real forward_speed;
	real left_speed;
	real target_weight;
	real move_weight;
	real air_weight;
	real side_weight;
	real phase;
	long now;
	boolean airborne;
	boolean crouching;
	boolean moving;
	short spine;
	short node_index;

	if (unit->unit.animation.state != _unit_state_throw_grenade ||
		unit->unit.animation.seat_index == NONE ||
		!unit_animation_enhanced(unit_index))
	{
		return;
	}

	animation_graph = animation_graph_definition_get(unit_definition->object.animation_graph.index);
	unit_seat = TAG_BLOCK_GET_ELEMENT(&animation_graph->unit_seats, unit->unit.animation.seat_index, struct animation_graph_unit_seat);
	weapon_class = TAG_BLOCK_GET_ELEMENT(&unit_seat->weapon_classes, unit->unit.animation.weapon_index, struct animation_graph_weapon_class);
	model = model_definition_get(unit_definition->object.model.index);
	now = game_time_get();
	if (model->nodes.count > MAXIMUM_NODES_PER_MODEL)
	{
		return;
	}

	/* the throw keeps the spine and everything above it */
	spine = model_find_node(unit_definition->object.model.index, "bip01 spine");
	if (spine == NONE)
	{
		return;
	}

	/* port: a rider throwing from a vehicle's seat stays seated: the lower
	body keeps the seat's own idle pose, as the throw is the standing one */
	if (unit->object.parent_object_index != NONE)
	{
		struct animation *seated_animation = unit_weapon_class_base_animation(
			model,
			animation_graph,
			weapon_class,
			_unit_weapon_class_animation_idle);

		if (seated_animation)
		{
			unit_animation_pose_at_phase(
				model,
				seated_animation,
				(real)(now % seated_animation->frame_count) / (real)seated_animation->frame_count,
				rest_orientations);
			for (node_index = 0; node_index < model->nodes.count; node_index++)
			{
				if (!unit_node_is_at_or_below(model, node_index, spine))
				{
					node_orientations[node_index] = rest_orientations[node_index];
				}
			}
		}

		return;
	}

	/* in the air the legs float as in a jump, and do not run; both ease in
	and out over a few ticks */
	airborne = unit_flying_through_air(unit_index);
	air_weight = unit_grenade_throw_ease(
		grenade_throw_air_weights,
		grenade_throw_air_weight_times,
		unit_index,
		airborne ? 1.f : 0.f);

	/* the unit's real speed along its facing and to its left, per second */
	cross_product3d(&unit->object.up, &unit->object.forward, &left);
	forward_speed = dot_product3d(&unit->object.translational_velocity, &unit->object.forward) * TICKS_PER_SECOND;
	left_speed = dot_product3d(&unit->object.translational_velocity, &left) * TICKS_PER_SECOND;
	target_weight = square_root(forward_speed * forward_speed + left_speed * left_speed) / GRENADE_THROW_FULL_MOVE_SPEED;
	target_weight = airborne ? 0.f : PIN(target_weight, 0.f, 1.f);
	move_weight = unit_grenade_throw_ease(
		grenade_throw_leg_weights,
		grenade_throw_leg_weight_times,
		unit_index,
		target_weight);

	/* the rest pose: the crouching idle when crouched, else the throw's own legs */
	crouching = unit->unit.animation.base_seat_index == _unit_base_seat_crouch;
	idle_animation = crouching ?
		unit_weapon_class_base_animation(model, animation_graph, weapon_class, _unit_weapon_class_animation_idle) :
		NULL;
	if (!idle_animation && move_weight <= 0.f && air_weight <= 0.f)
	{
		return;
	}
	if (idle_animation)
	{
		unit_animation_pose_at_phase(
			model,
			idle_animation,
			(real)(now % idle_animation->frame_count) / (real)idle_animation->frame_count,
			rest_orientations);
	}
	else
	{
		for (node_index = 0; node_index < model->nodes.count; node_index++)
		{
			rest_orientations[node_index] = node_orientations[node_index];
		}
	}

	/* the movement pose: the forward or back walk blended with the left or
	right walk by how much of the movement is sideways, all at one point in
	the walk cycle so the feet stay in step */
	front_animation = unit_weapon_class_base_animation(model, animation_graph, weapon_class, _unit_weapon_class_animation_moving_front);
	phase = front_animation ?
		(real)(now % front_animation->frame_count) / (real)front_animation->frame_count :
		0.f;
	forward_animation = unit_weapon_class_base_animation(
		model,
		animation_graph,
		weapon_class,
		forward_speed >= 0.f ? _unit_weapon_class_animation_moving_front : _unit_weapon_class_animation_moving_back);
	side_animation = unit_weapon_class_base_animation(
		model,
		animation_graph,
		weapon_class,
		left_speed > 0.f ? _unit_weapon_class_animation_moving_left : _unit_weapon_class_animation_moving_right);
	side_weight = (real)(fabs(forward_speed) + fabs(left_speed));
	side_weight = side_weight > 0.f ? (real)fabs(left_speed) / side_weight : 0.f;

	moving = FALSE;
	if (move_weight > 0.f)
	{
		if (forward_animation && side_animation)
		{
			unit_animation_pose_at_phase(model, forward_animation, phase, forward_orientations);
			unit_animation_pose_at_phase(model, side_animation, phase, side_orientations);
			unit_node_orientations_blend(model->nodes.count, forward_orientations, side_orientations, side_weight, move_orientations);
			moving = TRUE;
		}
		else if (forward_animation)
		{
			unit_animation_pose_at_phase(model, forward_animation, phase, move_orientations);
			moving = TRUE;
		}
		else if (side_animation)
		{
			unit_animation_pose_at_phase(model, side_animation, phase, move_orientations);
			moving = TRUE;
		}
	}

	legs = rest_orientations;
	if (moving)
	{
		unit_node_orientations_blend(model->nodes.count, rest_orientations, move_orientations, move_weight, legs_orientations);
		legs = legs_orientations;
	}

	/* the floating legs of a jump over all of it */
	air_animation = air_weight > 0.f ?
		unit_weapon_class_base_animation(model, animation_graph, weapon_class, _unit_weapon_class_animation_airborne) :
		NULL;
	if (air_animation)
	{
		unit_animation_pose_at_phase(
			model,
			air_animation,
			(real)(now % air_animation->frame_count) / (real)air_animation->frame_count,
			air_orientations);
		unit_node_orientations_blend(model->nodes.count, legs, air_orientations, air_weight, forward_orientations);
		legs = forward_orientations;
	}

	for (node_index = 0; node_index < model->nodes.count; node_index++)
	{
		if (!unit_node_is_at_or_below(model, node_index, spine))
		{
			node_orientations[node_index] = legs[node_index];
		}
	}

	return;
}

void unit_preprocess_node_orientations(
	long unit_index,
	struct real_orientation *node_orientations)
{
	struct unit_datum *unit;
	struct animation_graph *animation_graph;
	struct animation_graph_unit_seat *unit_seat;
	real_matrix4x3 matrix;
	struct unit_definition *unit_definition;
	struct animation *animation;
	short model_node_count;

	unit = unit_get(unit_index);
	unit_definition = unit_definition_get(unit->definition_index);
	animation_graph = animation_graph_definition_get(
		unit_definition->object.animation_graph.index);
	/* port: every animation below is one the graph has and that fits the
	model (unit_animation_get_fitting) */
	model_node_count = unit_animation_model_node_count(unit_index);
	unit_grenade_throw_keep_legs_moving(unit_index, node_orientations);

	if (unit->unit.animation.action_animation.index != NONE &&
		(animation = unit_animation_get_fitting(animation_graph,
			unit->unit.animation.action_animation.index, model_node_count)) != NULL)
	{
		replacement_animation_apply(
			animation,
			unit->unit.animation.action_animation.frame_index,
			node_orientations);
	}

	if (unit->unit.animation.overlay_action_animation.index != NONE &&
		(animation = unit_animation_get_fitting(animation_graph,
			unit->unit.animation.overlay_action_animation.index, model_node_count)) != NULL)
	{
		overlay_animation_apply(
			animation,
			unit->unit.animation.overlay_action_animation.frame_index,
			node_orientations);
	}

	if (unit->unit.animation.soft_ping_animation.index != NONE &&
		(animation = unit_animation_get_fitting(animation_graph,
			unit->unit.animation.soft_ping_animation.index, model_node_count)) != NULL)
	{
		overlay_animation_apply(
			animation,
			unit->unit.animation.soft_ping_animation.frame_index,
			node_orientations);
	}

	unit_verify_vectors(unit_index, "unit-preprocess-nodes");
	unit->unit.animation.aiming_with_euler_screen = FALSE;
	unit->unit.animation.looking_with_euler_screen = FALSE;

	if (!TEST_FLAG(unit_definition->unit.flags, _unit_simple_creature_bit) &&
		unit->unit.animation.seat_index != NONE)
	{
		unit_seat = TAG_BLOCK_GET_ELEMENT(
			&animation_graph->unit_seats,
			unit->unit.animation.seat_index,
			struct animation_graph_unit_seat);

		if (unit->unit.animation.emotion_index != NONE)
		{
			short animation_index;

			if (_unit_seat_animation_emotions < unit_seat->animations.count)
			{
				animation_index = animation_graph_animation_index_get(
					&unit_seat->animations)[_unit_seat_animation_emotions].animation_index;
			}
			else
			{
				animation_index = NONE;
			}

			if (unit->unit.override_emotion_animation_index != NONE)
			{
				animation_index = unit->unit.override_emotion_animation_index;
			}

			if (animation_index != NONE &&
				(animation = unit_animation_get_fitting(animation_graph,
					animation_index, model_node_count)) != NULL)
			{
				if (unit->unit.animation.emotion_index >= 0 &&
					unit->unit.animation.emotion_index < animation->frame_count)
				{
					overlay_animation_apply(
						animation,
						unit->unit.animation.emotion_index,
						node_orientations);
				}
			}
		}

		if (unit->unit.mouth_aperture > 0.f &&
			_unit_seat_animation_mouth_aperture < unit_seat->animations.count)
		{
			short animation_index;

			animation_index = animation_graph_animation_index_get(
				&unit_seat->animations)[_unit_seat_animation_mouth_aperture].animation_index;

			if (animation_index != NONE &&
				(animation = unit_animation_get_fitting(animation_graph,
					animation_index, model_node_count)) != NULL)
			{
				overlay_animation_apply_scaled(
					animation,
					0,
					unit->unit.mouth_aperture,
					node_orientations);
			}
		}

		if (TEST_FLAG(
			unit->unit.animation.flags,
			_unit_animation_showing_acceleration_bit))
		{
			short animation_index;
			long acceleration_index;

			for (acceleration_index = 0;
				acceleration_index < _unit_seat_animation_push_impact -
					_unit_seat_animation_acceleration_front_back;
				acceleration_index++)
			{
				if (_unit_seat_animation_acceleration_front_back +
						acceleration_index >= 0 &&
					_unit_seat_animation_acceleration_front_back +
						acceleration_index < unit_seat->animations.count)
				{
					animation_index = animation_graph_animation_index_get(
						&unit_seat->animations)[
							_unit_seat_animation_acceleration_front_back +
							acceleration_index].animation_index;

					if (animation_index != NONE &&
						(animation = unit_animation_get_fitting(animation_graph,
							animation_index, model_node_count)) != NULL)
					{
						overlay_animation_apply_continuous(
							animation,
							(animation->frame_count - 1) *
								unit->unit.seat_acceleration.n[acceleration_index],
							node_orientations);
					}
				}
			}
		}

		if (!TEST_FLAG(unit_definition->unit.flags, _unit_has_no_aiming_bit) &&
			unit_animation_aiming_screen(&unit->unit.animation))
		{
			real_euler_angles2d relative_aiming_angles;
			struct animation_graph_weapon_class *weapon_class =
				TAG_BLOCK_GET_ELEMENT(
					&unit_seat->weapon_classes,
					unit->unit.animation.weapon_index,
					struct animation_graph_weapon_class);

			relative_aiming_angles = *global_zero_angles2d;

			if (unit->unit.animation.aiming_screen_index != NONE &&
				(animation = unit_animation_get_fitting(animation_graph,
					unit->unit.animation.aiming_screen_index, model_node_count)) != NULL)
			{
				real_vector3d relative_aiming_vector;
				struct animation_aiming_screen_bounds const *aiming_bounds =
					&weapon_class->aiming_screen_bounds;

				match_assert_valid_real_normal3d(
					"c:\\halo\\SOURCE\\units\\units.c",
					1648,
					&unit->unit.aiming_vector);

				matrix.scale = 1.f;
				object_get_orientation(
					unit_index,
					&matrix.forward,
					&matrix.up);
				cross_product3d(
					&matrix.up,
					&matrix.forward,
					&matrix.left);
				matrix.position = *global_origin3d;
				matrix4x3_inverse_transform_normal(
					&matrix,
					&unit->unit.aiming_vector,
					&relative_aiming_vector);

				match_assert_valid_real_vector3d(
					"c:\\halo\\SOURCE\\units\\units.c",
					1661,
					&relative_aiming_vector);

				euler_angles2d_from_vector3d(
					&relative_aiming_angles,
					&relative_aiming_vector);

				match_assert_valid_real(
					"c:\\halo\\SOURCE\\units\\units.c",
					1664,
					relative_aiming_angles.pitch);
				match_assert_valid_real(
					"c:\\halo\\SOURCE\\units\\units.c",
					1665,
					relative_aiming_angles.yaw);

				unit->unit.animation.aiming_with_euler_screen = TRUE;
				unit->unit.animation.aiming_screen_bounds.x0 =
					-(aiming_bounds->negative_yaw_frame_count *
					aiming_bounds->negative_yaw_delta);
				unit->unit.animation.aiming_screen_bounds.x1 =
					aiming_bounds->positive_yaw_frame_count *
					aiming_bounds->positive_yaw_delta;
				unit->unit.animation.aiming_screen_bounds.y0 =
					-(aiming_bounds->negative_pitch_frame_count *
					aiming_bounds->negative_pitch_delta);
				unit->unit.animation.aiming_screen_bounds.y1 =
					aiming_bounds->positive_pitch_frame_count *
					aiming_bounds->positive_pitch_delta;
				aiming_screen_apply(
					animation,
					aiming_bounds,
					relative_aiming_angles.yaw,
					relative_aiming_angles.pitch,
					node_orientations);
			}

			if (unit->unit.current_weapon_index != NONE ||
				unit->unit.player_index != NONE)
			{
				struct animation_aiming_screen_bounds const *looking_bounds =
					&unit_seat->looking_screen_bounds;

				if (unit->unit.animation.looking_screen_index != NONE &&
					(animation = unit_animation_get_fitting(animation_graph,
						unit->unit.animation.looking_screen_index, model_node_count)) != NULL)
				{
					real_vector3d relative_looking_vector;
					real_euler_angles2d relative_looking_angles;

					matrix.scale = 1.f;
					object_get_orientation(
						unit_index,
						&matrix.forward,
						&matrix.up);
					cross_product3d(
						&matrix.up,
						&matrix.forward,
						&matrix.left);
					matrix.position = *global_origin3d;
					matrix4x3_inverse_transform_normal(
						&matrix,
						&unit->unit.looking_vector,
						&relative_looking_vector);

					match_assert_valid_real_vector3d(
						"c:\\halo\\SOURCE\\units\\units.c",
						1703,
						&relative_looking_vector);

					euler_angles2d_from_vector3d(
						&relative_looking_angles,
						&relative_looking_vector);
					relative_looking_angles.yaw -= relative_aiming_angles.yaw;
					relative_looking_angles.pitch -= relative_aiming_angles.pitch;

					match_assert_valid_real(
						"c:\\halo\\SOURCE\\units\\units.c",
						1710,
						relative_looking_angles.pitch);
					match_assert_valid_real(
						"c:\\halo\\SOURCE\\units\\units.c",
						1711,
						relative_looking_angles.yaw);

					unit->unit.animation.looking_with_euler_screen = TRUE;
					unit->unit.animation.looking_screen_bounds.x0 =
						-(looking_bounds->negative_yaw_frame_count *
						looking_bounds->negative_yaw_delta);
					unit->unit.animation.looking_screen_bounds.x1 =
						looking_bounds->positive_yaw_frame_count *
						looking_bounds->positive_yaw_delta;
					unit->unit.animation.looking_screen_bounds.y0 =
						-(looking_bounds->negative_pitch_frame_count *
						looking_bounds->negative_pitch_delta);
					unit->unit.animation.looking_screen_bounds.y1 =
						looking_bounds->positive_pitch_frame_count *
						looking_bounds->positive_pitch_delta;
					aiming_screen_apply(
						animation,
						looking_bounds,
						relative_looking_angles.yaw,
						relative_looking_angles.pitch,
						node_orientations);
				}
			}
		}
	}

	return;
}

void unit_postprocess_node_matrices(
	long unit_index,
	real_matrix4x3 *node_matrices)
{
	struct unit_datum *unit = unit_get(unit_index);
	struct unit_definition *unit_definition =
		unit_definition_get(unit->definition_index);
	struct animation_graph *animation_graph;
	struct animation_graph_unit_seat *unit_seat;
	struct animation_graph_weapon_class *weapon_class;
	struct animation_graph_ik_point *ik_point;
	struct animation_graph_ik_point *weapon_ik_point;
	struct unit_animation *animation;
	struct tag_block *ik_points;
	struct tag_block *weapon_ik_points;
	short ik_point_index;
	long ik_element_index;
	boolean weapon_ik_active;

	if (!TEST_FLAG(unit_definition->unit.flags, _unit_simple_creature_bit) &&
		unit->unit.animation.seat_index!=NONE)
	{
		animation_graph = animation_graph_definition_get(
			unit_definition->object.animation_graph.index);
		unit_seat = TAG_BLOCK_GET_ELEMENT(
				&animation_graph->unit_seats,
				unit->unit.animation.seat_index,
				struct animation_graph_unit_seat);
		weapon_class = TAG_BLOCK_GET_ELEMENT(
				&unit_seat->weapon_classes,
				unit->unit.animation.weapon_index,
				struct animation_graph_weapon_class);

		if (unit->object.parent_object_index!=NONE &&
			unit_animation_vehicle_ik(&unit->unit.animation) &&
			!unit_grenade_throw_releases_vehicle_ik(unit_index))
		{
			ik_points = &unit_seat->ik_points;
			ik_point_index = 0;
			if (ik_points->count>0)
			{
				ik_element_index = 0;
				do
				{
					ik_point = TAG_BLOCK_GET_ELEMENT(
							ik_points,
							ik_element_index,
							struct animation_graph_ik_point);

					object_inverse_kinematics(
						unit_index,
						ik_point->marker_name,
						unit->object.parent_object_index,
						ik_point->attached_to_marker_name,
						node_matrices);

					++ik_point_index;
					ik_element_index = ik_point_index;
				}
				while (ik_element_index<ik_points->count);
			}
		}

		if (unit->unit.current_weapon_index!=NONE)
		{
			animation = &unit->unit.animation;
			weapon_ik_active = unit_animation_weapon_ik(animation);
			if (weapon_ik_active)
			{
				long weapon_ik_point_index = 0;
				weapon_ik_points = &weapon_class->ik_points;
				if (weapon_ik_points->count>0)
				{
					ik_element_index = 0;
					do
					{
						weapon_ik_point = TAG_BLOCK_GET_ELEMENT(
								weapon_ik_points,
								ik_element_index,
								struct animation_graph_ik_point);

						object_inverse_kinematics(
							unit_index,
							weapon_ik_point->marker_name,
							unit_get_current_weapon_index(unit_index),
							weapon_ik_point->attached_to_marker_name,
							node_matrices);

						++weapon_ik_point_index;
						ik_element_index = (short)weapon_ik_point_index;
					}
					while (ik_element_index<weapon_ik_points->count);
				}

				SET_FLAG(
					animation->flags,
					_unit_animation_postpone_weapon_ik_until_interpolation_ends_bit,
					FALSE);
			}
		}
	}

	return;
}

void unit_control(
	long unit_index,
	struct unit_control_data const *control_data)
{
	struct unit_datum *unit = unit_get(unit_index);

	if (unit->unit.player_index == NONE)
		network_actors_note_control(unit_index, control_data);

	match_assert(
		"c:\\halo\\SOURCE\\units\\units.c",
		0x5e1,
		magnitude3d(&control_data->throttle)<=3.0f);
	match_assert(
		"c:\\halo\\SOURCE\\units\\units.c",
		0x5e2,
		control_data->animation_state>=0 &&
		control_data->animation_state<NUMBER_OF_UNIT_ANIMATION_STATES);
	match_assert(
		"c:\\halo\\SOURCE\\units\\units.c",
		0x5e3,
		control_data->aiming_speed>=0 &&
		control_data->aiming_speed<NUMBER_OF_UNIT_AIMING_SPEEDS);
	match_assert(
		"c:\\halo\\SOURCE\\units\\units.c",
		0x5e4,
		VALID_FLAGS(control_data->control_flags, NUMBER_OF_UNIT_CONTROL_FLAGS));
	match_assert_valid_real_normal3d(
		"c:\\halo\\SOURCE\\units\\units.c",
		0x5e5,
		&control_data->facing_vector);
	match_assert_valid_real_normal3d(
		"c:\\halo\\SOURCE\\units\\units.c",
		0x5e6,
		&control_data->aiming_vector);
	match_assert_valid_real_normal3d(
		"c:\\halo\\SOURCE\\units\\units.c",
		0x5e7,
		&control_data->looking_vector);
	match_assert(
		"c:\\halo\\SOURCE\\units\\units.c",
		0x5e8,
		control_data->weapon_index==NONE ||
			(control_data->weapon_index>=0 &&
			control_data->weapon_index<MAXIMUM_WEAPONS_PER_UNIT));
	match_assert(
		"c:\\halo\\SOURCE\\units\\units.c",
		0x5e9,
		control_data->grenade_index==NONE ||
			(control_data->grenade_index>=0 &&
			control_data->grenade_index<NUMBER_OF_UNIT_GRENADE_TYPES));
	match_assert(
		"c:\\halo\\SOURCE\\units\\units.c",
		0x5ea,
		control_data->zoom_level==NONE || (control_data->zoom_level>=0));
	match_assert_valid_real(
		"c:\\halo\\SOURCE\\units\\units.c",
		0x5eb,
		control_data->primary_trigger);

	unit->unit.throttle = control_data->throttle;
	unit->unit.primary_trigger = control_data->primary_trigger;
	unit->unit.aiming_speed = control_data->aiming_speed;
	if (control_data->weapon_index!=NONE)
	{
		unit->unit.desired_weapon_index = control_data->weapon_index;
	}
	if (control_data->grenade_index!=NONE)
	{
		unit->unit.desired_grenade_index = (char)control_data->grenade_index;
	}
	unit->unit.desired_zoom_level = (char)control_data->zoom_level;
	unit->unit.control_flags = control_data->control_flags;
	unit->unit.desired_looking_vector = control_data->looking_vector;
	unit->unit.desired_aiming_vector = control_data->aiming_vector;
	unit->unit.desired_facing_vector = control_data->facing_vector;
	unit->unit.animation.desired_state = control_data->animation_state;

	unit_verify_vectors(unit_index, "unit-control");

	return;
}

static boolean unit_vectors_are_valid(
	long unit_index)
{
	struct unit_datum *unit = unit_get(unit_index);

	boolean result = valid_real_normal3d(&unit->unit.desired_facing_vector) &&
		valid_real_normal3d(&unit->unit.desired_aiming_vector) &&
		valid_real_normal3d(&unit->unit.desired_looking_vector) &&
		valid_real_vector3d_axes2(&unit->object.forward, &unit->object.up) &&
		valid_real_normal3d(&unit->unit.aiming_vector) &&
		valid_real_normal3d(&unit->unit.looking_vector);

	return result;
}

static void unit_throw_grenade_release(
	long unit_index,
	boolean premature)
{
	real_vector3d initial_velocity;

	struct unit_datum *unit = unit_get(unit_index);
	struct unit_definition *unit_definition = unit_definition_get(unit->definition_index);

	if (unit->unit.grenade_throw_state==_unit_grenade_throw_in_hand)
	{
		if (unit->unit.grenade_object_index!=NONE)
		{
			long grenade_index = unit->unit.grenade_object_index;
			
			object_detach(grenade_index);
			
			// Get new grenade origin from the actor
			if (unit->unit.actor_index!=NONE)
			{
				real_point3d origin;

				object_get_origin(unit->unit.grenade_object_index, &origin);
				actor_aim_grenade(unit->unit.actor_index, &origin, &initial_velocity);
			}
			// Calculate the new position if unit is not an actor
			else
			{
				if (unit->unit.player_index!=NONE)
				{
					real_vector3d forward;
					real_vector3d left;
					real_vector3d up;
					real_point3d camera_position;
					double origin_scale;
					struct game_globals_player_information* player_information = TAG_BLOCK_GET_ELEMENT(
						&scenario_get_game_globals()->player_information,
						0,
						struct game_globals_player_information);

					forward = unit->unit.aiming_vector;
					
					if (normalize3d(cross_product3d(global_up3d, &forward, &up))==0.f)
					{
						up = *global_up3d;
					}

					normalize3d(cross_product3d(&forward, &up, &left));
					
					unit_get_camera_position(unit_index, &camera_position);
					origin_scale = player_information->grenade_origin.i;
					point_from_line3d(&camera_position, &forward, (real)origin_scale, &camera_position);
					origin_scale = player_information->grenade_origin.j;
					point_from_line3d(&camera_position, &up, (real)origin_scale, &camera_position);
					origin_scale = player_information->grenade_origin.k;
					point_from_line3d(&camera_position, &left, (real)origin_scale, &camera_position);
					
					object_translate(grenade_index, &camera_position, NULL);
				}

				{
					real scale = unit_definition->unit.grenade_velocity/TICKS_PER_SECOND;
					scale_vector3d(&unit->unit.aiming_vector, scale, &initial_velocity);
				}
			}

			if (premature)
			{
				real power = (real)unit->unit.grenade_throw_ticks/(real)unit->unit.grenade_throw_full_power_ticks;

				if (power < 1.f)
				{
					real_vector3d aiming_vector;

					real range = real_random_range(0.020000001f, 0.046666667f);

					scale_vector3d(&unit->unit.aiming_vector, range, &aiming_vector);
					scale_vector3d(&initial_velocity, power, &initial_velocity);
					point_from_line3d((real_point3d *)&initial_velocity, &aiming_vector, 1.f-power, (real_point3d *)&initial_velocity);
				}
			}

			{
				struct object_datum *grenade = object_get(grenade_index);

				subtract_vectors3d(&initial_velocity, &grenade->object.translational_velocity, &initial_velocity);
				projectile_accelerate(grenade_index, &initial_velocity);
			}
			
			unit->unit.grenade_throw_state = _unit_grenade_throw_ending;
			unit->unit.grenade_object_index = NONE;

			{
				real_point3d new_grenade_origin;

				unit_get_camera_position(unit_index, &new_grenade_origin);

				if (!object_force_inside_bsp(grenade_index, &new_grenade_origin))
				{
					object_delete(grenade_index);
				}
			}
		}
		else
		{
			unit->unit.grenade_throw_state = _unit_grenade_throw_ending;
		}
	}

	return;
}

static void unit_seat_update(
	long object_index)
{
	real_vector3d *acceleration_scale;
	real_vector3d new_velocity;
	struct object_marker seat_marker;

	struct unit_datum *unit = unit_get(object_index);
	boolean found = FALSE;

	if (unit->object.parent_object_index!=NONE && unit->unit.parent_seat_index!=NONE)
	{
		struct unit_definition *parent_unit_definition = unit_definition_get(unit_get(unit->object.parent_object_index)->definition_index);
		struct unit_seat *seat = TAG_BLOCK_GET_ELEMENT(&parent_unit_definition->unit.seats, unit->unit.parent_seat_index, struct unit_seat);

		if (object_get_marker_by_name(unit->object.parent_object_index, seat->marker_name, &seat_marker, 1))
		{
			object_get_origin(unit->object.parent_object_index, &seat_marker.matrix.position);
			acceleration_scale = &seat->acceleration_scale;
			found = TRUE;
		}
	}
	else
	{
		struct unit_definition *unit_definition = unit_definition_get(unit->definition_index);
		
		seat_marker.matrix.position = unit->object.position;
		seat_marker.matrix.forward = unit->object.forward;
		seat_marker.matrix.up = unit->object.up;
		acceleration_scale = &unit_definition->unit.acceleration_scale;
		found = TRUE;
	}

	if (found)
	{
		real_vector3d vp;
		real_vector3d last_velocity;
		last_velocity.i = seat_marker.matrix.position.x-unit->unit.seat_last_position.x;
		last_velocity.j = seat_marker.matrix.position.y-unit->unit.seat_last_position.y;
		last_velocity.k = seat_marker.matrix.position.z-unit->unit.seat_last_position.z;

		new_velocity.i = last_velocity.i - unit->unit.seat_last_velocity.i;
		new_velocity.j = last_velocity.j - unit->unit.seat_last_velocity.j;
		new_velocity.k = last_velocity.k - unit->unit.seat_last_velocity.k;
		
		cross_product3d(&seat_marker.matrix.up, &seat_marker.matrix.forward, &vp);
		
		unit->unit.seat_desired_acceleration.i = (dot_product3d(&new_velocity, &seat_marker.matrix.forward) * acceleration_scale->i) + 0.5f;
		unit->unit.seat_desired_acceleration.j = (dot_product3d(&new_velocity, &vp) * acceleration_scale->j) + 0.5f;
		unit->unit.seat_desired_acceleration.k = (dot_product3d(&new_velocity, &seat_marker.matrix.up) * acceleration_scale->k) + 0.5f;

		unit->unit.seat_desired_acceleration.i = PIN(unit->unit.seat_desired_acceleration.i, 0.f, 1.f);
		unit->unit.seat_desired_acceleration.j = PIN(unit->unit.seat_desired_acceleration.j, 0.f, 1.f);
		unit->unit.seat_desired_acceleration.k = PIN(unit->unit.seat_desired_acceleration.k, 0.f, 1.f);

		unit->unit.seat_last_position = seat_marker.matrix.position;
		unit->unit.seat_last_velocity = last_velocity;
	}
	else
	{
		unit->unit.seat_desired_acceleration.k = 0.5f;
		unit->unit.seat_desired_acceleration.j = 0.5f;
		unit->unit.seat_desired_acceleration.i = 0.5f;
	}

	return;
}

static char const *unit_get_seat_label(
	long object_index)
{
	char const *result;

	struct unit_datum *unit = unit_get(object_index);

	if (unit->object.parent_object_index==NONE || unit->unit.parent_seat_index==NONE)
	{
		short base_seat_index = unit->unit.animation.base_seat_index;
		match_assert("c:\\halo\\SOURCE\\units\\units.c", 8214, base_seat_index>=0 && base_seat_index<NUMBER_OF_UNIT_BASE_SEATS);
		result = base_seat_labels[base_seat_index];
	}
	else
	{
		struct unit_datum *parent_unit = unit_get(unit->object.parent_object_index);
		struct unit_definition *parent_unit_definition = unit_definition_get(parent_unit->definition_index);
		result = TAG_BLOCK_GET_ELEMENT(&parent_unit_definition->unit.seats, unit->unit.parent_seat_index, struct unit_seat)->label;
	}

	return result;
}

static long unit_get_weapon(
	struct unit_datum *unit,
	short index)
{
	long result = NONE;

	if (index!=NONE)
	{
		match_assert("c:\\halo\\SOURCE\\units\\units.c", 8371, index>=0 && index<MAXIMUM_WEAPONS_PER_UNIT);
		result = unit->unit.weapon_object_indices[index];
	}

	return result;
}

static void unit_drop_item(
	long unit_index,
	long item_index)
{
	struct unit_datum *unit = unit_get(unit_index);
	struct item_datum *item = item_get(item_index);
	
	if (item->object.parent_object_index==NONE)
	{
		object_reconnect_to_map(item_index, 0);
		object_set_visibility(item_index, 1);
		object_attach_to_marker(unit_index, "left hand", item_index, "");
	}
	else
	{
		match_assert("c:\\halo\\SOURCE\\units\\units.c", 8396, item->object.parent_object_index==unit_index)
	}

	item_in_unit_inventory(item_index, NONE);
	object_detach(item_index);
	
	item->object.translational_velocity = *global_zero_vector3d;
	item->object.angular_velocity = *global_zero_vector3d;

	{
		real_vector3d item_velocity;

		random_vector_in_cone3d(&unit->unit.aiming_vector, 0.f, 0.39269909f, &item_velocity);
		scale_vector3d(&item_velocity, real_random_range(0.026666667f, 0.040000003f), &item_velocity);
		{
			real_vector3d unit_velocity;
			object_get_velocities(unit_index, &unit_velocity, NULL);
			add_vectors3d(&item_velocity, &unit_velocity, &item_velocity);
		}

		item->item.ignore_object_index = unit_index;
		item_accelerate(item_index, &item_velocity, FALSE);
	}

	{
		real_point3d camera_position;
		unit_get_camera_position(unit_index, &camera_position);

		if (!object_force_inside_bsp(item_index, &camera_position) && !game_engine_running())
		{
			object_delete(item_index);
		}
	}

	if (TEST_FLAG(unit->unit.flags, _unit_doesnt_drop_items_bit))
	{
		object_delete(item_index);
	}

	return;
}

// TODO: Fix
static void unit_cause_continuous_melee_damage(
	long unit_index)
{
	struct unit_datum *unit = unit_get(unit_index);
	struct unit_definition *unit_definition = unit_definition_get(unit->definition_index);

	if (unit->unit.melee_attack_state==4 &&
		unit->object.parent_object_index!=NONE &&
		unit_definition->unit.melee_damage.index!=NONE)
	{
		struct collision_model_test_vector_result vector_result;
		struct damage_data damage_data;
		real_plane3d collision_plane;
		real_point3d collision_point;
		struct collision_model_instance instance;
		real_point3d collision_origin;
		real_vector3d collision_vector;
		boolean collision_passed = FALSE;

		if (unit->unit.melee_continuous_damage_effect_timer==0)
		{
			match_assert("c:\\halo\\SOURCE\\units\\units.c", 8941, global_current_collision_user_depth < MAXIMUM_COLLISION_USER_STACK_DEPTH);

			global_current_collision_users[global_current_collision_user_depth++] = 8;

			if (collision_model_instance_new(&instance, unit->object.parent_object_index))
			{
				object_get_origin(unit_index, &collision_origin);
				scale_vector3d(&unit->object.forward, 0.2f, &collision_vector);
				point_from_line3d(&collision_origin, &collision_vector, -0.5f, &collision_origin);
				
				if (collision_model_test_vector(
					&instance,
					FLAG(_collision_test_front_facing_surfaces_bit) | FLAG(_collision_test_back_facing_surfaces_bit),
					&collision_origin,
					&collision_vector,
					&vector_result))
				{
					point_from_line3d(&collision_origin, &collision_vector, vector_result.bsp_result.t, &collision_point);
					matrix4x3_transform_plane(&instance.matrices[vector_result.node_index], vector_result.bsp_result.plane, &collision_plane);
					
					if (vector_result.bsp_result.plane_designator & LONG_MIN)
					{
						plane3d_negate(&collision_plane, &collision_plane);
					}

					collision_passed = TRUE;
				}
			}

			match_assert("c:\\halo\\SOURCE\\units\\units.c", 8965, global_current_collision_user_depth > 1);
			--global_current_collision_user_depth;
		}
		
		damage_data_new(&damage_data, unit_definition->unit.melee_damage.index);
		
		damage_data.owner_object_index = unit_index;
		damage_data.owner_team_index = unit->object.owner_team_index;
		damage_data.owner_player_index = unit->unit.player_index;
		damage_data.scale = 1.f/30.f;

		if (collision_passed)
		{
			damage_data.epicenter = collision_point;
			damage_data.origin = collision_point;
			damage_data.direction = unit->object.forward;

			SET_FLAG(damage_data.flags, _damage_create_localized_effect_bit, TRUE);
			unit->unit.melee_continuous_damage_effect_timer = 10;

			object_cause_damage(
				&damage_data,
				unit->object.parent_object_index, 
				vector_result.node_index,
				vector_result.region_index,
				vector_result.bsp_result.material_index,
				&collision_plane.n);
		}
		else
		{
			object_cause_damage(
				&damage_data,
				unit->object.parent_object_index,
				NONE,
				NONE,
				NONE,
				NULL);
		}

		--unit->unit.melee_continuous_damage_effect_timer;
	}

	return;
}

static void unit_verify_vectors(
	long unit_index,
	char const *debugstring)
{
	char buffer[512];

	if (!unit_vectors_are_valid(unit_index))
	{
		struct unit_datum *unit = unit_get(unit_index);

		ai_debug_describe_actor(
			unit->unit.actor_index==NONE ? unit->unit.swarm_actor_index : unit->unit.actor_index,
			unit_index,
			1,
			buffer,
			NUMBEROF(buffer)
		);
		error(_error_silent, "**** unit_verify_vectors: problems with %s at location %s", buffer, debugstring);
		error(
			_error_silent,
			"  object: pos %f %f %f, fwd %f %f %f, up %f %f %f",
			unit->object.position.x,
			unit->object.position.y,
			unit->object.position.z,
			unit->object.forward.i,
			unit->object.forward.j,
			unit->object.forward.k,
			unit->object.up.i,
			unit->object.up.j,
			unit->object.up.k
		);
		error(
			_error_silent,
			"  desired facing %f %f %f, aiming %f %f %f, looking %f %f %f",
			unit->unit.desired_facing_vector.i,
			unit->unit.desired_facing_vector.j,
			unit->unit.desired_facing_vector.k,
			unit->unit.desired_aiming_vector.i,
			unit->unit.desired_aiming_vector.j,
			unit->unit.desired_aiming_vector.k,
			unit->unit.desired_looking_vector.i,
			unit->unit.desired_looking_vector.j,
			unit->unit.desired_looking_vector.k
		);
		error(
			_error_silent,
			"  aiming vector %f %f %f velocity %f %f %f",
			unit->unit.aiming_vector.i,
			unit->unit.aiming_vector.j,
			unit->unit.aiming_vector.k,
			unit->unit.aiming_velocity.i,
			unit->unit.aiming_velocity.j,
			unit->unit.aiming_velocity.k
		);
		error(
			_error_silent,
			"  looking vector %f %f %f velocity %f %f %f",
			unit->unit.looking_vector.i,
			unit->unit.looking_vector.j,
			unit->unit.looking_vector.k,
			unit->unit.looking_velocity.i,
			unit->unit.looking_velocity.j,
			unit->unit.looking_velocity.k
		);
		
		error(_error_silent, "  warning, hex dump follows...");

		error(
			_error_silent,
			"  object: pos %08X %08X %08X, fwd %08X %08X %08X, up %08X %08X %08X",
			*(long *)&unit->object.position.x,
			*(long *)&unit->object.position.y,
			*(long *)&unit->object.position.z,
			*(long *)&unit->object.forward.i,
			*(long *)&unit->object.forward.j,
			*(long *)&unit->object.forward.k,
			*(long *)&unit->object.up.i,
			*(long *)&unit->object.up.j,
			*(long *)&unit->object.up.k
		);
		error(
			_error_silent,
			"  desired facing %08X %08X %08X, aiming %08X %08X %08X, looking %08X %08X %08X",
			*(long *)&unit->unit.desired_facing_vector.i,
			*(long *)&unit->unit.desired_facing_vector.j,
			*(long *)&unit->unit.desired_facing_vector.k,
			*(long *)&unit->unit.desired_aiming_vector.i,
			*(long *)&unit->unit.desired_aiming_vector.j,
			*(long *)&unit->unit.desired_aiming_vector.k,
			*(long *)&unit->unit.desired_looking_vector.i,
			*(long *)&unit->unit.desired_looking_vector.j,
			*(long *)&unit->unit.desired_looking_vector.k
		);
		error(
			_error_silent,
			"  aiming vector %08X %08X %08X velocity %08X %08X %08X",
			*(long *)&unit->unit.aiming_vector.i,
			*(long *)&unit->unit.aiming_vector.j,
			*(long *)&unit->unit.aiming_vector.k,
			*(long *)&unit->unit.aiming_velocity.i,
			*(long *)&unit->unit.aiming_velocity.j,
			*(long *)&unit->unit.aiming_velocity.k
		);
		error(
			_error_silent,
			"  looking vector %08X %08X %08X velocity %08X %08X %08X",
			*(long *)&unit->unit.looking_vector.i,
			*(long *)&unit->unit.looking_vector.j,
			*(long *)&unit->unit.looking_vector.k,
			*(long *)&unit->unit.looking_velocity.i,
			*(long *)&unit->unit.looking_velocity.j,
			*(long *)&unit->unit.looking_velocity.k
		);

		match_vassert(
			"c:\\halo\\SOURCE\\units\\units.c",
			594,
			unit_vectors_are_valid(unit_index),
			"unit_verify_vectors FAILURE, see above for details"
		);
	}
	
	return;
}

static void unit_running_blind(
	long unit_index,
	real_vector3d *run_vector)
{
	struct unit_datum *unit = unit_get(unit_index);
	boolean actor_controlled = FALSE;
	
	if (unit->unit.actor_index==NONE || !actor_get_running_blind_vector(unit->unit.actor_index, run_vector))
	{
		*run_vector = *global_forward3d;
	}
	else
	{
		actor_controlled = TRUE;
	}

	match_assert_valid_real_normal3d("c:\\halo\\SOURCE\\units\\units.c", 9541, run_vector);

	{
		real angular_acceleration_this_tick;

		real positive_angle_allowed = 1.f;
		real negative_angle_allowed = 1.f;

		if (actor_controlled)
		{
			real negative_angle_bounds_dist = DEGREES_TO_RADIANS(45)-unit->unit.run_blindly_angle;
			real positive_angle_bounds_dist = DEGREES_TO_RADIANS(45)+unit->unit.run_blindly_angle;

			negative_angle_allowed = MIN(negative_angle_allowed, negative_angle_bounds_dist / DEGREES_TO_RADIANS(13.5f));
			positive_angle_allowed = MIN(positive_angle_allowed, positive_angle_bounds_dist / DEGREES_TO_RADIANS(13.5f));
		}

		{
			real negative_velocity_bounds_dist = DEGREES_TO_RADIANS(12.f)-unit->unit.run_blindly_angle_delta;
			real positive_velocity_bounds_dist = DEGREES_TO_RADIANS(12.f)+unit->unit.run_blindly_angle_delta;

			negative_angle_allowed = MIN(negative_angle_allowed, negative_velocity_bounds_dist * 15.915494f);
			positive_angle_allowed = MIN(positive_angle_allowed, positive_velocity_bounds_dist * 15.915494f);

		}

		if (negative_angle_allowed<positive_angle_allowed)
		{
			if (negative_angle_allowed<-1.f)
			{
				angular_acceleration_this_tick = -0.020943951f;
			}
			else
			{
				real min_allowed = MIN(1.f, negative_angle_allowed);
				angular_acceleration_this_tick = real_random_range(-0.020943951f, 0.020943951f * min_allowed);
			}
		}
		else
		{
			if (positive_angle_allowed<-1.f)
			{
				angular_acceleration_this_tick = 0.020943951f;
			}
			else
			{
				real max_allowed = MIN(1.f, positive_angle_allowed);
				angular_acceleration_this_tick = real_random_range(-0.020943951f * max_allowed, 0.020943951f);
			}
		}

		unit->unit.run_blindly_angle_delta+=angular_acceleration_this_tick;
	}

	unit->unit.run_blindly_angle += unit->unit.run_blindly_angle_delta;

	if (unit->unit.run_blindly_angle < -_pi)
	{
		unit->unit.run_blindly_angle += 2.f*_pi;
	}
	else if (unit->unit.run_blindly_angle > _pi)
	{
		unit->unit.run_blindly_angle -= 2.f*_pi;
	}
	
	rotate_vector_about_axis(run_vector, global_up3d, sine(unit->unit.run_blindly_angle), cosine(unit->unit.run_blindly_angle));

	match_assert_valid_real_normal3d("c:\\halo\\SOURCE\\units\\units.c", 9612, run_vector)

	return;
}

boolean unit_unsuspecting(
	long unit_index,
	real_point3d const *point)
{
	struct unit_datum *unit = unit_try_and_get(unit_index);

	if (unit!=NULL &&
		unit->object.type==_object_type_biped)
	{
		struct unit_definition *unit_definition = unit_definition_get(unit->definition_index);

		if (!TEST_FLAG(unit_definition->unit.flags, _unit_never_unsuspecting_bit))
		{
			real_vector3d direction;

			vector_from_points3d(point, &unit->object.bounding_sphere_center, &direction);

			if (dot_product3d(&direction, &unit->unit.looking_vector)>0.f)
			{
				return TRUE;
			}
			else
			{
				char const *base_seat_label =
					base_seat_labels[_unit_base_seat_asleep];

				if (csstrcmp(base_seat_label, unit_get_seat_label(unit_index))==0)
				{
					return TRUE;
				}
			}
		}
	}

	return FALSE;
}

static boolean unit_integrated_night_vision_is_active(
	long unit_index)
{
	boolean active = FALSE;

	if (unit_get(unit_index)->unit.current_zoom_level!=NONE)
	{
		long current_weapon_index = unit_get_current_weapon_index(unit_index);

		if (current_weapon_index!=NONE)
		{
			struct weapon_datum *weapon = weapon_get(current_weapon_index);
			struct weapon_definition* weapon_definition = weapon_definition_get(weapon->definition_index);
			
			if (TEST_FLAG(weapon_definition->weapon.flags, _weapon_enables_unit_integrated_night_vision_bit))
			{
				active = TRUE;
			}
		}
	}

	return active;
}

/* Verify the public seat-helper declaration without perturbing this legacy
 * translation unit's authenticated function-declaration order. */
/* the distributed netcode (port/linux/game/network_objects.c): a client's
unit carries the host's weapons, the same objects, moved in and out as the
host's unit had them (the host has applied the game's rules) */

void unit_network_forget_weapon(long unit_index, short slot);

/* the weapon into the slot, as unit_add_weapon_to_inventory puts one in */
void unit_network_add_weapon(
	long unit_index,
	long weapon_index,
	short slot)
{
	struct unit_datum *unit = unit_get(unit_index);
	struct weapon_datum *weapon = weapon_get(weapon_index);

	if (weapon->object.parent_object_index != NONE)
		object_detach(weapon_index);
	if (TEST_FLAG(weapon->object.flags, _object_connected_to_map_bit))
		object_disconnect_from_map(weapon_index);
	object_set_visibility(weapon_index, FALSE);
	item_in_unit_inventory(weapon_index, unit_index);
	unit->unit.weapon_object_indices[slot] = weapon_index;
	unit->unit.weapon_last_used_at_game_time[slot] = 0;
}

/* the slot's weapon out onto the ground, as unit_drop_current_weapon drops
one (where the host has it the objects' states say) */
void unit_network_drop_weapon(
	long unit_index,
	short slot)
{
	struct unit_datum *unit = unit_get(unit_index);
	long weapon_index = unit->unit.weapon_object_indices[slot];

	if (weapon_index == NONE)
		return;
	if (unit->unit.current_weapon_index == slot)
		first_person_weapon_message_from_unit(unit_index, 13);
	unit_drop_item(unit_index, weapon_index);
	unit_network_forget_weapon(unit_index, slot);
}

/* the slot emptied, its weapon about to be deleted */
void unit_network_forget_weapon(
	long unit_index,
	short slot)
{
	struct unit_datum *unit = unit_get(unit_index);

	unit->unit.weapon_object_indices[slot] = NONE;
	if (unit->unit.current_weapon_index == slot)
		unit->unit.current_weapon_index = NONE;
	if (unit->unit.desired_weapon_index == slot || unit->unit.desired_weapon_index == NONE)
		unit->unit.desired_weapon_index = unit_weapon_next_index(unit_index, NONE, 0);
}
