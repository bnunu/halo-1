/*
UI_WIDGET.C

symbols in this file:
000D32C0 0020:
	_code_000d32c0 (0000)
000D32E0 0020:
	_code_000d32e0 (0000)
000D3300 0030:
	_event_controller_index_compatible_with_widget (0000)
000D3330 0030:
	_set_ui_plasma_effect_color (0000)
000D3360 00a0:
	_ui_widgets_initialize (0000)
000D3400 0010:
	_ui_widgets_safe_to_load (0000)
000D3410 0010:
	_ui_widgets_set_fade_value (0000)
000D3420 0010:
	_ui_widget_debug_show_path (0000)
000D3430 0020:
	_widget_instance_count_children (0000)
000D3450 0050:
	_widget_instance_get_nth_child (0000)
000D34A0 0030:
	_ui_widget_realloc (0000)
000D34D0 0020:
	_widget_free (0000)
000D34F0 0030:
	_ui_widgets_active (0000)
000D3520 0070:
	_ui_widgets_active_for_local_player (0000)
000D3590 0050:
	_ui_widgets_inhibit_processing (0000)
000D35E0 0020:
	_code_000d35e0 (0000)
000D3600 0490:
	_draw_bitmap_in_rect (0000)
000D3A90 0020:
	_widget_instance_get_topmost_parent (0000)
000D3AB0 0040:
	_widget_instance_get_child_index_from_parent (0000)
000D3AF0 0060:
	_widget_instance_set_visibility_recursive (0000)
000D3B50 0010:
	_main_menu_active (0000)
000D3B60 0010:
	_main_menu_is_active (0000)
000D3B70 0030:
	_main_menu_screen_is_active (0000)
000D3BA0 0050:
	_ui_set_next_level (0000)
000D3BF0 0010:
	_ui_widget_load_progress_widget (0000)
000D3C00 0010:
	_filesystem_check_thread_is_active (0000)
000D3C10 0040:
	_code_000d3c10 (0000)
000D3C50 0030:
	_display_error_when_main_menu_loaded (0000)
000D3C80 0090:
	_display_error_deferred (0000)
000D3D10 0040:
	_display_error_abort_to_dashboard_deferred (0000)
000D3D50 0070:
	_ui_start_main_menu_music (0000)
000D3DC0 0060:
	_ui_stop_main_menu_music (0000)
000D3E20 0010:
	_ui_main_menu_music_active (0000)
000D3E30 0040:
	_ui_widgets_disable_pause_game (0000)
000D3E70 0080:
	_code_000d3e70 (0000)
000D3EF0 0060:
	_code_000d3ef0 (0000)
000D3F50 0030:
	_code_000d3f50 (0000)
000D3F80 0090:
	_code_000d3f80 (0000)
000D4010 0080:
	_code_000d4010 (0000)
000D4090 0050:
	_code_000d4090 (0000)
000D40E0 0020:
	_code_000d40e0 (0000)
000D4100 0080:
	_code_000d4100 (0000)
000D4180 0040:
	_code_000d4180 (0000)
000D41C0 0040:
	_code_000d41c0 (0000)
000D4200 0050:
	_code_000d4200 (0000)
000D4250 01a0:
	_code_000d4250 (0000)
000D43F0 0070:
	_code_000d43f0 (0000)
000D4460 0060:
	_code_000d4460 (0000)
000D44C0 0060:
	_code_000d44c0 (0000)
000D4520 0130:
	_remap_sticks_for_local_player (0000)
000D4650 0030:
	_code_000d4650 (0000)
000D4680 0010:
	_code_000d4680 (0000)
000D4690 0010:
	_code_000d4690 (0000)
000D46A0 0170:
	_code_000d46a0 (0000)
000D4810 00d0:
	_code_000d4810 (0000)
000D48E0 0020:
	_code_000d48e0 (0000)
000D4900 01d0:
	_code_000d4900 (0000)
000D4AD0 0030:
	_code_000d4ad0 (0000)
000D4B00 0040:
	_code_000d4b00 (0000)
000D4B40 0020:
	_code_000d4b40 (0000)
000D4B60 0060:
	_code_000d4b60 (0000)
000D4BC0 00a0:
	_code_000d4bc0 (0000)
000D4C60 0050:
	_get_ui_rgb_white (0000)
000D4CB0 0060:
	_get_ui_argb_white (0000)
000D4D10 0050:
	_code_000d4d10 (0000)
000D4D60 0040:
	_modulate_pixel32_by_real_alpha (0000)
000D4DA0 02a0:
	_ui_widget_delete (0000)
000D5040 0050:
	_ui_widgets_close_all (0000)
000D5090 0090:
	_ui_widgets_close_all_for_local_player (0000)
000D5120 0040:
	_ui_widgets_delete_history (0000)
000D5160 0060:
	_ui_widgets_pop_stack (0000)
000D51C0 0070:
	_main_screen_shell_begin_fade (0000)
000D5230 0060:
	_ui_play_audio_feedback_sound (0000)
000D5290 02a0:
	_code_000d5290 (0000)
000D5530 0030:
	_code_000d5530 (0000)
000D5560 0360:
	_draw_string_and_hack_in_icons (0000)
000D58C0 0310:
	_code_000d58c0 (0000)
000D5BD0 0440:
	_code_000d5bd0 (0000)
000D6010 0050:
	_code_000d6010 (0000)
000D6060 00a0:
	_code_000d6060 (0000)
000D6100 00e0:
	_code_000d6100 (0000)
000D61E0 0050:
	_ui_widgets_dispose (0000)
000D6230 0200:
	_widget_event_function_list_widget_goto_next_item (0000)
000D6430 0220:
	_widget_event_function_list_widget_goto_previous_item (0000)
000D6650 04f0:
	_code_000d6650 (0000)
000D6B40 02f0:
	_code_000d6b40 (0000)
000D6E30 00b0:
	_code_000d6e30 (0000)
000D6EE0 0180:
	_render_ui_widgets_postgame (0000)
000D7060 0230:
	_render_ui_widgets (0000)
000D7290 01f0:
	_code_000d7290 (0000)
000D7480 07e0:
	_code_000d7480 (0000)
000D7C60 0220:
	_ui_widget_load_by_name_or_tag (0000)
000D7E80 0130:
	_main_screen_shell_load (0000)
000D7FB0 00e0:
	_network_game_reset_to_pregame_ui (0000)
000D8090 0380:
	_display_error (0000)
000D8410 0090:
	_display_error_abort_to_dashboard (0000)
000D84A0 0090:
	_display_error_damaged_media (0000)
000D8530 0070:
	_display_errors_deferred_until_cinematic_stop (0000)
000D85A0 0260:
	_display_scenario_help (0000)
000D8800 02a0:
	_code_000d8800 (0000)
000D8AA0 0130:
	_code_000d8aa0 (0000)
000D8BD0 03c0:
	_process_ui_widgets (0000)
00271380 0012:
	??_C@_1BC@GEHLELKL@?$AAc?$AAu?$AAs?$AAt?$AAo?$AAm?$AA?9?$AA8?$AA?$AA@ (0000)
00271394 0012:
	??_C@_1BC@DMBIFLPN@?$AAc?$AAu?$AAs?$AAt?$AAo?$AAm?$AA?9?$AA7?$AA?$AA@ (0000)
002713A8 0012:
	??_C@_1BC@IEKEDMJI@?$AAc?$AAu?$AAs?$AAt?$AAo?$AAm?$AA?9?$AA6?$AA?$AA@ (0000)
002713BC 0012:
	??_C@_1BC@JGBBJDHG@?$AAc?$AAu?$AAs?$AAt?$AAo?$AAm?$AA?9?$AA5?$AA?$AA@ (0000)
002713D0 0012:
	??_C@_1BC@COKNPEBD@?$AAc?$AAu?$AAs?$AAt?$AAo?$AAm?$AA?9?$AA4?$AA?$AA@ (0000)
002713E4 0012:
	??_C@_1BC@LDHKMMKK@?$AAc?$AAu?$AAs?$AAt?$AAo?$AAm?$AA?9?$AA3?$AA?$AA@ (0000)
002713F8 0012:
	??_C@_1BC@LMGKLMP@?$AAc?$AAu?$AAs?$AAt?$AAo?$AAm?$AA?9?$AA2?$AA?$AA@ (0000)
0027140C 0012:
	??_C@_1BC@BJHDAECB@?$AAc?$AAu?$AAs?$AAt?$AAo?$AAm?$AA?9?$AA1?$AA?$AA@ (0000)
00271420 000a:
	??_C@_19MLOAAMON@?$AAl?$AAo?$AAo?$AAk?$AA?$AA@ (0000)
0027142C 000a:
	??_C@_19OACEEDLD@?$AAm?$AAo?$AAv?$AAe?$AA?$AA@ (0000)
00271438 000a:
	??_C@_19LHGBOOBH@?$AAb?$AAa?$AAc?$AAk?$AA?$AA@ (0000)
00271444 000e:
	??_C@_1O@CNPOHNDJ@?$AAa?$AAc?$AAc?$AAe?$AAp?$AAt?$AA?$AA@ (0000)
00271454 000a:
	??_C@_19MEDLKCJO@?$AAz?$AAo?$AAo?$AAm?$AA?$AA@ (0000)
00271460 000e:
	??_C@_1O@KCCHFBLA@?$AAc?$AAr?$AAo?$AAu?$AAc?$AAh?$AA?$AA@ (0000)
00271470 0020:
	??_C@_1CA@KFDCIJKK@?$AAr?$AAo?$AAt?$AAa?$AAt?$AAe?$AA?9?$AAg?$AAr?$AAe?$AAn?$AAa?$AAd?$AAe?$AAs?$AA?$AA@ (0000)
00271490 001e:
	??_C@_1BO@POELNCJG@?$AAr?$AAo?$AAt?$AAa?$AAt?$AAe?$AA?9?$AAw?$AAe?$AAa?$AAp?$AAo?$AAn?$AAs?$AA?$AA@ (0000)
002714B0 001c:
	??_C@_1BM@IKPPIMCO@?$AAu?$AAs?$AAe?$AA?9?$AAe?$AAq?$AAu?$AAi?$AAp?$AAm?$AAe?$AAn?$AAt?$AA?$AA@ (0000)
002714CC 000a:
	??_C@_19OBFFCANM@?$AAj?$AAu?$AAm?$AAp?$AA?$AA@ (0000)
002714D8 0022:
	??_C@_1CC@OGCINJG@?$AAi?$AAn?$AAt?$AAe?$AAg?$AAr?$AAa?$AAt?$AAe?$AAd?$AA?9?$AAl?$AAi?$AAg?$AAh?$AAt?$AA?$AA@ (0000)
002714FC 0020:
	??_C@_1CA@BEHOBGA@?$AAp?$AAr?$AAi?$AAm?$AAa?$AAr?$AAy?$AA?9?$AAt?$AAr?$AAi?$AAg?$AAg?$AAe?$AAr?$AA?$AA@ (0000)
0027151C 001c:
	??_C@_1BM@HECBOACM@?$AAt?$AAh?$AAr?$AAo?$AAw?$AA?9?$AAg?$AAr?$AAe?$AAn?$AAa?$AAd?$AAe?$AA?$AA@ (0000)
00271538 000e:
	??_C@_1O@HCAHPFFC@?$AAa?$AAc?$AAt?$AAi?$AAo?$AAn?$AA?$AA@ (0000)
00271548 0018:
	??_C@_1BI@IAFIHGKJ@?$AAr?$AAi?$AAg?$AAh?$AAt?$AA?9?$AAs?$AAt?$AAi?$AAc?$AAk?$AA?$AA@ (0000)
00271560 0016:
	??_C@_1BG@GNIACJFB@?$AAl?$AAe?$AAf?$AAt?$AA?9?$AAs?$AAt?$AAi?$AAc?$AAk?$AA?$AA@ (0000)
00271578 0018:
	??_C@_1BI@KFEEPENO@?$AAr?$AAi?$AAg?$AAh?$AAt?$AA?9?$AAt?$AAh?$AAu?$AAm?$AAb?$AA?$AA@ (0000)
00271590 0016:
	??_C@_1BG@EIJMKLCG@?$AAl?$AAe?$AAf?$AAt?$AA?9?$AAt?$AAh?$AAu?$AAm?$AAb?$AA?$AA@ (0000)
002715A8 0018:
	??_C@_1BI@KIILCNEB@?$AAb?$AAa?$AAc?$AAk?$AA?9?$AAb?$AAu?$AAt?$AAt?$AAo?$AAn?$AA?$AA@ (0000)
002715C0 001a:
	??_C@_1BK@CFIHHNDP@?$AAs?$AAt?$AAa?$AAr?$AAt?$AA?9?$AAb?$AAu?$AAt?$AAt?$AAo?$AAn?$AA?$AA@ (0000)
002715DC 0016:
	??_C@_1BG@CGEEKDJO@?$AAd?$AAp?$AAa?$AAd?$AA?9?$AAr?$AAi?$AAg?$AAh?$AAt?$AA?$AA@ (0000)
002715F4 0014:
	??_C@_1BE@LFENLMOB@?$AAd?$AAp?$AAa?$AAd?$AA?9?$AAl?$AAe?$AAf?$AAt?$AA?$AA@ (0000)
00271608 0014:
	??_C@_1BE@MAHEIGBI@?$AAd?$AAp?$AAa?$AAd?$AA?9?$AAd?$AAo?$AAw?$AAn?$AA?$AA@ (0000)
0027161C 0010:
	??_C@_1BA@CNADAJOL@?$AAd?$AAp?$AAa?$AAd?$AA?9?$AAu?$AAp?$AA?$AA@ (0000)
0027162C 001c:
	??_C@_1BM@DNLFOGBJ@?$AAr?$AAi?$AAg?$AAh?$AAt?$AA?9?$AAt?$AAr?$AAi?$AAg?$AAg?$AAe?$AAr?$AA?$AA@ (0000)
00271648 001a:
	??_C@_1BK@IONNGAAO@?$AAl?$AAe?$AAf?$AAt?$AA?9?$AAt?$AAr?$AAi?$AAg?$AAg?$AAe?$AAr?$AA?$AA@ (0000)
00271664 001a:
	??_C@_1BK@DHOCGHBB@?$AAw?$AAh?$AAi?$AAt?$AAe?$AA?9?$AAb?$AAu?$AAt?$AAt?$AAo?$AAn?$AA?$AA@ (0000)
00271680 001a:
	??_C@_1BK@JINDKLEA@?$AAb?$AAl?$AAa?$AAc?$AAk?$AA?9?$AAb?$AAu?$AAt?$AAt?$AAo?$AAn?$AA?$AA@ (0000)
0027169C 0012:
	??_C@_1BC@JDEDLPGE@?$AAy?$AA?9?$AAb?$AAu?$AAt?$AAt?$AAo?$AAn?$AA?$AA@ (0000)
002716B0 0012:
	??_C@_1BC@HMBBAJIF@?$AAx?$AA?9?$AAb?$AAu?$AAt?$AAt?$AAo?$AAn?$AA?$AA@ (0000)
002716C4 0012:
	??_C@_1BC@EADHJABC@?$AAb?$AA?9?$AAb?$AAu?$AAt?$AAt?$AAo?$AAn?$AA?$AA@ (0000)
002716D8 0012:
	??_C@_1BC@KKLBENHA@?$AAa?$AA?9?$AAb?$AAu?$AAt?$AAt?$AAo?$AAn?$AA?$AA@ (0000)
002716EC 000f:
	??_C@_0P@OHNLNAJA@levels?2d40?2d40?$AA@ (0000)
002716FC 000f:
	??_C@_0P@IAIGJJBI@levels?2d20?2d20?$AA@ (0000)
0027170C 000f:
	??_C@_0P@HAMJOBDA@levels?2c40?2c40?$AA@ (0000)
0027171C 000f:
	??_C@_0P@BHJEKILI@levels?2c20?2c20?$AA@ (0000)
0027172C 000f:
	??_C@_0P@CEDKAMHM@levels?2c10?2c10?$AA@ (0000)
0027173C 000f:
	??_C@_0P@ENPIGML@levels?2b40?2b40?$AA@ (0000)
0027174C 000f:
	??_C@_0P@MEDHKOMA@levels?2b30?2b30?$AA@ (0000)
0027175C 000f:
	??_C@_0P@DPFAEPEF@levels?2a50?2a50?$AA@ (0000)
0027176C 000f:
	??_C@_0P@FIANAGMN@levels?2a30?2a30?$AA@ (0000)
0027177C 000f:
	??_C@_0P@MMBGMDIK@levels?2a10?2a10?$AA@ (0000)
0027178C 0013:
	??_C@_0BD@CKKOGMCD@widget_memory_pool?$AA@ (0000)
002717A0 0008:
	__one_over_255 (0000)
	_SECONDS_PER_MILLISECOND (0004)
002717A8 0025:
	??_C@_0CF@BFKKDAFO@c?3?2halo?2SOURCE?2interface?2ui_widg@ (0000)
002717D0 0007:
	??_C@_06KLKLDLLI@widget?$AA@ (0000)
002717D8 0024:
	??_C@_0CE@JONHIOKA@expected?5a?5valid?5local_player_in@ (0000)
002717FC 001b:
	??_C@_0BL@IGOIGKAA@widget_globals?4initialized?$AA@ (0000)
00271818 0004:
	__real@38026b7e (0000)
0027181C 0004:
	__real@43490000 (0000)
00271820 0004:
	__real@38150d23 (0000)
00271824 0004:
	__real@37e0c670 (0000)
00271828 0004:
	__real@439b8000 (0000)
0027182C 0004:
	__real@3806dd76 (0000)
00271830 000e:
	??_C@_0O@CJGPHCNG@the_main_menu?$AA@ (0000)
00271840 000e:
	??_C@_0O@LGOJGHAO@unknown?5level?$AA@ (0000)
00271850 0048:
	??_C@_0EI@DFLNAEDH@the?5old?5loading?5progress?5screen?5@ (0000)
00271898 0059:
	??_C@_0FJ@HLIPNJGE@there?5is?5already?5an?5error?5messag@ (0000)
002718F8 0051:
	??_C@_0FB@JEKLNNGC@there?5is?5already?5a?5deferred?5erro@ (0000)
0027194C 0036:
	??_C@_0DG@OLDDPPAF@?$CIindex?$DO?$DN0?$CJ?5?$CG?$CG?5?$CIindex?$DMMAXIMUM_NUM@ (0000)
00271988 0047:
	??_C@_0EH@LLPKBLA@there?5is?5already?5a?5deferred?5dash@ (0000)
002719D0 001a:
	??_C@_0BK@LDNBEGIO@title?5music?5tag?5not?5found?$AA@ (0000)
002719EC 0019:
	??_C@_0BJ@DOPIPCPC@starting?5main?5menu?5music?$AA@ (0000)
00271A08 001a:
	??_C@_0BK@GFHNHPPO@sound?2music?2title1?2title1?$AA@ (0000)
00271A24 0019:
	??_C@_0BJ@OPCCAOJF@stopping?5main?5menu?5music?$AA@ (0000)
00271A40 0012:
	??_C@_0BC@MKLFKPBF@duration_ticks?$DO?$DN0?$AA@ (0000)
00271A54 0034:
	??_C@_0DE@HOKJOLHA@out?5of?5memory?$CB?5the?5UI?5screen?5his@ (0000)
00271A88 000c:
	??_C@_0M@EABMFPBO@top?5?$CG?$CG?5data?$AA@ (0000)
00271A94 0019:
	??_C@_0BJ@FOOJDKHD@tail_child?9?$DOnext?5?$DN?$DN?5NULL?$AA@ (0000)
00271AB0 0033:
	??_C@_0DD@MMOLGAOL@?$CIchild?9?$DOprevious?5?$DN?$DN?5NULL?$CJ?5?$CG?$CG?5?$CIch@ (0000)
00271AE4 0018:
	??_C@_0BI@LBDKLIFA@next?9?$DOprevious?5?$DN?$DN?5child?$AA@ (0000)
00271AFC 0018:
	??_C@_0BI@LBKIDJB@child?9?$DOprevious?5?$DN?$DN?5NULL?$AA@ (0000)
00271B18 0041:
	??_C@_0EB@BEJBEEHM@initial_indent?$DM0?5in?5render_state@ (0000)
00271B5C 001d:
	??_C@_0BN@EEMEPEOK@31?5?$DN?$DN?5get_icon_type?$CIL?$CClook?$CC?$CJ?$AA@ (0000)
00271B7C 0024:
	??_C@_0CE@DPIKLLNB@17?5?$DN?$DN?5get_icon_type?$CIL?$CCright?9stic@ (0000)
00271BA0 001d:
	??_C@_0BN@GOCJIAID@30?5?$DN?$DN?5get_icon_type?$CIL?$CCmove?$CC?$CJ?$AA@ (0000)
00271BC0 0023:
	??_C@_0CD@BJHDPKGJ@16?5?$DN?$DN?5get_icon_type?$CIL?$CCleft?9stick@ (0000)
00271BE8 0041:
	??_C@_0EB@PKGKIBFB@?$CD?$CD?$CDWARNING?3?5possible?5memory?5leak@ (0000)
00271C2C 0022:
	??_C@_0CC@MHEDEDIP@widget?5pause?5counter?5out?5of?5whac@ (0000)
00271C50 0025:
	??_C@_0CF@EFHCMBNF@event?5handler?5failed?5to?5spawn?5wi@ (0000)
00271C78 004b:
	??_C@_0EL@BONJAKAK@a?5?8widget?5deleted?8?5event?5handler@ (0000)
00271CC4 0025:
	??_C@_0CF@OIBDABJ@widget?5?$CG?$CG?5widget_globals?4initial@ (0000)
00271CEC 001a:
	??_C@_0BK@EFAHKEKA@sound?2sfx?2ui?2flag_failure?$AA@ (0000)
00271D08 0012:
	??_C@_0BC@LFDBPFAD@sound?2sfx?2ui?2back?$AA@ (0000)
00271D1C 0015:
	??_C@_0BF@FGMCPHMP@sound?2sfx?2ui?2forward?$AA@ (0000)
00271D34 0014:
	??_C@_0BE@EOGFPNHD@sound?2sfx?2ui?2cursor?$AA@ (0000)
00271D48 003a:
	??_C@_0DK@MJICNCDL@invalid?5controller?5index?5specifi@ (0000)
00271D88 005d:
	??_C@_0FN@LKLJLEJ@_list_items_generated_from_strin@ (0000)
00271DE8 0069:
	??_C@_0GJ@EGCLHEFN@no?5child?5widget?5references?5are?5n@ (0000)
00271E58 0063:
	??_C@_0GD@BFCMHBJK@_list_items_generated_from_strin@ (0000)
00271EBC 0026:
	??_C@_0CG@GCOFCMPN@icon_index?$DO?$DN0?5?$CG?$CG?5icon_index?$DMNUM_@ (0000)
00271EE4 0006:
	??_C@_05MAJJAKPI@FALSE?$AA@ (0000)
00271EEC 0004:
	??_C@_13EJFHHPOP@?$AA?$CF?$AA?$AA@ (0000)
00271EF0 0047:
	??_C@_0EH@BDEOCECI@failed?5to?5render?5text?5box?5widget@ (0000)
00271F38 0042:
	??_C@_0EC@OINFLBMA@failed?5to?5render?5text?5box?5widget@ (0000)
00271F7C 0020:
	??_C@_1CA@EBMNPDDP@?$AA?$DM?$AAo?$AAu?$AAt?$AA?5?$AAo?$AAf?$AA?5?$AAm?$AAe?$AAm?$AAo?$AAr?$AAy?$AA?$DO?$AA?$AA@ (0000)
00271FA0 0049:
	??_C@_0EJ@ICFFKBCK@failed?5to?5render?5spinner?5list?5it@ (0000)
00271FF0 0044:
	??_C@_0EE@GFHAOED@failed?5to?5render?5spinner?5list?5it@ (0000)
00272034 0029:
	??_C@_0CJ@CLCMGDOK@failed?5to?5find?5event?5focus?5targe@ (0000)
00272060 0046:
	??_C@_0EG@GAAKHNIE@failed?5to?5spawn?5thread?5for?5files@ (0000)
002720A8 003c:
	??_C@_0DM@DBODFMFA@begining?5filesystem?5checks?5?$CG?5sav@ (0000)
002720E4 002b:
	??_C@_0CL@DAIOFIBL@widget_globals?4initialization_th@ (0000)
00272110 003d:
	??_C@_0DN@CLOOAICL@failed?5to?5set?5focus?5to?5the?5next?5@ (0000)
00272150 006d:
	??_C@_0GN@CDIJNPDM@spinner?5lists?5with?5more?5that?51?5v@ (0000)
002721C0 0033:
	??_C@_0DD@MMCOEHPE@spinner?5lists?5must?5be?5either?51?9?5@ (0000)
002721F8 0041:
	??_C@_0EB@HAOLGJEB@failed?5to?5set?5focus?5to?5the?5?$CD?$CFd?5l@ (0000)
0027223C 0034:
	??_C@_0DE@PKDHGMBO@calling?5a?5list?5widget?5function?5o@ (0000)
00272270 0016:
	??_C@_0BG@ENJMIAKH@widget?9?$DOfocused_child?$AA@ (0000)
00272288 0048:
	??_C@_0EI@GEFPBJMP@couldn?8t?5load?5conditional?5widget@ (0000)
002722D0 0029:
	??_C@_0CJ@FPJLGLJO@condition?5handler?5failed?5to?5spaw@ (0000)
00272300 0045:
	??_C@_0EF@JLKENOEN@failed?5to?5open?5widget?5because?5th@ (0000)
00272348 0019:
	??_C@_0BJ@KIDHBPBC@previous?9?$DOnext?5?$DN?$DN?5widget?$AA@ (0000)
00272364 0019:
	??_C@_0BJ@FGBHJFEB@next?9?$DOprevious?5?$DN?$DN?5widget?$AA@ (0000)
00272380 0044:
	??_C@_0EE@NFLPKLKO@failed?5to?5close?5widget?5because?5e@ (0000)
002723C8 0045:
	??_C@_0EF@EEMBCNDP@failed?5to?5reload?5widget?5because?5@ (0000)
00272410 004e:
	??_C@_0EO@PGEAPKEN@failed?5to?5give?5focus?5to?5a?5widget@ (0000)
00272460 001e:
	??_C@_0BO@HFOFCHPP@event?5handler?5function?5failed?$AA@ (0000)
00272480 002a:
	??_C@_0CK@GOFLJEI@failed?5to?5run?5ui?5widget?5event?5sc@ (0000)
002724AC 000c:
	??_C@_0M@EJOMDMOI@ui?2small_ui?$AA@ (0000)
002724B8 0016:
	??_C@_0BG@ELECBMPG@window_bounds?5?$CB?$DN?5NULL?$AA@ (0000)
002724D0 001f:
	??_C@_0BP@JAOAINDB@failed?5to?5load?5widget?5children?$AA@ (0000)
002724F0 009a:
	??_C@_0JK@BLPAHHHK@if?5the?5_widget_pass_handled_even@ (0000)
0027258C 0021:
	??_C@_0CB@BADDKMAA@widget?9?$DOlocal_player_index?$DN?$DNNONE@ (0000)
002725B0 0037:
	??_C@_0DH@DCPBMEID@widget?5?$CG?$CG?5definition?5?$CG?$CG?5event?5?$CG?$CG@ (0000)
002725E8 002c:
	??_C@_0CM@OJPOGECC@ui_widget_definition?5tag?5?8?$CFs?8?1?$CFd@ (0000)
00272614 002c:
	??_C@_0CM@MEFFCNMB@failed?5to?5create?5new?5widget?$DL?5out@ (0000)
00272640 0035:
	??_C@_0DF@PBOMEHIM@?$CIwidget_stack?$DO?$DN0?$CJ?5?$CG?$CG?5?$CIwidget_sta@ (0000)
00272678 0026:
	??_C@_0CG@GBPNNCLI@?$CIname?5?$CB?$DN?5NULL?$CJ?5?$HM?$HM?5?$CItag_index?5?$CB?$DN?5@ (0000)
002726A0 002a:
	??_C@_0CK@OFOHOPCB@failed?5to?5initialize?5the?5virtual@ (0000)
002726CC 0028:
	??_C@_0CI@EBHLJEJP@failed?5to?5load?5main?5screen?5shell@ (0000)
002726F4 001d:
	??_C@_0BN@PLHBGNEF@ui?2shell?2main_menu?2main_menu?$AA@ (0000)
00272714 0018:
	??_C@_0BI@HFFEIMKP@xbox?5command?5line?$DN?5?8?$CFs?8?$AA@ (0000)
0027272C 0006:
	??_C@_05GLGMCIFH@xdemo?$AA@ (0000)
00272734 002f:
	??_C@_0CP@FMNMOLIB@failed?5to?5load?5networked?5pregame@ (0000)
00272768 005b:
	??_C@_0FL@CANKNMEF@ui?2shell?2main_menu?2multiplayer_t@ (0000)
002727C4 002a:
	??_C@_0CK@BADOGGKF@failed?5to?5load?5map?5select?5postga@ (0000)
002727F0 0060:
	??_C@_0GA@GFIHBLPH@ui?2shell?2main_menu?2multiplayer_t@ (0000)
00272850 0035:
	??_C@_0DF@GJABDHAE@failed?5to?5load?5pregame?5screen?5af@ (0000)
00272888 0063:
	??_C@_0GD@HIMIFBNM@ui?2shell?2main_menu?2multiplayer_t@ (0000)
002728EC 0025:
	??_C@_0CF@GLGHHELH@widget?5pause?5counter?5is?5out?5of?5w@ (0000)
00272914 002f:
	??_C@_0CP@BGKPJLCC@expected?5a?5text?5box?5widget?5in?5th@ (0000)
00272944 0032:
	??_C@_0DC@JNAKMNPN@error?5screen?5widget?5tag?5not?5laye@ (0000)
00272978 0020:
	??_C@_0CA@PLEKCLAA@failed?5to?5display?5error?5message?$AA@ (0000)
00272998 0048:
	??_C@_0EI@MDMAJGJE@there?5is?5already?5an?5error?5messag@ (0000)
002729E0 0032:
	??_C@_0DC@JLPDPOMG@aborting?5to?5the?5main?5menu?5root?0?5@ (0000)
00272A18 0044:
	??_C@_0EE@HGHDGOBG@?$CIwidget_stack?$DO?$DN0?$CJ?5?$CG?$CG?5?$CIwidget_sta@ (0000)
00272A5C 001b:
	??_C@_0BL@ILOCFJOB@invalid?5local?5player?5count?$AA@ (0000)
00272A78 0028:
	??_C@_0CI@NLLLFMIK@ui?2shell?2error?2error_nonmodal_qt@ (0000)
00272AA0 0025:
	??_C@_0CF@IPCJKMOA@ui?2shell?2error?2error_modal_qtrsc@ (0000)
00272AC8 0029:
	??_C@_0CJ@MDGMGIIG@ui?2shell?2error?2error_nonmodal_ha@ (0000)
00272AF4 0026:
	??_C@_0CG@GOFPHCDA@ui?2shell?2error?2error_modal_halfs@ (0000)
00272B1C 0029:
	??_C@_0CJ@COJPHONK@ui?2shell?2error?2error_nonmodal_fu@ (0000)
00272B48 0026:
	??_C@_0CG@IDKMGEGM@ui?2shell?2error?2error_modal_fulls@ (0000)
00272B70 0059:
	??_C@_0FJ@LCJINDPJ@there?5is?5already?5a?5deferred?9for?9@ (0000)
00272BCC 001b:
	??_C@_0BL@DHJKOIEB@failed?5to?5load?5?8?$CFs?8?5widget?$AA@ (0000)
00272BE8 001b:
	??_C@_0BL@PIJOAKFD@expected?5a?5text?5box?5widget?$AA@ (0000)
00272C04 003b:
	??_C@_0DL@MFGPNPO@ui?2shell?2error?2error_abort_to_da@ (0000)
00272C40 0028:
	??_C@_0CI@JFEBNJBP@ui?2shell?2error?2error_abort_to_da@ (0000)
00272C68 0017:
	??_C@_0BH@FIKKMDKG@Noooooooooooooooooo?$CB?$CB?$CB?$AA@ (0000)
00272C80 003a:
	??_C@_0DK@KGMPEFEN@can?8t?5display?5scenario?5help?5beca@ (0000)
00272CBC 003d:
	??_C@_0DN@BOLHGMPP@can?8t?5display?5scenario?5help?$DL?5unk@ (0000)
00272CFC 0023:
	??_C@_0CD@DGCLPNFD@failed?5to?5load?5in?9game?5help?5dial@ (0000)
00272D20 002f:
	??_C@_0CP@ODEPBIAI@expected?5text?5box?5widget?5in?5play@ (0000)
00272D50 0036:
	??_C@_0DG@EMGDHLEG@ui?2shell?2solo_game?2player_help?2p@ (0000)
00272D88 0004:
	??_C@_03JGNLDLAI@d40?$AA@ (0000)
00272D8C 0036:
	??_C@_0DG@EIOOAHPE@ui?2shell?2solo_game?2player_help?2p@ (0000)
00272DC4 0004:
	??_C@_03JCFGEHLK@d20?$AA@ (0000)
00272DC8 0036:
	??_C@_0DG@NBLEEDPP@ui?2shell?2solo_game?2player_help?2p@ (0000)
00272E00 0004:
	??_C@_03LAMADLB@c40?$AA@ (0000)
00272E04 0036:
	??_C@_0DG@NFDJDPEN@ui?2shell?2solo_game?2player_help?2p@ (0000)
00272E3C 0004:
	??_C@_03PIBHPAD@c20?$AA@ (0000)
00272E40 0036:
	??_C@_0DG@NHHPIBBE@ui?2shell?2solo_game?2player_help?2p@ (0000)
00272E78 0004:
	??_C@_03NMHMBFK@c10?$AA@ (0000)
00272E7C 0036:
	??_C@_0DG@GJAICEJK@ui?2shell?2solo_game?2player_help?2p@ (0000)
00272EB4 0004:
	??_C@_03LDLAGENE@b40?$AA@ (0000)
00272EB8 0036:
	??_C@_0DG@GMEHDCBP@ui?2shell?2solo_game?2player_help?2p@ (0000)
00272EF0 0004:
	??_C@_03LGPPHCFB@b30?$AA@ (0000)
00272EF4 0036:
	??_C@_0DG@HKHPOBED@ui?2shell?2solo_game?2player_help?2p@ (0000)
00272F2C 0004:
	??_C@_03KAMHKBAN@a50?$AA@ (0000)
00272F30 0036:
	??_C@_0DG@HOPCJNPB@ui?2shell?2solo_game?2player_help?2p@ (0000)
00272F68 0004:
	??_C@_03KEEKNNLP@a30?$AA@ (0000)
00272F6C 0036:
	??_C@_0DG@HNHGEJJP@ui?2shell?2solo_game?2player_help?2p@ (0000)
00272FA4 0004:
	??_C@_03KHMOAJNB@a10?$AA@ (0000)
00272FA8 0010:
	??_C@_0BA@CDPMFNHN@string_index?$DO?$DN0?$AA@ (0000)
00272FB8 002d:
	??_C@_0CN@FAHAPHCM@failed?5to?5load?5full?5screen?5pause@ (0000)
00272FE8 0029:
	??_C@_0CJ@OJHKNGAB@ui?2shell?2solo_game?2pause_game?2pa@ (0000)
00273018 0053:
	??_C@_0FD@MJAJHNPK@the?5ui?5seems?5to?5be?5confused?4?4?4?5a@ (0000)
0027306C 002e:
	??_C@_0CO@LCDPHEGN@failed?5to?5load?5split?5screen?5paus@ (0000)
0027309C 0036:
	??_C@_0DG@NEJLJPFI@ui?2shell?2solo_game?2pause_game?2pa@ (0000)
002730D4 0030:
	??_C@_0DA@JLINLCDA@invalid?5local?5player?5count?5for?5m@ (0000)
00273104 0033:
	??_C@_0DD@CLAJLOAB@ui?2shell?2multiplayer_game?2pause_@ (0000)
00273138 0033:
	??_C@_0DD@NDMLPFLH@ui?2shell?2multiplayer_game?2pause_@ (0000)
0027316C 002d:
	??_C@_0CN@BIDFDGIG@failed?5to?5load?5multiplayer?5pause@ (0000)
0027319C 0033:
	??_C@_0DD@KPKKNAGM@ui?2shell?2multiplayer_game?2pause_@ (0000)
002731D0 002a:
	??_C@_0CK@MAHDJCLA@invalid?5widget?5controller?5index?5@ (0000)
002731FC 0037:
	??_C@_0DH@EDIJCICF@waiting?5for?5?$CFd?5ticks?5before?5disp@ (0000)
00301DA0 413c:
	_data_00301da0 (0000)
	_widget_memory_pool (4034)
	_dashboard_abort_error (403c)
00454240 08b0:
	_bss_00454240 (0000)
*/

/* ---------- headers */

struct widget_instance;

#include "cseries.h"
#include "keyboard_glyphs.h" /* port: port/linux/game/keyboard_glyphs.c; HUD prompts only */
#include "menu_button_glyphs.h" /* port: port/linux/game/menu_button_glyphs.c */
#include "errors.h"
#include "bitmaps/bitmap_group.h"
#include "bitmaps/bitmaps.h"
#include "bink/bink_playback.h"
#include "bungie_net/common/thread.h"
#include "cache/cache_files.h"
#include "cache/texture_cache.h"
#include "cseries/cseries_windows.h"
#include "cutscene/cinematics.h"
#include "event_manager.h"
#include "game/game_engine.h"
#include "game/game_globals.h"
#include "game/players.h"
#include "hs/hs.h"
#include "input/input.h"
#include "input/input_abstraction.h"
#include "interface/attract_mode.h"
#include "interface/hud.h"
#include "interface/hud_definitions.h"
#include "interface/hud_draw.h"
#include "bitmaps/bitmap_color_conversion.h"
#include "interface/interface.h"
#include "interface/player_ui.h"
#include "interface/progress_bar.h"
#include "interface/ui_widget_game_data_input_functions.h"
#include "custom_edition_maps.h" /* port: port/linux/game/custom_edition_maps.c */
#include "interface/ui_widget_event_handler_functions.h"
#include "interface/ui_widget_text_search_and_replace_functions.h"
#include "interface/virtual_keyboard.h"
#include "main/main.h"
#include "memory/stack_memory_pool.h"
#include "networking/network_client_manager.h"
#include "networking/network_connection.h"
#include "networking/network_game_globals.h"
#include "networking/network_game_manager.h" /* port: struct network_game */
#include "networking/network_server_manager.h"
#include "network_coop.h" /* port: port/linux/game/network_coop.c */
#include "touch_postgame.h" /* port: port/linux/game/touch_postgame.c */
#include "rasterizer/rasterizer.h"
#include "saved games/player_profile.h"
#include "saved games/playlist_profile.h"
#include "saved games/saved_game_files.h"
#include "scenario/scenario.h"
#include "shell/shell_xbox.h"
#include "sound/game_sound.h"
#include "sound/sound_definitions.h"
#include "sound/sound_manager.h"
#include "tag_files/tag_files.h"
#include "tag_files/tag_groups.h"
#include "text/draw_string.h"
#include "text/text_group.h"
#include "text/unicode.h"
#include "ui_widget.h"
#include "mcc_pause_runtime.h" /* port: port/linux/game/mcc_pause_runtime.h */
#include "mcc_pause.h" /* port: port/linux/game/mcc_pause.h */
#include "mcc_ui_network.h" /* port: port/linux/game/mcc_ui_network.h */
#include "mcc_ui.h" /* port: port/linux/game/mcc_ui.h */
#include "mcc_hud_draw.h" /* port: port/linux/game/mcc_hud_draw.h */
#include "mcc_menu_preview.h" /* port: port/linux/game/mcc_menu_preview.c */
#include "mcc_menu_text.h" /* port: port/linux/game/mcc_menu_text.c */
#include "mcc_maps.h" /* port: port/linux/game/mcc_maps.h */
/* port: port/linux/game/menu_functions.c: a text's colour in place of its
own (the server browser's pings) */
boolean pc_menu_text_color(struct widget_instance *widget, real_argb_color *color);

/* ---------- constants */

enum
{
	/* port: 16 KB on the Xbox; the PC version's screens (port/assets/menus)
	have many more widgets */
	WIDGET_MEMORY_POOL_SIZE = 0x40000,
	MAXIMUM_WIDGET_MEMORY_POOL_BLOCKS = 4096
};

enum
{
	_ui_widget_type_container,
	_ui_widget_type_text_box,
	_ui_widget_type_spinner_list,
	_ui_widget_type_column_list,
	_ui_widget_type_game_model,
	_ui_widget_type_movie,
	_ui_widget_type_custom,
	NUMBER_OF_UI_WIDGET_TYPES
};

enum
{
	_ui_audio_feedback_none,
	_ui_audio_feedback_cursor,
	_ui_audio_feedback_forward,
	_ui_audio_feedback_back,
	_ui_audio_feedback_flag_failure,
	NUMBER_OF_UI_AUDIO_FEEDBACK_SOUNDS
};

enum
{
	_icon_a_button,
	_icon_b_button,
	_icon_x_button,
	_icon_y_button,
	_icon_black_button,
	_icon_white_button,
	_icon_left_trigger,
	_icon_right_trigger,
	_icon_dpad_up,
	_icon_dpad_down,
	_icon_dpad_left,
	_icon_dpad_right,
	_icon_start_button,
	_icon_back_button,
	_icon_left_thumb,
	_icon_right_thumb,
	_icon_left_stick,
	_icon_right_stick,
	_icon_action,
	_icon_throw_grenade,
	_icon_primary_trigger,
	_icon_integrated_light,
	_icon_jump,
	_icon_use_equipment,
	_icon_rotate_weapons,
	_icon_rotate_grenades,
	_icon_crouch,
	_icon_zoom,
	_icon_accept,
	_icon_back,
	_icon_move,
	_icon_look,
	_icon_custom_1,
	_icon_custom_2,
	_icon_custom_3,
	_icon_custom_4,
	_icon_custom_5,
	_icon_custom_6,
	_icon_custom_7,
	_icon_custom_8,
	NUMBER_OF_ICON_TYPES
};

enum
{
	/* only the icon types below _icon_action name a button bitmap of their own;
	the rest are resolved through the local player's control preferences */
	NUM_ICONS = _icon_action
};

enum
{
	_hud_icon_use_text_bit,
	_hud_icon_use_color_bit,
	_hud_icon_absolute_width_bit,
	NUMBER_OF_HUD_ICON_FLAGS
};

enum
{
	UI_WIDGET_DEFINITION_TAG = 'DeLa',
	FONT_GROUP_TAG = 'font'
};

enum
{
	_widget_controller0,
	_widget_controller1,
	_widget_controller2,
	_widget_controller3,
	_widget_controller_any,
	NUMBER_OF_WIDGET_CONTROLLERS
};

enum
{
	/* only the bits this file tests are named; when
	_widget_always_use_tag_controller_index_bit is set a definition asking for
	any player gets no controller at all, when it is clear the same request
	inherits the invoking widget's controller */
	_widget_pass_unhandled_events_to_children_bit = 0,
	_widget_pause_game_time_bit = 1,
	_widget_flash_background_bitmap_bit = 2,
	_widget_dpad_updown_tabs_thru_children_bit = 3,
	_widget_dpad_leftright_tabs_thru_children_bit = 4,
	_widget_dpad_updown_tabs_thru_list_items_bit = 5,
	_widget_dpad_leftright_tabs_thru_list_items_bit = 6,
	_widget_dont_focus_a_specific_child_bit = 7,
	_widget_pass_unhandled_events_to_all_children_bit = 8,
	_widget_render_regardless_of_controller_index_bit = 9,
	_widget_pass_handled_events_to_all_children_bit = 10,
	_widget_return_to_main_menu_if_no_history_bit = 11,
	_widget_always_use_tag_controller_index_bit = 12,
	_widget_always_render_with_nifty_fx_bit = 13,
	_widget_dont_push_history_data_bit = 14
};

enum
{
	_child_widget_use_custom_controller_index_bit = 0,
	NUMBER_OF_CHILD_WIDGET_FLAGS
};

enum
{
	_conditional_widget_load_if_event_handler_function_fails_bit = 0,
	NUMBER_OF_CONDITIONAL_WIDGET_FLAGS
};

enum
{
	_event_handler_close_current_widget_bit,
	_event_handler_close_other_widget_bit,
	_event_handler_close_all_widgets_bit,
	_event_handler_open_widget_bit,
	_event_handler_reload_self_bit,
	_event_handler_reload_widget_bit,
	_event_handler_give_focus_to_widget_bit,
	_event_handler_run_function_bit,
	_event_handler_replace_with_other_widget_bit,
	_event_handler_go_back_to_previous_widget_bit,
	_event_handler_run_scenario_script_bit,
	_event_handler_look_for_conditional_widget_on_failure_bit,
	NUMBER_OF_EVENT_HANDLER_FLAGS
};

enum
{
	_list_items_generated_in_code,
	_list_items_generated_from_string_list_tag,
	_list_items_only_one_tooltip_entry,
	_list_single_preview_box_no_scroll,
	NUMBER_OF_LIST_FLAGS
};

enum
{
	/* only the bit this file tests is named */
	_text_box_flashing_text_bit = 2
};

enum
{
	_text_justification_left,
	_text_justification_right,
	_text_justification_center,
	NUMBER_OF_TEXT_JUSTIFICATIONS
};

enum
{
	/* EVENT_MANAGER.C owns this enumeration and publishes none of it */
	_event_type_null,
	_event_type_left_stick,
	_event_type_right_stick,
	_event_type_button,
	NUMBER_OF_EVENT_TYPES
};

enum
{
	/* the button event types are the gamepad button indices; the enumeration
	runs 0..33 and only the types this file names are listed */
	_widget_event_b_button = _gamepad_analog_button_b,
	_widget_event_dpad_up = _gamepad_binary_button_dpad_up,
	_widget_event_dpad_down = _gamepad_binary_button_dpad_down,
	_widget_event_dpad_left = _gamepad_binary_button_dpad_left,
	_widget_event_dpad_right = _gamepad_binary_button_dpad_right,
	_widget_event_back_button = _gamepad_binary_button_back,
	_widget_event_left_stick_up = NUMBER_OF_GAMEPAD_BUTTONS,
	_widget_event_left_stick_down,
	_widget_event_left_stick_left,
	_widget_event_left_stick_right,
	_widget_event_right_stick_up,
	_widget_event_right_stick_down,
	_widget_event_right_stick_left,
	_widget_event_right_stick_right,
	_widget_event_created,
	_widget_event_deleted
};

enum
{
	/* a held dpad direction repeats no faster than this */
	DPAD_EVENT_REPEAT_MILLISECONDS = 250,
	NUMBER_OF_DPAD_DIRECTIONS =
		_widget_event_dpad_right - _widget_event_dpad_up + 1
};

enum
{
	/* deferred errors wait this long into a level before they are shown */
	DEFERRED_ERROR_DELAY_TICKS = 30
};

enum
{
	WIDGET_DELETED_PLAYER_CONTROL_INHIBIT_FLAGS = 0x0FFF
};

enum
{
	/* named for the errors each result raises; SAVED_GAME_FILES.H publishes no
	enumeration of its own */
	_file_system_check_result_none,
	_file_system_check_result_not_enough_free_space,
	_file_system_check_result_too_many_saved_games,
	NUMBER_OF_FILE_SYSTEM_CHECK_RESULTS
};

/* ---------- macros */

#define SIGN(n) ((n) >= 0 ? 1 : -1)

/* The public definition header intentionally exposes only the shared prefix;
this TU owns the complete January layout used below. */
#define ui_widget_definition_get(tag_index) \
	((struct ui_widget_definition *)tag_get(UI_WIDGET_DEFINITION_TAG, (tag_index)))

/* ---------- structures */

struct stack_memory_pool_block;

struct stack_memory_pool
{
	char const *name;
	byte *base_address;
	long size;
	long maximum_block_count;
	long next_block_index;
	long bytes_used;
	long maximum_bytes_used;
	unsigned long block_count;
	unsigned long maximum_block_count_used;
	long largest_block_size;
	boolean disable_compaction;
	byte unused29[3];
	struct stack_memory_pool_block *first_block;
	struct stack_memory_pool_block *last_block;
	struct stack_memory_pool_block *blocks[1];
};

struct stack_memory_pool_medium
{
	struct stack_memory_pool pool;
	struct stack_memory_pool_block *blocks[MAXIMUM_WIDGET_MEMORY_POOL_BLOCKS - 1];
};

typedef char verify_icon_hud_element_definition_size[
	sizeof(struct icon_hud_element_definition) == 0x10 ? 1 : -1];
typedef char verify_hud_globals_button_icons_offset[
	offsetof(struct hud_globals_definition, messaging.button_icons) == 0xC4 ? 1 : -1];
typedef char verify_interface_tag_references_definition_size[
	sizeof(struct game_globals_interface_tag_references) == 0x130 ? 1 : -1];
/* narrow views of the 'DeLa' widget definition tag and of the three block
elements this file walks; only the members this file reaches are named and
every other span is left explicitly unknown */

struct ui_widget_event_handler_reference
{
	long flags;
	short event_type;
	short function;
	struct tag_reference widget_tag;
	struct tag_reference sound_effect;
	char script[32];
};

struct ui_widget_child_reference
{
	struct tag_reference widget_tag;
	char name[32];
	long flags;
	short custom_controller_index;
	short vertical_offset;
	short horizontal_offset;
	byte unknown03A[0x50 - 0x3A];
};

struct ui_widget_conditional_reference
{
	struct tag_reference widget_tag;
	char name[32];
	long flags;
	short custom_controller_index;
	byte unknown036[0x50 - 0x36];
};

struct ui_widget_game_data_input_reference
{
	short function;
	byte unknown002[0x24 - 0x02];
};

struct ui_widget_search_and_replace_reference
{
	char search_string[32];
	short replace_function;
};

struct ui_widget_definition
{
	short type;
	short controller_index;
	char name[32];
	rectangle2d bounds;
	long flags;
	long milliseconds_to_auto_close;
	long auto_close_fade_time;
	struct tag_reference background_bitmap;
	struct tag_block game_data_inputs;
	struct tag_block event_handlers;
	struct tag_block search_and_replace_functions;
	byte unknown06C[0xEC - 0x6C];
	struct tag_reference text_label_string_list;
	struct tag_reference text_font;
	real_argb_color text_color;
	short justification;
	word text_box_flags;
	byte unknown120[0x12E - 0x120];
	short string_list_index;
	short horizontal_offset;
	short vertical_offset;
	byte unknown134[0x150 - 0x134];
	long list_flags;
	struct tag_reference list_header_bitmap;
	struct tag_reference list_footer_bitmap;
	rectangle2d list_header_bounds;
	rectangle2d list_footer_bounds;
	byte unknown184[0x1A4 - 0x184];
	struct tag_reference extended_description_widget;
	byte unknown1B4[0x2D4 - 0x1B4];
	struct tag_block conditional_widgets;
	byte unknown2E0[0x3E0 - 0x2E0];
	struct tag_block child_widgets;
};

typedef char verify_ui_widget_game_data_input_reference_size[
	sizeof(struct ui_widget_game_data_input_reference) == 0x24 ? 1 : -1];
typedef char verify_ui_widget_search_and_replace_reference_size[
	sizeof(struct ui_widget_search_and_replace_reference) == 0x22 ? 1 : -1];
typedef char verify_ui_widget_child_reference_size[
	sizeof(struct ui_widget_child_reference) == 0x50 ? 1 : -1];
typedef char verify_ui_widget_conditional_reference_size[
	sizeof(struct ui_widget_conditional_reference) == 0x50 ? 1 : -1];
typedef char verify_ui_widget_event_handler_reference_size[
	sizeof(struct ui_widget_event_handler_reference) == 0x48 ? 1 : -1];
typedef char verify_ui_widget_definition_bounds_offset[
	offsetof(struct ui_widget_definition, bounds) == 0x24 ? 1 : -1];
typedef char verify_ui_widget_definition_flags_offset[
	offsetof(struct ui_widget_definition, flags) == 0x2C ? 1 : -1];
typedef char verify_ui_widget_definition_game_data_inputs_offset[
	offsetof(struct ui_widget_definition, game_data_inputs) == 0x48 ? 1 : -1];
typedef char verify_ui_widget_definition_search_and_replace_offset[
	offsetof(struct ui_widget_definition, search_and_replace_functions) == 0x60 ? 1 : -1];
typedef char verify_ui_widget_definition_text_font_offset[
	offsetof(struct ui_widget_definition, text_font) == 0xFC ? 1 : -1];
typedef char verify_ui_widget_definition_text_color_offset[
	offsetof(struct ui_widget_definition, text_color) == 0x10C ? 1 : -1];
typedef char verify_ui_widget_definition_justification_offset[
	offsetof(struct ui_widget_definition, justification) == 0x11C ? 1 : -1];
typedef char verify_ui_widget_definition_text_box_flags_offset[
	offsetof(struct ui_widget_definition, text_box_flags) == 0x11E ? 1 : -1];
typedef char verify_ui_widget_definition_string_list_index_offset[
	offsetof(struct ui_widget_definition, string_list_index) == 0x12E ? 1 : -1];
typedef char verify_ui_widget_definition_horizontal_offset_offset[
	offsetof(struct ui_widget_definition, horizontal_offset) == 0x130 ? 1 : -1];
typedef char verify_ui_widget_definition_list_header_bitmap_offset[
	offsetof(struct ui_widget_definition, list_header_bitmap) == 0x154 ? 1 : -1];
typedef char verify_ui_widget_definition_list_header_bounds_offset[
	offsetof(struct ui_widget_definition, list_header_bounds) == 0x174 ? 1 : -1];
typedef char verify_ui_widget_definition_event_handlers_offset[
	offsetof(struct ui_widget_definition, event_handlers) == 0x54 ? 1 : -1];
typedef char verify_ui_widget_definition_background_bitmap_offset[
	offsetof(struct ui_widget_definition, background_bitmap) == 0x38 ? 1 : -1];
typedef char verify_ui_widget_definition_text_label_string_list_offset[
	offsetof(struct ui_widget_definition, text_label_string_list) == 0xEC ? 1 : -1];
typedef char verify_ui_widget_definition_list_flags_offset[
	offsetof(struct ui_widget_definition, list_flags) == 0x150 ? 1 : -1];
typedef char verify_ui_widget_definition_extended_description_offset[
	offsetof(struct ui_widget_definition, extended_description_widget) == 0x1A4 ? 1 : -1];
typedef char verify_ui_widget_definition_conditional_widgets_offset[
	offsetof(struct ui_widget_definition, conditional_widgets) == 0x2D4 ? 1 : -1];
typedef char verify_ui_widget_definition_child_widgets_offset[
	offsetof(struct ui_widget_definition, child_widgets) == 0x3E0 ? 1 : -1];
typedef char verify_ui_widget_definition_size[
	sizeof(struct ui_widget_definition) == 0x3EC ? 1 : -1];

struct ui_widget_deferred_error
{
	short error_code;
	short local_player_index;
	boolean modal;
	boolean pause_game_time;
};

struct ui_widget_deferred_cinematic_error
{
	short error_code;
	boolean modal;
	boolean pause_game_time;
};

struct widget_stack_data
{
	long previous_widget_tag;
	long focused_child_parent_widget_tag;
	short focused_child_index;
	short local_player_index;
};

struct widget_stack_node
{
	struct widget_stack_data data;
	struct widget_stack_node *next;
};

struct ui_widget_runtime_globals_prefix
{
	struct widget_instance *active_widgets[MAXIMUM_NUMBER_OF_LOCAL_PLAYERS];
	struct widget_stack_node *widget_stack[MAXIMUM_NUMBER_OF_LOCAL_PLAYERS];
	unsigned long current_system_milliseconds;
	long pause_disabled_ticks;
	short main_menu_deferred_error_code;
	short pause_game_time_count;
	real fade_to_black;
	struct ui_widget_deferred_error deferred_errors[MAXIMUM_NUMBER_OF_LOCAL_PLAYERS];
	short deferred_dashboard_error_code;
	boolean deferred_dashboard_optional;
	byte reserved004B;
	struct ui_widget_deferred_cinematic_error deferred_cinematic_errors[MAXIMUM_NUMBER_OF_LOCAL_PLAYERS];
	struct thread_reference *initialization_thread;
	short filesystem_check_result;
	boolean initialized;
	boolean dont_load_children_recursive;
	boolean debug_show_path;
	boolean processing_inhibited;
	boolean main_menu_music_active;
	boolean sound_paused;
};

struct ui_widget_bss_prefix
{
	wchar_t string_data[1024];
	struct ui_widget_runtime_globals_prefix widget_globals;
	boolean we_are_at_the_main_menu;
	byte unknown869[0x870 - 0x869];
	unsigned long dpad_event_times[MAXIMUM_NUMBER_OF_LOCAL_PLAYERS][NUMBER_OF_DPAD_DIRECTIONS];
};

typedef char verify_ui_widget_fade_to_black_offset[
	offsetof(
		struct ui_widget_runtime_globals_prefix,
		fade_to_black) == 0x2C ? 1 : -1];
typedef char verify_ui_widget_pause_disabled_ticks_offset[
	offsetof(
		struct ui_widget_runtime_globals_prefix,
		pause_disabled_ticks) == 0x24 ? 1 : -1];
typedef char verify_ui_widget_main_menu_deferred_error_code_offset[
	offsetof(
		struct ui_widget_runtime_globals_prefix,
		main_menu_deferred_error_code) == 0x28 ? 1 : -1];
typedef char verify_ui_widget_deferred_dashboard_error_code_offset[
	offsetof(
		struct ui_widget_runtime_globals_prefix,
		deferred_dashboard_error_code) == 0x48 ? 1 : -1];
typedef char verify_ui_widget_deferred_dashboard_optional_offset[
	offsetof(
		struct ui_widget_runtime_globals_prefix,
		deferred_dashboard_optional) == 0x4A ? 1 : -1];
typedef char verify_ui_widget_initialization_thread_offset[
	offsetof(
		struct ui_widget_runtime_globals_prefix,
		initialization_thread) == 0x5C ? 1 : -1];
typedef char verify_ui_widget_initialized_offset[
	offsetof(
		struct ui_widget_runtime_globals_prefix,
		initialized) == 0x62 ? 1 : -1];
typedef char verify_ui_widget_debug_show_path_offset[
	offsetof(
		struct ui_widget_runtime_globals_prefix,
		debug_show_path) == 0x64 ? 1 : -1];
typedef char verify_ui_widget_processing_inhibited_offset[
	offsetof(
		struct ui_widget_runtime_globals_prefix,
		processing_inhibited) == 0x65 ? 1 : -1];
typedef char verify_ui_widget_main_menu_music_active_offset[
	offsetof(
		struct ui_widget_runtime_globals_prefix,
		main_menu_music_active) == 0x66 ? 1 : -1];
typedef char verify_ui_widget_runtime_globals_prefix_size[
	sizeof(struct ui_widget_runtime_globals_prefix) == 0x68 ? 1 : -1];
typedef char verify_ui_widget_globals_offset[
	offsetof(
		struct ui_widget_bss_prefix,
		widget_globals) == 0x800 ? 1 : -1];
typedef char verify_ui_widget_main_menu_active_offset[
	offsetof(
		struct ui_widget_bss_prefix,
		we_are_at_the_main_menu) == 0x868 ? 1 : -1];
typedef char verify_ui_widget_dpad_event_times_offset[
	offsetof(
		struct ui_widget_bss_prefix,
		dpad_event_times) == 0x870 ? 1 : -1];

struct widget_animation_data
{
	short current_frame_index;
	short first_frame_index;
	short last_frame_index;
	short number_of_sprite_frames;
};

struct widget_instance
{
	long definition_tag_index;
	char const *name;
	short local_player_index;
	short horizontal_offset;
	short vertical_offset;
	short type;
	boolean visible;
	boolean render_regardless_of_controller_index;
	boolean disabled;
	boolean pause_game_time;
	boolean delete_recursion_lock;
	boolean widget_is_error_dialog;
	boolean close_if_local_player_controller_present;
	byte pad17;
	long creation_time;
	unsigned long milliseconds_to_auto_close;
	unsigned long auto_close_fade_time;
	real alpha_modifier;
	struct widget_instance *previous;
	struct widget_instance *next;
	struct widget_instance *parent;
	struct widget_instance *child;
	struct widget_instance *focused_child;
	union
	{
		struct
		{
			wchar_t *text;
			short string_list_index;
		} text_box;
		struct
		{
			short selected_index;
			/* counted back toward zero one step per rendered frame; the two
			tab functions start it at +15 and -15 and the column list renderer
			clears it */
			short last_list_tab_direction;
			void *list_items;
			word number_of_items;
			struct widget_instance *extended_description;
			wchar_t *item_text;
		} list;
	} parameters;
	struct widget_animation_data animation;
};

typedef char verify_widget_instance_size[
	sizeof(struct widget_instance) == 0x58 ? 1 : -1];
typedef char verify_widget_instance_animation_offset[
	offsetof(struct widget_instance, animation) == 0x50 ? 1 : -1];

typedef char verify_widget_instance_creation_time_offset[
	offsetof(struct widget_instance, creation_time) == 0x18 ? 1 : -1];
typedef char verify_widget_instance_alpha_modifier_offset[
	offsetof(struct widget_instance, alpha_modifier) == 0x24 ? 1 : -1];
typedef char verify_widget_instance_next_offset[
	offsetof(struct widget_instance, next) == 0x2C ? 1 : -1];
typedef char verify_widget_instance_focused_child_offset[
	offsetof(struct widget_instance, focused_child) == 0x38 ? 1 : -1];
typedef char verify_widget_instance_text_box_string_list_index_offset[
	offsetof(struct widget_instance, parameters.text_box.string_list_index) == 0x40 ? 1 : -1];

/* ---------- prototypes */

static boolean transition_to_game_in_progress(
	void);
static __inline real compute_offset_coordinate(
	long time,
	real delta_per_second);
static short get_icon_type(
	wchar_t const *string);
static void render_state_text(
	rectangle2d *bounds,
	rectangle2d *cursor_bounds,
	wchar_t const *text);
static void render_state_bitmap(
	rectangle2d *bounds,
	rectangle2d *cursor_bounds,
	pixel32 color,
	struct icon_hud_element_definition *icon);
static boolean should_flip_sticks_for_local_player(
	short local_player_index);
static unsigned long __stdcall filesystem_initialization_thread_proc(
	void *input);
static void perform_filesystem_initialization(
	void);
static void ui_widget_delete_children_recursive(
	struct widget_instance *widget);
static struct widget_instance *ui_widget_launch_widget(
	struct widget_instance *widget,
	long new_widget_tag_index);
static __inline boolean widget_instance_can_handle_events(
	struct widget_instance *widget);
static struct widget_instance *widget_instance_find_by_tag_index_recursive(
	struct widget_instance *widget,
	long tag_index);
static void widget_instance_give_focus_directly(
	struct widget_instance *widget,
	struct widget_instance *new_focus);
static void widget_instance_give_focus_by_tag(
	struct widget_instance *widget,
	long tag_index,
	short local_player_index);

static __inline struct widget_instance *widget_instance_get_tail_child_widget(
	struct widget_instance *widget);
static void ui_widget_add_child(
	struct widget_instance *parent,
	struct widget_instance *child);
static void push_widget(
	struct widget_stack_node **top,
	struct widget_stack_data *data);
static boolean widget_instance_can_receive_events(
	struct widget_instance *widget);
static void widget_instance_set_focused_child_by_index(
	long tag_index,
	struct widget_instance *widget,
	short child_index);
static void widget_instance_go_back_to_previous(
	struct widget_instance *widget);
static __inline struct widget_instance *widget_instance_find_by_tag_index(
	long tag_index);
static void widget_instance_reload_recursive(
	struct widget_instance *widget);
static void ui_widget_reload_by_tag(
	long tag_index);
static void event_handler_dispatch(
	struct widget_instance *widget,
	struct ui_widget_definition *definition,
	struct event_record *event,
	struct ui_widget_event_handler_reference *handler,
	boolean *calling_widget_deleted);
static boolean ui_widget_load_children_recursive(
	struct widget_instance *widget,
	struct ui_widget_definition *definition);
/* port: whether the tag is one of the menus' (port/linux/game/menu_tags.c) */
boolean pc_menu_tag(
	long tag_index);
/* port: whether this machine has, or is adding, a second player: co-op,
split screen, the lobby's (port/linux/game/menu_functions.c) */
unsigned char pc_menu_split_players(
	void);
/* port: local_player_count, 0 before the players' globals are made
(source/game/players.c) */
short players_port_local_player_count(
	void);
/* port: where in its widget, and how large, the menus draw a frame of
ui.map's that they scale (port/linux/game/menu_tags.c) */
boolean pc_menu_frame_placement(
	struct bitmap_data const *bitmap,
	short *x,
	short *y,
	short *width,
	short *height);
/* port: the placeholder bitmap the menus draw a sprite of a high-res
texture from, or the sheet if it has none (port/linux/game/hud_hires_tags.c) */
struct bitmap_data const *hud_hires_sprite_bitmap(
	struct bitmap_data const *sheet,
	short sequence_index);
static void widget_instance_initialize(
	struct widget_instance *widget,
	struct widget_instance *parent,
	struct ui_widget_definition *definition,
	long tag_index,
	short local_player_index,
	short widget_stack);
static __inline real widget_instance_get_cumulative_alpha_modifier(
	struct widget_instance *widget);
static boolean widget_instance_text_box_is_focused(
	struct widget_instance *widget);
static boolean string_has_icons_to_draw(
	wchar_t *string);
static long search_and_replace(
	wchar_t *search,
	wchar_t *replace,
	wchar_t **string);
static void widget_instance_render_text_box(
	struct widget_instance *widget,
	struct ui_widget_definition *definition,
	rectangle2d *clip_rect,
	point2d offset,
	boolean focus);
static void widget_instance_render_spinner_list(
	struct widget_instance *widget,
	struct ui_widget_definition *definition,
	rectangle2d *clip_rect,
	point2d offset,
	boolean focus);
static void widget_instance_render_column_list(
	struct widget_instance *widget,
	struct ui_widget_definition *definition,
	rectangle2d *clip_rect,
	point2d offset,
	boolean focus);
static void widget_instance_render_recursive(
	struct widget_instance *widget,
	rectangle2d *clip_rect,
	point2d offset,
	boolean focus,
	boolean use_nifty_plasma_fx);
static __inline void widget_instance_update_animation_parameters(
	struct widget_instance *widget);
static __inline void spinner_list_update(
	struct widget_instance *widget);
static void column_list_update(
	struct widget_instance *widget,
	struct ui_widget_definition *definition);
static void widget_instance_tab_to_next_valid_widget(
	struct widget_instance *widget);
static void widget_instance_tab_to_previous_valid_widget(
	struct widget_instance *widget);
static void widget_instance_process_one_event_recursive(
	struct widget_instance *widget,
	struct ui_widget_definition *definition,
	struct event_record *event,
	boolean *return_widget_deleted);
static boolean ui_check_for_pause_game(
	void);
static long spinner_string_list_extra_count(
	long string_list_index);
static wchar_t *spinner_string_list_get_string(
	long string_list_index,
	short string_index);

/* ---------- globals */

/* port: text boxes' string list indices from here are the descriptions of
spinners' extra items (kills_to_win_extra_descriptions), above the Custom
Edition maps' display indices (custom_edition_maps.c keeps them below 0x7000) */
#define SPINNER_EXTRA_DESCRIPTION_BASE 0x7000
/* ... and the pixels a spinner with extra items is wider (for three digits) */
#define SPINNER_EXTRA_WIDTH 12

/* port: the strings a spinner's string list has past the tag's own, as more
items: the higher kills to win of the Slayer game type editor
(ui_widget_event_handler_functions.c saves and loads them) */
static wchar_t const *const kills_to_win_extra_strings[] =
{
	L"75", L"100", L"150", L"200", L"250", L"500",
};

/* ... their descriptions, as a text box's string list index of
SPINNER_EXTRA_DESCRIPTION_BASE and up (ui_widget_spinner_extra_description) */
static wchar_t const *const kills_to_win_extra_descriptions[] =
{
	L"Seventy-five kills to win. Settle in for a long\r\nfight.",
	L"A hundred kills to win. Made for big games.",
	L"A hundred and fifty kills to win. Only a crowded\r\nserver gets there.",
	L"Two hundred kills to win. Bring friends. Lots of\r\nthem.",
	L"Two hundred and fifty kills to win.",
	L"Five hundred kills to win. You'll be here a while.",
};

/* port: an error message of the port's own text (display_error_text_deferred):
the text waiting for its dialog, then the dialog's text box showing it */
static wchar_t const *ui_widget_port_error_pending_text = NULL;
static wchar_t const *ui_widget_port_error_text = NULL;
static struct widget_instance *ui_widget_port_error_text_box = NULL;

static struct ui_widget_bss_prefix ui_widget_globals_storage;

#define string_data ui_widget_globals_storage.string_data
#define widget_globals ui_widget_globals_storage.widget_globals
#define we_are_at_the_main_menu ui_widget_globals_storage.we_are_at_the_main_menu
#define dpad_event_times ui_widget_globals_storage.dpad_event_times
real_argb_color ui_plasma_effect_color;
short local_player_index_for_draw_string_and_hack_in_icons;

/* January defines this and never references it, as we do not */
real const _one_over_255 = 1.0f / 255.0f;
real const SECONDS_PER_MILLISECOND = 0.001f;

static struct stack_memory_pool_medium __medium_widget_memory_pool =
{
	{
		"widget_memory_pool",
		NULL,
		0,
		MAXIMUM_WIDGET_MEMORY_POOL_BLOCKS
	}
};

struct stack_memory_pool *widget_memory_pool = &__medium_widget_memory_pool.pool;

static boolean main_screen_shell_first_load = TRUE;

short dashboard_abort_error = NONE;

static char const *scenario_paths[10] =
{
	"levels\\a10\\a10",
	"levels\\a30\\a30",
	"levels\\a50\\a50",
	"levels\\b30\\b30",
	"levels\\b40\\b40",
	"levels\\c10\\c10",
	"levels\\c20\\c20",
	"levels\\c40\\c40",
	"levels\\d20\\d20",
	"levels\\d40\\d40"
};

static boolean icon_is_special[NUM_ICONS] =
{
	FALSE,	/* a-button */
	FALSE,	/* b-button */
	FALSE,	/* x-button */
	FALSE,	/* y-button */
	FALSE,	/* black-button */
	FALSE,	/* white-button */
	TRUE,	/* left-trigger */
	TRUE,	/* right-trigger */
	FALSE,	/* dpad-up */
	FALSE,	/* dpad-down */
	FALSE,	/* dpad-left */
	FALSE,	/* dpad-right */
	FALSE,	/* start-button */
	FALSE,	/* back-button */
	TRUE,	/* left-thumb */
	TRUE,	/* right-thumb */
	TRUE,	/* left-stick */
	TRUE	/* right-stick */
};

static wchar_t const *icon_names[NUMBER_OF_ICON_TYPES] =
{
	L"a-button",
	L"b-button",
	L"x-button",
	L"y-button",
	L"black-button",
	L"white-button",
	L"left-trigger",
	L"right-trigger",
	L"dpad-up",
	L"dpad-down",
	L"dpad-left",
	L"dpad-right",
	L"start-button",
	L"back-button",
	L"left-thumb",
	L"right-thumb",
	L"left-stick",
	L"right-stick",
	L"action",
	L"throw-grenade",
	L"primary-trigger",
	L"integrated-light",
	L"jump",
	L"use-equipment",
	L"rotate-weapons",
	L"rotate-grenades",
	L"crouch",
	L"zoom",
	L"accept",
	L"back",
	L"move",
	L"look",
	L"custom-1",
	L"custom-2",
	L"custom-3",
	L"custom-4",
	L"custom-5",
	L"custom-6",
	L"custom-7",
	L"custom-8"
};

/* indexed by icon type - _icon_action.  The first ten entries are game control
indices for game_input_preferences.game_control_to_xbox_buttons (that enumeration
is private to INPUT_ABSTRACTION.C); the last four are icon types used directly
when the icon does not depend on the local player's control preferences. */
static char button_mappings[_icon_custom_1 - _icon_action] =
{
	2,		/* action -> action */
	6,		/* throw-grenade -> grenade */
	7,		/* primary-trigger -> primary trigger */
	5,		/* integrated-light -> flashlight */
	0,		/* jump -> jump */
	4,		/* use-equipment -> melee */
	3,		/* rotate-weapons -> switch weapons */
	1,		/* rotate-grenades -> switch grenades */
	11,		/* crouch -> zoom */
	10,		/* zoom -> crouch */
	_icon_a_button,		/* accept */
	_icon_b_button,		/* back */
	_icon_left_stick,	/* move */
	_icon_right_stick	/* look */
};

static real global_ui_white_red = 0.8f;
static real global_ui_white_green = 0.8f;
static real global_ui_white_blue = 0.8f;


/* ---------- public code */

void set_ui_plasma_effect_color(
	real alpha,
	real red,
	real green,
	real blue)
{
	ui_plasma_effect_color.alpha = alpha;
	ui_plasma_effect_color.red = red;
	ui_plasma_effect_color.green = green;
	ui_plasma_effect_color.blue = blue;

	return;
}

boolean event_controller_index_compatible_with_widget(
	struct event_record const *event,
	struct widget_instance const *widget)
{
	short widget_controller_index;

	widget_controller_index = widget->local_player_index;
	return widget_controller_index == NONE ||
		widget_controller_index == event->controller_index;
}

void ui_widgets_safe_to_load(
	boolean safe)
{
	return;
}

void ui_widgets_inhibit_processing(
	boolean inhibit)
{
	match_assert(
		"c:\\halo\\SOURCE\\interface\\ui_widget.c",
		1174,
		widget_globals.initialized);
	widget_globals.processing_inhibited = inhibit;

	return;
}

static __inline real compute_offset_coordinate(
	long time,
	real delta_per_second)
{
	real scaled_delta = delta_per_second * 0.001f;

	return (real)fmod(
		scaled_delta * time,
		1.0);
}

void draw_bitmap_in_rect(
	struct bitmap_data *bitmap,
	rectangle2d *rect,
	rectangle2d *bitmap_rect,
	rectangle2d *clip_rect,
	pixel32 argb,
	struct rasterizer_dynamic_screen_geometry_parameters *multitexture_params,
	boolean no_plasma)
{
	if (bitmap && rect)
	{
		real_argb_color plasma_fade = ui_plasma_effect_color;
		real_rgb_color map_tint = { 0.9f, 0.9f, 0.9f };
		real map_fade = 0.9f;
		rectangle2d temp;
		real_point2d points[NUMBER_OF_POINTS_PER_RECTANGLE];
		struct dynamic_screen_vertex vertices[NUMBER_OF_POINTS_PER_RECTANGLE];
		struct rasterizer_dynamic_screen_geometry_parameters parameters;
		real bitmap_width;
		real bitmap_height;
		real texture_width;
		real texture_height;
		real_point2d map0_offset;
		real_point2d map1_offset;
		short rectangle_x0;
		short rectangle_y0;
		short rectangle_width;
		short rectangle_height;
		short source_width;
		short source_height;
		short vertex_index;

		if (!bitmap_rect)
		{
			temp.x0 = 0;
			temp.y0 = 0;
			temp.x1 = bitmap->width;
			temp.y1 = bitmap->height;
			bitmap_rect = &temp;
		}

		rectangle_width = rect->x1 - rect->x0;
		rectangle_height = rect->y1 - rect->y0;
		rectangle_x0 = rect->x0;
		rectangle_y0 = rect->y0;
		source_width = bitmap_rect->x1 - bitmap_rect->x0;
		source_height = bitmap_rect->y1 - bitmap_rect->y0;
		points[0].x = (real)rectangle_x0;
		points[0].y = (real)rectangle_y0;
		points[1].x = (real)(rectangle_x0 + rectangle_width);
		points[1].y = (real)rectangle_y0;
		points[2].x = (real)(rectangle_x0 + rectangle_width);
		points[2].y = (real)(rectangle_y0 + rectangle_height);
		points[3].x = (real)rectangle_x0;
		points[3].y = (real)(rectangle_y0 + rectangle_height);

		if (clip_rect)
		{
			if (clip_rect->x0 > rect->x0)
			{
				points[0].x = points[3].x = (real)clip_rect->x0;
			}
			if (clip_rect->x1 < rect->x1)
			{
				points[1].x = points[2].x = (real)clip_rect->x1;
			}
			if (clip_rect->y0 > rect->y0)
			{
				points[0].y = points[1].y = (real)clip_rect->y0;
			}
			if (clip_rect->y1 < rect->y1)
			{
				points[2].y = points[3].y = (real)clip_rect->y1;
			}
		}

		bitmap_width = MAX(1.0f, (real)bitmap->width);
		texture_width = MIN(
			(real)source_width / bitmap_width,
			1.0f);
		bitmap_height = MAX(1.0f, (real)bitmap->height);
		texture_height = MIN(
			(real)source_height / bitmap_height,
			1.0f);

		for (vertex_index = 0;
			vertex_index < NUMBER_OF_POINTS_PER_RECTANGLE;
			vertex_index++)
		{
			vertices[vertex_index].color = argb;
			vertices[vertex_index].texture_coordinates.x =
				(vertex_index % 3) ? texture_width : 0.0f;
			vertices[vertex_index].texture_coordinates.y =
				(vertex_index > 1) ? texture_height : 0.0f;
			vertices[vertex_index].position = points[vertex_index];
		}

		csmemset(&parameters, 0, sizeof(parameters));
		if (no_plasma)
		{
			parameters.map_texture_scale[0].j = 1.0f;
			parameters.map_texture_scale[0].i = 1.0f;
			parameters.map_scale[0].j = 1.0f;
			parameters.map_scale[0].i = 1.0f;
			parameters.map[0] = bitmap;
		}
		else
		{
			struct bitmap_data *plasma_bitmap = TAG_BLOCK_GET_ELEMENT(
				&bitmap_group_get(
					interface_get_tag_index(_interface_bitmap_iface_map3))->bitmaps,
				0,
				struct bitmap_data);
			long time = system_milliseconds();

			map0_offset.x =
				compute_offset_coordinate(time, 0.03215434f) * 311.0f;
			map0_offset.y =
				compute_offset_coordinate(time, 0.026795285f) * 311.0f;
			map1_offset.x =
				-compute_offset_coordinate(time, 0.035536603f);
			map1_offset.x *= 201.0f;
			map1_offset.y =
				-compute_offset_coordinate(time, 0.031094525f);
			map1_offset.y *= 201.0f;

			parameters.map[0] = plasma_bitmap;
			parameters.map0_to_1_blend_function = 5;
			parameters.map_scale[0].i = 1.0f;
			parameters.map_scale[0].j = 1.0f;
			parameters.map_wrapped[0] = TRUE;
			parameters.map_anchor_screen[0] = TRUE;
			parameters.map_texture_scale[0].i = 1.0f / 311.0f;
			parameters.map_texture_scale[0].j = 1.0f / 311.0f;
			parameters.map_tint[0] = &map_tint;
			parameters.map_fade[0] = &map_fade;
			parameters.map_offset[0] = &map0_offset;
			parameters.map[1] = plasma_bitmap;
			parameters.map1_to_2_blend_function = 0;
			parameters.map_scale[1].i = 1.0f;
			parameters.map_scale[1].j = 1.0f;
			parameters.map_wrapped[1] = TRUE;
			parameters.map_anchor_screen[1] = TRUE;
			parameters.map_texture_scale[1].i = 1.0f / 201.0f;
			parameters.map_texture_scale[1].j = 1.0f / 201.0f;
			parameters.map_tint[1] = &map_tint;
			parameters.map_fade[1] = &map_fade;
			parameters.map_offset[1] = &map1_offset;
			parameters.plasma_fade = plasma_fade;
			parameters.map_fade[2] = NULL;
			parameters.map_texture_scale[2].j = 1.0f;
			parameters.map_texture_scale[2].i = 1.0f;
			parameters.map_scale[2].j = 1.0f;
			parameters.map_scale[2].i = 1.0f;
			parameters.map[2] = bitmap;
			parameters.doing_plasma_effect = TRUE;
		}

		parameters.meter_parameters = NULL;
		parameters.point_sampled = FALSE;
		/* port: (rasterizer.h) */
		parameters.alpha_weighted = FALSE;
		parameters.framebuffer_blend_function = 0;
		rasterizer_psuedo_dynamic_screen_quad_draw(&parameters, vertices);
	}

	return;
}

void ui_widgets_set_fade_value(
	real value)
{
	widget_globals.fade_to_black = value;

	return;
}

void ui_widget_debug_show_path(
	boolean show)
{
	widget_globals.debug_show_path = show;

	return;
}

int widget_instance_count_children(
	struct widget_instance *widget)
{
	int count;
	struct widget_instance *child;

	count = 0;
	if (widget)
	{
		for (child = widget->child; child; child = child->next)
			count++;
	}
	return count;
}

struct widget_instance *widget_instance_get_nth_child(
	struct widget_instance *widget,
	int n)
{
	int i;
	struct widget_instance *result;

	match_assert(
		"c:\\halo\\SOURCE\\interface\\ui_widget.c",
		1050,
		widget);
	result = widget->child;
	for (i = 0; i < n && result; i++)
		result = result->next;

	return result;
}

void widget_instance_set_visibility_recursive(
	struct widget_instance *widget,
	boolean visible)
{
	struct widget_instance *child;

	match_assert(
		"c:\\halo\\SOURCE\\interface\\ui_widget.c",
		1859,
		widget);
	widget->visible = visible;
	for (child = widget->child; child; child = child->next)
		widget_instance_set_visibility_recursive(child, visible);

	return;
}

void *ui_widget_realloc(
	void *pointer,
	word size,
	char const *file,
	unsigned long line)
{
	return pool_resize_pointer(
		widget_memory_pool,
		pointer,
		size,
		file,
		line);
}

/* port: MCC maps are selected by their full namespaced path. They do not enter
the Xbox/CE multiplayer level-index list. */
boolean ui_widget_port_mcc_multiplayer_map_choose(char const *level_name)
{
	short index = mcc_maps_find(level_name);
	struct network_game_server *server = global_network_game_server_get();

	if (index == NONE || mcc_maps_campaign(index))
		return FALSE;
	main_set_multiplayer_map_name(level_name);
	game_engine_override_map_name(level_name);
	if (server)
		network_game_server_change_map_name(server, level_name);
	saved_game_file_remember_last_used_multiplayer_map(level_name);
	return TRUE;
}

boolean ui_widget_port_mcc_cooperative_level_choose(char const *level_name, short difficulty)
{
	struct network_game_server *server = global_network_game_server_get();
	struct game_variant variant;

	if (!server || !mcc_maps_level_campaign(level_name) ||
		difficulty < 0 || difficulty >= NUMBER_OF_GAME_DIFFICULTY_LEVELS)
		return FALSE;
	csmemset(&variant, 0, sizeof(variant));
	ustrcpy(variant.human_readable_game_description, L"Co-op");
	main_set_difficulty(difficulty);
	main_set_multiplayer_map_name(level_name);
	network_game_server_port_set_cooperative(server, difficulty);
	network_game_server_change_map_name(server, level_name);
	network_game_server_change_game_variant(server, &variant);
	return TRUE;
}

void widget_free(
	void *ptr)
{
	dispose_pointer(widget_memory_pool, ptr);

	return;
}

void main_menu_active(
	boolean active)
{
	we_are_at_the_main_menu = active;

	return;
}

boolean main_menu_is_active(
	void)
{
	return we_are_at_the_main_menu;
}

void ui_widget_load_progress_widget(
	void)
{
	error(
		_error_silent,
		"the old loading progress screen has been replaced with glowy halo gravy");

	return;
}

boolean filesystem_check_thread_is_active(
	void)
{
	return widget_globals.initialization_thread != NULL;
}

void display_error_when_main_menu_loaded(
	short error_code)
{
	if (widget_globals.main_menu_deferred_error_code == NONE)
	{
		widget_globals.main_menu_deferred_error_code = error_code;
		return;
	}

	error(
		_error_silent,
		"there is already an error message queued for display at the main menu; ignoring this one");
	return;
}

/* port: an error of the port's own text (as display_error_text_deferred's)
shown when the main menu is next loaded, as display_error_when_main_menu_loaded
shows one of the game's: a network game left for a reason the game has no
error for (a map missing: cache_files.c). The text is copied, when no error
is queued already. */
void display_error_text_when_main_menu_loaded(
	wchar_t const *text)
{
	static wchar_t queued_text[512];

	if (widget_globals.main_menu_deferred_error_code != NONE)
	{
		error(_error_silent, "there is already an error message queued for display at the main menu; ignoring this one");
		return;
	}
	ustrncpy(queued_text, text, NUMBEROF(queued_text) - 1);
	queued_text[NUMBEROF(queued_text) - 1] = 0;
	ui_widget_port_error_pending_text = queued_text;
	widget_globals.main_menu_deferred_error_code = _error_cannot_create_saved_game_file_with_empty_name;

	return;
}

void display_error_abort_to_dashboard_deferred(
	short error_code,
	boolean optional)
{
	if (widget_globals.deferred_dashboard_error_code == NONE)
	{
		widget_globals.deferred_dashboard_error_code = error_code;
		widget_globals.deferred_dashboard_optional = optional;
		return;
	}

	error(
		_error_silent,
		"there is already a deferred dashbaord error queued; ignoring this one!");
	return;
}

boolean ui_main_menu_music_active(
	void)
{
	return widget_globals.main_menu_music_active;
}

void ui_widgets_disable_pause_game(
	long duration_ticks)
{
	match_assert(
		"c:\\halo\\SOURCE\\interface\\ui_widget.c",
		2519,
		duration_ticks>=0);
	widget_globals.pause_disabled_ticks = duration_ticks;

	return;
}

struct widget_instance *widget_instance_get_topmost_parent(
	struct widget_instance *widget)
{
	while (widget->parent)
		widget = widget->parent;

	return widget;
}

int widget_instance_get_child_index_from_parent(
	struct widget_instance *widget)
{
	int result = NONE;

	if (widget->parent)
	{
		struct widget_instance *child = widget->parent->child;
		int index = 0;

		while (child)
		{
			if (child == widget)
			{
				result = index;
				break;
			}
			child = child->next;
			index++;
		}
	}

	return result;
}

pixel32 modulate_pixel32_by_real_alpha(
	pixel32 argb,
	real alpha)
{
	real modulated_alpha = (argb >> 24) * alpha;

	return (fast_ftol(modulated_alpha) << 24) | (argb & 0x00FFFFFF);
}

void ui_set_next_level(
	short level)
{
	long level_index = level;

	if (level_index != NONE)
	{
		if (level_index >= 0 && level_index <= 9)
		{
			main_set_map_name(main_get_solo_level_name(level));
			main_disallow_persistent_storage();
		}
		else
		{
			error(_error_silent, "unknown level");
			main_goto_main_menu();
		}
	}
	else
	{
		main_roll_credits();
	}

	return;
}

boolean ui_widgets_active(
	void)
{
	boolean result = FALSE;

	if (widget_globals.initialized)
	{
		long local_player_index;

		for (local_player_index = 0;
			local_player_index < MAXIMUM_NUMBER_OF_LOCAL_PLAYERS;
			local_player_index++)
		{
			if (widget_globals.active_widgets[local_player_index])
			{
				result = TRUE;
				break;
			}
		}
	}

	return result;
}

boolean main_menu_screen_is_active(
	void)
{
	if (we_are_at_the_main_menu == TRUE &&
		widget_globals.active_widgets[0] &&
		strcmp(widget_globals.active_widgets[0]->name, "the_main_menu") == 0)
	{
		return TRUE;
	}

	return FALSE;
}

static void *pool_alloc(
	unsigned long size)
{
	return match_malloc("c:\\halo\\SOURCE\\interface\\ui_widget.c", 117, size);
}

static void pool_free(
	void *pointer)
{
	match_free("c:\\halo\\SOURCE\\interface\\ui_widget.c", 118, pointer);

	return;
}

void ui_widgets_initialize(
	void)
{
	boolean success = TRUE;
	byte *base_address;
	long local_player_index;

	base_address = pool_alloc(WIDGET_MEMORY_POOL_SIZE);
	if (base_address)
	{
		widget_memory_pool->base_address = base_address;
		widget_memory_pool->size = WIDGET_MEMORY_POOL_SIZE;
	}
	else
	{
		success = FALSE;
	}
	stack_memory_pool_reset(widget_memory_pool);

	memset(&widget_globals, 0, sizeof(widget_globals));
	widget_globals.main_menu_deferred_error_code = NONE;
	widget_globals.deferred_dashboard_error_code = NONE;
	for (local_player_index = 0;
		local_player_index < MAXIMUM_NUMBER_OF_LOCAL_PLAYERS;
		local_player_index++)
	{
		widget_globals.deferred_errors[local_player_index].error_code = NONE;
		widget_globals.deferred_cinematic_errors[local_player_index].error_code = NONE;
	}
	widget_globals.initialized = success;
	widget_globals.fade_to_black = -1.0f;

	return;
}

void ui_widgets_dispose(
	void)
{
	ui_widgets_close_all();
	if (widget_memory_pool->base_address)
		pool_free(widget_memory_pool->base_address);
	widget_memory_pool->base_address = NULL;
	widget_memory_pool->size = 0;
	memset(&widget_globals, 0, sizeof(widget_globals));

	return;
}

static void pop_widget(
	struct widget_stack_node **top,
	struct widget_stack_data *data)
{
	struct widget_stack_node *node;

	match_assert(
		"c:\\halo\\SOURCE\\interface\\ui_widget.c",
		2556,
		top && data);
	node = *top;
	*data = node->data;
	*top = node->next;
	dispose_pointer(widget_memory_pool, node);

	return;
}

static void dispose_widget_stack(
	struct widget_stack_node **top)
{
	while (*top)
	{
		struct widget_stack_node *node = *top;

		*top = node->next;
		dispose_pointer(widget_memory_pool, node);
	}

	return;
}

void ui_widget_delete(
	struct widget_instance *widget)
{
	struct ui_widget_definition *definition;
	long handler_index;
	long widget_index;

	match_assert(
		"c:\\halo\\SOURCE\\interface\\ui_widget.c",
		469,
		widget && widget_globals.initialized);
	if (widget->delete_recursion_lock)
		return;
	widget->delete_recursion_lock = TRUE;
	if (widget->local_player_index != NONE && !widget->parent)
		player_control_inhibit_buttons(
			widget->local_player_index,
			WIDGET_DELETED_PLAYER_CONTROL_INHIBIT_FLAGS,
			TRUE);
	definition = ui_widget_definition_get(widget->definition_tag_index);
	for (handler_index = 0;
		handler_index < definition->event_handlers.count;
		handler_index++)
	{
		struct ui_widget_event_handler_reference *handler =
			(struct ui_widget_event_handler_reference *)definition->event_handlers.address + handler_index;

		if (handler->event_type == _widget_event_deleted &&
			TEST_FLAG(handler->flags, _event_handler_run_function_bit))
		{
			/* port: an event of nothing, not none: a map's widget may name a
			function for this handler that reads its event */
			static struct event_record no_event;
			boolean widget_deleted = FALSE;
			boolean handled;

			csmemset(&no_event, 0, sizeof(no_event));
			no_event.controller_index = NONE;
			handled = ui_widget_event_handler_function_invoke(
				widget,
				&no_event,
				handler->function,
				&widget_deleted);

			match_vassert(
				"c:\\halo\\SOURCE\\interface\\ui_widget.c",
				520,
				!widget_deleted,
				"a 'widget deleted' event handler tried to delete the widget being deleted!");
			if (handled == TRUE &&
				TEST_FLAG(handler->flags, _event_handler_open_widget_bit))
			{
				long new_widget_tag_index = handler->widget_tag.index;

				if (new_widget_tag_index != NONE &&
					!ui_widget_launch_widget(widget, new_widget_tag_index))
				{
					error(_error_silent, "event handler failed to spawn widget");
				}
			}
		}
	}
	if (widget->pause_game_time == TRUE)
	{
		match_vassert(
			"c:\\halo\\SOURCE\\interface\\ui_widget.c",
			544,
			widget_globals.pause_game_time_count > 0,
			"widget pause counter out of whack");
		if (--widget_globals.pause_game_time_count == 0)
		{
			if (game_time_get_paused())
			{
				game_time_set_paused(FALSE);
				if (we_are_at_the_main_menu)
				{
					main_menu_ensure_player_queues_exist();
					game_time_dispose_from_old_map();
					game_time_initialize_for_new_map();
					game_time_start();
				}
			}
			if (widget_globals.sound_paused == TRUE)
			{
				sound_pause(FALSE);
				widget_globals.sound_paused = FALSE;
			}
		}
	}
	ui_widget_delete_children_recursive(widget);
	if (widget->previous)
		widget->previous->next = widget->next;
	if (widget->next)
		widget->next->previous = widget->previous;
	if (widget->parent && widget->parent->child == widget)
		widget->parent->child = widget->next;
	switch (widget->type)
	{
	case _ui_widget_type_text_box:
		if (widget->parameters.text_box.text)
			dispose_pointer(widget_memory_pool, widget->parameters.text_box.text);
		if (widget == ui_widget_port_error_text_box)
		{
			ui_widget_port_error_text_box = NULL;
			ui_widget_port_error_text = NULL;
		}
		break;
	case _ui_widget_type_spinner_list:
	case _ui_widget_type_column_list:
		if (widget->parameters.list.list_items)
		{
			error(
				_error_silent,
				"###WARNING: possible memory leak disposing of a list widget (%s)",
				definition->name ? definition->name : "<unknown>");
		}
		if (widget->parameters.list.item_text)
			dispose_pointer(widget_memory_pool, widget->parameters.list.item_text);
		if (widget->parameters.list.extended_description)
			ui_widget_delete(widget->parameters.list.extended_description);
		break;
	}
	dispose_pointer(widget_memory_pool, widget);
	for (widget_index = 0;
		widget_index < MAXIMUM_NUMBER_OF_LOCAL_PLAYERS;
		widget_index++)
	{
		if (widget_globals.active_widgets[widget_index] == widget)
		{
			widget_globals.active_widgets[widget_index] = NULL;
			break;
		}
	}

	return;
}

static void ui_widget_delete_children_recursive(
	struct widget_instance *widget)
{
	struct widget_instance *child = widget->child;

	while (child)
	{
		struct widget_instance *next = child->next;

		match_assert(
			"c:\\halo\\SOURCE\\interface\\ui_widget.c",
			2750,
			child->previous == NULL);
		if (next)
		{
			match_assert(
				"c:\\halo\\SOURCE\\interface\\ui_widget.c",
				2754,
				next->previous == child);
		}
		ui_widget_delete(child);
		if (next)
			next->previous = NULL;
		child = next;
	}

	return;
}

static struct widget_instance *ui_widget_launch_widget(
	struct widget_instance *widget,
	long new_widget_tag_index)
{
	struct ui_widget_definition *definition = ui_widget_definition_get(new_widget_tag_index);
	struct widget_instance *root;
	struct widget_instance *new_widget;
	short local_player_index;

	/* port: the menus of multiplayer with other machines (not split screen's
	or co-op's) open only on maps of a build that plays multiplayer with the
	others (cache_files.c, cache_files_multiplayer_region); otherwise the
	player is told why, and the menu stays */
	{
		static char const multiplayer_menus[] = "ui\\shell\\main_menu\\multiplayer_type_select\\connected\\";
		char const *name = tag_get_name(new_widget_tag_index);
		char build[0x20];

		if (name &&
			!csstrncmp(name, multiplayer_menus, sizeof(multiplayer_menus) - 1) &&
			!cache_files_multiplayer_region(build))
		{
			cache_files_show_multiplayer_unavailable(NULL, build);

			return NULL;
		}
	}

	if (TEST_FLAG(definition->flags, _widget_always_use_tag_controller_index_bit))
	{
		switch (definition->controller_index)
		{
		case _widget_controller0:
			local_player_index = 0;
			break;
		case _widget_controller1:
			local_player_index = 1;
			break;
		case _widget_controller2:
			local_player_index = 2;
			break;
		case _widget_controller3:
			local_player_index = 3;
			break;
		case _widget_controller_any:
			local_player_index = NONE;
			break;
		default:
			match_vassert(
				"c:\\halo\\SOURCE\\interface\\ui_widget.c",
				5380,
				FALSE,
				"invalid widget controller index specified");
			break;
		}
	}
	else
	{
		switch (definition->controller_index)
		{
		case _widget_controller0:
			local_player_index = 0;
			break;
		case _widget_controller1:
			local_player_index = 1;
			break;
		case _widget_controller2:
			local_player_index = 2;
			break;
		case _widget_controller3:
			local_player_index = 3;
			break;
		case _widget_controller_any:
			local_player_index = widget->local_player_index;
			break;
		default:
			match_vassert(
				"c:\\halo\\SOURCE\\interface\\ui_widget.c",
				5392,
				FALSE,
				"invalid widget controller index specified");
			break;
		}
	}
	root = widget_instance_get_topmost_parent(widget);
	new_widget = ui_widget_load_by_name_or_tag(
		NULL,
		new_widget_tag_index,
		NULL,
		local_player_index,
		root->definition_tag_index,
		widget->parent ? widget->parent->definition_tag_index : NONE,
		(short)widget_instance_get_child_index_from_parent(widget));
	if (!new_widget)
		error(_error_silent, "event handler failed to spawn widget");

	return new_widget;
}

static __inline boolean widget_instance_can_handle_events(
	struct widget_instance *widget)
{
	struct ui_widget_definition *definition = ui_widget_definition_get(widget->definition_tag_index);

	if (!widget->disabled &&
		(definition->event_handlers.count > 0 ||
		widget->type == _ui_widget_type_spinner_list ||
		widget->type == _ui_widget_type_column_list))
	{
		return TRUE;
	}

	return FALSE;
}

/* port: a label in the PC version's lists (port/assets/menus): an item that
takes no events and has nothing in it that does, which its game passes over
(its rows of settings take none themselves, but their spinners do), or one
hidden (a list's rows past its items: port/linux/game/menu_functions.c) or
disabled (the server browser's column titles, which sort nothing) */
static boolean widget_instance_port_is_label(
	struct widget_instance *widget)
{
	return (!widget_instance_can_handle_events(widget) && !widget->child) || !widget->visible || widget->disabled;
}

static struct widget_instance *widget_instance_find_by_tag_index_recursive(
	struct widget_instance *widget,
	long tag_index)
{
	struct widget_instance *result = NULL;

	if (widget->definition_tag_index == tag_index)
	{
		result = widget;
	}
	else
	{
		struct widget_instance *child;

		for (child = widget->child; child && !result; child = child->next)
		{
			if (child->definition_tag_index == tag_index)
				result = child;
			else
				result = widget_instance_find_by_tag_index_recursive(child, tag_index);
		}
	}

	return result;
}

/* port: giving the focus clears focused_child all along the old focus's path,
then links only the new focus's parents. Backing out of a screen does that: it
loads the screen, focused on its first row, then focuses the row you left it
from, so the first row is left with no focused child. A row that only passes
events on (a setting's row and its spinner, a lobby row) then passes the d-pad,
keys and buttons to nothing when it has the focus again, and only the mouse,
which focuses the spinner itself, could change it. Such a row gets its first
child that takes events back, as it had when its screen was loaded, whenever
it takes the focus (here, and the d-pad's widget_instance_tab_to_*) */
static void widget_instance_refocus_passthrough(
	struct widget_instance *widget)
{
	struct widget_instance *child;
	long flags;

	if (!widget || widget->focused_child || widget_instance_can_handle_events(widget))
		return;
	flags = ui_widget_definition_get(widget->definition_tag_index)->flags;
	if (!TEST_FLAG(flags, _widget_pass_unhandled_events_to_children_bit) ||
		TEST_FLAG(flags, _widget_dont_focus_a_specific_child_bit))
	{
		return;
	}
	for (child = widget->child; child; child = child->next)
	{
		if (widget_instance_can_handle_events(child))
		{
			widget->focused_child = child;
			break;
		}
	}

	return;
}

static void widget_instance_give_focus_directly(
	struct widget_instance *widget,
	struct widget_instance *new_focus)
{
	struct widget_instance *focused_child =
		widget_instance_get_topmost_parent(widget)->focused_child;
	struct widget_instance *leaf;

	if (new_focus->disabled == TRUE)
	{
		struct widget_instance *substitute;

		for (substitute = new_focus->next; substitute; substitute = substitute->next)
		{
			if (widget_instance_can_handle_events(substitute))
				break;
		}
		if (!substitute && new_focus->parent)
		{
			for (substitute = new_focus->parent->child; substitute; substitute = substitute->next)
			{
				if (widget_instance_can_handle_events(substitute))
					break;
			}
			if (substitute == new_focus->parent->focused_child)
			{
				for (substitute = new_focus->previous; substitute; substitute = substitute->previous)
				{
					if (widget_instance_can_handle_events(substitute))
						break;
				}
			}
		}
		if (substitute)
			new_focus = substitute;
	}
	leaf = new_focus;
	if (focused_child)
	{
		if (new_focus &&
			focused_child->parent == new_focus->parent &&
			focused_child->parent)
		{
			new_focus->parent->focused_child = new_focus;
			widget_instance_refocus_passthrough(leaf);

			return;
		}
		while (focused_child)
		{
			focused_child->parent->focused_child = NULL;
			focused_child = focused_child->focused_child;
		}
	}
	while (new_focus->parent)
	{
		new_focus->parent->focused_child = new_focus;
		new_focus = new_focus->parent;
	}
	widget_instance_refocus_passthrough(leaf);

	return;
}

static void widget_instance_give_focus_by_tag(
	struct widget_instance *widget,
	long tag_index,
	short local_player_index)
{
	struct widget_instance *root = widget_instance_get_topmost_parent(widget);
	struct widget_instance *new_focus = widget_instance_find_by_tag_index_recursive(root, tag_index);

	if (new_focus)
		widget_instance_give_focus_directly(root, new_focus);
	else
		error(_error_silent, "failed to find event focus target widget");

	return;
}

boolean widget_event_function_list_widget_goto_next_item(
	struct widget_instance *widget,
	struct event_record *event,
	boolean *widget_deleted)
{
	struct ui_widget_definition *definition;
	struct widget_instance *child;
	long item_index;
	boolean result = TRUE;

	match_assert(
		"c:\\halo\\SOURCE\\interface\\ui_widget.c",
		1230,
		widget);
	definition = ui_widget_definition_get(widget->definition_tag_index);
	match_vassert(
		"c:\\halo\\SOURCE\\interface\\ui_widget.c",
		1233,
		widget->type == _ui_widget_type_spinner_list || widget->type == _ui_widget_type_column_list,
		"calling a list widget function on a non-list widget");
	if (widget->parameters.list.list_items && widget->parameters.list.number_of_items > 0)
	{
		item_index = widget->parameters.list.selected_index + 1;
		if (item_index >= widget->parameters.list.number_of_items)
			item_index = 0;
		if (widget->type == _ui_widget_type_column_list)
		{
			child = widget_instance_get_nth_child(widget, item_index);
			if (child)
			{
				widget_instance_give_focus_by_tag(
					widget,
					child->definition_tag_index,
					widget->local_player_index);
				widget->parameters.list.selected_index = (short)item_index;
			}
			else
			{
				error(
					_error_silent,
					"failed to set focus to the #%d list item of a column list widget",
					item_index);
				result = FALSE;
			}
		}
		else if (widget->type == _ui_widget_type_spinner_list)
		{
			if (definition->child_widgets.count > 1)
			{
				match_vassert(
					"c:\\halo\\SOURCE\\interface\\ui_widget.c",
					1268,
					definition->child_widgets.count == 3,
					"spinner lists must be either 1- or 3-wide... sorry");
				if (widget->focused_child == widget->child ||
					widget->focused_child == widget->child->next)
				{
					if (widget->focused_child->next)
						widget_instance_give_focus_directly(widget, widget->focused_child->next);
				}
			}
			widget->parameters.list.selected_index = (short)item_index;
		}
	}
	else
	{
		if (widget->type == _ui_widget_type_spinner_list)
		{
			match_vassert(
				"c:\\halo\\SOURCE\\interface\\ui_widget.c",
				1288,
				definition->child_widgets.count <= 1,
				"spinner lists with more that 1 visible item need to have code-generated lists associated with them... sorry.");
		}
		if (widget->type == _ui_widget_type_spinner_list &&
			TEST_FLAG(definition->list_flags, _list_items_generated_from_string_list_tag) &&
			definition->child_widgets.count == 0)
		{
			widget->parameters.list.selected_index++;
			if (widget->parameters.list.selected_index == widget->parameters.list.number_of_items)
				widget->parameters.list.selected_index = 0;
		}
		else
		{
			child = NULL;
			if (widget->focused_child)
			{
				item_index = widget->parameters.list.selected_index + 1;
				child = widget->focused_child->next;
				if (item_index == widget->parameters.list.number_of_items)
					child = NULL;
			}
			if (!child)
			{
				child = widget->child;
				item_index = 0;
			}
			/* port: the PC version's lists (port/assets/menus) pass over the
			children that take no events (their labels and lines), as its
			game does */
			if (child && widget->type == _ui_widget_type_column_list &&
				pc_menu_tag(widget->definition_tag_index))
			{
				long tries = 0;

				while (widget_instance_port_is_label(child) && tries++ < 256)
				{
					child = child->next;
					item_index++;
					if (!child)
					{
						child = widget->child;
						item_index = 0;
					}
				}
			}
			if (child)
			{
				widget_instance_give_focus_by_tag(
					widget,
					child->definition_tag_index,
					widget->local_player_index);
				widget->parameters.list.selected_index = (short)item_index;
			}
			else
			{
				error(
					_error_silent,
					"failed to set focus to the next list item of a column widget");
				result = FALSE;
			}
		}
	}
	if (result)
		widget->parameters.list.last_list_tab_direction = 15;

	return result;
}

boolean widget_event_function_list_widget_goto_previous_item(
	struct widget_instance *widget,
	struct event_record *event,
	boolean *widget_deleted)
{
	struct ui_widget_definition *definition;
	struct widget_instance *child;
	long item_index;

	match_assert(
		"c:\\halo\\SOURCE\\interface\\ui_widget.c",
		1359,
		widget);
	definition = ui_widget_definition_get(widget->definition_tag_index);
	match_vassert(
		"c:\\halo\\SOURCE\\interface\\ui_widget.c",
		1362,
		widget->type == _ui_widget_type_spinner_list || widget->type == _ui_widget_type_column_list,
		"calling a list widget function on a non-list widget");
	if (widget->parameters.list.list_items && widget->parameters.list.number_of_items > 0)
	{
		item_index = widget->parameters.list.selected_index - 1;
		if (item_index < 0)
			item_index = widget->parameters.list.number_of_items - 1;
		if (widget->type == _ui_widget_type_column_list)
		{
			child = widget_instance_get_nth_child(widget, item_index);
			if (child)
			{
				widget_instance_give_focus_by_tag(
					widget,
					child->definition_tag_index,
					widget->local_player_index);
				widget->parameters.list.selected_index = (short)item_index;
			}
			else
			{
				error(
					_error_silent,
					"failed to set focus to the #%d list item of a column list widget",
					item_index);

				return FALSE;
			}
		}
		else if (widget->type == _ui_widget_type_spinner_list)
		{
			if (definition->child_widgets.count > 1)
			{
				match_vassert(
					"c:\\halo\\SOURCE\\interface\\ui_widget.c",
					1396,
					definition->child_widgets.count == 3,
					"spinner lists must be either 1- or 3-wide... sorry");
				if (widget->focused_child != widget->child)
				{
					match_assert(
						"c:\\halo\\SOURCE\\interface\\ui_widget.c",
						1405,
						widget->focused_child);
					if (widget->focused_child->previous)
						widget_instance_give_focus_directly(widget, widget->focused_child->previous);
				}
			}
			widget->parameters.list.selected_index = (short)item_index;
		}
	}
	else
	{
		if (widget->type == _ui_widget_type_spinner_list)
		{
			match_vassert(
				"c:\\halo\\SOURCE\\interface\\ui_widget.c",
				1420,
				definition->child_widgets.count <= 1,
				"spinner lists with more that 1 visible item need to have code-generated lists associated with them... sorry.");
		}
		if (widget->type == _ui_widget_type_spinner_list &&
			TEST_FLAG(definition->list_flags, _list_items_generated_from_string_list_tag) &&
			definition->child_widgets.count == 0)
		{
			widget->parameters.list.selected_index--;
			if (widget->parameters.list.selected_index < 0)
				widget->parameters.list.selected_index = widget->parameters.list.number_of_items - 1;
		}
		else
		{
			child = NULL;
			if (widget->focused_child)
			{
				item_index = widget->parameters.list.selected_index - 1;
				child = widget->focused_child->previous;
			}
			if (!child)
			{
				child = widget->child;
				item_index = 0;
				while (child->next)
				{
					child = child->next;
					item_index++;
				}
			}
			/* port: as goto_next_item, over the PC version's labels */
			if (widget->type == _ui_widget_type_column_list && pc_menu_tag(widget->definition_tag_index))
			{
				long tries = 0;

				while (widget_instance_port_is_label(child) && tries++ < 256)
				{
					child = child->previous;
					item_index--;
					if (!child)
					{
						child = widget->child;
						item_index = 0;
						while (child->next)
						{
							child = child->next;
							item_index++;
						}
					}
				}
			}
			widget_instance_give_focus_by_tag(
				widget,
				child->definition_tag_index,
				widget->local_player_index);
			widget->parameters.list.selected_index = (short)item_index;
		}
	}
	widget->parameters.list.last_list_tab_direction = -15;

	return TRUE;
}

void ui_widgets_close_all(
	void)
{
	long local_player_index;

	/* port: the virtual keyboard goes with the widgets (while the widget
	whose text it edits is still there): left open, it drew on after a game
	loaded, with the menu map's font, which the game's tags no longer have
	(a player typing when the host started the game) */
	if (virtual_keyboard_active())
		virtual_keyboard_close();
	for (local_player_index = 0;
		local_player_index < MAXIMUM_NUMBER_OF_LOCAL_PLAYERS;
		local_player_index++)
	{
		if (widget_globals.active_widgets[local_player_index])
			ui_widget_delete(widget_globals.active_widgets[local_player_index]);
		if (widget_globals.widget_stack[local_player_index])
			dispose_widget_stack(&widget_globals.widget_stack[local_player_index]);
	}
	/* port: Release only an editor opened by MCC after its widgets are gone. */
	{

		mcc_ui_settings_close(NONE);
	}

	return;
}

void ui_widgets_close_all_for_local_player(
	short local_player_index)
{
	long widget_index;

	match_vassert(
		"c:\\halo\\SOURCE\\interface\\ui_widget.c",
		1154,
		local_player_index>=0 && local_player_index<MAXIMUM_NUMBER_OF_LOCAL_PLAYERS,
		"expected a valid local_player_index");
	for (widget_index = 0;
		widget_index < MAXIMUM_NUMBER_OF_LOCAL_PLAYERS;
		widget_index++)
	{
		struct widget_instance *widget = widget_globals.active_widgets[widget_index];

		if (widget && widget->local_player_index == local_player_index)
		{
			ui_widget_delete(widget);
			if (widget_globals.widget_stack[widget_index])
				dispose_widget_stack(&widget_globals.widget_stack[widget_index]);
		}
	}
	/* port: and MCC's pause menu's settings (mcc_ui.c) */
	mcc_ui_settings_close(local_player_index);

	return;
}

void ui_widgets_delete_history(
	void)
{
	long local_player_index;

	for (local_player_index = 0;
		local_player_index < MAXIMUM_NUMBER_OF_LOCAL_PLAYERS;
		local_player_index++)
	{
		if (widget_globals.widget_stack[local_player_index])
			dispose_widget_stack(&widget_globals.widget_stack[local_player_index]);
	}

	return;
}

void ui_widgets_pop_stack(
	short local_player_index)
{
	struct widget_stack_data data;

	if (local_player_index == NONE)
	{
		local_player_index = 0;
	}
	else
	{
		match_assert(
			"c:\\halo\\SOURCE\\interface\\ui_widget.c",
			1204,
			(local_player_index>=0) && (local_player_index<MAXIMUM_NUMBER_OF_LOCAL_PLAYERS));
	}
	if (widget_globals.widget_stack[local_player_index])
		pop_widget(&widget_globals.widget_stack[local_player_index], &data);

	return;
}

void main_screen_shell_begin_fade(
	unsigned long fade_duration_milliseconds)
{
	long local_player_index;

	ui_stop_main_menu_music();
	for (local_player_index = 0;
		local_player_index < MAXIMUM_NUMBER_OF_LOCAL_PLAYERS;
		local_player_index++)
	{
		if (widget_globals.active_widgets[local_player_index] &&
			!widget_globals.active_widgets[local_player_index]->widget_is_error_dialog)
		{
			widget_globals.active_widgets[local_player_index]->auto_close_fade_time = fade_duration_milliseconds;
			widget_globals.active_widgets[local_player_index]->milliseconds_to_auto_close =
				widget_globals.current_system_milliseconds -
				widget_globals.active_widgets[local_player_index]->creation_time + 100;
			if (widget_globals.widget_stack[local_player_index])
				dispose_widget_stack(&widget_globals.widget_stack[local_player_index]);
		}
	}

	return;
}

static void play_sound_tag(
	long sound_tag_index)
{
	if (sound_tag_index != NONE)
		unspatialized_impulse_sound_new(sound_tag_index, 1.0f);

	return;
}

void ui_play_audio_feedback_sound(
	short audio_feedback)
{
	switch (audio_feedback)
	{
	case _ui_audio_feedback_cursor:
		play_sound_tag(tag_loaded(SOUND_DEFINITION_TAG, "sound\\sfx\\ui\\cursor"));
		break;
	case _ui_audio_feedback_forward:
		play_sound_tag(tag_loaded(SOUND_DEFINITION_TAG, "sound\\sfx\\ui\\forward"));
		break;
	case _ui_audio_feedback_back:
		play_sound_tag(tag_loaded(SOUND_DEFINITION_TAG, "sound\\sfx\\ui\\back"));
		break;
	case _ui_audio_feedback_flag_failure:
		play_sound_tag(tag_loaded(SOUND_DEFINITION_TAG, "sound\\sfx\\ui\\flag_failure"));
		break;
	}

	return;
}

static __inline struct widget_instance *widget_instance_get_tail_child_widget(
	struct widget_instance *widget)
{
	struct widget_instance *child = widget->child;

	if (child)
	{
		while (child->next)
			child = child->next;
	}

	return child;
}

static void ui_widget_add_child(
	struct widget_instance *parent,
	struct widget_instance *child)
{
	struct widget_instance *tail_child = NULL;
	struct widget_instance *sibling = parent->child;

	match_assert(
		"c:\\halo\\SOURCE\\interface\\ui_widget.c",
		2719,
		(child->previous == NULL) && (child->next == NULL));
	while (sibling)
	{
		tail_child = sibling;
		sibling = sibling->next;
	}
	if (tail_child)
	{
		match_assert(
			"c:\\halo\\SOURCE\\interface\\ui_widget.c",
			2729,
			tail_child->next == NULL);
		tail_child->next = child;
		child->previous = tail_child;
	}
	else
	{
		parent->child = child;
	}

	return;
}

static void push_widget(
	struct widget_stack_node **top,
	struct widget_stack_data *data)
{
	struct widget_stack_node *node = pool_new_pointer(
		widget_memory_pool,
		sizeof(struct widget_stack_node),
		"c:\\halo\\SOURCE\\interface\\ui_widget.c",
		2532);

	match_assert(
		"c:\\halo\\SOURCE\\interface\\ui_widget.c",
		2534,
		top && data);
	if (node)
	{
		node->data = *data;
		node->next = *top;
		*top = node;
	}
	else
	{
		match_vwarn(
			"c:\\halo\\SOURCE\\interface\\ui_widget.c",
			2544,
			FALSE,
			"out of memory! the UI screen history will be hosed.");
	}

	return;
}

static boolean widget_instance_can_receive_events(
	struct widget_instance *widget)
{
	if (widget->disabled)
		return FALSE;
	if (widget->parent)
	{
		/* NOTE January carries the parent's definition over from one ancestor to
		the next: the flags it tests belong to the ancestor BELOW the one whose
		type it tests, from the second iteration onward.  Reproduced, not
		repaired - the June 2011 rebuild of this function decompiles to the same
		carried-over definition pointer */
		struct ui_widget_definition *definition =
			ui_widget_definition_get(widget->parent->definition_tag_index);
		struct widget_instance *parent;
		boolean result = TRUE;

		for (parent = widget->parent; parent && result; parent = parent->parent)
		{
			struct ui_widget_definition *parent_definition =
				ui_widget_definition_get(parent->definition_tag_index);

			result = TEST_FLAG(definition->flags, _widget_pass_unhandled_events_to_children_bit) ||
				parent->type == _ui_widget_type_spinner_list ||
				parent->type == _ui_widget_type_column_list;
			definition = parent_definition;
		}

		return result;
	}

	return TRUE;
}

static void widget_instance_set_focused_child_by_index(
	long tag_index,
	struct widget_instance *widget,
	short child_index)
{
	struct widget_instance *parent;

	if (tag_index == NONE)
		return;
	parent = widget_instance_find_by_tag_index_recursive(widget, tag_index);
	if (!parent)
		return;
	if (child_index >= 0)
	{
		struct widget_instance *child;
		long index;

		for (child = parent->child, index = 0; child; child = child->next, index++)
		{
			if (child->type == _ui_widget_type_spinner_list &&
				ui_widget_definition_get(child->definition_tag_index)->child_widgets.count > 1)
			{
				return;
			}
			if (index == child_index)
			{
				widget_instance_give_focus_directly(widget, child);
				if (child->parent &&
					(child->parent->type == _ui_widget_type_spinner_list ||
					child->parent->type == _ui_widget_type_column_list))
				{
					child->parent->parameters.list.selected_index = (short)index;
				}

				return;
			}
		}
	}
	else
	{
		/* with no index the found widget takes the focus itself, unless it is a
		multiple-item spinner list, which owns the focus of its own items */
		if (widget_instance_can_receive_events(parent) &&
			!(parent->type == _ui_widget_type_spinner_list &&
			ui_widget_definition_get(parent->definition_tag_index)->child_widgets.count > 1))
		{
			widget_instance_give_focus_directly(widget, parent);
		}
	}

	return;
}

static void widget_instance_go_back_to_previous(
	struct widget_instance *widget)
{
	struct widget_stack_data data;
	short widget_stack = (widget->local_player_index == NONE) ? 0 : widget->local_player_index;
	short previous_local_player_index = NONE;
	long previous_widget_tag;

	if (widget_globals.widget_stack[widget_stack])
	{
		pop_widget(&widget_globals.widget_stack[widget_stack], &data);
		previous_local_player_index = data.local_player_index;
		previous_widget_tag = data.previous_widget_tag;
	}
	else
	{
		previous_widget_tag = NONE;
	}
	ui_widget_delete(widget_instance_get_topmost_parent(widget));
	if (previous_widget_tag != NONE)
	{
		struct widget_instance *new_widget = ui_widget_load_by_name_or_tag(
			NULL,
			previous_widget_tag,
			NULL,
			previous_local_player_index,
			NONE,
			NONE,
			NONE);

		if (new_widget)
		{
			widget_instance_set_focused_child_by_index(
				data.focused_child_parent_widget_tag,
				new_widget,
				data.focused_child_index);
		}
	}

	return;
}

static __inline struct widget_instance *widget_instance_find_by_tag_index(
	long tag_index)
{
	struct widget_instance *result = NULL;
	long widget_index;

	for (widget_index = 0;
		widget_index < MAXIMUM_NUMBER_OF_LOCAL_PLAYERS && !result;
		widget_index++)
	{
		if (widget_globals.active_widgets[widget_index])
		{
			result = widget_instance_find_by_tag_index_recursive(
				widget_globals.active_widgets[widget_index],
				tag_index);
		}
	}

	return result;
}

static void event_handler_dispatch(
	struct widget_instance *widget,
	struct ui_widget_definition *definition,
	struct event_record *event,
	struct ui_widget_event_handler_reference *handler,
	boolean *calling_widget_deleted)
{
	/* port: the widget's tag, named when its function fails (which may have
	deleted the widget) */
	long definition_tag_index = widget->definition_tag_index;
	boolean widget_deleted = FALSE;
	boolean success = TRUE;
	boolean function_failed = FALSE;
	long audio_feedback = _ui_audio_feedback_none;
	boolean close_widget_after = FALSE;
	boolean close_current = FALSE;
	boolean close_all = FALSE;

	if (TEST_FLAG(handler->flags, _event_handler_run_scenario_script_bit) &&
		handler->script[0])
	{
		if (!hs_evaluate_by_name(handler->script))
			error(_error_silent, "failed to run ui widget event script '%s'", handler->script);
	}
	if (TEST_FLAG(handler->flags, _event_handler_run_function_bit) &&
		!widget_deleted &&
		!ui_widget_event_handler_function_invoke(
			widget,
			event,
			handler->function,
			&widget_deleted))
	{
		error(_error_silent, "event handler function %d of %s failed", handler->function,
			tag_get_name(definition_tag_index));
		function_failed = TRUE;
	}
	else
	{
		if (TEST_FLAG(handler->flags, _event_handler_give_focus_to_widget_bit) &&
			!widget_deleted)
		{
			if (handler->widget_tag.index != NONE)
			{
				widget_instance_give_focus_by_tag(
					widget,
					handler->widget_tag.index,
					widget->local_player_index);
				audio_feedback = _ui_audio_feedback_cursor;
			}
			else
			{
				error(
					_error_silent,
					"failed to give focus to a widget because event_handler->ui_widget_tag == NONE");
				success = FALSE;
			}
		}
		if (TEST_FLAG(handler->flags, _event_handler_reload_self_bit) &&
			!widget_deleted)
		{
			widget_instance_reload_recursive(widget);
		}
		if (TEST_FLAG(handler->flags, _event_handler_reload_widget_bit) &&
			!widget_deleted)
		{
			if (handler->widget_tag.index != NONE)
			{
				ui_widget_reload_by_tag(handler->widget_tag.index);
			}
			else
			{
				error(
					_error_silent,
					"failed to reload widget because event_handler->ui_widget_tag == NONE");
				success = FALSE;
			}
		}
		if (TEST_FLAG(handler->flags, _event_handler_close_current_widget_bit) &&
			!widget_deleted)
		{
			close_widget_after = TRUE;
		}
		if (TEST_FLAG(handler->flags, _event_handler_close_other_widget_bit) &&
			!widget_deleted &&
			handler->widget_tag.index != NONE)
		{
			struct widget_instance *other_widget =
				widget_instance_find_by_tag_index(handler->widget_tag.index);

			if (other_widget)
			{
				if (other_widget == widget)
					close_current = TRUE;
				else
					ui_widget_delete(other_widget);
			}
			else
			{
				error(
					_error_silent,
					"failed to close widget because event_handler->ui_widget_tag == NONE");
				success = FALSE;
			}
		}
		if (TEST_FLAG(handler->flags, _event_handler_close_all_widgets_bit) &&
			!widget_deleted)
		{
			close_all = TRUE;
		}
		/* port: not from a widget its function deleted (it went back:
		menu_functions.c's profile_save_changes), which the Xbox's opened
		from regardless */
		/* port: MCC's New Game can close every widget at once. The event ends
		with that deletion, before anything can open from the deleted tree */
		if (TEST_FLAG(handler->flags, _event_handler_open_widget_bit) &&
			!widget_deleted && handler->widget_tag.index != NONE &&
			mcc_ui_new_game(widget, &widget_deleted))
		{
			*calling_widget_deleted = widget_deleted;
			return;
		}
		if (TEST_FLAG(handler->flags, _event_handler_open_widget_bit) &&
			!widget_deleted &&
			handler->widget_tag.index != NONE)
		{
			if (!ui_widget_launch_widget(widget, handler->widget_tag.index))
			{
				error(_error_silent, "event handler failed to spawn widget");
				success = FALSE;
			}
			else
			{
				if (audio_feedback == _ui_audio_feedback_none)
					audio_feedback = _ui_audio_feedback_forward;
				widget_deleted = TRUE;
			}
		}
		if (TEST_FLAG(handler->flags, _event_handler_replace_with_other_widget_bit) &&
			!widget_deleted &&
			handler->widget_tag.index != NONE)
		{
			struct widget_instance *new_widget = ui_widget_load_by_name_or_tag(
				NULL,
				handler->widget_tag.index,
				widget,
				widget->local_player_index,
				NONE,
				NONE,
				NONE);

			if (new_widget)
			{
				struct widget_instance *parent = widget->parent;
				struct widget_instance *next = widget->next;
				struct widget_instance *previous = widget->previous;
				long widget_index;

				if (new_widget->previous)
					new_widget->previous->next = NULL;
				new_widget->previous = NULL;
				new_widget->parent = NULL;
				new_widget->horizontal_offset += widget->horizontal_offset;
				new_widget->vertical_offset += widget->vertical_offset;
				if (parent)
				{
					new_widget->parent = parent;
					if (parent->child == widget)
						parent->child = new_widget;
					if (parent->focused_child == widget)
						parent->focused_child = new_widget;
				}
				if (next)
				{
					match_assert(
						"c:\\halo\\SOURCE\\interface\\ui_widget.c",
						3721,
						next->previous == widget);
					next->previous = new_widget;
				}
				new_widget->next = next;
				if (previous)
				{
					match_assert(
						"c:\\halo\\SOURCE\\interface\\ui_widget.c",
						3728,
						previous->next == widget);
					previous->next = new_widget;
				}
				new_widget->previous = previous;
				for (widget_index = 0;
					widget_index < MAXIMUM_NUMBER_OF_LOCAL_PLAYERS;
					widget_index++)
				{
					if (widget_globals.active_widgets[widget_index] == new_widget)
					{
						widget_globals.active_widgets[widget_index] = NULL;
						break;
					}
				}
				if (audio_feedback == _ui_audio_feedback_none)
					audio_feedback = _ui_audio_feedback_forward;
				widget->previous = NULL;
				widget->next = NULL;
				widget->parent = NULL;
				close_current = TRUE;
			}
			else
			{
				error(
					_error_silent,
					"failed to open widget because the specified widget tag was not found");
				success = FALSE;
			}
		}
		if (TEST_FLAG(handler->flags, _event_handler_go_back_to_previous_widget_bit))
		{
			widget_instance_go_back_to_previous(widget);
			if (audio_feedback == _ui_audio_feedback_none)
				audio_feedback = _ui_audio_feedback_back;
			widget_deleted = TRUE;
		}
		if (handler->sound_effect.index != NONE)
			unspatialized_impulse_sound_new(handler->sound_effect.index, 1.0f);
		if (close_all)
		{
			long widget_index;

			for (widget_index = 0;
				widget_index < MAXIMUM_NUMBER_OF_LOCAL_PLAYERS;
				widget_index++)
			{
				if (widget_globals.active_widgets[widget_index])
					ui_widget_delete(widget_globals.active_widgets[widget_index]);
				while (widget_globals.widget_stack[widget_index])
				{
					struct widget_stack_data data;

					pop_widget(&widget_globals.widget_stack[widget_index], &data);
				}
			}
			widget_deleted = TRUE;
		}
		else if (close_widget_after)
		{
			ui_widget_delete(widget_instance_get_topmost_parent(widget));
			widget_deleted = TRUE;
		}
		else if (close_current)
		{
			ui_widget_delete(widget);
			widget_deleted = TRUE;
		}
	}
	if (!success || function_failed)
	{
		if (TEST_FLAG(handler->flags, _event_handler_look_for_conditional_widget_on_failure_bit))
		{
			long conditional_index;

			for (conditional_index = 0;
				conditional_index < definition->conditional_widgets.count;
				conditional_index++)
			{
				struct ui_widget_conditional_reference *conditional =
					(struct ui_widget_conditional_reference *)definition->conditional_widgets.address +
					conditional_index;

				if (function_failed == TRUE &&
					TEST_FLAG(
						conditional->flags,
						_conditional_widget_load_if_event_handler_function_fails_bit))
				{
					if (!widget_deleted)
					{
						if (conditional->widget_tag.index != NONE)
						{
							if (!ui_widget_launch_widget(widget, conditional->widget_tag.index))
								error(_error_silent, "condition handler failed to spawn widget");
							else
								widget_deleted = TRUE;
						}
					}
					else
					{
						error(
							_error_silent,
							"couldn't load conditional widget because the calling widget was deleted");
					}
				}
			}
		}
	}
	ui_play_audio_feedback_sound(audio_feedback);
	*calling_widget_deleted = widget_deleted;

	return;
}

static boolean ui_widget_load_children_recursive(
	struct widget_instance *widget,
	struct ui_widget_definition *definition)
{
	boolean result = TRUE;
	long child_index;

	if (TEST_FLAG(definition->list_flags, _list_items_generated_from_string_list_tag))
	{
		struct string_list *string_list;
		long string_index;

		match_vassert(
			"c:\\halo\\SOURCE\\interface\\ui_widget.c",
			2592,
			widget->type == _ui_widget_type_spinner_list,
			"_list_items_generated_from_string_list_tag flag should only be set for 1-wide spinner list widgets");
		match_vassert(
			"c:\\halo\\SOURCE\\interface\\ui_widget.c",
			2594,
			definition->child_widgets.count == 0,
			"no child widget references are needed to define list items when generating a list from a string list tag");
		match_vassert(
			"c:\\halo\\SOURCE\\interface\\ui_widget.c",
			2596,
			definition->text_label_string_list.index != NONE,
			"_list_items_generated_from_string_list_tag flag was set but no string list tag was specified");
		string_list = unicode_string_list_definition_get(definition->text_label_string_list.index);
		widget_globals.dont_load_children_recursive = TRUE;
		for (string_index = 0;
			string_index < string_list->strings.count +
				spinner_string_list_extra_count(definition->text_label_string_list.index);
			string_index++)
		{
			struct widget_instance *child = ui_widget_load_by_name_or_tag(
				NULL,
				widget->definition_tag_index,
				widget,
				widget->local_player_index,
				NONE,
				NONE,
				NONE);

			if (!child)
			{
				result = FALSE;
				break;
			}
			ui_widget_add_child(widget, child);
			widget->parameters.list.number_of_items++;
		}
		widget_globals.dont_load_children_recursive = FALSE;
	}
	for (child_index = 0;
		child_index < definition->child_widgets.count;
		child_index++)
	{
		struct ui_widget_child_reference *reference =
			(struct ui_widget_child_reference *)definition->child_widgets.address + child_index;
		short controller_index = widget->local_player_index;

		if (TEST_FLAG(reference->flags, _child_widget_use_custom_controller_index_bit))
		{
			if (reference->custom_controller_index >= 0 &&
				reference->custom_controller_index < MAXIMUM_GAMEPADS)
			{
				controller_index = reference->custom_controller_index;
			}
			else
			{
				error(
					_error_silent,
					"invalid controller index specified for child widget (#%d)",
					reference->custom_controller_index);
			}
		}
		if (reference->widget_tag.index != NONE)
		{
			struct widget_instance *child = ui_widget_load_by_name_or_tag(
				NULL,
				reference->widget_tag.index,
				widget,
				controller_index,
				NONE,
				NONE,
				NONE);

			if (!child)
			{
				result = FALSE;
				break;
			}
			child->horizontal_offset = reference->horizontal_offset + widget->horizontal_offset;
			child->vertical_offset = reference->vertical_offset + widget->vertical_offset;
			ui_widget_add_child(widget, child);
		}
	}
	if (widget->type == _ui_widget_type_column_list &&
		definition->extended_description_widget.index != NONE)
	{
		widget->parameters.list.extended_description = ui_widget_load_by_name_or_tag(
			NULL,
			definition->extended_description_widget.index,
			widget,
			widget->local_player_index,
			NONE,
			NONE,
			NONE);
		if (widget->parameters.list.extended_description)
		{
			if (widget->parameters.list.extended_description->previous)
				widget->parameters.list.extended_description->previous->next = NULL;
			widget->parameters.list.extended_description->previous = NULL;
			widget->parameters.list.extended_description->parent = NULL;
		}
	}
	if (!TEST_FLAG(definition->flags, _widget_dont_focus_a_specific_child_bit))
	{
		boolean focus_a_child = FALSE;

		if (widget->type == _ui_widget_type_spinner_list ||
			widget->type == _ui_widget_type_column_list)
		{
			widget->parameters.list.selected_index = 0;
			widget->parameters.list.last_list_tab_direction = 0;
			focus_a_child = TRUE;
		}
		else if (TEST_FLAG(definition->flags, _widget_pass_unhandled_events_to_children_bit))
		{
			focus_a_child = TRUE;
		}
		if (focus_a_child)
		{
			struct widget_instance *child;
			/* port: the PC version's lists (port/assets/menus) start on their
			first child that takes events, past their labels */
			boolean skip_labels = widget->type == _ui_widget_type_column_list &&
				pc_menu_tag(widget->definition_tag_index);
			short index = 0;

			for (child = widget->child; child; child = child->next, index++)
			{
				if (((widget->type == _ui_widget_type_spinner_list ||
					widget->type == _ui_widget_type_column_list) &&
					(!skip_labels || !widget_instance_port_is_label(child))) ||
					widget_instance_can_handle_events(child))
				{
					widget->focused_child = child;
					if (skip_labels)
						widget->parameters.list.selected_index = index;
					break;
				}
			}
			if (!widget->focused_child && skip_labels)
				widget->focused_child = widget->child;
		}
	}

	return result;
}

static void widget_instance_initialize(
	struct widget_instance *widget,
	struct widget_instance *parent,
	struct ui_widget_definition *definition,
	long tag_index,
	short local_player_index,
	short widget_stack)
{
	long handler_index;

	memset(widget, 0, sizeof(struct widget_instance));
	if (TEST_FLAG(definition->list_flags, _list_items_generated_from_string_list_tag) &&
		parent &&
		tag_index == parent->definition_tag_index)
	{
		widget->type = _ui_widget_type_text_box;
	}
	widget->definition_tag_index = tag_index;
	widget->name = definition->name;
	widget->local_player_index = local_player_index;
	widget->type = definition->type;
	widget->visible = TRUE;
	widget->render_regardless_of_controller_index =
		TEST_FLAG(definition->flags, _widget_render_regardless_of_controller_index_bit);
	/* port: a network co-op game never pauses (it opens the campaign's pause
	screen, which would) */
	widget->pause_game_time = TEST_FLAG(definition->flags, _widget_pause_game_time_bit) &&
		!network_coop_active();
	widget->creation_time = widget_globals.current_system_milliseconds;
	widget->milliseconds_to_auto_close = MAX(definition->milliseconds_to_auto_close, 0);
	widget->auto_close_fade_time = MAX(definition->auto_close_fade_time, 0);
	widget->alpha_modifier = 1.0f;
	widget->parent = parent;
	switch (widget->type)
	{
	case _ui_widget_type_text_box:
		widget->parameters.text_box.string_list_index = NONE;
		break;
	}
	if (definition->background_bitmap.index != NONE)
	{
		widget->animation.number_of_sprite_frames = TAG_BLOCK_GET_ELEMENT(
			&bitmap_group_get(definition->background_bitmap.index)->sequences,
			0,
			struct bitmap_group_sequence)->bitmap_count;
	}
	if (!widget_globals.dont_load_children_recursive)
	{
		if (!ui_widget_load_children_recursive(widget, definition))
			error(_error_silent, "failed to load widget children");
	}
	for (handler_index = 0;
		handler_index < definition->event_handlers.count;
		handler_index++)
	{
		struct ui_widget_event_handler_reference *handler =
			(struct ui_widget_event_handler_reference *)definition->event_handlers.address + handler_index;

		if (handler->event_type == _widget_event_created)
		{
			struct event_record event = {0};
			boolean widget_deleted;

			event.controller_index = widget->local_player_index;
			event_handler_dispatch(widget, definition, &event, handler, &widget_deleted);
		}
	}
	if (!widget->focused_child)
	{
		struct widget_instance *child;

		for (child = widget->child; child; child = child->next)
		{
			if (widget_instance_can_handle_events(child))
				widget_instance_give_focus_directly(widget, child);
		}
	}
	if (widget->pause_game_time == TRUE)
	{
		widget_globals.pause_game_time_count++;
		/* port: not at the main menu, where the clock only runs the menu's
		scene, which stopped behind the screen and started again from its
		beginning as it closed */
		if (!game_time_get_paused() && !we_are_at_the_main_menu)
			game_time_set_paused(TRUE);
		if (!widget_globals.sound_paused && !we_are_at_the_main_menu)
		{
			sound_pause(TRUE);
			widget_globals.sound_paused = TRUE;
		}
	}

	return;
}

/* port: (menu_functions.c's ADD PLAYER screen) the profile spinner among
the widget's children on the first profile that is usable ("player profile
list initialize" starts on player 1's), when the one it is on is not: the
enumeration's high bit (available to its player) and the callback's say;
the profile it is on, NONE if it has none */
long ui_widget_port_spinner_first_available(
	struct widget_instance *widget,
	boolean (*usable)(short controller, long profile))
{
	struct widget_instance *spinner;
	long const *profiles;
	word index;

	for (spinner = widget->child; spinner && spinner->type != _ui_widget_type_spinner_list; spinner = spinner->next)
		;
	if (!spinner || !(profiles = spinner->parameters.list.list_items) || !spinner->parameters.list.number_of_items)
		return NONE;
#define SPINNER_USABLE(profile) ((profile) != NONE && ((profile) & 0x80000000) && 	usable(spinner->local_player_index, (profile)))
	if ((word)spinner->parameters.list.selected_index < spinner->parameters.list.number_of_items &&
		SPINNER_USABLE(profiles[spinner->parameters.list.selected_index]))
	{
		return profiles[spinner->parameters.list.selected_index];
	}
	for (index = 0; index < spinner->parameters.list.number_of_items; index++)
	{
		if (SPINNER_USABLE(profiles[index]))
		{
			spinner->parameters.list.selected_index = (short)index;
			return profiles[index];
		}
	}
#undef SPINNER_USABLE
	return NONE;
}

/* port: the PC version's events that this engine never sends (its custom
activation), for the menus' functions (port/linux/game/menu_functions.c):
runs the widget's handlers for the event, else the first descendant's that
has any (depth first); FALSE if none has */
boolean ui_widget_port_dispatch_event(
	struct widget_instance *widget,
	short event_type,
	short controller_index,
	boolean *deleted)
{
	struct ui_widget_definition *definition = ui_widget_definition_get(widget->definition_tag_index);
	struct widget_instance *child;
	boolean found = FALSE;
	long handler_index;

	/* (deleted: a handler deleted the widget's screen, opening another or
	going back; the callers' widgets are gone with it) */
	*deleted = FALSE;

	for (handler_index = 0; handler_index < definition->event_handlers.count; handler_index++)
	{
		struct ui_widget_event_handler_reference *handler =
			(struct ui_widget_event_handler_reference *)definition->event_handlers.address + handler_index;

		if (handler->event_type == event_type)
		{
			struct event_record event = {0};
			boolean widget_deleted = FALSE;

			event.controller_index = controller_index;
			event_handler_dispatch(widget, definition, &event, handler, &widget_deleted);
			found = TRUE;
			if (widget_deleted)
			{
				*deleted = TRUE;
				return TRUE;
			}
		}
	}
	for (child = widget->child; child && !found; child = child->next)
	{
		found = ui_widget_port_dispatch_event(child, event_type, controller_index, deleted);
		if (*deleted)
			break;
	}
	return found;
}

/* port: the number of a list's focused item: among all its children, but
in the PC version's lists (port/assets/menus) among those that take events,
past their labels, as its game counts them */
short ui_widget_port_list_index(
	struct widget_instance *list_widget)
{
	boolean skip_labels = pc_menu_tag(list_widget->definition_tag_index);
	struct widget_instance *child;
	short index = 0;

	for (child = list_widget->child; child && child != list_widget->focused_child; child = child->next)
	{
		if (!skip_labels || !widget_instance_port_is_label(child))
			index++;
	}
	return index;
}

/* port: back to the screen before, for the menus' functions */
void ui_widget_port_go_back(
	struct widget_instance *widget)
{
	widget_instance_go_back_to_previous(widget);
}

struct widget_instance *ui_widget_load_by_name_or_tag(
	char const *name,
	long tag_index,
	struct widget_instance *parent,
	short local_player_index,
	long invoking_widget_tag,
	long focused_child_parent_widget_tag,
	short focused_child_index)
{
	struct ui_widget_definition *definition;
	struct widget_instance *widget = NULL;
	short widget_stack = (local_player_index == NONE) ? 0 : local_player_index;

	match_assert(
		"c:\\halo\\SOURCE\\interface\\ui_widget.c",
		377,
		widget_globals.initialized);
	match_assert(
		"c:\\halo\\SOURCE\\interface\\ui_widget.c",
		378,
		(name != NULL) || (tag_index != NONE));
	match_assert(
		"c:\\halo\\SOURCE\\interface\\ui_widget.c",
		379,
		(widget_stack>=0) && (widget_stack<MAXIMUM_GAMEPADS));
	if (tag_index == NONE)
		tag_index = tag_loaded(UI_WIDGET_DEFINITION_TAG, name);
	if (tag_index != NONE)
	{
		definition = ui_widget_definition_get(tag_index);
		widget = pool_new_pointer(
			widget_memory_pool,
			sizeof(struct widget_instance),
			"c:\\halo\\SOURCE\\interface\\ui_widget.c",
			395);
		if (widget)
		{
			if (!parent)
			{
				short previous_local_player_index;

				if (widget_globals.active_widgets[widget_stack])
				{
					previous_local_player_index =
						widget_globals.active_widgets[widget_stack]->local_player_index;
					ui_widget_delete(widget_globals.active_widgets[widget_stack]);
				}
				else
				{
					previous_local_player_index = NONE;
				}
				widget_globals.active_widgets[widget_stack] = widget;
				if (invoking_widget_tag != NONE &&
					!TEST_FLAG(
						ui_widget_definition_get(invoking_widget_tag)->flags,
						_widget_dont_push_history_data_bit))
				{
					struct widget_stack_data data;

					data.previous_widget_tag = invoking_widget_tag;
					data.focused_child_parent_widget_tag = focused_child_parent_widget_tag;
					data.focused_child_index = focused_child_index;
					data.local_player_index = previous_local_player_index;
					push_widget(&widget_globals.widget_stack[widget_stack], &data);
				}
			}
			if (local_player_index == NONE)
			{
				switch (definition->controller_index)
				{
				case _widget_controller0:
					local_player_index = 0;
					break;
				case _widget_controller1:
					local_player_index = 1;
					break;
				case _widget_controller2:
					local_player_index = 2;
					break;
				case _widget_controller3:
					local_player_index = 3;
					break;
				case _widget_controller_any:
					local_player_index = NONE;
					break;
				}
			}
			widget_instance_initialize(
				widget,
				parent,
				definition,
				tag_index,
				local_player_index,
				widget_stack);
		}
		else
		{
			error(_error_silent, "failed to create new widget; out of memory!");
		}
	}
	else
	{
		error(_error_silent, "ui_widget_definition tag '%s'/%d not loaded", name, NONE);
	}

	return widget;
}

static void render_state_text(
	rectangle2d *bounds,
	rectangle2d *cursor_bounds,
	wchar_t const *text)
{
	rectangle2d text_bounds;
	short initial_indent = cursor_bounds->x0 - bounds->x0;

	if (initial_indent < 0)
		error(_error_silent, "initial_indent<0 in render_state_text() and was about to explode");
	initial_indent = MAX(0, initial_indent);
	draw_string_set_indents(initial_indent, 0);
	draw_unicode_string_compute_bounds(bounds, text, &text_bounds, cursor_bounds);
	cursor_bounds->x0 -= 3;
	text_bounds.x0 = bounds->x0;
	rasterizer_draw_unicode_string(&text_bounds, NULL, NULL, 0, text);
	bounds->y0 = cursor_bounds->y0;

	return;
}

static void render_state_bitmap(
	rectangle2d *bounds,
	rectangle2d *cursor_bounds,
	pixel32 color,
	struct icon_hud_element_definition *icon)
{
	struct game_globals *game_globals;
	struct game_globals_interface_tag_references *interface_tag_references;
	long bitmap_group_index;
	long frame_index;
	struct bitmap_data const *bitmap;
	real_rectangle2d const *clip;
	real scale;
	point2d point;

	global_scenario_get();
	game_globals = scenario_get_game_globals();
	interface_tag_references = game_globals->interface_tag_references.count
		? TAG_BLOCK_GET_ELEMENT(
			&game_globals->interface_tag_references,
			0,
			struct game_globals_interface_tag_references)
		: NULL;
	bitmap_group_index = interface_tag_references->interface_tag_references[_interface_bitmap_iface_map2].index;
	frame_index = 0;
	bitmap = NULL;
	clip = NULL;
	if (icon->frame_rate)
		frame_index = system_milliseconds() * 30 / 1000 / icon->frame_rate;
	hud_retrieve_bitmap_and_bounding_rect(
		bitmap_group_index,
		icon->sequence_index,
		frame_index,
		&bitmap,
		&clip);
	if (bitmap && _texture_cache_bitmap_get_hardware_format(
		(struct bitmap_data *)bitmap, FALSE, TRUE))
	{
		/* port: an MCC map's icon is drawn as its HUD's are (mcc_hud_draw.c) */
		if (mcc_hud_icon_draw(bitmap_group_index, bitmap, clip, cursor_bounds, color, icon, TRUE))
			return;
		/* port: the sprite from a high-res texture, if it has one */
		bitmap = hud_hires_sprite_bitmap(bitmap, icon->sequence_index);
		scale = hud_globals_get_scale(local_player_count() > 1);
		point.x = (short)(icon->offset.x * scale + cursor_bounds->x0 + 1.0f);
		point.y = (short)(cursor_bounds->y1 - icon->offset.y * scale - 2.0f);
		hud_draw_bitmap_direct(
			bitmap,
			_hud_anchor_bottom_left,
			&point,
			clip,
			scale,
			0.0f,
			TEST_FLAG(icon->flags, _hud_icon_use_color_bit) ? icon->color : color,
			FALSE);
		if (TEST_FLAG(icon->flags, _hud_icon_absolute_width_bit))
			cursor_bounds->x0 = icon->width_offset + point.x;
		else if (clip)
			cursor_bounds->x0 = (short)((clip->x1 - clip->x0) * bitmap->width + icon->width_offset + point.x);
		else
			cursor_bounds->x0 = bitmap->width + icon->width_offset + point.x;
	}

	return;
}

void draw_string_and_hack_in_icons(
	rectangle2d *bounds,
	rectangle2d *clip,
	point2d *cursor_reference,
	short height_adjust,
	wchar_t const *instring,
	boolean ignore_icon_color)
{
	wchar_t *current = string_data;
	rectangle2d cursor_bounds = *bounds;
	unsigned long length;

	/* port: no more than the buffer holds (the text is the map's: a string
	list's or a hud message's; retail strings are up to 398 characters of
	1024). A longer one is cut, said once. */
	for (length = 0; length < NUMBEROF(string_data) - 1 && instring[length]; length++)
		string_data[length] = instring[length];
	string_data[length] = 0;
	if (instring[length])
	{
		static boolean long_string_reported = FALSE;

		if (!long_string_reported)
		{
			long_string_reported = TRUE;
			error(
				_error_silent,
				"string of more than %d characters cut",
				(long)NUMBEROF(string_data) - 1);
		}
	}
	while (current)
	{
		wchar_t *icon_spec = wcschr(current, L'%');
		short icon_type;

		if (!icon_spec)
			break;
		*icon_spec = 0;
		icon_spec++;
		render_state_text(bounds, &cursor_bounds, current);
		current = icon_spec;
		icon_type = get_icon_type(icon_spec);
		if (icon_type == NONE)
		{
			render_state_text(bounds, &cursor_bounds, L"%");
		}
		else
		{
			short remapped_icon_type;
			short icon_index;

			current = icon_spec + wcslen(icon_names[icon_type]);
			/* port: HUD/scoreboard callers pass TRUE; menu styling has its own opt-in. */
			if (ignore_icon_color && keyboard_hud_inline(icon_type, &cursor_bounds))
				continue;
			/* port: optional PC-menu artwork preserves the literal button meaning. */
			if (!ignore_icon_color && menu_button_glyph_inline(icon_type,
				local_player_index_for_draw_string_and_hack_in_icons, &cursor_bounds))
				continue;
			remapped_icon_type = remap_sticks_for_local_player(
				icon_type,
				local_player_index_for_draw_string_and_hack_in_icons);
			icon_index = NONE;
			if (remapped_icon_type > _icon_right_stick)
			{
				if (remapped_icon_type <= _icon_look)
				{
					if (remapped_icon_type <= _icon_accept)
					{
						struct game_input_preferences preferences;

						input_abstraction_get_local_player_preferences(
							local_player_index_for_draw_string_and_hack_in_icons,
							&preferences);
						icon_index = preferences.game_control_to_xbox_buttons[
							button_mappings[remapped_icon_type - _icon_action]];
					}
					else
					{
						icon_index = button_mappings[remapped_icon_type - _icon_action];
						switch (remapped_icon_type)
						{
						case _icon_accept:
							icon_index = _icon_start_button;
							break;
						case _icon_back:
							icon_index = _icon_back_button;
							break;
						case _icon_move:
							icon_index = _icon_left_stick;
							break;
						case _icon_look:
							icon_index = _icon_right_stick;
							break;
						}
					}
				}
				else
				{
					match_assert(
						"c:\\halo\\SOURCE\\interface\\ui_widget.c",
						4341,
						FALSE);
				}
			}
			else
			{
				icon_index = remapped_icon_type;
			}

			{
				struct icon_hud_element_definition *icon = TAG_BLOCK_GET_ELEMENT(
					&hud_globals->messaging.button_icons,
					icon_index,
					struct icon_hud_element_definition);
				byte saved_flags = icon->flags;
				short saved_width_offset = icon->width_offset;
				real_argb_color icon_color;
				real_argb_color text_color;
				long alpha;

				pixel32_to_real_argb_color(icon->color, &icon_color);
				icon->flags &= ~FLAG(_hud_icon_use_color_bit);
				match_assert(
					"c:\\halo\\SOURCE\\interface\\ui_widget.c",
					4362,
					icon_index>=0 && icon_index<NUM_ICONS);
				if (icon_is_special[icon_index])
				{
					icon->flags &= ~FLAG(_hud_icon_absolute_width_bit);
					icon->width_offset = -5;
				}
				draw_string_get_color(&text_color);
				alpha = (long)(text_color.alpha * 255.0f) << 24;
				if (icon->color == 0 || ignore_icon_color)
					icon_color = text_color;
				icon_color.red *= text_color.alpha;
				icon_color.green *= text_color.alpha;
				icon_color.blue *= text_color.alpha;
				render_state_bitmap(
					bounds,
					&cursor_bounds,
					(real_argb_color_to_pixel32(&icon_color) & 0x00ffffff) | alpha,
					icon);
				bounds->x0++;
				icon->flags = saved_flags;
				icon->width_offset = saved_width_offset;
			}
		}
	}
	if (current)
		render_state_text(bounds, &cursor_bounds, current);
	draw_string_set_indents(0, 0);

	return;
}

void ui_start_main_menu_music(
	void)
{
	if (!widget_globals.main_menu_music_active && !main_menu_fade_active())
	{
		long sound_definition_index = tag_loaded(LOOPING_SOUND_DEFINITION_TAG, "sound\\music\\title1\\title1");

		if (sound_definition_index != NONE)
		{
			error(_error_silent, "starting main menu music");
			scripted_looping_sound_start(sound_definition_index, NONE, 1.0f);
			widget_globals.main_menu_music_active = TRUE;
		}
		else
		{
			error(_error_silent, "title music tag not found");
		}
	}

	return;
}

void ui_stop_main_menu_music(
	void)
{
	if (widget_globals.main_menu_music_active == TRUE)
	{
		long sound_definition_index = tag_loaded(LOOPING_SOUND_DEFINITION_TAG, "sound\\music\\title1\\title1");

		if (sound_definition_index != NONE)
		{
			error(_error_silent, "stopping main menu music");
			scripted_looping_sound_stop(sound_definition_index);
		}
		else
		{
			error(_error_silent, "title music tag not found");
		}
		widget_globals.main_menu_music_active = FALSE;
	}

	return;
}

void display_error_deferred(
	short error_code,
	short local_player_index,
	boolean modal,
	boolean pause_game_time)
{
	long index;

	if (local_player_index == NONE)
	{
		index = 0;
	}
	else
	{
		index = local_player_index;
		match_assert(
			"c:\\halo\\SOURCE\\interface\\ui_widget.c",
			2288,
			(index>=0) && (index<MAXIMUM_NUMBER_OF_LOCAL_PLAYERS));
	}
	if (widget_globals.deferred_errors[index].error_code == NONE)
	{
		widget_globals.deferred_errors[index].error_code = error_code;
		widget_globals.deferred_errors[index].local_player_index = local_player_index;
		widget_globals.deferred_errors[index].modal = modal;
		widget_globals.deferred_errors[index].pause_game_time = pause_game_time;
	}
	else
	{
		error(
			_error_silent,
			"there is already a deferred error message for local player %d; ignoring this one",
			index);
	}

	return;
}

/* port: an error message of the port's own text (the maps have only the
Xbox's), in the dialog of an error whose text it takes the place of */
void display_error_text_deferred(
	wchar_t const *text,
	short local_player_index)
{
	short index = local_player_index == NONE ? 0 : local_player_index;

	if (!VALID_INDEX(index, MAXIMUM_NUMBER_OF_LOCAL_PLAYERS) || widget_globals.deferred_errors[index].error_code != NONE)
		return;
	ui_widget_port_error_pending_text = text;
	display_error_deferred(_error_cannot_create_saved_game_file_with_empty_name, local_player_index, TRUE, FALSE);

	return;
}

void display_errors_deferred_until_cinematic_stop(
	void)
{
	short local_player_index;

	match_vassert(
		"c:\\halo\\SOURCE\\interface\\ui_widget.c",
		2367,
		!cinematic_in_progress(),
		"Noooooooooooooooooo!!!");
	for (local_player_index = 0;
		local_player_index < MAXIMUM_NUMBER_OF_LOCAL_PLAYERS;
		local_player_index++)
	{
		struct ui_widget_deferred_cinematic_error *deferred_error =
			&widget_globals.deferred_cinematic_errors[local_player_index];

		if (deferred_error->error_code >= 0 && deferred_error->error_code < NUMBER_OF_ERROR_CODES)
		{
			display_error(
				deferred_error->error_code,
				local_player_index,
				deferred_error->modal,
				deferred_error->pause_game_time);
		}
		deferred_error->error_code = NONE;
	}

	return;
}

boolean ui_widgets_active_for_local_player(
	short local_player_index)
{
	boolean result = FALSE;

	match_vassert(
		"c:\\halo\\SOURCE\\interface\\ui_widget.c",
		1110,
		local_player_index>=0 && local_player_index<MAXIMUM_NUMBER_OF_LOCAL_PLAYERS,
		"expected a valid local_player_index");
	if (widget_globals.initialized)
	{
		long widget_index;

		for (widget_index = 0;
			widget_index < MAXIMUM_NUMBER_OF_LOCAL_PLAYERS;
			widget_index++)
		{
			if (widget_globals.active_widgets[widget_index] &&
				widget_globals.active_widgets[widget_index]->local_player_index == local_player_index)
			{
				result = TRUE;
				break;
			}
		}
	}

	return result;
}

void display_error(
	short error_code,
	short local_player_index,
	boolean modal,
	boolean pause_game_time)
{
	if (cinematic_in_progress())
	{
		if (local_player_index == NONE)
			local_player_index = 0;
		match_assert(
			"c:\\halo\\SOURCE\\interface\\ui_widget.c",
			2077,
			local_player_index>=0 && local_player_index<MAXIMUM_NUMBER_OF_LOCAL_PLAYERS);
		if (widget_globals.deferred_cinematic_errors[local_player_index].error_code == NONE)
		{
			widget_globals.deferred_cinematic_errors[local_player_index].error_code = error_code;
			widget_globals.deferred_cinematic_errors[local_player_index].modal = modal;
			widget_globals.deferred_cinematic_errors[local_player_index].pause_game_time = pause_game_time;
		}
		else
		{
			error(_error_silent, "there is already a deferred-for-cinematic error queued for player #%d; ignoring this one", local_player_index);
		}
	}
	else
	{
		char const *widget_name = NULL;
		short error_local_player_index;
		short local_player_count;
		short widget_stack;
		boolean first_local_player = TRUE;
		struct widget_instance *top_widget;
		long top_widget_tag_index;
		struct widget_instance *widget;
		/* (port: the port's own text for this error, taken whether or not its
		dialog opens, so that it is never another's) */
		wchar_t const *port_text = NULL;

		if (error_code == _error_cannot_create_saved_game_file_with_empty_name)
		{
			port_text = ui_widget_port_error_pending_text;
			ui_widget_port_error_pending_text = NULL;
		}
		if (local_player_index != NONE)
		{
			short index;

			local_player_count = 0;
			error_local_player_index = NONE;
			for (index = local_player_get_next(NONE); index != NONE; index = local_player_get_next(index))
			{
				if (index == local_player_index)
				{
					error_local_player_index = local_player_index;
					if (local_player_count > 0)
						first_local_player = FALSE;
				}
				local_player_count++;
			}
		}
		else
		{
			error_local_player_index = NONE;
			local_player_count = 0;
		}
		if (error_local_player_index == NONE && !we_are_at_the_main_menu)
			local_player_index = NONE;
		switch (local_player_count)
		{
		case 0:
		case 1:
			widget_name = modal
				? "ui\\shell\\error\\error_modal_fullscreen"
				: "ui\\shell\\error\\error_nonmodal_fullscreen";
			break;
		case 2:
			widget_name = modal
				? "ui\\shell\\error\\error_modal_halfscreen"
				: "ui\\shell\\error\\error_nonmodal_halfscreen";
			break;
		case 3:
			if (first_local_player == TRUE)
			{
				widget_name = modal
					? "ui\\shell\\error\\error_modal_halfscreen"
					: "ui\\shell\\error\\error_nonmodal_halfscreen";
			}
			else
			{
				widget_name = modal
					? "ui\\shell\\error\\error_modal_qtrscreen"
					: "ui\\shell\\error\\error_nonmodal_qtrscreen";
			}
			break;
		case 4:
			widget_name = modal
				? "ui\\shell\\error\\error_modal_qtrscreen"
				: "ui\\shell\\error\\error_nonmodal_qtrscreen";
			break;
		default:
			match_vassert(
				"c:\\halo\\SOURCE\\interface\\ui_widget.c",
				2161,
				FALSE,
				"invalid local player count");
			break;
		}
		if (widget_name)
		{
			widget_stack = local_player_index == NONE ? 0 : local_player_index;
			match_assert(
				"c:\\halo\\SOURCE\\interface\\ui_widget.c",
				2168,
				(widget_stack>=0) && (widget_stack<MAXIMUM_NUMBER_OF_LOCAL_PLAYERS));
			if (transition_to_game_in_progress())
			{
				error(_error_silent, "aborting to the main menu root, for safety's sake");
				main_screen_shell_load();
				main_defer_map_map_change();
				ui_widgets_set_fade_value(-1.0f);
			}
			top_widget = widget_globals.active_widgets[widget_stack];
			top_widget_tag_index = top_widget ? top_widget->definition_tag_index : NONE;
			if (top_widget && top_widget->widget_is_error_dialog == TRUE)
			{
				error(_error_silent, "there is already an error message displayed for this local player index");
				widget = NULL;
			}
			else
			{
				widget = ui_widget_load_by_name_or_tag(
					widget_name,
					NONE,
					NULL,
					local_player_index,
					top_widget_tag_index,
					NONE,
					NONE);
			}
			if (widget)
			{
				struct widget_instance *text_box;

				match_vassert(
					"c:\\halo\\SOURCE\\interface\\ui_widget.c",
					2206,
					widget->child && widget->child->child,
					"error screen widget tag not layed out as expected");
				text_box = widget->child->child;
				match_vassert(
					"c:\\halo\\SOURCE\\interface\\ui_widget.c",
					2208,
					text_box->type == _ui_widget_type_text_box,
					"expected a text box widget in the error widget");
				text_box->parameters.text_box.string_list_index = PIN(error_code, 0, NUMBER_OF_ERROR_CODES - 1);
				/* (port: the port's own text in place of the error's) */
				if (port_text)
				{
					ui_widget_port_error_text_box = text_box;
					ui_widget_port_error_text = port_text;
				}
				widget->widget_is_error_dialog = TRUE;
				if (!widget->pause_game_time)
				{
					widget->pause_game_time = pause_game_time;
					if (widget->pause_game_time == TRUE)
					{
						match_vassert(
							"c:\\halo\\SOURCE\\interface\\ui_widget.c",
							2217,
							widget_globals.pause_game_time_count>=0,
							"widget pause counter is out of whack");
						widget_globals.pause_game_time_count++;
						/* port: not at the main menu (above) */
						if (!game_time_get_paused() && !we_are_at_the_main_menu)
							game_time_set_paused(TRUE);
						if (!widget_globals.sound_paused && !we_are_at_the_main_menu)
						{
							sound_pause(TRUE);
							widget_globals.sound_paused = TRUE;
						}
					}
				}
				switch (error_code)
				{
				case _error_controller_unplugged_start_to_continue:
					widget->milliseconds_to_auto_close = 0;
					widget->auto_close_fade_time = 0;
					break;
				case _error_controller_unplugged:
					widget->close_if_local_player_controller_present = TRUE;
					widget->milliseconds_to_auto_close = 0;
					widget->auto_close_fade_time = 0;
					break;
				default:
					widget->close_if_local_player_controller_present = FALSE;
					break;
				}
			}
			else
			{
				error(_error_silent, "failed to display error message");
			}
		}
	}

	return;
}

void display_error_abort_to_dashboard(
	short error_code,
	boolean optional)
{
	char const *widget_name;
	struct widget_instance *widget;

	if (optional == TRUE)
		widget_name = "ui\\shell\\error\\error_abort_to_dashboard";
	else
		widget_name = "ui\\shell\\error\\error_abort_to_dashboard_you_have_no_choice";
	if (!optional)
		ui_widgets_close_all();
	widget = ui_widget_load_by_name_or_tag(widget_name, NONE, NULL, NONE, NONE, NONE, NONE);
	if (widget)
	{
		match_vassert(
			"c:\\halo\\SOURCE\\interface\\ui_widget.c",
			2319,
			widget->type == _ui_widget_type_text_box,
			"expected a text box widget");
		widget->parameters.text_box.string_list_index = error_code;
		widget->widget_is_error_dialog = TRUE;
		dashboard_abort_error = error_code;
	}
	else
	{
		error(_error_silent, "failed to load '%s' widget", widget_name);
	}

	return;
}

void display_error_damaged_media(
	void)
{
	display_error_abort_to_dashboard(_error_media_damaged, FALSE);
	input_frame_end();
	main_loop_of_death();

	return;
}

/* port: whether this machine hosts network co-op on a campaign level */
static boolean ui_network_server_hosting_cooperative(
	void)
{
	struct network_game *game = network_game_get_game();

	return global_network_game_server_get() && game && !game->variant.game_engine_index &&
		custom_edition_maps_level_campaign(game->map.name);
}

void network_game_reset_to_pregame_ui(
	void)
{
	ui_widgets_close_all();
	/* port: An MCC restart retains this round's settings and returns directly to
	 * the ready lobby. The ordinary host map picker pauses its countdown. */
	{

		if (mcc_ui_network_restart_pregame())
		{
			char const *screen = network_game_is_splitscreen_local() ?
				"ui\\shell\\main_menu\\multiplayer_type_select\\split_screen\\pregame\\splitscreen_pregame_wrapper_normal" :
				pc_menus_screen("ui\\shell\\main_menu\\multiplayer_type_select\\connected\\pregame\\connected_pregame_screen");
			if (!ui_widget_load_by_name_or_tag(screen, NONE, NULL, NONE, NONE, NONE, NONE))
				error(_error_silent, "mcc: failed to load restart pregame screen");
			return;
		}
	}
	if (network_game_is_splitscreen_local())
	{
		if (network_game_is_quickstart_local())
		{
			if (!ui_widget_load_by_name_or_tag(
				"ui\\shell\\main_menu\\multiplayer_type_select\\split_screen\\pregame\\splitscreen_pregame_wrapper_normal",
				NONE, NULL, NONE, NONE, NONE, NONE))
			{
				error(_error_silent, "failed to load pregame screen after quickstart match");
			}
		}
		else
		{
			if (!ui_widget_load_by_name_or_tag(
				"ui\\shell\\main_menu\\multiplayer_type_select\\split_screen\\splitscreen_map_select_postgame_wrapper",
				NONE, NULL, NONE, NONE, NONE, NONE))
			{
				error(_error_silent, "failed to load map select postgame screen");
			}
		}
	}
	else
	{
		/* port: network_game_server_port_cooperative_won has already set up a co-op host's next level.
		The host gets the same lobby as a client, and its START NOW starts that level. */
		if (global_network_game_server_get() && !ui_network_server_hosting_cooperative())
		{
			network_game_server_pause_countdown(global_network_game_server_get(), TRUE);
			/* port: with the PC version's menus, theirs (port/linux/game/menu_tags.c) */
			if (!ui_widget_load_by_name_or_tag(
				pc_menus_screen("ui\\shell\\main_menu\\multiplayer_type_select\\connected\\connected_map_select_postgame_wrapper"),
				NONE, NULL, NONE, NONE, NONE, NONE))
			{
				error(_error_silent, "failed to load map select postgame screen");
			}
		}
		else
		{
			if (!ui_widget_load_by_name_or_tag(
				pc_menus_screen("ui\\shell\\main_menu\\multiplayer_type_select\\connected\\pregame\\connected_pregame_screen"),
				NONE, NULL, NONE, NONE, NONE, NONE))
			{
				error(_error_silent, "failed to load networked pregame status screen");
			}
		}
	}

	return;
}

void display_scenario_help(
	short string_index)
{
	char scenario_name[256];

	match_assert(
		"c:\\halo\\SOURCE\\interface\\ui_widget.c",
		2407,
		string_index>=0);
	if (global_scenario_index != NONE)
	{
		char const *widget_name;
		short local_player_index;
		struct widget_instance *widget;

		csstrncpy(scenario_name, tag_get_name(global_scenario_index), sizeof(scenario_name) - 1);
		scenario_name[sizeof(scenario_name) - 1] = 0;
		strlwr(scenario_name);
		if (strstr(scenario_name, "a10"))
			widget_name = "ui\\shell\\solo_game\\player_help\\player_help_screen_a10";
		else if (strstr(scenario_name, "a30"))
			widget_name = "ui\\shell\\solo_game\\player_help\\player_help_screen_a30";
		else if (strstr(scenario_name, "a50"))
			widget_name = "ui\\shell\\solo_game\\player_help\\player_help_screen_a50";
		else if (strstr(scenario_name, "b30"))
			widget_name = "ui\\shell\\solo_game\\player_help\\player_help_screen_b30";
		else if (strstr(scenario_name, "b40"))
			widget_name = "ui\\shell\\solo_game\\player_help\\player_help_screen_b40";
		else if (strstr(scenario_name, "c10"))
			widget_name = "ui\\shell\\solo_game\\player_help\\player_help_screen_c10";
		else if (strstr(scenario_name, "c20"))
			widget_name = "ui\\shell\\solo_game\\player_help\\player_help_screen_c20";
		else if (strstr(scenario_name, "c40"))
			widget_name = "ui\\shell\\solo_game\\player_help\\player_help_screen_c40";
		else if (strstr(scenario_name, "d20"))
			widget_name = "ui\\shell\\solo_game\\player_help\\player_help_screen_d20";
		else if (strstr(scenario_name, "d40"))
			widget_name = "ui\\shell\\solo_game\\player_help\\player_help_screen_d40";
		else
		{
			error(_error_silent, "can't display scenario help; unknown scenario is active '%s'", scenario_name);

			return;
		}
		local_player_index = player_ui_get_single_player_local_player_controller(0);
		widget = ui_widget_load_by_name_or_tag(
			widget_name,
			NONE,
			NULL,
			local_player_index,
			NONE,
			NONE,
			NONE);
		if (widget)
		{
			struct widget_instance *text_box;

			for (text_box = widget->child; text_box; text_box = text_box->next)
			{
				if (text_box->type == _ui_widget_type_text_box)
					break;
			}
			match_vassert(
				"c:\\halo\\SOURCE\\interface\\ui_widget.c",
				2438,
				text_box && text_box->type == _ui_widget_type_text_box,
				"expected text box widget in player help screen");
			text_box->parameters.text_box.string_list_index = string_index;
		}
		else
		{
			error(_error_silent, "failed to load in-game help dialog");
		}
	}
	else
	{
		error(_error_silent, "can't display scenario help because no scenario is loaded");
	}

	return;
}

static boolean transition_to_game_in_progress(
	void)
{
	if (
		we_are_at_the_main_menu &&
		widget_globals.fade_to_black <= 1.0f &&
		widget_globals.fade_to_black >= 0.0f)
	{
		return TRUE;
	}

	return FALSE;
}

static short get_icon_type(
	wchar_t const *string)
{
	short icon_index;

	for (icon_index = 0; icon_index < NUMBEROF(icon_names); icon_index++)
	{
		if (_wcsnicmp(string, icon_names[icon_index], wcslen(icon_names[icon_index])) == 0)
			break;
	}

	if (icon_index == NUMBEROF(icon_names))
		icon_index = NONE;

	return icon_index;
}

static boolean should_flip_sticks_for_local_player(
	short local_player_index)
{
	struct game_input_preferences preferences;

	if (local_player_index == NONE)
		local_player_index = local_player_get_next(NONE);
	memset(&preferences, 0, sizeof(preferences));
	if (local_player_index != NONE)
		input_abstraction_get_local_player_preferences(local_player_index, &preferences);
	switch (preferences.joystick_controls)
	{
	case _joystick_preset_south_paw:
	case _joystick_preset_legacy_south_paw:
		return TRUE;
	}

	return FALSE;
}

short remap_sticks_for_local_player(
	short icon,
	short local_player_index)
{
	switch (icon)
	{
	case _icon_left_stick:
	case _icon_move:
		match_assert(
			"c:\\halo\\SOURCE\\interface\\ui_widget.c",
			4243,
			16 == get_icon_type(L"left-stick"));
		match_assert(
			"c:\\halo\\SOURCE\\interface\\ui_widget.c",
			4244,
			30 == get_icon_type(L"move"));
		icon = should_flip_sticks_for_local_player(local_player_index) ? _icon_right_stick : _icon_left_stick;
		break;
	case _icon_right_stick:
	case _icon_look:
		match_assert(
			"c:\\halo\\SOURCE\\interface\\ui_widget.c",
			4250,
			17 == get_icon_type(L"right-stick"));
		match_assert(
			"c:\\halo\\SOURCE\\interface\\ui_widget.c",
			4251,
			31 == get_icon_type(L"look"));
		icon = should_flip_sticks_for_local_player(local_player_index) ? _icon_left_stick : _icon_right_stick;
		break;
	}

	return icon;
}

static unsigned long __stdcall filesystem_initialization_thread_proc(
	void *input)
{
	widget_globals.filesystem_check_result = saved_game_perform_file_system_checks();
	if (!widget_globals.filesystem_check_result)
	{
		word number_of_profiles = 1;
		long profile_index;

		playlist_profiles_enumerate_available_to_local_player_index(NONE, &number_of_profiles, &profile_index);
		player_profiles_enumerate_available_to_local_player_index(NONE, &number_of_profiles, &profile_index, TRUE);
		player_ui_get_player1_last_used_profile_index();
	}

	return 0;
}

static void perform_filesystem_initialization(
	void)
{
	match_assert(
		"c:\\halo\\SOURCE\\interface\\ui_widget.c",
		5439,
		widget_globals.initialization_thread==NULL);
	error(_error_silent, "begining filesystem checks & saved game file enumeration...");
	ui_widgets_inhibit_processing(TRUE);
	widget_globals.filesystem_check_result = 0;
	if (!create_thread(0, filesystem_initialization_thread_proc, NULL, &widget_globals.initialization_thread))
	{
		error(_error_silent, "failed to spawn thread for filesystem checks - running synchronously!");
		widget_globals.initialization_thread = NULL;
		filesystem_initialization_thread_proc(NULL);
		ui_widgets_inhibit_processing(FALSE);
	}

	return;
}

void main_screen_shell_load(
	void)
{
	boolean load_main_menu = TRUE;

	ui_widgets_inhibit_processing(FALSE);
	if (main_screen_shell_first_load == TRUE)
	{
		char const *command_line = shell_get_command_line();

		if (command_line && _stricmp(command_line, "xdemo") == 0)
		{
			error(_error_silent, "xbox command line= '%s'", command_line);
		}
		else
		{
			bink_playback_start(
				attract_mode_get_localized_movie_path(_bink_intro_movie),
				FLAG(_bink_playback_button_click_stops_movie_bit) |
					FLAG(_bink_playback_prevent_events_to_ui_bit) |
					FLAG(_bink_playback_return_to_main_menu_when_finished_bit) |
					FLAG(_bink_playback_dont_allow_skipping_if_filesystem_check_thread_is_active_bit) |
					FLAG(_bink_playback_eat_up_memory_like_a_goddamn_beaver_bit));
			load_main_menu = FALSE;
			if (!bink_playback_active())
				load_main_menu = TRUE;
		}
		perform_filesystem_initialization();
		input_abstraction_reset_controller_detection_timer();
	}
	if (load_main_menu)
	{
		attract_mode_reset_timer();
		ui_widgets_close_all();
		/* port: the menus' main menu, when they are there (port/linux/game/menu_tags.c) */
		{
			extern char const *pc_menus_root_name(void);

			if (!ui_widget_load_by_name_or_tag(pc_menus_root_name(), NONE, NULL, NONE, NONE, NONE, NONE))
				error(_error_silent, "failed to load main screen shell window");
		}
		if (widget_globals.main_menu_deferred_error_code != NONE)
		{
			display_error(widget_globals.main_menu_deferred_error_code, NONE, TRUE, FALSE);
			widget_globals.main_menu_deferred_error_code = NONE;
		}
		if (!widget_globals.main_menu_music_active)
			ui_start_main_menu_music();
		reset_last_player1_profile_index();
	}
	if (!virtual_keyboard_initialize())
		error(_error_silent, "failed to initialize the virtual keyboard");
	main_screen_shell_first_load = FALSE;

	return;
}

static void widget_instance_reload_recursive(
	struct widget_instance *widget)
{
	return;
}

static void ui_widget_reload_by_tag(
	long tag_index)
{
	return;
}

real_rgb_color get_ui_rgb_white(
	void)
{
	real_rgb_color result;

	result = *global_real_rgb_white;
	result.red = global_ui_white_red;
	result.green = global_ui_white_green;
	result.blue = global_ui_white_blue;

	return result;
}

real_argb_color get_ui_argb_white(
	void)
{
	real_argb_color result;

	result = *global_real_argb_white;
	result.red = global_ui_white_red;
	result.green = global_ui_white_green;
	result.blue = global_ui_white_blue;

	return result;
}

static __inline real widget_instance_get_cumulative_alpha_modifier(
	struct widget_instance *widget)
{
	real alpha_modifier = widget->alpha_modifier;

	widget = widget->parent;
	while (widget)
	{
		alpha_modifier *= widget->alpha_modifier;
		widget = widget->parent;
	}

	return alpha_modifier;
}

static boolean widget_instance_text_box_is_focused(
	struct widget_instance *widget)
{
	struct widget_instance *parent = widget->parent;
	boolean focused = parent ? parent->focused_child == widget : TRUE;

	if (!focused && parent)
	{
		struct widget_instance *ancestor;

		do
		{
			ancestor = parent->parent;
			if (ancestor)
			{
				if (ancestor->focused_child != parent)
					break;
				focused = (ancestor->type == _ui_widget_type_spinner_list ||
					ancestor->type == _ui_widget_type_column_list) &&
					ancestor->focused_child == parent;
			}
			parent = ancestor;
		}
		while (ancestor);
	}

	return focused;
}

static boolean string_has_icons_to_draw(
	wchar_t *string)
{
	match_assert(
		"c:\\halo\\SOURCE\\interface\\ui_widget.c",
		4181,
		string);
	while (string)
	{
		wchar_t *icon_spec = wcschr(string, L'%');

		if (!icon_spec)
			break;
		icon_spec++;
		if (get_icon_type(icon_spec) != NONE)
			return TRUE;
		string = icon_spec;
	}

	return FALSE;
}

static long search_and_replace(
	wchar_t *search,
	wchar_t *replace,
	wchar_t **string)
{
	long replacements = 0;

	if (string && *string)
	{
		wchar_t *buffer = *string;
		long search_length = ustrlen(search);
		long replace_length = ustrlen(replace);
		long length = ustrlen(buffer) + 1;
		long delta;
		wchar_t *match;

		if (replace_length <= search_length)
		{
			delta = search_length - replace_length;
			match = ustrstr(buffer, search);
			if (match)
			{
				do
				{
					replacements++;
					csmemcpy(match, replace, 2 * replace_length);
					if (delta > 0)
					{
						csmemmove(
							&match[replace_length],
							&match[search_length],
							2 * (length - (match - buffer) - replace_length));
						length -= delta;
					}
					match = ustrstr(buffer, search);
				}
				while (match);
			}
		}
		else
		{
			delta = replace_length - search_length;
			for (match = ustrstr(buffer, search);
				match;
				match = ustrstr(&match[search_length], search))
			{
				replacements++;
			}
			if (replacements > 0)
			{
				buffer = pool_resize_pointer(
					widget_memory_pool,
					buffer,
					2 * (delta * replacements + length),
					"c:\\halo\\SOURCE\\interface\\ui_widget.c",
					4994);
				if (!buffer)
				{
					replacements = NONE;
				}
				else
				{
					for (match = ustrstr(buffer, search);
						match;
						match = ustrstr(buffer, search))
					{
						csmemmove(
							&match[replace_length],
							&match[search_length],
							2 * (length - (match - buffer) - search_length));
						csmemcpy(match, replace, 2 * replace_length);
						length += delta;
					}
					*string = buffer;
				}
			}
		}
	}

	return replacements;
}

/* port: the port's own error text (display_error_text_deferred), which may be
longer than a line of its dialog: broken at spaces (the strings' line break,
'\r') where a line would be wider than `width` in the font drawn with.
FALSE when a line is still wider (a word longer than a line). */
static boolean ui_widget_port_text_wrap(
	wchar_t *text,
	rectangle2d const *bounds,
	short width)
{
	long line_start = 0;
	long last_space = NONE;
	long index;
	boolean fits = TRUE;

	for (index = 0; ; index++)
	{
		wchar_t character = text[index];

		if (character == L'\r' || character == L'\n')
		{
			line_start = index + 1;
			last_space = NONE;
			continue;
		}
		if (character == L' ' || character == 0)
		{
			wchar_t line[256];
			long length = MIN(index - line_start, (long)NUMBEROF(line) - 1);
			rectangle2d text_bounds;
			rectangle2d cursor_bounds;

			csmemcpy(line, text + line_start, length * sizeof(wchar_t));
			line[length] = 0;
			draw_unicode_string_compute_bounds(bounds, line, &text_bounds, &cursor_bounds);
			if (cursor_bounds.x0 - bounds->x0 > width && last_space != NONE)
			{
				text[last_space] = L'\r';
				line_start = last_space + 1;
				/* (the line begun, the word just measured: wider alone?) */
				length = MIN(index - line_start, (long)NUMBEROF(line) - 1);
				csmemcpy(line, text + line_start, length * sizeof(wchar_t));
				line[length] = 0;
				draw_unicode_string_compute_bounds(bounds, line, &text_bounds, &cursor_bounds);
			}
			if (cursor_bounds.x0 - bounds->x0 > width)
				fits = FALSE;
			if (character == 0)
				break;
			last_space = index;
		}
	}

	return fits;
}

/* port: port/linux/game/menu_functions.c; optional centered PC join legends. */
boolean menu_functions_glyph_text(struct widget_instance *widget, rectangle2d const *bounds);
/* port: MCC-owned keyboard legend identities, with native controller fallback. */
short menu_functions_glyph_button(struct widget_instance *widget, short native);

static void widget_instance_render_text_box(
	struct widget_instance *widget,
	struct ui_widget_definition *definition,
	rectangle2d *clip_rect,
	point2d offset,
	boolean focus)
{
	wchar_t **text;
	long search_index;
	long font_index;
	short justification;
	real alpha_modifier;
	real color_alpha;
	real_argb_color color;
	rectangle2d bounds;
	rectangle2d clip;

	if (definition->text_label_string_list.index != NONE)
	{
		short string_list_index;
		wchar_t *string;
		unsigned long length;

		if (widget->parameters.text_box.string_list_index == NONE)
			string_list_index = definition->string_list_index;
		else
			string_list_index = widget->parameters.text_box.string_list_index;
		string = widget == ui_widget_port_error_text_box && ui_widget_port_error_text ?
			(wchar_t *)ui_widget_port_error_text :
			unicode_string_list_get_string(definition->text_label_string_list.index, string_list_index);
		/* port: the description of a spinner's extra item */
		if (string_list_index >= SPINNER_EXTRA_DESCRIPTION_BASE &&
			string_list_index < SPINNER_EXTRA_DESCRIPTION_BASE + (short)NUMBEROF(kills_to_win_extra_descriptions))
		{
			string = (wchar_t *)kills_to_win_extra_descriptions[string_list_index - SPINNER_EXTRA_DESCRIPTION_BASE];
		}
		length = ustrlen(string);
		widget->parameters.text_box.text = pool_resize_pointer(
			widget_memory_pool,
			widget->parameters.text_box.text,
			2 * length + 2,
			"c:\\halo\\SOURCE\\interface\\ui_widget.c",
			4421);
		if (widget->parameters.text_box.text)
		{
			csmemcpy(widget->parameters.text_box.text, string, 2 * length);
			widget->parameters.text_box.text[length] = 0;
		}
		else
		{
			widget->parameters.text_box.text = L"<out of memory>";
		}
	}
	text = &widget->parameters.text_box.text;
	if (!*text || !**text)
		return;
	for (search_index = 0;
		search_index < definition->search_and_replace_functions.count;
		search_index++)
	{
		struct ui_widget_search_and_replace_reference *reference =
			(struct ui_widget_search_and_replace_reference *)
				definition->search_and_replace_functions.address +
			search_index;

		if (reference && reference->search_string[0])
		{
			wchar_t search_string[32];
			wchar_t *replace = ui_widget_search_and_replace_invoke(
				widget,
				reference->replace_function);

			search_and_replace(
				ascii_to_wide(
					reference->search_string,
					search_string,
					sizeof(search_string)),
				replace,
				text);
		}
	}
	font_index = definition->text_font.index;
	if (font_index == NONE)
	{
		error(
			_error_silent,
			"failed to render text box widget because the font tag was invalid");

		return;
	}
	justification = definition->justification;
	if (justification < 0 || justification >= NUMBER_OF_TEXT_JUSTIFICATIONS)
	{
		error(
			_error_silent,
			"failed to render text box widget because the justification was invalid");

		return;
	}
	if (!widget->visible)
		return;
	alpha_modifier = widget_instance_get_cumulative_alpha_modifier(widget);
	bounds = definition->bounds;
	clip = clip_rect ? *clip_rect : definition->bounds;
	bounds.x1 += offset.x;
	bounds.y1 += offset.y;
	bounds.x0 += offset.x;
	bounds.y0 += offset.y;
	bounds.x0 += definition->horizontal_offset;
	bounds.y0 += definition->vertical_offset;
	if (focus)
	{
		color = get_ui_argb_white();
		color_alpha = definition->text_color.alpha;
	}
	else
	{
		color = definition->text_color;
		if (1.0f == color.red && 1.0f == color.green && 1.0f == color.blue)
		{
			color = get_ui_argb_white();
			color_alpha = definition->text_color.alpha;
		}
		else
		{
			color_alpha = color.alpha;
		}
	}
	/* port: a colour of the port's for the text (its alpha the box's) */
	pc_menu_text_color(widget, &color);
	color.alpha = alpha_modifier * color_alpha;
	if (TEST_FLAG(definition->text_box_flags, _text_box_flashing_text_bit))
	{
		color.alpha = (((real)cos(
			widget_globals.current_system_milliseconds *
				SECONDS_PER_MILLISECOND * 3.0f) + 1.5f) * 0.4f) * color.alpha;
	}
	draw_string_set_draw_mode(font_index, NONE, justification, 0, &color);
	/* port: the port's own error text, wrapped to its dialog: the box its
	background draws (narrower than the text box; its picture is padded to a
	power of two, so a margin of the text's offset on each side and as much
	again), and as tall as the text box less its offset above and as much
	below (the dialog's footer). Text that would be taller, or wider (a word
	longer than a line), is drawn in the menus' smaller font. */
	if (widget == ui_widget_port_error_text_box && ui_widget_port_error_text)
	{
		struct bitmap_data *background = definition->background_bitmap.index != NONE ?
			bitmap_group_get_bitmap_from_sequence(definition->background_bitmap.index, 0, 0) : NULL;
		short width = (short)(definition->bounds.x1 - definition->bounds.x0);
		short height = (short)(definition->bounds.y1 - definition->bounds.y0 - 2 * definition->vertical_offset);
		wchar_t wrapped[512];
		rectangle2d text_bounds;
		rectangle2d cursor_bounds;

		if (background && background->width > 0 && background->width < width)
			width = background->width;
		width = (short)(width - 4 * definition->horizontal_offset);
		ustrncpy(wrapped, *text, NUMBEROF(wrapped) - 1);
		wrapped[NUMBEROF(wrapped) - 1] = 0;
		if (!ui_widget_port_text_wrap(wrapped, &bounds, width) ||
			(draw_unicode_string_compute_bounds(&bounds, wrapped, &text_bounds, &cursor_bounds),
			cursor_bounds.y1 - bounds.y0 > height))
		{
			long small_font_index = tag_loaded('font', "ui\\small_ui");

			if (small_font_index != NONE)
			{
				font_index = small_font_index;
				draw_string_set_draw_mode(font_index, NONE, justification, 0, &color);
			}
		}
		ui_widget_port_text_wrap(*text, &bounds, width);
	}
	/* port: MCC owns these text boxes; wrap their instance copies to the panel
	 * width without changing the map's objective strings or other menus. */
	{
		if (mcc_pause_text_wrap_needed(widget->definition_tag_index))
		{
			ui_widget_port_text_wrap(*text, &bounds, (short)(bounds.x1 - bounds.x0));
			clip.x0 = MAX(clip.x0, bounds.x0);
			clip.y0 = MAX(clip.y0, bounds.y0);
			clip.x1 = MIN(clip.x1, bounds.x1);
			clip.y1 = MIN(clip.y1, bounds.y1);
		}
	}
	/* port: authored MCC catalog text needs measured wrapping in every
	 * frontend. The MCC module owns formatting and clips to this text box. */
	if (mcc_menu_text_draw(widget->definition_tag_index, widget->parameters.text_box.string_list_index,
		*text, font_index, justification, &color, &bounds, &clip))
		return;
	/* port: the feature may center the identified PC join line as one unit. */
	if (menu_functions_glyph_text(widget, &bounds))
		return;
	if (string_has_icons_to_draw(*text))
		draw_string_and_hack_in_icons(&bounds, &clip, NULL, 0, *text, FALSE);
	else
		rasterizer_draw_unicode_string(&bounds, &clip, NULL, 0, *text);

	return;
}

static void widget_instance_render_spinner_list(
	struct widget_instance *widget,
	struct ui_widget_definition *definition,
	rectangle2d *clip_rect,
	point2d offset,
	boolean focus)
{
	long header_frame_index = 0;
	long footer_frame_index = 0;
	real alpha_modifier = widget_instance_get_cumulative_alpha_modifier(widget);
	short last_list_tab_direction;
	struct bitmap_data *bitmap;
	rectangle2d bounds;

	if (!widget->visible)
		return;
	last_list_tab_direction = widget->parameters.list.last_list_tab_direction;
	if (last_list_tab_direction)
	{
		switch (SIGN(last_list_tab_direction))
		{
		case -1:
			widget->parameters.list.last_list_tab_direction =
				last_list_tab_direction + 1;
			header_frame_index = 1;
			break;

		case 1:
			widget->parameters.list.last_list_tab_direction =
				last_list_tab_direction - 1;
			footer_frame_index = 1;
			break;
		}
	}
	bitmap = bitmap_group_get_bitmap_from_sequence(
		definition->list_header_bitmap.index,
		0,
		header_frame_index);
	if (bitmap)
	{
		struct rasterizer_dynamic_screen_geometry_parameters parameters;
		long alpha = fast_ftol(alpha_modifier * 255.0f);

		csmemset(&parameters, 0, sizeof(parameters));
		bounds = definition->list_header_bounds;
		/* port: a spinner with extra items is wider, to the left, its arrow
		with it */
		if (spinner_string_list_extra_count(definition->text_label_string_list.index))
		{
			bounds.x0 -= SPINNER_EXTRA_WIDTH;
			bounds.x1 -= SPINNER_EXTRA_WIDTH;
		}
		bounds.x0 += offset.x;
		bounds.y0 += offset.y;
		bounds.x1 += offset.x;
		bounds.y1 += offset.y;
		draw_bitmap_in_rect(
			bitmap,
			&bounds,
			&bounds,
			clip_rect,
			(alpha << 24) | 0x00FFFFFF,
			&parameters,
			FALSE);
	}
	bitmap = bitmap_group_get_bitmap_from_sequence(
		definition->list_footer_bitmap.index,
		0,
		footer_frame_index);
	if (bitmap)
	{
		struct rasterizer_dynamic_screen_geometry_parameters parameters;
		long alpha = fast_ftol(alpha_modifier * 255.0f);

		csmemset(&parameters, 0, sizeof(parameters));
		bounds = definition->list_footer_bounds;
		bounds.x0 += offset.x;
		bounds.y0 += offset.y;
		bounds.x1 += offset.x;
		bounds.y1 += offset.y;
		draw_bitmap_in_rect(
			bitmap,
			&bounds,
			&bounds,
			clip_rect,
			(alpha << 24) | 0x00FFFFFF,
			&parameters,
			FALSE);
	}
	if (definition->child_widgets.count == 0)
	{
		wchar_t *item_text;

		if (definition->text_label_string_list.index != NONE)
		{
			short string_index = widget->parameters.list.selected_index;
			wchar_t *string = spinner_string_list_get_string(
				definition->text_label_string_list.index,
				string_index);
			unsigned long length = ustrlen(string);

			item_text = pool_new_pointer(
				widget_memory_pool,
				2 * length + 2,
				"c:\\halo\\SOURCE\\interface\\ui_widget.c",
				4610);
			if (item_text)
			{
				long search_index;

				csmemcpy(item_text, string, 2 * length);
				item_text[length] = 0;
				for (search_index = 0;
					search_index < definition->search_and_replace_functions.count;
					search_index++)
				{
					struct ui_widget_search_and_replace_reference *reference =
						(struct ui_widget_search_and_replace_reference *)
							definition->search_and_replace_functions.address +
						search_index;

					if (reference && reference->search_string[0])
					{
						wchar_t search_string[32];
						wchar_t *replace = ui_widget_search_and_replace_invoke(
							widget,
							reference->replace_function);

						search_and_replace(
							ascii_to_wide(
								reference->search_string,
								search_string,
								sizeof(search_string)),
							replace,
							&item_text);
					}
				}
			}
		}
		else
		{
			item_text = widget->parameters.list.item_text;
		}
		if (item_text)
		{
			if (definition->text_font.index == NONE)
			{
				error(
					_error_silent,
					"failed to render spinner list item because the font tag was invalid");
			}
			else if (definition->justification < 0 ||
				definition->justification >= NUMBER_OF_TEXT_JUSTIFICATIONS)
			{
				error(
					_error_silent,
					"failed to render spinner list item because the justification was invalid");
			}
			else
			{
				real text_alpha_modifier =
					widget_instance_get_cumulative_alpha_modifier(widget);
				real color_alpha;
				real_argb_color color;
				rectangle2d clip = clip_rect ? *clip_rect : definition->bounds;

				bounds = definition->bounds;
				bounds.x1 += offset.x;
				bounds.y1 += offset.y;
				bounds.x0 += offset.x;
				bounds.y0 += offset.y;
				/* port: a spinner with extra items (three digits) is wider, to the
				left */
				if (spinner_string_list_extra_count(definition->text_label_string_list.index))
				{
					bounds.x0 -= SPINNER_EXTRA_WIDTH;
					clip.x0 -= SPINNER_EXTRA_WIDTH;
				}
				if (focus)
				{
					color.alpha = definition->text_color.alpha;
					color.rgb = get_ui_rgb_white();
					color_alpha = color.alpha;
				}
				else
				{
					color = definition->text_color;
					if (1.0f == color.red &&
						1.0f == color.green &&
						1.0f == color.blue)
					{
						color_alpha = definition->text_color.alpha;
					}
					else
					{
						color_alpha = color.alpha;
					}
				}
				color.alpha = text_alpha_modifier * color_alpha;
				if (TEST_FLAG(
						definition->text_box_flags,
						_text_box_flashing_text_bit))
				{
					color.alpha = (((real)sin(
						widget_globals.current_system_milliseconds *
							SECONDS_PER_MILLISECOND * 3.0f) + 1.0f) * 0.5f) *
						color.alpha;
				}
				draw_string_set_draw_mode(
					definition->text_font.index,
					NONE,
					definition->justification,
					0,
					&color);
				rasterizer_draw_unicode_string(&bounds, &clip, NULL, 0, item_text);
			}
		}
		if (definition->text_label_string_list.index != NONE)
			dispose_pointer(widget_memory_pool, item_text);
	}

	return;
}

/* port/linux/game/menu_functions.c's: a text box drawn, at the bounds its
text was drawn in (the lobby's speaker icons) */
void menu_functions_text_box_drawn(struct widget_instance *widget, rectangle2d const *bounds);

/* ---------- the mouse (desktop builds)

The menus were made for a controller: the d-pad moves the focus through a
screen's items and A activates the focused one. With the mouse
(port/linux/include/halo_ui_pointer.h) the item under the pointer takes the
focus, a left click presses A on it (on a spinner list, left or right by the
half clicked), a right click presses B and the wheel the d-pad. The items and
where they are drawn are noted while the menus draw (ui_mouse_note_target),
for the next frame's processing (ui_widgets_process_mouse), which forgets
them: widgets can be deleted from then on. */

#define UI_MOUSE_MAXIMUM_TARGETS 96

enum ui_mouse_target_kind
{
	/* an item the d-pad moves the focus to: a click presses A on it */
	_ui_mouse_target_item,
	/* a list showing one value at a time: a click on either half steps it
	that way */
	_ui_mouse_target_value,
	/* one of the items a list shows side by side (profiles, levels): the
	list steps to it, and a click then presses A */
	_ui_mouse_target_list_slot,
	/* a button's icon and label in a screen's key: a click presses it */
	_ui_mouse_target_button,
	/* the band of a list's slot rows left of its first slot (a list showing
	several items): a click steps the list back by one */
	_ui_mouse_target_list_back,
	/* the band right of its last slot: a click steps it forward by one */
	_ui_mouse_target_list_forward
};

struct ui_mouse_target
{
	struct widget_instance *widget;
	rectangle2d bounds;
	short kind;
	short button_index;
	/* where a value splits into previous / next: the middle of its box
	before ui_mouse_widen_values widens it, so across the middle of the value
	as drawn, where its arrows are, and up and down the middle of its row once
	ui_mouse_merge_setting_rows has given it the row's height. A widened box
	that a neighbour or the screen's edge holds to one side keeps it */
	short split_x;
	short split_y;
	/* a button's parts, kept so its area can be settled once the whole frame's
	buttons are known (ui_mouse_fit_button_targets) */
	rectangle2d icon_bounds;
	struct widget_instance *label;
	rectangle2d label_bounds;
};

static struct ui_mouse_target ui_mouse_targets[UI_MOUSE_MAXIMUM_TARGETS];
static long ui_mouse_target_count = 0;
static boolean ui_mouse_noting_targets = FALSE;
/* the frame's targets are noted and settled (ui_mouse_fit_button_targets,
ui_mouse_merge_setting_rows, ui_mouse_widen_values) by one render only:
render_ui_widgets runs once per player's viewport, and once more for a
mirror (render.c), and a second render would note the same widgets again
and widen the values once more, which moves their split off the arrows.
Cleared with the targets, by ui_widgets_process_mouse */
static boolean ui_mouse_targets_settled = FALSE;

/* The presses the mouse makes, posted one a frame: the event queue keeps
only the latest event posted between two frames (queue_event). What they
lead to (the focus, a list's position) is only known once they have been
processed, so the pointer's next hover or click waits for them. */
#define UI_MOUSE_MAXIMUM_PRESSES 16

static short ui_mouse_presses[UI_MOUSE_MAXIMUM_PRESSES];
static long ui_mouse_press_count = 0;
static boolean ui_mouse_hover_pending = FALSE;
static boolean ui_mouse_click_pending = FALSE;
/* whether the pointer moved the focus last (the d-pad clears it): a list
then doesn't scroll on at its end, which the pointer, moving, would make it
do every frame (menu_functions.c) */
static boolean ui_mouse_focused_last = FALSE;
/* port: whether the pointer passing over a widget moved the focus last, not
a click (or the d-pad): the server browser's game stays chosen
(menu_functions.c) */
static boolean ui_mouse_hovered_last = FALSE;
static short ui_mouse_hover_x, ui_mouse_hover_y;
static short ui_mouse_click_x, ui_mouse_click_y;
/* whether the latest pointer read was the touchscreen: the taller legend
areas, the merged setting rows and the widened values are for a finger only
(render_ui_widgets); the desktop mouse keeps their original areas */
static boolean ui_mouse_pointer_is_touch = FALSE;

/* ---------- the debug view of the targets (debug.touch_targets)

Drawn by render_ui_widgets in the pass that noted the targets: the list is
only valid from the render that noted them to the next
ui_widgets_process_mouse, which clears it. */

#define UI_DEBUG_MARK_MILLISECONDS 3000
#define UI_DEBUG_KEYBOARD_RECTANGLES 96

struct ui_debug_mark
{
	boolean shown;
	short x, y;
	unsigned long time;
};

static struct ui_debug_mark ui_debug_down_mark, ui_debug_tap_mark;
/* the frames ui_widgets_process_mouse has run, and the one the latest tap arrived in */
static long ui_debug_frame, ui_debug_click_frame;

void platform_log(char const *format, ...);
int config_boolean(char const *name);

/* whether debug.touch_targets is on; read once: settings do not change
while the game runs */
static boolean ui_debug_targets_enabled(
	void)
{
	static long enabled = NONE;

	if (enabled == NONE)
		enabled = config_boolean("debug.touch_targets") != 0;

	return enabled != 0;
}

/* outlines a rectangle, 2 menu pixels thick, in menu coordinates; the
caller has the menus' centering offset on (render.c sets
halo_screen_ui_offset(TRUE) around it; adding the offset inside would
double it) */
static void ui_debug_draw_outline(
	rectangle2d const *bounds,
	pixel32 color)
{
	rectangle2d edge;

	edge = *bounds;
	edge.y1 = edge.y0 + 2;
	draw_quad(&edge, color);
	edge = *bounds;
	edge.y0 = edge.y1 - 2;
	draw_quad(&edge, color);
	edge = *bounds;
	edge.x1 = edge.x0 + 2;
	draw_quad(&edge, color);
	edge = *bounds;
	edge.x0 = edge.x1 - 2;
	draw_quad(&edge, color);

	return;
}

/* draws a cross of about 8 menu pixels at a mark that is still within its 3 seconds */
static void ui_debug_draw_mark(
	struct ui_debug_mark const *mark,
	pixel32 color)
{
	rectangle2d arm;

	if (!mark->shown ||
		system_milliseconds() - mark->time > UI_DEBUG_MARK_MILLISECONDS)
	{
		return;
	}
	arm.x0 = mark->x - 4;
	arm.x1 = mark->x + 4;
	arm.y0 = mark->y - 1;
	arm.y1 = mark->y + 1;
	draw_quad(&arm, color);
	arm.x0 = mark->x - 1;
	arm.x1 = mark->x + 1;
	arm.y0 = mark->y - 4;
	arm.y1 = mark->y + 4;
	draw_quad(&arm, color);

	return;
}

/* outlines the targets the pointer code collected this frame and the last
finger-down and tap points; only for the render that notes the targets
(the first player's): a split-screen viewport that is not hit-tested shows
nothing or it would show outlines that no tap uses */
static void ui_debug_draw_targets(
	boolean first_players_render)
{
	long index;

	if (!first_players_render || !ui_debug_targets_enabled())
		return;
	if (virtual_keyboard_active())
	{
		rectangle2d rectangles[UI_DEBUG_KEYBOARD_RECTANGLES];
		long count = virtual_keyboard_target_rectangles(rectangles, UI_DEBUG_KEYBOARD_RECTANGLES);

		for (index = 0; index < count; index++)
			ui_debug_draw_outline(&rectangles[index], 0xc0ffffff);
	}
	else
	{
		for (index = 0; index < ui_mouse_target_count; index++)
		{
			/* in the order of enum ui_mouse_target_kind; the band's two sides
			share orange, which no other kind uses */
			static pixel32 const colors[] = { 0xc000ff00, 0xc04080ff, 0xc0ffff00, 0xc0ff0000, 0xc0ff8000, 0xc0ff8000 };
			short kind = ui_mouse_targets[index].kind;

			ui_debug_draw_outline(&ui_mouse_targets[index].bounds, colors[PIN(kind, 0, 5)]);
		}
	}
	ui_debug_draw_mark(&ui_debug_down_mark, 0xffff00ff);
	ui_debug_draw_mark(&ui_debug_tap_mark, 0xff00ffff);

	return;
}

/* remembers a finger-down or tap point for the debug view, to show it
for 3 seconds */
static void ui_debug_set_mark(
	struct ui_debug_mark *mark,
	short x,
	short y)
{
	mark->shown = TRUE;
	mark->x = x;
	mark->y = y;
	mark->time = system_milliseconds();

	return;
}

static void ui_mouse_press(
	short button_index)
{
	if (ui_mouse_press_count < UI_MOUSE_MAXIMUM_PRESSES)
		ui_mouse_presses[ui_mouse_press_count++] = button_index;

	return;
}

/* the widget's place among its parent's children, or NONE */
static long ui_mouse_child_index(
	struct widget_instance *widget)
{
	struct widget_instance *child;
	long index = 0;

	for (child = widget->parent ? widget->parent->child : NULL; child; child = child->next, index++)
	{
		if (child == widget)
			return index;
	}

	return NONE;
}

static void ui_mouse_list_directions(
	struct widget_instance *widget,
	short *back,
	short *forward);

/* a list that shows several of its items at once */
static boolean ui_mouse_list_shows_several(
	struct widget_instance *widget)
{
	return widget->type == _ui_widget_type_spinner_list &&
		ui_widget_definition_get(widget->definition_tag_index)->child_widgets.count > 1;
}

/* the button a screen key's icon stands for ("a_butn" and so on), or NONE */
static short ui_mouse_key_button(
	struct widget_instance *widget)
{
	static struct
	{
		char const *name;
		short button_index;
	} const keys[] =
	{
		{ "a_butn", _gamepad_analog_button_a },
		{ "b_butn", _gamepad_analog_button_b },
		{ "x_butn", _gamepad_analog_button_x },
		{ "y_butn", _gamepad_analog_button_y },
		{ "black_butn", _gamepad_analog_button_black },
		{ "white_butn", _gamepad_analog_button_white },
		{ "start_butn", _gamepad_binary_button_start },
		{ "back_butn", _gamepad_binary_button_back },
	};
	char const *name = tag_get_name(widget->definition_tag_index);
	char const *leaf = name ? strrchr(name, '\\') : NULL;
	long key_index;

	leaf = leaf ? leaf + 1 : name;
	for (key_index = 0; leaf && key_index < NUMBEROF(keys); key_index++)
	{
		long length = (long)strlen(keys[key_index].name);

		/* and the smaller icons of the dialogs, "a_butn_sm" */
		if (!strncmp(leaf, keys[key_index].name, length) &&
			(leaf[length] == '\0' || leaf[length] == '_'))
		{
			return keys[key_index].button_index;
		}
	}

	return NONE;
}

/* whether the d-pad moves the focus to a widget: an item of a column list,
or a child its parent tabs through */
static boolean ui_mouse_widget_is_item(
	struct widget_instance *widget)
{
	struct widget_instance *parent = widget->parent;
	struct ui_widget_definition *definition;
	long index;

	if (!parent || parent->type == _ui_widget_type_spinner_list)
		return FALSE;
	index = ui_mouse_child_index(widget);
	if (index == NONE || !widget_instance_can_receive_events(widget))
		return FALSE;
	if (parent->type == _ui_widget_type_column_list)
	{
		return !parent->parameters.list.list_items ||
			index < parent->parameters.list.number_of_items;
	}
	definition = ui_widget_definition_get(parent->definition_tag_index);
	if (!TEST_FLAG(definition->flags, _widget_dpad_updown_tabs_thru_children_bit) &&
		!TEST_FLAG(definition->flags, _widget_dpad_leftright_tabs_thru_children_bit))
	{
		return FALSE;
	}
	definition = ui_widget_definition_get(widget->definition_tag_index);

	return definition->event_handlers.count > 0 ||
		TEST_FLAG(definition->flags, _widget_pass_unhandled_events_to_children_bit) ||
		widget->type == _ui_widget_type_spinner_list ||
		widget->type == _ui_widget_type_column_list;
}

/* notes a target that steps a list by one, unless it is empty or the
targets are full */
static void ui_mouse_note_list_step(
	struct widget_instance *widget,
	rectangle2d const *bounds,
	short kind)
{
	struct ui_mouse_target *target;

	if (ui_mouse_target_count >= UI_MOUSE_MAXIMUM_TARGETS ||
		bounds->x0 >= bounds->x1 ||
		bounds->y0 >= bounds->y1)
	{
		return;
	}
	target = &ui_mouse_targets[ui_mouse_target_count++];
	target->widget = widget;
	target->bounds = *bounds;
	target->kind = kind;
	target->button_index = NONE;
	/* as ui_mouse_note_target sets them for any target that is not a
	button: only a value's split and a button's parts are read, and no stale
	field of an earlier frame's target stays in the slot */
	target->split_x = (bounds->x0 + bounds->x1) / 2;
	target->split_y = (bounds->y0 + bounds->y1) / 2;
	target->icon_bounds = *bounds;
	target->label = NULL;
	target->label_bounds = *bounds;

	return;
}

/* notes the band that steps a list showing several items side by side:
left of the first slot (steps back), right of the last (steps forward). A
stock several-items list covers the whole 640x480 screen, so a tap beside
its slots cannot mean "step"; the band is limited to the slots' rows, and a
tap on the title, between slots or under them does nothing. The caller notes
nothing for the list itself, and widget_instance_render_recursive notes the
slots after this; ui_mouse_target_at takes the last target noted under a
point, so a slot would win over the band. A list the d-pad steps up and down
has no side to tap and gets no band */
static void ui_mouse_note_slot_band(
	struct widget_instance *widget,
	struct ui_widget_definition const *definition,
	point2d offset)
{
	struct widget_instance *child;
	rectangle2d bounds;
	rectangle2d slots;
	short back, forward;
	boolean found = FALSE;

	ui_mouse_list_directions(widget, &back, &forward);
	if (back != _widget_event_dpad_left)
		return;
	for (child = widget->child; child; child = child->next)
	{
		rectangle2d slot;

		if (!child->visible)
			continue;
		/* where widget_instance_render_recursive draws the child: the
		list's offset plus the child's own */
		slot = ui_widget_definition_get(child->definition_tag_index)->bounds;
		slot.x0 += offset.x + child->horizontal_offset;
		slot.x1 += offset.x + child->horizontal_offset;
		slot.y0 += offset.y + child->vertical_offset;
		slot.y1 += offset.y + child->vertical_offset;
		if (!found)
		{
			slots = slot;
			found = TRUE;
		}
		else
		{
			slots.x0 = MIN(slots.x0, slot.x0);
			slots.x1 = MAX(slots.x1, slot.x1);
			slots.y0 = MIN(slots.y0, slot.y0);
			slots.y1 = MAX(slots.y1, slot.y1);
		}
	}
	if (!found)
		return;
	bounds.y0 = slots.y0;
	bounds.y1 = slots.y1;
	bounds.x0 = definition->bounds.x0 + offset.x;
	bounds.x1 = slots.x0;
	ui_mouse_note_list_step(widget, &bounds, _ui_mouse_target_list_back);
	bounds.x0 = slots.x1;
	bounds.x1 = definition->bounds.x1 + offset.x;
	ui_mouse_note_list_step(widget, &bounds, _ui_mouse_target_list_forward);

	return;
}

/* notes a widget as a tap target of this frame, if it is one: an item,
a value, a list slot, or a legend button (kept in parts that
ui_mouse_fit_button_targets settles into the tap area); a list showing
several items notes the band beside its slots */
static void ui_mouse_note_target(
	struct widget_instance *widget,
	struct ui_widget_definition const *definition,
	point2d offset)
{
	struct widget_instance *parent = widget->parent;
	struct ui_mouse_target *target;
	rectangle2d bounds = definition->bounds;
	rectangle2d icon_bounds = bounds;
	rectangle2d target_label_bounds = bounds;
	struct widget_instance *target_label = NULL;
	short button_index;
	short kind;

	if (!ui_mouse_noting_targets ||
		ui_mouse_target_count >= UI_MOUSE_MAXIMUM_TARGETS ||
		widget->disabled)
	{
		return;
	}
	bounds.x0 += offset.x;
	bounds.x1 += offset.x;
	bounds.y0 += offset.y;
	bounds.y1 += offset.y;
	button_index = ui_mouse_key_button(widget);
	if (button_index != NONE)
	{
		/* the icon and its label: the text beginning just right of it */
		struct widget_instance *sibling;
		rectangle2d label_bounds = bounds;
		struct widget_instance *label = NULL;
		short best_distance = 17;

		kind = _ui_mouse_target_button;
		for (sibling = parent ? parent->child : NULL; sibling; sibling = sibling->next)
		{
			struct ui_widget_definition *sibling_definition;
			rectangle2d sibling_bounds;
			short distance;

			if (sibling->type != _ui_widget_type_text_box || !sibling->visible)
				continue;
			sibling_definition = ui_widget_definition_get(sibling->definition_tag_index);
			sibling_bounds = sibling_definition->bounds;
			sibling_bounds.x0 += offset.x - widget->horizontal_offset + sibling->horizontal_offset;
			sibling_bounds.x1 += offset.x - widget->horizontal_offset + sibling->horizontal_offset;
			sibling_bounds.y0 += offset.y - widget->vertical_offset + sibling->vertical_offset;
			sibling_bounds.y1 += offset.y - widget->vertical_offset + sibling->vertical_offset;
			distance = (short)ABS(sibling_bounds.x0 - bounds.x1);
			if (distance < best_distance &&
				sibling_bounds.y0 < bounds.y1 && sibling_bounds.y1 > bounds.y0)
			{
				best_distance = distance;
				label_bounds = sibling_bounds;
				label = sibling;
			}
		}
		icon_bounds = bounds;
		target_label = best_distance < 17 ? label : NULL;
		target_label_bounds = label_bounds;
	}
	else if (parent && ui_mouse_list_shows_several(parent))
	{
		if (parent->disabled ||
			ui_mouse_child_index(widget) == NONE ||
			!widget_instance_can_receive_events(parent))
		{
			return;
		}
		kind = _ui_mouse_target_list_slot;
	}
	else if (widget->type == _ui_widget_type_spinner_list)
	{
		if (!widget_instance_can_receive_events(widget))
			return;
		/* a list showing several items is picked through them, and stepped
		by the band beside them */
		if (ui_mouse_list_shows_several(widget))
		{
			ui_mouse_note_slot_band(widget, definition, offset);
			return;
		}
		if (!parent)
			return;
		kind = _ui_mouse_target_value;
	}
	else if (ui_mouse_widget_is_item(widget))
	{
		kind = _ui_mouse_target_item;
	}
	else
	{
		return;
	}
	target = &ui_mouse_targets[ui_mouse_target_count++];
	target->widget = widget;
	target->bounds = bounds;
	target->kind = kind;
	target->button_index = button_index;
	target->split_x = (bounds.x0 + bounds.x1) / 2;
	target->split_y = (bounds.y0 + bounds.y1) / 2;
	target->icon_bounds = icon_bounds;
	target->label = target_label;
	target->label_bounds = target_label_bounds;

	return;
}

/* makes a setting row its value: an item whose only value child is a value
stops being a target, and the value takes the row's height. A tap anywhere
else on the row would press A, which on a setting screen is ACCEPT. The
value keeps its width and left/right half rule. Only the value's parent is
the row; an item holding several values (a panel, a list of spinners) is not
a row and keeps its own area. Only exactly one value merges because growing
several values to the item's height would pile up boxes of which only the
last noted could be tapped. Runs after ui_mouse_fit_button_targets so the
legends are fitted against the whole row: a legend growing up into a row's
label area would press ACCEPT there */
static void ui_mouse_merge_setting_rows(
	void)
{
	long index;
	long other;
	long kept = 0;

	for (index = 0; index < ui_mouse_target_count; index++)
	{
		struct ui_mouse_target *row = &ui_mouse_targets[index];
		struct ui_mouse_target *only_value = NULL;
		long value_count = 0;

		if (row->kind != _ui_mouse_target_item)
			continue;
		for (other = 0; other < ui_mouse_target_count; other++)
		{
			struct ui_mouse_target *value = &ui_mouse_targets[other];

			if (value->kind == _ui_mouse_target_value && value->widget->parent == row->widget)
			{
				only_value = value;
				value_count++;
			}
		}
		if (value_count == 1)
		{
			only_value->bounds.y0 = row->bounds.y0;
			only_value->bounds.y1 = row->bounds.y1;
			row->kind = NONE;
		}
	}
	/* in place, in order: the last target noted wins an overlap */
	for (index = 0; index < ui_mouse_target_count; index++)
	{
		if (ui_mouse_targets[index].kind != NONE)
			ui_mouse_targets[kept++] = ui_mouse_targets[index];
	}
	ui_mouse_target_count = kept;

	return;
}

/* doubles the width of each value (a setting) around its centre: its arrows
are small for a finger. Each half grows outward by half the box's width.
The noted[] snapshot holds the boxes as noted; every measure is against them,
not as already widened, so two neighbours never take each other's room
whatever the order they were noted in. A box is not held to its row (the
last arrow of a row sits near the row's end) but stops at the edge of a
non-button target beside it that it overlaps vertically, at the midpoint of
the gap to another value facing it, and at the menu's drawable area. A box
never shrinks. Where a limit holds one side back, the split between previous
and next stays at the original centre where the arrows are. Runs after
ui_mouse_merge_setting_rows, once the rows have gone, and once a frame */
static void ui_mouse_widen_values(
	void)
{
	rectangle2d noted[UI_MOUSE_MAXIMUM_TARGETS];
	long index;
	long other;
	short screen_width = (short)halo_screen_width();

	for (index = 0; index < ui_mouse_target_count; index++)
		noted[index] = ui_mouse_targets[index].bounds;
	for (index = 0; index < ui_mouse_target_count; index++)
	{
		struct ui_mouse_target *value = &ui_mouse_targets[index];
		rectangle2d const *box = &noted[index];
		short half = (short)((box->x1 - box->x0) / 2);
		short left = (short)(box->x0 - half);
		short right = (short)(box->x1 + half);

		if (value->kind != _ui_mouse_target_value)
			continue;
		value->split_x = (box->x0 + box->x1) / 2;
		value->split_y = (box->y0 + box->y1) / 2;
		left = MAX(left, (short)(-(screen_width - 640) / 2));
		right = MIN(right, (short)(640 + (screen_width - 640) / 2));
		for (other = 0; other < ui_mouse_target_count; other++)
		{
			struct ui_mouse_target const *beside = &ui_mouse_targets[other];
			rectangle2d const *beside_box = &noted[other];
			boolean is_value = beside->kind == _ui_mouse_target_value;

			if (other == index || beside->kind == _ui_mouse_target_button ||
				beside_box->y0 >= box->y1 || beside_box->y1 <= box->y0)
			{
				continue;
			}
			if (beside_box->x1 <= box->x0)
				left = MAX(left, is_value ? (short)((beside_box->x1 + box->x0) / 2) : beside_box->x1);
			else if (beside_box->x0 >= box->x1)
				right = MIN(right, is_value ? (short)((box->x1 + beside_box->x0) / 2) : beside_box->x0);
		}
		value->bounds.x0 = MIN(left, box->x0);
		value->bounds.x1 = MAX(right, box->x1);
	}

	return;
}

/* the right edge of a legend label's text as the game draws it, or NONE
when it cannot be measured (no text, no font, or icons in the string, which
the measuring does not account for) */
static short ui_mouse_label_text_right(
	struct widget_instance *label,
	rectangle2d const *label_bounds)
{
	struct ui_widget_definition *definition = ui_widget_definition_get(label->definition_tag_index);
	wchar_t const *text = label->parameters.text_box.text;
	real_argb_color color = { 1.0f, 1.0f, 1.0f, 1.0f };
	rectangle2d bounds = *label_bounds;
	rectangle2d text_bounds;
	rectangle2d cursor_bounds;

	if (!text || !*text || definition->text_font.index == NONE ||
		definition->justification < 0 || definition->justification >= NUMBER_OF_TEXT_JUSTIFICATIONS ||
		string_has_icons_to_draw(text))
	{
		return NONE;
	}
	/* as widget_instance_render_text_box places the text */
	bounds.x0 += definition->horizontal_offset;
	bounds.y0 += definition->vertical_offset;
	/* this leaves the label's font, justification and a white colour as the
	global draw mode. That is harmless: it runs at the end of the widget
	render, where the menu's own text has already left a menu font there, and
	the menu, HUD, cinematic and terminal draws set their font before they
	draw; the debug texts that set only a colour or format find a menu font
	either way */
	draw_string_set_draw_mode(definition->text_font.index, NONE, definition->justification, 0, &color);
	draw_unicode_string_compute_bounds(&bounds, text, &text_bounds, &cursor_bounds);

	return text_bounds.x1 == SHORT_MIN ? NONE : text_bounds.x1;
}

/* settles the areas of the frame's legend buttons: each reaches its label's
drawn text (the text box can be far wider), stops before the next legend's
icon on its row, and for a touchscreen only grows up to 8 units upward (not
past an item, value or list slot above it) and in the screen's bottom strip
runs down to the screen's edge (not past an item, value or list slot below
it); recomputed from the parts kept at noting, so running again changes nothing */
static void ui_mouse_fit_button_targets(
	void)
{
	long index;
	long other;

	for (index = 0; index < ui_mouse_target_count; index++)
	{
		struct ui_mouse_target *target = &ui_mouse_targets[index];
		short text_right;

		if (target->kind != _ui_mouse_target_button)
			continue;
		target->bounds = target->icon_bounds;
		if (target->label)
		{
			target->bounds.x1 = MAX(target->bounds.x1, target->label_bounds.x1);
			target->bounds.y0 = MIN(target->bounds.y0, target->label_bounds.y0);
			target->bounds.y1 = MAX(target->bounds.y1, target->label_bounds.y1);
			text_right = ui_mouse_label_text_right(target->label, &target->label_bounds);
			if (text_right != NONE)
			{
				target->bounds.x1 = MAX(target->icon_bounds.x1, MIN(target->label_bounds.x1, text_right + 2));
			}
		}
		target->bounds.x0 = target->icon_bounds.x0;
	}
	for (index = 0; index < ui_mouse_target_count; index++)
	{
		struct ui_mouse_target *target = &ui_mouse_targets[index];

		if (target->kind != _ui_mouse_target_button)
			continue;
		for (other = 0; other < ui_mouse_target_count; other++)
		{
			struct ui_mouse_target const *next = &ui_mouse_targets[other];

			if (other != index && next->kind == _ui_mouse_target_button &&
				next->icon_bounds.x0 > target->icon_bounds.x0 &&
				next->bounds.y0 < target->bounds.y1 && next->bounds.y1 > target->bounds.y0 &&
				next->icon_bounds.x0 - 1 < target->bounds.x1)
			{
				target->bounds.x1 = MAX(next->icon_bounds.x0 - 1, target->icon_bounds.x1);
			}
		}
	}
	for (index = 0; index < ui_mouse_target_count; index++)
	{
		struct ui_mouse_target *target = &ui_mouse_targets[index];
		short top;

		if (target->kind != _ui_mouse_target_button || !ui_mouse_pointer_is_touch)
			continue;
		if (target->bounds.y1 >= 400)
		{
			short bottom = 480;

			/* running down to the screen's edge must not take the tap from an
			item, value or list slot that a mod puts under the legend, as
			growing upward must not above it: the legend is noted later, so it
			would win the overlap */
			for (other = 0; other < ui_mouse_target_count; other++)
			{
				struct ui_mouse_target const *below = &ui_mouse_targets[other];

				if (below->kind != _ui_mouse_target_button &&
					below->bounds.y0 >= target->bounds.y1 &&
					below->bounds.x0 < target->bounds.x1 && below->bounds.x1 > target->bounds.x0)
				{
					bottom = MIN(bottom, below->bounds.y0);
				}
			}
			target->bounds.y1 = bottom;
		}
		/* growing upward must not take the tap from an item, value or list
		slot above the legend: those keep their areas, and the legend is noted
		later, so it would win the overlap */
		top = target->bounds.y0 - 8;
		for (other = 0; other < ui_mouse_target_count; other++)
		{
			struct ui_mouse_target const *above = &ui_mouse_targets[other];

			if (above->kind != _ui_mouse_target_button &&
				above->bounds.y1 <= target->bounds.y0 &&
				above->bounds.x0 < target->bounds.x1 && above->bounds.x1 > target->bounds.x0)
			{
				top = MAX(top, above->bounds.y1);
			}
		}
		target->bounds.y0 = top;
	}

	return;
}

/* the target drawn last (so on top, and the innermost) under a point */
static struct ui_mouse_target *ui_mouse_target_at(
	short x,
	short y)
{
	long index;

	for (index = ui_mouse_target_count - 1; index >= 0; index--)
	{
		rectangle2d const *bounds = &ui_mouse_targets[index].bounds;

		if (x >= bounds->x0 && x < bounds->x1 && y >= bounds->y0 && y < bounds->y1)
			return &ui_mouse_targets[index];
	}

	return NULL;
}

static boolean ui_mouse_widget_has_focus(
	struct widget_instance *widget)
{
	for (; widget->parent; widget = widget->parent)
	{
		if (widget->parent->focused_child != widget)
			return FALSE;
	}

	return TRUE;
}

/* moves the focus to the widget as the d-pad would, with the selection of
the column lists on the way (widget_event_function_list_widget_goto_next_item) */
static void ui_mouse_give_focus(
	struct widget_instance *widget)
{
	struct widget_instance *ancestor;

	ui_mouse_focused_last = TRUE;
	if (ui_mouse_widget_has_focus(widget))
		return;
	widget_instance_give_focus_directly(widget_instance_get_topmost_parent(widget), widget);
	for (ancestor = widget; ancestor->parent; ancestor = ancestor->parent)
	{
		if (ancestor->parent->type == _ui_widget_type_column_list)
			ancestor->parent->parameters.list.selected_index = (short)ui_mouse_child_index(ancestor);
	}
	ui_play_audio_feedback_sound(_ui_audio_feedback_cursor);

	return;
}

/* port: whether the pointer moved the menus' focus last, rather than the
d-pad (menu_functions.c's lists) */
boolean ui_widget_port_pointer_focused(
	void)
{
	return ui_mouse_focused_last;
}

/* port: whether the pointer passing over a widget, not a click or the
d-pad, moved the menus' focus last (menu_functions.c's server browser) */
boolean ui_widget_port_hover_focused(
	void)
{
	return ui_mouse_hovered_last;
}

/* the d-pad buttons that step a widget back and forward */
static void ui_mouse_list_directions(
	struct widget_instance *widget,
	short *back,
	short *forward)
{
	struct ui_widget_definition *definition = ui_widget_definition_get(widget->definition_tag_index);

	if (TEST_FLAG(definition->flags, _widget_dpad_leftright_tabs_thru_list_items_bit) ||
		TEST_FLAG(definition->flags, _widget_dpad_leftright_tabs_thru_children_bit))
	{
		*back = _widget_event_dpad_left;
		*forward = _widget_event_dpad_right;
	}
	else
	{
		*back = _widget_event_dpad_up;
		*forward = _widget_event_dpad_down;
	}

	return;
}

/* steps a list that shows several items to the one shown in a slot, as
pressing the d-pad that many times would */
static void ui_mouse_step_list_to_slot(
	struct widget_instance *slot)
{
	struct widget_instance *list = slot->parent;
	long steps;
	short back, forward;

	ui_mouse_give_focus(list);
	if (!list->focused_child)
		return;
	steps = ui_mouse_child_index(slot) - ui_mouse_child_index(list->focused_child);
	ui_mouse_list_directions(list, &back, &forward);
	for (; steps > 0; steps--)
		ui_mouse_press(forward);
	for (; steps < 0; steps++)
		ui_mouse_press(back);

	return;
}

/* the widget the wheel steps: the innermost on the focus's way that the
d-pad steps through; a touch drag skips a list showing one value at a time
(a setting), moving between the rows, so a drag never changes a value; that
takes a tap on one of its halves; the desktop wheel keeps the game's own
d-pad reach and steps a setting's value */
static struct widget_instance *ui_mouse_wheel_widget(
	struct widget_instance *root,
	boolean skip_settings)
{
	struct widget_instance *result = NULL;
	struct widget_instance *widget;

	for (widget = root; widget; widget = widget->focused_child)
	{
		struct ui_widget_definition *definition = ui_widget_definition_get(widget->definition_tag_index);

		if (skip_settings && widget->type == _ui_widget_type_spinner_list && !ui_mouse_list_shows_several(widget))
			continue;
		if (definition->flags & (FLAG(_widget_dpad_updown_tabs_thru_children_bit) |
			FLAG(_widget_dpad_leftright_tabs_thru_children_bit) |
			FLAG(_widget_dpad_updown_tabs_thru_list_items_bit) |
			FLAG(_widget_dpad_leftright_tabs_thru_list_items_bit)))
		{
			result = widget;
		}
	}

	return result;
}

/* whether the focus passes over a child of a list, which gives a press no
place to land; the PC version's lists skip their labels and hidden rows
(widget_instance_port_is_label) at both ends and wrap past them, so
counting one would let a drag step past the last usable row */
static boolean ui_mouse_wheel_skips_child(
	struct widget_instance *list,
	struct widget_instance *child)
{
	return list->type == _ui_widget_type_column_list && pc_menu_tag(list->definition_tag_index) &&
		widget_instance_port_is_label(child);
}

/* how many presses of a d-pad button can step the focus before it would
wrap around to the other end; the game's lists wrap, so a long touch drag
would lap a short list: the drag stops at the ends; the press goes down from
the screen to the first widget on the focus's way that tabs in its direction
(widget_instance_process_one_event_recursive), and the ends are that
widget's; 0 at the end or for a setting (changed by clicks only), NONE when
no widget tabs that way */
static long ui_mouse_wheel_room(
	struct widget_instance *root,
	short button,
	boolean forward)
{
	boolean horizontal = button == _widget_event_dpad_left || button == _widget_event_dpad_right;
	struct widget_instance *widget;

	for (widget = root; widget; widget = widget->focused_child)
	{
		struct ui_widget_definition *definition = ui_widget_definition_get(widget->definition_tag_index);
		boolean list = widget->type == _ui_widget_type_spinner_list || widget->type == _ui_widget_type_column_list;
		struct widget_instance *child;
		long room = 0;

		if (TEST_FLAG(definition->flags, horizontal ?
				_widget_dpad_leftright_tabs_thru_children_bit :
				_widget_dpad_updown_tabs_thru_children_bit) &&
			widget->focused_child)
		{
			/* a child the tab functions would skip gives no press a place to
			land, so only the ones that take events, pass them on, or sit in
			a list count */
			for (child = forward ? widget->focused_child->next : widget->focused_child->previous;
				child;
				child = forward ? child->next : child->previous)
			{
				struct ui_widget_definition *child_definition = ui_widget_definition_get(child->definition_tag_index);

				if ((child_definition->event_handlers.count > 0 ||
					TEST_FLAG(child_definition->flags, _widget_pass_unhandled_events_to_children_bit) ||
					list) &&
					!ui_mouse_wheel_skips_child(widget, child))
				{
					room++;
				}
			}

			return room;
		}
		if (list && TEST_FLAG(definition->flags, horizontal ?
			_widget_dpad_leftright_tabs_thru_list_items_bit :
			_widget_dpad_updown_tabs_thru_list_items_bit))
		{
			/* a setting is changed by clicks only */
			if (widget->type == _ui_widget_type_spinner_list && !ui_mouse_list_shows_several(widget))
				return 0;
			if (widget->parameters.list.list_items && widget->parameters.list.number_of_items > 0)
			{
				room = forward ?
					widget->parameters.list.number_of_items - 1 - widget->parameters.list.selected_index :
					widget->parameters.list.selected_index;
			}
			else if (widget->focused_child)
			{
				/* a list of the tag's children, which the focus walks; the
				focus skips a disabled child to the next that takes events, or
				wraps (widget_instance_give_focus_directly): the main menu ends
				with its hidden "game demos" */
				for (child = forward ? widget->focused_child->next : widget->focused_child->previous;
					child;
					child = forward ? child->next : child->previous)
				{
					if (!child->disabled && !ui_mouse_wheel_skips_child(widget, child))
						room++;
				}
				if (forward && widget->parameters.list.number_of_items > 0)
				{
					room = MIN(room, widget->parameters.list.number_of_items - 1 -
						widget->parameters.list.selected_index);
				}
			}

			return MAX(room, 0);
		}
	}

	return NONE;
}

/* the menu the mouse drives: the first player's, or everyone's */
static struct widget_instance *ui_mouse_menu(
	void)
{
	long widget_index;

	for (widget_index = 0; widget_index < MAXIMUM_NUMBER_OF_LOCAL_PLAYERS; widget_index++)
	{
		struct widget_instance *widget = widget_globals.active_widgets[widget_index];

		if (widget && (widget->local_player_index == NONE || widget->local_player_index == 0))
			return widget;
	}

	return NULL;
}

static boolean ui_mouse_menus_active(
	void)
{
	if (widget_globals.initialization_thread || progress_bar_is_active())
		return FALSE;

	/* (and the scores after a game, which take A and B like the menus:
	game_engine_update_non_deterministic) */
	return virtual_keyboard_active() || ui_mouse_menu() != NULL || game_engine_showing_postgame();
}

/* port: a row of a list to choose from (the PC version's menus' lists of
gametypes, maps, profiles, levels and games, and the port's of mods:
port/assets/menus) */
static boolean ui_mouse_selection_row(
	struct widget_instance *widget)
{
	return widget && widget->parent && pc_menu_tag(widget->parent->definition_tag_index) &&
		(!strncmp(widget->name, "list_item_", 10) || !strncmp(widget->name, "server_item_", 12) ||
			!strncmp(widget->name, "op_mod_", 7));
}

/* logs a tap the menus resolved: where it was and the target the click acts
on; a click waits for the presses the mouse queued, so it is resolved
against the targets of a later frame than the one the tap arrived in; the
frames between are logged so the target can be read against the screen that
showed then; a value's line also gives the point where it splits into
previous and next */
static void ui_debug_log_click(
	short x,
	short y,
	struct ui_mouse_target const *target,
	long frames)
{
	/* in the order of enum ui_mouse_target_kind */
	static char const *const kinds[] = { "item", "value", "list slot", "button", "list back", "list forward" };

	if (target)
	{
		char const *kind = kinds[PIN(target->kind, 0, 5)];
		char const *name = tag_get_name(target->widget->definition_tag_index);

		/* a value steps by the side of its split the tap is on, and once
		widened the split is not the middle of the logged box
		(ui_mouse_widen_values) */
		if (target->kind == _ui_mouse_target_value)
		{
			platform_log("touch targets: tap at %d,%d hit %s %s [%d,%d,%d,%d] split %d,%d after %ld frames", x, y,
				kind, name, target->bounds.x0, target->bounds.y0, target->bounds.x1, target->bounds.y1,
				target->split_x, target->split_y, frames);
		}
		else
		{
			platform_log("touch targets: tap at %d,%d hit %s %s [%d,%d,%d,%d] after %ld frames", x, y,
				kind, name, target->bounds.x0, target->bounds.y0, target->bounds.x1, target->bounds.y1, frames);
		}
	}
	else
	{
		platform_log("touch targets: tap at %d,%d hit none after %ld frames", x, y, frames);
	}

	return;
}

/* port: a press the menus post from their updates (menu_functions.c: the
server browser's join, once its game is reached), posted where the mouse's
are: one posted while the widgets update or draw would be overwritten by the
next frame's events (queue_event keeps the latest) */
static short ui_widget_port_press_controller = NONE;
static short ui_widget_port_press_button;

void ui_widget_port_post_button(
	short controller_index,
	short button_index)
{
	ui_widget_port_press_controller = controller_index;
	ui_widget_port_press_button = button_index;

	return;
}

/* logs a tap while the virtual keyboard is up: the key or legend that took it */
static void ui_debug_log_keyboard_tap(
	short x,
	short y,
	long hit)
{
	rectangle2d rectangles[UI_DEBUG_KEYBOARD_RECTANGLES];
	long count = virtual_keyboard_target_rectangles(rectangles, UI_DEBUG_KEYBOARD_RECTANGLES);

	if (hit == NONE || hit >= count)
	{
		platform_log("touch targets: tap at %d,%d hit no keyboard key", x, y);
	}
	else
	{
		platform_log("touch targets: tap at %d,%d hit keyboard %s %ld [%d,%d,%d,%d]", x, y,
			hit == count - 2 ? "BACK legend" : hit == count - 1 ? "ENTER legend" : "key",
			hit, rectangles[hit].x0, rectangles[hit].y0, rectangles[hit].x1, rectangles[hit].y1);
	}

	return;
}

/* turns the pointer's motion, clicks and wheel since the last frame into the
first player's controller events; while the virtual keyboard is up it gets
the clicks and the menu behind gets nothing; a touch drag stops at the list's
ends and skips settings; the desktop wheel wraps as the d-pad does; it
records whether the pointer read is the touchscreen because the legends'
taller tap areas are for a finger only; with debug.touch_targets on it also
notes the finger-down and tap points for the debug view, and logs each tap: a
menu click where it is resolved, a keyboard click where it is taken; a click
on the band beside a list's slots steps the list by one; a PC list's row is
selected by its first click and used by a click on the selected row; it
forgets the frame's targets at the end, so that the next frame's render notes
and settles them anew */
static void ui_widgets_process_mouse(
	void)
{
	struct halo_ui_pointer pointer;
	struct ui_mouse_target *target;
	short controller_index = 0;
	boolean pointer_active;
	boolean keyboard_active;

	ui_debug_frame++;
	pointer_active = halo_ui_pointer_update(ui_mouse_menus_active(), &pointer) != 0;
	if (pointer_active)
		ui_mouse_pointer_is_touch = pointer.touch != 0;
	if (pointer_active && ui_debug_targets_enabled())
	{
		if (pointer.downs)
			ui_debug_set_mark(&ui_debug_down_mark, pointer.down_x, pointer.down_y);
		if (pointer.left_clicks)
		{
			ui_debug_set_mark(&ui_debug_tap_mark, pointer.click_x, pointer.click_y);
			ui_debug_click_frame = ui_debug_frame;
		}
	}
	/* the virtual keyboard takes the pointer's clicks itself; a click that
	closes it (Done) is not also a click on the menu behind */
	keyboard_active = virtual_keyboard_active();
	if (pointer_active && keyboard_active && pointer.left_clicks)
	{
		long hit;

		virtual_keyboard_click(pointer.click_x, pointer.click_y, &hit);
		if (ui_debug_targets_enabled())
			ui_debug_log_keyboard_tap(pointer.click_x, pointer.click_y, hit);
	}
	if (!pointer_active || keyboard_active)
	{
		ui_mouse_press_count = 0;
		ui_mouse_hover_pending = FALSE;
		ui_mouse_click_pending = FALSE;
	}
	else
	{
		if (pointer.moved)
		{
			ui_mouse_hover_pending = TRUE;
			ui_mouse_hover_x = pointer.x;
			ui_mouse_hover_y = pointer.y;
		}
		/* (a click on the carnage report's touchscreen buttons is the
		report's: touch_postgame.c) */
		if (pointer.left_clicks &&
			(!game_engine_showing_postgame() || !touch_postgame_click(pointer.click_x, pointer.click_y)))
		{
			ui_mouse_click_pending = TRUE;
			ui_mouse_click_x = pointer.click_x;
			ui_mouse_click_y = pointer.click_y;
		}
		if (pointer.right_clicks)
			ui_mouse_press(_widget_event_b_button);
		if (pointer.wheel_steps && ui_mouse_menu())
		{
			boolean touch = pointer.touch != 0;
			struct widget_instance *widget = ui_mouse_wheel_widget(ui_mouse_menu(), touch);
			boolean going_forward = pointer.wheel_steps < 0;
			short back = _widget_event_dpad_up, forward = _widget_event_dpad_down, button;
			long room = NONE;
			long index;
			long step;

			if (widget)
				ui_mouse_list_directions(widget, &back, &forward);
			/* a sideways PC widget (typically a screen's button bar) steps
			sideways only, and the d-pad's up and down pass over it to a widget
			above it on the focus's way, so a touch drag or the wheel leaves it
			the same way (the wheel, past a list's last row onto the bar, went
			back and forth between its buttons) */
			if (widget && back == _widget_event_dpad_left && pc_menu_tag(widget->definition_tag_index) &&
				ui_mouse_wheel_room(ui_mouse_menu(), _widget_event_dpad_up, FALSE) != NONE)
			{
				back = _widget_event_dpad_up;
				forward = _widget_event_dpad_down;
			}
			button = going_forward ? forward : back;
			/* only a touch drag stops at the ends, and does nothing with no list
			to step; the desktop wheel wraps as the d-pad does, and falls back
			to up and down */
			if (touch)
			{
				room = widget ? ui_mouse_wheel_room(ui_mouse_menu(), button, going_forward) : 0;
				/* the presses still waiting step the list first */
				for (index = 0; widget && room != NONE && index < ui_mouse_press_count; index++)
				{
					if (ui_mouse_presses[index] == button)
						room = MAX(room - 1, 0);
					else if (ui_mouse_presses[index] == (going_forward ? back : forward))
						room++;
				}
			}
			for (step = 0; step < ABS(pointer.wheel_steps) && step < 4 && (room == NONE || step < room); step++)
				ui_mouse_press(button);
		}
		if (!ui_mouse_press_count && ui_mouse_hover_pending)
		{
			ui_mouse_hover_pending = FALSE;
			target = ui_mouse_target_at(ui_mouse_hover_x, ui_mouse_hover_y);
			if (target)
			{
				switch (target->kind)
				{
				case _ui_mouse_target_item:
				case _ui_mouse_target_value:
					/* (a selection list's row is chosen by a click, not
					by passing over it on the way to its buttons) */
					if (!ui_mouse_selection_row(target->widget))
					{
						ui_mouse_give_focus(target->widget);
						/* port: (ui_mouse_hovered_last) */
						ui_mouse_hovered_last = TRUE;
					}
					break;
				case _ui_mouse_target_list_slot:
					ui_mouse_step_list_to_slot(target->widget);
					break;
				/* stepping is a click's action: hovering a band does nothing, so
				that passing a finger over a list's side never moves its focus or
				its items */
				case _ui_mouse_target_list_back:
				case _ui_mouse_target_list_forward:
					break;
				}
			}
		}
		if (!ui_mouse_press_count && ui_mouse_click_pending)
		{
			ui_mouse_click_pending = FALSE;
			/* port: (ui_mouse_hovered_last) */
			ui_mouse_hovered_last = FALSE;
			target = ui_mouse_target_at(ui_mouse_click_x, ui_mouse_click_y);
			if (ui_debug_targets_enabled())
				ui_debug_log_click(ui_mouse_click_x, ui_mouse_click_y, target, ui_debug_frame - ui_debug_click_frame);
			if (target)
			{
				switch (target->kind)
				{
				case _ui_mouse_target_item:
					/* (a selection list's row: chosen by a click, used by
					a click on it chosen) */
					if (ui_mouse_selection_row(target->widget) && target->widget->parent &&
						target->widget->parent->focused_child != target->widget)
					{
						ui_mouse_give_focus(target->widget);
						break;
					}
					ui_mouse_give_focus(target->widget);
					ui_mouse_press(_gamepad_analog_button_a);
					break;
				case _ui_mouse_target_value:
				{
					short back, forward;
					boolean first_half;

					ui_mouse_give_focus(target->widget);
					ui_mouse_list_directions(target->widget, &back, &forward);
					first_half = back == _widget_event_dpad_left ?
						ui_mouse_click_x < target->split_x :
						ui_mouse_click_y < target->split_y;
					ui_mouse_press(first_half ? back : forward);
					break;
				}
				case _ui_mouse_target_list_slot:
					ui_mouse_step_list_to_slot(target->widget);
					ui_mouse_press(_gamepad_analog_button_a);
					break;
				case _ui_mouse_target_button:
					ui_mouse_press(target->button_index);
					break;
				case _ui_mouse_target_list_back:
				case _ui_mouse_target_list_forward:
				{
					short back, forward;

					ui_mouse_give_focus(target->widget);
					/* as ui_mouse_step_list_to_slot: a list with no focused item
					(empty, or one the focus left, which cleared its focused_child)
					has no item to step from, so the first tap on its band only
					gives it the focus, and a d-pad press would move the focus off
					the list instead */
					if (!target->widget->focused_child)
						break;
					ui_mouse_list_directions(target->widget, &back, &forward);
					ui_mouse_press(target->kind == _ui_mouse_target_list_back ? back : forward);
					break;
				}
				}
			}
			else
			{
				long index;

				/* a screen with nothing to pick (a message to dismiss): the
				click is its A */
				for (index = 0; index < ui_mouse_target_count; index++)
				{
					if (ui_mouse_targets[index].kind != _ui_mouse_target_button)
						break;
				}
				if (index == ui_mouse_target_count)
					ui_mouse_press(_gamepad_analog_button_a);
			}
		}
		if (ui_mouse_press_count)
		{
			event_manager_post_button(controller_index, ui_mouse_presses[0]);
			csmemmove(ui_mouse_presses, ui_mouse_presses + 1, --ui_mouse_press_count * sizeof(ui_mouse_presses[0]));
		}
	}
	ui_mouse_target_count = 0;
	ui_mouse_targets_settled = FALSE;

	return;
}

/* port: port/linux/game/menu_functions.c; optional PC legend spacing. */
short menu_functions_glyph_offset(struct widget_instance *widget);

static void widget_instance_render_recursive(
	struct widget_instance *widget,
	rectangle2d *clip_rect,
	point2d offset,
	boolean focus,
	boolean use_nifty_plasma_fx)
{
	struct ui_widget_definition *definition =
		ui_widget_definition_get(widget->definition_tag_index);
	real alpha_modifier = widget_instance_get_cumulative_alpha_modifier(widget);
	boolean render_children = TRUE;
	long input_index;
	struct widget_instance *child;
	struct bitmap_data *bitmap;
	/* port: (custom_edition_maps_picture) */
	struct bitmap_data *custom_edition_picture;
	short frame_index;

	short mcc_picture_index; /* port: MCC owns its preview cache. */

	if (!use_nifty_plasma_fx &&
		TEST_FLAG(definition->flags, _widget_always_render_with_nifty_fx_bit))
	{
		use_nifty_plasma_fx = TRUE;
	}
	offset.x += widget->horizontal_offset;
	/* port: move the complete PC legend group, including its mouse target. */
	offset.x += menu_functions_glyph_offset(widget);
	offset.y += widget->vertical_offset;
	for (input_index = 0;
		input_index < definition->game_data_inputs.count;
		input_index++)
	{
		struct ui_widget_game_data_input_reference *input =
			(struct ui_widget_game_data_input_reference *)
				definition->game_data_inputs.address +
			input_index;

		ui_widget_game_data_function_invoke(widget, input->function);
	}
	if (!widget->visible)
		return;
	ui_mouse_note_target(widget, definition, offset);
	/* port: a Custom Edition map's picture, drawn over the whole widget, or
	the unknown level's frame for a map without one
	(port/linux/game/custom_edition_maps.c) */
	frame_index = widget->animation.current_frame_index;
	/* port: MCC display indices never enter the CE provider. A failed MCC
	 * preview retains the existing unknown frame in every menu style. */
	mcc_picture_index = mcc_menu_preview_index(definition->background_bitmap.index, frame_index);
	if (mcc_picture_index != NONE)
	{
		frame_index = 13;
		custom_edition_picture = NULL;
	}
	else
		custom_edition_picture = custom_edition_maps_picture(definition->background_bitmap.index, &frame_index);
	bitmap = custom_edition_picture ? custom_edition_picture : bitmap_group_get_bitmap_from_sequence(
		definition->background_bitmap.index,
		0,
		frame_index);
	if (bitmap)
	{
		real alpha = alpha_modifier;
		rectangle2d bounds = definition->bounds;
		rectangle2d *clip = clip_rect;
		rectangle2d local_clip;
		pixel32 color;
		struct rasterizer_dynamic_screen_geometry_parameters multitexture_params;
		struct bitmap_group *bitmap_group =
			bitmap_group_get(definition->background_bitmap.index);
		struct bitmap_group_sequence *sequence = TAG_BLOCK_GET_ELEMENT(
			&bitmap_group->sequences,
			0,
			struct bitmap_group_sequence);
		/* port: a widget whose bounds cover the whole 640x480 design space (the
		 * pause menu's dim, for example) should cover the whole screen too,
		 * not just the centered 640 columns -- same as the fade_to_black
		 * quad in render_ui_widgets(). Both the bounds and the clip are
		 * widened symmetrically below, before the centering offset is
		 * added. Only flat fills (the dims, and the menus' vertical
		 * gradient, at most 16 texels wide): a picture is drawn texel for
		 * texel, so wider bounds would shift it (the loading screen). */
		/* (a split screen's too: the pause menu's dim of a half screen,
		640x240, and of a quarter, 320x240, whose right edge is already the
		quarter's: the 640 columns are centred in the screen) */
		boolean widen_to_screen =
			bounds.x0 <= 0 && bounds.y0 <= 0 &&
			bounds.x1 >= 320 && bounds.y1 >= 240 &&
			bitmap->width <= 16 &&
			halo_screen_width() > 640;
		/* (the view's margins: the centring shift, on either side of the
		640 columns, or of a quarter's 320; declared here, before the first
		statement: C89) */
		long extra = halo_screen_ui_offset_current();
		short design_right = bounds.x1;

		if (use_nifty_plasma_fx)
		{
			ui_plasma_effect_color.alpha = 0.0f;
			ui_plasma_effect_color.red = 0.05f;
			ui_plasma_effect_color.green = 0.05f;
			ui_plasma_effect_color.blue = 0.05f;
		}
		if (widen_to_screen)
		{
			bounds.x0 -= (short)extra;
			bounds.x1 = (short)(bounds.x1 + extra);
		}
		bounds.x0 += offset.x;
		bounds.x1 += offset.x;
		bounds.y0 += offset.y;
		bounds.y1 += offset.y;
		if (clip)
		{
			local_clip = *clip;
			clip = &local_clip;
			clip->x0 += offset.x;
			clip->x1 += offset.x;
			clip->y0 += offset.y;
			clip->y1 += offset.y;
		}
		if (widen_to_screen && clip)
		{
			/* widen the clip by the same amount (it is offset-shifted but
			 * still 640-wide at the edges) so the dim is not clipped back
			 * to the centered columns */
			if (clip->x0 <= 0)
				clip->x0 -= (short)extra;
			if (clip->x1 >= design_right)
				clip->x1 = (short)(clip->x1 + extra);
		}
		if (TEST_FLAG(definition->flags, _widget_flash_background_bitmap_bit))
		{
			alpha = (((real)cos(
				widget_globals.current_system_milliseconds *
					SECONDS_PER_MILLISECOND * 3.0f) + 1.0f) * 0.5f) *
				alpha_modifier;
		}
		color = modulate_pixel32_by_real_alpha(0xFFFFFFFF, alpha);
		{
			/* port: a frame of ui.map's that the menus scale (the Xbox's
			picture of the button settings, in the profile settings' smaller
			box): drawn at their size, from where they place it, in units of
			that size rather than one to a texel. A Custom Edition map's own
			picture is stretched over the widget; a stock campaign level's is
			laid out as the stock pictures are */
			rectangle2d texels = bounds;
			short frame_x, frame_y, frame_width, frame_height;
			boolean shown = TRUE;
			boolean stretched = custom_edition_picture &&
				custom_edition_maps_campaign_level(widget->animation.current_frame_index) == NONE;

			/* port: MCC art fits and clips to this style's authored thumbnail
			 * rectangle; only the MCC module decodes or owns its texture. */
			if (mcc_picture_index != NONE && mcc_menu_preview_draw(mcc_picture_index, &bounds, clip, color, &multitexture_params))
				shown = FALSE;
			if (shown && pc_menu_frame_placement(bitmap, &frame_x, &frame_y, &frame_width, &frame_height))
			{
				bounds.x0 += frame_x;
				bounds.y0 += frame_y;
				texels.x0 = 0;
				texels.y0 = 0;
				texels.x1 = (short)((long)(bounds.x1 - bounds.x0) * bitmap->width / frame_width);
				texels.y1 = (short)((long)(bounds.y1 - bounds.y0) * bitmap->height / frame_height);
				shown = bounds.x1 > bounds.x0 && bounds.y1 > bounds.y0;
			}
			/* port: retain native drawing unless a recognized PC-menu legend is replaced. */
			if (shown && !menu_button_glyph_draw(menu_functions_glyph_button(widget, ui_mouse_key_button(widget)),
				widget->local_player_index >= 0 && widget->local_player_index < 4 ?
				widget->local_player_index : 0, &bounds, clip, color))
			{
				draw_bitmap_in_rect(
					bitmap,
					&bounds,
					stretched ? NULL : &texels,
					clip,
					color,
					&multitexture_params,
					FALSE);
			}
		}
		if (use_nifty_plasma_fx)
		{
			ui_plasma_effect_color.alpha = 0.0f;
			ui_plasma_effect_color.red = 0.0f;
			ui_plasma_effect_color.green = 0.0f;
			ui_plasma_effect_color.blue = 0.0f;
		}
	}
	switch (widget->type)
	{
	case _ui_widget_type_text_box:
		widget_instance_render_text_box(
			widget,
			definition,
			clip_rect,
			offset,
			widget_instance_text_box_is_focused(widget));
		/* port: what the menus draw beside a text (the lobby's speaker
		icons: port/linux/game/menu_functions.c), its bounds as it was
		drawn in them (widget_instance_render_text_box), its font and
		justification still set */
		{
			rectangle2d text_bounds = definition->bounds;

			offset_rectangle2d(&text_bounds, offset.x, offset.y);
			text_bounds.x0 += definition->horizontal_offset;
			text_bounds.y0 += definition->vertical_offset;
			menu_functions_text_box_drawn(widget, &text_bounds);
		}
		break;

	case _ui_widget_type_spinner_list:
		widget_instance_render_spinner_list(
			widget,
			definition,
			clip_rect,
			offset,
			focus);
		if (TEST_FLAG(
				definition->list_flags,
				_list_items_generated_from_string_list_tag) &&
			definition->child_widgets.count == 0)
		{
			render_children = FALSE;
		}
		break;

	case _ui_widget_type_column_list:
		widget_instance_render_column_list(
			widget,
			definition,
			clip_rect,
			offset,
			focus);
		render_children = !TEST_FLAG(
			definition->list_flags,
			_list_items_generated_in_code);
		break;
	}
	if (render_children)
	{
		for (child = widget->child; child; child = child->next)
		{
			focus = child == widget->focused_child;
			use_nifty_plasma_fx = focus &&
				(widget->type == _ui_widget_type_spinner_list ||
				widget->type == _ui_widget_type_column_list);
			widget_instance_render_recursive(
				child,
				clip_rect,
				offset,
				focus,
				use_nifty_plasma_fx);
		}
	}

	return;
}

void render_ui_widgets_postgame(
	short local_player_index,
	rectangle2d *window_bounds)
{
	point2d const offsets[MAXIMUM_NUMBER_OF_LOCAL_PLAYERS][MAXIMUM_NUMBER_OF_LOCAL_PLAYERS] =
	{
		{ { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 } },
		{ { 0, 0 }, { 0, 240 }, { 0, 0 }, { 0, 0 } },
		{ { 0, 0 }, { 0, 240 }, { 320, 240 }, { 0, 0 } },
		{ { 0, 0 }, { 320, 0 }, { 0, 240 }, { 320, 240 } }
	};
	rectangle2d bounds;
	long widget_index;

	if (virtual_keyboard_active())
		return;
	local_player_index = PIN(
		local_player_index,
		0,
		MAXIMUM_NUMBER_OF_LOCAL_PLAYERS - 1);
	for (widget_index = 0;
		widget_index < MAXIMUM_NUMBER_OF_LOCAL_PLAYERS;
		widget_index++)
	{
		struct widget_instance *widget = widget_globals.active_widgets[widget_index];
		boolean should_render = FALSE;

		if (widget)
		{
			if (widget->render_regardless_of_controller_index == TRUE)
			{
				should_render = TRUE;
			}
			else if (widget->widget_is_error_dialog == TRUE)
			{
				if (widget->local_player_index == local_player_index ||
					widget->local_player_index == NONE ||
					local_player_index == NONE ||
					we_are_at_the_main_menu)
				{
					should_render = TRUE;
				}
			}
			else if (widget->local_player_index == NONE && widget_index == 0)
			{
				should_render = TRUE;
			}
			else if (widget->local_player_index == local_player_index)
			{
				should_render = TRUE;
			}
		}
		if (should_render)
		{
			bounds.x0 = 0;
			bounds.y0 = 0;
			bounds.x1 = window_bounds->x1 - window_bounds->x0;
			bounds.y1 = window_bounds->y1 - window_bounds->y0;
			widget_instance_render_recursive(
				widget_globals.active_widgets[widget_index],
				&bounds,
				offsets[local_player_count() - 1][local_player_index],
				TRUE,
				FALSE);
		}
	}

	return;
}

/* renders the active widgets for one local player's viewport (or the whole
screen), noting and settling the first player's tap targets in the frame's
first render that can, and draws the debug view of the targets and the
virtual keyboard when they are up */
void render_ui_widgets(
	short local_player_index,
	rectangle2d const *window_bounds)
{
	rectangle2d bounds;
	long widget_index;
	boolean first_players_render = local_player_index == NONE || local_player_index == 0;

	match_assert(
		"c:\\halo\\SOURCE\\interface\\ui_widget.c",
		911,
		window_bounds != NULL);
	local_player_index_for_draw_string_and_hack_in_icons =
		local_player_index == NONE ? 0 : local_player_index;
	if (bink_playback_ui_rendering_inhibited())
		return;
	if (!virtual_keyboard_active())
	{
		local_player_index = PIN(
			local_player_index,
			0,
			MAXIMUM_NUMBER_OF_LOCAL_PLAYERS - 1);
		for (widget_index = 0;
			widget_index < MAXIMUM_NUMBER_OF_LOCAL_PLAYERS;
			widget_index++)
		{
			struct widget_instance *widget = widget_globals.active_widgets[widget_index];
			boolean should_render = FALSE;

			if (widget)
			{
				if (widget->render_regardless_of_controller_index == TRUE)
				{
					should_render = TRUE;
				}
				else if (widget->widget_is_error_dialog == TRUE)
				{
					if (widget->local_player_index == local_player_index ||
						widget->local_player_index == NONE ||
						local_player_index == NONE ||
						we_are_at_the_main_menu)
					{
						should_render = TRUE;
					}
				}
				else if (widget->local_player_index == NONE && widget_index == 0)
				{
					should_render = TRUE;
				}
				else if (widget->local_player_index == local_player_index)
				{
					should_render = TRUE;
				}
			}
			if (should_render)
			{
				point2d offset;

				bounds.x0 = 0;
				bounds.y0 = 0;
				bounds.x1 = window_bounds->x1 - window_bounds->x0;
				bounds.y1 = window_bounds->y1 - window_bounds->y0;
				offset.x = 0;
				offset.y = 0;
				/* the mouse drives the first player's menus; a widget shown in
				every viewport (a dialog for everyone) is noted in the first
				player's render only, and once a frame */
				ui_mouse_noting_targets = first_players_render && !ui_mouse_targets_settled &&
					(widget->local_player_index == NONE || widget->local_player_index == 0);
				widget_instance_render_recursive(
					widget_globals.active_widgets[widget_index],
					&bounds,
					offset,
					TRUE,
					FALSE);
				ui_mouse_noting_targets = FALSE;
				if (widget_globals.debug_show_path)
				{
					real_argb_color color = { 1.0f, 1.0f, 1.0f, 1.0f };

					bounds.x0 += 32;
					bounds.x1 += 32;
					bounds.y0 += 32;
					bounds.y1 += 32;
					draw_string_set_draw_mode(
						tag_loaded(FONT_GROUP_TAG, "ui\\small_ui"),
						NONE,
						0,
						0,
						&color);
					rasterizer_draw_string(
						&bounds,
						NULL,
						NULL,
						0,
						tag_get_name(
							widget_globals.active_widgets[widget_index]->definition_tag_index));
				}
			}
		}
		if (first_players_render && !ui_mouse_targets_settled)
		{
			/* fit the legends while the rows are still targets (merging
			removes them); widen after merging (the merged values are
			row-tall). A finger's targets only: the mouse clicks the
			labels and values where CE's menus have them */
			ui_mouse_fit_button_targets();
			if (ui_mouse_pointer_is_touch)
			{
				ui_mouse_merge_setting_rows();
				ui_mouse_widen_values();
			}
			ui_mouse_targets_settled = TRUE;
		}
		if (widget_globals.fade_to_black >= 0.0f &&
			widget_globals.fade_to_black <= 1.0f)
		{
			real alpha;

			/* the whole screen, around the centered 640 columns */
			bounds.x0 = (short)(-(halo_screen_width() - 640) / 2);
			bounds.x1 = (short)(640 + (halo_screen_width() - 640) / 2);
			bounds.y0 = 0;
			bounds.y1 = 480;
			if (widget_globals.fade_to_black >= 0.95f)
				widget_globals.fade_to_black = 1.0f;
			alpha = widget_globals.fade_to_black * 255.0f;
			draw_quad(&bounds, fast_ftol(alpha) << 24);
		}
	}
	else
	{
		virtual_keyboard_render();
	}
	ui_debug_draw_targets(first_players_render);

	return;
}

/* ---------- private code */


static long spinner_string_list_extra_count(
	long string_list_index)
{
	if (string_list_index != NONE && !csstrcmp(tag_get_name(string_list_index),
		"ui\\shell\\main_menu\\settings_select\\multiplayer_setup\\playlist_edit\\slayer_edit\\var_kills_to_win"))
	{
		return NUMBEROF(kills_to_win_extra_strings);
	}
	return 0;
}

/* a spinner's items of its string list's own (the descriptions of a list of
spinners count those: ui_widget_game_data_input_functions.c) */
short ui_widget_spinner_own_item_count(
	struct widget_instance *spinner)
{
	struct ui_widget_definition *definition = ui_widget_definition_get(spinner->definition_tag_index);

	return (short)(spinner->parameters.list.number_of_items -
		spinner_string_list_extra_count(definition->text_label_string_list.index));
}

/* the string list index of the description of a spinner's extra item, or
NONE for an item of its own */
short ui_widget_spinner_extra_description(
	struct widget_instance *spinner,
	short item_index)
{
	short own = ui_widget_spinner_own_item_count(spinner);

	if (item_index < own)
		return NONE;
	return (short)(SPINNER_EXTRA_DESCRIPTION_BASE + item_index - own);
}

/* a string of a string list, or of its extra strings past the tag's own */
static wchar_t *spinner_string_list_get_string(
	long string_list_index,
	short string_index)
{
	struct string_list *string_list = unicode_string_list_definition_get(string_list_index);

	if (string_list && string_index >= string_list->strings.count &&
		string_index - string_list->strings.count < spinner_string_list_extra_count(string_list_index))
	{
		return (wchar_t *)kills_to_win_extra_strings[string_index - string_list->strings.count];
	}
	return unicode_string_list_get_string(string_list_index, string_index);
}

static void widget_instance_render_column_list(
	struct widget_instance *widget,
	struct ui_widget_definition *definition,
	rectangle2d *clip_rect,
	point2d offset,
	boolean focus)
{
	if (widget->parameters.list.extended_description)
	{
		widget->parameters.list.extended_description->alpha_modifier =
			widget_instance_get_cumulative_alpha_modifier(widget);
		widget_instance_render_recursive(
			widget->parameters.list.extended_description,
			clip_rect,
			offset,
			FALSE,
			TRUE);
	}
	if (TEST_FLAG(definition->list_flags, _list_items_generated_in_code))
	{
		struct widget_instance *child;
		long item_index = 0;

		for (child = widget->child; child; child = child->next)
		{
			if (item_index >= widget->parameters.list.number_of_items)
				break;
			widget_instance_render_recursive(
				child,
				clip_rect,
				offset,
				focus,
				item_index == widget->parameters.list.selected_index);
			item_index++;
		}
	}
	widget->parameters.list.last_list_tab_direction = 0;

	return;
}

static __inline void widget_instance_update_animation_parameters(
	struct widget_instance *widget)
{
	widget->animation.first_frame_index =
		FLOOR(widget->animation.first_frame_index, 0);
	widget->animation.last_frame_index =
		FLOOR(widget->animation.last_frame_index, 0);

	return;
}

static __inline void spinner_list_update(
	struct widget_instance *widget)
{
	struct widget_instance *child;

	for (child = widget->child; child; child = child->next)
	{
		child->animation.current_frame_index = 0;
		if (child == widget->focused_child &&
			child->animation.number_of_sprite_frames == 2)
		{
			child->animation.current_frame_index = 1;
		}
	}

	return;
}

/* the definition is passed by January's only call site but is not read; the
spinner list and the column list keep separate updaters rather than sharing one
parameterised helper */

static void column_list_update(
	struct widget_instance *widget,
	struct ui_widget_definition *definition)
{
	struct widget_instance *child;

	for (child = widget->child; child; child = child->next)
	{
		if (child == widget->focused_child)
		{
			if (child->animation.number_of_sprite_frames == 2)
				child->animation.current_frame_index = 1;
		}
		else if (child->animation.number_of_sprite_frames == 2)
		{
			child->animation.current_frame_index = 0;
		}
	}

	return;
}

static void widget_instance_tab_to_next_valid_widget(
	struct widget_instance *widget)
{
	struct widget_instance *child;

	if (widget->focused_child && widget->focused_child->next)
		child = widget->focused_child->next;
	else
		child = widget->child;
	while (child && child != widget->focused_child)
	{
		struct ui_widget_definition *definition =
			ui_widget_definition_get(child->definition_tag_index);

		if ((definition->event_handlers.count > 0 ||
			TEST_FLAG(definition->flags, _widget_pass_unhandled_events_to_children_bit) ||
			widget->type == _ui_widget_type_spinner_list ||
			widget->type == _ui_widget_type_column_list) &&
			/* port: over the PC version's labels and hidden rows */
			!(widget->type == _ui_widget_type_column_list && pc_menu_tag(widget->definition_tag_index) &&
				widget_instance_port_is_label(child)))
		{
			widget->focused_child = child;
			/* port: (widget_instance_refocus_passthrough) */
			widget_instance_refocus_passthrough(child);
			break;
		}
		child = child->next;
		if (!child)
			child = widget->child;
	}

	return;
}

static void widget_instance_tab_to_previous_valid_widget(
	struct widget_instance *widget)
{
	struct widget_instance *child;

	if (widget->focused_child)
	{
		if (widget->focused_child->previous)
			child = widget->focused_child->previous;
		else
			child = widget_instance_get_tail_child_widget(widget);
	}
	else
	{
		child = widget->child->previous;
		if (!child)
			child = widget->child;
	}
	while (child && child != widget->focused_child)
	{
		struct ui_widget_definition *definition =
			ui_widget_definition_get(child->definition_tag_index);

		if ((definition->event_handlers.count > 0 ||
			TEST_FLAG(definition->flags, _widget_pass_unhandled_events_to_children_bit) ||
			widget->type == _ui_widget_type_spinner_list ||
			widget->type == _ui_widget_type_column_list) &&
			/* port: over the PC version's labels and hidden rows */
			!(widget->type == _ui_widget_type_column_list && pc_menu_tag(widget->definition_tag_index) &&
				widget_instance_port_is_label(child)))
		{
			widget->focused_child = child;
			/* port: (widget_instance_refocus_passthrough) */
			widget_instance_refocus_passthrough(child);
			break;
		}
		if (child->previous)
			child = child->previous;
		else
			child = widget_instance_get_tail_child_widget(widget);
	}

	return;
}

/* port: with one person playing, any controller drives the first player's
menus, not only the first port's: a phone can list a device of its own (its
touch controls) before the gamepad, which then reads port 2, and a player
picks up whichever pad is at hand. Its screens read every controller's
events (process_ui_widgets), and take them
(widget_takes_events_of_controller). With two or more players, each
controller keeps to its own player's menus */
static boolean widget_takes_any_controller(
	struct widget_instance const *widget)
{
	return widget->local_player_index == 0 && !pc_menu_split_players() &&
		(we_are_at_the_main_menu || players_port_local_player_count() <= 1);
}

/* port: the controller whose events a screen reads (NONE: every one's) */
static short widget_event_controller(
	struct widget_instance const *widget)
{
	return widget_takes_any_controller(widget) || pc_menu_screen_reads_every_controller(widget) ? NONE :
		widget->local_player_index;
}

/* port: whether a widget of the local player (NONE: any) takes the
controller's events. In co-op's menus (Multiplayer's CO-OP CAMPAIGN,
port/linux/game/menu_functions.c) the screens it shares with one player's
campaign, New Game's levels and the difficulty, are player 1's (their rows
the first controller's), and either player's controller uses them: player 1's
is the one that chose co-op, player 2's the one that chose their profile */
static boolean widget_takes_events_of_controller(
	struct widget_instance const *widget,
	short controller_index)
{
	short player;

	if (pc_menu_screen_routes_by_controller(widget))
		return !widget->parent || widget->local_player_index == controller_index;
	if (widget->local_player_index == NONE || widget->local_player_index == controller_index)
		return TRUE;
	if (controller_index > 0 && controller_index < MAXIMUM_NUMBER_OF_LOCAL_PLAYERS &&
		(widget_takes_any_controller(widget) || pc_menu_screen_reads_every_controller(widget)))
	{
		return TRUE;
	}
	if (widget->local_player_index != 0 || !we_are_at_the_main_menu || player_spawn_count < 2 ||
		controller_index < 0 || controller_index >= MAXIMUM_NUMBER_OF_LOCAL_PLAYERS)
	{
		return FALSE;
	}
	for (player = 0; player < 2; player++)
	{
		if (player_ui_get_single_player_local_player_controller(player) == controller_index)
			return TRUE;
	}
	return FALSE;
}

static void widget_instance_process_one_event_recursive(
	struct widget_instance *widget,
	struct ui_widget_definition *definition,
	struct event_record *event,
	boolean *return_widget_deleted)
{
	boolean event_handled = FALSE;
	boolean widget_deleted = FALSE;
	boolean event_for_this_widget = widget_takes_events_of_controller(widget, event->controller_index);
	long audio_feedback = _ui_audio_feedback_none;

	match_assert(
		"c:\\halo\\SOURCE\\interface\\ui_widget.c",
		3067,
		widget && definition && event && return_widget_deleted);
	if (event->type == _event_type_button &&
		event->data.button.index >= _widget_event_dpad_up &&
		event->data.button.index <= _widget_event_dpad_right)
	{
		ui_mouse_focused_last = FALSE;
		/* port: (ui_mouse_hovered_last) */
		ui_mouse_hovered_last = FALSE;
	}
	if (event->type == _event_type_button &&
		event->data.button.value > 1 &&
		event->controller_index >= 0 &&
		event->controller_index < MAXIMUM_NUMBER_OF_LOCAL_PLAYERS &&
		event->data.button.index >= _widget_event_dpad_up &&
		event->data.button.index <= _widget_event_dpad_right &&
		widget_globals.current_system_milliseconds -
			dpad_event_times[event->controller_index]
				[event->data.button.index - _widget_event_dpad_up] >=
			DPAD_EVENT_REPEAT_MILLISECONDS)
	{
		event->data.button.value = 1;
	}
	if (widget->close_if_local_player_controller_present == TRUE)
	{
		if (widget->local_player_index >= 0 &&
			widget->local_player_index < MAXIMUM_NUMBER_OF_LOCAL_PLAYERS)
		{
			if (input_has_gamepad(widget->local_player_index))
			{
				ui_widget_delete(widget_instance_get_topmost_parent(widget));
				widget_deleted = TRUE;
			}
		}
		else
		{
			short controller_index;

			match_assert(
				"c:\\halo\\SOURCE\\interface\\ui_widget.c",
				3107,
				widget->local_player_index==NONE);
			for (controller_index = 0;
				controller_index < MAXIMUM_NUMBER_OF_LOCAL_PLAYERS;
				controller_index++)
			{
				if (input_has_gamepad(controller_index))
				{
					ui_widget_delete(widget_instance_get_topmost_parent(widget));
					widget_deleted = TRUE;
					break;
				}
			}
		}
	}
	if (event_for_this_widget && !widget_deleted)
	{
		if (event->type == _event_type_button &&
			event->data.button.value == 1)
		{
			boolean handled_by_event_handler = FALSE;
			long handler_index;

			if (event->data.button.index == _widget_event_back_button)
			{
				for (handler_index = 0;
					handler_index < definition->event_handlers.count;
					handler_index++)
				{
					struct ui_widget_event_handler_reference *handler =
						(struct ui_widget_event_handler_reference *)
							definition->event_handlers.address + handler_index;

					if (handler->event_type == _widget_event_back_button)
					{
						handled_by_event_handler = TRUE;
						break;
					}
				}
			}
			else if (event->data.button.index == _widget_event_b_button)
			{
				for (handler_index = 0;
					handler_index < definition->event_handlers.count;
					handler_index++)
				{
					struct ui_widget_event_handler_reference *handler =
						(struct ui_widget_event_handler_reference *)
							definition->event_handlers.address + handler_index;

					if (handler->event_type == _widget_event_b_button)
					{
						handled_by_event_handler = TRUE;
						break;
					}
				}
			}
			else
			{
				handled_by_event_handler = TRUE;
			}
			if (!handled_by_event_handler)
			{
				widget_instance_go_back_to_previous(widget);
				audio_feedback = _ui_audio_feedback_back;
				widget_deleted = TRUE;
				event_handled = TRUE;
			}
		}
	}
	if (!widget_deleted)
	{
		if (widget->milliseconds_to_auto_close > 0)
		{
			if (widget_globals.current_system_milliseconds - widget->creation_time >=
				widget->auto_close_fade_time + widget->milliseconds_to_auto_close)
			{
				ui_widget_delete(widget_instance_get_topmost_parent(widget));
				widget_deleted = TRUE;
			}
			else if (widget->auto_close_fade_time > 0)
			{
				long faded_milliseconds =
					widget_globals.current_system_milliseconds -
						widget->creation_time -
						widget->milliseconds_to_auto_close;

				if (faded_milliseconds > 0)
				{
					widget->alpha_modifier = 1.0f - (real)faded_milliseconds /
						(real)widget->auto_close_fade_time;
				}
			}
		}
	}
	if (!widget_deleted)
	{
		widget_instance_update_animation_parameters(widget);
		if (widget->type == _ui_widget_type_spinner_list)
			spinner_list_update(widget);
		else if (widget->type == _ui_widget_type_column_list)
			column_list_update(widget, definition);
		if (event_for_this_widget)
		{
			if (!event_handled &&
				TEST_FLAG(definition->flags, _widget_dpad_updown_tabs_thru_children_bit) &&
				widget->focused_child &&
				!widget_deleted)
			{
				if (event->type == _event_type_button &&
					event->data.button.value == 1)
				{
					switch (event->data.button.index)
					{
					case _widget_event_dpad_up:
						widget_instance_tab_to_previous_valid_widget(widget);
						if (audio_feedback == _ui_audio_feedback_none)
							audio_feedback = _ui_audio_feedback_cursor;
						event_handled = TRUE;
						break;
					case _widget_event_dpad_down:
						widget_instance_tab_to_next_valid_widget(widget);
						if (audio_feedback == _ui_audio_feedback_none)
							audio_feedback = _ui_audio_feedback_cursor;
						event_handled = TRUE;
						break;
					}
				}
				else if (event->type == _event_type_left_stick)
				{
					switch (event->data.stick.y)
					{
					case SHORT_MIN:
						widget_instance_tab_to_next_valid_widget(widget);
						if (audio_feedback == _ui_audio_feedback_none)
							audio_feedback = _ui_audio_feedback_cursor;
						event_handled = TRUE;
						break;
					case SHORT_MAX:
						widget_instance_tab_to_previous_valid_widget(widget);
						if (audio_feedback == _ui_audio_feedback_none)
							audio_feedback = _ui_audio_feedback_cursor;
						event_handled = TRUE;
						break;
					}
				}
			}
			if (!event_handled &&
				TEST_FLAG(definition->flags, _widget_dpad_leftright_tabs_thru_children_bit) &&
				widget->focused_child &&
				!widget_deleted)
			{
				if (event->type == _event_type_button &&
					event->data.button.value == 1)
				{
					switch (event->data.button.index)
					{
					case _widget_event_dpad_left:
						widget_instance_tab_to_previous_valid_widget(widget);
						if (audio_feedback == _ui_audio_feedback_none)
							audio_feedback = _ui_audio_feedback_cursor;
						event_handled = TRUE;
						break;
					case _widget_event_dpad_right:
						widget_instance_tab_to_next_valid_widget(widget);
						if (audio_feedback == _ui_audio_feedback_none)
							audio_feedback = _ui_audio_feedback_cursor;
						event_handled = TRUE;
						break;
					}
				}
				else if (event->type == _event_type_left_stick)
				{
					switch (event->data.stick.x)
					{
					case SHORT_MIN:
						widget_instance_tab_to_previous_valid_widget(widget);
						if (audio_feedback == _ui_audio_feedback_none)
							audio_feedback = _ui_audio_feedback_cursor;
						event_handled = TRUE;
						break;
					case SHORT_MAX:
						widget_instance_tab_to_next_valid_widget(widget);
						if (audio_feedback == _ui_audio_feedback_none)
							audio_feedback = _ui_audio_feedback_cursor;
						event_handled = TRUE;
						break;
					}
				}
			}
			if (TEST_FLAG(definition->flags, _widget_dpad_updown_tabs_thru_list_items_bit) &&
				(widget->type == _ui_widget_type_spinner_list ||
					widget->type == _ui_widget_type_column_list) &&
				!event_handled &&
				!widget_deleted)
			{
				if (event->type == _event_type_button &&
					event->data.button.value == 1)
				{
					switch (event->data.button.index)
					{
					case _widget_event_dpad_up:
						widget_event_function_list_widget_goto_previous_item(
							widget,
							event,
							&widget_deleted);
						if (audio_feedback == _ui_audio_feedback_none)
							audio_feedback = _ui_audio_feedback_cursor;
						event_handled = TRUE;
						break;
					case _widget_event_dpad_down:
						widget_event_function_list_widget_goto_next_item(
							widget,
							event,
							&widget_deleted);
						if (audio_feedback == _ui_audio_feedback_none)
							audio_feedback = _ui_audio_feedback_cursor;
						event_handled = TRUE;
						break;
					}
				}
				else if (event->type == _event_type_left_stick)
				{
					switch (event->data.stick.y)
					{
					case SHORT_MIN:
						widget_event_function_list_widget_goto_next_item(
							widget,
							event,
							&widget_deleted);
						if (audio_feedback == _ui_audio_feedback_none)
							audio_feedback = _ui_audio_feedback_cursor;
						event_handled = TRUE;
						break;
					case SHORT_MAX:
						widget_event_function_list_widget_goto_previous_item(
							widget,
							event,
							&widget_deleted);
						if (audio_feedback == _ui_audio_feedback_none)
							audio_feedback = _ui_audio_feedback_cursor;
						event_handled = TRUE;
						break;
					}
				}
			}
			if (TEST_FLAG(definition->flags, _widget_dpad_leftright_tabs_thru_list_items_bit) &&
				(widget->type == _ui_widget_type_spinner_list ||
					widget->type == _ui_widget_type_column_list) &&
				!event_handled &&
				!widget_deleted)
			{
				if (event->type == _event_type_button &&
					event->data.button.value == 1)
				{
					switch (event->data.button.index)
					{
					case _widget_event_dpad_left:
						widget_event_function_list_widget_goto_previous_item(
							widget,
							event,
							&widget_deleted);
						if (audio_feedback == _ui_audio_feedback_none)
							audio_feedback = _ui_audio_feedback_cursor;
						event_handled = TRUE;
						break;
					case _widget_event_dpad_right:
						widget_event_function_list_widget_goto_next_item(
							widget,
							event,
							&widget_deleted);
						if (audio_feedback == _ui_audio_feedback_none)
							audio_feedback = _ui_audio_feedback_cursor;
						event_handled = TRUE;
						break;
					}
				}
				else if (event->type == _event_type_left_stick)
				{
					switch (event->data.stick.x)
					{
					case SHORT_MIN:
						widget_event_function_list_widget_goto_previous_item(
							widget,
							event,
							&widget_deleted);
						if (audio_feedback == _ui_audio_feedback_none)
							audio_feedback = _ui_audio_feedback_cursor;
						event_handled = TRUE;
						break;
					case SHORT_MAX:
						widget_event_function_list_widget_goto_next_item(
							widget,
							event,
							&widget_deleted);
						if (audio_feedback == _ui_audio_feedback_none)
							audio_feedback = _ui_audio_feedback_cursor;
						event_handled = TRUE;
						break;
					}
				}
			}
		}
	}
	if (event_for_this_widget)
	{
		long handler_index;

		for (handler_index = 0;
			handler_index < definition->event_handlers.count;
			handler_index++)
		{
			struct ui_widget_event_handler_reference *handler;
			boolean handler_matches = FALSE;

			if (widget_deleted)
				break;
			handler = (struct ui_widget_event_handler_reference *)
				definition->event_handlers.address + handler_index;
			switch (event->type)
			{
			case _event_type_left_stick:
				switch (handler->event_type)
				{
				case _widget_event_left_stick_up:
					handler_matches = event->data.stick.y == SHORT_MAX;
					break;
				case _widget_event_left_stick_down:
					handler_matches = event->data.stick.y == SHORT_MIN;
					break;
				case _widget_event_left_stick_left:
					handler_matches = event->data.stick.x == SHORT_MIN;
					break;
				case _widget_event_left_stick_right:
					handler_matches = event->data.stick.x == SHORT_MAX;
					break;
				}
				break;
			case _event_type_right_stick:
				switch (handler->event_type)
				{
				case _widget_event_right_stick_up:
					handler_matches = event->data.stick.y == SHORT_MAX;
					break;
				case _widget_event_right_stick_down:
					handler_matches = event->data.stick.y == SHORT_MIN;
					break;
				case _widget_event_right_stick_left:
					handler_matches = event->data.stick.x == SHORT_MIN;
					break;
				case _widget_event_right_stick_right:
					handler_matches = event->data.stick.x == SHORT_MAX;
					break;
				}
				break;
			case _event_type_button:
				handler_matches = handler->event_type == event->data.button.index &&
					event->data.button.value == 1;
				break;
			}
			if (handler_matches)
			{
				event_handled = TRUE;
				event_handler_dispatch(
					widget,
					definition,
					event,
					handler,
					&widget_deleted);
			}
		}
	}
	match_vwarn(
		"c:\\halo\\SOURCE\\interface\\ui_widget.c",
		3477,
		!TEST_FLAG(definition->flags, _widget_pass_handled_events_to_all_children_bit) ||
			TEST_FLAG(definition->flags, _widget_pass_unhandled_events_to_children_bit),
		"if the _widget_pass_handled_events_to_all_children_bit flag is checked, _widget_pass_unhandled_events_to_children_bit must also be checked for it to work");
	if ((TEST_FLAG(definition->flags, _widget_pass_handled_events_to_all_children_bit) ||
			!event_handled) &&
		(TEST_FLAG(definition->flags, _widget_pass_unhandled_events_to_children_bit) ||
			TEST_FLAG(definition->flags, _widget_pass_unhandled_events_to_all_children_bit)) &&
		!widget_deleted)
	{
		if (TEST_FLAG(definition->flags, _widget_pass_unhandled_events_to_all_children_bit))
		{
			struct widget_instance *child;

			for (child = widget->child; child; child = child->next)
			{
				if (widget_takes_events_of_controller(child, event->controller_index))
				{
					widget_instance_process_one_event_recursive(
						child,
						ui_widget_definition_get(child->definition_tag_index),
						event,
						&widget_deleted);
					if (widget_deleted == TRUE)
						break;
				}
			}
		}
		else if (widget->focused_child)
		{
			if (widget_takes_events_of_controller(widget->focused_child, event->controller_index))
			{
				widget_instance_process_one_event_recursive(
					widget->focused_child,
					ui_widget_definition_get(
						widget->focused_child->definition_tag_index),
					event,
					&widget_deleted);
			}
		}
	}
	if (widget_deleted == TRUE &&
		TEST_FLAG(definition->flags, _widget_return_to_main_menu_if_no_history_bit))
	{
		long widget_index;

		for (widget_index = 0;
			widget_index < MAXIMUM_NUMBER_OF_LOCAL_PLAYERS;
			widget_index++)
		{
			if (widget_globals.active_widgets[widget_index])
				break;
		}
		if (widget_index == MAXIMUM_NUMBER_OF_LOCAL_PLAYERS)
			main_goto_main_menu();
	}
	if (event->type == _event_type_button &&
		event->data.button.value == 1 &&
		event->controller_index >= 0 &&
		event->controller_index < MAXIMUM_NUMBER_OF_LOCAL_PLAYERS &&
		event->data.button.index >= _widget_event_dpad_up &&
		event->data.button.index <= _widget_event_dpad_right)
	{
		dpad_event_times[event->controller_index]
			[event->data.button.index - _widget_event_dpad_up] =
			widget_globals.current_system_milliseconds;
	}
	ui_play_audio_feedback_sound(audio_feedback);
	*return_widget_deleted = widget_deleted;

	return;
}

static boolean ui_check_for_pause_game(
	void)
{
	boolean pause_pressed = FALSE;
	boolean network_game = network_game_is_active();
	short controller_index = NONE;

	if (game_in_progress() &&
		!cinematic_in_progress() &&
		game_connection() != _game_connection_film_playback &&
		!we_are_at_the_main_menu &&
		widget_globals.pause_disabled_ticks == 0)
	{
		long gamepad_index;

		for (gamepad_index = 0;
			gamepad_index < MAXIMUM_NUMBER_OF_LOCAL_PLAYERS;
			gamepad_index++)
		{
			if (input_has_gamepad(gamepad_index) &&
				local_player_exists(gamepad_index) &&
				input_get_gamepad_state(gamepad_index)->
					buttons[_gamepad_binary_button_start] == 1)
			{
				pause_pressed = TRUE;
				controller_index = gamepad_index;
				break;
			}
		}
	}
	if (pause_pressed)
	{
		boolean pressed_by_first_local_player = TRUE;
		short local_player_count = 0;
		short pressing_local_player_index = NONE;
		short local_player_index;

		for (local_player_index = local_player_get_next(NONE);
			local_player_index != NONE;
			local_player_index = local_player_get_next(local_player_index))
		{
			if (local_player_index == controller_index)
			{
				pressing_local_player_index = controller_index;
				if (local_player_count >= 1)
					pressed_by_first_local_player = FALSE;
			}
			local_player_count++;
		}
		/* port: MCC selects its owned stock-style pause tree; embedded map menus
		 * remain available to custom scripts but are never the pause entry. */
		{


			if (mcc_pause_runtime_active())
			{
				if ((!network_game || game_engine_allow_pause()) &&
					pressing_local_player_index == controller_index)
				{
					if (widget_globals.active_widgets[controller_index])
						ui_widgets_close_all_for_local_player(controller_index);
					else if (network_game || local_player_count <= 1 || !game_time_get_paused())
					{
						long screen = mcc_pause_runtime_screen(local_player_count, pressed_by_first_local_player);
						if (screen == NONE || !ui_widget_load_by_name_or_tag(NULL, screen, NULL,
							controller_index, NONE, NONE, NONE))
							error(_error_silent, "mcc: failed to open native pause menu");
					}
				}
				return pause_pressed;
			}
		}
		if (network_game)
		{
			if (game_engine_allow_pause() &&
				pressing_local_player_index == controller_index)
			{
				if (!widget_globals.active_widgets[controller_index])
				{
					struct network_game_client *client = global_network_game_client_get();
					struct network_game *network_game_data =
						network_game_client_get_game(client);
					short machine_index =
						network_game_client_get_machine_index(client);
					char const *widget_name;

					/* port: a campaign map has only the campaign's pause screen */
					if (network_coop_active())
						widget_name = "ui\\shell\\solo_game\\pause_game\\pause_game";
					else switch (local_player_count)
					{
					case 1:
						widget_name =
							"ui\\shell\\multiplayer_game\\pause_game\\1p_pause_game";
						break;
					case 2:
						widget_name =
							"ui\\shell\\multiplayer_game\\pause_game\\2p_pause_game";
						break;
					case 3:
						widget_name = pressed_by_first_local_player == TRUE
							? "ui\\shell\\multiplayer_game\\pause_game\\2p_pause_game"
							: "ui\\shell\\multiplayer_game\\pause_game\\4p_pause_game";
						break;
					case 4:
						widget_name =
							"ui\\shell\\multiplayer_game\\pause_game\\4p_pause_game";
						break;
					default:
						error(
							_error_silent,
							"invalid local player count for multiplayer game");
						widget_name = NULL;
						break;
					}
					if (widget_name &&
						!ui_widget_load_by_name_or_tag(
							widget_name,
							NONE,
							NULL,
							controller_index,
							NONE,
							NONE,
							NONE))
					{
						error(
							_error_silent,
							"failed to load multiplayer pause game window");
					}
				}
				else
				{
					ui_widget_delete(widget_globals.active_widgets[controller_index]);
				}
			}
		}
		else
		{
			switch (local_player_count)
			{
			case 0:
			case 1:
				if (widget_globals.active_widgets[controller_index])
				{
					if (game_time_get_paused() == TRUE)
						ui_widgets_close_all();
					/* port: (and a multiplayer map's own, below, which pauses
					nothing, closes as in a multiplayer game) */
					else if (tag_loaded(UI_WIDGET_DEFINITION_TAG, "ui\\shell\\solo_game\\pause_game\\pause_game") == NONE)
						ui_widget_delete(widget_globals.active_widgets[controller_index]);
				}
				/* port: a multiplayer map played alone (New Game's MULTIPLAYER
				maps) has no campaign pause screen, but its own (LEAVE GAME
				goes to the main menu: network_game_remove_local_player) */
				else if (tag_loaded(UI_WIDGET_DEFINITION_TAG, "ui\\shell\\solo_game\\pause_game\\pause_game") == NONE &&
					tag_loaded(UI_WIDGET_DEFINITION_TAG, "ui\\shell\\multiplayer_game\\pause_game\\1p_pause_game") != NONE)
				{
					if (!ui_widget_load_by_name_or_tag(
						"ui\\shell\\multiplayer_game\\pause_game\\1p_pause_game",
						NONE,
						NULL,
						controller_index,
						NONE,
						NONE,
						NONE))
					{
						error(
							_error_silent,
							"failed to load multiplayer pause game window");
					}
				}
				else if (!ui_widget_load_by_name_or_tag(
					"ui\\shell\\solo_game\\pause_game\\pause_game",
					NONE,
					NULL,
					controller_index,
					NONE,
					NONE,
					NONE))
				{
					error(
						_error_silent,
						"failed to load full screen pause game window");
				}
				break;
			case 2:
				if (widget_globals.active_widgets[controller_index])
				{
					if (game_time_get_paused() == TRUE)
						ui_widgets_close_all();
				}
				else if (!game_time_get_paused())
				{
					if (!ui_widget_load_by_name_or_tag(
						"ui\\shell\\solo_game\\pause_game\\pause_game_split_screen",
						NONE,
						NULL,
						controller_index,
						NONE,
						NONE,
						NONE))
					{
						error(
							_error_silent,
							"failed to load split screen pause game window");
					}
				}
				break;
			default:
				error(
					_error_silent,
					"the ui seems to be confused... assuming you are playing full-screen single player?");
				if (!ui_widgets_active())
				{
					if (!ui_widget_load_by_name_or_tag(
						"ui\\shell\\solo_game\\pause_game\\pause_game",
						NONE,
						NULL,
						controller_index,
						NONE,
						NONE,
						NONE))
					{
						error(
							_error_silent,
							"failed to load full screen pause game window");
					}
				}
				else
				{
					ui_widgets_close_all();
				}
				break;
			}
		}
	}
	/* This runs once a frame, several frames per tick on the native builds
	(port/linux/game/render_interpolation.c): count the lock down in 30 Hz
	ticks of real time, not in frames. */
	{
		static real leftover_ticks = 0.f;
		long ticks;

		leftover_ticks += main_get_seconds_elapsed() * TICKS_PER_SECOND;
		ticks = (long)leftover_ticks;
		leftover_ticks -= (real)ticks;
		widget_globals.pause_disabled_ticks =
			FLOOR(widget_globals.pause_disabled_ticks - ticks, 0);
	}

	return pause_pressed;
}

void process_ui_widgets(
	void)
{
	boolean widgets_processed = FALSE;
	boolean widget_deleted;
	boolean any_modal_widget_active;
	boolean pause_pressed;
	boolean modal_widget_active[MAXIMUM_NUMBER_OF_LOCAL_PLAYERS];
	long widget_index;

	match_assert(
		"c:\\halo\\SOURCE\\interface\\ui_widget.c",
		644,
		widget_globals.initialized);
	widget_globals.current_system_milliseconds = system_milliseconds();
	ui_widgets_process_mouse();
	if (ui_widget_port_press_controller != NONE)
	{
		event_manager_post_button(ui_widget_port_press_controller, ui_widget_port_press_button);
		ui_widget_port_press_controller = NONE;
	}
	if (widget_globals.initialization_thread)
	{
		if (!thread_has_exited(widget_globals.initialization_thread))
			return;
		dispose_thread(widget_globals.initialization_thread);
		widget_globals.initialization_thread = NULL;
		ui_widgets_inhibit_processing(FALSE);
		switch (widget_globals.filesystem_check_result)
		{
		case _file_system_check_result_not_enough_free_space:
			if (bink_playback_in_progress())
				bink_playback_stop();
			display_error_abort_to_dashboard(
				_error_hard_drive_not_enough_free_space,
				TRUE);
			return;
		case _file_system_check_result_too_many_saved_games:
			if (bink_playback_in_progress())
				bink_playback_stop();
			display_error_abort_to_dashboard(
				_error_hard_drive_maximum_saved_game_files,
				TRUE);
			return;
		}

		return;
	}
	if (progress_bar_is_active())
		return;
	if (virtual_keyboard_active())
	{
		virtual_keyboard_process();
		event_manager_flush();

		return;
	}
	if (attract_mode_should_start())
	{
		attract_mode_start();

		return;
	}
	if (widget_globals.deferred_dashboard_error_code != NONE)
	{
		display_error_abort_to_dashboard(
			widget_globals.deferred_dashboard_error_code,
			widget_globals.deferred_dashboard_optional);
		widget_globals.deferred_dashboard_error_code = NONE;

		return;
	}
	if (widget_globals.deferred_errors[0].error_code != NONE ||
		widget_globals.deferred_errors[1].error_code != NONE ||
		widget_globals.deferred_errors[2].error_code != NONE ||
		widget_globals.deferred_errors[3].error_code != NONE)
	{
		for (widget_index = 0;
			widget_index < MAXIMUM_NUMBER_OF_LOCAL_PLAYERS;
			widget_index++)
		{
			struct ui_widget_deferred_error *deferred_error =
				&widget_globals.deferred_errors[widget_index];

			if (deferred_error->error_code != NONE)
			{
				if (!we_are_at_the_main_menu &&
					!network_game_is_active() &&
					game_time_get() < DEFERRED_ERROR_DELAY_TICKS)
				{
					error(
						_error_silent,
						"waiting for %d ticks before displaying deferred errors",
						DEFERRED_ERROR_DELAY_TICKS);
				}
				else
				{
					display_error(
						deferred_error->error_code,
						deferred_error->local_player_index,
						deferred_error->modal,
						deferred_error->pause_game_time);
					deferred_error->error_code = NONE;
				}
			}
		}

		return;
	}
	pause_pressed = ui_check_for_pause_game();
	any_modal_widget_active = FALSE;
	for (widget_index = 0;
		widget_index < MAXIMUM_NUMBER_OF_LOCAL_PLAYERS;
		widget_index++)
	{
		modal_widget_active[widget_index] =
			widget_globals.active_widgets[widget_index] &&
			widget_globals.active_widgets[widget_index]->widget_is_error_dialog == TRUE;
		any_modal_widget_active |= modal_widget_active[widget_index];
	}
	for (widget_index = 0;
		widget_index < MAXIMUM_NUMBER_OF_LOCAL_PLAYERS;
		widget_index++)
	{
		struct widget_instance *widget = widget_globals.active_widgets[widget_index];
		boolean process_widget;

		if (modal_widget_active[widget_index] == TRUE)
			process_widget = widget && widget->widget_is_error_dialog == TRUE;
		else if (we_are_at_the_main_menu)
			process_widget = widget && !any_modal_widget_active;
		else
			process_widget = widget != NULL;
		if (process_widget == TRUE)
		{
			struct ui_widget_definition *definition =
				ui_widget_definition_get(widget->definition_tag_index);
			struct event_record event = {0};

			if (widget_globals.processing_inhibited ||
				!get_next_event(&event, widget_event_controller(widget)))
			{
				/* the widget still gets one empty event so that its animation,
				auto-close timer and fade keep running */
				if (!pause_pressed)
				{
					event.controller_index = widget->local_player_index;
					widget_instance_process_one_event_recursive(
						widget,
						definition,
						&event,
						&widget_deleted);
				}
			}
			else
			{
				do
				{
					if (!pause_pressed)
					{
						widget_instance_process_one_event_recursive(
							widget,
							definition,
							&event,
							&widget_deleted);
						if (widget_deleted == TRUE)
							break;
					}
					if (widget != widget_globals.active_widgets[widget_index])
						break;
				}
				while (get_next_event(&event, widget_event_controller(widget)));
			}
			widgets_processed = TRUE;
			if (!widget_globals.active_widgets[widget_index] &&
				widget_globals.widget_stack[widget_index])
			{
				struct widget_stack_data data;

				pop_widget(&widget_globals.widget_stack[widget_index], &data);
				if (data.previous_widget_tag != NONE)
				{
					struct widget_instance *new_widget = ui_widget_load_by_name_or_tag(
						NULL,
						data.previous_widget_tag,
						NULL,
						data.local_player_index,
						NONE,
						NONE,
						NONE);

					if (new_widget)
					{
						widget_instance_set_focused_child_by_index(
							data.focused_child_parent_widget_tag,
							new_widget,
							data.focused_child_index);
					}
				}
			}
		}
	}
	if (widgets_processed)
		event_manager_flush();

	return;
}
