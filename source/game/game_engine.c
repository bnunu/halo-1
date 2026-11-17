/*
GAME_ENGINE.C

symbols in this file:
00096780 0020:
	_game_globals_get_weapon (0000)
000967A0 0070:
	_game_engine_get_team_score (0000)
00096810 0020:
	_linear_to_non_linear_alpha (0000)
00096830 0020:
	_game_engine_dispose (0000)
00096850 0040:
	_initialize_player_multiplayer_data (0000)
00096890 0120:
	_game_engine_build_lighting (0000)
000969B0 0020:
	_game_engine_dispose_from_old_map (0000)
000969D0 0020:
	_game_engine_game_ending (0000)
000969F0 0020:
	_game_engine_game_starting (0000)
00096A10 0040:
	_players_in_game (0000)
00096A50 0020:
	_game_engine_statistics_append (0000)
00096A70 0020:
	_game_engine_handle_client_message (0000)
00096A90 0020:
	_game_engine_handle_server_message (0000)
00096AB0 0030:
	_sort_statistic_buffer (0000)
00096AE0 0050:
	_sort_statistic_buffer_ranking (0000)
00096B30 0010:
	_is_place_tied (0000)
00096B40 0010:
	_place_get_position (0000)
00096B50 0050:
	_get_place_string (0000)
00096BA0 0070:
	_drawline (0000)
00096C10 0020:
	_get_selected_color (0000)
00096C30 0050:
	_get_postgame_hilite_colors (0000)
00096C80 0060:
	_find_closest_player_callback (0000)
00096CE0 0020:
	_game_engine_post_rasterize_objects (0000)
00096D00 0030:
	_can_delete_item (0000)
00096D30 0100:
	_game_engine_update_purge (0000)
00096E30 00a0:
	_update_weapon_inventory (0000)
00096ED0 0150:
	_game_engine_update_weapons (0000)
00097020 00b0:
	_item_collection_get_total (0000)
000970D0 0050:
	_game_engine_load_stage (0000)
00097120 0030:
	_game_engine_playlist_begin (0000)
00097150 0060:
	_game_engine_get_current_stage (0000)
000971B0 0030:
	_game_engine_end_game (0000)
000971E0 0020:
	_game_engine_allow_pick_up (0000)
00097200 0050:
	_game_engine_player_damaged_player (0000)
00097250 00a0:
	_game_engine_player_is_odd_man_out (0000)
000972F0 0040:
	_game_engine_player_is_out_of_lives (0000)
00097330 0100:
	_game_engine_should_spawn_player (0000)
00097430 0070:
	_game_engine_player_get_team_index (0000)
000974A0 0040:
	_game_engine_prespawn_player_update (0000)
000974E0 0010:
	_game_engine_running (0000)
000974F0 0020:
	_game_engine_can_score (0000)
00097510 0020:
	_game_engine_force_single_screen (0000)
00097530 0030:
	_get_blink_alpha (0000)
00097560 0010:
	_game_engine_press_start_to_begin (0000)
00097570 00f0:
	_nearby_vehicle (0000)
00097660 0100:
	_game_engine_rasterize_message (0000)
00097760 00e0:
	_game_engine_picking_up (0000)
00097840 0080:
	_goal_matches_player (0000)
000978C0 00a0:
	_game_engine_player_get_custom_motion_sensor_positions (0000)
00097960 0030:
	_get_flag_definition_index (0000)
00097990 0030:
	_get_ball_definition_index (0000)
000979C0 0040:
	_game_engine_switch_to_postgame (0000)
00097A00 0010:
	_game_engine_get_variant (0000)
00097A10 0020:
	_game_engine_get_goal_in_use (0000)
00097A30 0060:
	_game_engine_get_goal_position (0000)
00097A90 0080:
	_game_engine_set_goal_position (0000)
00097B10 0020:
	_game_engine_clear_goal_position (0000)
00097B30 00d0:
	_game_engine_render_nav_points (0000)
00097C00 0020:
	_game_engine_infinite_grenades_internal (0000)
00097C20 0030:
	_game_engine_infinite_grenades (0000)
00097C50 0020:
	_game_engine_has_teams (0000)
00097C70 0030:
	_game_engine_display_team_indicators (0000)
00097CA0 0030:
	_game_engine_has_shield (0000)
00097CD0 0060:
	_weapon_definition_index_to_list_index (0000)
00097D30 0030:
	_list_index_to_weapon_definition_index (0000)
00097D60 00c0:
	_game_engine_remap_vehicle (0000)
00097E20 0190:
	_game_engine_remap_weapon (0000)
00097FB0 0070:
	_game_engine_man_out (0000)
00098020 0030:
	_game_engine_state_message (0000)
00098050 0090:
	_game_engine_player_get_change_color (0000)
000980E0 0070:
	_game_engine_player_has_flag (0000)
00098150 0050:
	_game_engine_player_depower_active_camo (0000)
000981A0 00e0:
	_get_place_name (0000)
00098280 0170:
	_game_engine_get_place (0000)
000983F0 0040:
	_game_engine_hud_draw_motion_sensor (0000)
00098430 0020:
	_game_engine_test_flag (0000)
00098450 0020:
	_game_engine_test_trait (0000)
00098470 00a0:
	_netgame_flag_verify_no_team_duplicates (0000)
00098510 0070:
	_netgame_flag_verify_team_range (0000)
00098580 0070:
	_game_engine_playlist_next (0000)
000985F0 0090:
	_build_game_variant_slayer (0000)
00098680 0090:
	_build_game_variant_slayer_pro (0000)
00098710 0090:
	_build_game_variant_elimination (0000)
000987A0 0090:
	_build_game_variant_phantoms (0000)
00098830 0090:
	_build_game_variant_endurance (0000)
000988C0 0090:
	_build_game_variant_rockets (0000)
00098950 0090:
	_build_game_variant_snipers (0000)
000989E0 0090:
	_build_game_variant_team_slayer (0000)
00098A70 0090:
	_build_game_variant_oddball (0000)
00098B00 0090:
	_build_game_variant_team_oddball (0000)
00098B90 0090:
	_build_game_variant_reverse_tag (0000)
00098C20 00a0:
	_build_game_variant_accumulation (0000)
00098CC0 00a0:
	_build_game_variant_juggernaut (0000)
00098D60 00a0:
	_build_game_variant_stalker (0000)
00098E00 0080:
	_build_game_variant_king (0000)
00098E80 0090:
	_build_game_variant_king_pro (0000)
00098F10 0080:
	_build_game_variant_crazy_king (0000)
00098F90 0090:
	_build_game_variant_team_king (0000)
00099020 0090:
	_build_game_variant_ctf (0000)
000990B0 0090:
	_build_game_variant_ctf_pro (0000)
00099140 0090:
	_build_game_variant_invasion (0000)
000991D0 0090:
	_build_game_variant_iron_ctf (0000)
00099260 0090:
	_build_game_variant_race (0000)
000992F0 0090:
	_build_game_variant_rally (0000)
00099380 0090:
	_build_game_variant_team_race (0000)
00099410 0090:
	_build_game_variant_team_rally (0000)
000994A0 0030:
	_game_engine_override_map_name (0000)
000994D0 0020:
	_game_engine_override_game_variant (0000)
000994F0 01a0:
	_rasterize_in_game_score_draw_line (0000)
00099690 0060:
	_game_engine_hud_draw_messages (0000)
000996F0 0080:
	_game_engine_player_has_stealth_weapon (0000)
00099770 0110:
	_game_engine_weapon_fired (0000)
00099880 0030:
	_test_any_gamepad_button (0000)
000998B0 00c0:
	_ticks_to_unicode_time_string (0000)
00099970 0060:
	_game_engine_flag_reset (0000)
000999D0 01b0:
	_game_engine_variant_cleanup (0000)
00099B80 0010:
	_game_engine_allow_pause (0000)
00099B90 0270:
	_game_engine_predict_resources (0000)
00099E00 0020:
	_game_engine_draw_object_in_motion_sensor (0000)
00099E20 0020:
	_game_engine_allow_dynamic_lighting (0000)
00099E40 0020:
	_game_engine_allow_integrated_lights (0000)
00099E60 0030:
	_game_engine_force_autopickup (0000)
00099E90 0060:
	_game_engine_initialize (0000)
00099EF0 0100:
	_multiple_teams_alive (0000)
00099FF0 00f0:
	_team_has_players (0000)
0009A0E0 0020:
	_game_engine_should_end_game (0000)
0009A100 0080:
	_adjust_score_for_ranking (0000)
0009A180 0230:
	_populate_statistic_buffer (0000)
0009A3B0 0080:
	_game_engine_get_player_place (0000)
0009A430 0060:
	_postgame_statistic_get_rating (0000)
0009A490 01f0:
	_select_players_to_display (0000)
0009A680 01c0:
	_find_closest_player_index (0000)
0009A840 0100:
	_internal_rasterize_target_name (0000)
0009A940 0920:
	_internal_rasterize_score (0000)
0009B260 0080:
	_random_item (0000)
0009B2E0 0020:
	_game_engine_get_type (0000)
0009B300 00a0:
	_match_game_type (0000)
0009B3A0 0150:
	_game_engine_update_item_spawn (0000)
0009B4F0 0070:
	_game_engine_update_player_no_shield (0000)
0009B560 0060:
	_game_engine_update_player_always_invis (0000)
0009B5C0 00e0:
	_game_engine_update_non_deterministic (0000)
0009B6A0 0040:
	_multiplayer_message_internal (0000)
0009B6E0 0090:
	_multiplayer_message (0000)
0009B770 0010:
	_game_show_score_one_player (0000)
0009B780 0060:
	_game_show_score_team (0000)
0009B7E0 00d0:
	_game_show_score_you_ally_enemy (0000)
0009B8B0 0080:
	_game_show_score_extended (0000)
0009B930 0020:
	_game_show_score (0000)
0009B950 0110:
	_find_netgame_flags (0000)
0009BA60 0040:
	_find_netgame_flag (0000)
0009BAA0 0130:
	_handle_custom_starting_equipment (0000)
0009BBD0 0150:
	_game_engine_postspawn_player_update (0000)
0009BD20 00d0:
	_game_engine_get_damage_multiplier (0000)
0009BDF0 03e0:
	_game_engine_update_teleporter (0000)
0009C1D0 0170:
	_game_engine_get_distance_rating_for_spawn (0000)
0009C340 0120:
	_game_engine_get_friendly_bonus (0000)
0009C460 00b0:
	_default_starting_location_rate_function (0000)
0009C510 0060:
	_game_engine_get_starting_location_rating (0000)
0009C570 0220:
	_game_engine_get_variant_by_name (0000)
0009C790 0130:
	_game_engine_remap_equipment (0000)
0009C8C0 0070:
	_game_engine_remap_object_definition (0000)
0009C930 0140:
	_game_engine_get_state_message (0000)
0009CA70 00c0:
	_game_engine_did_player_win_default (0000)
0009CB30 0030:
	_game_engine_did_player_win (0000)
0009CB60 0080:
	_game_engine_did_team_win (0000)
0009CBE0 0040:
	_netgame_flag_verify_team_exists (0000)
0009CC20 0060:
	_netgame_verify_spawn_points (0000)
0009CC80 0070:
	_netgame_verify_equipment (0000)
0009CCF0 0280:
	_game_engine_verify_current_map (0000)
0009CF70 0010:
	_game_engine_playlist_initialize (0000)
0009CF80 0080:
	_game_engine_initialize_for_new_map (0000)
0009D000 0140:
	_game_engine_player_added (0000)
0009D140 0480:
	_game_engine_generate_title_string (0000)
0009D5C0 09e0:
	_game_engine_post_rasterize_post_game (0000)
0009DFA0 02f0:
	_game_engine_update (0000)
0009E290 0340:
	_game_engine_player_killed (0000)
0009E5D0 00a0:
	_game_engine_nonplayer_post_rasterize (0000)
0009E670 0350:
	_game_engine_rasterize_in_game_score (0000)
0009E9C0 0140:
	_game_engine_post_rasterize_in_game (0000)
0009EB00 0060:
	_game_engine_post_rasterize (0000)
0025B190 0008:
	__real@3ffe666660000000 (0000)
0025B198 0002:
	??_C@_11LOCGONAA@?$AA?$AA@ (0000)
0025B19C 0019:
	??_C@_0BJ@IAIEOKNM@ui?2multiplayer_game_text?$AA@ (0000)
0025B1B8 001d:
	??_C@_0BN@BLHEDFPC@weapon_is_flag?$CIweapon_index?$CJ?$AA@ (0000)
0025B1D8 0022:
	??_C@_0CC@HKAICMNO@c?3?2halo?2SOURCE?2game?2game_engine?4@ (0000)
0025B1FC 003c:
	??_C@_0DM@INOFLLBD@?$CIitem?9?$DOobject?4scale?5?$DO?$DN?50?45f?$CJ?5?$CG?$CG?5@ (0000)
0025B238 0014:
	??_C@_0BE@PEGHDBAP@NULL?5?$CB?$DN?5game_engine?$AA@ (0000)
0025B24C 0014:
	??_C@_0BE@CKCLKIJA@variant?5?$CG?$CG?5map_name?$AA@ (0000)
0025B260 001a:
	??_C@_0BK@EKPIBKAB@dead_player_index?5?$CB?$DN?5NONE?$AA@ (0000)
0025B27C 000c:
	??_C@_0M@CLNODINK@game_engine?$AA@ (0000)
0025B288 0008:
	__real@3f53104b57cf969e (0000)
0025B290 0007:
	??_C@_06IEOJBDIK@object?$AA@ (0000)
0025B298 0097:
	??_C@_0JH@PNHLNBOL@?$CBallow_pick_up?5?$HM?$HM?5?$CBTEST_FLAG?$CIwea@ (0000)
0025B330 001a:
	??_C@_0BK@LOIEOAAE@global_goal?$FLindex?$FN?4in_use?$AA@ (0000)
0025B34C 0004:
	__real@3f2147ae (0000)
0025B350 0015:
	??_C@_0BF@MOJDLNAG@NONE?5?$CB?$DN?5lookup_index?$AA@ (0000)
0025B368 001d:
	??_C@_0BN@OIBHHACP@place?4place?5?$DM?5maximum_places?$AA@ (0000)
0025B388 002c:
	??_C@_0CM@IEEKPKHB@?$CI?$CBall_tied?5?$HM?$HM?5?$CItied?$CJ?$CJ?5?$HM?$HM?5?$CI1?5?$DN?$DN?5g@ (0000)
0025B3B4 001e:
	??_C@_0BO@HPNHAHOG@levels?2test?2carousel?2carousel?$AA@ (0000)
0025B3D4 000c:
	??_C@_1M@CDCAHPKB@?$AA?$CF?$AAs?$AA?3?$AA?$CF?$AAs?$AA?$AA@ (0000)
0025B3E0 0008:
	??_C@_17PMDFIHDK@?$AA0?$AA?$CF?$AAd?$AA?$AA@ (0000)
0025B3E8 0006:
	??_C@_15KNBIKKIN@?$AA?$CF?$AAd?$AA?$AA@ (0000)
0025B3F0 0004:
	??_C@_13HOIJIPNN@?$AA?5?$AA?$AA@ (0000)
0025B3F8 0046:
	??_C@_0EG@GDPJDEHO@NETGAME?5CODE?5FAILURE?3?5game_engin@ (0000)
0025B440 001b:
	??_C@_0BL@FBPHGMEB@player?9?$DOteam_index?5?$CB?$DN?5NONE?$AA@ (0000)
0025B45C 0013:
	??_C@_0BD@JDPLED@player_index?$CB?$DNNONE?$AA@ (0000)
0025B470 002b:
	??_C@_0CL@DLMKKJM@player_count?5?$DM?5MULTIPLAYER_MAXIM@ (0000)
0025B49C 0022:
	??_C@_0CC@FPLNHLNP@place?$DMMULTIPLAYER_MAXIMUM_PLAYER@ (0000)
0025B4C0 0013:
	??_C@_0BD@EGHPGJBP@found?5local?5player?$AA@ (0000)
0025B4D4 001d:
	??_C@_0BN@LMCPNAFN@player_count?$DN?$CFd?0?5maxcount?$DN?$CFd?$AA@ (0000)
0025B4F8 0008:
	__real@3fc0bf25a0000000 (0000)
0025B500 0015:
	??_C@_0BF@IHIEPAI@NONE?5?$CB?$DN?5player_index?$AA@ (0000)
0025B518 0016:
	??_C@_0BG@OJNPNGFL@failed?5to?5teleport?5?$CFd?$AA@ (0000)
0025B530 0008:
	__real@3fe3333340000000 (0000)
0025B538 000a:
	??_C@_09DOOCKEDN@team_king?$AA@ (0000)
0025B544 0005:
	??_C@_04PJOEONHN@king?$AA@ (0000)
0025B54C 0008:
	??_C@_07LIOCHKOH@ironctf?$AA@ (0000)
0025B554 0004:
	??_C@_03JHHHHEKD@ctf?$AA@ (0000)
0025B558 0008:
	??_C@_07JHGHBFJP@oddball?$AA@ (0000)
0025B560 000d:
	??_C@_0N@JBNPINGA@accumulation?$AA@ (0000)
0025B570 000d:
	??_C@_0N@CPCFMJOB@team_oddball?$AA@ (0000)
0025B580 0008:
	??_C@_07IAHNGGND@stalker?$AA@ (0000)
0025B588 000c:
	??_C@_0M@MLMHALFL@elimination?$AA@ (0000)
0025B594 000c:
	??_C@_0M@NOCMPHHF@team_slayer?$AA@ (0000)
0025B5A0 0007:
	??_C@_06CBFFIGEC@slayer?$AA@ (0000)
0025B5A8 0006:
	??_C@_05GCEFBECL@rally?$AA@ (0000)
0025B5B0 000a:
	??_C@_09KMEIIIPA@team_race?$AA@ (0000)
0025B5BC 0005:
	??_C@_04GLEOMBLA@race?$AA@ (0000)
0025B5C4 0004:
	__real@3f0ccccd (0000)
0025B5C8 003b:
	??_C@_0DL@MBPNEOKF@NETGAME?5MAP?5FAILURE?3?5failed?5to?5f@ (0000)
0025B604 003b:
	??_C@_0DL@FDFHGCGI@NETGAME?5MAP?5FAILURE?3?5failed?5to?5f@ (0000)
0025B640 003e:
	??_C@_0DO@GDPJBCBM@NETGAME?5MAP?5FAILURE?3?5failed?5to?5f@ (0000)
0025B680 003d:
	??_C@_0DN@GLOHFEFE@NETGAME?5MAP?5FAILURE?3?5failed?5to?5f@ (0000)
0025B6C0 003a:
	??_C@_0DK@COLEDOMN@NETGAME?5MAP?5FAILURE?3?5failed?5to?5f@ (0000)
0025B700 0047:
	??_C@_0EH@GJGBEMDH@NETGAME?5MAP?5FAILURE?3?5failed?5to?5f@ (0000)
0025B748 0047:
	??_C@_0EH@BMGHBGKH@NETGAME?5MAP?5FAILURE?3?5failed?5to?5f@ (0000)
0025B790 004a:
	??_C@_0EK@DHDLIGMN@NETGAME?5MAP?5FAILURE?3?5failed?5to?5f@ (0000)
0025B7E0 0049:
	??_C@_0EJ@FACIONE@NETGAME?5MAP?5FAILURE?3?5failed?5to?5f@ (0000)
0025B830 004f:
	??_C@_0EP@CGHPHLAM@NETGAME?5MAP?5FAILURE?3?5failed?5to?5f@ (0000)
0025B880 004f:
	??_C@_0EP@MJLNBADC@NETGAME?5MAP?5FAILURE?3?5failed?5to?5f@ (0000)
0025B8D0 0039:
	??_C@_0DJ@MPJKFIIJ@NETGAME?5MAP?5FAILURE?3?5duplicate?5r@ (0000)
0025B90C 0031:
	??_C@_0DB@KDFIELID@NETGAME?5MAP?5FAILURE?3?5missing?5rac@ (0000)
0025B940 0034:
	??_C@_0DE@KIBANADL@NETGAME?5MAP?5FAILURE?3?5missing?5odd@ (0000)
0025B974 0031:
	??_C@_0DB@IKODHNKI@NETGAME?5MAP?5FAILURE?3?5missing?5hil@ (0000)
0025B9A8 0035:
	??_C@_0DF@EHBELDKM@NETGAME?5MAP?5FAILURE?3?5ctf?5flag?5ou@ (0000)
0025B9E0 0032:
	??_C@_0DC@DCAAPLNK@NETGAME?5MAP?5FAILURE?3?5duplicate?5c@ (0000)
0025BA14 0030:
	??_C@_0DA@LKKJMHEH@NETGAME?5MAP?5FAILURE?3?5missing?5ctf@ (0000)
0025BA48 0057:
	??_C@_0FH@ODDMDCIB@failed?5to?5initialize?5custome?5gam@ (0000)
0025BAA0 000d:
	??_C@_0N@MABFIMPF@title_string?$AA@ (0000)
0025BAB0 001e:
	??_C@_1BO@NNCACCCJ@?$AA?5?$AA?7?$AA?5?$AA?7?$AA?5?$AA?7?$AA?5?$AA?7?$AA?5?$AA?7?$AA?5?$AA?7?$AA?$CF?$AAd?$AA?$AA@ (0000)
0025BAD0 001a:
	??_C@_1BK@JPAKMPFP@?$AA?5?$AA?7?$AA?5?$AA?7?$AA?5?$AA?7?$AA?5?$AA?7?$AA?5?$AA?7?$AA?$CF?$AAd?$AA?$AA@ (0000)
0025BAEC 0016:
	??_C@_1BG@EEDCCJKF@?$AA?5?$AA?7?$AA?5?$AA?7?$AA?5?$AA?7?$AA?5?$AA?7?$AA?$CF?$AAd?$AA?$AA@ (0000)
0025BB04 0012:
	??_C@_1BC@DGONEHBJ@?$AA?5?$AA?7?$AA?5?$AA?7?$AA?5?$AA?7?$AA?$CF?$AAs?$AA?$AA@ (0000)
0025BB18 000e:
	??_C@_1O@HAEHHN@?$AA?5?$AA?7?$AA?5?$AA?7?$AA?$CF?$AAs?$AA?$AA@ (0000)
0025BB28 000a:
	??_C@_19GCIMBEIF@?$AA?5?$AA?7?$AA?$CF?$AAs?$AA?$AA@ (0000)
0025BB34 0026:
	??_C@_1CG@MDCDIADO@?$AA?7?$AA?$CF?$AAs?$AA?7?$AA?$CF?$AAs?$AA?7?$AA?$CF?$AAs?$AA?7?$AA?$CF?$AAs?$AA?7?$AA?$CF?$AAs?$AA?7?$AA?$CF?$AAs?$AA?$AA@ (0000)
0025BB5C 0012:
	??_C@_0BC@FNEEDLLE@ambient_computers?$AA@ (0000)
0025BB70 0012:
	??_C@_0BC@GOFPLFEH@ambient_machinery?$AA@ (0000)
0025BB84 000f:
	??_C@_0P@PLBDAHNN@ambient_nature?$AA@ (0000)
0025BB94 0014:
	??_C@_1BE@EFJKLKFF@?$AA?7?$AA?$CF?$AAs?$AA?7?$AA?$CF?$AAs?$AA?7?$AA?$CF?$AAs?$AA?$AA@ (0000)
0025BBA8 001b:
	??_C@_0BL@NPJONDMM@NONE?5?$CB?$DN?5local_player_index?$AA@ (0000)
002DE3E0 0020:
	_debug_player_color (0000)
0043E498 047c:
	_global_goal (0000)
	_global_variant (0400)
	_game_engine (0468)
	_global_autogenerate_list (0474)
	_global_autogenerate_count (0478)
*/

/* ---------- headers */

#include "cseries.h"
#include "game_modes.h" /* port: port/linux/game/game_modes.c */
#include "keyboard_glyphs.h" /* port: port/linux/game/keyboard_glyphs.c */
#include "infection.h" /* port: port/linux/game/infection.c */
#include "cseries_windows.h"
#include "errors.h"
#include "aim_assist.h"
#include "game_engine.h"
#include "game_engine_list.h"
#include "game_engine_place.h"

#include "bitmaps/bitmap_group.h"
#include "bitmaps/bitmap_group_lookup.h"
#include "effects/player_effects.h"
#include "game_allegiance.h"
#include "game.h"
#include "game_globals.h"
#include "interface/interface.h"
#include "interface/hud.h"
#include "interface/hud_definitions.h"
#include "interface/hud_messaging.h"
#include "interface/player_ui.h"
#include "interface/terminal.h"
#include "interface/ui_widget.h"
#include "input/input.h"
#include "items/equipment_definitions.h"
#include "items/item_definitions.h"
#include "items/weapon_definitions.h"
#include "items/weapons.h"
#include "main/console.h"
#include "main/main.h"
#include "math/integer_math.h"
#include "networking/network_game_globals.h"
#include "networking/network_server_manager.h"
#include "networking/network_game_manager.h"
#include "network_coop.h" /* port: port/linux/game/network_coop.c */
#include "network_votekick.h" /* port: port/linux/game/network_votekick.c */
#include "network_voice.h" /* port: port/linux/game/network_voice.c */
#include "network_chat.h" /* port: port/linux/game/network_chat.c */
/* (network_server_manager_internal.h's: the host's game record) */
struct network_game *network_game_server_get_game(struct network_game_server *server);
#include "objects.h"
#include "objects/damage_effect_definitions.h"
#include "physics/collision_features.h"
#include "player_rumble.h"
#include "players.h"
#include "players_runtime.h"
#include "rasterizer/rasterizer.h"
#include "rasterizer/rasterizer_console_vars.h"
#include "render/render.h"
#include "saved games/player_profile.h"
#include "scenario/scenario.h"
#include "scenario/scenario_definitions.h"
#include "sound/sound_classes.h"
#include "text/draw_string.h"
#include "text/font_group.h"
#include "text/text_group.h"
#include "text/unicode.h"
#include "units/bipeds.h"
#include "units/units.h"
#include "custom_edition_cache.h" /* port: port/linux/game/custom_edition_cache.c */
#include "mcc_cache.h" /* port: port/linux/game/mcc_cache.h */
#include "mcc_objects.h" /* port: port/linux/game/mcc_objects.h */
#include "mcc_player.h" /* port: port/linux/game/mcc_player.h */
#include "mcc_ui_teams.h" /* port: port/linux/game/mcc_ui_teams.c */
#include "server_team_switch.h" /* port: port/linux/game/server_team_switch.c */
#include "latency_meter.h" /* port: port/linux/game/latency_meter.c */
#include "touch_postgame.h" /* port: port/linux/game/touch_postgame.c */

/* network_game_globals.c's */
boolean network_game_distributed_client(void);
/* port/linux/game/network_distributed.c's */
void network_distributed_player_killed(long *killing_player_index, long *killing_object_index,
	long dead_player_index, boolean *friendly_fire);
/* port/linux/game/network_damage.c's */
boolean network_damage_killer_score(long player_index, long *score);

/* ---------- constants */

enum
{
	/* port: the native builds' session limit (halo_port_limits.h) */
	MULTIPLAYER_MAXIMUM_PLAYERS = HALO_PORT_MAXIMUM_NETWORK_PLAYERS,
	/* ui\multiplayer_game_text only has strings for the first 16 places */
	NUMBER_OF_PLACE_STRINGS = 16,
};

/* scenario_starting_equipment.flags (HCEX names) */
enum
{
	_netgame_starting_equipment_flag_no_grenades_bit = 0,
	_netgame_starting_equipment_flag_plasma_greandes_bit, /* (sic) HCEX spelling */
};

/* game_engine_globals.flags and universal_variant.flags bits used only by this file
   (the lower bits are declared with their siblings in game_engine.h) */
enum
{
	_game_engine_9_or_more_players_bit = 3,
	_game_engine_game_over_sound_disabled_bit,
};

enum
{
	_game_engine_all = 12,
	_game_engine_all_non_team,
	_game_engine_all_normal,
};

enum
{
	_multiplayer_sound_game_over = 0x1,
	_multiplayer_sound_double_kill = 0xE,
	_multiplayer_sound_triple_kill,
	_multiplayer_sound_killtacular_kill,
	_multiplayer_sound_running_riot,
	_multiplayer_sound_killing_spree,
	_multiplayer_sound_teleporter_activate = 0x1B,
	_multiplayer_sound_countdown_for_respawn = 0x1D,
	_multiplayer_sound_respawn = 0x1F,
};

enum game_engine_mode
{
	game_engine_mode_active = 0,
	game_engine_mode_postgame_delay,
	game_engine_mode_postgame_rasterize_delay,
	game_engine_mode_postgame_rasterize,
};

enum postgame_statistic
{
	_postgame_statistic_ranking = 0,
	_postgame_statistic_score,
	_postgame_statistic_kills,
	_postgame_statistic_assists,
	_postgame_statistic_deaths,
	NUMBER_OF_POSTGAME_STATISTICS,
};

enum game_engine_weapons
{
	_game_engine_weapons_normal = 0,
	_game_engine_weapons_pistols,
	_game_engine_weapons_assault_rifles,
	_game_engine_weapons_plasma_weapons,
	_game_engine_weapons_sniping,
	_game_engine_weapons_no_sniping,
	_game_engine_weapons_rocket_launchers,
	_game_engine_weapons_shotguns,
	_game_engine_weapons_short_range,
	_game_engine_weapons_human,
	_game_engine_weapons_no_grenades,
	/* port: the PC version's */
	_game_engine_weapons_covenant,
	_game_engine_weapons_classic,
	_game_engine_weapons_heavy,
	NUMBER_OF_GAME_ENGINE_WEAPON_SETS,
};

enum goal_radar
{
	_radar_motion_tracker = 0,
	_radar_nav_point,
	_radar_none,
};

enum game_engine_vehicles
{
	_game_engine_vehicles_default = 0,
	_game_engine_vehicles_none,
	_game_engine_vehicles_warthog,
	_game_engine_vehicles_ghost,
	_game_engine_vehicles_tank,
	NUMBER_OF_GAME_ENGINE_VEHICLE_SETS,
};

enum
{
	_weapon_list_assault_rifle = 0,
	_weapon_list_flamethrower,
	_weapon_list_gravity_rifle,
	_weapon_list_needler,
	_weapon_list_pistol,
	_weapon_list_plasma_pistol,
	_weapon_list_plasma_rifle,
	_weapon_list_rocket_launcher,
	_weapon_list_shotgun,
	_weapon_list_sniper_rifle,
	_weapon_list_ball,
	_weapon_list_flag,
	_weapon_list_frag_grenade,
	_weapon_list_plasma_grenade,
};

/* ---------- macros */

#define hud_globals_definition_get(index) \
	((struct hud_globals_definition *)tag_get('hudg', (index)))

#define item_collection_definition_get(index) ((struct item_collection_definition *)tag_get(ITEM_COLLECTION_DEFINITION_TAG, index))

/* ---------- structures */

struct netgame_goal
{
	real_point3d position;
	boolean in_use;
	long player_index;
	short team_index;
	long ignore_player_index;
	short nav_index;
};

struct game_engine_globals
{
	unsigned long flags;
	long next_team_index;
	real postgame_timer;
	real postgame_progress;
	long postgame_state;
	real hud_message_timers[MAXIMUM_LOCAL_PLAYERS];
};

struct game_engine_stage
{
	char map_name[64];
	struct game_variant variant;
};

struct statistic_buffer
{
	long player_index;
	long score;
	long custom;
	long kills;
	long deaths;
	long assists;
	long place;
};

typedef char verify_statistic_buffer_size[
	sizeof(struct statistic_buffer) == 0x1C ? 1 : -1];
typedef char verify_netgame_goal_size[sizeof(struct netgame_goal) == 0x20 ? 1 : -1];
typedef char verify_game_engine_globals_postgame_timer_offset[
	offsetof(struct game_engine_globals, postgame_timer) == 0x8 ? 1 : -1];
typedef char verify_game_engine_globals_next_team_index_offset[
	offsetof(struct game_engine_globals, next_team_index) == 0x4 ? 1 : -1];
typedef char verify_game_engine_globals_postgame_progress_offset[
	offsetof(struct game_engine_globals, postgame_progress) == 0xC ? 1 : -1];
typedef char verify_game_engine_globals_postgame_state_offset[
	offsetof(struct game_engine_globals, postgame_state) == 0x10 ? 1 : -1];
typedef char verify_game_engine_globals_hud_message_timers_offset[
	offsetof(struct game_engine_globals, hud_message_timers) == 0x14 ? 1 : -1];
typedef char verify_game_engine_globals_size[
	sizeof(struct game_engine_globals) == 0x24 ? 1 : -1];
typedef char verify_game_engine_stage_variant_offset[
	offsetof(struct game_engine_stage, variant) == 0x40 ? 1 : -1];
typedef char verify_game_engine_stage_size[
	sizeof(struct game_engine_stage) == 0xA8 ? 1 : -1];

/* ---------- prototypes */

void game_engine_playlist_next(
	long parameter0,
	long parameter1,
	long playlist_type);

static void game_engine_build_lighting(
	void);

static boolean is_place_tied(
	struct statistic_buffer const *entry);

int __cdecl sort_statistic_buffer(
	void const *entry0_pointer,
	void const *entry1_pointer);

int __cdecl sort_statistic_buffer_ranking(
	void const *entry0_pointer,
	void const *entry1_pointer);

static void drawline(
	wchar_t const *string,
	long row_index,
	short justification);

static boolean game_engine_infinite_grenades_internal(
	void);

static long select_players_to_display(
	enum postgame_statistic statistic,
	long player_index,
	struct statistic_buffer *output,
	long maximum_count);

static void netgame_flag_verify_no_team_duplicates(
	short flag_type,
	char const *error_message);

static void netgame_flag_verify_team_range(
	short flag_type,
	short minimum_index,
	short maximum_index,
	char const *error_message);

static void netgame_verify_spawn_points(
	short game_type,
	short unused_team_index,
	short minimum_count,
	char const *error_message);

static void netgame_verify_equipment(
	short game_type,
	char const *error_message);

boolean multiple_teams_alive(
	void);

static void game_engine_post_rasterize_in_game(
	void);

static void game_engine_rasterize_in_game_score(
	long player_index,
	real alpha);

static void game_engine_predict_resources(
	void);

static void game_engine_verify_current_map(
	void);

void game_engine_post_rasterize_post_game(
	void);

static boolean internal_rasterize_score(
	long player_index,
	long message,
	long message_data,
	wchar_t *buffer,
	long buffer_size);

static void game_engine_update_player_no_shield(
	long player_index);

static void game_engine_update_teleporter(
	long player_index);

static void game_engine_update_weapons(
	void);

static void game_engine_update_item_spawn(
	void);

boolean team_has_players(
	long team_index);

static struct statistic_buffer game_engine_get_player_place(
	long player_index);

long postgame_statistic_get_rating(
	long player_index,
	enum postgame_statistic statistic,
	boolean inverse);

long populate_statistic_buffer(
	struct statistic_buffer *statistic_buffer,
	enum postgame_statistic statistic,
	boolean inverse);

static boolean find_closest_player_callback(
	long object_index,
	void *custom_data);

static long find_closest_player_index(
	long player_index);

static void internal_rasterize_target_name(
	long player_index);

static long adjust_score_for_ranking(
	long score,
	long player_index);

/* ---------- globals */

short debug_player_color = NONE;

/* port: slayer's kill-in-order target uses the player's absolute index as
its goal index, so every player needs a goal slot */
struct netgame_goal global_goal[MAX(32, MULTIPLAYER_MAXIMUM_PLAYERS)] = { 0 };
struct game_variant global_variant = { 0 };
struct game_engine *game_engine = NULL;

extern struct game_engine_globals game_engine_globals;
extern struct game_engine_stage global_stage;
extern long timeout_for_endgame_sound;

/* whether a client has had the host's game type state this game (what
changes in the first, from this machine's own start, it only takes: the
game types show what changes in the next) */
static boolean game_engine_network_state_read = FALSE;

/* ---------- public code */

long game_globals_get_weapon(
	struct game_globals *game_globals,
	long weapon_list_index)
{
	struct tag_reference *weapon;
	long weapon_definition_index;

	/* port: none past the end of the list. A Custom Edition map's globals can
	list fewer weapons than the Xbox's fourteen (stopping before the
	grenades). */
	if (weapon_list_index < 0 || weapon_list_index >= game_globals->weapon_list.count)
		return NONE;
	weapon = TAG_BLOCK_GET_ELEMENT(
		&game_globals->weapon_list,
		weapon_list_index,
		struct tag_reference);
	weapon_definition_index = weapon->index;

	return weapon_definition_index;
}

long game_engine_get_team_score(
	long team_index)
{
	struct data_iterator iterator;
	struct player_datum *player;

	data_iterator_new(&iterator, player_data);
	player = (struct player_datum *)data_iterator_next(&iterator);
	while (player)
	{
		if (player->team_index == team_index)
			return game_engine->get_player_score(iterator.datum_index, TRUE);

		player = (struct player_datum *)data_iterator_next(&iterator);
	}

	return 0;
}

real linear_to_non_linear_alpha(
	real linear_alpha)
{
	real non_linear_alpha = (real)pow((double)linear_alpha, 1.9f);

	return non_linear_alpha;
}

static void initialize_player_multiplayer_data(
	long player_index)
{
	struct player_datum *player = player_get(player_index);

	player->state_message = NONE;
	player->state_message_player_index = NONE;
	player->speed_multiplier = 1.0f;
	player->teleporter_index = NONE;
	player->player_display_index = NONE;
	csmemset(
		&player->statistics.multiplayer_statistics,
		0,
		sizeof(long));

	return;
}

/* port: English ordinal for a zero-based place past the string list's 16
("17th", "22nd", "111th"; "tied for 17th" when tied), in a static buffer */
static wchar_t *place_ordinal_string(
	long place,
	boolean tied)
{
	static wchar_t string[32];
	long number = place + 1;
	wchar_t const *suffix = L"th";

	if (number % 100 < 11 || number % 100 > 13)
	{
		switch (number % 10)
		{
		case 1:
			suffix = L"st";
			break;
		case 2:
			suffix = L"nd";
			break;
		case 3:
			suffix = L"rd";
			break;
		}
	}

	usnprintf(
		string,
		NUMBEROF(string),
		tied ? L"tied for %d%s" : L"%d%s",
		number,
		suffix);
	string[NUMBEROF(string) - 1] = 0;

	return string;
}

static wchar_t *get_place_string(
	struct statistic_buffer *entry)
{
	long string_index = PIN(entry->place & 0x7F, 0, 15);
	long string_list_index;

	/* port: places past the 16th are spelled out instead of clamped (the
	caller's format string marks a tie) */
	if ((entry->place & 0x7F) >= NUMBER_OF_PLACE_STRINGS)
		return place_ordinal_string(entry->place & 0x7F, FALSE);
	string_list_index = tag_loaded('ustr', "ui\\multiplayer_game_text");
	if (string_list_index != NONE)
		return unicode_string_list_get_string(
			string_list_index,
			string_index + 36);

	return L"";
}

static long game_engine_did_team_win(
	long team_index)
{
	struct data_iterator iterator;
	struct player_datum *player;
	long result = FALSE;

	data_iterator_new(&iterator, player_data);
	player = (struct player_datum *)data_iterator_next(&iterator);
	while (player)
	{
		if (player->team_index == team_index)
		{
			result = FALSE;
			if (game_engine)
			{
				if (game_engine->did_player_win)
					result =
						game_engine->did_player_win(iterator.datum_index);
				else
					result = game_engine_did_player_win_default(
						iterator.datum_index);
			}
			break;
		}

		player = (struct player_datum *)data_iterator_next(&iterator);
	}

	return result;
}



static void game_engine_generate_title_string(
	wchar_t *title_string,
	long player_index)
{
	wchar_t *secondary_string;
	wchar_t life_string[128];
	wchar_t time_left_string[160];
	wchar_t score_string[256];
	long string_list_index;
	wchar_t *format_string;

	player_get(player_index);
	secondary_string = L"";
	match_assert(
		"c:\\halo\\SOURCE\\game\\game_engine.c",
		0x36E,
		title_string);
	if (global_variant.universal_variant.lives > 0)
	{
		struct player_datum *player = player_get(player_index);
		long remaining_lives =
			global_variant.universal_variant.lives - player->statistics.deaths;

		switch (remaining_lives)
		{
		case 0:
			string_list_index =
				tag_loaded('ustr', "ui\\multiplayer_game_text");
			if (string_list_index != NONE)
			{
				secondary_string = unicode_string_list_get_string(
					string_list_index,
					0x34);
			}
			else
				secondary_string = L"";
			break;

		case 1:
			string_list_index =
				tag_loaded('ustr', "ui\\multiplayer_game_text");
			if (string_list_index != NONE)
			{
				secondary_string = unicode_string_list_get_string(
					string_list_index,
					0x35);
			}
			else
				secondary_string = L"";
			break;

		default:
			string_list_index =
				tag_loaded('ustr', "ui\\multiplayer_game_text");
			if (string_list_index != NONE)
			{
				format_string = unicode_string_list_get_string(
					string_list_index,
					0x36);
			}
			else
				format_string = L"";

			usnprintf(
				life_string,
				NUMBEROF(life_string),
				ustring_format_checked(format_string, "d"),
				remaining_lives);
			life_string[NUMBEROF(life_string) - 1] = 0;
			secondary_string = life_string;
			break;
		}
	}

	/* port: the gametype's time limit's time left (game_variant_options) */
	if (game_variant_options_get()->time_limit > 0 &&
		game_engine_globals.postgame_state == game_engine_mode_active)
	{
		/* port: Infection's round's time starts after its countdown */
		long left = infection_enabled() ? infection_remaining_ticks() :
			game_variant_options_get()->time_limit * 60L * TICKS_PER_SECOND - game_time_get();
		wchar_t time_string[32];

		ticks_to_unicode_time_string(MAX(left, 0), NUMBEROF(time_string), time_string);
		if (secondary_string[0] && secondary_string != time_left_string)
		{
			usnprintf(time_left_string, NUMBEROF(time_left_string), L"%s, %s left", secondary_string, time_string);
		}
		else
			usnprintf(time_left_string, NUMBEROF(time_left_string), L"%s left", time_string);
		time_left_string[NUMBEROF(time_left_string) - 1] = 0;
		secondary_string = time_left_string;
	}

	if (game_engine_globals.postgame_state == game_engine_mode_postgame_delay)
	{
		long did_player_win = game_engine_did_player_win(player_index);
		boolean has_teams = game_engine_has_teams();
		wchar_t *outcome_string;

		switch (did_player_win)
		{
		case NONE:
			string_list_index =
				tag_loaded('ustr', "ui\\multiplayer_game_text");
			if (string_list_index != NONE)
			{
				outcome_string = unicode_string_list_get_string(
					string_list_index,
					0x37);
			}
			else
				outcome_string = L"";
			ustrncpy(title_string, outcome_string, 80);
			break;

		case FALSE:
			if (has_teams)
			{
				string_list_index =
					tag_loaded('ustr', "ui\\multiplayer_game_text");
				if (string_list_index != NONE)
				{
					outcome_string = unicode_string_list_get_string(
						string_list_index,
						0x38);
				}
				else
					outcome_string = L"";
				ustrncpy(title_string, outcome_string, 80);
			}
			else
			{
				string_list_index =
					tag_loaded('ustr', "ui\\multiplayer_game_text");
				if (string_list_index != NONE)
				{
					outcome_string = unicode_string_list_get_string(
						string_list_index,
						0x39);
				}
				else
					outcome_string = L"";
				ustrncpy(title_string, outcome_string, 80);
			}
			break;

		case TRUE:
			if (has_teams)
			{
				string_list_index =
					tag_loaded('ustr', "ui\\multiplayer_game_text");
				if (string_list_index != NONE)
				{
					outcome_string = unicode_string_list_get_string(
						string_list_index,
						0x3A);
				}
				else
					outcome_string = L"";
				ustrncpy(title_string, outcome_string, 80);
			}
			else
			{
				string_list_index =
					tag_loaded('ustr', "ui\\multiplayer_game_text");
				if (string_list_index != NONE)
				{
					outcome_string = unicode_string_list_get_string(
						string_list_index,
						0x3B);
				}
				else
					outcome_string = L"";
				ustrncpy(title_string, outcome_string, 80);
			}
			break;
		}
	}
	else if (game_engine_has_teams())
	{
		wchar_t team0_name[8];
		wchar_t team1_name[8];
		long team0_score;
		long team1_score;

		game_engine->format_team_name(
			0,
			team0_name);
		game_engine->format_team_name(
			1,
			team1_name);
		team0_score = game_engine_get_team_score(0);
		team1_score = game_engine_get_team_score(1);

		if (team0_score > team1_score)
		{
			string_list_index =
				tag_loaded('ustr', "ui\\multiplayer_game_text");
			if (string_list_index != NONE)
			{
				format_string =
					unicode_string_list_get_string(string_list_index, 0x3C);
			}
			else
				format_string = L"";

			usnprintf(
				title_string,
				80,
				ustring_format_checked(format_string, "sss"),
				team0_name,
				team1_name,
				secondary_string);
		}
		else if (team0_score < team1_score)
		{
			string_list_index =
				tag_loaded('ustr', "ui\\multiplayer_game_text");
			if (string_list_index != NONE)
			{
				format_string =
					unicode_string_list_get_string(string_list_index, 0x3D);
			}
			else
				format_string = L"";

			usnprintf(
				title_string,
				80,
				ustring_format_checked(format_string, "sss"),
				team1_name,
				team0_name,
				secondary_string);
		}
		else
		{
			string_list_index =
				tag_loaded('ustr', "ui\\multiplayer_game_text");
			if (string_list_index != NONE)
			{
				format_string =
					unicode_string_list_get_string(string_list_index, 0x3E);
			}
			else
				format_string = L"";

			usnprintf(
				title_string,
				80,
				ustring_format_checked(format_string, "ss"),
				team1_name,
				secondary_string);
		}
	}
	else
	{
		struct statistic_buffer entry;

		entry = game_engine_get_player_place(player_index);
		game_engine->format_player_score(player_index, score_string);

		if (is_place_tied(&entry))
		{
			string_list_index =
				tag_loaded('ustr', "ui\\multiplayer_game_text");
			if (string_list_index != NONE)
			{
				format_string = unicode_string_list_get_string(
					string_list_index,
					0x3F);
			}
			else
				format_string = L"";

			usnprintf(
				title_string,
				80,
				ustring_format_checked(format_string, "sss"),
				get_place_string(&entry),
				score_string,
				secondary_string);
		}
		else
		{
			string_list_index =
				tag_loaded('ustr', "ui\\multiplayer_game_text");
			if (string_list_index != NONE)
			{
				format_string = unicode_string_list_get_string(
					string_list_index,
					0x40);
			}
			else
				format_string = L"";

			usnprintf(
				title_string,
				80,
				ustring_format_checked(format_string, "sss"),
				get_place_string(&entry),
				score_string,
				secondary_string);
		}
	}

	title_string[79] = 0;

	return;
}

static void rasterize_in_game_score_draw_line(
	wchar_t const *string,
	boolean brighten,
	real_argb_color *color,
	long row_index)
{
	rectangle2d bounds = render.camera.window_bounds;
	short narrow_tab_stops[3];
	short wide_tab_stops[3];
	short *tab_stops;
	boolean splitscreen;
	long font_index;

	splitscreen = local_player_count() > 1;
	font_index = hud_get_font_index();
	narrow_tab_stops[0] = 80;
	narrow_tab_stops[1] = 125;
	narrow_tab_stops[2] = 200;
	wide_tab_stops[0] = 130;
	wide_tab_stops[1] = 195;
	wide_tab_stops[2] = 315;

	if (bounds.x1 - bounds.x0 > 320)
		tab_stops = wide_tab_stops;
	else
		tab_stops = narrow_tab_stops;

	if (row_index)
		draw_string_set_tab_stops(tab_stops, 3);
	else
		draw_string_set_tab_stops(NULL, 0);

	offset_rectangle2d(
		&bounds,
		-render.camera.viewport_bounds.x0,
		-render.camera.viewport_bounds.y0);

	if (font_index != NONE)
	{
		struct font_header *font = font_definition_get(font_index);
		long row_offset = 4 * (splitscreen == FALSE) + 4;
		long line_height = font->leading_height;

		if (!splitscreen)
			line_height += font->descending_height;
		line_height += font->ascending_height;

		if (brighten)
		{
			color->red += 0.4f;
			color->green += 0.4f;
			color->blue += 0.4f;

			if (color->red > 1.0f)
				color->red = 1.0f;
			if (color->green > 1.0f)
				color->green = 1.0f;
			if (color->blue > 1.0f)
				color->blue = 1.0f;
		}

		row_index += row_offset;
		bounds.y0 = (short)(row_index * line_height);
		bounds.y1 = (short)((row_index + 1) * line_height);
		draw_string_set_draw_mode(font_index, NONE, 0, 0, color);
		rasterizer_draw_unicode_string(&bounds, NULL, NULL, 0, string);
	}

	draw_string_set_tab_stops(NULL, 0);

	return;
}

/* port: the in-game scoreboard's lists (game_engine_rasterize_scoreboard,
game_engine_rasterize_in_game_score): the players in the game, not those who
quit, ranked and placed among themselves */
static boolean statistic_buffer_in_game_only = FALSE;

long populate_statistic_buffer(
	struct statistic_buffer *statistic_buffer,
	enum postgame_statistic statistic,
	boolean inverse)
{
	long player_count = 0;
	boolean invert = statistic == _postgame_statistic_deaths ? !inverse : inverse;
	struct data_iterator player_iterator;
	struct player_datum *player;
	long statistic_index;

	data_iterator_new(&player_iterator, player_data);
	while ((player = (struct player_datum *)data_iterator_next(&player_iterator)) != NULL)
	{
		match_assert(
			"c:\\halo\\SOURCE\\game\\game_engine.c",
			0x2C8,
			player_count < MULTIPLAYER_MAXIMUM_PLAYERS);
		if (statistic_buffer_in_game_only && player->quit_out_of_game)
			continue;
		if (player_count < MULTIPLAYER_MAXIMUM_PLAYERS)
		{
			statistic_buffer[player_count].player_index = player_iterator.datum_index;
			player_count++;
		}
	}

	for (statistic_index = 0; statistic_index < player_count; statistic_index++)
	{
		long player_index = statistic_buffer[statistic_index].player_index;

		player = player_get(player_index);
		switch (statistic)
		{
		case _postgame_statistic_ranking:
			statistic_buffer[statistic_index].custom = 0;
			if (game_engine->get_player_score)
			{
				statistic_buffer[statistic_index].custom = adjust_score_for_ranking(
					game_engine->get_player_score(player_index, FALSE),
					player_index);
			}
			statistic_buffer[statistic_index].kills = player->statistics.kills[0];
			statistic_buffer[statistic_index].assists = player->statistics.assists[0];
			statistic_buffer[statistic_index].deaths = player->statistics.deaths;
			break;

		case _postgame_statistic_score:
			statistic_buffer[statistic_index].score = 0;
			if (game_engine->get_player_score)
			{
				statistic_buffer[statistic_index].score = adjust_score_for_ranking(
					game_engine->get_player_score(player_index, FALSE),
					player_index);
			}
			break;

		case _postgame_statistic_kills:
			statistic_buffer[statistic_index].score = player->statistics.kills[0];
			break;

		case _postgame_statistic_assists:
			statistic_buffer[statistic_index].score = player->statistics.assists[0];
			break;

		case _postgame_statistic_deaths:
			statistic_buffer[statistic_index].score = player->statistics.deaths;
			break;

		default:
			match_assert(
				"c:\\halo\\SOURCE\\game\\game_engine.c",
				0x2FD,
				!"unreachable");
			break;
		}

		if (invert)
			statistic_buffer[statistic_index].score = -statistic_buffer[statistic_index].score;
	}

	if (statistic == _postgame_statistic_ranking)
	{
		qsort(
			statistic_buffer,
			player_count,
			sizeof(*statistic_buffer),
			sort_statistic_buffer_ranking);
	}
	else
	{
		qsort(
			statistic_buffer,
			player_count,
			sizeof(*statistic_buffer),
			sort_statistic_buffer);
	}

	for (statistic_index = 0; statistic_index < player_count; statistic_index++)
	{
		if (statistic_index != 0 &&
			sort_statistic_buffer_ranking(
				&statistic_buffer[statistic_index - 1],
				&statistic_buffer[statistic_index]) == 0)
		{
			statistic_buffer[statistic_index - 1].place |= 0x80000000;
			statistic_buffer[statistic_index].place = statistic_buffer[statistic_index - 1].place;
		}
		else
		{
			statistic_buffer[statistic_index].place = statistic_index;
		}
	}

	return player_count;
}






static long select_players_to_display(
	enum postgame_statistic statistic,
	long player_index,
	struct statistic_buffer *output,
	long maximum_count)
{
	struct statistic_buffer statistic_buffer[MULTIPLAYER_MAXIMUM_PLAYERS];
	long player_count = populate_statistic_buffer(statistic_buffer, statistic, FALSE);
	boolean debug = rasterizer_debug_options.pad3 == 'E';
	long local_player_count = 0;
	long statistic_index;

	if (debug)
	{
		terminal_printf(
			global_real_argb_white,
			"player_count=%d, maxcount=%d",
			player_count,
			maximum_count);
	}

	for (statistic_index = 0; statistic_index < player_count; statistic_index++)
	{
		struct player_datum *player = player_get(statistic_buffer[statistic_index].player_index);

		if (player->local_player_index != NONE)
			local_player_count++;
	}

	if (player_count > maximum_count)
	{
		long outside_range_count = 0;
		struct statistic_buffer outside_range[MAXIMUM_LOCAL_PLAYERS];
		long outside_range_index;

		for (statistic_index = maximum_count; statistic_index < player_count; statistic_index++)
		{
			struct player_datum *player = player_get(statistic_buffer[statistic_index].player_index);

			if (player && player->local_player_index != NONE)
			{
				if (debug)
				{
					terminal_printf(
						global_real_argb_white,
						"found local player");
				}

				outside_range[outside_range_count] = statistic_buffer[statistic_index];
				outside_range_count++;
			}
		}

		for (outside_range_index = 0; outside_range_index < outside_range_count; outside_range_index++)
		{
			long insertion_index;

			for (insertion_index = maximum_count - 1; insertion_index >= 0; insertion_index--)
			{
				struct player_datum *player = player_get(statistic_buffer[insertion_index].player_index);

				if (player->local_player_index == NONE)
				{
					csmemmove(
						&statistic_buffer[insertion_index],
						&statistic_buffer[insertion_index + 1],
						maximum_count * sizeof(struct statistic_buffer) -
							(insertion_index + 1) * sizeof(struct statistic_buffer));
					statistic_buffer[maximum_count - 1] = outside_range[outside_range_index];
					break;
				}
			}
		}
	}

	csmemcpy(
		output,
		statistic_buffer,
		MIN(maximum_count, player_count) *
			sizeof(struct statistic_buffer));
	return MIN(maximum_count, player_count);
}

/* port: network co-op's scoreboard list (game_engine_rasterize_scoreboard):
the players in the game, as they joined */
static long populate_campaign_player_buffer(
	struct statistic_buffer *statistic_buffer)
{
	struct data_iterator iterator;
	struct player_datum *player;
	long player_count = 0;

	data_iterator_new(&iterator, player_data);
	while ((player = data_iterator_next(&iterator)) != NULL && player_count < MULTIPLAYER_MAXIMUM_PLAYERS)
	{
		if (player->quit_out_of_game)
			continue;
		csmemset(&statistic_buffer[player_count], 0, sizeof(*statistic_buffer));
		statistic_buffer[player_count++].player_index = iterator.datum_index;
	}

	return player_count;
}

/* port: the scoreboard of a full-screen view (game_engine_rasterize_in_game_score;
a split-screen view's keeps the Xbox's six rows): SCOREBOARD_SCALE times the
HUD's text, centred, on a panel (display.scoreboard_background). A team game's
players in a column for each team (display.scoreboard_team_layout), else in
order of score in one column, or two when one has too few rows; each with
the player's ping in a network game (the host's measure:
network_distributed.c). More players than a page are scrolled to with the
mouse wheel and Page Up/Down (platform_scoreboard_scroll), a footer telling
which are shown; opened, it shows the viewer's own player's page. Network
co-op's campaign, with no game engine, lists its players as they joined,
with only their names and pings. */
enum
{
	/* the rows' widths (in the scoreboard's text, before it is scaled):
	place, name, score, ping (the last as wide as "Ping" or three digits,
	so that the columns centred are their text centred) */
	SCOREBOARD_PLACE_WIDTH = 55,
	SCOREBOARD_NAME_WIDTH = 150,
	SCOREBOARD_SCORE_WIDTH = 95,
	SCOREBOARD_PING_WIDTH = 40,
	SCOREBOARD_COLUMN_WIDTH = SCOREBOARD_PLACE_WIDTH + SCOREBOARD_NAME_WIDTH + SCOREBOARD_SCORE_WIDTH + SCOREBOARD_PING_WIDTH,
	SCOREBOARD_COLUMN_GAP = 40,
	/* the rows a column has: as many as fit between 6 rows of the screen
	from its top and 2 from its bottom (the motion sensor); the scoreboard
	is centred on the screen, but never nearer its top than 4 rows (the
	HUD's shields and health; its messages, such as "hold BACK for score",
	are hidden while the scoreboard shows) */
	SCOREBOARD_LAYOUT_TOP_ROWS = 6,
	SCOREBOARD_BOTTOM_ROWS = 2,
	SCOREBOARD_MINIMUM_TOP_ROWS = 4,
	/* the entries (or a team column's rows) a notch of the wheel scrolls */
	SCOREBOARD_WHEEL_STEP = 3,
};

#define SCOREBOARD_SCALE 0.75f

/* network_distributed.c's */
long distributed_player_ping(short player_index);
/* port_config.c's */
int config_boolean(const char *name);
const char *config_string(const char *name);
unsigned long config_changes(void);
/* cinematics.c's */
void draw_quad(rectangle2d *rectangle, pixel32 color);

/* display.scoreboard_background(_color): the panel behind the scoreboard's
text, its colour "red, green, blue, alpha" (0 to 255 each), or 0 for none */
static pixel32 scoreboard_background_color(
	void)
{
	static unsigned long read_at = (unsigned long)-1;
	static pixel32 color = 0;

	/* (read again when Settings changes it) */
	if (read_at != config_changes())
	{
		char const *text = config_string("display.scoreboard_background_color");
		long parts[4] = { 16, 16, 16, 150 };
		short part;

		read_at = config_changes();
		color = 0;
		if (!config_boolean("display.scoreboard_background"))
			return color;
		for (part = 0; part < 4 && text && *text; part++)
		{
			long value = 0;
			boolean digits = FALSE;

			while (*text == ' ' || *text == ',')
				text++;
			while (*text >= '0' && *text <= '9')
			{
				value = value * 10 + (*text++ - '0');
				digits = TRUE;
			}
			if (digits)
				parts[part] = PIN(value, 0, 255);
			while (*text && *text != ',')
				text++;
		}
		color = ((pixel32)parts[3] << 24) | ((pixel32)parts[0] << 16) | ((pixel32)parts[1] << 8) | (pixel32)parts[2];
	}

	return color;
}

/* a row of the scoreboard: its text (tab separated) from the column's left
(tabs: the column's stops), on the row below the title's (at top) */
static void scoreboard_draw_row(
	wchar_t const *string,
	boolean brighten,
	real_argb_color const *row_color,
	long row_index,
	short top,
	short left,
	boolean tabs)
{
	rectangle2d bounds = render.camera.window_bounds;
	long font_index = hud_get_font_index();
	/* (brightened as a copy: the team colours serve every row) */
	real_argb_color color = *row_color;
	short tab_stops[4];
	struct font_header *font;
	long line_height;

	if (font_index == NONE)
		return;
	offset_rectangle2d(&bounds, -render.camera.viewport_bounds.x0, -render.camera.viewport_bounds.y0);
	font = font_definition_get(font_index);
	line_height = font->leading_height + font->descending_height + font->ascending_height;
	if (brighten)
	{
		color.red = MIN(color.red + 0.4f, 1.0f);
		color.green = MIN(color.green + 0.4f, 1.0f);
		color.blue = MIN(color.blue + 0.4f, 1.0f);
	}
	tab_stops[0] = left;
	tab_stops[1] = (short)(left + SCOREBOARD_PLACE_WIDTH);
	tab_stops[2] = (short)(tab_stops[1] + SCOREBOARD_NAME_WIDTH);
	tab_stops[3] = (short)(tab_stops[2] + SCOREBOARD_SCORE_WIDTH);
	draw_string_set_tab_stops(tabs ? tab_stops : NULL, tabs ? 4 : 0);
	/* (the row as wide as the screen once scaled) */
	bounds.x1 = (short)(bounds.x0 + (bounds.x1 - bounds.x0) / SCOREBOARD_SCALE);
	if (!tabs)
		bounds.x0 = left;
	bounds.y0 = (short)(top + row_index * line_height);
	bounds.y1 = (short)(bounds.y0 + line_height);
	draw_string_set_draw_mode(font_index, NONE, 0, 0, &color);
	rasterizer_draw_unicode_string(&bounds, NULL, NULL, 0, string);
	draw_string_set_tab_stops(NULL, 0);

	return;
}

/* sdl_platform.c's: the wheel's and Page Up/Down's moves of the open
scoreboard (notches and pages, down positive) */
void platform_scoreboard_scroll(int open, long *notches, long *pages);

/* the scoreboard's scroll (entries of one list, or rows of the team
columns), and whether it showed last frame (opened, it shows the viewer's
own player) */
static long scoreboard_scroll = 0;
static boolean scoreboard_open = FALSE;

/* display.scoreboard_team_layout: a team game's players in a column for each
team ("teams", the red team's on the left), or all in order of score
("score") */
static boolean scoreboard_team_columns(
	void)
{
	static short setting = NONE;
	static unsigned long read_at = (unsigned long)-1;

	if (read_at != config_changes())
	{
		char const *value = config_string("display.scoreboard_team_layout");

		read_at = config_changes();
		setting = value && !csstrcmp(value, "score") ? FALSE : TRUE;
	}

	return (boolean)setting;
}

/* a network game's scoreboard picks players with the mouse
(halo_scoreboard_pointer_update: a right click frees it): a player's name
picked opens a menu of what may be done to them, the players' vote to kick
them (network_votekick.c), muting their voice and text chat on this
machine (network_voice.c, network_chat.c), and the host's Mute for everyone
(network_chat.c), Kick and Ban, Ban picked twice */
enum
{
	_scoreboard_item_vote,
	_scoreboard_item_mute,
	_scoreboard_item_host_mute,
	_scoreboard_item_kick,
	_scoreboard_item_ban,
	_scoreboard_item_cancel,
	NUMBER_OF_SCOREBOARD_ITEMS,

	/* the menu's width (the scoreboard's text, before it is scaled) */
	SCOREBOARD_MENU_WIDTH = 220,
};

/* the menu: the player it is for (NONE: none open), the row under whose
name it opens (from the title's), that name's left, and whether Ban was
picked once */
static struct
{
	long player_index;
	long row_index;
	short left;
	boolean ban_picked;
} scoreboard_menu = { NONE, 0, 0, FALSE };

/* the scoreboard closed: its scroll forgotten, the wheel the weapons' again */
static void game_engine_scoreboard_closed(
	void)
{
	if (scoreboard_open)
	{
		struct halo_ui_pointer pointer;

		scoreboard_open = FALSE;
		platform_scoreboard_scroll(FALSE, NULL, NULL);
		halo_scoreboard_pointer_update(FALSE, &pointer);
	}
	scoreboard_menu.player_index = NONE;
}

/* the menu's items for the player: whether each is offered, and what it says */
static void scoreboard_menu_items(
	wchar_t (*labels)[48],
	boolean *offered)
{
	struct network_votekick_status status;
	short item;

	for (item = 0; item < NUMBER_OF_SCOREBOARD_ITEMS; item++)
	{
		offered[item] = FALSE;
		labels[item][0] = 0;
	}
	offered[_scoreboard_item_vote] = TRUE;
	if (network_votekick_get_status(&status) && status.kind == _network_vote_kick &&
		status.player_index == DATUM_INDEX_TO_ABSOLUTE_INDEX(scoreboard_menu.player_index))
	{
		usprintf(labels[_scoreboard_item_vote], status.voted ? L"Voted to kick (%d of %d)" :
			status.voted_no ? L"Voted not to kick (%d of %d)" : L"Vote to kick (%d of %d)", status.votes,
			status.needed);
	}
	else
	{
		usprintf(labels[_scoreboard_item_vote], L"Start a vote to kick");
	}
	/* (its machine's voice and text chat lines: this machine's own choice) */
	{
		struct player_datum *player = player_try_and_get(scoreboard_menu.player_index);

		if (player)
		{
			offered[_scoreboard_item_mute] = TRUE;
			usprintf(labels[_scoreboard_item_mute],
				network_voice_machine_muted(player->network_player_data.machine_index) ? L"Unmute voice and chat" :
					L"Mute voice and chat");
		}
	}
	if (network_votekick_host())
	{
		/* (the host's mute of their voice and lines, for every machine:
		network_chat.c) */
		offered[_scoreboard_item_host_mute] = TRUE;
		usprintf(labels[_scoreboard_item_host_mute],
			network_chat_host_mute_minutes((short)DATUM_INDEX_TO_ABSOLUTE_INDEX(scoreboard_menu.player_index)) ?
				L"Unmute for everyone" : L"Mute for everyone");
		offered[_scoreboard_item_kick] = TRUE;
		offered[_scoreboard_item_ban] = TRUE;
		usprintf(labels[_scoreboard_item_kick], L"Kick");
		usprintf(labels[_scoreboard_item_ban], scoreboard_menu.ban_picked ? L"Ban: pick again to ban" : L"Ban");
	}
	offered[_scoreboard_item_cancel] = TRUE;
	usprintf(labels[_scoreboard_item_cancel], L"Cancel");
}

/* the menu's first row (from the title's): under the name, or above it when
there is no room below (last_row: the screen's) */
static long scoreboard_menu_first_row(
	short item_count,
	long last_row)
{
	if (scoreboard_menu.row_index + 1 + item_count <= last_row)
		return scoreboard_menu.row_index + 1;
	return MAX(scoreboard_menu.row_index - item_count, 0);
}

/* a rectangle of the scoreboard's text (laid out at full size, from left
and the row from the title's) as it is drawn: scaled about the title's top
left (bounds_left, top) */
static void scoreboard_rectangle(
	rectangle2d *rectangle,
	short bounds_left,
	short top,
	long line_height,
	short left,
	short width,
	long row_index,
	long row_count)
{
	rectangle->x0 = (short)(bounds_left + (left - bounds_left) * SCOREBOARD_SCALE);
	rectangle->x1 = (short)(rectangle->x0 + width * SCOREBOARD_SCALE);
	rectangle->y0 = (short)(top + row_index * line_height * SCOREBOARD_SCALE);
	rectangle->y1 = (short)(top + (row_index + row_count) * line_height * SCOREBOARD_SCALE);
}

static boolean scoreboard_point_in(
	rectangle2d const *rectangle,
	short x,
	short y)
{
	return x >= rectangle->x0 && x < rectangle->x1 && y >= rectangle->y0 && y < rectangle->y1;
}

/* the item picked, done */
static void scoreboard_menu_pick(
	short item)
{
	short player_index = (short)DATUM_INDEX_TO_ABSOLUTE_INDEX(scoreboard_menu.player_index);

	switch (item)
	{
	case _scoreboard_item_vote:
		network_votekick_request(player_index);
		break;
	case _scoreboard_item_mute:
	{
		struct player_datum *player = player_try_and_get(scoreboard_menu.player_index);

		if (player)
		{
			long machine_index = player->network_player_data.machine_index;

			network_voice_mute_machine(machine_index, !network_voice_machine_muted(machine_index));
		}
		break;
	}
	case _scoreboard_item_host_mute:
		network_chat_host_mute(player_index, !network_chat_host_mute_minutes(player_index));
		break;
	case _scoreboard_item_kick:
		network_votekick_host_kick(player_index, FALSE);
		break;
	case _scoreboard_item_ban:
		/* (a ban lasts: picked twice) */
		if (!scoreboard_menu.ban_picked)
		{
			scoreboard_menu.ban_picked = TRUE;
			return;
		}
		network_votekick_host_kick(player_index, TRUE);
		break;
	default:
		break;
	}
	scoreboard_menu.player_index = NONE;
}

static void game_engine_rasterize_scoreboard(
	long player_index,
	real alpha)
{
	struct statistic_buffer ranked[MULTIPLAYER_MAXIMUM_PLAYERS];
	/* (the players of each column's list, as indices into ranked: a team
	game's by team, else all in one list over both columns) */
	short lists[2][MULTIPLAYER_MAXIMUM_PLAYERS];
	long list_counts[2] = { 0, 0 };
	wchar_t row_string[256];
	wchar_t score_string[256];
	wchar_t title_string[80];
	wchar_t ping_string[16];
	real_argb_color text_color;
	real_argb_color team_colors[2];
	real_argb_color color;
	rectangle2d bounds = render.camera.window_bounds;
	boolean has_teams = game_engine_has_teams();
	boolean network = game_connection() == _game_connection_network_client ||
		game_connection() == _game_connection_network_server;
	boolean campaign = !game_engine;
	boolean team_columns;
	long font_index = hud_get_font_index();
	long string_list_index;
	long line_height;
	long rows;
	long columns;
	long ranked_count;
	long total;
	long page;
	long shown_rows;
	long index;
	short width;
	short left;
	short top;
	wchar_t *column_name;
	wchar_t *score_name;
	/* port: the pointer (1: picking; -1: none on this platform), where it
	is in the drawing's coordinates, and whether its click was taken */
	struct halo_ui_pointer pointer;
	int pointer_state;
	short pointer_x;
	short pointer_y;
	short click_x;
	short click_y;
	boolean clicked;
	struct network_votekick_status vote;
	boolean voting;
	long extra_rows;
	long next_row;
	wchar_t menu_labels[NUMBER_OF_SCOREBOARD_ITEMS][48];
	boolean menu_offered[NUMBER_OF_SCOREBOARD_ITEMS];
	short menu_item_count = 0;
	long menu_first_row = 0;
	long last_row;

	if (font_index == NONE)
		return;
	offset_rectangle2d(&bounds, -render.camera.viewport_bounds.x0, -render.camera.viewport_bounds.y0);
	{
		struct font_header *font = font_definition_get(font_index);

		line_height = font->leading_height + font->descending_height + font->ascending_height;
	}
	if (line_height <= 0)
		return;
	/* (laid out at full size, then drawn scaled about the title's top left:
	the screen holds 1/SCOREBOARD_SCALE as much) */
	width = (short)((bounds.x1 - bounds.x0) / SCOREBOARD_SCALE);
	/* (port: a network game's lines beneath, the vote running and how to
	pick a player, have their rows) */
	pointer_state = halo_scoreboard_pointer_update(network && network_votekick_available(), &pointer);
	voting = network && network_votekick_get_status(&vote);
	extra_rows = (voting ? 1 : 0) + (network && pointer_state >= 0 ? 1 : 0);
	rows = (long)((bounds.y1 - SCOREBOARD_LAYOUT_TOP_ROWS * line_height) / SCOREBOARD_SCALE / line_height) - 2 -
		SCOREBOARD_BOTTOM_ROWS - extra_rows;
	rows = MAX(rows, 1);
	if (campaign)
	{
		ranked_count = populate_campaign_player_buffer(ranked);
	}
	else
	{
		statistic_buffer_in_game_only = TRUE;
		ranked_count = populate_statistic_buffer(ranked, _postgame_statistic_ranking, FALSE);
		statistic_buffer_in_game_only = FALSE;
	}
	team_columns = has_teams && scoreboard_team_columns() && width >= 2 * SCOREBOARD_COLUMN_WIDTH + SCOREBOARD_COLUMN_GAP;
	for (index = 0; index < ranked_count; index++)
	{
		struct player_datum *player = player_try_and_get(ranked[index].player_index);
		short list = team_columns && player ? (short)PIN(player->team_index, 0, 1) : 0;

		lists[list][list_counts[list]++] = (short)index;
	}
	/* (a page: the rows of both team columns, or both columns of one list;
	the scroll in rows of the team columns, or entries of the list) */
	if (team_columns)
	{
		columns = 2;
		total = MAX(list_counts[0], list_counts[1]);
		page = rows;
	}
	else
	{
		columns = list_counts[0] > rows && width >= 2 * SCOREBOARD_COLUMN_WIDTH + SCOREBOARD_COLUMN_GAP ? 2 : 1;
		total = list_counts[0];
		page = rows * columns;
	}
	/* (opened, at the viewer's own player's page; then where the wheel and
	Page Up/Down take it) */
	{
		long notches = 0;
		long pages = 0;

		platform_scoreboard_scroll(TRUE, &notches, &pages);
		if (!scoreboard_open)
		{
			scoreboard_open = TRUE;
			scoreboard_scroll = 0;
			for (index = 0; index < 2; index++)
			{
				long position;

				for (position = 0; position < list_counts[index]; position++)
				{
					if (ranked[lists[index][position]].player_index == player_index)
						scoreboard_scroll = position / page * page;
				}
			}
		}
		scoreboard_scroll += notches * SCOREBOARD_WHEEL_STEP + pages * page;
		scoreboard_scroll = PIN(scoreboard_scroll, 0, MAX(total - page, 0));
	}
	left = (short)(bounds.x0 + (width - (columns * SCOREBOARD_COLUMN_WIDTH + (columns - 1) * SCOREBOARD_COLUMN_GAP)) / 2);
	/* (centred on the rows shown: the title's, the heading's, the longest
	column's, and the footer telling where the scroll is) */
	shown_rows = MIN(rows, total);
	if (total > page)
		shown_rows++;
	shown_rows += extra_rows;
	{
		real height = (2 + shown_rows) * line_height * SCOREBOARD_SCALE;

		top = (short)(bounds.y0 + ((bounds.y1 - bounds.y0) - height) / 2);
		top = MAX(top, (short)(SCOREBOARD_MINIMUM_TOP_ROWS * line_height));
		/* (the panel behind it, half a row beyond its text, fading with it) */
		{
			pixel32 background = scoreboard_background_color();
			real padding = 0.5f * line_height * SCOREBOARD_SCALE;
			real block_width = (columns * SCOREBOARD_COLUMN_WIDTH + (columns - 1) * SCOREBOARD_COLUMN_GAP) * SCOREBOARD_SCALE;
			real block_left = bounds.x0 + (left - bounds.x0) * SCOREBOARD_SCALE;

			if (background >> 24)
			{
				rectangle2d panel;
				long panel_alpha = (long)((background >> 24) * PIN(alpha, 0.0f, 1.0f) + 0.5f);

				panel.x0 = (short)(block_left - padding);
				/* (the ping, nearly as wide as its column, given room) */
				panel.x1 = (short)(block_left + block_width + padding + 8.0f * SCOREBOARD_SCALE);
				panel.y0 = (short)(top - padding);
				panel.y1 = (short)(top + height + padding);
				draw_quad(&panel, ((pixel32)panel_alpha << 24) | (background & 0x00FFFFFF));
			}
		}
	}
	rasterizer_text_set_scale(SCOREBOARD_SCALE, (real)bounds.x0, (real)top);

	/* port: the pointer's click, on the menu's items (where they are
	drawn), else on a player's row (below); one elsewhere closes the menu */
	pointer_x = (short)(pointer.x - render.camera.viewport_bounds.x0);
	pointer_y = (short)(pointer.y - render.camera.viewport_bounds.y0);
	click_x = (short)(pointer.click_x - render.camera.viewport_bounds.x0);
	click_y = (short)(pointer.click_y - render.camera.viewport_bounds.y0);
	clicked = pointer_state > 0 && pointer.left_clicks > 0;
	/* (the menu opens below its row, beyond the scoreboard if need be, but
	above the motion sensor's rows) */
	last_row = (long)((bounds.y1 - top) / (line_height * SCOREBOARD_SCALE)) - SCOREBOARD_BOTTOM_ROWS;
	if (pointer_state <= 0 || (scoreboard_menu.player_index != NONE && !player_try_and_get(scoreboard_menu.player_index)))
		scoreboard_menu.player_index = NONE;
	if (scoreboard_menu.player_index != NONE)
	{
		short item;

		scoreboard_menu_items(menu_labels, menu_offered);
		for (item = 0; item < NUMBER_OF_SCOREBOARD_ITEMS; item++)
			menu_item_count += menu_offered[item] ? 1 : 0;
		menu_first_row = scoreboard_menu_first_row(menu_item_count, last_row);
		if (clicked)
		{
			rectangle2d rectangle;
			short shown = 0;

			for (item = 0; item < NUMBER_OF_SCOREBOARD_ITEMS && clicked; item++)
			{
				if (!menu_offered[item])
					continue;
				scoreboard_rectangle(&rectangle, bounds.x0, top, line_height, scoreboard_menu.left,
					SCOREBOARD_MENU_WIDTH, menu_first_row + shown++, 1);
				if (scoreboard_point_in(&rectangle, click_x, click_y))
				{
					scoreboard_menu_pick(item);
					clicked = FALSE;
				}
			}
			if (clicked)
				scoreboard_menu.player_index = NONE;
			else if (scoreboard_menu.player_index != NONE)
				scoreboard_menu_items(menu_labels, menu_offered);
		}
	}

	team_colors[0].alpha = alpha;
	team_colors[0].red = 0.6f;
	team_colors[0].green = 0.3f;
	team_colors[0].blue = 0.3f;
	team_colors[1].alpha = alpha;
	team_colors[1].red = 0.3f;
	team_colors[1].green = 0.3f;
	team_colors[1].blue = 0.6f;

	if (campaign)
		usprintf(title_string, L"Co-op");
	else
		game_engine_generate_title_string(title_string, player_index);
	color.alpha = alpha;
	color.red = color.green = color.blue = 0.7f;
	scoreboard_draw_row(title_string, FALSE, &color, 0, top, left, FALSE);

	string_list_index = tag_loaded('ustr', "ui\\multiplayer_game_text");
	column_name = string_list_index != NONE && !campaign ? unicode_string_list_get_string(string_list_index, 0x43) : L"";
	score_name = string_list_index != NONE ? unicode_string_list_get_string(string_list_index, 0x44) : L"";
	score_string[0] = 0;
	if (!campaign)
		game_engine->format_score_name(score_string);
	/* port: bounded (the map's column names) */
	usnprintf(row_string, NUMBEROF(row_string), L"\t%s\t%s\t%s\t%s", column_name, score_name, score_string,
		network ? L"Ping" : L"");
	row_string[NUMBEROF(row_string) - 1] = 0;
	{
		long column;

		for (column = 0; column < columns; column++)
		{
			/* (a team column's heading in its team's colour, brightened) */
			if (team_columns)
			{
				color = team_colors[column];
				color.red = MIN(color.red + 0.25f, 1.0f);
				color.green = MIN(color.green + 0.25f, 1.0f);
				color.blue = MIN(color.blue + 0.25f, 1.0f);
			}
			else
			{
				color.alpha = alpha;
				color.red = color.green = color.blue = 0.5f;
			}
			scoreboard_draw_row(row_string, FALSE, &color, 1, top,
				(short)(left + column * (SCOREBOARD_COLUMN_WIDTH + SCOREBOARD_COLUMN_GAP)), TRUE);
		}
	}

	/* each slot of the page: a team column's row, or a place in the list */
	for (index = 0; index < rows * columns; index++)
	{
		long column = team_columns ? index % 2 : index / rows;
		long row = team_columns ? index / 2 : index % rows;
		long list = team_columns ? column : 0;
		long position = scoreboard_scroll + (team_columns ? row : index);
		struct statistic_buffer *entry;
		struct player_datum *player;
		wchar_t *status_string;
		real_argb_color *row_color;
		long ping = NONE;
		boolean ping_colored;

		if (row >= rows || position >= list_counts[list])
			continue;
		entry = &ranked[lists[list][position]];
		player = player_try_and_get(entry->player_index);
		if (!player)
			continue;
		color = *hud_get_text_color(&text_color);
		color.alpha = alpha;
		score_string[0] = 0;
		status_string = score_string;
		if (!campaign)
		{
			game_engine->format_player_score(entry->player_index, score_string);
			if (game_engine_player_is_out_of_lives(entry->player_index))
				status_string = string_list_index != NONE ? unicode_string_list_get_string(string_list_index, 0x8A) : L"";
			else if (player->quit_out_of_game)
				status_string = string_list_index != NONE ? unicode_string_list_get_string(string_list_index, 0x8B) : L"";
		}
		ping_string[0] = 0;
		if (network)
		{
			/* port: as this machine times it, with the latency meter on
			(latency_meter.c); else the host's measure, as before */
			ping = latency_meter_player_ping((short)DATUM_INDEX_TO_ABSOLUTE_INDEX(entry->player_index));

			/* (three digits at most: its column's width) */
			if (ping == NONE)
				usprintf(ping_string, L"-");
			else if (ping > 999)
				usprintf(ping_string, L"999+");
			else
				usprintf(ping_string, L"%ld", ping);
		}
		/* port: with the latency meter on, a known ping is drawn on its own,
		in the meter's colour for it (display.latency_meter) */
		ping_colored = ping != NONE && latency_meter_shown();
		usnprintf(
			row_string,
			NUMBEROF(row_string),
			L"\t%s\t%s\t%s\t%s",
			campaign ? L"" : get_place_string(entry),
			player->name,
			status_string,
			ping_colored ? L"" : ping_string);
		row_string[NUMBEROF(row_string) - 1] = 0;
		row_color = has_teams ? &team_colors[PIN(player->team_index, 0, 1)] : &color;
		/* port: a player talking (or muted) in voice chat, its speaker just
		right of the name */
		if (network && (network_voice_machine_speaking(player->network_player_data.machine_index) ||
			network_voice_machine_muted(player->network_player_data.machine_index)))
		{
			rectangle2d icon;
			rectangle2d text;
			short row_left = (short)(left + column * (SCOREBOARD_COLUMN_WIDTH + SCOREBOARD_COLUMN_GAP));
			short size;
			short middle;

			rectangle2d ink;
			rectangle2d cursor;
			short name_end;

			/* (after the name's last letter, centred on its capitals: both
			measured as the row is laid out, the name at its column's tab
			stop, then scaled as it is drawn, as scoreboard_rectangle has it;
			within the name's column) */
			text.x0 = (short)(row_left + SCOREBOARD_PLACE_WIDTH);
			text.x1 = (short)(text.x0 + SCOREBOARD_NAME_WIDTH);
			text.y0 = (short)(top + (2 + row) * line_height);
			text.y1 = (short)(text.y0 + line_height);
			draw_string_set_draw_mode(font_index, NONE, 0, 0, row_color);
			draw_unicode_string_compute_bounds(&text, player->name, &ink, &cursor);
			middle = (short)(top + (draw_unicode_string_capital_middle(&text, player->name) - top) * SCOREBOARD_SCALE);
			scoreboard_rectangle(&icon, bounds.x0, top, line_height, text.x0, SCOREBOARD_NAME_WIDTH, 2 + row, 1);
			size = (short)((icon.y1 - icon.y0) * 2 / 3);
			name_end = (short)(bounds.x0 + (MAX(ink.x1, text.x0) - bounds.x0) * SCOREBOARD_SCALE);
			icon.x0 = (short)MIN(name_end + 4, icon.x1 - size);
			icon.x1 = (short)(icon.x0 + size);
			icon.y0 = (short)(middle - size / 2);
			icon.y1 = (short)(icon.y0 + size);
			network_voice_draw_icon(&icon, network_voice_machine_muted(player->network_player_data.machine_index),
				alpha);
		}
		/* port: another machine's player, picked with the pointer: lit
		under it (not under the menu), and its menu opened when clicked */
		if (pointer_state > 0 && player->local_player_index == NONE)
		{
			rectangle2d rectangle;
			rectangle2d menu_rectangle;
			short row_left = (short)(left + column * (SCOREBOARD_COLUMN_WIDTH + SCOREBOARD_COLUMN_GAP));

			scoreboard_rectangle(&rectangle, bounds.x0, top, line_height, row_left, SCOREBOARD_COLUMN_WIDTH,
				2 + row, 1);
			scoreboard_rectangle(&menu_rectangle, bounds.x0, top, line_height, scoreboard_menu.left,
				SCOREBOARD_MENU_WIDTH, menu_first_row, menu_item_count);
			if (scoreboard_point_in(&rectangle, pointer_x, pointer_y) &&
				(scoreboard_menu.player_index == NONE || !scoreboard_point_in(&menu_rectangle, pointer_x, pointer_y)))
			{
				draw_quad(&rectangle, ((pixel32)(long)(80 * PIN(alpha, 0.0f, 1.0f)) << 24) | 0x00FFFFFF);
			}
			if (clicked && scoreboard_point_in(&rectangle, click_x, click_y))
			{
				scoreboard_menu.player_index = entry->player_index;
				scoreboard_menu.row_index = 2 + row;
				scoreboard_menu.left = (short)(row_left + SCOREBOARD_PLACE_WIDTH);
				scoreboard_menu.ban_picked = FALSE;
				clicked = FALSE;
			}
		}
		scoreboard_draw_row(
			row_string,
			player_index == entry->player_index,
			row_color,
			2 + row,
			top,
			(short)(left + column * (SCOREBOARD_COLUMN_WIDTH + SCOREBOARD_COLUMN_GAP)),
			TRUE);
		if (ping_colored)
		{
			real_argb_color ping_color;

			latency_meter_color(ping, &ping_color);
			ping_color.alpha = alpha;
			usnprintf(row_string, NUMBEROF(row_string), L"\t\t\t\t%s", ping_string);
			row_string[NUMBEROF(row_string) - 1] = 0;
			scoreboard_draw_row(
				row_string,
				FALSE,
				&ping_color,
				2 + row,
				top,
				(short)(left + column * (SCOREBOARD_COLUMN_WIDTH + SCOREBOARD_COLUMN_GAP)),
				TRUE);
		}
	}
	/* (where the scroll is, and how to move it) */
	if (total > page)
	{
		long first = scoreboard_scroll + 1;
		long last = MIN(scoreboard_scroll + page, total);

		color.alpha = alpha;
		color.red = color.green = color.blue = 0.6f;
		usprintf(row_string, L"%ld-%ld of %ld   (Page Up / Page Down, mouse wheel)", first, last, total);
		scoreboard_draw_row(row_string, FALSE, &color, 2 + rows, top, left, FALSE);
	}
	/* port: the vote running, and how to pick a player */
	next_row = 2 + MIN(rows, total) + (total > page ? 1 : 0);
	if (voting)
	{
		struct data_iterator iterator;
		struct player_datum *target;
		wchar_t const *name = L"?";
		wchar_t const *note = vote.voted ? L" (you voted yes)" : vote.voted_no ? L" (you voted no)" :
			vote.may_vote ? L"  F1 yes, F2 no" : L" (too new to vote)";

		data_iterator_new(&iterator, player_data);
		while (vote.kind == _network_vote_kick &&
			(target = (struct player_datum *)data_iterator_next(&iterator)) != NULL)
		{
			if (DATUM_INDEX_TO_ABSOLUTE_INDEX(iterator.datum_index) == vote.player_index)
			{
				name = target->name;
				if (target->local_player_index != NONE)
					note = L" (against you)";
			}
		}
		color.alpha = alpha;
		color.red = 1.0f;
		color.green = 0.75f;
		color.blue = 0.3f;
		if (vote.kind == _network_vote_end_game)
		{
			usprintf(row_string, L"Vote to end the game: %d yes, %d no, %d needed, %d s left%s", vote.votes,
				vote.no_votes, vote.needed, vote.seconds_left, note);
		}
		else
		{
			usprintf(row_string, L"Vote to kick %s: %d yes, %d no, %d needed, %d s left%s", name, vote.votes,
				vote.no_votes, vote.needed, vote.seconds_left, note);
		}
		scoreboard_draw_row(row_string, FALSE, &color, next_row++, top, left, FALSE);
	}
	if (network && pointer_state >= 0)
	{
		color.alpha = alpha;
		color.red = color.green = color.blue = 0.6f;
		if (pointer_state == 0)
			usprintf(row_string, L"Right-click to pick a player");
		else
			usprintf(row_string, L"Pick a player   (right-click to aim)");
		scoreboard_draw_row(row_string, FALSE, &color, next_row++, top, left, FALSE);
	}
	/* port: the picked player's menu, over the rows, its item under the
	pointer lit */
	if (scoreboard_menu.player_index != NONE && menu_item_count > 0)
	{
		rectangle2d rectangle;
		short item;
		short shown = 0;
		long panel_alpha = (long)(255 * PIN(alpha, 0.0f, 1.0f));

		scoreboard_rectangle(&rectangle, bounds.x0, top, line_height, scoreboard_menu.left, SCOREBOARD_MENU_WIDTH,
			menu_first_row, menu_item_count);
		draw_quad(&rectangle, ((pixel32)panel_alpha << 24) | 0x00202830);
		for (item = 0; item < NUMBER_OF_SCOREBOARD_ITEMS; item++)
		{
			if (!menu_offered[item])
				continue;
			scoreboard_rectangle(&rectangle, bounds.x0, top, line_height, scoreboard_menu.left,
				SCOREBOARD_MENU_WIDTH, menu_first_row + shown, 1);
			if (scoreboard_point_in(&rectangle, pointer_x, pointer_y))
				draw_quad(&rectangle, ((pixel32)(long)(90 * PIN(alpha, 0.0f, 1.0f)) << 24) | 0x00FFFFFF);
			color.alpha = alpha;
			color.red = color.green = color.blue = 0.9f;
			if (item == _scoreboard_item_ban)
			{
				color.green = color.blue = 0.45f;
			}
			scoreboard_draw_row(menu_labels[item], FALSE, &color, menu_first_row + shown, top,
				(short)(scoreboard_menu.left + 6), FALSE);
			shown++;
		}
	}
	rasterizer_text_set_scale(1.0f, 0.0f, 0.0f);

	return;
}

static void game_engine_rasterize_in_game_score(
	long player_index,
	real alpha)
{
	wchar_t row_string[256];
	wchar_t score_string[256];
	struct statistic_buffer entries[6];
	wchar_t title_string[80];
	real_argb_color text_color;
	real_argb_color team_colors[2];
	real_argb_color color;
	boolean has_teams = game_engine_has_teams();
	boolean is_current_player;
	long entry_count;
	long entry_index;
	long string_list_index;
	wchar_t *column_name;
	wchar_t *score_name;

	/* port: a full-screen view's its own, as is the campaign's
	(game_engine_rasterize_scoreboard) */
	if (local_player_count() <= 1 || !game_engine)
	{
		game_engine_rasterize_scoreboard(player_index, alpha);
		return;
	}
	game_engine_generate_title_string(title_string, player_index);
	statistic_buffer_in_game_only = TRUE;
	entry_count = select_players_to_display(
		_postgame_statistic_ranking,
		player_index,
		entries,
		NUMBEROF(entries));
	statistic_buffer_in_game_only = FALSE;

	color.alpha = alpha;
	color.red = 0.7f;
	color.green = 0.7f;
	color.blue = 0.7f;
	rasterize_in_game_score_draw_line(title_string, FALSE, &color, 0);

	color.red = 0.5f;
	color.green = 0.5f;
	color.blue = 0.5f;
	color.alpha = alpha;

	string_list_index = tag_loaded('ustr', "ui\\multiplayer_game_text");
	if (string_list_index != NONE)
		column_name = unicode_string_list_get_string(string_list_index, 0x43);
	else
		column_name = L"";

	string_list_index = tag_loaded('ustr', "ui\\multiplayer_game_text");
	if (string_list_index != NONE)
		score_name = unicode_string_list_get_string(string_list_index, 0x44);
	else
		score_name = L"";

	game_engine->format_score_name(score_string);
	usnprintf(row_string, NUMBEROF(row_string), L"\t%s\t%s\t%s", column_name, score_name, score_string);
	row_string[NUMBEROF(row_string) - 1] = 0;
	rasterize_in_game_score_draw_line(row_string, FALSE, &color, 1);

	for (entry_index = 0; entry_index < entry_count; entry_index++)
	{
		long entry_player_index = entries[entry_index].player_index;
		struct player_datum *player =
			player_try_and_get(entry_player_index);
		wchar_t *status_string;
		wchar_t *place_string;
		real_argb_color *row_color;

		is_current_player = player_index == entry_player_index;
		if (player)
		{
			color = *hud_get_text_color(&text_color);
			player = player_get(entry_player_index);

			team_colors[0].alpha = alpha;
			team_colors[0].red = 0.6f;
			team_colors[0].green = 0.3f;
			team_colors[0].blue = 0.3f;
			team_colors[1].alpha = alpha;
			team_colors[1].red = 0.3f;
			team_colors[1].green = 0.3f;
			team_colors[1].blue = 0.6f;
			color.alpha = alpha;

			game_engine->format_player_score(
				entry_player_index,
				score_string);

			if (game_engine_player_is_out_of_lives(entry_player_index))
			{
				string_list_index =
					tag_loaded('ustr', "ui\\multiplayer_game_text");
				if (string_list_index != NONE)
					status_string =
						unicode_string_list_get_string(string_list_index, 0x8A);
				else
					status_string = L"";
			}
			else if (player->quit_out_of_game)
			{
				string_list_index =
					tag_loaded('ustr', "ui\\multiplayer_game_text");
				if (string_list_index != NONE)
					status_string =
						unicode_string_list_get_string(string_list_index, 0x8B);
				else
					status_string = L"";
			}
			else
				status_string = score_string;

			place_string = get_place_string(&entries[entry_index]);

			usnprintf(
				row_string,
				NUMBEROF(row_string),
				L"\t%s\t%s\t%s",
				place_string,
				player->name,
				status_string);
			row_string[NUMBEROF(row_string) - 1] = 0;

			if (has_teams)
				row_color = &team_colors[PIN(player->team_index, 0, 1)];
			else
				row_color = &color;

			rasterize_in_game_score_draw_line(
				row_string,
				is_current_player,
				row_color,
				entry_index + 2);
		}
	}

	return;
}

static void drawline(
	wchar_t const *string,
	long row_index,
	short justification)
{
	rectangle2d bounds = render.camera.window_bounds;
	long line_height = 18;
	long line_spacing = 8;

	offset_rectangle2d(
		&bounds,
		-render.camera.viewport_bounds.x0,
		-render.camera.viewport_bounds.y0);
	bounds.y0 = (short)(row_index * line_height);
	bounds.y1 = (short)((row_index + 1) * line_height + line_spacing);
	draw_string_set_format(NONE, justification, 0);
	rasterizer_draw_unicode_string(&bounds, NULL, NULL, 0, string);

	return;
}

void game_engine_post_rasterize_post_game(
	void)
{
	/* port: sized like every buffer handed to populate_statistic_buffer */
	struct statistic_buffer entries[MULTIPLAYER_MAXIMUM_PLAYERS];
	wchar_t score_string[256];
	wchar_t row_string[256];
	short tab_stops[6];
	real_argb_color winner_color;
	real_argb_color normal_color;
	real_argb_color hilite_color;
	long font_index;
	long entry_count;
	long entry_index;

	if (!game_engine)
		return;

	tab_stops[0] = 50;
	tab_stops[1] = 125;
	tab_stops[2] = 250;
	tab_stops[3] = 350;
	tab_stops[4] = 410;
	tab_stops[5] = 500;

	font_index = hud_globals->messaging.single_player_font.index;
	get_postgame_hilite_colors(
		&winner_color,
		&normal_color,
		&hilite_color);
	draw_string_set_draw_mode(
		font_index,
		NONE,
		2,
		8,
		&winner_color);
	draw_string_set_color(&winner_color);
	draw_string_set_format(NONE, 0, 0);

	{
		struct hud_globals_definition *hud_definition =
			hud_globals_definition_get(
				interface_get_tag_index(_interface_hud_globals));
		rectangle2d bounds;

		bounds.x0 = 0;
		bounds.y0 = 0;
		bounds.x1 = 640;
		bounds.y1 = 480;
		if (bitmap_group_try_and_get_bitmap(hud_definition->carnage_report_bitmap.index, 0))
		{
			draw_bitmap_in_rect(
				bitmap_group_try_and_get_bitmap(hud_definition->carnage_report_bitmap.index, 0),
				&bounds,
				&bounds,
				NULL,
				NONE,
				NULL,
				TRUE);
		}
	}

	if (global_variant.universal_variant.teams)
	{
		short team_tab_stops[6] = { 50, 200, 300, 350, 410, 500 };
		long team_order[2] = { 0, 1 };
		wchar_t const *team_formats[2];
		long string_list_index;
		long team_row;
		long red_team_won;

		red_team_won = game_engine_did_team_win(0);
		string_list_index = tag_loaded('ustr', "ui\\multiplayer_game_text");
		team_formats[0] =
			string_list_index != NONE ?
				unicode_string_list_get_string(string_list_index, 0x41) :
				L"";
		string_list_index = tag_loaded('ustr', "ui\\multiplayer_game_text");
		team_formats[1] =
			string_list_index != NONE ?
				unicode_string_list_get_string(string_list_index, 0x42) :
				L"";

		if (!red_team_won)
		{
			team_order[0] = 1;
			team_order[1] = 0;
		}

		draw_string_set_tab_stops(team_tab_stops, NUMBEROF(team_tab_stops));
		for (team_row = 0; team_row < 2; team_row++)
		{
			long team_index = team_order[team_row];

			game_engine->format_team_name(
				team_index,
				score_string);
			usnprintf(
				row_string,
				NUMBEROF(row_string),
				ustring_format_checked(team_formats[team_index], "s"),
				score_string);
			row_string[NUMBEROF(row_string) - 1] = 0;
			drawline(row_string, team_row + 4, 0);
		}
	}

	{
		long string_list_index;
		wchar_t const *column_strings[5];

		string_list_index = tag_loaded('ustr', "ui\\multiplayer_game_text");
		column_strings[0] =
			string_list_index != NONE ?
				unicode_string_list_get_string(string_list_index, 0x43) :
				L"";
		string_list_index = tag_loaded('ustr', "ui\\multiplayer_game_text");
		column_strings[1] =
			string_list_index != NONE ?
				unicode_string_list_get_string(string_list_index, 0x44) :
				L"";
		string_list_index = tag_loaded('ustr', "ui\\multiplayer_game_text");
		column_strings[2] =
			string_list_index != NONE ?
				unicode_string_list_get_string(string_list_index, 0x45) :
				L"";
		string_list_index = tag_loaded('ustr', "ui\\multiplayer_game_text");
		column_strings[3] =
			string_list_index != NONE ?
				unicode_string_list_get_string(string_list_index, 0x46) :
				L"";
		string_list_index = tag_loaded('ustr', "ui\\multiplayer_game_text");
		column_strings[4] =
			string_list_index != NONE ?
				unicode_string_list_get_string(string_list_index, 0x47) :
				L"";

		game_engine->format_score_name(score_string);
		usnprintf(
			row_string,
			NUMBEROF(row_string),
			L"\t%s\t%s\t%s\t%s\t%s\t%s",
			column_strings[0],
			column_strings[1],
			score_string,
			column_strings[2],
			column_strings[3],
			column_strings[4]);
		row_string[NUMBEROF(row_string) - 1] = 0;
		draw_string_set_tab_stops(tab_stops, NUMBEROF(tab_stops));
		drawline(row_string, 7, 0);
	}

	entry_count = select_players_to_display(_postgame_statistic_ranking, NONE, entries, 12);
	for (entry_index = 0; entry_index < entry_count; entry_index++)
	{
		long player_index = entries[entry_index].player_index;
		long draw_row = entry_index + 8;
		struct player_datum *player = player_get(player_index);
		real_argb_color team_colors[2];
		wchar_t const *place_string;

		if (player->local_player_index != NONE)
			draw_string_set_color(&normal_color);
		else
			draw_string_set_color(&winner_color);
		draw_string_set_tab_stops(tab_stops, NUMBEROF(tab_stops));
		place_string = get_place_string(&entries[entry_index]);
		usnprintf(row_string, NUMBEROF(row_string), L" \t%s", place_string);
		row_string[NUMBEROF(row_string) - 1] = 0;
		drawline(row_string, draw_row, 0);
		draw_string_set_color(&winner_color);

		if (global_variant.universal_variant.teams)
		{
			long team_index = player->team_index;

			team_colors[0].red = 0.8f;
			team_colors[0].green = 0.4f;
			team_colors[0].blue = 0.4f;
			team_colors[0].alpha = 1.0f;
			team_colors[1].red = 0.4f;
			team_colors[1].green = 0.4f;
			team_colors[1].blue = 0.8f;
			team_colors[1].alpha = 1.0f;
			draw_string_set_color(
				&team_colors[PIN(team_index, 0, 1)]);
		}
		usnprintf(row_string, NUMBEROF(row_string), L" \t \t%s", player->name);
		row_string[NUMBEROF(row_string) - 1] = 0;
		drawline(row_string, draw_row, 0);
		draw_string_set_color(&winner_color);

		if (!postgame_statistic_get_rating(player_index, _postgame_statistic_score, FALSE))
			draw_string_set_color(&hilite_color);
		game_engine->format_player_score(
			player_index,
			score_string);
		usnprintf(row_string, NUMBEROF(row_string), L" \t \t \t%s", score_string);
		row_string[NUMBEROF(row_string) - 1] = 0;
		drawline(row_string, draw_row, 0);
		draw_string_set_color(&winner_color);

		if (!postgame_statistic_get_rating(player_index, _postgame_statistic_kills, FALSE))
			draw_string_set_color(&hilite_color);
		usnprintf(
			row_string,
			NUMBEROF(row_string),
			L" \t \t \t \t%d",
			(long)player->statistics.kills[0]);
		row_string[NUMBEROF(row_string) - 1] = 0;
		drawline(row_string, draw_row, 0);
		draw_string_set_color(&winner_color);

		if (!postgame_statistic_get_rating(player_index, _postgame_statistic_assists, FALSE))
			draw_string_set_color(&hilite_color);
		usnprintf(
			row_string,
			NUMBEROF(row_string),
			L" \t \t \t \t \t%d",
			(long)player->statistics.assists[0]);
		row_string[NUMBEROF(row_string) - 1] = 0;
		drawline(row_string, draw_row, 0);
		draw_string_set_color(&winner_color);

		if (!postgame_statistic_get_rating(player_index, _postgame_statistic_deaths, FALSE))
			draw_string_set_color(&hilite_color);
		usnprintf(
			row_string,
			NUMBEROF(row_string),
			L" \t \t \t \t \t \t%d",
			(long)player->statistics.deaths);
		row_string[NUMBEROF(row_string) - 1] = 0;
		drawline(row_string, draw_row, 0);
		draw_string_set_tab_stops(tab_stops, NUMBEROF(tab_stops));
	}

	{
		real_argb_color prompt_color;
		long string_list_index;
		wchar_t const *prompt;
		struct network_game_server *server;
		rectangle2d bounds;

		bounds = render.camera.window_bounds;
		prompt_color = winner_color;
		prompt_color.alpha = game_engine_globals.postgame_progress;
		bounds.y0 = 410;
		bounds.x0 = 70;
		offset_rectangle2d(
			&bounds,
			-render.camera.viewport_bounds.x0,
			-render.camera.viewport_bounds.y0);
		draw_string_set_tab_stops(NULL, 0);
		draw_string_set_color(&prompt_color);

		server = global_network_game_server_get();
		/* port: on the touchscreen, buttons for the prompt's A and B
		(port/linux/game/touch_postgame.c) */
		if (touch_postgame_draw(font_index, server != NULL, &prompt_color))
			return;
		if (server)
		{
			bounds.x0 = 380;
			string_list_index =
				tag_loaded('ustr', "ui\\multiplayer_game_text");
			if (string_list_index != NONE)
				prompt =
					unicode_string_list_get_string(string_list_index, 0x48);
			else
				prompt = L"";
			draw_string_and_hack_in_icons(
				&bounds,
				NULL,
				NULL,
				0,
				prompt,
				FALSE);
			return;
		}

		bounds.x0 = 520;
		string_list_index = tag_loaded('ustr', "ui\\multiplayer_game_text");
		if (string_list_index != NONE)
			prompt = unicode_string_list_get_string(string_list_index, 0x49);
		else
			prompt = L"";
		draw_string_and_hack_in_icons(
			&bounds,
			NULL,
			NULL,
			0,
			prompt,
			FALSE);
	}

	return;
}

static long find_closest_player_index(
	long player_index)
{
	struct player_datum *player = player_get(player_index);
	long best_object_index = NONE;

	if (player->local_player_index != NONE &&
		player_control_get_autoaim_level(player->local_player_index) > 0.0f)
	{
		best_object_index =
			player_control_get_target_object_index(player->local_player_index);
	}

	if (best_object_index == NONE)
	{
		long object_indices[32];
		real_point3d camera_position;
		real_vector3d facing_direction;
		long object_count;
		long object_index;

		unit_get_camera_position(player->unit_index, &camera_position);
		player_control_get_facing_direction(
			player->local_player_index,
			&facing_direction);
		object_count = find_objects_from_point_vector(
			&camera_position,
			&facing_direction,
			find_closest_player_callback,
			&player_index,
			NUMBEROF(object_indices),
			object_indices);

		for (object_index = 0; object_index < object_count; object_index++)
		{
			long candidate_object_index = object_indices[object_index];
			struct unit_datum *candidate = unit_get(candidate_object_index);
			real distance_squared = distance_squared3d(
				&camera_position,
				&candidate->object.position);
			real_point3d target_position;
			real_vector3d target_direction;
			real target_distance;
			real target_angle;

			if ((candidate->unit.active_camouflage < 1.0f ||
				player->player_display_index == player_index_from_unit_index(candidate_object_index)) &&
				autoaim_compute_target(
					object_indices[object_index],
					&camera_position,
					&facing_direction,
					player->unit_index,
					&target_position,
					&target_direction,
					&target_distance,
					&target_angle) &&
				fabs(target_angle) < 0.13083334267139435 &&
				distance_squared < 400.0f &&
				distance_squared < 900.0f)
			{
				best_object_index = object_indices[object_index];
			}
		}
	}

	if (best_object_index != NONE)
		best_object_index = player_index_from_unit_index(best_object_index);

	return best_object_index;
}

long game_engine_remap_equipment(
	long equipment_definition_index)
{
	struct equipment_definition *equipment;
	long weapon_list_index;
	boolean remap_equipment;

	if (equipment_definition_index == NONE)
		equipment = NULL;
	else
		equipment = equipment_definition_get(equipment_definition_index);

	weapon_list_index =
		weapon_definition_index_to_list_index(equipment_definition_index);
	remap_equipment = weapon_list_index == _weapon_list_frag_grenade ||
		weapon_list_index == _weapon_list_plasma_grenade;
	if (!remap_equipment)
	{
		if (equipment)
		{
			if (equipment->equipment.powerup_type == _equipment_powerup_overshield)
			{
				if (TEST_FLAG(global_variant.universal_variant.flags, _game_variant_no_shields_bit))
					return NONE;
			}
			else if (equipment->equipment.powerup_type == _equipment_powerup_active_camouflage)
			{
				if (TEST_FLAG(global_variant.universal_variant.flags, _game_variant_always_invisible_bit))
					return NONE;
			}
		}
	}
	else
	{
		switch (global_variant.universal_variant.weapon_set)
		{
		case _game_engine_weapons_plasma_weapons:
			weapon_list_index = _weapon_list_plasma_grenade;
			break;

		case _game_engine_weapons_human:
			weapon_list_index = _weapon_list_frag_grenade;
			break;

		case _game_engine_weapons_no_grenades:
			weapon_list_index = NONE;
			break;
		}

		/* (port: whatever the number of players, as
		game_engine_infinite_grenades_internal) */
		if (game_engine_infinite_grenades_internal())
		{
			weapon_list_index = NONE;
		}

		if (TEST_FLAG(game_engine_globals.flags, _game_engine_9_or_more_players_bit))
		{
			if (real_seed_random(get_global_random_seed_address()) > 0.3f)
				weapon_list_index = NONE;
		}
		else if (TEST_FLAG(game_engine_globals.flags, _game_engine_5_or_more_players_bit))
		{
			if (real_seed_random(get_global_random_seed_address()) > 0.55f)
				weapon_list_index = NONE;
		}

	}

	if (remap_equipment)
		return list_index_to_weapon_definition_index(weapon_list_index);

	return equipment_definition_index;
}

int __cdecl sort_statistic_buffer(
	void const *entry0_pointer,
	void const *entry1_pointer)
{
	struct statistic_buffer const *entry0 = entry0_pointer;
	struct statistic_buffer const *entry1 = entry1_pointer;
	long result = 0;

	if (entry0->score > entry1->score)
		result = -1;
	else if (entry1->score > entry0->score)
		result = 1;

	return result;
}

int __cdecl sort_statistic_buffer_ranking(
	void const *entry0_pointer,
	void const *entry1_pointer)
{
	struct statistic_buffer const *entry0 = entry0_pointer;
	struct statistic_buffer const *entry1 = entry1_pointer;
	long result = 0;

	if (entry0->custom > entry1->custom)
		result = -1;
	else if (entry1->custom > entry0->custom)
		result = 1;
	else if (entry0->kills > entry1->kills)
		result = -1;
	else if (entry1->kills > entry0->kills)
		result = 1;
	else if (entry0->deaths > entry1->deaths)
		result = 1;
	else if (entry1->deaths > entry0->deaths)
		result = -1;
	else if (entry0->assists > entry1->assists)
		result = -1;
	else if (entry1->assists > entry0->assists)
		result = 1;

	return result;
}

static boolean is_place_tied(
	struct statistic_buffer const *entry)
{
	boolean result =
		(entry->place & 0x80000000) > 0;

	return result;
}

static long place_get_position(
	struct statistic_buffer const *entry)
{
	return entry->place & ~FLAG(31);
}

static void get_selected_color(
	real_argb_color *color)
{
	color->red = 0.98f;
	color->green = 0.96f;
	color->blue = 0.96f;
	color->alpha = 1.0f;

	return;
}

static boolean can_delete_item(
	long weapon_index)
{
	boolean result = TRUE;

	if (weapon_try_and_get(weapon_index) &&
		weapon_is_flag(weapon_index))
	{
		result = FALSE;
	}

	return result;
}

static boolean multiplayer_message_internal(
	long player_index,
	long message,
	long message_data,
	wchar_t *buffer,
	long buffer_size)
{
	boolean result = FALSE;

	if (game_engine->format_message)
	{
		result = game_engine->format_message(
			player_index,
			message,
			message_data,
			buffer,
			buffer_size);
	}

	if (!result)
	{
		result = internal_rasterize_score(
			player_index,
			message,
			message_data,
			buffer,
			buffer_size);
	}

	return result;
}


static void multiplayer_message(
	long player_index,
	long message,
	long message_data)
{
	struct player_datum *player = player_get(player_index);

	if (player->local_player_index != NONE)
	{
		wchar_t buffer[1024];

		if (multiplayer_message_internal(
			player_index,
			message,
			message_data,
			buffer,
			NUMBEROF(buffer)))
		{
			buffer[NUMBEROF(buffer) - 1] = 0;
			hud_print_message(player->local_player_index, buffer);
		}
	}

	return;
}

static void game_show_score_one_player(
	long player_index,
	long message,
	long message_data)
{
	if (message != NONE)
		multiplayer_message(player_index, message, message_data);

	return;
}

void game_show_score_team(
	long team_index,
	long score)
{
	struct data_iterator iterator;
	struct player_datum *player;

	data_iterator_new(&iterator, player_data);
	player = (struct player_datum *)data_iterator_next(&iterator);
	while (player)
	{
		if (player->team_index == team_index)
		{
			game_show_score_one_player(iterator.datum_index, score, NONE);
		}

		player = (struct player_datum *)data_iterator_next(&iterator);
	}

	return;
}

void game_show_score_you_ally_enemy(
	long player_index,
	long you_score,
	long ally_score,
	long enemy_score,
	long other_player_index)
{
	struct player_datum *local_player;
	struct data_iterator iterator;
	struct player_datum *player;
	long score;

	local_player = player_get(player_index);

	match_assert(
		"c:\\halo\\SOURCE\\game\\game_engine.c",
		0xB1A,
		NONE != player_index);

	data_iterator_new(&iterator, player_data);
	player = (struct player_datum *)data_iterator_next(&iterator);
	while (player)
	{
		if (iterator.datum_index == player_index)
		{
			if (you_score != NONE)
			{
				game_show_score_one_player(
					iterator.datum_index,
					you_score,
					other_player_index);
			}
		}
		else
		{
			if (game_team_is_enemy(
				(short)local_player->team_index,
				(short)player->team_index))
			{
				score = enemy_score;
			}
			else
				score = ally_score;

			if (score != NONE)
			{
				game_show_score_one_player(
					iterator.datum_index,
					score,
					other_player_index);
			}
		}

		player = (struct player_datum *)data_iterator_next(&iterator);
	}

	return;
}

void game_show_score_extended(
	long player_index,
	long score,
	long team_index)
{
	if (player_index != NONE)
	{
		game_show_score_one_player(player_index, score, team_index);
	}
	else
	{
		struct data_iterator iterator;

		data_iterator_new(&iterator, player_data);
		while (data_iterator_next(&iterator))
		{
			game_show_score_one_player(
				iterator.datum_index,
				score,
				team_index);
		}
	}

	return;
}

long players_in_game(
	void)
{
	struct data_iterator iterator;
	long player_count = 0;

	data_iterator_new(&iterator, player_data);
	while (data_iterator_next(&iterator))
		player_count++;

	return player_count;
}

static void game_engine_press_start_to_begin(
	void)
{
	return;
}

static boolean goal_matches_player(
	struct player_datum *player,
	long player_index,
	long goal_index)
{
	boolean result = FALSE;

	if (game_engine)
	{
		struct netgame_goal *goal = &global_goal[goal_index];

		if (game_engine->player_can_see_goal)
		{
			if (goal->in_use)
				result = game_engine->player_can_see_goal(player_index, goal_index);
		}
		else if (
			goal->in_use &&
			(goal->player_index == NONE || player_index == goal->player_index) &&
			(goal->team_index == NONE || player->team_index == goal->team_index) &&
			(goal->ignore_player_index == NONE || player_index != goal->ignore_player_index))
		{
			result = TRUE;
		}
	}

	return result;
}

static boolean nearby_vehicle(
	long player_index,
	struct player_starting_location const *starting_location)
{
	struct location location;
	long object_indices[16];
	short object_count;
	short object_index;

	player_get(player_index);
	scenario_location_from_point(&location, &starting_location->position);
	object_count = objects_in_sphere(
		0,
		0x11F,
		&location,
		&starting_location->position,
		0.1f,
		object_indices,
		NUMBEROF(object_indices));

	for (object_index = 0; object_index < object_count; object_index++)
	{
		struct object_datum *object = object_get(object_indices[object_index]);

		match_assert(
			"c:\\halo\\SOURCE\\game\\game_engine.c",
			0xEE9,
			object);

		if (object->object.type == _object_type_vehicle)
		{
			struct vehicle_datum *vehicle =
				(struct vehicle_datum *)object_get_and_verify_type(
					object_indices[object_index],
					_object_mask_vehicle);

			match_assert(
				"c:\\halo\\SOURCE\\game\\game_engine.c",
				0xEEE,
				vehicle);

			if (vehicle)
				return TRUE;

			return FALSE;
		}
	}

	return FALSE;
}

static real game_engine_get_friendly_bonus(
	long player_index,
	real_point3d const *position)
{
	struct player_datum *player = player_get(player_index);
	struct data_iterator iterator;
	struct player_datum *other_player;
	real rating = 0.0f;

	data_iterator_new(&iterator, player_data);
	other_player = (struct player_datum *)data_iterator_next(&iterator);
	while (other_player)
	{
		if (player->team_index == other_player->team_index &&
			other_player->unit_index != NONE)
		{
			real_point3d origin;
			real distance;

			object_get_origin(other_player->unit_index, &origin);
			distance = distance3d(&origin, position);
			if (distance >= 1.0f && distance <= 6.0f)
			{
				rating += (real)pow(
					(double)(1.0f - (distance - 1.0f) * 0.2f),
					(double)0.6f);
			}
		}

		other_player = (struct player_datum *)data_iterator_next(&iterator);
	}

	if (rating > 3.0f)
		rating = 3.0f;

	return rating * 3.0f + 1.0f;
}

/* port: Teamplay Options' FORCE TEAM SPAWNS (game_variant_options): a team
game other than CTF spawns a red or blue player as CTF does, only at the
map's CTF starting locations, and only at his own team's. Not on a map with
no CTF starting location for one of the teams, where his team would have
nowhere to spawn */
static long game_engine_get_type(
	void);

static boolean game_engine_force_team_spawns(
	long player_index)
{
	struct player_datum *player = player_get(player_index);
	boolean team_has_spawn[2] = { FALSE, FALSE };
	short starting_location_count;
	short starting_location_index;

	if (!game_engine_running() ||
		!game_engine_has_teams() ||
		game_engine_get_type() == game_engine_ctf ||
		!game_variant_options_get()->force_team_spawns ||
		(player->team_index != _team_red && player->team_index != _team_blue))
	{
		return FALSE;
	}

	starting_location_count = player_get_starting_location_count();
	for (starting_location_index = 0;
		starting_location_index < starting_location_count;
		starting_location_index++)
	{
		struct player_starting_location const *starting_location =
			player_get_starting_location(starting_location_index);

		if ((starting_location->team_index == _team_red ||
				starting_location->team_index == _team_blue) &&
			match_game_type(game_engine_ctf, 4, starting_location->game_types))
		{
			team_has_spawn[starting_location->team_index] = TRUE;
		}
	}

	return team_has_spawn[_team_red] && team_has_spawn[_team_blue];
}

static real default_starting_location_rate_function(
	long player_index,
	struct player_starting_location const *starting_location)
{
	struct player_datum *player = player_get(player_index);
	real rating = 1.0f;

	if (game_engine_running() &&
		(game_engine_test_flag(0) || game_engine_force_team_spawns(player_index)) &&
		player->team_index != starting_location->team_index)
	{
		rating = 0.0f;
	}

	if (rating > 0.0f)
	{
		rating *= game_engine_get_distance_rating_for_spawn(
			player_index,
			&starting_location->position);
	}

	if (game_engine_running() && rating > 0.0f && game_engine_has_teams())
	{
		rating *= game_engine_get_friendly_bonus(
			player_index,
			&starting_location->position);
	}

	if (game_engine_running() && game_engine->starting_location_rating)
	{
		rating *= game_engine->starting_location_rating(
			player_index,
			starting_location);
	}

	return rating;
}

static boolean test_any_gamepad_button(
	long button_index)
{
	boolean result = FALSE;
	long gamepad_index;

	for (gamepad_index = 0; gamepad_index < 4; gamepad_index++)
	{
		struct gamepad_state const *gamepad_state =
			input_get_gamepad_state((short)gamepad_index);

		if (gamepad_state && gamepad_state->buttons[button_index])
		{
			result = TRUE;
			break;
		}
	}

	return result;
}

boolean match_game_type(
	long game_type,
	long count,
	short const *game_types)
{
	boolean result;
	long index;

	if (game_engine)
	{
		result = FALSE;
		for (index = 0; index < count; index++)
		{
			short entry = game_types[index];

			result = result | (entry == game_type);
			if (entry == _game_engine_all)
			{
				result = result | TRUE;
			}
			else if (entry == _game_engine_all_non_team)
			{
				result = result | (game_type != game_engine_ctf);
			}
			else if (entry == _game_engine_all_normal)
			{
				result = result | (game_type != game_engine_ctf && game_type != game_engine_race);
			}
		}
	}
	else
	{
		result = TRUE;
		for (index = 0; index < count; index++)
			result = result & (game_types[index] == game_engine_none);
	}

	return result;
}

static boolean game_engine_infinite_grenades_internal(
	void)
{
	/* port: whatever the number of players (the Xbox game's had none with
	five or more) */
	return TEST_FLAG(global_variant.universal_variant.flags, _game_variant_infinite_grenades_bit);
}

static boolean find_closest_player_callback(
	long object_index,
	void *custom_data)
{
	long excluded_player = *(long *)custom_data;
	boolean result = FALSE;
	struct object_datum *object = object_get(object_index);

	if (!TEST_FLAG(object->object.flags, _object_invisible_bit) &&
		TEST_FLAG(_object_mask_biped, object->object.type) &&
		!TEST_FLAG(object->object.damage_flags, _object_dead_bit) &&
		player_index_from_unit_index(object_index) != excluded_player)
	{
		result = TRUE;
	}

	return result;
}

static void game_engine_update_purge(
	void)
{
	long cutoff_time = game_time_get() - 900;

	/* (a client of the distributed netcode removes items when the host does,
	port/linux/game/network_distributed.c) */
	if (!network_game_distributed_client())
	{
		struct object_iterator item_iterator;

		object_iterator_new(&item_iterator, _object_mask_item, 0);
		while (object_iterator_next(&item_iterator))
		{
			struct item_datum *item = item_get(item_iterator.index);

			if (item->item.last_owned_time < cutoff_time &&
				!TEST_FLAG(item->item.flags, _item_attached_to_unit_bit))
			{
				long item_index = item_iterator.index;
				if (can_delete_item(item_index))
					object_delete(item_iterator.index);
			}
		}
	}

	{
		struct object_iterator biped_iterator;

		object_iterator_new(&biped_iterator, _object_mask_biped, 0);
		while (object_iterator_next(&biped_iterator))
		{
			struct object_datum *object;

			cutoff_time = 900;
			object = object_get(biped_iterator.index);

			if (object->object.idle_ticks > (short)cutoff_time &&
				TEST_FLAG(object->object.damage_flags, _object_dead_bit))
			{
				object_delete(biped_iterator.index);
			}
		}
	}

	return;
}

void game_engine_flag_reset(
	long weapon_index,
	real_point3d const *position)
{
	if (weapon_index != NONE)
	{
		struct weapon_datum *weapon = weapon_get(weapon_index);

		object_set_position(weapon_index, position, global_forward3d, global_up3d);
		object_reset(weapon_index);
		SET_FLAG(weapon->weapon.flags, _weapon_runtime_game_engine_active_bit, FALSE);
		weapon->item.last_owned_time = game_time_get();
		weapon->item.ignore_object_index = NONE;
	}

	return;
}

void ticks_to_unicode_time_string(
	long ticks,
	unsigned long character_count,
	wchar_t *string)
{
	long total_seconds;
	long minutes;
	long seconds;
	wchar_t minute_string[64];
	wchar_t second_string[64];

	total_seconds = ticks / 30;
	minutes = total_seconds / 60;
	seconds = total_seconds - minutes * 60;

	if (minutes == 0)
		usnprintf(minute_string, NUMBEROF(minute_string), L" ");
	else
		usnprintf(minute_string, NUMBEROF(minute_string), L"%d", minutes);

	if (seconds <= 9)
		usnprintf(second_string, NUMBEROF(second_string), L"0%d", seconds);
	else
		usnprintf(second_string, NUMBEROF(second_string), L"%d", seconds);

	usnprintf(string, character_count, L"%s:%s", minute_string, second_string);

	return;
}

void game_engine_playlist_initialize(
	void)
{
	game_engine_playlist_next(0, 0, 2);

	return;
}

void game_engine_playlist_begin(
	void)
{
	main_set_multiplayer_map_name(global_stage.map_name);
	game_set_game_variant(&global_stage.variant);

	if (!network_game_is_active())
		main_reset_map();

	return;
}

boolean game_engine_get_current_stage(
	struct game_variant *variant,
	char *map_name)
{
	match_assert("c:\\halo\\SOURCE\\game\\game_engine.c", 0x918, variant && map_name);

	csmemcpy(variant, &global_stage.variant, sizeof(*variant));
	csstrncpy(map_name, global_stage.map_name, sizeof(global_stage.map_name)-1);
	map_name[sizeof(global_stage.map_name)-1] = 0;

	return TRUE;
}

long list_index_to_weapon_definition_index(
	long weapon_list_index)
{
	long weapon_definition_index = NONE;

	if (weapon_list_index!=NONE)
	{
		struct game_globals *game_globals = scenario_get_game_globals();

		weapon_definition_index = game_globals_get_weapon(game_globals, weapon_list_index);
	}

	return weapon_definition_index;
}

long weapon_definition_index_to_list_index(
	long weapon_definition_index)
{
	struct game_globals *game_globals = scenario_get_game_globals();
	struct tag_reference *weapons = game_globals->weapon_list.count ?
		TAG_BLOCK_GET_ELEMENT(&game_globals->weapon_list, 0, struct tag_reference) :
		NULL;
	long result = NONE;
	long weapon_list_index;

	for (weapon_list_index = 0;
		weapon_list_index < game_globals->weapon_list.count;
		weapon_list_index++)
	{
		long definition_index = weapons[weapon_list_index].index;

		if (weapon_definition_index == definition_index)
		{
			result = weapon_list_index;
			break;
		}
	}

	return result;
}

void game_engine_state_message(
	long player_index,
	long state_message,
	long state_message_player_index)
{
	struct player_datum *player = player_get(player_index);

	player->state_message = state_message;
	player->state_message_player_index = state_message_player_index;

	return;
}

void game_engine_player_depower_active_camo(
	long player_index)
{
	if (player_index!=NONE)
	{
		struct player_datum *player = player_get(player_index);

		if (player->unit_index!=NONE)
		{
			struct unit_datum *unit = unit_get(player->unit_index);

			if (TEST_FLAG(unit->unit.flags, _unit_active_camouflaged_bit))
				unit->unit.active_camouflage = 0.5f;
		}
	}

	return;
}

real get_blink_alpha(
	void)
{
	long phase = system_milliseconds()%2700;

	return sin(phase * (3.14159265358979/2700.0));
}

void game_engine_rasterize_message(
	wchar_t const *message,
	real alpha)
{
	rectangle2d bounds;
	real_argb_color color;
	long font_index;
	long terminal_font_index;

	if (local_player_count())
		font_index = hud_globals->messaging.multi_player_font.index;
	else
		font_index = hud_globals->messaging.single_player_font.index;

	bounds = render.camera.window_bounds;

	terminal_font_index = interface_get_tag_index(_interface_font_terminal);
	draw_string_set_draw_mode(
		terminal_font_index,
		NONE,
		0,
		0,
		global_real_argb_white);

	color.alpha = alpha;
	color.red = 0.45882353f;
	color.green = 0.7294118f;
	color.blue = 1.0f;

	offset_rectangle2d(
		&bounds,
		-render.camera.viewport_bounds.x0,
		-render.camera.viewport_bounds.y0);

	bounds.y1 = (short)((5 * bounds.y0 + bounds.y1) / 6 + 9);
	bounds.y0 = bounds.y1 - 15;

	draw_string_set_draw_mode(font_index, NONE, 2, 8, &color);
	draw_string_set_color(&color);
	rasterizer_draw_unicode_string(&bounds, NULL, NULL, 0, message);
	draw_string_set_format(NONE, 0, 0);
	draw_string_set_tab_stops(NULL, 0);

	return;
}

/* port: who is talking in voice chat (network_voice.c), down the view's
left from below its middle: each machine's first player's name after its
speaker (this machine's own too, as it talks), VOICE_SPEAKERS_SCALE times
the HUD's text, in the colours of the names above players' heads (an ally's
or an enemy's: hud_player_name_color; in co-op every player an ally), this
machine's own white; while the scores are hidden */
#define VOICE_SPEAKERS_SCALE 0.8f

static void game_engine_rasterize_voice_speakers(
	void)
{
	enum
	{
		MAXIMUM_SPEAKER_ROWS = 8,
	};
	rectangle2d bounds = render.camera.window_bounds;
	long font_index = hud_get_font_index();
	long machines_listed[MULTIPLAYER_MAXIMUM_PLAYERS];
	short listed_count = 0;
	struct data_iterator iterator;
	struct player_datum *player;
	struct font_header *font;
	short line_height;
	short row = 0;
	short left;
	short top;
	struct player_datum *viewer;

	if (font_index == NONE || !network_voice_available())
		return;
	offset_rectangle2d(&bounds, -render.camera.viewport_bounds.x0, -render.camera.viewport_bounds.y0);
	font = font_definition_get(font_index);
	line_height = (short)(font->leading_height + font->descending_height + font->ascending_height);
	if (line_height <= 0)
		return;
	/* (laid out at full size, from the list's top left, and drawn scaled
	about it) */
	left = (short)(bounds.x0 + 16);
	top = (short)(bounds.y0 + (bounds.y1 - bounds.y0) * 55 / 100);
	rasterizer_text_set_scale(VOICE_SPEAKERS_SCALE, (real)left, (real)top);
	viewer = local_player_get_next(NONE) != NONE ?
		player_try_and_get(local_player_get_player_index(local_player_get_next(NONE))) : NULL;
	data_iterator_new(&iterator, player_data);
	while ((player = (struct player_datum *)data_iterator_next(&iterator)) != NULL && row < MAXIMUM_SPEAKER_ROWS)
	{
		long machine_index = player->network_player_data.machine_index;
		real_argb_color color;
		rectangle2d icon;
		rectangle2d text;
		short index;

		if (player->quit_out_of_game)
			continue;
		for (index = 0; index < listed_count && machines_listed[index] != machine_index; index++)
			;
		if (index < listed_count || listed_count >= (short)NUMBEROF(machines_listed))
			continue;
		machines_listed[listed_count++] = machine_index;
		if (!network_voice_machine_speaking(machine_index))
			continue;
		/* (the name after the speaker, which is centred on its capitals:
		where they are drawn, scaled) */
		text.x0 = (short)(left + line_height + 4);
		text.x1 = (short)(left + (bounds.x1 - left) / VOICE_SPEAKERS_SCALE);
		text.y0 = (short)(top + row * line_height);
		text.y1 = (short)(text.y0 + line_height);
		/* (allies as the names above heads have them: the same team) */
		if (player->local_player_index != NONE)
		{
			color.alpha = 1.0f;
			color.red = color.green = color.blue = 1.0f;
		}
		else
		{
			hud_player_name_color(!game_engine || (viewer && player->team_index == viewer->team_index), &color);
		}
		draw_string_set_draw_mode(font_index, NONE, 0, 0, &color);
		icon.x0 = left;
		icon.x1 = (short)(left + line_height * VOICE_SPEAKERS_SCALE);
		icon.y0 = (short)(top + (draw_unicode_string_capital_middle(&text, player->name) - top) * VOICE_SPEAKERS_SCALE -
			line_height * VOICE_SPEAKERS_SCALE / 2);
		icon.y1 = (short)(icon.y0 + line_height * VOICE_SPEAKERS_SCALE);
		network_voice_draw_icon(&icon, FALSE, 1.0f);
		rasterizer_draw_unicode_string(&text, NULL, NULL, 0, player->name);
		row++;
	}
	rasterizer_text_set_scale(1.0f, 0.0f, 0.0f);
}

static void game_engine_post_rasterize_in_game(
	void)
{
	long local_player_index;
	long player_index;
	struct player_datum *player;
	struct gamepad_state const *gamepad;
	real fade;

	local_player_index = render.local_player_index;
	player_index = local_player_get_player_index(local_player_index);
	player = player_get(player_index);

	match_assert(
		"c:\\halo\\SOURCE\\game\\game_engine.c",
		0x770,
		NONE != local_player_index);
	match_assert(
		"c:\\halo\\SOURCE\\game\\game_engine.c",
		0x771,
		NULL != game_engine || network_coop_active());

	if (game_engine && player)
		internal_rasterize_target_name(player_index);

	gamepad = input_get_gamepad_state(local_player_index);
	fade = game_engine_globals.hud_message_timers[local_player_index];
	if ((!gamepad ||
		!gamepad->buttons[_gamepad_binary_button_back]) &&
		(!game_engine || game_engine_globals.postgame_state != game_engine_mode_postgame_delay))
	{
		/* a frame is no longer a tick (render_interpolation.c): fade in half
		a second, not in 15 frames */
		fade -= 0.06666667f * main_get_seconds_elapsed() * TICKS_PER_SECOND;
	}
	else
	{
		fade += 0.06666667f * main_get_seconds_elapsed() * TICKS_PER_SECOND;
	}

	fade = PIN(fade, 0.0f, 1.0f);
	if (fade > 0.0f)
	{
		real alpha = linear_to_non_linear_alpha(fade);

		game_engine_rasterize_in_game_score(player_index, alpha);
	}
	else if (local_player_count() <= 1)
	{
		/* (port: the full-screen scoreboard's scroll forgotten) */
		game_engine_scoreboard_closed();
	}
	/* (port: who is talking, in the first view, while the scores are not
	shown) */
	if (fade <= 0.0f && local_player_index == local_player_get_next(NONE))
		game_engine_rasterize_voice_speakers();

	game_engine_globals.hud_message_timers[local_player_index] = fade;

	return;
}

long game_engine_player_get_team_index(
	long player_index)
{
	long team_index = 1;

	match_assert("c:\\halo\\SOURCE\\game\\game_engine.c", 0xC11, game_engine);

	if (!game_engine->team_index_override)
		/* port: 0 or 1 (another machine's player has no local player: -1) */
		team_index = PIN(player_get(player_index)->local_player_index % 2, 0, 1);

	return team_index;
}

void game_engine_update_player_always_invis(
	long player_index)
{
	if (game_engine)
	{
		if ((TEST_FLAG(global_variant.universal_variant.flags, _game_variant_always_invisible_bit) ||
			game_engine_test_trait(player_index, 1)) &&
			player_get(player_index)->unit_index!=NONE)
		{
			player_handle_powerup_minor(player_index, 0, 15);
		}
	}

	return;
}

boolean game_engine_player_has_flag(
	long player_index)
{
	boolean has_flag = FALSE;

	if (player_index!=NONE)
	{
		struct player_datum *player = player_get(player_index);

		if (player->unit_index!=NONE)
		{
			struct unit_datum *unit = unit_get(player->unit_index);
			long weapon_index;

			for (weapon_index = 0; weapon_index<MAXIMUM_WEAPONS_PER_UNIT; weapon_index++)
			{
				long weapon_object_index = unit->unit.weapon_object_indices[weapon_index];

				if (weapon_object_index!=NONE && weapon_is_flag(weapon_object_index))
				{
					has_flag = TRUE;
					break;
				}
			}
		}
	}

	return has_flag;
}

void game_show_score(
	long player_index,
	long score)
{
	game_show_score_extended(player_index, score, NONE);

	return;
}

void get_postgame_hilite_colors(
	real_argb_color *winner_color,
	real_argb_color *normal_color,
	real_argb_color *hilite_color)
{
	winner_color->red = 0.45882353f;
	winner_color->green = 0.7294118f;
	winner_color->blue = 1.0f;
	winner_color->alpha = 1.0f;

	normal_color->red = 1.0f;
	normal_color->green = 1.0f;
	normal_color->blue = 0.0f;
	normal_color->alpha = 1.0f;

	get_selected_color(hilite_color);

	return;
}

boolean game_engine_running(
	void)
{
	boolean running = game_engine!=NULL;

	return running;
}

/* port: whether the game is over and its scores are shown (where the
host's button starts the next, game_engine_update) */
boolean game_engine_showing_postgame(
	void)
{
	return game_engine != NULL && game_engine_globals.postgame_state == game_engine_mode_postgame_rasterize;
}

/* port: whether a game is being played: one of no game engine's (the
campaign, co-op), or its game not over (port/linux/game/latency_meter.c) */
boolean game_engine_in_play(
	void)
{
	return game_engine == NULL || game_engine_globals.postgame_state == game_engine_mode_active;
}

void game_variant_options_default(
	struct game_variant const *variant,
	struct game_variant_options *options)
{
	csmemset(options, 0, sizeof(*options));
	options->friendly_fire = _friendly_fire_on;
	options->loadout = _loadout_category;
	options->primary_weapon = _loadout_weapon_assault_rifle;
	options->secondary_weapon = _loadout_weapon_pistol;
	options->radar_players = variant &&
		!TEST_FLAG(variant->universal_variant.flags, _game_variant_draw_object_in_motion_sensor_bit) ?
		_radar_players_none : _radar_players_all;
	options->vehicle_set[0] = options->vehicle_set[1] =
		variant ? (byte)variant->universal_variant.vehicle_set : 0;
	/* port: OpenCE's gametypes' options at their defaults, with no mode */
	game_modes_options_default(options);

	return;
}

boolean game_engine_force_single_screen(
	void)
{
	boolean force_single_screen = FALSE;

	if (game_engine &&
		game_engine_globals.postgame_state>=game_engine_mode_postgame_rasterize_delay &&
		game_engine_globals.postgame_state<=game_engine_mode_postgame_rasterize)
	{
		force_single_screen = TRUE;
	}

	return force_single_screen;
}

void game_engine_dispose(
	void)
{
	if (game_engine)
	{
		if (game_engine->dispose)
			game_engine->dispose();

		game_engine = NULL;
	}

	return;
}

static void game_engine_update_player_no_shield(
	long player_index)
{
	struct player_datum *player;

	if (game_engine_has_shield(player_index))
		return;

	player = player_get(player_index);
	if (player->unit_index != NONE)
	{
		struct unit_datum *unit = unit_get(player->unit_index);

		unit->object.shield_vitality = 0.0f;
		unit->object.maximum_shield_vitality = 0.0f;
	}

	return;
}

static void game_engine_build_lighting(
	void)
{
	long player_count = 0;

	{
		struct data_iterator iterator;

		data_iterator_new(&iterator, player_data);
		while (data_iterator_next(&iterator))
			player_count++;
	}

	/* port: the Xbox turned dynamic lights off past 13 players, or past 8
	with two vehicles, and the flashlight off past 4 players or 4 vehicles.
	A match on a map with a few vehicles then had a flashlight, and the next
	one did not. The beam is a dynamic light, so both stay on. */

	if (player_count >= 5)
		SET_FLAG(game_engine_globals.flags, _game_engine_5_or_more_players_bit, TRUE);

	if (player_count >= 9)
		SET_FLAG(game_engine_globals.flags, _game_engine_9_or_more_players_bit, TRUE);

	return;
}

void game_engine_dispose_from_old_map(
	void)
{
	if (game_engine && game_engine->dispose_from_old_map)
		game_engine->dispose_from_old_map();

	return;
}

void game_engine_game_ending(
	void)
{
	if (game_engine && game_engine->game_ending)
		game_engine->game_ending();

	return;
}

void game_engine_game_starting(
	void)
{
	if (game_engine)
	{
		if (game_engine->game_starting)
			game_engine->game_starting();

		game_engine_build_lighting();
	}

	return;
}

void game_engine_statistics_append(
	long statistic)
{
	if (game_engine && game_engine->statistics_append)
		game_engine->statistics_append(statistic);

	return;
}

void game_engine_handle_client_message(
	void *message)
{
	if (game_engine && game_engine->handle_client_message)
		game_engine->handle_client_message(message);

	return;
}

void game_engine_handle_server_message(
	void *message)
{
	if (game_engine && game_engine->handle_server_message)
		game_engine->handle_server_message(message);

	return;
}

void game_engine_post_rasterize_objects(
	void)
{
	if (game_engine && game_engine->post_rasterize_objects)
		game_engine->post_rasterize_objects();

	return;
}

void game_engine_post_rasterize(
	void)
{
	if (game_engine)
	{
		switch (game_engine_globals.postgame_state)
		{
		case game_engine_mode_active:
		case game_engine_mode_postgame_delay:
			game_engine_post_rasterize_in_game();
			break;
		case game_engine_mode_postgame_rasterize_delay:
		case game_engine_mode_postgame_rasterize:
			game_engine_post_rasterize_post_game();
			break;
		default:
			match_assert(
				"c:\\halo\\SOURCE\\game\\game_engine.c",
				0x7B7,
				!"unreachable");
			break;
		}
	}
	/* port: network co-op's campaign has the scoreboard too, of names and
	pings (game_engine_rasterize_scoreboard) */
	else if (network_coop_active())
	{
		game_engine_post_rasterize_in_game();
	}

	return;
}

void game_engine_nonplayer_post_rasterize(
	void)
{
	if (game_engine)
	{
		switch (game_engine_globals.postgame_state)
		{
		case game_engine_mode_active:
		case game_engine_mode_postgame_delay:
			game_engine_press_start_to_begin();
			break;

		case game_engine_mode_postgame_rasterize_delay:
		case game_engine_mode_postgame_rasterize:
			{
				rectangle2d window_bounds;
				long local_player_index;

				game_engine_post_rasterize_post_game();
				window_bounds.x0 = 0;
				window_bounds.x1 = 640;
				window_bounds.y0 = 0;
				window_bounds.y1 = 480;

				for (local_player_index = 0;
					local_player_index < MAXIMUM_LOCAL_PLAYERS;
					local_player_index++)
				{
					render_ui_widgets_postgame(
						(short)local_player_index,
						&window_bounds);
					rumble_player_clear((short)local_player_index);
				}
			}
			break;

		default:
			match_assert(
				"c:\\halo\\SOURCE\\game\\game_engine.c",
				0xE3B,
				!"unreachable");
			break;
		}
	}

	return;
}

static void game_engine_update_teleporter(
	long player_index)
{
	static int blocked_message_delay = 0;
	static long fade_function = 0;
	static long screen_flash_type = 6;
	static real max_intensity = 1.0f;
	static real alpha = 0.5f;
	static real red = 0.35f;
	static real green = 1.0f;
	static real blue = 0.35f;
	static real duration = 1.0f;
	struct scenario *scenario = global_scenario_get();
	struct player_datum *player = player_get(player_index);
	struct unit_datum *unit;
	struct scenario_netgame_flag *source_flag;
	struct scenario_netgame_flag *destination_flag;
	real_vector3d forward;
	long source_flag_index;
	long destination_flag_index;

	if (player->unit_index == NONE)
		return;

	unit = unit_get(player->unit_index);
	if (player->teleporter_index != NONE)
	{
		struct scenario_netgame_flag *previous_flag = TAG_BLOCK_GET_ELEMENT(
			&scenario->netgame_flags,
			player->teleporter_index,
			struct scenario_netgame_flag);

		if (distance_squared3d(&previous_flag->position, &unit->object.position) > 1.0f)
			player->teleporter_index = NONE;
	}

	source_flag_index = NONE;
	find_netgame_flags(
		&unit->object.position,
		0.5f,
		0.0f,
		_netgame_flag_teleporter_source,
		NONE,
		1,
		&source_flag_index);
	if (source_flag_index == NONE ||
		source_flag_index == player->teleporter_index)
	{
		return;
	}

	source_flag = TAG_BLOCK_GET_ELEMENT(
		&scenario->netgame_flags,
		source_flag_index,
		struct scenario_netgame_flag);
	destination_flag_index = NONE;
	find_netgame_flags(
		NULL,
		0.0f,
		0.0f,
		_netgame_flag_teleporter_target,
		source_flag->team_index,
		1,
		&destination_flag_index);
	if (destination_flag_index != NONE)
	{
		destination_flag = TAG_BLOCK_GET_ELEMENT(
			&scenario->netgame_flags,
			destination_flag_index,
			struct scenario_netgame_flag);

		{
			struct collision_feature_list features;
			struct player_datum *unit_player;
			real_point3d position;
			real height;
			real radius;

			unit = unit_get(player->unit_index);
			forward = unit->object.forward;
			unit_player = player_get(player_index);
			biped_get_physics_pill(
				unit_player->unit_index,
				&position,
				&height,
				&radius);
			position = destination_flag->position;
			{
				struct collision_plane point_test_result;

				if (collision_get_features_in_sphere(
						0x200380,
						&position,
						height + radius * 2.0f,
						height,
						radius,
						NONE,
						&features) &&
					collision_features_test_point(
						&features,
						&position,
						&point_test_result))
				{
					if (point_test_result.object_index != NONE)
					{
						if (TEST_FLAG(
								_object_mask_unit,
								object_get(
									point_test_result.object_index)
									->object.type))
						{
							struct unit_datum *blocking_unit = unit_get(
								point_test_result.object_index);
							if (blocking_unit->unit.player_index != NONE)
							{
								struct player_datum *blocking_player =
									player_get(blocking_unit->unit.player_index);
								blocking_player->is_blocking_teleporter = TRUE;
								blocking_player->telefrag_timeout++;
							}
						}
					}

					if (blocked_message_delay > 0)
					{
						blocked_message_delay--;
						return;
					}

					blocked_message_delay = 120;
					{
						long string_list_index =
							tag_loaded('ustr', "ui\\multiplayer_game_text");
						wchar_t const *message;

						if (string_list_index != NONE)
							message = unicode_string_list_get_string(
								string_list_index,
								0x65);
						else
							message = L"";
						hud_print_message(
							unit_get_local_player_index(unit_player->unit_index),
							message);
					}
					return;
				}
			}
		}

		if (player->local_player_index != NONE)
		{
			game_engine_play_multiplayer_sound(_multiplayer_sound_teleporter_activate);
			if (player->local_player_index != NONE)
			{
				struct screen_flash_definition screen_flash = { 0 };

				screen_flash.fade_function = fade_function;
				screen_flash.type = screen_flash_type;
				screen_flash.duration = duration;
				screen_flash.priority = 2;
				screen_flash.max_intensity = max_intensity;
				screen_flash.zero_scale_factor = 0.0f;
				screen_flash.screen_flash_color.alpha = alpha;
				screen_flash.screen_flash_color.red = red;
				screen_flash.screen_flash_color.green = green;
				screen_flash.screen_flash_color.blue = blue;
				player_effect_screen_flash(
					player_index,
					&screen_flash,
					1.0f);
			}
		}

		{
			real angle =
				arctangent(forward.j, forward.i) +
				destination_flag->facing -
				source_flag->facing;

			forward.i = cosine(angle);
			forward.j = sine(angle);
			normalize3d(&forward);
			object_set_position(
				player->unit_index,
				&destination_flag->position,
				&forward,
				NULL);
			if (player->local_player_index != NONE)
				player_control_set_facing(player->local_player_index, &forward);

			player->teleporter_index = find_netgame_flag(
				&unit->object.position,
				1.0f,
				0.0f,
				_netgame_flag_teleporter_source,
				NONE);
		}
		return;
	}

	console_printf(
		FALSE,
		"failed to teleport %d",
		source_flag->team_index);

	return;
}

/* port: the gametype's vehicle respawn time (game_variant_options): the
host puts a vehicle destroyed, or left empty away from where the map put
it, back there after the time (the clients get the host's: the distributed
netcode). Vehicles otherwise stay where they are left (the Xbox game's). */
enum
{
	/* (a map places up to 80 vehicles, a Custom Edition map more: those
	past this never came back) */
	MAXIMUM_VEHICLE_HOMES = 1024
};

static struct
{
	long object_index;
	long definition_index;
	real_point3d position;
	real_vector3d forward;
	real_vector3d up;
	long idle_since;
	/* (the scenario name the map's scripts teleport; NONE when it had none) */
	short name_index;
} game_engine_vehicle_homes[MAXIMUM_VEHICLE_HOMES];
static short game_engine_vehicle_home_count = NONE;

/* still on the pad the map put it on: further than this, and empty, it can
come back */
#define VEHICLE_HOME_AWAY_SQUARED 4.0f

/* whether nobody rides the vehicle: no driver or gunner, and no rider among
its children (which hold its own weapons too) */
static boolean game_engine_vehicle_empty(
	struct unit_datum *vehicle)
{
	long child_index;

	if (vehicle->unit.driver_object_index != NONE || vehicle->unit.gunner_object_index != NONE)
		return FALSE;
	for (child_index = vehicle->object.first_child_object_index; child_index != NONE;
		child_index = object_get(child_index)->object.next_object_index)
	{
		if (object_get(child_index)->object.type == _object_type_biped)
			return FALSE;
	}
	return TRUE;
}

/* a living vehicle of the home's kind on its pad, but except, that no other
home follows (one a map's script put there in place of the home's: another
home's vehicle parked there is still that home's), else NONE */
static long game_engine_vehicle_home_on_pad(
	short home,
	long except)
{
	struct object_iterator iterator;

	object_iterator_new(&iterator, _object_mask_vehicle, 0);
	while (object_iterator_next(&iterator))
	{
		struct object_datum *object = object_get(iterator.index);
		real dx = object->object.position.x - game_engine_vehicle_homes[home].position.x;
		real dy = object->object.position.y - game_engine_vehicle_homes[home].position.y;
		real dz = object->object.position.z - game_engine_vehicle_homes[home].position.z;
		short other;

		if (iterator.index == except || TEST_FLAG(object->object.damage_flags, _object_dead_bit) ||
			object->definition_index != game_engine_vehicle_homes[home].definition_index ||
			dx * dx + dy * dy + dz * dz > VEHICLE_HOME_AWAY_SQUARED)
		{
			continue;
		}
		for (other = 0; other < game_engine_vehicle_home_count; other++)
		{
			if (other != home && game_engine_vehicle_homes[other].object_index == iterator.index)
				break;
		}
		if (other == game_engine_vehicle_home_count)
			return iterator.index;
	}
	return NONE;
}

/* the home follows this vehicle, which keeps (or takes) the home's name */
static void game_engine_vehicle_home_follow(
	short home,
	long object_index)
{
	game_engine_vehicle_homes[home].object_index = object_index;
	if (game_engine_vehicle_homes[home].name_index == NONE)
		game_engine_vehicle_homes[home].name_index = object_get(object_index)->object.name_index;
	else
		object_claim_scenario_name(object_index, game_engine_vehicle_homes[home].name_index);
}

/* the vehicle this home should follow. The recorded one, if it is still
alive. A map script often destroys that one and creates it again under the
same name, or in its place: that one is this home's vehicle, and a second is
not put on the pad with it. */
static long game_engine_vehicle_home_current(
	short home)
{
	long object_index = game_engine_vehicle_homes[home].object_index;
	struct unit_datum *vehicle = unit_try_and_get(object_index);
	long named_index;

	if (vehicle && !TEST_FLAG(vehicle->object.damage_flags, _object_dead_bit))
		return object_index;

	named_index = game_engine_vehicle_homes[home].name_index == NONE ? NONE :
		object_index_from_name_index(game_engine_vehicle_homes[home].name_index);
	vehicle = unit_try_and_get(named_index);
	if (vehicle && !TEST_FLAG(vehicle->object.damage_flags, _object_dead_bit))
	{
		game_engine_vehicle_homes[home].object_index = named_index;
		return named_index;
	}

	object_index = game_engine_vehicle_home_on_pad(home, NONE);
	if (object_index != NONE)
		game_engine_vehicle_home_follow(home, object_index);
	return object_index;
}

static void game_engine_update_vehicle_respawn(
	void)
{
	long respawn_ticks = game_variant_options_get()->vehicle_respawn_time * TICKS_PER_SECOND;
	long now = game_time_get();
	short index;

	if (respawn_ticks <= 0 || network_game_distributed_client() || now % TICKS_PER_SECOND)
		return;
	/* (where the map put them: the vehicles there at the first look) */
	if (game_engine_vehicle_home_count == NONE)
	{
		struct object_iterator iterator;

		game_engine_vehicle_home_count = 0;
		object_iterator_new(&iterator, _object_mask_vehicle, 0);
		while (object_iterator_next(&iterator) && game_engine_vehicle_home_count < MAXIMUM_VEHICLE_HOMES)
		{
			struct object_datum *object = object_get(iterator.index);
			short home = game_engine_vehicle_home_count++;

			game_engine_vehicle_homes[home].object_index = iterator.index;
			game_engine_vehicle_homes[home].definition_index = object->definition_index;
			game_engine_vehicle_homes[home].position = object->object.position;
			object_get_orientation(iterator.index, &game_engine_vehicle_homes[home].forward,
				&game_engine_vehicle_homes[home].up);
			game_engine_vehicle_homes[home].idle_since = NONE;
			game_engine_vehicle_homes[home].name_index = object->object.name_index;
		}
	}
	for (index = 0; index < game_engine_vehicle_home_count; index++)
	{
		long object_index = game_engine_vehicle_home_current(index);
		struct unit_datum *vehicle = unit_try_and_get(object_index);
		boolean waiting = TRUE;

		if (vehicle && !TEST_FLAG(vehicle->object.damage_flags, _object_dead_bit))
		{
			real dx = vehicle->object.position.x - game_engine_vehicle_homes[index].position.x;
			real dy = vehicle->object.position.y - game_engine_vehicle_homes[index].position.y;
			real dz = vehicle->object.position.z - game_engine_vehicle_homes[index].position.z;

			waiting = dx * dx + dy * dy + dz * dz > VEHICLE_HOME_AWAY_SQUARED && game_engine_vehicle_empty(vehicle);
		}
		if (!waiting)
			game_engine_vehicle_homes[index].idle_since = NONE;
		else if (game_engine_vehicle_homes[index].idle_since == NONE)
			game_engine_vehicle_homes[index].idle_since = now;
		else if (now - game_engine_vehicle_homes[index].idle_since >= respawn_ticks)
		{
			struct object_placement_data placement_data;
			long stale_index = game_engine_vehicle_homes[index].object_index;
			long named_index = game_engine_vehicle_homes[index].name_index == NONE ? NONE :
				object_index_from_name_index(game_engine_vehicle_homes[index].name_index);
			struct unit_datum *named = unit_try_and_get(named_index);
			long on_pad = game_engine_vehicle_home_on_pad(index, object_index);

			/* (a vehicle under the home's name that someone rides is theirs:
			the home follows it, and waits for it) */
			if (named && named_index != stale_index && !TEST_FLAG(named->object.damage_flags, _object_dead_bit) &&
				!game_engine_vehicle_empty(named))
			{
				game_engine_vehicle_homes[index].object_index = named_index;
				game_engine_vehicle_homes[index].idle_since = NONE;
				continue;
			}
			/* (the home's vehicle, empty or wrecked, goes; and the one under
			its name, but for one on the pad, which it follows instead of a
			new one: as many vehicles as before) */
			if (unit_try_and_get(stale_index))
				object_delete(stale_index);
			if (named && named_index != stale_index && named_index != on_pad)
				object_delete(named_index);
			if (on_pad != NONE)
			{
				game_engine_vehicle_home_follow(index, on_pad);
				game_engine_vehicle_homes[index].idle_since = NONE;
				continue;
			}
			object_placement_data_new(&placement_data, game_engine_vehicle_homes[index].definition_index, NONE);
			placement_data.position = game_engine_vehicle_homes[index].position;
			placement_data.forward = game_engine_vehicle_homes[index].forward;
			placement_data.up = game_engine_vehicle_homes[index].up;
			game_engine_vehicle_homes[index].object_index = object_new(&placement_data);
			object_claim_scenario_name(
				game_engine_vehicle_homes[index].object_index,
				game_engine_vehicle_homes[index].name_index);
			game_engine_vehicle_homes[index].idle_since = NONE;
		}
	}
}

void game_engine_update(
	void)
{
	if (game_engine)
	{
		game_engine_update_multiplayer_sound();
		game_engine_update_purge();
		game_engine_update_weapons();
		game_engine_update_item_spawn();
		game_engine_update_vehicle_respawn();

		{
			struct data_iterator player_iterator;

			data_iterator_new(&player_iterator, player_data);
			while (data_iterator_next(&player_iterator))
			{
				game_engine_update_player_no_shield(player_iterator.datum_index);
				game_engine_update_player_always_invis(player_iterator.datum_index);
				game_engine_update_teleporter(player_iterator.datum_index);

				/* (on a client of the distributed netcode each game type
				skips what the host decides, whose scores, flags, balls and
				hills it has: game_engine_read_network_state) */
				if (game_engine->player_update_each_tick)
					game_engine->player_update_each_tick(player_iterator.datum_index);
			}
		}

		if (game_engine->update)
			game_engine->update();

		switch (game_engine_globals.postgame_state)
		{
		case game_engine_mode_active:
			/* (a client of the distributed netcode ends the game when the host
			has) */
			if (
				!network_game_distributed_client() &&
				game_engine_should_end_game())
			{
				game_engine_end_game();
			}
			break;

		case game_engine_mode_postgame_delay:
			if (game_engine_globals.postgame_timer <= 2.0f &&
				!TEST_FLAG(game_engine_globals.flags, _game_engine_game_over_sound_disabled_bit))
			{
				sound_class_set_gain("", 0.0f, 30);
				sound_class_set_gain("ambient_nature", 0.2f, 30);
				sound_class_set_gain("ambient_machinery", 0.2f, 30);
				sound_class_set_gain("ambient_computers", 0.2f, 30);
				SET_FLAG(game_engine_globals.flags, _game_engine_game_over_sound_disabled_bit, TRUE);
			}

			game_engine_globals.postgame_timer -= 1.0f / TICKS_PER_SECOND;
			if (game_engine_globals.postgame_timer <= 0.0f)
			{
				struct network_game_server *server;

				game_engine_globals.postgame_progress = 0.0f;
				game_engine_globals.postgame_state = game_engine_mode_postgame_rasterize_delay;
				game_engine_globals.postgame_timer = 5.0f;

				{
					struct data_iterator player_iterator;
					struct player_datum *player;

					data_iterator_new(&player_iterator, player_data);
					player = (struct player_datum *)data_iterator_next(&player_iterator);
					while (player)
					{
						if (player->unit_index != NONE)
							unit_kill(player->unit_index);
						if (player->local_player_index != NONE)
							rumble_player_clear(player->local_player_index);

						player = (struct player_datum *)data_iterator_next(&player_iterator);
					}
				}

				{
					struct object_iterator object_iterator;

					object_iterator_new(
						&object_iterator,
						_object_mask_vehicle,
						0);
					while (object_iterator_next(&object_iterator))
					{
						object_delete(object_iterator.index);
					}
				}

				server = global_network_game_server_get();
				if (server)
					network_game_server_switch_to_postgame(
						global_network_game_server_get());
			}
			break;

		case game_engine_mode_postgame_rasterize_delay:
		case game_engine_mode_postgame_rasterize:
			break;

		default:
			match_assert(
				"c:\\halo\\SOURCE\\game\\game_engine.c",
				0xA03,
				!"unreachable");
			break;
		}
	}

	return;
}


enum game_engine_message
{
	_game_engine_message_welcome = 0,
	_game_engine_message_killed_by_unknown,
	_game_engine_message_killed_by_biped,
	_game_engine_message_killed_by_vehicle,
	_game_engine_message_killed_by_player,
	_game_engine_message_killed_by_friendly_fire,
	_game_engine_message_killed_by_self,
	_game_engine_message_double_kill,
	_game_engine_message_killed_enemy,
	_game_engine_message_triple_kill,
	_game_engine_message_multi_kill,
	_game_engine_message_five_kills_in_row,
	_game_engine_message_ten_kills_in_a_row,
	_game_engine_message_killed_friendly,
	_game_engine_message_multi_kill_with_score,
	_game_engine_message_triple_kill_with_score,
	_game_engine_message_double_kill_with_score,
	_game_engine_message_ten_kills_in_a_row_with_score,
	_game_engine_message_five_kills_in_row_with_score,
	_game_engine_message_killed_enemy_with_score,
	_game_engine_message_winner,
	_game_engine_message_team_winner,
	_game_engine_message_show_score,
	_game_engine_message_odd_man_out,
	_game_engine_message_out_of_lives,
	_game_engine_message_respawn_timer,
	_game_engine_message_waiting_for_space_to_clear,
	_game_engine_message_player_quit_self,
	_game_engine_message_quit,
	_game_engine_message_press_back_for_score,
	_game_engine_message_time_left,
	NUMBER_OF_GAME_ENGINE_MESSAGES,
};

enum
{
	_equipment_created_at_rest_bit = 0,
};

/* port: each player's friendly fire penalty (game_variant_options), added to
his next respawn: by the player's absolute index */
static short game_engine_betrayal_penalty[HALO_PORT_MAXIMUM_NETWORK_PLAYERS];

void game_engine_player_killed(
	long killing_player_index,
	long killing_object_index,
	long dead_player_index,
	boolean friendly_fire)
{
	struct player_datum *dead_player = player_get(dead_player_index);
	boolean player_kill;
	boolean same_player;
	boolean valid_players;
	long message = NONE;

	match_assert(
		"c:\\halo\\SOURCE\\game\\game_engine.c",
		0xA31,
		dead_player_index != NONE);

	if (!game_engine)
		return;

	/* port: the death of an MCC team change is no one's kill (mcc_ui_teams.c) */
	mcc_ui_team_respawn_death(&killing_player_index, &killing_object_index, dead_player_index, &friendly_fire);
	/* port: and a dedicated server's !switch's (server_team_switch.c) */
	server_team_switch_player_killed(&killing_player_index, &killing_object_index, dead_player_index,
		&friendly_fire);

	/* the distributed netcode: a client's copy of a death has the host's
	killer (port/linux/game/network_distributed.c) */
	network_distributed_player_killed(&killing_player_index, &killing_object_index, dead_player_index,
		&friendly_fire);
	/* port: a killer who has left the game since is no one's kill */
	if (killing_player_index != NONE && !player_try_and_get(killing_player_index))
		killing_player_index = NONE;
	/* the host's kill of a player who quit, ahead of this client's clock
	(game_update_quit_players has not come to its time yet) */
	if (network_game_distributed_client() && dead_player->quit_out_of_game_time != NONE &&
		killing_player_index == dead_player_index)
	{
		dead_player->quit_out_of_game = TRUE;
	}
	dead_player->death_time = game_time_get();
	if (game_engine->player_killed_player)
	{
		game_engine->player_killed_player(
			killing_player_index,
			killing_object_index,
			dead_player_index,
			friendly_fire);
	}
	/* port: OpenCE's gametypes' (Hardcore's restore of the killer) */
	game_modes_player_killed(killing_player_index, dead_player_index, friendly_fire);

	same_player = killing_player_index == dead_player_index;
	if (killing_player_index != NONE && dead_player_index != NONE)
		valid_players = TRUE;
	else
		valid_players = FALSE;
	player_kill = !friendly_fire && valid_players && !same_player;

	dead_player->respawn_timer =
		dead_player->respawn_penalty + global_variant.universal_variant.respawn_time;
	if (global_variant.universal_variant.respawn_time_growth > 0)
	{
		dead_player->respawn_penalty += global_variant.universal_variant.respawn_time_growth;
		dead_player->respawn_penalty =
			MIN(
				dead_player->respawn_penalty,
				global_variant.universal_variant.respawn_time_growth * 5);
		if (player_kill && killing_player_index != NONE)
		{
			struct player_datum *killing_player =
				player_get(killing_player_index);
			killing_player->respawn_penalty -= global_variant.universal_variant.respawn_time_growth;
			killing_player->respawn_penalty =
				MAX(killing_player->respawn_penalty, 0);
		}
	}

	if (!player_kill)
		dead_player->respawn_timer += global_variant.universal_variant.suicide_penalty;

	/* port: a dedicated server's !switch moves the dead here, as the auto
	team balance below does, once their unit has dropped what it carried
	(server_team_switch.c) */
	server_team_switch_player_dead(dead_player_index);

	/* port: the gametype's auto team balance (game_variant_options): the host
	moves the dead to the other team when his is two or more bigger (his
	next unit is the other team's, which the clients take it from:
	network_player_attach_unit) */
	/* port: (not Infection's teams, which are its roles: infection.c) */
	if (global_variant.universal_variant.teams && game_variant_options_get()->auto_team_balance && !infection_enabled() &&
		!network_game_distributed_client() && VALID_INDEX(dead_player->team_index, 2) && !dead_player->quit_out_of_game)
	{
		struct data_iterator iterator;
		struct player_datum *player;
		short count[2] = { 0, 0 };

		data_iterator_new(&iterator, player_data);
		while ((player = (struct player_datum *)data_iterator_next(&iterator)) != NULL)
		{
			if (!player->quit_out_of_game && VALID_INDEX(player->team_index, 2))
				count[player->team_index]++;
		}
		if (count[dead_player->team_index] >= count[1 - dead_player->team_index] + 2)
		{
			struct network_game_server *server = global_network_game_server_get();
			struct network_game *game = server ? network_game_server_get_game(server) : NULL;
			long list_index = dead_player->network_player_data.player_list_index;

			dead_player->team_index = 1 - dead_player->team_index;
			dead_player->network_player_data.team_index = (char)dead_player->team_index;
			/* (and the host's game record, which late joins count and the
			lobby shows) */
			if (game && VALID_INDEX(list_index, HALO_PORT_MAXIMUM_NETWORK_PLAYERS))
				game->players[list_index].team_index = (char)dead_player->team_index;
		}
	}

	/* port: Infection's respawn time (infection.c), the betrayal penalty's
	after it */
	if (infection_enabled())
		dead_player->respawn_timer = game_variant_options_get()->infection_respawn_time * TICKS_PER_SECOND;

	/* port: the gametype's friendly fire penalty: the team killer's next
	respawn later (the dead's own, if he has one waiting) */
	{
		long dead_absolute = DATUM_INDEX_TO_ABSOLUTE_INDEX(dead_player_index);

		if (dead_absolute < HALO_PORT_MAXIMUM_NETWORK_PLAYERS)
		{
			dead_player->respawn_timer += game_engine_betrayal_penalty[dead_absolute];
			game_engine_betrayal_penalty[dead_absolute] = 0;
		}
		if (friendly_fire && valid_players && !same_player && global_variant.universal_variant.teams &&
			DATUM_INDEX_TO_ABSOLUTE_INDEX(killing_player_index) < HALO_PORT_MAXIMUM_NETWORK_PLAYERS)
		{
			game_engine_betrayal_penalty[DATUM_INDEX_TO_ABSOLUTE_INDEX(killing_player_index)] =
				(short)(game_variant_options_get()->friendly_fire_penalty * TICKS_PER_SECOND);
		}
	}

	/* port: (Infection's respawn time may be shorter) */
	if (!infection_enabled())
		dead_player->respawn_timer = MAX(dead_player->respawn_timer, 90);
	dead_player = player_get(dead_player_index);

	if (!dead_player->quit_out_of_game)
	{
		if (killing_player_index == NONE)
		{
			if (killing_object_index == NONE)
				message = _game_engine_message_killed_by_unknown;
			else
			{
				struct object_datum *object = object_get(killing_object_index);

				unit_try_and_get(killing_object_index);
				switch (object->object.type)
				{
				case _object_type_biped:
					message = _game_engine_message_killed_by_biped;
					break;
				case _object_type_vehicle:
					message = _game_engine_message_killed_by_vehicle;
					break;
				default:
					message = _game_engine_message_killed_by_unknown;
					break;
				}
			}
		}
		else if (killing_player_index == dead_player_index)
			message = _game_engine_message_killed_by_self;
		else
			message =
				_game_engine_message_killed_by_player + (friendly_fire != FALSE);

		game_show_score_extended(
			dead_player_index,
			message,
			killing_player_index);
	}
	else
	{
		struct data_iterator iterator;

		data_iterator_new(&iterator, player_data);
		while (data_iterator_next(&iterator))
		{
			multiplayer_message(
				iterator.datum_index,
				_game_engine_message_quit,
				dead_player_index);
		}
	}

	if (message == _game_engine_message_killed_by_friendly_fire)
	{
		if (killing_player_index != NONE)
		{
			multiplayer_message(
				killing_player_index,
				_game_engine_message_killed_friendly,
				dead_player_index);
		}
		else
		{
			struct data_iterator iterator;

			data_iterator_new(&iterator, player_data);
			while (data_iterator_next(&iterator))
			{
				multiplayer_message(
					iterator.datum_index,
					_game_engine_message_killed_friendly,
					dead_player_index);
			}
		}
	}
	else if (message == _game_engine_message_killed_by_player)
	{
		struct player_datum *killing_player =
			player_get(killing_player_index);

		if (killing_player->statistics.multiple_kills >= 4)
		{
			message = _game_engine_message_multi_kill;
		}
		else if (killing_player->statistics.multiple_kills == 3)
			message = _game_engine_message_triple_kill;
		else if (killing_player->statistics.multiple_kills == 2)
			message = _game_engine_message_double_kill;
		else if (killing_player->statistics.kills_in_a_row == 5)
			message = _game_engine_message_five_kills_in_row;
		/* port: none in a row is no spree. A network client's copy of the
		killer's statistics can have this kill credited to no one, or to
		someone else, before the host's statistics come (the Xbox's credited
		the killer here, so never had none): Running Riot was announced */
		else if (killing_player->statistics.kills_in_a_row <= 0)
			message = _game_engine_message_killed_enemy;
		else
		{
			message = (killing_player->statistics.kills_in_a_row % 5)
				? _game_engine_message_killed_enemy
				: _game_engine_message_ten_kills_in_a_row;
		}

		game_show_score_extended(
			killing_player_index,
			message,
			dead_player_index);
	}

	return;
}

void game_engine_update_non_deterministic(
	real delta_seconds)
{
	if (game_engine)
	{
		switch (game_engine_globals.postgame_state)
		{
		case game_engine_mode_postgame_rasterize_delay:
			rumble_clear_all_now();
			game_engine_globals.postgame_timer -= delta_seconds;
			if (game_engine_globals.postgame_timer <= 0.0f)
				game_engine_globals.postgame_state = game_engine_mode_postgame_rasterize;
			break;

		case game_engine_mode_postgame_rasterize:
			rumble_clear_all_now();
			game_engine_globals.postgame_progress += delta_seconds;
			if (game_engine_globals.postgame_progress > 1.0f)
				game_engine_globals.postgame_progress = 1.0f;

			/* port: or the touchscreen's buttons for A and B
			(port/linux/game/touch_postgame.c) */
			if (test_any_gamepad_button(0) || test_any_gamepad_button(12) ||
				touch_postgame_pressed(_gamepad_analog_button_a))
			{
				if (global_network_game_server_get())
				network_game_server_reset_to_pregame(
					global_network_game_server_get());
			}
			else if (test_any_gamepad_button(1) || test_any_gamepad_button(13) ||
				touch_postgame_pressed(_gamepad_analog_button_b))
			{
				network_game_abort();
			}
			break;
		}
	}

	return;
}

boolean game_engine_display_team_indicators(
	void)
{
	boolean display = FALSE;

	if (game_engine)
		display = TEST_FLAG(global_variant.universal_variant.flags, _game_variant_allow_friendly_navpoints_bit) && global_variant.universal_variant.teams;

	return display;
}

boolean game_engine_can_score(
	void)
{
	boolean can_score = TRUE;

	if (game_engine)
		can_score = game_engine_globals.postgame_state==game_engine_mode_active;

	return can_score;
}

boolean game_engine_infinite_grenades(
	long player_index)
{
	boolean infinite_grenades = FALSE;

	if (game_engine_running() && player_index!=NONE)
		infinite_grenades = game_engine_infinite_grenades_internal();

	return infinite_grenades;
}

boolean game_engine_has_shield(
	long player_index)
{
	boolean has_shield = TRUE;

	if (game_engine && player_index!=NONE)
		has_shield = !TEST_FLAG(global_variant.universal_variant.flags, _game_variant_no_shields_bit);

	return has_shield;
}

/* port: the local player whose motion sensor is drawn (motion_sensor.c),
for the gametype's FRIENDS radar (game_variant_options) */
static short game_engine_motion_sensor_local_player = NONE;

void game_engine_motion_sensor_viewer(
	short local_player_index)
{
	game_engine_motion_sensor_local_player = local_player_index;
}

boolean game_engine_draw_object_in_motion_sensor(
	long unit_index)
{
	boolean draw_object = TRUE;

	if (game_engine)
		draw_object = TEST_FLAG(global_variant.universal_variant.flags, _game_variant_draw_object_in_motion_sensor_bit);
	/* port: friends only: the viewer's team's */
	if (draw_object && game_engine && global_variant.universal_variant.teams &&
		game_variant_options_get()->radar_players == _radar_players_friends &&
		game_engine_motion_sensor_local_player != NONE)
	{
		long player_index = local_player_get_player_index(game_engine_motion_sensor_local_player);
		struct player_datum *viewer = player_index != NONE ? player_try_and_get(player_index) : NULL;

		draw_object = viewer && object_get(unit_index)->object.owner_team_index == viewer->team_index;
	}

	return draw_object;
}

boolean game_engine_hud_draw_motion_sensor(
	long player_index)
{
	boolean draw_motion_sensor = TRUE;

	if (game_engine)
	{
		boolean default_draw_motion_sensor;

		draw_motion_sensor =
			TEST_FLAG(global_variant.universal_variant.flags, _game_variant_draw_object_in_motion_sensor_bit);
		default_draw_motion_sensor = global_variant.universal_variant.goal_radar == _radar_motion_tracker;

		if (global_variant.game_engine_index == game_engine_slayer && !global_variant.game_engine_variant.slayer.kill_in_order)
			default_draw_motion_sensor = FALSE;

		draw_motion_sensor |= default_draw_motion_sensor;
		/* port: Hardcore has none (port/linux/game/game_modes.c) */
		if (game_modes_motion_sensor_hidden())
			draw_motion_sensor = FALSE;
	}

	return draw_motion_sensor;
}

boolean game_engine_player_has_stealth_weapon(
	long player_index)
{
	boolean has_stealth_weapon = FALSE;

	if (player_index != NONE)
	{
		struct player_datum *player = player_get(player_index);
		if (player->unit_index != NONE)
		{
			struct unit_datum *unit = unit_get(player->unit_index);
			long current_weapon_index = unit->unit.current_weapon_index;

			if (current_weapon_index != NONE)
			{
				long weapon_index =
					unit->unit.weapon_object_indices[current_weapon_index];
				if (weapon_index != NONE)
				{
					struct weapon_datum *weapon = weapon_get(weapon_index);
					if (weapon->definition_index != NONE)
					{
						struct weapon_definition *weapon_definition =
							weapon_definition_get(weapon->definition_index);
						has_stealth_weapon = TEST_FLAG(
							weapon_definition->weapon.flags,
							_weapon_does_not_depower_active_camo_bit);
					}
				}
			}
		}
	}

	return has_stealth_weapon;
}

void game_engine_weapon_fired(
	long player_index)
{
	if (game_engine && player_index != NONE)
	{
		struct player_datum *player = player_get(player_index);
		long unit_index = player->unit_index;

		if (unit_index != NONE)
		{
			struct unit_datum *unit = unit_get(unit_index);
			long weapon_index = unit_inventory_get_weapon(
				unit_index,
				unit_get(unit_index)->unit.current_weapon_index);
			real active_camouflage_decrease = 0.1f;

			if (game_engine_player_has_stealth_weapon(player_index))
			{
				active_camouflage_decrease = 0.0f;
			}
			else if (weapon_index != NONE)
			{
				struct weapon_datum *weapon = weapon_get(weapon_index);
				struct weapon_definition *weapon_definition =
					weapon_definition_get(weapon->definition_index);

				if (0.0f != weapon_definition->weapon.active_camo_ding)
					active_camouflage_decrease =
						weapon_definition->weapon.active_camo_ding;
			}

			if (unit->unit.active_camouflage >= 0.05f)
			{
				unit->unit.active_camouflage -= active_camouflage_decrease;
				unit->unit.cause_for_camo_regrowth = 1;
				unit->unit.active_camouflage =
					MAX(0.05f, unit->unit.active_camouflage);
			}
		}
	}

	return;
}

wchar_t *get_place_name(
	struct game_engine_place place)
{
	long lookup_index;
	long string_list_index;

	/* port: a session can have more places than the string list names */
	match_vassert(
		"c:\\halo\\SOURCE\\game\\game_engine.c",
		0x1316,
		place.place < MULTIPLAYER_MAXIMUM_PLAYERS,
		"place.place < maximum_places");

	if (TEST_FLAG(place.flags, _place_two_groups) && TEST_FLAG(place.flags, _place_tied))
		lookup_index = 35;
	else if (TEST_FLAG(place.flags, _place_two_groups) && place.place == 0)
		lookup_index = 33;
	else if (TEST_FLAG(place.flags, _place_two_groups) && place.place == 1)
		lookup_index = 34;
	else if (TEST_FLAG(place.flags, _place_all_tied))
		lookup_index = 32;
	/* port: places past the 16th are spelled out; a tie is marked like the
	string list's tied places */
	else if (place.place >= NUMBER_OF_PLACE_STRINGS)
		return place_ordinal_string(place.place, TEST_FLAG(place.flags, _place_tied));
	else
	{
		lookup_index = place.place;
		if (TEST_FLAG(place.flags, _place_tied))
			lookup_index += 16;
	}

	match_vassert(
		"c:\\halo\\SOURCE\\game\\game_engine.c",
		0x1331,
		lookup_index != NONE,
		"NONE != lookup_index");

	string_list_index = tag_loaded('ustr', "ui\\multiplayer_game_text");
	if (string_list_index != NONE)
		return unicode_string_list_get_string(
			string_list_index,
			lookup_index + 0x66);

	return L"";
}

long find_netgame_flags(
	real_point3d const *position,
	real radius,
	real height,
	short type,
	short team_index,
	long maximum_count,
	long *flag_indices)
{
	real radius_squared = radius * radius;
	long found_count = 0;
	/* port: a long counter, for a map's long count */
	long flag_index;
	struct scenario *scenario;

	scenario = global_scenario_get();

	for (flag_index = 0; flag_index < scenario->netgame_flags.count; flag_index++)
	{
		struct scenario_netgame_flag *flag = TAG_BLOCK_GET_ELEMENT(
			&scenario->netgame_flags,
			flag_index,
			struct scenario_netgame_flag);

		if (type != NONE && type != flag->type)
			continue;

		if (team_index != NONE && team_index != flag->team_index)
			continue;

		if (position)
		{
			if (radius >= 0.0f &&
				distance_squared3d(&flag->position, position) > radius_squared)
			{
				continue;
			}

			if (height > 0.0f &&
				fabs(flag->position.z - position->z) > height)
			{
				continue;
			}
		}

		if (found_count < maximum_count)
			flag_indices[found_count++] = flag_index;
	}

	return found_count;
}

long find_netgame_flag(
	real_point3d const *position,
	real radius,
	real height,
	short type,
	short team_index)
{
	long flag_index = NONE;

	find_netgame_flags(
		position,
		radius,
		height,
		type,
		team_index,
		1,
		&flag_index);

	return flag_index;
}

/* playlist data: in the Aug-15-2001 build the playlist functions were the only users of these two globals;
   this build no longer reads them. their definition point here, with the playlist code, is inferred (the
   Aug-2001 users and the later builds' layout), not attested */
void *global_autogenerate_list = NULL;
long global_autogenerate_count = 0;

void game_engine_playlist_next(
	long parameter0,
	long parameter1,
	long playlist_type)
{
	char *map_name;
	struct game_variant variant;

	csstrcpy(global_stage.map_name, "levels\\test\\carousel\\carousel");
	/* port: a dedicated server's rotation's next map, not carousel, which it
	may not have (port/linux/game/server_director.c) */
	if (halo_server_mode() && server_director_first_level())
	{
		csstrncpy(global_stage.map_name, server_director_first_level(), sizeof(global_stage.map_name) - 1);
		global_stage.map_name[sizeof(global_stage.map_name) - 1] = 0;
	}
	map_name = main_get_multiplayer_map_name();
	if (map_name && map_name[0])
	{
		csstrncpy(global_stage.map_name, map_name, sizeof(global_stage.map_name) - 1);
		global_stage.map_name[sizeof(global_stage.map_name) - 1] = 0;
	}

	if (player_ui_game_variant_specified(&variant))
		csmemcpy(&global_stage.variant, &variant, sizeof(global_stage.variant));

	return;
}

/* port: whether a team game ends as soon as only one team's players are in
it, the Xbox game's rule: a dedicated server's server.one_team_ends_game (off
by default), a player's host's network.one_team_ends_game (on) */
static boolean game_engine_one_team_ends_game(
	void)
{
	return halo_server_mode() ?
		config_boolean("server.one_team_ends_game") :
		config_boolean("network.one_team_ends_game");
}

/* port: whether players of more than one team are in the game, living, dead
or out of lives (game_engine_one_team_ends_game) */
static boolean game_engine_multiple_teams_playing(
	void)
{
	struct data_iterator iterator;
	struct player_datum *player;
	long team_index = NONE;

	data_iterator_new(&iterator, player_data);
	while ((player = (struct player_datum *)data_iterator_next(&iterator)) != NULL)
	{
		if (player->quit_out_of_game)
			continue;
		if (team_index != NONE && player->team_index != team_index)
			return TRUE;
		team_index = player->team_index;
	}

	return FALSE;
}

/* (defined below) */
static boolean game_engine_player_is_odd_man_out(long player_index);

/* port: whether any player in the game can still play: living, or waiting
to respawn with lives left (multiple_teams_alive's test) */
static boolean game_engine_any_player_alive(
	void)
{
	struct data_iterator iterator;
	struct player_datum *player;

	data_iterator_new(&iterator, player_data);
	while ((player = (struct player_datum *)data_iterator_next(&iterator)) != NULL)
	{
		if (!player->quit_out_of_game &&
			(player->unit_index != NONE ||
				(!game_engine_player_is_odd_man_out(iterator.datum_index) &&
					!game_engine_player_is_out_of_lives(iterator.datum_index))))
		{
			return TRUE;
		}
	}

	return FALSE;
}

boolean game_engine_should_end_game(
	void)
{
	boolean should_end_game = FALSE;

	/* port: Infection's round ends by its own rules (infection.c) */
	if (infection_enabled())
		return FALSE;
	/* port: without the rule (game_engine_one_team_ends_game), a game only
	one team's players are in goes on: the others still loading as it
	started, or gone. One whose other team is out of lives still ends, and
	so does one where no player can play any more (the only team out of
	lives) */
	if (game_engine && !multiple_teams_alive() &&
		(game_engine_one_team_ends_game() || game_engine_multiple_teams_playing() ||
			!game_engine_any_player_alive()))
	{
		should_end_game = TRUE;
	}
	/* port: the gametype's time limit (game_variant_options) */
	if (game_engine && game_variant_options_get()->time_limit > 0 &&
		game_time_get() >= game_variant_options_get()->time_limit * 60L * TICKS_PER_SECOND)
	{
		should_end_game = TRUE;
	}

	return should_end_game;
}

void game_engine_clear_goal_position(
	short goal_index)
{
	/* port: a goal of the table only */
	if (!VALID_INDEX(goal_index, NUMBEROF(global_goal)))
		return;
	csmemset(&global_goal[goal_index], 0, sizeof(struct netgame_goal));

	return;
}

/* port: a flag or ball deleted on a client of the distributed netcode, on
the host's word (port/linux/game/network_objects.c): the goals its game
type's objective_weapon_update set at it each tick, which nothing else
clears (ctf: its team's, and its carrier's goal; oddball: its team's) */
void game_engine_objective_weapon_deleted(
	long weapon_index)
{
	short team_index = weapon_get(weapon_index)->object.owner_team_index;

	if (!game_engine || team_index == NONE)
		return;
	if (global_variant.game_engine_index == game_engine_ctf)
	{
		game_engine_clear_goal_position(team_index);
		game_engine_clear_goal_position(team_index + 2);
	}
	else if (global_variant.game_engine_index == game_engine_oddball)
	{
		game_engine_clear_goal_position(team_index);
	}

	return;
}

long get_flag_definition_index(
	void)
{
	struct game_globals *game_globals;
	struct game_globals_multiplayer_information *multiplayer_information;

	global_scenario_get();
	game_globals = scenario_get_game_globals();
	multiplayer_information = TAG_BLOCK_GET_ELEMENT(
		&game_globals->multiplayer_information,
		0,
		struct game_globals_multiplayer_information);

	return multiplayer_information->flag.index;
}

long get_ball_definition_index(
	void)
{
	struct game_globals *game_globals;
	struct game_globals_multiplayer_information *multiplayer_information;

	global_scenario_get();
	game_globals = scenario_get_game_globals();
	multiplayer_information = TAG_BLOCK_GET_ELEMENT(
		&game_globals->multiplayer_information,
		0,
		struct game_globals_multiplayer_information);

	return multiplayer_information->ball.index;
}

void game_engine_override_map_name(
	char const *map_name)
{
	if (map_name && map_name[0])
		csstrncpy(global_stage.map_name, map_name, sizeof(global_stage.map_name)-1);

	return;
}

void game_engine_override_game_variant(
	struct game_variant const *variant)
{
	if (variant)
		csmemcpy(&global_stage.variant, variant, sizeof(global_stage.variant));

	return;
}

void game_engine_switch_to_postgame(
	void)
{
	if (game_engine_globals.postgame_state==game_engine_mode_active)
	{
		if (global_network_game_server_get())
		{
			game_engine_globals.postgame_state = game_engine_mode_postgame_delay;
			game_engine_globals.postgame_timer = 7.0f;
		}
		else
		{
			game_engine_globals.postgame_state = game_engine_mode_postgame_rasterize;
		}
	}

	return;
}

void game_engine_load_stage(
	char const *map_name)
{
	if (!map_name || csstrcmp(global_stage.map_name, map_name))
		main_set_multiplayer_map_name(global_stage.map_name);

	game_set_game_variant(&global_stage.variant);
	if (!network_game_is_active())
		main_reset_map();

	return;
}

void game_engine_end_game(
	void)
{
	if (game_engine_globals.postgame_state==game_engine_mode_active)
	{
		game_engine_globals.postgame_state = game_engine_mode_postgame_delay;
		game_engine_globals.postgame_timer = 7.0f;
		game_engine_play_multiplayer_sound(_multiplayer_sound_game_over);
		ui_widgets_close_all();
	}

	return;
}

void game_engine_player_damaged_player(
	long damaging_player_index,
	long dead_player_index,
	boolean damage_type)
{
	match_assert("c:\\halo\\SOURCE\\game\\game_engine.c", 0xA20, dead_player_index != NONE);

	/* (a client of the distributed netcode replaying the host's damage has
	the host's game type state, game_engine_read_network_state) */
	if (network_game_distributed_client())
		return;
	if (game_engine && game_engine->player_damaged_player)
		game_engine->player_damaged_player(damaging_player_index, dead_player_index, damage_type);

	return;
}

static boolean game_engine_player_is_odd_man_out(
	long player_index)
{
	struct player_datum *player = player_get(player_index);
	boolean result = FALSE;

	if (global_variant.universal_variant.odd_man_out && player->unit_index==NONE)
	{
		struct data_iterator iterator;
		struct player_datum *other_player;

		result = TRUE;
		data_iterator_new(&iterator, player_data);
		other_player = (struct player_datum *)data_iterator_next(&iterator);
		while (other_player)
		{
			if (other_player->unit_index==NONE &&
				other_player!=player &&
				(other_player->death_time>player->death_time ||
					(other_player->death_time==player->death_time &&
						DATUM_INDEX_TO_ABSOLUTE_INDEX(player_index)<
							DATUM_INDEX_TO_ABSOLUTE_INDEX(iterator.datum_index))))
			{
				result = FALSE;
			}

			other_player = (struct player_datum *)data_iterator_next(&iterator);
		}
	}

	return result;
}

boolean game_engine_player_is_out_of_lives(
	long player_index)
{
	boolean out_of_lives = FALSE;

	if (global_variant.universal_variant.lives>0)
	{
		struct player_datum *player = player_get(player_index);

		if (player->unit_index==NONE && player->statistics.deaths>=global_variant.universal_variant.lives)
			out_of_lives = TRUE;
	}

	return out_of_lives;
}

short game_engine_player_get_custom_motion_sensor_positions(
	long player_index,
	real_point2d *positions,
	byte *goal_indices,
	short maximum_count)
{
	short count = 0;

	if (game_engine && global_variant.universal_variant.goal_radar == _radar_motion_tracker && player_index != NONE)
	{
		struct player_datum *player = player_get(player_index);
		long goal_index;

		/* port: every goal, one per player in the native builds' sessions */
		for (goal_index = 0; goal_index < (long)NUMBEROF(global_goal); goal_index++)
		{
			struct netgame_goal *goal = &global_goal[goal_index];

			if (goal_matches_player(player, player_index, goal_index) && count < maximum_count)
			{
				goal_indices[count] = goal_index;
				positions[count].x = goal->position.x;
				positions[count].y = goal->position.y;
				count++;
			}
		}
	}

	return count;
}

void game_engine_render_nav_points(
	short local_player_index)
{
	if (game_engine &&
		global_variant.universal_variant.goal_radar == _radar_nav_point &&
		local_player_index != NONE)
	{
		long player_index = local_player_get_player_index(local_player_index);

		if (player_index != NONE)
		{
			struct player_datum *player = player_get(player_index);
			long goal_index;

			if (player->unit_index != NONE)
			{
				real_point3d head_position;

				unit_get_head_position(player->unit_index, &head_position);
		/* port: every goal, one per player in the native builds' sessions */
		for (goal_index = 0; goal_index < (long)NUMBEROF(global_goal); goal_index++)
				{
					if (goal_matches_player(player, player_index, goal_index))
					{
						short render_type = hud_get_nav_point_render_type(
							local_player_index,
							&head_position,
							&global_goal[goal_index].position,
							NONE);

						custom_render_nav_point(
							local_player_index,
							&global_goal[goal_index].position,
							global_goal[goal_index].nav_index,
							render_type);
					}
				}
			}
		}
	}

	return;
}

boolean game_engine_hud_draw_messages(
	long player_index)
{
	boolean draw_messages = TRUE;

	if (game_engine && player_index!=NONE)
	{
		struct player_datum *player = player_get(player_index);

		if (player->local_player_index!=NONE &&
			game_engine_globals.hud_message_timers[player->local_player_index]>0.0f)
		{
			draw_messages = FALSE;
		}
	}

	return draw_messages;
}

boolean game_engine_force_autopickup(
	long unit_index,
	long weapon_index)
{
	boolean force_autopickup = FALSE;

	if (game_engine && global_variant.game_engine_index==game_engine_ctf && weapon_is_flag(weapon_index))
		force_autopickup = TRUE;

	return force_autopickup;
}

boolean game_engine_allow_pick_up(
	long unit_index,
	long weapon_index)
{
	boolean allow_pick_up = TRUE;

	if (game_engine && game_engine->allow_pick_up)
		allow_pick_up = game_engine->allow_pick_up(unit_index, weapon_index);

	return allow_pick_up;
}

boolean game_engine_picking_up(
	long unit_index,
	long weapon_index)
{
	boolean allow_pick_up = TRUE;

	if (game_engine)
	{
		struct weapon_datum *weapon = weapon_try_and_get(weapon_index);

		if (weapon && weapon_is_flag(weapon_index))
		{
			if (TEST_FLAG(weapon->weapon.flags, _weapon_runtime_game_engine_active_bit))
			{
				SET_FLAG(weapon->weapon.flags, _weapon_runtime_game_engine_active_bit, FALSE);

				if (game_engine->weapon_dropped)
					game_engine->weapon_dropped(weapon_index);
			}

			SET_FLAG(weapon->weapon.flags, _weapon_runtime_game_engine_active_bit, TRUE);

			if (game_engine->picking_up)
			{
				long player_index = player_index_from_unit_index(unit_index);

				allow_pick_up = game_engine->picking_up(
					weapon_index,
					player_index);
			}

			match_assert(
				"c:\\halo\\SOURCE\\game\\game_engine.c",
				0xF58,
				!allow_pick_up ||
					!TEST_FLAG(weapon->weapon.flags, _weapon_must_be_readied_bit) ||
					!unit_has_weapon_with_flag(unit_index, _weapon_must_be_readied_bit));
		}
	}

	return allow_pick_up;
}

boolean game_engine_test_flag(
	long flag)
{
	boolean result = FALSE;

	if (game_engine && game_engine->test_flag)
		result = game_engine->test_flag(flag);

	return result;
}

boolean game_engine_test_trait(
	long trait,
	long value)
{
	boolean result = FALSE;

	if (game_engine && game_engine->test_trait)
		result = game_engine->test_trait(trait, value);

	return result;
}

void game_engine_prespawn_player_update(
	long player_index)
{
	if (game_engine)
	{
		if (game_engine->prespawn_player_update)
			game_engine->prespawn_player_update(player_index);
		else
		{
			struct player_datum *player = player_get(player_index);
			/* port: 0 or 1 (another machine's player has no local player: -1) */
			player->team_index = PIN(player->local_player_index % 2, 0, 1);
		}
	}

	return;
}

static struct statistic_buffer game_engine_get_player_place(
	long player_index)
{
	struct statistic_buffer statistic_buffer[MULTIPLAYER_MAXIMUM_PLAYERS];
	long place = 0;

	populate_statistic_buffer(statistic_buffer, _postgame_statistic_ranking, FALSE);
	while (TRUE)
	{
		if (statistic_buffer[place].player_index == player_index)
			break;

		place++;
		match_assert(
			"c:\\halo\\SOURCE\\game\\game_engine.c",
			0x362,
			place<MULTIPLAYER_MAXIMUM_PLAYERS);
	}

	return statistic_buffer[place];
}

struct game_variant *build_game_variant_king(
	struct game_variant *variant)
{
	struct game_variant result = { 0 };

	SET_FLAG(result.universal_variant.flags, _game_variant_draw_object_in_motion_sensor_bit, TRUE);
	SET_FLAG(result.universal_variant.flags, _game_variant_allow_friendly_navpoints_bit, TRUE);
	SET_FLAG(result.universal_variant.flags, _game_variant_infinite_grenades_bit, FALSE);
	SET_FLAG(result.universal_variant.flags, _game_variant_no_shields_bit, FALSE);
	SET_FLAG(result.universal_variant.flags, _game_variant_always_invisible_bit, FALSE);
	SET_FLAG(result.universal_variant.flags, _game_variant_generic_starting_equipment_bit, FALSE);
	result.universal_variant.goal_radar = _radar_nav_point;
	result.universal_variant.respawn_time = 150;
	result.universal_variant.score_to_win = 2;
	result.flags = 1;
	result.universal_variant.suicide_penalty = 150;
	result.universal_variant.vehicle_set = _game_engine_vehicles_warthog;
	result.game_engine_index = game_engine_king;
	result.universal_variant.health = 1.0f;
	result.universal_variant.lives = 0;
	result.universal_variant.odd_man_out = FALSE;
	result.universal_variant.respawn_time_growth = 0;
	result.universal_variant.teams = FALSE;
	result.universal_variant.weapon_set = _game_engine_weapons_normal;
	result.game_engine_variant.king.moving_hill = FALSE;

	*variant = result;

	return variant;
}

struct game_variant *build_game_variant_crazy_king(
	struct game_variant *variant)
{
	struct game_variant result = { 0 };

	SET_FLAG(result.universal_variant.flags, _game_variant_draw_object_in_motion_sensor_bit, TRUE);
	SET_FLAG(result.universal_variant.flags, _game_variant_allow_friendly_navpoints_bit, TRUE);
	SET_FLAG(result.universal_variant.flags, _game_variant_infinite_grenades_bit, FALSE);
	SET_FLAG(result.universal_variant.flags, _game_variant_no_shields_bit, FALSE);
	SET_FLAG(result.universal_variant.flags, _game_variant_always_invisible_bit, FALSE);
	SET_FLAG(result.universal_variant.flags, _game_variant_generic_starting_equipment_bit, FALSE);
	result.universal_variant.goal_radar = _radar_nav_point;
	result.universal_variant.score_to_win = 2;
	result.game_engine_variant.king.moving_hill = TRUE;
	result.universal_variant.vehicle_set = _game_engine_vehicles_warthog;
	result.flags = 1;
	result.game_engine_index = game_engine_king;
	result.universal_variant.health = 1.0f;
	result.universal_variant.lives = 0;
	result.universal_variant.odd_man_out = FALSE;
	result.universal_variant.respawn_time = 0;
	result.universal_variant.respawn_time_growth = 0;
	result.universal_variant.suicide_penalty = 150;
	result.universal_variant.teams = FALSE;
	result.universal_variant.weapon_set = _game_engine_weapons_normal;

	*variant = result;

	return variant;
}

struct game_variant *build_game_variant_slayer(
	struct game_variant *variant)
{
	struct game_variant result = { 0 };

	result.game_engine_index = game_engine_slayer;
	SET_FLAG(result.universal_variant.flags, _game_variant_draw_object_in_motion_sensor_bit, TRUE);
	SET_FLAG(result.universal_variant.flags, _game_variant_allow_friendly_navpoints_bit, TRUE);
	SET_FLAG(result.universal_variant.flags, _game_variant_infinite_grenades_bit, FALSE);
	SET_FLAG(result.universal_variant.flags, _game_variant_no_shields_bit, FALSE);
	SET_FLAG(result.universal_variant.flags, _game_variant_always_invisible_bit, FALSE);
	SET_FLAG(result.universal_variant.flags, _game_variant_generic_starting_equipment_bit, FALSE);
	result.universal_variant.vehicle_set = _game_engine_vehicles_warthog;
	result.universal_variant.goal_radar = _radar_motion_tracker;
	result.universal_variant.health = 1.0f;
	result.universal_variant.lives = 0;
	result.universal_variant.odd_man_out = FALSE;
	result.universal_variant.respawn_time = 0;
	result.universal_variant.respawn_time_growth = 0;
	result.universal_variant.score_to_win = 15;
	result.universal_variant.suicide_penalty = 300;
	result.universal_variant.teams = FALSE;
	result.universal_variant.weapon_set = _game_engine_weapons_normal;
	result.game_engine_variant.slayer.no_death_bonus = FALSE;
	result.game_engine_variant.slayer.no_kill_penalty = FALSE;
	result.game_engine_variant.slayer.kill_in_order = FALSE;
	result.flags = 1;

	*variant = result;

	return variant;
}

struct game_variant *build_game_variant_team_slayer(
	struct game_variant *variant)
{
	struct game_variant result = { 0 };

	SET_FLAG(result.universal_variant.flags, _game_variant_draw_object_in_motion_sensor_bit, TRUE);
	SET_FLAG(result.universal_variant.flags, _game_variant_allow_friendly_navpoints_bit, TRUE);
	SET_FLAG(result.universal_variant.flags, _game_variant_infinite_grenades_bit, FALSE);
	SET_FLAG(result.universal_variant.flags, _game_variant_no_shields_bit, FALSE);
	SET_FLAG(result.universal_variant.flags, _game_variant_always_invisible_bit, FALSE);
	SET_FLAG(result.universal_variant.flags, _game_variant_generic_starting_equipment_bit, FALSE);
	result.universal_variant.respawn_time = 300;
	result.universal_variant.suicide_penalty = 300;
	result.game_engine_index = game_engine_slayer;
	result.universal_variant.teams = TRUE;
	result.universal_variant.vehicle_set = _game_engine_vehicles_warthog;
	result.flags = 1;
	result.universal_variant.goal_radar = _radar_motion_tracker;
	result.universal_variant.health = 1.0f;
	result.universal_variant.lives = 0;
	result.universal_variant.odd_man_out = FALSE;
	result.universal_variant.respawn_time_growth = 0;
	result.universal_variant.score_to_win = 50;
	result.universal_variant.weapon_set = _game_engine_weapons_normal;
	result.game_engine_variant.slayer.no_death_bonus = FALSE;
	result.game_engine_variant.slayer.no_kill_penalty = FALSE;
	result.game_engine_variant.slayer.kill_in_order = FALSE;

	*variant = result;

	return variant;
}

struct game_variant *build_game_variant_elimination(
	struct game_variant *variant)
{
	struct game_variant result = { 0 };

	SET_FLAG(result.universal_variant.flags, _game_variant_draw_object_in_motion_sensor_bit, TRUE);
	SET_FLAG(result.universal_variant.flags, _game_variant_allow_friendly_navpoints_bit, TRUE);
	SET_FLAG(result.universal_variant.flags, _game_variant_infinite_grenades_bit, FALSE);
	SET_FLAG(result.universal_variant.flags, _game_variant_no_shields_bit, FALSE);
	SET_FLAG(result.universal_variant.flags, _game_variant_always_invisible_bit, FALSE);
	SET_FLAG(result.universal_variant.flags, _game_variant_generic_starting_equipment_bit, FALSE);
	result.game_engine_index = game_engine_slayer;
	result.universal_variant.lives = 1;
	result.universal_variant.vehicle_set = _game_engine_vehicles_warthog;
	result.flags = 1;
	result.universal_variant.goal_radar = _radar_motion_tracker;
	result.universal_variant.health = 1.0f;
	result.universal_variant.odd_man_out = FALSE;
	result.universal_variant.respawn_time = 0;
	result.universal_variant.respawn_time_growth = 0;
	result.universal_variant.score_to_win = 25;
	result.universal_variant.suicide_penalty = 300;
	result.universal_variant.teams = FALSE;
	result.universal_variant.weapon_set = _game_engine_weapons_normal;
	result.game_engine_variant.slayer.no_death_bonus = FALSE;
	result.game_engine_variant.slayer.no_kill_penalty = FALSE;
	result.game_engine_variant.slayer.kill_in_order = FALSE;

	*variant = result;

	return variant;
}

struct game_variant *build_game_variant_phantoms(
	struct game_variant *variant)
{
	struct game_variant result = { 0 };

	SET_FLAG(result.universal_variant.flags, _game_variant_draw_object_in_motion_sensor_bit, FALSE);
	SET_FLAG(result.universal_variant.flags, _game_variant_allow_friendly_navpoints_bit, TRUE);
	SET_FLAG(result.universal_variant.flags, _game_variant_infinite_grenades_bit, FALSE);
	SET_FLAG(result.universal_variant.flags, _game_variant_no_shields_bit, FALSE);
	SET_FLAG(result.universal_variant.flags, _game_variant_always_invisible_bit, TRUE);
	SET_FLAG(result.universal_variant.flags, _game_variant_generic_starting_equipment_bit, FALSE);
	result.game_engine_index = game_engine_slayer;
	result.universal_variant.respawn_time = 150;
	result.universal_variant.goal_radar = _radar_nav_point;
	result.universal_variant.vehicle_set = _game_engine_vehicles_warthog;
	result.universal_variant.suicide_penalty = 150;
	result.game_engine_variant.slayer.no_death_bonus = TRUE;
	result.game_engine_variant.slayer.no_kill_penalty = TRUE;
	result.game_engine_variant.slayer.kill_in_order = TRUE;
	result.flags = 1;
	result.universal_variant.health = 1.0f;
	result.universal_variant.lives = 0;
	result.universal_variant.odd_man_out = FALSE;
	result.universal_variant.respawn_time_growth = 0;
	result.universal_variant.score_to_win = 10;
	result.universal_variant.teams = FALSE;
	result.universal_variant.weapon_set = _game_engine_weapons_normal;

	*variant = result;

	return variant;
}

struct game_variant *build_game_variant_endurance(
	struct game_variant *variant)
{
	struct game_variant result = { 0 };

	SET_FLAG(result.universal_variant.flags, _game_variant_draw_object_in_motion_sensor_bit, TRUE);
	SET_FLAG(result.universal_variant.flags, _game_variant_allow_friendly_navpoints_bit, TRUE);
	SET_FLAG(result.universal_variant.flags, _game_variant_infinite_grenades_bit, FALSE);
	SET_FLAG(result.universal_variant.flags, _game_variant_no_shields_bit, FALSE);
	SET_FLAG(result.universal_variant.flags, _game_variant_always_invisible_bit, FALSE);
	SET_FLAG(result.universal_variant.flags, _game_variant_generic_starting_equipment_bit, FALSE);
	result.game_engine_index = game_engine_slayer;
	result.universal_variant.odd_man_out = TRUE;
	result.universal_variant.respawn_time_growth = 300;
	result.universal_variant.vehicle_set = _game_engine_vehicles_warthog;
	result.flags = 1;
	result.universal_variant.suicide_penalty = 300;
	result.universal_variant.goal_radar = _radar_motion_tracker;
	result.universal_variant.health = 1.0f;
	result.universal_variant.lives = 5;
	result.universal_variant.respawn_time = 0;
	result.universal_variant.score_to_win = 10;
	result.universal_variant.teams = FALSE;
	result.universal_variant.weapon_set = _game_engine_weapons_normal;
	result.game_engine_variant.slayer.no_death_bonus = FALSE;
	result.game_engine_variant.slayer.no_kill_penalty = FALSE;
	result.game_engine_variant.slayer.kill_in_order = FALSE;

	*variant = result;

	return variant;
}

struct game_variant *build_game_variant_rockets(
	struct game_variant *variant)
{
	struct game_variant result = { 0 };

	SET_FLAG(result.universal_variant.flags, _game_variant_draw_object_in_motion_sensor_bit, FALSE);
	SET_FLAG(result.universal_variant.flags, _game_variant_allow_friendly_navpoints_bit, TRUE);
	SET_FLAG(result.universal_variant.flags, _game_variant_infinite_grenades_bit, FALSE);
	SET_FLAG(result.universal_variant.flags, _game_variant_no_shields_bit, FALSE);
	SET_FLAG(result.universal_variant.flags, _game_variant_always_invisible_bit, FALSE);
	SET_FLAG(result.universal_variant.flags, _game_variant_generic_starting_equipment_bit, TRUE);
	result.game_engine_index = game_engine_slayer;
	result.universal_variant.goal_radar = _radar_nav_point;
	result.universal_variant.vehicle_set = _game_engine_vehicles_warthog;
	result.flags = 1;
	result.universal_variant.health = 1.0f;
	result.universal_variant.lives = 0;
	result.universal_variant.odd_man_out = FALSE;
	result.universal_variant.respawn_time = 0;
	result.universal_variant.respawn_time_growth = 0;
	result.universal_variant.score_to_win = 25;
	result.universal_variant.suicide_penalty = 300;
	result.universal_variant.teams = FALSE;
	result.universal_variant.weapon_set = _game_engine_weapons_rocket_launchers;
	result.game_engine_variant.slayer.no_death_bonus = FALSE;
	result.game_engine_variant.slayer.no_kill_penalty = FALSE;
	result.game_engine_variant.slayer.kill_in_order = FALSE;

	*variant = result;

	return variant;
}

struct game_variant *build_game_variant_snipers(
	struct game_variant *variant)
{
	struct game_variant result = { 0 };

	SET_FLAG(result.universal_variant.flags, _game_variant_draw_object_in_motion_sensor_bit, FALSE);
	SET_FLAG(result.universal_variant.flags, _game_variant_allow_friendly_navpoints_bit, TRUE);
	SET_FLAG(result.universal_variant.flags, _game_variant_infinite_grenades_bit, FALSE);
	SET_FLAG(result.universal_variant.flags, _game_variant_no_shields_bit, FALSE);
	SET_FLAG(result.universal_variant.flags, _game_variant_always_invisible_bit, FALSE);
	SET_FLAG(result.universal_variant.flags, _game_variant_generic_starting_equipment_bit, TRUE);
	result.game_engine_index = game_engine_slayer;
	result.universal_variant.goal_radar = _radar_nav_point;
	result.universal_variant.vehicle_set = _game_engine_vehicles_warthog;
	result.flags = 1;
	result.universal_variant.health = 1.0f;
	result.universal_variant.lives = 0;
	result.universal_variant.odd_man_out = FALSE;
	result.universal_variant.respawn_time = 0;
	result.universal_variant.respawn_time_growth = 150;
	result.universal_variant.score_to_win = 15;
	result.universal_variant.suicide_penalty = 300;
	result.universal_variant.teams = FALSE;
	result.universal_variant.weapon_set = _game_engine_weapons_sniping;
	result.game_engine_variant.slayer.no_death_bonus = FALSE;
	result.game_engine_variant.slayer.no_kill_penalty = FALSE;
	result.game_engine_variant.slayer.kill_in_order = FALSE;

	*variant = result;

	return variant;
}

struct game_variant *build_game_variant_oddball(
	struct game_variant *variant)
{
	struct game_variant result = { 0 };

	SET_FLAG(result.universal_variant.flags, _game_variant_draw_object_in_motion_sensor_bit, TRUE);
	SET_FLAG(result.universal_variant.flags, _game_variant_allow_friendly_navpoints_bit, TRUE);
	SET_FLAG(result.universal_variant.flags, _game_variant_infinite_grenades_bit, FALSE);
	SET_FLAG(result.universal_variant.flags, _game_variant_no_shields_bit, FALSE);
	SET_FLAG(result.universal_variant.flags, _game_variant_always_invisible_bit, FALSE);
	SET_FLAG(result.universal_variant.flags, _game_variant_generic_starting_equipment_bit, FALSE);
	result.universal_variant.respawn_time = 150;
	result.universal_variant.goal_radar = _radar_nav_point;
	result.universal_variant.suicide_penalty = 150;
	result.universal_variant.vehicle_set = _game_engine_vehicles_none;
	result.game_engine_variant.oddball.ball_spawn_count = 1;
	result.game_engine_variant.oddball.ball_spawn_delay = TRUE;
	result.flags = 1;
	result.game_engine_index = game_engine_oddball;
	result.universal_variant.health = 1.0f;
	result.universal_variant.lives = 0;
	result.universal_variant.odd_man_out = FALSE;
	result.universal_variant.respawn_time_growth = 0;
	result.universal_variant.score_to_win = 2;
	result.universal_variant.teams = FALSE;
	result.universal_variant.weapon_set = _game_engine_weapons_normal;
	result.game_engine_variant.oddball.random_start = FALSE;
	result.game_engine_variant.oddball.oddball_ball_type = 0;
	result.game_engine_variant.oddball.trait_with_ball = 0;
	result.game_engine_variant.oddball.trait_without_ball = 0;
	result.game_engine_variant.oddball.speed_with_ball = 0;

	*variant = result;

	return variant;
}

struct game_variant *build_game_variant_team_oddball(
	struct game_variant *variant)
{
	/* port: cleared, as every other builder's is: only some of its fields
	are set below, and the rest were the stack's */
	struct game_variant result = { 0 };

	SET_FLAG(result.universal_variant.flags, _game_variant_draw_object_in_motion_sensor_bit, TRUE);
	SET_FLAG(result.universal_variant.flags, _game_variant_allow_friendly_navpoints_bit, TRUE);
	SET_FLAG(result.universal_variant.flags, _game_variant_infinite_grenades_bit, FALSE);
	SET_FLAG(result.universal_variant.flags, _game_variant_no_shields_bit, FALSE);
	SET_FLAG(result.universal_variant.flags, _game_variant_always_invisible_bit, FALSE);
	SET_FLAG(result.universal_variant.flags, _game_variant_generic_starting_equipment_bit, TRUE);
	result.universal_variant.goal_radar = _radar_nav_point;
	result.universal_variant.lives = 0;
	result.universal_variant.teams = TRUE;
	result.universal_variant.odd_man_out = FALSE;
	result.universal_variant.vehicle_set = _game_engine_vehicles_none;
	result.universal_variant.respawn_time_growth = 0;
	result.game_engine_variant.oddball.ball_spawn_count = 1;
	result.universal_variant.weapon_set = _game_engine_weapons_normal;
	result.flags = 1;
	result.game_engine_variant.oddball.ball_spawn_delay = FALSE;
	result.game_engine_variant.oddball.random_start = FALSE;
	result.game_engine_variant.oddball.oddball_ball_type = 0;
	result.game_engine_variant.oddball.trait_with_ball = 0;
	result.game_engine_variant.oddball.trait_without_ball = 0;
	result.game_engine_variant.oddball.speed_with_ball = 0;
	result.game_engine_index = game_engine_oddball;
	result.universal_variant.health = 1.0f;
	result.universal_variant.respawn_time = 300;
	result.universal_variant.score_to_win = 2;
	result.universal_variant.suicide_penalty = 150;

	*variant = result;

	return variant;
}

struct game_variant *build_game_variant_reverse_tag(
	struct game_variant *variant)
{
	struct game_variant result = { 0 };

	SET_FLAG(result.universal_variant.flags, _game_variant_draw_object_in_motion_sensor_bit, FALSE);
	SET_FLAG(result.universal_variant.flags, _game_variant_allow_friendly_navpoints_bit, TRUE);
	SET_FLAG(result.universal_variant.flags, _game_variant_infinite_grenades_bit, FALSE);
	SET_FLAG(result.universal_variant.flags, _game_variant_no_shields_bit, FALSE);
	SET_FLAG(result.universal_variant.flags, _game_variant_always_invisible_bit, FALSE);
	SET_FLAG(result.universal_variant.flags, _game_variant_generic_starting_equipment_bit, FALSE);
	result.universal_variant.respawn_time = 150;
	result.universal_variant.goal_radar = _radar_nav_point;
	result.universal_variant.suicide_penalty = 150;
	result.universal_variant.vehicle_set = _game_engine_vehicles_none;
	result.game_engine_variant.oddball.ball_spawn_count = 1;
	result.game_engine_variant.oddball.ball_spawn_delay = TRUE;
	result.game_engine_variant.oddball.oddball_ball_type = 1;
	result.flags = 1;
	result.game_engine_index = game_engine_oddball;
	result.universal_variant.health = 1.0f;
	result.universal_variant.lives = 0;
	result.universal_variant.odd_man_out = FALSE;
	result.universal_variant.respawn_time_growth = 0;
	result.universal_variant.score_to_win = 2;
	result.universal_variant.teams = FALSE;
	result.universal_variant.weapon_set = _game_engine_weapons_normal;
	result.game_engine_variant.oddball.random_start = FALSE;
	result.game_engine_variant.oddball.trait_with_ball = 0;
	result.game_engine_variant.oddball.trait_without_ball = 0;
	result.game_engine_variant.oddball.speed_with_ball = 0;

	*variant = result;

	return variant;
}

struct game_variant *build_game_variant_accumulation(
	struct game_variant *variant)
{
	struct game_variant result = { 0 };

	SET_FLAG(result.universal_variant.flags, _game_variant_draw_object_in_motion_sensor_bit, FALSE);
	SET_FLAG(result.universal_variant.flags, _game_variant_allow_friendly_navpoints_bit, TRUE);
	SET_FLAG(result.universal_variant.flags, _game_variant_infinite_grenades_bit, FALSE);
	SET_FLAG(result.universal_variant.flags, _game_variant_no_shields_bit, FALSE);
	SET_FLAG(result.universal_variant.flags, _game_variant_always_invisible_bit, FALSE);
	SET_FLAG(result.universal_variant.flags, _game_variant_generic_starting_equipment_bit, FALSE);
	result.universal_variant.respawn_time = 150;
	result.universal_variant.suicide_penalty = 150;
	result.universal_variant.vehicle_set = _game_engine_vehicles_none;
	result.game_engine_variant.oddball.oddball_ball_type = 1;
	result.flags = 1;
	result.game_engine_index = game_engine_oddball;
	result.universal_variant.goal_radar = _radar_none;
	result.universal_variant.health = 1.0f;
	result.universal_variant.lives = 0;
	result.universal_variant.odd_man_out = FALSE;
	result.universal_variant.respawn_time_growth = 0;
	result.universal_variant.score_to_win = 5;
	result.universal_variant.teams = FALSE;
	result.universal_variant.weapon_set = _game_engine_weapons_normal;
	result.game_engine_variant.oddball.ball_spawn_count = 16;
	result.game_engine_variant.oddball.ball_spawn_delay = FALSE;
	result.game_engine_variant.oddball.random_start = FALSE;
	result.game_engine_variant.oddball.trait_with_ball = 0;
	result.game_engine_variant.oddball.trait_without_ball = 0;

	*variant = result;

	return variant;
}

struct game_variant *build_game_variant_juggernaut(
	struct game_variant *variant)
{
	struct game_variant result = { 0 };

	SET_FLAG(result.universal_variant.flags, _game_variant_draw_object_in_motion_sensor_bit, TRUE);
	SET_FLAG(result.universal_variant.flags, _game_variant_allow_friendly_navpoints_bit, TRUE);
	SET_FLAG(result.universal_variant.flags, _game_variant_infinite_grenades_bit, FALSE);
	SET_FLAG(result.universal_variant.flags, _game_variant_no_shields_bit, FALSE);
	SET_FLAG(result.universal_variant.flags, _game_variant_always_invisible_bit, FALSE);
	SET_FLAG(result.universal_variant.flags, _game_variant_generic_starting_equipment_bit, FALSE);
	result.universal_variant.respawn_time = 150;
	result.universal_variant.suicide_penalty = 150;
	result.universal_variant.goal_radar = _radar_nav_point;
	result.universal_variant.vehicle_set = _game_engine_vehicles_warthog;
	result.game_engine_variant.oddball.ball_spawn_count = 1;
	result.game_engine_variant.oddball.trait_with_ball = 2;
	result.flags = 1;
	result.game_engine_variant.oddball.oddball_ball_type = 2;
	result.game_engine_index = game_engine_oddball;
	result.universal_variant.health = 1.0f;
	result.universal_variant.lives = 0;
	result.universal_variant.odd_man_out = FALSE;
	result.universal_variant.respawn_time_growth = 0;
	result.universal_variant.score_to_win = 10;
	result.universal_variant.teams = FALSE;
	result.universal_variant.weapon_set = _game_engine_weapons_normal;
	result.game_engine_variant.oddball.ball_spawn_delay = FALSE;
	result.game_engine_variant.oddball.random_start = FALSE;
	result.game_engine_variant.oddball.trait_without_ball = 0;

	*variant = result;

	return variant;
}

struct game_variant *build_game_variant_stalker(
	struct game_variant *variant)
{
	struct game_variant result = { 0 };

	SET_FLAG(result.universal_variant.flags, _game_variant_draw_object_in_motion_sensor_bit, FALSE);
	SET_FLAG(result.universal_variant.flags, _game_variant_allow_friendly_navpoints_bit, TRUE);
	SET_FLAG(result.universal_variant.flags, _game_variant_infinite_grenades_bit, FALSE);
	SET_FLAG(result.universal_variant.flags, _game_variant_no_shields_bit, FALSE);
	SET_FLAG(result.universal_variant.flags, _game_variant_always_invisible_bit, FALSE);
	SET_FLAG(result.universal_variant.flags, _game_variant_generic_starting_equipment_bit, FALSE);
	result.universal_variant.respawn_time = 150;
	result.universal_variant.suicide_penalty = 150;
	result.game_engine_variant.oddball.ball_spawn_count = 1;
	result.game_engine_index = game_engine_oddball;
	result.game_engine_variant.oddball.trait_with_ball = 1;
	result.game_engine_variant.oddball.trait_without_ball = 3;
	result.flags = 1;
	result.universal_variant.vehicle_set = _game_engine_vehicles_warthog;
	result.game_engine_variant.oddball.oddball_ball_type = 2;
	result.game_engine_variant.oddball.speed_with_ball = 2;
	result.universal_variant.goal_radar = _radar_motion_tracker;
	result.universal_variant.health = 1.0f;
	result.universal_variant.lives = 0;
	result.universal_variant.odd_man_out = FALSE;
	result.universal_variant.respawn_time_growth = 0;
	result.universal_variant.score_to_win = 10;
	result.universal_variant.teams = FALSE;
	result.universal_variant.weapon_set = _game_engine_weapons_normal;
	result.game_engine_variant.oddball.ball_spawn_delay = FALSE;
	result.game_engine_variant.oddball.random_start = FALSE;

	*variant = result;

	return variant;
}

struct game_variant *build_game_variant_king_pro(
	struct game_variant *variant)
{
	struct game_variant result = { 0 };

	SET_FLAG(result.universal_variant.flags, _game_variant_draw_object_in_motion_sensor_bit, TRUE);
	SET_FLAG(result.universal_variant.flags, _game_variant_allow_friendly_navpoints_bit, TRUE);
	SET_FLAG(result.universal_variant.flags, _game_variant_infinite_grenades_bit, FALSE);
	SET_FLAG(result.universal_variant.flags, _game_variant_no_shields_bit, FALSE);
	SET_FLAG(result.universal_variant.flags, _game_variant_always_invisible_bit, FALSE);
	SET_FLAG(result.universal_variant.flags, _game_variant_generic_starting_equipment_bit, TRUE);
	result.universal_variant.goal_radar = _radar_nav_point;
	result.universal_variant.score_to_win = 2;
	result.flags = 1;
	result.universal_variant.vehicle_set = _game_engine_vehicles_warthog;
	result.game_engine_index = game_engine_king;
	result.universal_variant.health = 1.0f;
	result.universal_variant.lives = 0;
	result.universal_variant.odd_man_out = FALSE;
	result.universal_variant.respawn_time = 300;
	result.universal_variant.respawn_time_growth = 0;
	result.universal_variant.suicide_penalty = 450;
	result.universal_variant.teams = FALSE;
	result.universal_variant.weapon_set = _game_engine_weapons_normal;
	result.game_engine_variant.king.moving_hill = FALSE;

	*variant = result;

	return variant;
}

struct game_variant *build_game_variant_team_king(
	struct game_variant *variant)
{
	struct game_variant result = { 0 };

	SET_FLAG(result.universal_variant.flags, _game_variant_draw_object_in_motion_sensor_bit, TRUE);
	SET_FLAG(result.universal_variant.flags, _game_variant_allow_friendly_navpoints_bit, TRUE);
	SET_FLAG(result.universal_variant.flags, _game_variant_infinite_grenades_bit, FALSE);
	SET_FLAG(result.universal_variant.flags, _game_variant_no_shields_bit, FALSE);
	SET_FLAG(result.universal_variant.flags, _game_variant_always_invisible_bit, FALSE);
	SET_FLAG(result.universal_variant.flags, _game_variant_generic_starting_equipment_bit, FALSE);
	result.universal_variant.score_to_win = 2;
	result.universal_variant.goal_radar = _radar_nav_point;
	result.universal_variant.vehicle_set = _game_engine_vehicles_warthog;
	result.universal_variant.teams = TRUE;
	result.game_engine_variant.king.moving_hill = TRUE;
	result.flags = 1;
	result.game_engine_index = game_engine_king;
	result.universal_variant.health = 1.0f;
	result.universal_variant.lives = 0;
	result.universal_variant.odd_man_out = FALSE;
	result.universal_variant.respawn_time = 300;
	result.universal_variant.respawn_time_growth = 0;
	result.universal_variant.suicide_penalty = 150;
	result.universal_variant.weapon_set = _game_engine_weapons_normal;

	*variant = result;

	return variant;
}

struct game_variant *build_game_variant_slayer_pro(
	struct game_variant *variant)
{
	struct game_variant result = { 0 };

	result.game_engine_index = game_engine_slayer;
	SET_FLAG(result.universal_variant.flags, _game_variant_draw_object_in_motion_sensor_bit, TRUE);
	SET_FLAG(result.universal_variant.flags, _game_variant_allow_friendly_navpoints_bit, TRUE);
	SET_FLAG(result.universal_variant.flags, _game_variant_infinite_grenades_bit, FALSE);
	SET_FLAG(result.universal_variant.flags, _game_variant_no_shields_bit, FALSE);
	SET_FLAG(result.universal_variant.flags, _game_variant_always_invisible_bit, FALSE);
	SET_FLAG(result.universal_variant.flags, _game_variant_generic_starting_equipment_bit, TRUE);
	result.universal_variant.vehicle_set = _game_engine_vehicles_warthog;
	result.game_engine_variant.slayer.no_death_bonus = TRUE;
	result.game_engine_variant.slayer.no_kill_penalty = TRUE;
	result.flags = 1;
	result.universal_variant.goal_radar = _radar_motion_tracker;
	result.universal_variant.health = 1.0f;
	result.universal_variant.lives = 0;
	result.universal_variant.odd_man_out = FALSE;
	result.universal_variant.respawn_time = 0;
	result.universal_variant.respawn_time_growth = 0;
	result.universal_variant.score_to_win = 25;
	result.universal_variant.suicide_penalty = 450;
	result.universal_variant.teams = FALSE;
	result.universal_variant.weapon_set = _game_engine_weapons_normal;
	result.game_engine_variant.slayer.kill_in_order = FALSE;

	*variant = result;

	return variant;
}

struct game_variant *build_game_variant_ctf(
	struct game_variant *variant)
{
	struct game_variant result = { 0 };

	result.game_engine_index = game_engine_ctf;
	SET_FLAG(result.universal_variant.flags, _game_variant_draw_object_in_motion_sensor_bit, TRUE);
	SET_FLAG(result.universal_variant.flags, _game_variant_allow_friendly_navpoints_bit, TRUE);
	SET_FLAG(result.universal_variant.flags, _game_variant_infinite_grenades_bit, FALSE);
	SET_FLAG(result.universal_variant.flags, _game_variant_no_shields_bit, FALSE);
	SET_FLAG(result.universal_variant.flags, _game_variant_always_invisible_bit, FALSE);
	SET_FLAG(result.universal_variant.flags, _game_variant_generic_starting_equipment_bit, FALSE);
	result.universal_variant.goal_radar = _radar_nav_point;
	result.universal_variant.teams = TRUE;
	result.flags = 1;
	result.universal_variant.health = 1.0f;
	result.universal_variant.lives = 0;
	result.universal_variant.odd_man_out = FALSE;
	result.universal_variant.respawn_time = 300;
	result.universal_variant.respawn_time_growth = 0;
	result.universal_variant.score_to_win = 3;
	result.universal_variant.suicide_penalty = 150;
	result.universal_variant.weapon_set = _game_engine_weapons_normal;
	result.universal_variant.vehicle_set = _game_engine_vehicles_warthog;
	result.game_engine_variant.ctf.assault = FALSE;
	result.game_engine_variant.ctf.flag_at_home_to_score = FALSE;
	result.game_engine_variant.ctf.flag_must_reset = FALSE;
	result.game_engine_variant.ctf.reset_on_capture = FALSE;
	result.game_engine_variant.ctf.single_flag_time = 0;

	*variant = result;

	return variant;
}

struct game_variant *build_game_variant_ctf_pro(
	struct game_variant *variant)
{
	struct game_variant result = { 0 };

	result.game_engine_index = game_engine_ctf;
	SET_FLAG(result.universal_variant.flags, _game_variant_draw_object_in_motion_sensor_bit, TRUE);
	SET_FLAG(result.universal_variant.flags, _game_variant_allow_friendly_navpoints_bit, TRUE);
	SET_FLAG(result.universal_variant.flags, _game_variant_infinite_grenades_bit, FALSE);
	SET_FLAG(result.universal_variant.flags, _game_variant_no_shields_bit, FALSE);
	SET_FLAG(result.universal_variant.flags, _game_variant_always_invisible_bit, FALSE);
	SET_FLAG(result.universal_variant.flags, _game_variant_generic_starting_equipment_bit, TRUE);
	result.universal_variant.goal_radar = _radar_nav_point;
	result.universal_variant.teams = TRUE;
	result.game_engine_variant.ctf.flag_at_home_to_score = TRUE;
	result.flags = 1;
	result.universal_variant.health = 1.0f;
	result.universal_variant.lives = 0;
	result.universal_variant.odd_man_out = FALSE;
	result.universal_variant.respawn_time = 300;
	result.universal_variant.respawn_time_growth = 0;
	result.universal_variant.score_to_win = 3;
	result.universal_variant.suicide_penalty = 450;
	result.universal_variant.weapon_set = _game_engine_weapons_normal;
	result.universal_variant.vehicle_set = _game_engine_vehicles_warthog;
	result.game_engine_variant.ctf.assault = FALSE;
	result.game_engine_variant.ctf.flag_must_reset = FALSE;
	result.game_engine_variant.ctf.reset_on_capture = FALSE;
	result.game_engine_variant.ctf.single_flag_time = 0;

	*variant = result;

	return variant;
}

struct game_variant *build_game_variant_invasion(
	struct game_variant *variant)
{
	struct game_variant result = { 0 };

	result.game_engine_index = game_engine_ctf;
	SET_FLAG(result.universal_variant.flags, _game_variant_draw_object_in_motion_sensor_bit, TRUE);
	SET_FLAG(result.universal_variant.flags, _game_variant_allow_friendly_navpoints_bit, TRUE);
	SET_FLAG(result.universal_variant.flags, _game_variant_infinite_grenades_bit, FALSE);
	SET_FLAG(result.universal_variant.flags, _game_variant_no_shields_bit, FALSE);
	SET_FLAG(result.universal_variant.flags, _game_variant_always_invisible_bit, FALSE);
	SET_FLAG(result.universal_variant.flags, _game_variant_generic_starting_equipment_bit, FALSE);
	result.universal_variant.goal_radar = _radar_nav_point;
	result.universal_variant.teams = TRUE;
	result.game_engine_variant.ctf.assault = TRUE;
	result.flags = 1;
	result.universal_variant.health = 1.0f;
	result.universal_variant.lives = 5;
	result.universal_variant.odd_man_out = FALSE;
	result.universal_variant.respawn_time = 0;
	result.universal_variant.respawn_time_growth = 0;
	result.universal_variant.score_to_win = 3;
	result.universal_variant.suicide_penalty = 150;
	result.universal_variant.weapon_set = _game_engine_weapons_normal;
	result.universal_variant.vehicle_set = _game_engine_vehicles_warthog;
	result.game_engine_variant.ctf.flag_at_home_to_score = FALSE;
	result.game_engine_variant.ctf.flag_must_reset = FALSE;
	result.game_engine_variant.ctf.reset_on_capture = FALSE;
	result.game_engine_variant.ctf.single_flag_time = 0;

	*variant = result;

	return variant;
}

struct game_variant *build_game_variant_iron_ctf(
	struct game_variant *variant)
{
	struct game_variant result = { 0 };

	result.game_engine_index = game_engine_ctf;
	SET_FLAG(result.universal_variant.flags, _game_variant_draw_object_in_motion_sensor_bit, TRUE);
	SET_FLAG(result.universal_variant.flags, _game_variant_allow_friendly_navpoints_bit, TRUE);
	SET_FLAG(result.universal_variant.flags, _game_variant_infinite_grenades_bit, FALSE);
	SET_FLAG(result.universal_variant.flags, _game_variant_no_shields_bit, FALSE);
	SET_FLAG(result.universal_variant.flags, _game_variant_always_invisible_bit, FALSE);
	SET_FLAG(result.universal_variant.flags, _game_variant_generic_starting_equipment_bit, FALSE);
	result.universal_variant.goal_radar = _radar_nav_point;
	result.universal_variant.teams = TRUE;
	result.game_engine_variant.ctf.flag_must_reset = TRUE;
	result.flags = 1;
	result.universal_variant.health = 2.0f;
	result.universal_variant.lives = 0;
	result.universal_variant.odd_man_out = FALSE;
	result.universal_variant.respawn_time = 450;
	result.universal_variant.respawn_time_growth = 0;
	result.universal_variant.score_to_win = 3;
	result.universal_variant.suicide_penalty = 150;
	result.universal_variant.weapon_set = _game_engine_weapons_normal;
	result.universal_variant.vehicle_set = _game_engine_vehicles_tank;
	result.game_engine_variant.ctf.assault = FALSE;
	result.game_engine_variant.ctf.flag_at_home_to_score = FALSE;
	result.game_engine_variant.ctf.reset_on_capture = FALSE;
	result.game_engine_variant.ctf.single_flag_time = 0;

	*variant = result;

	return variant;
}

struct game_variant *build_game_variant_race(
	struct game_variant *variant)
{
	struct game_variant result = { 0 };

	SET_FLAG(result.universal_variant.flags, _game_variant_draw_object_in_motion_sensor_bit, TRUE);
	SET_FLAG(result.universal_variant.flags, _game_variant_allow_friendly_navpoints_bit, TRUE);
	SET_FLAG(result.universal_variant.flags, _game_variant_infinite_grenades_bit, FALSE);
	SET_FLAG(result.universal_variant.flags, _game_variant_no_shields_bit, FALSE);
	SET_FLAG(result.universal_variant.flags, _game_variant_always_invisible_bit, FALSE);
	SET_FLAG(result.universal_variant.flags, _game_variant_generic_starting_equipment_bit, FALSE);
	result.universal_variant.goal_radar = _radar_nav_point;
	result.flags = 1;
	result.game_engine_index = game_engine_race;
	result.universal_variant.health = 1.0f;
	result.universal_variant.lives = 0;
	result.universal_variant.odd_man_out = FALSE;
	result.universal_variant.respawn_time = 0;
	result.universal_variant.respawn_time_growth = 0;
	result.universal_variant.score_to_win = 3;
	result.universal_variant.suicide_penalty = 300;
	result.universal_variant.teams = FALSE;
	result.universal_variant.weapon_set = _game_engine_weapons_normal;
	result.universal_variant.vehicle_set = _game_engine_vehicles_warthog;
	result.game_engine_variant.race.race_type = 0;
	result.game_engine_variant.race.team_scoring = 0;

	*variant = result;

	return variant;
}

struct game_variant *build_game_variant_rally(
	struct game_variant *variant)
{
	struct game_variant result = { 0 };

	SET_FLAG(result.universal_variant.flags, _game_variant_draw_object_in_motion_sensor_bit, TRUE);
	SET_FLAG(result.universal_variant.flags, _game_variant_allow_friendly_navpoints_bit, TRUE);
	SET_FLAG(result.universal_variant.flags, _game_variant_infinite_grenades_bit, FALSE);
	SET_FLAG(result.universal_variant.flags, _game_variant_no_shields_bit, FALSE);
	SET_FLAG(result.universal_variant.flags, _game_variant_always_invisible_bit, FALSE);
	SET_FLAG(result.universal_variant.flags, _game_variant_generic_starting_equipment_bit, FALSE);
	result.universal_variant.goal_radar = _radar_nav_point;
	result.universal_variant.vehicle_set = _game_engine_vehicles_warthog;
	result.game_engine_variant.race.race_type = 2;
	result.flags = 1;
	result.game_engine_index = game_engine_race;
	result.universal_variant.health = 1.0f;
	result.universal_variant.lives = 0;
	result.universal_variant.odd_man_out = FALSE;
	result.universal_variant.respawn_time = 0;
	result.universal_variant.respawn_time_growth = 0;
	result.universal_variant.score_to_win = 15;
	result.universal_variant.suicide_penalty = 300;
	result.universal_variant.teams = FALSE;
	result.universal_variant.weapon_set = _game_engine_weapons_normal;
	result.game_engine_variant.race.team_scoring = 0;

	*variant = result;

	return variant;
}

struct game_variant *build_game_variant_team_race(
	struct game_variant *variant)
{
	struct game_variant result = { 0 };

	SET_FLAG(result.universal_variant.flags, _game_variant_draw_object_in_motion_sensor_bit, TRUE);
	SET_FLAG(result.universal_variant.flags, _game_variant_allow_friendly_navpoints_bit, TRUE);
	SET_FLAG(result.universal_variant.flags, _game_variant_infinite_grenades_bit, FALSE);
	SET_FLAG(result.universal_variant.flags, _game_variant_no_shields_bit, FALSE);
	SET_FLAG(result.universal_variant.flags, _game_variant_always_invisible_bit, FALSE);
	SET_FLAG(result.universal_variant.flags, _game_variant_generic_starting_equipment_bit, FALSE);
	result.universal_variant.goal_radar = _radar_nav_point;
	result.universal_variant.teams = TRUE;
	result.flags = 1;
	result.game_engine_index = game_engine_race;
	result.universal_variant.health = 1.0f;
	result.universal_variant.lives = 0;
	result.universal_variant.odd_man_out = FALSE;
	result.universal_variant.respawn_time = 0;
	result.universal_variant.respawn_time_growth = 0;
	result.universal_variant.score_to_win = 3;
	result.universal_variant.suicide_penalty = 300;
	result.universal_variant.weapon_set = _game_engine_weapons_normal;
	result.universal_variant.vehicle_set = _game_engine_vehicles_warthog;
	result.game_engine_variant.race.race_type = 0;
	result.game_engine_variant.race.team_scoring = 0;

	*variant = result;

	return variant;
}

struct game_variant *build_game_variant_team_rally(
	struct game_variant *variant)
{
	struct game_variant result = { 0 };

	result.game_engine_index = game_engine_race;
	SET_FLAG(result.universal_variant.flags, _game_variant_draw_object_in_motion_sensor_bit, TRUE);
	SET_FLAG(result.universal_variant.flags, _game_variant_allow_friendly_navpoints_bit, TRUE);
	SET_FLAG(result.universal_variant.flags, _game_variant_infinite_grenades_bit, FALSE);
	SET_FLAG(result.universal_variant.flags, _game_variant_no_shields_bit, FALSE);
	SET_FLAG(result.universal_variant.flags, _game_variant_always_invisible_bit, FALSE);
	SET_FLAG(result.universal_variant.flags, _game_variant_generic_starting_equipment_bit, FALSE);
	result.universal_variant.goal_radar = _radar_nav_point;
	result.universal_variant.score_to_win = 5;
	result.universal_variant.teams = TRUE;
	result.universal_variant.vehicle_set = _game_engine_vehicles_warthog;
	result.flags = 1;
	result.game_engine_variant.race.race_type = 2;
	result.universal_variant.health = 1.0f;
	result.universal_variant.lives = 0;
	result.universal_variant.odd_man_out = FALSE;
	result.universal_variant.respawn_time = 0;
	result.universal_variant.respawn_time_growth = 0;
	result.universal_variant.suicide_penalty = 300;
	result.universal_variant.weapon_set = _game_engine_weapons_normal;
	result.game_engine_variant.race.team_scoring = 0;

	*variant = result;

	return variant;
}

long postgame_statistic_get_rating(
	long player_index,
	enum postgame_statistic statistic,
	boolean inverse)
{
	struct statistic_buffer statistic_buffer[MULTIPLAYER_MAXIMUM_PLAYERS];
	long player_count = 0;
	long rating;
	long statistic_index;

	player_count = populate_statistic_buffer(statistic_buffer, statistic, inverse);
	rating = 0;
	if (statistic_buffer[0].player_index != player_index)
	{
		for (statistic_index = 1; statistic_index < player_count; statistic_index++)
		{
			if (statistic_buffer[statistic_index - 1].score != statistic_buffer[statistic_index].score)
				rating++;

			if (statistic_buffer[statistic_index].player_index == player_index)
				break;
		}
	}

	return rating;
}

long game_engine_did_player_win(
	long player_index)
{
	long result = 0;

	if (game_engine)
	{
		if (game_engine->did_player_win)
			result = game_engine->did_player_win(player_index);
		else
			result = game_engine_did_player_win_default(player_index);
	}

	return result;
}

boolean multiple_teams_alive(
	void)
{
	boolean result = FALSE;

	if (players_in_game() <= 1)
		return TRUE;
	else
	{
		struct data_iterator iterator;
		struct player_datum *player;
		long team_index = NONE;

		data_iterator_new(&iterator, player_data);
		player = (struct player_datum *)data_iterator_next(&iterator);
		while (player)
		{
			if (!player->quit_out_of_game &&
				(player->unit_index != NONE ||
					(!game_engine_player_is_odd_man_out(iterator.datum_index) &&
						!game_engine_player_is_out_of_lives(
							iterator.datum_index))))
			{
				match_assert(
					"c:\\halo\\SOURCE\\game\\game_engine.c",
					0x1BE,
					player->team_index != NONE);

				if (player->team_index != team_index)
				{
					if (team_index != NONE)
					{
						result = TRUE;
						break;
					}

					team_index = player->team_index;
				}
			}

			player = (struct player_datum *)data_iterator_next(&iterator);
		}
	}

	return result;
}

boolean team_has_players(
	long team_index)
{
	boolean result = FALSE;

	if (players_in_game() <= 1)
	{
		result = TRUE;
		goto done;
	}
	else
	{
		struct data_iterator iterator;
		struct player_datum *player;

		data_iterator_new(&iterator, player_data);
		player = (struct player_datum *)data_iterator_next(&iterator);
		while (player)
		{
			if (!player->quit_out_of_game &&
				(player->unit_index != NONE ||
					(!game_engine_player_is_odd_man_out(iterator.datum_index) &&
						!game_engine_player_is_out_of_lives(
							iterator.datum_index))))
			{
				match_assert(
					"c:\\halo\\SOURCE\\game\\game_engine.c",
					0x1F7,
					player->team_index != NONE);

				/* Preserve the retail first-eligible-player behavior. */
				if (player->team_index == team_index)
					result = TRUE;

				break;
			}

			player = (struct player_datum *)data_iterator_next(&iterator);
		}
	}

done:
	return result;
}

long game_engine_did_player_win_default(
	long player_index)
{
	long result = 0;

	if (global_variant.universal_variant.teams)
	{
		long team0_score = game_engine_get_team_score(0);
		long team1_score = game_engine_get_team_score(1);
		long winning_team_index;
		struct player_datum *player = player_get(player_index);

		if (multiple_teams_alive())
			winning_team_index = team0_score == team1_score ? NONE : (team0_score > team1_score ? 0 : 1);
		else
			winning_team_index = team_has_players(0) ? 0 : 1;

		if (winning_team_index == NONE)
			result = NONE;
		else if (player->team_index == winning_team_index)
			result = TRUE;
		else
			result = FALSE;
	}
	else
	{
		struct statistic_buffer entry;

		entry = game_engine_get_player_place(player_index);
		if (is_place_tied(&entry) &&
			!place_get_position(&entry))
			result = NONE;
		else if (!place_get_position(&entry))
			result = TRUE;
		else
			result = FALSE;
	}

	return result;
}

static long game_engine_get_type(
	void)
{
	long game_engine_type = NONE;

	if (game_engine)
		game_engine_type = game_engine->type;

	return game_engine_type;
}

struct game_variant *game_engine_get_variant(
	void)
{
	return &global_variant;
}

struct game_variant *game_engine_get_variant_by_name(
	struct game_variant *variant,
	char const *name)
{
	struct game_variant temporary;
	struct game_variant result;

	csmemset(&result, 0, sizeof(result));

	if (csstrcmp(name, "race") == 0)
		result = *build_game_variant_race(&temporary);
	else if (csstrcmp(name, "team_race") == 0)
		result = *build_game_variant_team_race(&temporary);
	else if (csstrcmp(name, "rally") == 0)
		result = *build_game_variant_rally(&temporary);
	else if (csstrcmp(name, "slayer") == 0)
		result = *build_game_variant_slayer(&temporary);
	else if (csstrcmp(name, "team_slayer") == 0)
		result = *build_game_variant_team_slayer(&temporary);
	else if (csstrcmp(name, "elimination") == 0)
		result = *build_game_variant_elimination(&temporary);
	else if (csstrcmp(name, "stalker") == 0)
		result = *build_game_variant_stalker(&temporary);
	else if (csstrcmp(name, "team_oddball") == 0)
		result = *build_game_variant_team_oddball(&temporary);
	else if (csstrcmp(name, "accumulation") == 0)
		result = *build_game_variant_accumulation(&temporary);
	else if (csstrcmp(name, "oddball") == 0)
		result = *build_game_variant_oddball(&temporary);
	else if (csstrcmp(name, "ctf") == 0)
		result = *build_game_variant_ctf(&temporary);
	else if (csstrcmp(name, "ironctf") == 0)
		result = *build_game_variant_iron_ctf(&temporary);
	else if (csstrcmp(name, "king") == 0)
		result = *build_game_variant_king(&temporary);
	else if (csstrcmp(name, "team_king") == 0)
		result = *build_game_variant_team_king(&temporary);

	*variant = result;

	return variant;
}

boolean game_engine_get_goal_in_use(
	short goal_index)
{
	return global_goal[goal_index].in_use;
}

real_point3d *game_engine_get_goal_position(
	real_point3d *position,
	short index)
{
	match_assert(
		"c:\\halo\\SOURCE\\game\\game_engine.c",
		0xFE2,
		global_goal[index].in_use);
	*position = global_goal[index].position;

	return position;
}

void game_engine_set_goal_position(
	short goal_index,
	real_point3d const *position,
	real height,
	char const *name,
	long player_index,
	short team_index,
	long ignore_player_index)
{
	/* port: a goal of the table only */
	if (!VALID_INDEX(goal_index, NUMBEROF(global_goal)))
		return;
	global_goal[goal_index].ignore_player_index = ignore_player_index;
	global_goal[goal_index].nav_index = find_nav_point(name);
	global_goal[goal_index].in_use = TRUE;
	global_goal[goal_index].position = *position;
	global_goal[goal_index].team_index = team_index;
	global_goal[goal_index].position.z += height + 0.63f;
	global_goal[goal_index].player_index = player_index;

	return;
}

boolean game_engine_man_out(
	long player_index)
{
	struct player_datum *player = player_get(player_index);
	boolean man_out = player->quit_out_of_game ||
		game_engine_player_is_out_of_lives(player_index) ||
		game_engine_player_is_odd_man_out(player_index);

	return man_out;
}

real_rgb_color *game_engine_player_get_change_color(
	real_rgb_color *change_color,
	long player_index)
{
	struct player_datum *player = player_get(player_index);
	real_rgb_color result;

	if (global_variant.universal_variant.teams)
	{
		if (player->team_index == 0)
			result = *global_real_rgb_red;
		else
			result = *global_real_rgb_blue;
	}
	else
	{
		long color_index = player->network_player_data.primary_color_index;

		if (debug_player_color != NONE)
			color_index = debug_player_color;

		result = player_profile_get_rgb_color(color_index);
	}

	*change_color = result;

	return change_color;
}

boolean game_engine_has_teams(
	void)
{
	boolean has_teams = FALSE;

	if (game_engine)
		has_teams = global_variant.universal_variant.teams;

	return has_teams;
}

boolean game_engine_allow_pause(
	void)
{
	return game_engine_globals.postgame_state==game_engine_mode_active;
}

boolean game_engine_allow_dynamic_lighting(
	void)
{
	boolean allow_dynamic_lighting = TRUE;

	if (game_engine)
		allow_dynamic_lighting = !TEST_FLAG(game_engine_globals.flags, _game_engine_disable_dynamic_light_bit);

	return allow_dynamic_lighting;
}

boolean game_engine_allow_integrated_lights(
	long object_index)
{
	boolean allow_integrated_lights = TRUE;

	if (game_engine)
		allow_integrated_lights = !TEST_FLAG(game_engine_globals.flags, _game_engine_disable_integrated_lights_bit);

	return allow_integrated_lights;
}

void game_engine_variant_cleanup(
	struct game_variant *variant)
{
	struct game_variant original = *variant;

	variant->human_readable_game_description[NUMBEROF(variant->human_readable_game_description) - 1] = 0;
	/* port: the CTF/Slayer game type of an earlier build (10) is capture the
	flag with Kills Score on (game_engine_ctf.c) */
	if (variant->game_engine_index == 10)
	{
		variant->game_engine_index = game_engine_ctf;
		variant->game_engine_variant.ctf.kills_score = TRUE;
	}
	variant->game_engine_index = PIN(variant->game_engine_index, 1, 5);
	variant->universal_variant.teams = !!variant->universal_variant.teams;
	variant->universal_variant.odd_man_out = !!variant->universal_variant.odd_man_out;
	variant->universal_variant.respawn_time_growth = MAX(variant->universal_variant.respawn_time_growth, 0);
	variant->universal_variant.respawn_time = MAX(variant->universal_variant.respawn_time, 0);
	variant->universal_variant.suicide_penalty = MAX(variant->universal_variant.suicide_penalty, 0);
	variant->universal_variant.lives = MAX(variant->universal_variant.lives, 0);
	/* port: a NaN passes no comparison, so PIN keeps it */
	if (!(variant->universal_variant.health == variant->universal_variant.health))
		variant->universal_variant.health = 1.0f;
	variant->universal_variant.health = PIN(variant->universal_variant.health, 0.25f, 4.0f);
	variant->universal_variant.weapon_set = PIN(variant->universal_variant.weapon_set, 0, NUMBER_OF_GAME_ENGINE_WEAPON_SETS - 1);
	variant->universal_variant.vehicle_set = PIN(variant->universal_variant.vehicle_set, 0, NUMBER_OF_GAME_ENGINE_VEHICLE_SETS - 1);

	switch (variant->game_engine_index)
	{
	case game_engine_ctf:
		variant->game_engine_variant.ctf.assault = !!variant->game_engine_variant.ctf.assault;
		variant->game_engine_variant.ctf.reset_on_capture = !!variant->game_engine_variant.ctf.reset_on_capture;
		variant->game_engine_variant.ctf.flag_must_reset = !!variant->game_engine_variant.ctf.flag_must_reset;
		variant->universal_variant.teams = TRUE;
		variant->game_engine_variant.ctf.flag_at_home_to_score = !!variant->game_engine_variant.ctf.flag_at_home_to_score;
		variant->game_engine_variant.ctf.single_flag_time = FLOOR(variant->game_engine_variant.ctf.single_flag_time, 0);
		/* port: Kills Score, and a capture's points (25 where none are kept:
		a gametype of a build without them) */
		variant->game_engine_variant.ctf.kills_score = !!variant->game_engine_variant.ctf.kills_score;
		if (variant->game_engine_variant.ctf.points_per_capture <= 0)
			variant->game_engine_variant.ctf.points_per_capture = 25;
		variant->game_engine_variant.ctf.points_per_capture =
			PIN(variant->game_engine_variant.ctf.points_per_capture, 1, 100);
		break;

	case game_engine_slayer:
		variant->game_engine_variant.slayer.no_death_bonus = !!variant->game_engine_variant.slayer.no_death_bonus;
		variant->game_engine_variant.slayer.no_kill_penalty = !!variant->game_engine_variant.slayer.no_kill_penalty;
		variant->game_engine_variant.slayer.kill_in_order = !!variant->game_engine_variant.slayer.kill_in_order;
		break;

	case game_engine_king:
		/* port: Multi-Hill's hills at once: 2 to 5, else none */
		if (variant->game_engine_variant.king.multi_hill < 2 || variant->game_engine_variant.king.multi_hill > 5)
			variant->game_engine_variant.king.multi_hill = 0;
		break;

	case game_engine_oddball:
		/* port: the balls' arrays and goals hold no more */
		variant->game_engine_variant.oddball.ball_spawn_count =
			PIN(variant->game_engine_variant.oddball.ball_spawn_count, 0, MAXIMUM_ODDBALLS);
		break;
	}

	if (csmemcmp(&original, variant, sizeof(original)) != 0)
	{
		struct game_variant cleaned;

		csmemcpy(&cleaned, &original, sizeof(cleaned));
		csmemcpy(&cleaned, variant, sizeof(cleaned));
		error(
			_error_silent,
			"NETGAME CODE FAILURE: game_engine_variant_cleanup changed the variant");
	}

	return;
}

/* port: one of the globals' three multiplayer vehicles (0 warthog, 1 ghost,
2 scorpion), or NONE for one the map's globals lack: a Custom Edition map's
may have fewer, which the original read past the end of */
static long game_engine_multiplayer_vehicle(
	long index)
{
	struct game_globals *game_globals = scenario_get_game_globals();
	struct game_globals_multiplayer_information *information;

	if (game_globals->multiplayer_information.count <= 0)
		return NONE;
	information = TAG_BLOCK_GET_ELEMENT(
		&game_globals->multiplayer_information,
		0,
		struct game_globals_multiplayer_information);
	if (index >= information->vehicles.count)
		return NONE;
	return TAG_BLOCK_GET_ELEMENT(&information->vehicles, index, struct game_globals_vehicle)->vehicle.index;
}

static void game_engine_predict_multiplayer_vehicle(
	long index)
{
	long definition_index = game_engine_multiplayer_vehicle(index);

	if (definition_index != NONE)
		object_definition_predict(definition_index);
}

static void game_engine_predict_resources(
	void)
{
	long weapon_indices[10];
	long weapon_index;

	switch (global_variant.universal_variant.vehicle_set)
	{
	case _game_engine_vehicles_warthog:
		game_engine_predict_multiplayer_vehicle(0);
		break;

	case _game_engine_vehicles_ghost:
		game_engine_predict_multiplayer_vehicle(1);
		break;

	case _game_engine_vehicles_tank:
		game_engine_predict_multiplayer_vehicle(2);
		break;

	default:
		game_engine_predict_multiplayer_vehicle(0);
		game_engine_predict_multiplayer_vehicle(1);
		game_engine_predict_multiplayer_vehicle(2);
		break;
	}

	object_definition_predict(list_index_to_weapon_definition_index(_weapon_list_frag_grenade));
	object_definition_predict(list_index_to_weapon_definition_index(_weapon_list_plasma_grenade));

	/* port: (and Infection's zombies' skull) */
	if (global_variant.game_engine_index == game_engine_oddball ||
		game_modes_variant_mode(&global_variant, game_variant_options_get()) == _game_mode_infection)
	{
		object_definition_predict(list_index_to_weapon_definition_index(_weapon_list_ball));
	}

	if (global_variant.game_engine_index == game_engine_ctf)
		object_definition_predict(list_index_to_weapon_definition_index(_weapon_list_flag));

	weapon_indices[0] = list_index_to_weapon_definition_index(0);
	weapon_indices[1] = list_index_to_weapon_definition_index(1);
	weapon_indices[2] = list_index_to_weapon_definition_index(2);
	weapon_indices[3] = list_index_to_weapon_definition_index(3);
	weapon_indices[4] = list_index_to_weapon_definition_index(4);
	weapon_indices[5] = list_index_to_weapon_definition_index(5);
	weapon_indices[6] = list_index_to_weapon_definition_index(6);
	weapon_indices[7] = list_index_to_weapon_definition_index(7);
	weapon_indices[8] = list_index_to_weapon_definition_index(8);
	weapon_indices[9] = list_index_to_weapon_definition_index(9);

	for (weapon_index = 0; weapon_index < 10; weapon_index++)
	{
		object_definition_predict(
			game_engine_remap_weapon(weapon_indices[weapon_index]));
	}

	return;
}

void game_engine_initialize(
	struct game_variant *variant)
{
	csmemset(&game_engine_globals, 0, sizeof(game_engine_globals));
	game_engine_globals.postgame_state = game_engine_mode_active;

	if (variant && variant->game_engine_index)
	{
		global_variant = *variant;
		game_engine_variant_cleanup(&global_variant);
		/* port: the cleaned variant's (the one given may be any number) */
		game_engine = game_engines[global_variant.game_engine_index];
		/* port: an OpenCE gametype's mode's (port/linux/game/game_modes.c) */
		game_engine = game_modes_engine(game_engine);
	}

	return;
}

void game_engine_initialize_for_new_map(
	void)
{
	if (game_engine)
	{
		game_engine_verify_current_map();
		game_engine_intialize_queued_sounds();
		csmemset(global_goal, 0, sizeof(global_goal));
		game_engine_globals.next_team_index = 0;
		csmemset(game_engine_betrayal_penalty, 0, sizeof(game_engine_betrayal_penalty));
		game_engine_vehicle_home_count = NONE;
		timeout_for_endgame_sound = 0;
		game_engine_network_state_read = FALSE;

		if (game_engine->initialize_for_new_map &&
			!game_engine->initialize_for_new_map())
		{
			error(
				_error_silent,
				"failed to initialize custome game engine for new map, reverting to default game engine");

			if (game_engine)
			{
				if (game_engine->dispose)
					game_engine->dispose();

				game_engine = NULL;
			}
		}

		game_engine_predict_resources();
	}

	return;
}

void game_engine_player_added(
	long player_index)
{
	initialize_player_multiplayer_data(player_index);
	/* port: not the friendly fire penalty of the slot's last player */
	if (DATUM_INDEX_TO_ABSOLUTE_INDEX(player_index) < HALO_PORT_MAXIMUM_NETWORK_PLAYERS)
		game_engine_betrayal_penalty[DATUM_INDEX_TO_ABSOLUTE_INDEX(player_index)] = 0;

	if (game_engine)
	{
		struct player_datum *player = player_get(player_index);
		long *next_team_index = &game_engine_globals.next_team_index;

		if (global_variant.universal_variant.teams)
		{
			if (global_network_game_client_get())
			{
				player->team_index =
					PIN((signed char)player->network_player_data.team_index % 2, 0, 1);
			}
			else
			{
				player->network_player_data.team_index =
					(char)*next_team_index;
				player->team_index =
					(signed char)*next_team_index;
				*next_team_index = (*next_team_index + 1) % 2;
			}
		}
		else
		{
			/* port: a free-for-all team is the player's own slot, which stays
			below the player limit and is the same on every machine */
			long team_index = DATUM_INDEX_TO_ABSOLUTE_INDEX(player_index);

			player->network_player_data.team_index =
				(char)team_index;
			player->team_index =
				(signed char)team_index;
		}

		if (player_index != NONE)
		{
			multiplayer_message(player_index, 0, NONE);
		}
		else
		{
			struct data_iterator iterator;

			data_iterator_new(&iterator, player_data);
			while (data_iterator_next(&iterator))
				multiplayer_message(iterator.datum_index, 0, NONE);
		}

		if (game_engine->player_added)
			game_engine->player_added(player_index);
	}

	return;
}

real game_engine_get_distance_rating_for_spawn(
	long player_index,
	real_point3d const *position)
{
	boolean has_teams = game_engine ? global_variant.universal_variant.teams : FALSE;
	struct player_datum *player;
	struct data_iterator iterator;
	struct player_datum *other_player;
	real rating;

	player = player_get(player_index);
	rating = 1.0f;
	data_iterator_new(&iterator, player_data);
	other_player = (struct player_datum *)data_iterator_next(&iterator);
	while (other_player)
	{
		if (other_player->unit_index!=NONE)
		{
			real_point3d origin;
			real distance;

			object_get_origin(other_player->unit_index, &origin);
			distance = distance3d(&origin, position);

			if (!has_teams ||
				other_player->team_index!=player->team_index ||
				!(distance>0.25f))
			{
				if (distance<0.25f)
					rating = 0.0f;
				else if (distance<1.0f)
					rating *= 0.1f;

				if (other_player->team_index!=player->team_index)
				{
					if (distance<2.0f)
						rating = 0.0f;
					else if (!(distance>5.0f))
						rating = (distance-2.0f)*rating*0.33333334f;
				}
			}
		}

		other_player = (struct player_datum *)data_iterator_next(&iterator);
	}

	return rating;
}


real game_engine_get_starting_location_rating(
	long player_index,
	struct player_starting_location const *starting_location)
{
	/* port: (or CTF's: game_engine_force_team_spawns) */
	if (!match_game_type(
		game_engine_force_team_spawns(player_index) ? game_engine_ctf : game_engine_get_type(),
		4,
		starting_location->game_types))
	{
		return 0.0f;
	}

	if (nearby_vehicle(player_index, starting_location))
		return 0.0f;

	return default_starting_location_rate_function(player_index, starting_location);
}

/* port: the gametype's vehicles of each team (game_variant_options) */
static byte game_engine_vehicles_placed[2][NUMBER_OF_VARIANT_VEHICLES];

void game_engine_vehicle_placement_begin(
	void)
{
	csmemset(game_engine_vehicles_placed, 0, sizeof(game_engine_vehicles_placed));
}

/* a vehicle definition's type: the globals' multiplayer vehicles (warthog,
ghost, scorpion), else by its tag's name; NONE for none of them */
static short game_engine_variant_vehicle_type(
	long definition_index)
{
	struct game_globals *game_globals = scenario_get_game_globals();
	struct game_globals_multiplayer_information *information;
	char const *name;
	short index;

	if (definition_index == NONE)
		return NONE;
	if (game_globals->multiplayer_information.count > 0)
	{
		information = TAG_BLOCK_GET_ELEMENT(&game_globals->multiplayer_information, 0,
			struct game_globals_multiplayer_information);
		for (index = 0; index < 3 && index < information->vehicles.count; index++)
		{
			if (TAG_BLOCK_GET_ELEMENT(&information->vehicles, index, struct game_globals_vehicle)->vehicle.index ==
				definition_index)
			{
				return index == 0 ? _variant_vehicle_warthog : index == 1 ? _variant_vehicle_ghost :
					_variant_vehicle_scorpion;
			}
		}
	}
	name = tag_get_name(definition_index);
	if (!name)
		return NONE;
	if (strstr(name, "rwarthog") || strstr(name, "rocket"))
		return _variant_vehicle_rocket_warthog;
	if (strstr(name, "banshee"))
		return _variant_vehicle_banshee;
	if (strstr(name, "turret"))
		return _variant_vehicle_gun_turret;
	if (strstr(name, "warthog"))
		return _variant_vehicle_warthog;
	if (strstr(name, "ghost"))
		return _variant_vehicle_ghost;
	if (strstr(name, "scorpion"))
		return _variant_vehicle_scorpion;
	return NONE;
}

/* the team whose spawn points are nearest the point (red's if none) */
static short game_engine_nearest_team(
	real_point3d const *point)
{
	struct scenario *scenario = global_scenario_get();
	real best = -1.0f;
	short team = 0;
	long index;

	for (index = 0; index < scenario->players.count; index++)
	{
		struct player_starting_location *location =
			TAG_BLOCK_GET_ELEMENT(&scenario->players, index, struct player_starting_location);
		real dx = location->position.x - point->x;
		real dy = location->position.y - point->y;
		real dz = location->position.z - point->z;
		real distance = dx * dx + dy * dy + dz * dz;

		if (VALID_INDEX(location->team_index, 2) && (best < 0.0f || distance < best))
		{
			best = distance;
			team = location->team_index;
		}
	}
	return team;
}

boolean game_engine_vehicle_placement_allowed(
	struct scenario_object_datum const *placement,
	struct tag_block *palette)
{
	struct game_variant_options const *options = game_variant_options_get();
	short side, type;
	byte set;

	if (!game_engine || placement->palette_entry_index == NONE)
		return TRUE;
	side = global_variant.universal_variant.teams ? game_engine_nearest_team(&placement->position) : 0;
	set = options->vehicle_set[side];
	if (set == VARIANT_VEHICLE_SET_PC)
		return TRUE;
	type = game_engine_variant_vehicle_type(TAG_BLOCK_GET_ELEMENT(palette, placement->palette_entry_index,
		struct scenario_object_palette_entry)->reference.index);
	/* (the map's own: the multiplayer ones of the globals, as the Xbox
	game's remap allows) */
	if (set == _game_engine_vehicles_default)
		return type == NONE || type <= _variant_vehicle_scorpion;
	if (type == NONE)
		return set != _game_engine_vehicles_none;
	if (set == VARIANT_VEHICLE_SET_CUSTOM)
	{
		if (game_engine_vehicles_placed[side][type] >= options->vehicle_counts[side][type])
			return FALSE;
		game_engine_vehicles_placed[side][type]++;
		return TRUE;
	}
	/* (the sets: none, then each type alone, in the options' order) */
	return set != _game_engine_vehicles_none && set - _game_engine_vehicles_warthog == type;
}

/* port: the gametype's friendly fire (game_variant_options) on a teammate's
hit of the object (its damage marked friendly: object_cause_damage): all of
it, none, its shields only, or explosives' only. A player's own hit of
himself, or of what is not a player's teammate's, is the game's own. */
short game_engine_friendly_damage(
	long attacker_player_index,
	long object_index,
	boolean explosive)
{
	struct game_variant_options const *options = game_variant_options_get();
	struct player_datum *attacker;
	short friendly_fire;

	/* port: network co-op's friendly fire is Server Setup's FRIENDLY FIRE,
	in the host's game settings (only the host deals damage), between its
	players. Each player also has a choice of their own
	(network_coop_players_friendly_fire). Their AI allies they hurt as in the
	campaign, or not at all with Server Setup's ALLY FRIENDLY FIRE off
	(network_coop_ally_friendly_fire), so that no player can kill the allies
	a level needs; the AI's own hits of its allies are the game's own */
	if (network_coop_active())
	{
		struct network_game *game = network_game_get_game();
		struct unit_datum *unit = (struct unit_datum *)object_try_and_get_and_verify_type(object_index,
			_object_mask_unit);

		if (!game || !unit)
			return _friendly_damage_all;
		if (unit->unit.player_index == NONE)
		{
			return attacker_player_index != NONE && !network_coop_ally_friendly_fire() ?
				_friendly_damage_none : _friendly_damage_all;
		}
		if (!network_coop_players_friendly_fire(attacker_player_index, unit->unit.player_index))
			return _friendly_damage_none;
		friendly_fire = game->variant_options.friendly_fire;
	}
	else if (!game_engine || !global_variant.universal_variant.teams)
		return _friendly_damage_all;
	else
		friendly_fire = options->friendly_fire;
	if (attacker_player_index == NONE || friendly_fire == _friendly_fire_on)
		return _friendly_damage_all;
	attacker = (struct player_datum *)datum_try_and_get(player_data, attacker_player_index);
	if (!attacker || attacker->unit_index == object_index)
		return _friendly_damage_all;
	switch (friendly_fire)
	{
	case _friendly_fire_off: return _friendly_damage_none;
	case _friendly_fire_shields_only: return _friendly_damage_shields;
	case _friendly_fire_explosives_only: return explosive ? _friendly_damage_all : _friendly_damage_none;
	}
	return _friendly_damage_all;
}

real game_engine_get_damage_multiplier(
	long damaging_player_index,
	long damaged_player_index)
{
	real result = 1.0f;

	if (game_engine)
		result /= PIN(global_variant.universal_variant.health, 0.25f, 4.0f);

	if (damaging_player_index != NONE &&
		damaged_player_index != NONE)
	{
		if (game_engine_test_trait(damaging_player_index, 2))
		{
			result *= 1.5f;
		}

		if (game_engine_test_trait(damaged_player_index, 3))
		{
			result *= 0.5f;
		}
	}

	return result;
}

long game_engine_remap_vehicle(
	long vehicle_definition_index)
{
	long result = vehicle_definition_index;

	if (mcc_vehicle_placement_rules())
		return result;

	/* port: a Halo Custom Edition map's vehicles are chosen by their
	placements, and its scripts may create any
	(port/linux/game/custom_edition_objects.c) */
	if (custom_edition_vehicles_by_placement())
		return result;
	/* port: and the PC vehicle set's are every one the map places
	(game_engine_vehicle_placement_allowed) */
	if (game_engine && (game_variant_options_get()->vehicle_set[0] == VARIANT_VEHICLE_SET_PC ||
		game_variant_options_get()->vehicle_set[1] == VARIANT_VEHICLE_SET_PC))
	{
		return result;
	}
	if (game_engine)
	{
		/* (port: NONE for one the map's globals lack) */
		long vehicle0 = game_engine_multiplayer_vehicle(0);
		long vehicle1 = game_engine_multiplayer_vehicle(1);
		long vehicle2 = game_engine_multiplayer_vehicle(2);

		/* (port: and the other types a gametype's sets name, which a map may
		have: game_engine_variant_vehicle_type) */
		if (result != vehicle0 &&
			result != vehicle1 &&
			result != vehicle2 &&
			((game_variant_options_get()->vehicle_set[0] == _game_engine_vehicles_default &&
				game_variant_options_get()->vehicle_set[1] == _game_engine_vehicles_default) ||
				game_engine_variant_vehicle_type(result) == NONE))
		{
			result = NONE;
		}

		/* port: the per-team sets decide at placement
		(game_engine_vehicle_placement_allowed) */
		switch (game_variant_options_get()->vehicle_set[0] == game_variant_options_get()->vehicle_set[1] ?
			global_variant.universal_variant.vehicle_set : _game_engine_vehicles_default)
		{
		case _game_engine_vehicles_none:
			result = NONE;
			break;

		case _game_engine_vehicles_warthog:
			if (vehicle0 != result)
				result = NONE;
			break;

		case _game_engine_vehicles_ghost:
			if (vehicle1 != result)
				result = NONE;
			break;

		case _game_engine_vehicles_tank:
			if (vehicle2 != result)
				result = NONE;
			break;
		}
	}

	return result;
}

long game_engine_remap_weapon(
	long weapon_definition_index)
{
	long weapon_list_index =
		weapon_definition_index_to_list_index(weapon_definition_index);

	if (weapon_list_index == _weapon_list_ball ||
		weapon_list_index == _weapon_list_flag ||
		weapon_list_index == NONE)
	{
		return weapon_definition_index;
	}

	/* (port: and the gravity rifle, a cut weapon, as the flamethrower,
	which the Xbox's maps have unfinished: neither is a multiplayer weapon) */
	if (weapon_list_index == _weapon_list_flamethrower || weapon_list_index == _weapon_list_gravity_rifle)
		weapon_list_index = _weapon_list_rocket_launcher;

	/* port: a custom loadout (game_variant_options) has no weapon set */
	if (game_variant_options_get()->loadout == _loadout_custom)
		return list_index_to_weapon_definition_index(weapon_list_index);

	switch (global_variant.universal_variant.weapon_set)
	{
	case _game_engine_weapons_pistols:
		switch (weapon_list_index)
		{
		case _weapon_list_needler:
		case _weapon_list_pistol:
		case _weapon_list_plasma_rifle:
		case _weapon_list_rocket_launcher:
			weapon_list_index = _weapon_list_plasma_pistol;
			break;

		case _weapon_list_plasma_pistol:
		default:
			weapon_list_index = _weapon_list_pistol;
			break;
		}
		break;

	case _game_engine_weapons_assault_rifles:
		switch (weapon_list_index)
		{
		case _weapon_list_needler:
		case _weapon_list_pistol:
		case _weapon_list_plasma_rifle:
		case _weapon_list_rocket_launcher:
			weapon_list_index = _weapon_list_plasma_rifle;
			break;

		case _weapon_list_plasma_pistol:
		default:
			weapon_list_index = _weapon_list_assault_rifle;
			break;
		}
		break;

	case _game_engine_weapons_plasma_weapons:
		if (weapon_list_index >= _weapon_list_needler && weapon_list_index <= _weapon_list_plasma_pistol)
			weapon_list_index = _weapon_list_plasma_pistol;
		else
			weapon_list_index = _weapon_list_plasma_rifle;
		break;

	case _game_engine_weapons_sniping:
		if (weapon_list_index != _weapon_list_pistol && weapon_list_index != _weapon_list_sniper_rifle)
			weapon_list_index = _weapon_list_sniper_rifle;
		break;

	case _game_engine_weapons_no_sniping:
		switch (weapon_list_index)
		{
		case _weapon_list_pistol:
			weapon_list_index = _weapon_list_assault_rifle;
			break;

		case _weapon_list_sniper_rifle:
			weapon_list_index = _weapon_list_shotgun;
			break;
		}
		break;

	case _game_engine_weapons_rocket_launchers:
		weapon_list_index = _weapon_list_rocket_launcher;
		break;

	case _game_engine_weapons_shotguns:
		weapon_list_index = _weapon_list_shotgun;
		break;

	case _game_engine_weapons_short_range:
		switch (weapon_list_index)
		{
		case _weapon_list_assault_rifle:
		case _weapon_list_needler:
		case _weapon_list_pistol:
		case _weapon_list_sniper_rifle:
			weapon_list_index = _weapon_list_shotgun;
			break;
		}
		break;

	case _game_engine_weapons_human:
		switch (weapon_list_index)
		{
		case _weapon_list_needler:
		case _weapon_list_plasma_rifle:
			weapon_list_index = _weapon_list_assault_rifle;
			break;

		case _weapon_list_plasma_pistol:
			weapon_list_index = _weapon_list_pistol;
			break;
		}
		break;

	/* port: the PC version's sets. Covenant: the Covenant's weapons (the
	needler for the rocket launcher: the Xbox's maps have no fuel rod) */
	case _game_engine_weapons_covenant:
		switch (weapon_list_index)
		{
		case _weapon_list_assault_rifle:
			weapon_list_index = _weapon_list_plasma_rifle;
			break;

		case _weapon_list_pistol:
			weapon_list_index = _weapon_list_plasma_pistol;
			break;

		case _weapon_list_shotgun:
		case _weapon_list_sniper_rifle:
			weapon_list_index = _weapon_list_needler;
			break;

		case _weapon_list_rocket_launcher:
			weapon_list_index = _weapon_list_needler;
			break;
		}
		break;

	/* classic: the Xbox game's weapons (the flamethrower and the gravity
	rifle are already rocket launchers) */
	case _game_engine_weapons_classic:
		break;

	/* heavy weapons: rocket launchers */
	case _game_engine_weapons_heavy:
		weapon_list_index = _weapon_list_rocket_launcher;
		break;
	}

	return list_index_to_weapon_definition_index(weapon_list_index);
}

long game_engine_remap_object_definition(
	long definition_index)
{
	short object_type;

	if (!game_engine_running() || definition_index==NONE)
	{
		return definition_index;
	}

	object_type = object_definition_get(definition_index)->object.type;
	if (object_type==_object_type_vehicle)
	{
		return game_engine_remap_vehicle(definition_index);
	}
	if (object_type==_object_type_weapon)
	{
		return game_engine_remap_weapon(definition_index);
	}
	if (object_type==_object_type_equipment)
	{
		return game_engine_remap_equipment(definition_index);
	}

	return definition_index;
}

/* ---------- private code */

static void netgame_flag_verify_no_team_duplicates(
	short flag_type,
	char const *error_message)
{
	struct scenario *scenario = global_scenario_get();
	/* port: long counters, for a map's long count */
	long flag_index;

	for (flag_index = 0;
		flag_index < scenario->netgame_flags.count;
		flag_index++)
	{
		struct scenario_netgame_flag *flag = TAG_BLOCK_GET_ELEMENT(
			&scenario->netgame_flags,
			flag_index,
			struct scenario_netgame_flag);
		long duplicate_index;

		if (flag_type != flag->type)
			continue;

		for (duplicate_index = flag_index + 1;
			duplicate_index < scenario->netgame_flags.count;
			duplicate_index++)
		{
			struct scenario_netgame_flag *duplicate = TAG_BLOCK_GET_ELEMENT(
				&scenario->netgame_flags,
				duplicate_index,
				struct scenario_netgame_flag);

			if (flag_type == duplicate->type &&
				duplicate->team_index == flag->team_index)
			{
				error(
					_error_silent,
					error_message,
					duplicate->team_index);
			}
		}
	}

	return;
}

static void update_weapon_inventory(
	long weapon_index)
{
	struct weapon_datum *weapon;

	match_assert(
		"c:\\halo\\SOURCE\\game\\game_engine.c",
		0x80A,
		weapon_index != NONE);
	match_assert(
		"c:\\halo\\SOURCE\\game\\game_engine.c",
		0x80B,
		weapon_is_flag(weapon_index));

	weapon = weapon_get(weapon_index);
	if (weapon->object.parent_object_index == NONE &&
		!TEST_FLAG(weapon->item.flags, _item_attached_to_unit_bit) &&
		TEST_FLAG(
			weapon->weapon.flags,
			_weapon_runtime_game_engine_active_bit))
	{
		weapon->weapon.flags &=
			~FLAG(_weapon_runtime_game_engine_active_bit);

		if (game_engine->weapon_dropped)
			game_engine->weapon_dropped(weapon_index);
	}

	return;
}

static void game_engine_update_weapons(
	void)
{
	struct object_iterator iterator;

	match_assert(
		"c:\\halo\\SOURCE\\game\\game_engine.c",
		0x828,
		NULL != game_engine);

	object_iterator_new(
		&iterator,
		_object_mask_item,
		0);
	while (object_iterator_next(&iterator))
	{
		struct item_datum *item = item_get(iterator.index);

		if (TEST_FLAG(item->item.flags, _item_attached_to_unit_bit))
		{
			item->object.scale = 1.f;
		}
		else
		{
			struct item_definition *definition =
				item_definition_get(item->definition_index);

			item->object.scale =
				definition->item.scale != 0.f
					? definition->item.scale
					: 1.f;

			match_assert(
				"c:\\halo\\SOURCE\\game\\game_engine.c",
				0x83E,
				((item->object.scale >= 0.5f) &&
					(item->object.scale <= 3.f)) ||
				/* port: MCC-authored miniature items retain their tag scale. */
				mcc_item_small_scale(item->object.scale));
		}

		/*
		 * Slot 0x38 is populated by the CTF and oddball engines. It receives
		 * each objective weapon during the per-tick item scan.
		 */
		if (game_engine->objective_weapon_update)
		{
			struct weapon_datum *weapon = weapon_try_and_get(iterator.index);

			if (weapon && weapon_is_flag(iterator.index))
			{
				update_weapon_inventory(iterator.index);
				game_engine->objective_weapon_update(
					iterator.index,
					weapon);
			}
		}
	}

	return;
}

static void netgame_flag_verify_team_range(
	short flag_type,
	short minimum_index,
	short maximum_index,
	char const *error_message)
{
	struct scenario *scenario = global_scenario_get();
	/* port: a long counter, for a map's long count */
	long flag_index;

	for (flag_index = 0;
		flag_index < scenario->netgame_flags.count;
		flag_index++)
	{
		struct scenario_netgame_flag *flag = TAG_BLOCK_GET_ELEMENT(
			&scenario->netgame_flags,
			flag_index,
			struct scenario_netgame_flag);

		if (flag_type == flag->type &&
			(flag->team_index < minimum_index ||
				flag->team_index > maximum_index))
		{
			error(
				_error_silent,
				error_message,
				flag->team_index);
		}
	}

	return;
}

static void netgame_verify_equipment(
	short game_type,
	char const *error_message)
{
	long matching_count = 0;
	struct scenario *scenario = global_scenario_get();
	/* port: a long counter, for a map's long count */
	long equipment_index;

	for (equipment_index = 0;
		equipment_index < scenario->netgame_equipment.count;
		equipment_index++)
	{
		struct scenario_netgame_equipment *equipment =
			TAG_BLOCK_GET_ELEMENT(
				&scenario->netgame_equipment,
				equipment_index,
				struct scenario_netgame_equipment);

		if (match_game_type(
			game_type,
			4,
			equipment->game_type))
		{
			matching_count++;
		}
	}

	if (matching_count == 0)
		error(_error_silent, error_message);

	return;
}

static void netgame_flag_verify_team_exists(
	short flag_type,
	short flag_index,
	char const *error_message)
{
	if (find_netgame_flag(
		NULL,
		0.f,
		0.f,
		flag_type,
		flag_index) == NONE)
	{
		error(
			_error_silent,
			error_message,
			flag_index);
	}

	return;
}

static void game_engine_verify_current_map(
	void)
{
	netgame_flag_verify_team_exists(
		_netgame_flag_ctf_flag,
		0,
		"NETGAME MAP FAILURE: missing ctf flag [team %d]");
	netgame_flag_verify_team_exists(
		_netgame_flag_ctf_flag,
		1,
		"NETGAME MAP FAILURE: missing ctf flag [team %d]");

	netgame_flag_verify_no_team_duplicates(
		_netgame_flag_ctf_flag,
		"NETGAME MAP FAILURE: duplicate ctf flag [team %d]");
	netgame_flag_verify_team_range(
		_netgame_flag_ctf_flag,
		0,
		1,
		"NETGAME MAP FAILURE: ctf flag out of range [team %d]");

	netgame_flag_verify_team_exists(
		_netgame_flag_hill,
		0,
		"NETGAME MAP FAILURE: missing hill flag [team %d]");
	netgame_flag_verify_team_exists(
		_netgame_flag_hill,
		1,
		"NETGAME MAP FAILURE: missing hill flag [team %d]");
	netgame_flag_verify_team_exists(
		_netgame_flag_oddball_ball_spawn,
		0,
		"NETGAME MAP FAILURE: missing oddball flag [team %d]");
	netgame_flag_verify_team_exists(
		_netgame_flag_oddball_ball_spawn,
		1,
		"NETGAME MAP FAILURE: missing oddball flag [team %d]");
	netgame_flag_verify_team_exists(
		_netgame_flag_race_track,
		0,
		"NETGAME MAP FAILURE: missing race flag [team %d]");
	netgame_flag_verify_team_exists(
		_netgame_flag_race_track,
		1,
		"NETGAME MAP FAILURE: missing race flag [team %d]");

	netgame_flag_verify_no_team_duplicates(
		_netgame_flag_race_track,
		"NETGAME MAP FAILURE: duplicate race track flag [team %d]");

	/* (the team index is not read: both checks count every ctf spawn) */
	netgame_verify_spawn_points(
		game_engine_ctf,
		0,
		4,
		"NETGAME MAP FAILURE: failed to find enough spawn points for ctf team 0 (%d/%d)");
	netgame_verify_spawn_points(
		game_engine_ctf,
		0,
		4,
		"NETGAME MAP FAILURE: failed to find enough spawn points for ctf team 1 (%d/%d)");
	netgame_verify_spawn_points(
		game_engine_slayer,
		0,
		4,
		"NETGAME MAP FAILURE: failed to find enough spawn points for slayer %d/%d");
	netgame_verify_spawn_points(
		game_engine_oddball,
		0,
		4,
		"NETGAME MAP FAILURE: failed to find enough spawn points for oddball %d/%d");
	netgame_verify_spawn_points(
		game_engine_king,
		0,
		4,
		"NETGAME MAP FAILURE: failed to find enough spawn points for king %d/%d");
	netgame_verify_spawn_points(
		game_engine_race,
		0,
		4,
		"NETGAME MAP FAILURE: failed to find enough spawn points for race %d/%d");

	netgame_verify_equipment(
		game_engine_ctf,
		"NETGAME MAP FAILURE: failed to find any equipment for ctf");
	netgame_verify_equipment(
		game_engine_slayer,
		"NETGAME MAP FAILURE: failed to find any equipment for slayer");
	netgame_verify_equipment(
		game_engine_oddball,
		"NETGAME MAP FAILURE: failed to find any equipment for oddball");
	netgame_verify_equipment(
		game_engine_king,
		"NETGAME MAP FAILURE: failed to find any equipment for king");
	netgame_verify_equipment(
		game_engine_race,
		"NETGAME MAP FAILURE: failed to find any equipment for race");

	return;
}

static void internal_rasterize_target_name(
	long player_index)
{
	struct player_datum *player;
	long target_player_index;

	player = player_get(player_index);
	target_player_index = NONE;
	if (player->local_player_index != NONE && player->unit_index != NONE)
	{
		target_player_index = find_closest_player_index(player_index);
		if (target_player_index == player_index)
			target_player_index = NONE;
	}

	/* This is drawn once a frame, several frames per tick
	(render_interpolation.c): the hold time counts ticks, as it did on the
	Xbox. */
	{
		static long last_game_times[MAXIMUM_NUMBER_OF_LOCAL_PLAYERS];
		long *last_game_time = NULL;

		if (player->local_player_index >= 0 &&
			player->local_player_index < MAXIMUM_NUMBER_OF_LOCAL_PLAYERS)
		{
			last_game_time = &last_game_times[player->local_player_index];
		}
		if (!last_game_time || *last_game_time != game_time_get())
		{
			if (last_game_time)
				*last_game_time = game_time_get();
	if (player->player_display_index != target_player_index)
	{
		if (player->player_display_count > 0)
			player->player_display_count--;
		if (player->player_display_count == 0)
			player->player_display_index = target_player_index;
	}
	else if (player->player_display_count < 15)
	{
		player->player_display_count++;
	}
		}
	}

	if (player->player_display_index != NONE)
	{
		struct player_datum *target_player = player_get(player->player_display_index);
		wchar_t target_name[12] = { 0 };
		long hold_time = player->player_display_count;
		real alpha;

		if (hold_time >= 10)
			hold_time = 10;
		ustrncpy(target_name, target_player->name, NUMBEROF(target_name) - 1);
		target_name[NUMBEROF(target_name) - 1] = 0;
		alpha = linear_to_non_linear_alpha(hold_time * 0.1f) * 0.5f;
		game_engine_rasterize_message(target_name, alpha);
	}

	return;
}

static boolean internal_rasterize_score(
	long player_index,
	long message,
	long message_data,
	wchar_t *buffer,
	long buffer_size)
{
	struct player_datum *player;
	struct player_datum *other_player;
	long score;
	long string_list_index;
	wchar_t const *format;
	boolean result;

	result = TRUE;
	player = player_get(player_index);
	score = 0;

	if (game_engine_test_flag(1))
	{
		switch (message)
		{
		case _game_engine_message_multi_kill:
			message = _game_engine_message_multi_kill_with_score;
			break;
		case _game_engine_message_triple_kill:
			message = _game_engine_message_triple_kill_with_score;
			break;
		case _game_engine_message_double_kill:
			message = _game_engine_message_double_kill_with_score;
			break;
		case _game_engine_message_ten_kills_in_a_row:
			message = _game_engine_message_ten_kills_in_a_row_with_score;
			break;
		case _game_engine_message_five_kills_in_row:
			message = _game_engine_message_five_kills_in_row_with_score;
			break;
		case _game_engine_message_killed_enemy:
			message = _game_engine_message_killed_enemy_with_score;
			break;
		default:
			break;
		}

		if (message >= _game_engine_message_multi_kill_with_score &&
			message <= _game_engine_message_killed_enemy_with_score)
		{
			/* port: a client of the distributed netcode replaying the host's
			killing blow: the score the host's game type gave its killer
			(its own has it only once the game type's state comes) */
			if (!network_damage_killer_score(player_index, &score))
				score = game_engine->get_player_score(player_index, TRUE);
		}
	}

	switch (message)
	{
	case _game_engine_message_welcome:
		string_list_index = tag_loaded('ustr', "ui\\multiplayer_game_text");
		if (string_list_index != NONE)
			format = unicode_string_list_get_string(string_list_index, 0x4A);
		else
			format = L"";
		usnprintf(
			buffer,
			buffer_size,
			ustring_format_checked(format, "s"),
			player->name);
		break;
	case _game_engine_message_killed_by_unknown:
		string_list_index = tag_loaded('ustr', "ui\\multiplayer_game_text");
		if (string_list_index != NONE)
			format = unicode_string_list_get_string(string_list_index, 0x4B);
		else
			format = L"";
		usnprintf(
			buffer,
			buffer_size,
			ustring_format_checked(format, "s"),
			player->name);
		break;
	case _game_engine_message_killed_by_biped:
		string_list_index = tag_loaded('ustr', "ui\\multiplayer_game_text");
		if (string_list_index != NONE)
			format = unicode_string_list_get_string(string_list_index, 0x4C);
		else
			format = L"";
		usnprintf(
			buffer,
			buffer_size,
			ustring_format_checked(format, "s"),
			player->name);
		break;
	case _game_engine_message_killed_by_vehicle:
		string_list_index = tag_loaded('ustr', "ui\\multiplayer_game_text");
		if (string_list_index != NONE)
			format = unicode_string_list_get_string(string_list_index, 0x4D);
		else
			format = L"";
		usnprintf(
			buffer,
			buffer_size,
			ustring_format_checked(format, "s"),
			player->name);
		break;
	case _game_engine_message_killed_by_player:
		other_player = player_get(message_data);
		string_list_index = tag_loaded('ustr', "ui\\multiplayer_game_text");
		if (string_list_index != NONE)
			format = unicode_string_list_get_string(string_list_index, 0x4E);
		else
			format = L"";
		usnprintf(
			buffer,
			buffer_size,
			ustring_format_checked(format, "ss"),
			player->name,
			other_player->name);
		break;
	case _game_engine_message_killed_by_friendly_fire:
		other_player = player_get(message_data);
		string_list_index = tag_loaded('ustr', "ui\\multiplayer_game_text");
		if (string_list_index != NONE)
			format = unicode_string_list_get_string(string_list_index, 0x4F);
		else
			format = L"";
		usnprintf(
			buffer,
			buffer_size,
			ustring_format_checked(format, "ss"),
			player->name,
			other_player->name);
		break;
	case _game_engine_message_quit:
		other_player = player_get(message_data);
		string_list_index = tag_loaded('ustr', "ui\\multiplayer_game_text");
		if (string_list_index != NONE)
			format = unicode_string_list_get_string(string_list_index, 0x50);
		else
			format = L"";
		usnprintf(
			buffer,
			buffer_size,
			ustring_format_checked(format, "s"),
			other_player->name);
		break;
	case _game_engine_message_killed_by_self:
		string_list_index = tag_loaded('ustr', "ui\\multiplayer_game_text");
		if (string_list_index != NONE)
			format = unicode_string_list_get_string(string_list_index, 0x51);
		else
			format = L"";
		usnprintf(
			buffer,
			buffer_size,
			ustring_format_checked(format, "s"),
			player->name);
		break;
	case _game_engine_message_killed_friendly:
		other_player = player_get(message_data);
		string_list_index = tag_loaded('ustr', "ui\\multiplayer_game_text");
		if (string_list_index != NONE)
			format = unicode_string_list_get_string(string_list_index, 0x52);
		else
			format = L"";
		usnprintf(
			buffer,
			buffer_size,
			ustring_format_checked(format, "s"),
			other_player->name);
		break;
	case _game_engine_message_multi_kill:
		string_list_index = tag_loaded('ustr', "ui\\multiplayer_game_text");
		if (string_list_index != NONE)
			format = unicode_string_list_get_string(string_list_index, 0x53);
		else
			format = L"";
		ustrncpy(
			buffer,
			format,
			buffer_size);
		game_engine_play_multiplayer_sound(_multiplayer_sound_killtacular_kill);
		break;
	case _game_engine_message_triple_kill:
		string_list_index = tag_loaded('ustr', "ui\\multiplayer_game_text");
		if (string_list_index != NONE)
			format = unicode_string_list_get_string(string_list_index, 0x54);
		else
			format = L"";
		ustrncpy(
			buffer,
			format,
			buffer_size);
		game_engine_play_multiplayer_sound(_multiplayer_sound_triple_kill);
		break;
	case _game_engine_message_double_kill:
		string_list_index = tag_loaded('ustr', "ui\\multiplayer_game_text");
		if (string_list_index != NONE)
			format = unicode_string_list_get_string(string_list_index, 0x55);
		else
			format = L"";
		ustrncpy(
			buffer,
			format,
			buffer_size);
		game_engine_play_multiplayer_sound(_multiplayer_sound_double_kill);
		break;
	case _game_engine_message_ten_kills_in_a_row:
		string_list_index = tag_loaded('ustr', "ui\\multiplayer_game_text");
		if (string_list_index != NONE)
			format = unicode_string_list_get_string(string_list_index, 0x56);
		else
			format = L"";
		ustrncpy(
			buffer,
			format,
			buffer_size);
		game_engine_play_multiplayer_sound(_multiplayer_sound_running_riot);
		break;
	case _game_engine_message_five_kills_in_row:
		string_list_index = tag_loaded('ustr', "ui\\multiplayer_game_text");
		if (string_list_index != NONE)
			format = unicode_string_list_get_string(string_list_index, 0x57);
		else
			format = L"";
		ustrncpy(
			buffer,
			format,
			buffer_size);
		game_engine_play_multiplayer_sound(_multiplayer_sound_killing_spree);
		break;
	case _game_engine_message_killed_enemy:
		other_player = player_get(message_data);
		string_list_index = tag_loaded('ustr', "ui\\multiplayer_game_text");
		if (string_list_index != NONE)
			format = unicode_string_list_get_string(string_list_index, 0x58);
		else
			format = L"";
		usnprintf(
			buffer,
			buffer_size,
			ustring_format_checked(format, "s"),
			other_player->name);
		break;
	case _game_engine_message_multi_kill_with_score:
		string_list_index = tag_loaded('ustr', "ui\\multiplayer_game_text");
		if (string_list_index != NONE)
			format = unicode_string_list_get_string(string_list_index, 0x59);
		else
			format = L"";
		usnprintf(
			buffer,
			buffer_size,
			ustring_format_checked(format, "d"),
			score);
		game_engine_play_multiplayer_sound(_multiplayer_sound_killtacular_kill);
		break;
	case _game_engine_message_triple_kill_with_score:
		string_list_index = tag_loaded('ustr', "ui\\multiplayer_game_text");
		if (string_list_index != NONE)
			format = unicode_string_list_get_string(string_list_index, 0x5A);
		else
			format = L"";
		usnprintf(
			buffer,
			buffer_size,
			ustring_format_checked(format, "d"),
			score);
		game_engine_play_multiplayer_sound(_multiplayer_sound_triple_kill);
		break;
	case _game_engine_message_double_kill_with_score:
		string_list_index = tag_loaded('ustr', "ui\\multiplayer_game_text");
		if (string_list_index != NONE)
			format = unicode_string_list_get_string(string_list_index, 0x5B);
		else
			format = L"";
		usnprintf(
			buffer,
			buffer_size,
			ustring_format_checked(format, "d"),
			score);
		game_engine_play_multiplayer_sound(_multiplayer_sound_double_kill);
		break;
	case _game_engine_message_ten_kills_in_a_row_with_score:
		string_list_index = tag_loaded('ustr', "ui\\multiplayer_game_text");
		if (string_list_index != NONE)
			format = unicode_string_list_get_string(string_list_index, 0x5C);
		else
			format = L"";
		usnprintf(
			buffer,
			buffer_size,
			ustring_format_checked(format, "d"),
			score);
		game_engine_play_multiplayer_sound(_multiplayer_sound_running_riot);
		break;
	case _game_engine_message_five_kills_in_row_with_score:
		string_list_index = tag_loaded('ustr', "ui\\multiplayer_game_text");
		if (string_list_index != NONE)
			format = unicode_string_list_get_string(string_list_index, 0x5D);
		else
			format = L"";
		usnprintf(
			buffer,
			buffer_size,
			ustring_format_checked(format, "d"),
			score);
		game_engine_play_multiplayer_sound(_multiplayer_sound_killing_spree);
		break;
	case _game_engine_message_killed_enemy_with_score:
		other_player = player_get(message_data);
		string_list_index = tag_loaded('ustr', "ui\\multiplayer_game_text");
		if (string_list_index != NONE)
			format = unicode_string_list_get_string(string_list_index, 0x5E);
		else
			format = L"";
		usnprintf(
			buffer,
			buffer_size,
			ustring_format_checked(format, "sd"),
			other_player->name,
			score);
		break;
	case _game_engine_message_odd_man_out:
		string_list_index = tag_loaded('ustr', "ui\\multiplayer_game_text");
		if (string_list_index != NONE)
			format = unicode_string_list_get_string(string_list_index, 0x5F);
		else
			format = L"";
		ustrncpy(
			buffer,
			format,
			buffer_size);
		break;
	case _game_engine_message_out_of_lives:
		string_list_index = tag_loaded('ustr', "ui\\multiplayer_game_text");
		if (string_list_index != NONE)
			format = unicode_string_list_get_string(string_list_index, 0x60);
		else
			format = L"";
		ustrncpy(
			buffer,
			format,
			buffer_size);
		break;
	case _game_engine_message_respawn_timer:
		string_list_index = tag_loaded('ustr', "ui\\multiplayer_game_text");
		if (string_list_index != NONE)
			format = unicode_string_list_get_string(string_list_index, 0x61);
		else
			format = L"";
		usnprintf(
			buffer,
			buffer_size,
			ustring_format_checked(format, "d"),
			message_data);
		break;
	case _game_engine_message_waiting_for_space_to_clear:
		string_list_index = tag_loaded('ustr', "ui\\multiplayer_game_text");
		if (string_list_index != NONE)
			format = unicode_string_list_get_string(string_list_index, 0x62);
		else
			format = L"";
		ustrncpy(
			buffer,
			format,
			buffer_size);
		break;
	case _game_engine_message_player_quit_self:
		string_list_index = tag_loaded('ustr', "ui\\multiplayer_game_text");
		if (string_list_index != NONE)
			format = unicode_string_list_get_string(string_list_index, 0x63);
		else
			format = L"";
		ustrncpy(
			buffer,
			format,
			buffer_size);
		break;
	case _game_engine_message_press_back_for_score:
		string_list_index = tag_loaded('ustr', "ui\\multiplayer_game_text");
		if (string_list_index != NONE)
			format = unicode_string_list_get_string(string_list_index, 0x64);
		else
			format = L"";
		ustrncpy(
			buffer,
			format,
			buffer_size);
		/* port: preserve the identified hint's wording and offer its score key
		 * to the optional keyboard prompt feature before semantic context is lost. */
		buffer[buffer_size - 1] = 0;
		keyboard_score_hint(player_index, buffer, buffer_size);
		break;
	default:
		result = FALSE;
		break;
	}

	buffer[buffer_size - 1] = 0;
	return result;
}

static long adjust_score_for_ranking(
	long score,
	long player_index)
{
	struct player_datum *player;
	long result = 0;

	match_assert(
		"c:\\halo\\SOURCE\\game\\game_engine.c",
		0x29D,
		player_index!=NONE);

	player = player_get(player_index);
	if (score < -1000)
		score = -1000;
	score += 1000;
	if (player->statistics.deaths <
		global_variant.universal_variant.lives)
	{
		result = 0x40000000;
	}
	if (!player->quit_out_of_game)
		result |= 0x20000000;

	return result | score;
}

static long item_collection_get_total(
	struct tag_block const *permutations)
{
	struct item_permutation_definition const *permutation =
		(struct item_permutation_definition const *)permutations->address;
	long permutation_count = permutations->count;
	long result = 0;
	long permutation_index;

	for (permutation_index = 0;
		permutation_index < permutation_count;
		permutation_index++)
	{
		result = (long)(
			(real)result + permutation[permutation_index].weight);
	}

	return result;
}

static long random_item(
	long item_collection_index)
{
	struct item_collection_definition *definition =
		item_collection_definition_get(item_collection_index);
	struct tag_block *permutations = &definition->permutations;
	long permutation_count = permutations->count;
	long remaining_weight = seed_random_range(
		get_global_random_seed_address(),
		0,
		(short)item_collection_get_total(permutations));
	struct item_permutation_definition const *permutation =
		permutations->address;
	long permutation_index = 0;

	while (permutation_index < permutation_count)
	{
		remaining_weight =
			(long)((real)remaining_weight -
				permutation[permutation_index].weight);
		if (remaining_weight < 0)
			return permutation[permutation_index].item.index;

		permutation_index++;
	}

	return NONE;
}


static void game_engine_update_item_spawn(
	void)
{
	struct scenario *scenario = global_scenario_get();
	/* port: a long counter, for a map's long count */
	long equipment_index;

	/* a client of the distributed netcode has the host's items
	(port/linux/game/network_distributed.c) */
	if (network_game_distributed_client())
		return;

	for (equipment_index = 0;
		equipment_index < scenario->netgame_equipment.count;
		equipment_index++)
	{
		struct scenario_netgame_equipment *equipment =
			TAG_BLOCK_GET_ELEMENT(
				&scenario->netgame_equipment,
				equipment_index,
				struct scenario_netgame_equipment);
		if (match_game_type(
			game_engine_get_type(),
			NUMBEROF(equipment->game_type),
			equipment->game_type))
		{
			long respawn_period = 30 * TICKS_PER_SECOND;
			short respawn_time = equipment->spawn_time;

			if (respawn_time == 0)
			{
				if (equipment->item_collection.index != NONE)
				{
					struct item_collection_definition *item_collection =
						item_collection_definition_get(
							equipment->item_collection.index);

					respawn_time = item_collection->spawn_time;
					if (respawn_time != 0)
						respawn_period = respawn_time * TICKS_PER_SECOND;
				}
			}
			else
			{
				respawn_period = respawn_time * TICKS_PER_SECOND;
			}

			if (game_time_get() % respawn_period == 0)
			{
				long definition_index =
					random_item(equipment->item_collection.index);
				struct object_placement_data placement_data;
				long object_index;

				/* port: the gametype's no weapons on the map */
				if (definition_index != NONE && game_variant_options_get()->no_map_weapons &&
					object_definition_get(definition_index)->object.type == _object_type_weapon)
				{
					continue;
				}
				object_placement_data_new(
					&placement_data,
					definition_index,
					NONE);
				placement_data.position = equipment->position;
				if (mcc_cache_tags_loaded())
					vector3d_from_angle(&placement_data.forward, equipment->facing);
				/* port: Halo PC faces a Custom Edition map's items the way
				their placements do, where the Xbox copied only the position */
				if (custom_edition_cache_tags_loaded())
					vector3d_from_angle(&placement_data.forward, equipment->facing);
				object_index = object_new(&placement_data);
				if (object_index != NONE)
				{
					struct item_datum *item = item_get(object_index);

					object_set_garbage(object_index, FALSE);
					if (TEST_FLAG(
						equipment->flags,
						_equipment_created_at_rest_bit))
					{
						SET_FLAG(item->object.flags, _object_at_rest_bit, TRUE);
					}
					item->item.last_owned_time +=
						respawn_period - 30 * TICKS_PER_SECOND;
				}
			}
		}
	}

	return;
}

static void handle_custom_starting_equipment(
	long unit_index,
	long *fragmentation_grenade_count,
	long *plasma_grenade_count)
{
	struct scenario *scenario = global_scenario_get();
	long starting_equipment_index;

	for (starting_equipment_index = 0;
		starting_equipment_index < scenario->scenario_starting_equipment.count;
		starting_equipment_index++)
	{
		struct scenario_starting_equipment *starting_equipment = TAG_BLOCK_GET_ELEMENT(
			&scenario->scenario_starting_equipment,
			starting_equipment_index,
			struct scenario_starting_equipment);

		if (match_game_type(
			game_engine_get_type(),
			4,
			starting_equipment->game_type))
		{
			boolean first_weapon = TRUE;
			long item_collection_index;

			/* January visits the first five of the six item collections */
			for (item_collection_index = 0;
				item_collection_index < 5;
				item_collection_index++)
			{
				if (starting_equipment->item_collection[item_collection_index].index != NONE)
				{
					long definition_index = random_item(
						starting_equipment->item_collection[item_collection_index].index);
					long weapon_index;
					struct object_placement_data placement_data;

					object_placement_data_new(
						&placement_data,
						definition_index,
						NONE);
					weapon_index = object_new(&placement_data);
					if (weapon_index != NONE)
					{
						struct object_datum *weapon = object_get_and_verify_type(
							weapon_index,
							_object_mask_item);

						if (!first_weapon &&
							unit_has_weapon_definition_index(
								unit_index,
								weapon->definition_index))
						{
							object_delete(weapon_index);
						}
						else
						{
							unit_add_weapon_to_inventory(
								unit_index,
								weapon_index,
								first_weapon ? 2 : 0);
							first_weapon = FALSE;
						}
					}
				}
			}

			if (TEST_FLAG(starting_equipment->flags, _netgame_starting_equipment_flag_no_grenades_bit))
			{
				*fragmentation_grenade_count = 0;
				*plasma_grenade_count = 0;
			}

			if (TEST_FLAG(starting_equipment->flags, _netgame_starting_equipment_flag_plasma_greandes_bit))
			{
				*plasma_grenade_count += *fragmentation_grenade_count;
				*fragmentation_grenade_count = 0;
			}

			break;
		}
	}

	return;
}

/* port: a custom loadout's weapon's place in the globals' weapon list
(game_engine_h's _loadout_weapon_*, from the assault rifle); random: one
of those the map has (and port/linux/game/gun_game.c's weapons) */
long game_engine_loadout_weapon_definition(
	byte weapon)
{
	static short const list_indices[] =
	{
		_weapon_list_assault_rifle, _weapon_list_pistol, _weapon_list_shotgun, _weapon_list_sniper_rifle,
		_weapon_list_rocket_launcher, _weapon_list_plasma_pistol, _weapon_list_plasma_rifle, _weapon_list_needler
	};
	short index;

	if (weapon == _loadout_weapon_random)
	{
		long available[NUMBEROF(list_indices)];
		short count = 0;

		for (index = 0; index < NUMBEROF(list_indices); index++)
		{
			long definition_index = list_index_to_weapon_definition_index(list_indices[index]);

			if (definition_index != NONE)
				available[count++] = definition_index;
		}
		return count ? available[local_random_range(0, count)] : NONE;
	}
	index = (short)(weapon - _loadout_weapon_assault_rifle);
	return index >= 0 && index < NUMBEROF(list_indices) ? list_index_to_weapon_definition_index(list_indices[index]) :
		NONE;
}

/* port: the unit's weapons the loadout's (its others gone): the primary,
then the secondary */
static void game_engine_give_loadout(
	long unit_index)
{
	struct game_variant_options const *options = game_variant_options_get();
	byte const weapons[2] = { options->primary_weapon, options->secondary_weapon };
	boolean first = TRUE;
	short index;

	unit_delete_all_weapons(unit_index);
	for (index = 0; index < 2; index++)
	{
		long definition_index = game_engine_loadout_weapon_definition(weapons[index]);
		struct object_placement_data placement_data;
		long weapon_index;

		if (definition_index == NONE)
			continue;
		object_placement_data_new(&placement_data, definition_index, NONE);
		weapon_index = object_new(&placement_data);
		if (weapon_index == NONE)
			continue;
		if (!first && unit_has_weapon_definition_index(unit_index, definition_index))
			object_delete(weapon_index);
		else if (unit_add_weapon_to_inventory(unit_index, weapon_index, first ? 2 : 0))
			first = FALSE;
		else
			object_delete(weapon_index);
	}
}

void game_engine_postspawn_player_update(
	long player_index)
{
	struct player_datum *player;
	long unit_index;
	struct game_globals_grenade *fragmentation_grenade;
	struct game_globals_grenade *plasma_grenade;
	long fragmentation_grenade_count;
	long plasma_grenade_count;

	if (!game_engine)
		return;

	if (game_engine->player_update)
	{
		game_engine->player_update(player_index);
		return;
	}

	player = player_get(player_index);
	unit_index = player->unit_index;
	fragmentation_grenade = TAG_BLOCK_GET_ELEMENT(
		&scenario_get_game_globals()->grenades,
		_unit_grenade_human_fragmentation,
		struct game_globals_grenade);
	plasma_grenade = TAG_BLOCK_GET_ELEMENT(
		&scenario_get_game_globals()->grenades,
		_unit_grenade_covenant_plasma,
		struct game_globals_grenade);
	plasma_grenade_count = plasma_grenade->maximum_count;
	fragmentation_grenade_count = fragmentation_grenade->maximum_count;

	if (TEST_FLAG(game_engine_globals.flags, _game_engine_9_or_more_players_bit))
	{
		fragmentation_grenade_count = 1;
		plasma_grenade_count = 1;
	}
	else if (TEST_FLAG(game_engine_globals.flags, _game_engine_5_or_more_players_bit))
	{
		fragmentation_grenade_count = 2;
		plasma_grenade_count = 2;
	}

	{
		long starting_fragmentation_grenade_count =
			fragmentation_grenade_count;
		long starting_plasma_grenade_count = 0;

		/* port: a custom loadout's weapons (game_variant_options) */
		if (game_variant_options_get()->loadout == _loadout_custom)
		{
			game_engine_give_loadout(unit_index);
		}
		else if (!TEST_FLAG(global_variant.universal_variant.flags, _game_variant_generic_starting_equipment_bit))
		{
			handle_custom_starting_equipment(
				unit_index,
				&starting_fragmentation_grenade_count,
				&starting_plasma_grenade_count);
		}

		if (game_engine_infinite_grenades_internal())
		{
			starting_plasma_grenade_count =
				plasma_grenade_count;
			starting_fragmentation_grenade_count =
				fragmentation_grenade_count;
		}

		if (unit_index != NONE)
		{
			struct unit_datum *unit = object_get_and_verify_type(
				unit_index,
				_object_mask_unit);

			switch (global_variant.universal_variant.weapon_set)
			{
			case _game_engine_weapons_plasma_weapons:
				starting_plasma_grenade_count +=
					starting_fragmentation_grenade_count;
				starting_fragmentation_grenade_count = 0;
				break;
			case _game_engine_weapons_human:
				starting_fragmentation_grenade_count +=
					starting_plasma_grenade_count;
				starting_plasma_grenade_count = 0;
				break;
			case _game_engine_weapons_no_grenades:
				if (!game_engine_infinite_grenades_internal())
				{
					starting_fragmentation_grenade_count = 0;
					starting_plasma_grenade_count = 0;
				}
				break;
			}

			starting_fragmentation_grenade_count = MIN(
				starting_fragmentation_grenade_count,
				fragmentation_grenade_count);
			starting_plasma_grenade_count = MIN(
				starting_plasma_grenade_count,
				plasma_grenade_count);
			unit->unit.grenade_counts[
				_unit_grenade_human_fragmentation] =
				(char)starting_fragmentation_grenade_count;
			unit->unit.grenade_counts[
				_unit_grenade_covenant_plasma] =
				(char)starting_plasma_grenade_count;
			/* port: and MCC's expanded grenade types (mcc_player.c) */
			mcc_player_postspawn_grenades(unit_index,
				global_variant.universal_variant.weapon_set == _game_engine_weapons_no_grenades &&
				!game_engine_infinite_grenades_internal());
		}
	}
	/* port: an OpenCE gametype's mode's equipment (Gun Game's weapon,
	Infection's skull: port/linux/game/game_modes.c) */
	game_modes_postspawn(player_index);

	return;
}




boolean game_engine_get_state_message(
	long player_index,
	wchar_t *message,
	long message_character_count)
{
	struct player_datum *player;
	long state_message_parameter;
	long state_message;
	long respawn_timer;
	boolean result = FALSE;

	if (game_engine)
	{
		player = player_get(player_index);

		if (player->state_message >= _game_engine_message_odd_man_out &&
			player->state_message <=
				_game_engine_message_waiting_for_space_to_clear)
		{
			player->state_message = NONE;
		}

		if (player->unit_index == NONE)
		{
			state_message_parameter = 0;
			if (player->quit_out_of_game == TRUE)
				state_message = _game_engine_message_player_quit_self;
			else if (game_engine_player_is_out_of_lives(player_index))
				state_message = _game_engine_message_out_of_lives;
			else if (game_engine_player_is_odd_man_out(player_index))
				state_message = _game_engine_message_odd_man_out;
			else
			{
				respawn_timer = player->respawn_timer;
				if (respawn_timer > 0)
				{
					state_message = _game_engine_message_respawn_timer;
					state_message_parameter = respawn_timer / TICKS_PER_SECOND;
				}
				else
					state_message =
						_game_engine_message_waiting_for_space_to_clear;
			}

			result = multiplayer_message_internal(
				player_index,
				state_message,
				state_message_parameter,
				message,
				message_character_count);
		}
		else if (game_time_get() < 15 * TICKS_PER_SECOND)
		{
			result = multiplayer_message_internal(
				player_index,
				_game_engine_message_press_back_for_score,
				NONE,
				message,
				message_character_count);
		}
		else if (player->state_message != NONE)
		{
			result = multiplayer_message_internal(
				player_index,
				player->state_message,
				player->state_message_player_index,
				message,
				message_character_count);
		}
	}

	return result;
}

/* port: a distributed client's players spawn when the host spawns them
(network_player_attach_unit), but their respawn countdown runs here as it
does on the host (game_engine_should_spawn_player), for the hud's count and
its sounds: it stops a tick short, where the host's unit ends it */
void game_engine_client_respawn_countdown(
	long player_index)
{
	struct player_datum *player;

	if (!game_engine)
		return;
	player = player_get(player_index);
	if (player->quit_out_of_game == TRUE ||
		player->statistics.deaths == 0 ||
		player->respawn_timer <= 1 ||
		game_engine_player_is_out_of_lives(player_index) ||
		game_engine_player_is_odd_man_out(player_index) ||
		game_engine_globals.postgame_state == game_engine_mode_postgame_rasterize ||
		game_engine_globals.postgame_state == game_engine_mode_postgame_rasterize_delay)
	{
		return;
	}
	if (player->local_player_index != NONE &&
		(player->respawn_timer == 90 || player->respawn_timer == 60 || player->respawn_timer == 30))
	{
		game_engine_play_multiplayer_sound(_multiplayer_sound_countdown_for_respawn);
	}
	player->respawn_timer--;
	if (player->local_player_index != NONE && player->respawn_timer == 1)
		game_engine_play_multiplayer_sound(_multiplayer_sound_respawn);

	return;
}

boolean game_engine_should_spawn_player(
	long player_index)
{
	boolean should_spawn = FALSE;

	if (game_engine)
	{
		struct player_datum *player = player_get(player_index);

		if (player->quit_out_of_game == TRUE)
		{
			should_spawn = FALSE;
		}
		else if (player->statistics.deaths == 0)
		{
			should_spawn = TRUE;
		}
		else if (game_engine_player_is_out_of_lives(player_index))
		{
			should_spawn = FALSE;
		}
		else if (game_engine_player_is_odd_man_out(player_index))
		{
			should_spawn = FALSE;
		}
		else if (game_engine_globals.postgame_state ==
				game_engine_mode_postgame_rasterize ||
			game_engine_globals.postgame_state ==
				game_engine_mode_postgame_rasterize_delay)
		{
			should_spawn = FALSE;
		}
		else
		{
			long respawn_timer = player->respawn_timer;

			if (respawn_timer > 0)
			{
				if (player->local_player_index != NONE)
				{
					if (respawn_timer == 90)
					{
						game_engine_play_multiplayer_sound(
							_multiplayer_sound_countdown_for_respawn);
					}
					else if (respawn_timer == 60)
					{
						game_engine_play_multiplayer_sound(
							_multiplayer_sound_countdown_for_respawn);
					}
					else if (respawn_timer == 30)
					{
						game_engine_play_multiplayer_sound(
							_multiplayer_sound_countdown_for_respawn);
					}
					else if (respawn_timer == 1)
					{
						game_engine_play_multiplayer_sound(
							_multiplayer_sound_respawn);
					}
				}

				player->respawn_timer--;
				should_spawn = player->respawn_timer == 0;
			}
			else
			{
				should_spawn = TRUE;
			}
		}

		if (should_spawn &&
			game_time_get() > 3 &&
			game_time_get() % 32 !=
				DATUM_INDEX_TO_ABSOLUTE_INDEX(player_index) % 32)
		{
			should_spawn = FALSE;
		}
	}

	return should_spawn;
}

struct game_engine_place game_engine_get_place(
	long player_index,
	enum get_score_type score_type)
{
	struct player_datum *player;
	long group_count;
	boolean tied;
	boolean all_tied;
	struct game_engine_place result;

	player = player_get(player_index);
	all_tied = TRUE;
	tied = FALSE;
	group_count = 1;
	result.place = 0;

	if (game_engine->get_player_score)
	{
		struct data_iterator iterator;
		struct player_datum *other_player;
		/* port: free-for-all team indices run up to the player limit */
		unsigned long team_mask[BIT_VECTOR_SIZE_IN_LONGS(MULTIPLAYER_MAXIMUM_PLAYERS)] = { 0 };
		long score = game_engine->get_player_score(player_index, score_type);

		data_iterator_new(&iterator, player_data);
		other_player = (struct player_datum *)data_iterator_next(&iterator);
		while (other_player)
		{
			boolean different_player;

			if (score_type == _get_score_team)
			{
				different_player =
					other_player->team_index != player->team_index;
			}
			else
				different_player = iterator.datum_index != player_index;

			if (different_player &&
				score_type == _get_score_team)
			{
				/* a team outside the mask (none yet) counts on its own */
				if (VALID_INDEX(other_player->team_index, MULTIPLAYER_MAXIMUM_PLAYERS))
				{
					if (BIT_VECTOR_TEST_FLAG(team_mask, other_player->team_index))
						different_player = FALSE;
					else
						BIT_VECTOR_SET_FLAG(team_mask, other_player->team_index, TRUE);
				}
			}

			if (different_player)
			{
				long other_score = game_engine->get_player_score(
					iterator.datum_index,
					score_type);

				group_count++;
				if (other_score != score)
					all_tied = FALSE;

				if (other_score > score)
					result.place++;
				else if (other_score == score)
					tied = TRUE;
			}

			other_player =
				(struct player_datum *)data_iterator_next(&iterator);
		}

		match_vassert(
			"c:\\halo\\SOURCE\\game\\game_engine.c",
			0x1377,
			!all_tied || tied || group_count == 1,
			"(!all_tied || (tied)) || (1 == group_count)");
	}

	all_tied &= tied;
	result.flags = score_type != _get_score_team
		? 0
		: FLAG(_place_team);
	SET_FLAG(result.flags, _place_tied, tied);
	SET_FLAG(
		result.flags,
		_place_all_tied,
		all_tied);
	SET_FLAG(
		result.flags,
		_place_two_groups,
		group_count == 2);

	return result;
}

static void netgame_verify_spawn_points(
	short game_type,
	short unused_team_index,
	short minimum_count,
	char const *error_message)
{
	short starting_location_count;
	long matching_count;
	short starting_location_index;

	starting_location_count = player_get_starting_location_count();
	matching_count = 0;
	for (starting_location_index = 0;
		starting_location_index < starting_location_count;
		starting_location_index++)
	{
		struct player_starting_location *starting_location =
			player_get_starting_location(starting_location_index);

		if (match_game_type(
			game_type,
			4,
			starting_location->game_types))
		{
			matching_count++;
		}
	}

	if (matching_count < minimum_count)
	{
		error(
			_error_silent,
			error_message,
			matching_count,
			minimum_count);
	}

	return;
}

long game_engine_slayer_write_network_state(byte *buffer, long size);
boolean game_engine_slayer_read_network_state(byte const *buffer, long size, boolean first);
long game_engine_ctf_write_network_state(byte *buffer, long size);
boolean game_engine_ctf_read_network_state(byte const *buffer, long size, boolean first);
long game_engine_oddball_write_network_state(byte *buffer, long size);
boolean game_engine_oddball_read_network_state(byte const *buffer, long size, boolean first);
long game_engine_king_write_network_state(byte *buffer, long size);
boolean game_engine_king_read_network_state(byte const *buffer, long size, boolean first);
long game_engine_race_write_network_state(byte *buffer, long size);
boolean game_engine_race_read_network_state(byte const *buffer, long size, boolean first);

/* the distributed netcode (port/linux/game/network_damage.c): a player's
score as the game type has it (with its killing blows' messages), 0 for
none */
long game_engine_network_player_score(
	long player_index)
{
	if (!game_engine || !game_engine->get_player_score)
		return 0;
	return game_engine->get_player_score(player_index, TRUE);
}

/* the distributed netcode (port/linux/game/network_distributed.c): the
current game type's state (scores, and what else every machine must agree
on), which the host sends its clients; the size written, 0 for none */
long game_engine_write_network_state(
	byte *buffer,
	long size)
{
	long postgame_state;
	long written;

	if (!game_engine || size < (long)sizeof(postgame_state))
		return 0;
	/* whether the game is over, then the game type's */
	postgame_state = game_engine_globals.postgame_state;
	csmemcpy(buffer, &postgame_state, sizeof(postgame_state));
	buffer += sizeof(postgame_state);
	size -= sizeof(postgame_state);
	switch (game_engine_get_type())
	{
	case game_engine_ctf: written = game_engine_ctf_write_network_state(buffer, size); break;
	/* port: (Infection's own state: infection.c) */
	case game_engine_slayer: written = infection_enabled() ? infection_write_network_state(buffer, size) :
		game_engine_slayer_write_network_state(buffer, size); break;
	case game_engine_oddball: written = game_engine_oddball_write_network_state(buffer, size); break;
	case game_engine_king: written = game_engine_king_write_network_state(buffer, size); break;
	case game_engine_race: written = game_engine_race_write_network_state(buffer, size); break;
	default: written = 0; break;
	}
	return sizeof(postgame_state) + written;
}

/* a client: the host's */
void game_engine_read_network_state(
	byte const *buffer,
	long size)
{
	long postgame_state;
	boolean first;
	boolean read;

	if (!game_engine || size < (long)sizeof(postgame_state))
		return;
	csmemcpy(&postgame_state, buffer, sizeof(postgame_state));
	buffer += sizeof(postgame_state);
	size -= sizeof(postgame_state);
	first = !game_engine_network_state_read;
	switch (game_engine_get_type())
	{
	case game_engine_ctf: read = game_engine_ctf_read_network_state(buffer, size, first); break;
	case game_engine_slayer: read = infection_enabled() ? infection_read_network_state(buffer, size, first) :
		game_engine_slayer_read_network_state(buffer, size, first); break;
	case game_engine_oddball: read = game_engine_oddball_read_network_state(buffer, size, first); break;
	case game_engine_king: read = game_engine_king_read_network_state(buffer, size, first); break;
	case game_engine_race: read = game_engine_race_read_network_state(buffer, size, first); break;
	default: read = TRUE; break;
	}
	/* (a state the game type refused is not had: the first it takes is,
	whose events it only takes) */
	if (!read)
		return;
	game_engine_network_state_read = TRUE;
	/* the game ended on the host (after what ended it is shown, as the host
	shows it) */
	if (postgame_state != 0 && game_engine_globals.postgame_state == 0)
		game_engine_end_game();
}
