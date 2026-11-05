/*
NETWORK_CLIENT_MANAGER.C

symbols in this file:
001141C0 0040:
	_check_networking_and_generate_error (0000)
00114200 0060:
	_network_game_client_dispose (0000)
00114260 0020:
	_network_game_client_keep_alive (0000)
00114280 0070:
	_network_game_client_get_state (0000)
001142F0 0100:
	_network_game_client_initiate_join_game (0000)
001143F0 0070:
	_network_game_client_set_machine (0000)
00114460 0030:
	_network_game_client_get_machine (0000)
00114490 0040:
	_network_game_client_get_machine_index (0000)
001144D0 0040:
	_network_game_client_get_available_games (0000)
00114510 0040:
	_network_game_client_get_error (0000)
00114550 0040:
	_network_game_client_get_seconds_to_game_start (0000)
00114590 0010:
	_network_game_client_write (0000)
001145A0 00d0:
	_network_game_client_address_matches_server (0000)
00114670 0070:
	_network_game_client_game_out_of_sync (0000)
001146E0 00b0:
	_network_game_client_ponged (0000)
00114790 0110:
	_network_game_client_accepted_into_game (0000)
001148A0 0130:
	_network_game_client_game_settings_updated (0000)
001149D0 0060:
	_unstrip_player_index (0000)
00114A30 01a0:
	_network_game_client_game_has_started (0000)
00114BD0 0190:
	_network_game_client_handle_game_update (0000)
00114D60 0100:
	_network_game_client_add_player_to_game (0000)
00114E60 0050:
	_network_game_client_switch_to_postgame (0000)
00114EB0 00b0:
	_network_game_client_switch_to_pregame (0000)
00114F60 0040:
	_network_game_client_get_connection (0000)
00114FA0 0050:
	_network_game_client_get_remote_server_address (0000)
00114FF0 0040:
	_network_game_client_get_game (0000)
00115030 0040:
	_network_game_client_server_has_started_game (0000)
00115070 0040:
	_network_game_client_get_next_update_number (0000)
001150B0 0040:
	_network_client_get_oos (0000)
001150F0 01f0:
	_network_game_client_add_player (0000)
001152E0 0100:
	_network_game_client_update_local_player_data (0000)
001153E0 00d0:
	_network_game_client_request_start_time_change (0000)
001154B0 0050:
	_network_game_client_countdown_timer_update (0000)
00115500 0030:
	_network_game_client_advertised_game_is_valid (0000)
00115530 02d0:
	_add_advertised_game (0000)
00115800 0050:
	_network_game_client_set_error (0000)
00115850 00c0:
	_network_game_client_update_precache_status (0000)
00115910 0080:
	_network_game_client_process_incoming_messages (0000)
00115990 0260:
	_network_game_client_leave_game (0000)
00115BF0 01f0:
	_network_game_client_request_remove_player (0000)
00115DE0 0170:
	_network_game_client_remove_player (0000)
00115F50 0050:
	_network_game_client_new_advertised_game (0000)
00115FA0 0070:
	_network_game_client_game_shutdown (0000)
00116010 00e0:
	_network_game_client_reset (0000)
001160F0 02c0:
	_network_game_client_idle_searching (0000)
001163B0 0180:
	_network_game_client_idle_joining (0000)
00116530 00d0:
	_network_game_client_idle_pregame (0000)
00116600 0190:
	_network_game_client_idle_ingame (0000)
00116790 00a0:
	_network_game_client_idle_postgame (0000)
00116830 0090:
	_network_game_client_create (0000)
001168C0 0130:
	_network_game_client_idle (0000)
001169F0 00c0:
	_network_game_client_rejected_by_game (0000)
0027FC5C 0018:
	??_C@_0BI@DEMLIONO@network?5client?5disposed?$AA@ (0000)
0027FC74 002d:
	??_C@_0CN@POHNGKMA@network_game_client_dont_use_dir@ (0000)
0027FCA4 0033:
	??_C@_0DD@GHLNCEHN@c?3?2halo?2SOURCE?2networking?2networ@ (0000)
0027FCD8 0007:
	??_C@_06DGDLPBAN@client?$AA@ (0000)
0027FCE0 0035:
	??_C@_0DF@IEPBBGCD@failed?5attempt?5to?5initiate?5a?5con@ (0000)
0027FD18 0023:
	??_C@_0CD@NIGOILLF@attempting?5to?5connect?5to?5game?5?$EA?5@ (0000)
0027FD40 00e1:
	??_C@_0OB@CAKDCBMD@client?5?$CG?$CG?5?$CIclient?9?$DOstate?5?$DN?$DN?5_net@ (0000)
0027FE28 0065:
	??_C@_0GF@JAOOPLIH@client?5?$CG?$CG?5?$CIclient?9?$DOmachine_index@ (0000)
0027FE90 001e:
	??_C@_0BO@IIIGFMHE@address?9?$DOaddress?4ipv4_address?$AA@ (0000)
0027FEB0 0010:
	??_C@_0BA@IONABMGK@address?5?$CB?$DN?5NULL?$AA@ (0000)
0027FEC0 0013:
	??_C@_0BD@PONNKDL@client?9?$DOconnection?$AA@ (0000)
0027FED4 000f:
	??_C@_0P@FOLJNCCO@client?5?$CB?$DN?5NULL?$AA@ (0000)
0027FEE4 002d:
	??_C@_0CN@FOHIFFFA@local?5machine?5is?5out?5of?5sync?5wit@ (0000)
0027FF14 0036:
	??_C@_0DG@JNMFILGG@received?5a?5pong?5from?5a?5system?5we@ (0000)
0027FF4C 0020:
	??_C@_0CA@IDICKDHG@received?5a?5pong?5from?5the?5future?$AA@ (0000)
0027FF6C 0019:
	??_C@_0BJ@NIPOLBAB@client?5?$CG?$CG?5source_address?$AA@ (0000)
0027FF88 004c:
	??_C@_0EM@DJKCNCJP@received?5a?5message_server_machin@ (0000)
0027FFD4 003b:
	??_C@_0DL@PONBKBJJ@failed?5to?5create?5a?5message_clien@ (0000)
00280010 005b:
	??_C@_0FL@OHPOKFBL@network_game_client_write?$CI?$CJ?5fail@ (0000)
0028006C 0033:
	??_C@_0DD@POLEJMBC@successfully?5joined?5a?5net?5game?$DL?5@ (0000)
002800A0 0064:
	??_C@_0GE@OLBCDHF@client?5?$CG?$CG?5source_address?5?$CG?$CG?5mess@ (0000)
00280108 005e:
	??_C@_0FO@NIJHHMKN@invalid?5message_server_game_sett@ (0000)
00280168 0021:
	??_C@_0CB@KMIKJFNP@player?5count?5?$CFd?5machine?5count?5?$CFd@ (0000)
00280190 0060:
	??_C@_0GA@FCJMEIKG@received?5updated?5game?5settings?5f@ (0000)
002801F0 0017:
	??_C@_0BH@KPFLOHLF@precaching?5map?5?8?$CFs?8?4?4?4?$AA@ (0000)
00280208 0019:
	??_C@_0BJ@MGLALEIH@client?5?$CG?$CG?5message_packet?$AA@ (0000)
00280224 0027:
	??_C@_0CH@CHLJGLMB@failed?5to?5load?5the?5necessary?5gam@ (0000)
0028024C 0031:
	??_C@_0DB@BGLEMKKH@failed?5to?5create?5a?5message_clien@ (0000)
00280280 0051:
	??_C@_0FB@NFJPBNDI@network_game_client_write?$CI?$CJ?5fail@ (0000)
002802D4 0028:
	??_C@_0CI@DFFIDDAF@local?5machine?5is?5loaded?5?$CG?5ready?5@ (0000)
00280300 0040:
	??_C@_0EA@MFKNPKNH@client?5?$CG?$CG?5?$CIclient?9?$DOstate?5?$DN?$DN?5_net@ (0000)
00280340 0036:
	??_C@_0DG@PKKMJLJH@client?5is?5lagging?5behind?5the?5ser@ (0000)
00280378 0061:
	??_C@_0GB@LLAIIL@out?5of?5sync?3?5client?1server?5rando@ (0000)
002803DC 002d:
	??_C@_0CN@HEOJJEAC@not?5a?5bug?0?5but?5update?5?$CFd?5time?5?$CFd@ (0000)
0028040C 003e:
	??_C@_0DO@OFILJPBP@out?5of?5sync?3?5missed?5a?5server?5upd@ (0000)
0028044C 003c:
	??_C@_0DM@JCJEPCIA@added?5new?5player?5to?5the?5game?5?$CIma@ (0000)
00280488 0011:
	??_C@_0BB@NJALFKEL@client?5?$CG?$CG?5player?$AA@ (0000)
0028049C 0016:
	??_C@_0BG@OEHGDLGH@switching?5to?5postgame?$AA@ (0000)
002804B4 0015:
	??_C@_0BF@FFECCEDB@switching?5to?5pregame?$AA@ (0000)
002804CC 001e:
	??_C@_0BO@NFAGDPGC@client?5is?5in?5an?5unknown?5state?$AA@ (0000)
002804EC 002e:
	??_C@_0CO@BDJGBKN@client?5tried?5to?5add?5a?5new?5player@ (0000)
00280520 0044:
	??_C@_0EE@EPIGONGI@failed?5to?5create?5a?5message_clien@ (0000)
00280568 0064:
	??_C@_0GE@JOEDNBHN@network_game_client_write?$CI?$CJ?5fail@ (0000)
002805D0 0045:
	??_C@_0EF@CDEPJEJP@failed?5to?5create?5a?5message_clien@ (0000)
00280618 0065:
	??_C@_0GF@EOEDLFEI@network_game_client_write?$CI?$CJ?5fail@ (0000)
00280680 0039:
	??_C@_0DJ@IAEGENPP@can?8t?5add?5players?5to?5a?5game?5unti@ (0000)
002806BC 0034:
	??_C@_0DE@DFECLBOM@requesting?5a?5player?5addition?5?$CIco@ (0000)
002806F0 005a:
	??_C@_0FK@DNGOMOFK@client?5?$CG?$CG?5?$CIlocal_player_index?$DO?$DN0@ (0000)
00280750 0075:
	??_C@_0HF@KCCMHIFI@network_game_client_update_local@ (0000)
002807C8 0020:
	??_C@_0CA@GNHJCNBC@network_player_is_valid?$CIplayer?$CJ?$AA@ (0000)
002807E8 002d:
	??_C@_0CN@MLPKDKFB@player?9?$DOmachine_index?$DN?$DNclient?9?$DOm@ (0000)
00280818 005b:
	??_C@_0FL@EPANHPNG@failed?5to?5send?5a?5message_client_@ (0000)
00280878 006b:
	??_C@_0GL@IAFAMNHJ@network_game_client_request_star@ (0000)
002808E8 0042:
	??_C@_0EC@OCMHEGGP@?$CIrequest_type?$DO?$DN0?$CJ?5?$CG?$CG?5?$CIrequest_ty@ (0000)
0028092C 0038:
	??_C@_0DI@NNABEADG@there?5is?5?$CFs?5?$CFs?5net?5game?5with?5?$CFd?5@ (0000)
00280964 0009:
	??_C@_08EDJFMNDP@a?5closed?$AA@ (0000)
00280970 0008:
	??_C@_07JPIGLNNP@an?5open?$AA@ (0000)
00280978 0013:
	??_C@_0BD@DEAGDPEH@?$DMunknown?5platform?$DO?$AA@ (0000)
0028098C 0003:
	??_C@_02LHFGBEJL@PC?$AA@ (0000)
00280990 0005:
	??_C@_04KEMOGDLN@XBox?$AA@ (0000)
00280998 0008:
	??_C@_17JLFEDMMI@?$AA?$DP?$AA?$DP?$AA?$DP?$AA?$AA@ (0000)
002809A0 0050:
	??_C@_0FA@NLFFOHBN@not?5fatal?0?5but?5we?5have?5to?5many?5a@ (0000)
002809F0 000f:
	??_C@_0P@JAPJELHA@current?9?$DOvalid?$AA@ (0000)
00280A00 0065:
	??_C@_0GF@OLAKLCGH@network_game_client_write?$CI?$CJ?5fail@ (0000)
00280A68 005f:
	??_C@_0FP@INNHEEHK@network_game_client_handle_messa@ (0000)
00280AC8 004b:
	??_C@_0EL@PNCEEMAP@network_connection_disconnect?$CI?$CJ?5@ (0000)
00280B18 0066:
	??_C@_0GG@PLNHOJNK@network_game_client_write?$CI?$CJ?5fail@ (0000)
00280B80 0049:
	??_C@_0EJ@ENIOKCKK@network_connection_disconnect?$CI?$CJ?5@ (0000)
00280BD0 004a:
	??_C@_0EK@GLADDNKH@network_connection_disconnect?$CI?$CJ?5@ (0000)
00280C20 0045:
	??_C@_0EF@IGAGJDLA@failed?5to?5create?5a?5message_clien@ (0000)
00280C68 004a:
	??_C@_0EK@PGHLLHE@network_connection_disconnect?$CI?$CJ?5@ (0000)
00280CB4 0032:
	??_C@_0DC@NBAMODMB@?$CBnetwork_connection_connected?$CIcl@ (0000)
00280CE8 0015:
	??_C@_0BF@IHNDJMJI@leaving?5network?5game?$AA@ (0000)
00280D00 001d:
	??_C@_0BN@LOKEMLAG@client?5?$CG?$CG?5client?9?$DOconnection?$AA@ (0000)
00280D20 0049:
	??_C@_0EJ@ENDAFHHG@failed?5to?5create?5a?5message_clien@ (0000)
00280D70 0069:
	??_C@_0GJ@FAIKKIGI@network_game_client_write?$CI?$CJ?5fail@ (0000)
00280DE0 0047:
	??_C@_0EH@PAHGGHIO@failed?5to?5create?5a?5message_clien@ (0000)
00280E28 0047:
	??_C@_0EH@NIHALOOC@failed?5to?5create?5a?5message_clien@ (0000)
00280E70 003e:
	??_C@_0DO@LCAGMDAF@can?8t?5remove?5players?5from?5a?5game@ (0000)
00280EB0 0033:
	??_C@_0DD@MJJEPI@requesting?5a?5player?5removal?5?$CIcon@ (0000)
00280EE4 0039:
	??_C@_0DJ@ICFAMCBO@client?8s?5can?5only?5remove?5players@ (0000)
00280F20 002a:
	??_C@_0CK@HGKAMGLN@client?5?$CG?$CG?5network_player_is_vali@ (0000)
00280F50 004b:
	??_C@_0EL@IAJNHEOO@network?5game?5tried?5to?5delete?5a?5p@ (0000)
00280F9C 003a:
	??_C@_0DK@NONHICBE@no?5local?5players?5remain?5in?5the?5g@ (0000)
00280FD8 0027:
	??_C@_0CH@HDEKFDCH@?$CFx?5quit?5of?5of?5game?5at?5tick?5?$CFd?5?$CIn@ (0000)
00281000 001f:
	??_C@_0BP@BGMFPCP@the?5game?5host?5is?5shutting?5down?$AA@ (0000)
00281020 002b:
	??_C@_0CL@FIOOPCIN@failed?5to?5reinitialize?5network?5g@ (0000)
0028104C 002f:
	??_C@_0CP@KMAGFDPK@failed?5to?5create?5a?5message_clien@ (0000)
00281080 004f:
	??_C@_0EP@PHHJAEIL@network_game_client_write?$CI?$CJ?5fail@ (0000)
002810D0 0040:
	??_C@_0EA@BPICBMAG@failed?5to?5create?5a?5message_clien@ (0000)
00281110 0060:
	??_C@_0GA@PDNHMAGG@network_game_client_write?$CI?$CJ?5fail@ (0000)
00281170 0028:
	??_C@_0CI@BAIHEHNF@sent?5out?5a?5broadcast?5game?5search@ (0000)
00281198 005f:
	??_C@_0FP@MCNPILK@network_game_client_process_inco@ (0000)
002811F8 0049:
	??_C@_0EJ@CNGHFAJM@network_connection_idle?$CI?$CJ?5failed@ (0000)
00281244 0030:
	??_C@_0DA@OHNFENCP@network_game_client_initiate_joi@ (0000)
00281278 0047:
	??_C@_0EH@NDJKGODE@network_connection_idle?$CI?$CJ?5failed@ (0000)
002812C0 0045:
	??_C@_0EF@EDNLPKBO@client?5connection?5process?5has?5ti@ (0000)
00281308 005d:
	??_C@_0FN@FLBCCMKM@network_game_client_process_inco@ (0000)
00281368 003c:
	??_C@_0DM@ONBHNBGE@failed?5to?5create?5a?5message_clien@ (0000)
002813A8 0056:
	??_C@_0FG@EPCJDBOJ@network_game_client_write?$CI?$CJ?5fail@ (0000)
00281400 005d:
	??_C@_0FN@PFNMPIAB@network_game_client_process_inco@ (0000)
00281460 0047:
	??_C@_0EH@HNFELKJJ@network_connection_idle?$CI?$CJ?5failed@ (0000)
002814A8 0046:
	??_C@_0EG@BKDKHIKF@network_connection_idle?$CI?$CJ?5failed@ (0000)
002814F0 001c:
	??_C@_0BM@GGCBKEBE@new2?5idle?5in?5game?5abort?5hit?$AA@ (0000)
0028150C 001b:
	??_C@_0BL@IEONCJFK@new?5idle?5in?5game?5abort?5hit?$AA@ (0000)
00281528 005c:
	??_C@_0FM@JKCCAGHN@network_game_client_process_inco@ (0000)
00281588 0050:
	??_C@_0FA@IKFHHCAC@network?5client?5connection?5has?5be@ (0000)
002815D8 002d:
	??_C@_0CN@GOKOEOFK@network?5connection?5went?5down?5?$CIid@ (0000)
00281608 005e:
	??_C@_0FO@IANECALJ@network_game_client_process_inco@ (0000)
00281668 0048:
	??_C@_0EI@HOIILDIN@network_connection_idle?$CI?$CJ?5failed@ (0000)
002816B0 0049:
	??_C@_0EJ@KIFFADMI@network_game_create_client?$CI?$CJ?5fai@ (0000)
002816FC 002e:
	??_C@_0CO@LNIBCIDF@?$CBnetwork_game_client_dont_use_di@ (0000)
0028172C 0018:
	??_C@_0BI@HFOMKHNG@?$CB?$CCunknown?5client?5state?$CC?$AA@ (0000)
00281744 002b:
	??_C@_0CL@EHLJKOCJ@network_game_client_idle_postgam@ (0000)
00281770 0029:
	??_C@_0CJ@NNEAELPC@network_game_client_idle_ingame?$CI@ (0000)
0028179C 002a:
	??_C@_0CK@IFHOOMMF@network_game_client_idle_pregame@ (0000)
002817C8 002a:
	??_C@_0CK@NBDMDINO@network_game_client_idle_joining@ (0000)
002817F4 002c:
	??_C@_0CM@IDLHBFKK@network_game_client_idle_searchi@ (0000)
00281820 0024:
	??_C@_0CE@DOGOGKDE@unable?5to?5join?5game?3?5reason?$DN?5?$CD?$CFd@ (0000)
00281844 0024:
	??_C@_0CE@JOPGMOA@_rejection_code_blacklisted_mach@ (0000)
00281868 001f:
	??_C@_0BP@EHGPFGEK@_rejection_code_game_is_closed?$AA@ (0000)
00281888 001d:
	??_C@_0BN@IDHMJFHO@_rejection_code_game_is_full?$AA@ (0000)
002818A8 001d:
	??_C@_0BN@NIEJJECI@_rejection_code_bad_password?$AA@ (0000)
002818C8 001f:
	??_C@_0BP@MGMNOIHD@_rejection_code_bad_join_token?$AA@ (0000)
002818E8 0020:
	??_C@_0CA@EEFCENHL@_rejection_code_version_too_new?$AA@ (0000)
00281908 0020:
	??_C@_0CA@JCNAFAAD@_rejection_code_version_too_old?$AA@ (0000)
004566D8 0002:
	_allow_out_of_sync (0000)
	_network_game_client_dont_use_directly_in_use (0001)
*/

/* ---------- headers */

#include "cseries.h"
#include "bungie_net/common/message_header.h"
#include "bungie_net/network/transport.h"
#include "bungie_net/network/transport_address_constants.h"
#include "bungie_net/network/transport_endpoint_winsock.h"
#include "cache/cache_files.h"
#include "cseries/cseries_windows.h"
#include "cseries/errors.h"
#include "game/game_engine.h"
#include "game/local_players.h"
#include "game/player_queues_new.h"
#include "game/players.h"
#include "interface/player_ui.h"
#include "saved games/player_profile.h"
#include "interface/ui_widget.h"
#include "main/main.h"
#include "memory/data.h"
#include "networking/network_client_manager.h"
#include "networking/network_client_message_handler.h"
#include "networking/network_connection.h"
#include "networking/network_game_globals.h"
#include "networking/network_game_manager.h"
#include "networking/network_game_protocol.h"
#include "networking/network_messages.h"
#include "networking/network_server_manager.h"
#include "text/unicode.h"
#include "map_share.h" /* port: port/linux/game/map_share.c */
#include "custom_edition_cache.h" /* port: port/linux/game/custom_edition_cache.c */

/* ---------- constants */

enum
{
	/* the native builds' session limits (port/linux/include/halo_port_limits.h) */
	MAXIMUM_NETWORK_MACHINE_COUNT = HALO_PORT_MAXIMUM_NETWORK_MACHINES,
	MAXIMUM_NETWORK_ADVERTISED_GAMES = 9,
	MAXIMUM_NUMBER_OF_PLAYERS = HALO_PORT_MAXIMUM_NETWORK_PLAYERS,
	MAXIMUM_MACHINE_NAME_LENGTH = 32,
	NETWORK_GAME_NAME_LENGTH = 16,
	TRANSPORT_NONCE_LENGTH = 8,
	NETWORK_PLAYER_NAME_LENGTH = 12,
	NETWORK_GAME_MAP_NAME_LENGTH = 0x100,
	/* the client's buffer for any message from the server */
	MAXIMUM_NETWORK_MESSAGE_SIZE = HALO_PORT_MAXIMUM_NETWORK_MESSAGE_SIZE,
	/* how long the network may be down in a game before the game ends (as
	long as the host waits for a silent machine, network_server_manager.c):
	network_game_client_network_lost */
	NETWORK_GAME_CLIENT_LINK_DOWN_TIMEOUT = 15000,
	/* port: how long a frame the client spends on the messages that came
	(a host that floods it with them would hold each frame for as long as
	they took): those left wait in the queue, in order, for the next
	frame's, and the stream what the queue has no room for. One is always
	handled (network_game_client_process_messages) */
	MAXIMUM_MESSAGE_MILLISECONDS_PER_IDLE = 50,
};

enum network_game_client_state
{
	_network_game_client_state_searching,
	_network_game_client_state_joining,
	_network_game_client_state_pregame,
	_network_game_client_state_ingame,
	_network_game_client_state_postgame,
	NUMBER_OF_NETWORK_GAME_CLIENT_STATES,
};

enum
{
	_network_game_client_error_none = 0,
	_network_game_client_error_unknown = 1,
	_network_game_client_error_server_shutdown = 8,
	NUMBER_OF_NETWORK_GAME_CLIENT_ERRORS = 9
};

enum
{
	_network_game_client_connection_established_bit,
	_network_game_client_join_request_sent_bit
};

enum
{
	NUMBER_OF_GAME_START_REQUESTS = 4
};

enum
{
	_game_advertisement_open_bit = 1,
	_game_advertisement_has_teams_bit,
	_game_advertisement_oddball_variant_bit
};

enum network_game_platform
{
	_network_game_platform_xbox,
	_network_game_platform_pc,
	NUMBER_OF_NETWORK_GAME_PLATFORMS
};

enum
{
	_network_connection_type_client = 2,
};

enum
{
	BROADCAST_IPV4_ADDRESS = 0xFFFFFFFF
};

/* ---------- macros */

#define network_game_get_local_platform() _network_game_platform_xbox

#define network_machine_is_valid(machine) \
	((machine) && (machine)->machine_index >= 0 && \
	(machine)->machine_index < MAXIMUM_NETWORK_MACHINE_COUNT)

/* ---------- structures */

struct transport_key_id
{
	byte data[8];
};

struct transport_key
{
	byte data[16];
};

struct transport_xnaddr
{
	byte data[12];
};

struct network_advertised_game
{
	struct transport_key_id key_id;
	struct transport_key key;
	struct transport_xnaddr xnaddr;
	byte nonce[TRANSPORT_NONCE_LENGTH];
	unsigned long update_time;
	wchar_t game_name[NETWORK_GAME_NAME_LENGTH];
	struct network_game_map map;
	short engine_type;
	word machine_count;
	word player_count;
	short maximum_player_count;
	short unknown100;
	short platform;
	boolean open;
	boolean valid;
	boolean has_teams;
	boolean oddball_variant;
};

struct message_server_game_advertise
{
	byte client_nonce[TRANSPORT_NONCE_LENGTH];
	byte nonce[TRANSPORT_NONCE_LENGTH];
	struct transport_key_id key_id;
	struct transport_key key;
	struct transport_xnaddr xnaddr;
	word port;
	word version;
	word platform;
	wchar_t game_name[NETWORK_GAME_NAME_LENGTH];
	byte __unknown5A[0x1A];
	struct network_game_map map;
	short engine_type;
	short machine_count;
	short player_count;
	short maximum_player_count;
	short unknown100;
	word flags;
	byte join_game_token[0x10];
};

struct network_join_parameters
{
	word __unknown0;
	word __unknown2;
	byte __unknown4[0xE];
	byte join_token[0x10];
};

struct network_connection;
struct connect_process_input;

struct message_client_game_start_request
{
	short request_type;
};

struct message_client_add_player_request
{
	struct network_player player;
};

struct message_client_remove_player_request
{
	struct network_player player;
};

struct message_server_game_update
{
	unsigned long update_number;
	long random_seed;
	long game_time;
	byte __unknown0C[2];
	short local_player_count;
	struct player_action player_actions[MAXIMUM_NUMBER_OF_PLAYERS];
};

struct message_client_loaded
{
	long opaque;
};

struct message_client_broadcast_game_search
{
	word port;
	word __unknown2;
	byte nonce[8];
};

struct message_client_ping
{
	long timestamp;
	word port;
	byte __padding6[2];
};

/* port/linux/src/p2p.c's */
void p2p_hardware_id(char *hex, int size);
void p2p_join_refused(int rejection_code);
/* port/linux/src/player_link.c's (a Discord account's pass, shown to a
dedicated server that asks for it) */
int player_link_join_fill(unsigned long host_virtual_address, unsigned char *pass, unsigned char *proof);
int player_link_join_refused(int rejection_code);
void player_link_join_reset(void);
/* port: a join made again with the pass (network_game_client_rejected_by_game),
not a new one; and one made while the refused connection's end is handled
(its idle failed: the join goes on) */
static boolean network_game_client_joining_with_link;
static boolean network_game_client_joined_again_with_link;

struct message_client_join_game_request
{
	wchar_t machine_name[MAXIMUM_MACHINE_NAME_LENGTH];
	byte join_token[0x10];
	/* port: this machine's hardware id, as hex (p2p.c's p2p_hardware_id) */
	char hardware_id[0x20];
	/* port: a Discord account's pass and its proof for this connection, all
	zero for none (port/linux/src/player_link.c) */
	byte link_pass[170];
	byte link_proof[64];
};

struct message_client_map_is_precached_pregame
{
	char map_name[NETWORK_GAME_MAP_NAME_LENGTH];
};

struct message_server_machine_accepted
{
	long random_seed;
	short machine_index;
	byte __padding6[2];
};

struct message_client_settings_request
{
	wchar_t machine_name[MAXIMUM_MACHINE_NAME_LENGTH];
	char machine_index;
	byte __padding41[3];
};

struct network_game_client
{
	word machine_index;
	byte __padding2[2];
	struct network_advertised_game available_games[MAXIMUM_NETWORK_ADVERTISED_GAMES];
	struct transport_address ping_address;
	unsigned long last_ping_time;
	byte __padding824[2];
	word ping_sample_count;
	word average_ping;
	boolean pinging;
	byte __padding82B;
	struct network_connection *connection;
	struct connect_process_input *connect_process;
	unsigned long connection_attempt_time;
	struct network_join_parameters join_parameters;
	byte __padding85A[2];
	struct network_game game;
	unsigned long join_in_progress;
	unsigned long last_broadcast_search_time;
	unsigned long next_update_number;
	unsigned long last_precache_time;
	short seconds_to_game_start;
	word state;
	short error;
	word flags;
	boolean connection_silent;
};

typedef char network_machine_size_assert[
	sizeof(struct network_machine) == 0x44 ? 1 : -1];
typedef char player_profile_size_assert[
	sizeof(struct player_profile) == 0x30 ? 1 : -1];
/* the update and settings layouts follow the session limits
(port/linux/include/halo_port_limits.h) */
typedef char message_server_game_update_size_assert[
	sizeof(struct message_server_game_update) == 0x10 + MAXIMUM_NUMBER_OF_PLAYERS * 0x20 ? 1 : -1];
typedef char message_server_machine_accepted_size_assert[
	sizeof(struct message_server_machine_accepted) == 8 ? 1 : -1];
typedef char message_client_settings_request_size_assert[
	sizeof(struct message_client_settings_request) == 0x44 ? 1 : -1];
typedef char network_game_size_assert[
	sizeof(struct network_game) == HALO_PORT_NETWORK_GAME_SIZE ? 1 : -1];
typedef char network_game_map_name_offset_assert[
	offsetof(struct network_game, map.name) == 0x24 ? 1 : -1];
typedef char network_game_machine_count_offset_assert[
	offsetof(struct network_game, machine_count) == 0x112 ? 1 : -1];
typedef char network_game_local_data_offset_assert[
	offsetof(struct network_game, local_data) == HALO_PORT_NETWORK_GAME_LOCAL_DATA_OFFSET ? 1 : -1];
typedef char network_game_player_count_offset_assert[
	offsetof(struct network_game, player_count) == HALO_PORT_NETWORK_GAME_PLAYER_COUNT_OFFSET ? 1 : -1];
typedef char network_game_players_offset_assert[
	offsetof(struct network_game, players) == HALO_PORT_NETWORK_GAME_PLAYERS_OFFSET ? 1 : -1];
typedef char network_advertised_game_update_time_offset_assert[
	offsetof(struct network_advertised_game, update_time) == 0x2C ? 1 : -1];
typedef char network_advertised_game_size_assert2[
	sizeof(struct message_server_game_advertise) == 0x114 ? 1 : -1];
typedef char network_advertised_game_game_name_offset_assert[
	offsetof(struct network_advertised_game, game_name) == 0x30 ? 1 : -1];
typedef char network_advertised_game_engine_type_offset_assert[
	offsetof(struct network_advertised_game, engine_type) == 0xD4 ? 1 : -1];
typedef char network_advertised_game_platform_offset_assert[
	offsetof(struct network_advertised_game, platform) == 0xDE ? 1 : -1];
typedef char network_advertised_game_open_offset_assert[
	offsetof(struct network_advertised_game, open) == 0xE0 ? 1 : -1];
typedef char network_advertised_game_valid_offset_assert[
	offsetof(struct network_advertised_game, valid) == 0xE1 ? 1 : -1];
typedef char network_join_parameters_size_assert[
	sizeof(struct network_join_parameters) == 0x22 ? 1 : -1];
typedef char data_iterator_datum_index_offset_assert[
	offsetof(struct data_iterator, datum_index) == 8 ? 1 : -1];
typedef char network_advertised_game_size_assert[
	sizeof(struct network_advertised_game) == 0xE4 ? 1 : -1];
typedef char network_game_client_ping_address_offset_assert[
	offsetof(struct network_game_client, ping_address) == 0x808 ? 1 : -1];
typedef char network_game_client_ping_sample_count_offset_assert[
	offsetof(struct network_game_client, ping_sample_count) == 0x826 ? 1 : -1];
typedef char network_game_client_pinging_offset_assert[
	offsetof(struct network_game_client, pinging) == 0x82A ? 1 : -1];
typedef char network_game_client_connection_offset_assert[
	offsetof(struct network_game_client, connection) == 0x82C ? 1 : -1];
typedef char network_game_client_connect_process_offset_assert[
	offsetof(struct network_game_client, connect_process) == 0x830 ? 1 : -1];
typedef char network_game_client_connection_attempt_time_offset_assert[
	offsetof(struct network_game_client, connection_attempt_time) == 0x834 ? 1 : -1];
typedef char network_game_client_join_parameters_offset_assert[
	offsetof(struct network_game_client, join_parameters) == 0x838 ? 1 : -1];
typedef char network_game_client_game_offset_assert[
	offsetof(struct network_game_client, game) == 0x85C ? 1 : -1];
/* the fields after the game settings record move with its size */
#define NETWORK_GAME_CLIENT_TAIL_OFFSET (0x85C + HALO_PORT_NETWORK_GAME_SIZE)
typedef char network_game_client_join_in_progress_offset_assert[
	offsetof(struct network_game_client, join_in_progress) == NETWORK_GAME_CLIENT_TAIL_OFFSET ? 1 : -1];
typedef char network_game_client_next_update_number_offset_assert[
	offsetof(struct network_game_client, next_update_number) == NETWORK_GAME_CLIENT_TAIL_OFFSET + 8 ? 1 : -1];
typedef char network_game_client_last_broadcast_search_time_offset_assert[
	offsetof(struct network_game_client, last_broadcast_search_time) == NETWORK_GAME_CLIENT_TAIL_OFFSET + 4 ? 1 : -1];
typedef char network_game_client_flags_offset_assert[
	offsetof(struct network_game_client, flags) == NETWORK_GAME_CLIENT_TAIL_OFFSET + 0x16 ? 1 : -1];
typedef char network_game_client_last_precache_time_offset_assert[
	offsetof(struct network_game_client, last_precache_time) == NETWORK_GAME_CLIENT_TAIL_OFFSET + 0xC ? 1 : -1];
typedef char message_client_broadcast_game_search_size_assert[
	sizeof(struct message_client_broadcast_game_search) == 0xC ? 1 : -1];
typedef char message_client_ping_size_assert[
	sizeof(struct message_client_ping) == 8 ? 1 : -1];
/* (port: and a Discord account's pass and proof after the Xbox's 0x70) */
typedef char message_client_join_game_request_size_assert[
	sizeof(struct message_client_join_game_request) == 0x70 + 170 + 64 ? 1 : -1];
typedef char network_game_client_seconds_to_game_start_offset_assert[
	offsetof(struct network_game_client, seconds_to_game_start) == NETWORK_GAME_CLIENT_TAIL_OFFSET + 0x10 ? 1 : -1];
typedef char network_game_client_error_offset_assert[
	offsetof(struct network_game_client, error) == NETWORK_GAME_CLIENT_TAIL_OFFSET + 0x14 ? 1 : -1];
typedef char network_game_client_connection_silent_offset_assert[
	offsetof(struct network_game_client, connection_silent) == NETWORK_GAME_CLIENT_TAIL_OFFSET + 0x18 ? 1 : -1];

/* ---------- prototypes */

static boolean check_networking_and_generate_error(
	void);
static void network_game_client_set_error(
	struct network_game_client *client,
	word error);
static boolean add_advertised_game(
	struct network_advertised_game *available_games,
	struct message_server_game_advertise *advertisement);
static boolean network_game_client_process_incoming_messages(
	struct network_game_client *client);
static boolean network_game_client_process_messages(
	struct network_game_client *client,
	boolean budgeted);
static boolean network_game_client_process_last_messages(
	struct network_game_client *client);
static void network_game_client_update_precache_status(
	struct network_game_client *client);
static boolean network_game_client_map_name_is_valid(
	char const *map_name,
	long size);
static boolean network_game_client_game_record_is_valid(
	struct network_game *game);
static boolean network_game_client_idle_searching(
	struct network_game_client *client);
static boolean network_game_client_idle_joining(
	struct network_game_client *client);
static boolean network_game_client_idle_pregame(
	struct network_game_client *client);
static boolean network_game_client_idle_ingame(
	struct network_game_client *client);
static boolean network_game_client_idle_postgame(
	struct network_game_client *client);

/* ---------- globals */

/* the host's game time when it told this client to start a game in
progress, 16 bits of it (0 for a game starting); and whether the first game
update is to bring the host's time (at every start) */
long network_game_client_late_join_time;
static boolean network_game_client_late_join_clock_pending;
/* port: the host said to start while this machine was still downloading the
map (map sharing): started once the map is there, network_game_client_idle_pregame,
if the game is still the one put off (its map, and the games played before
it: the host may end it, or go on to another, meanwhile) */
static boolean network_game_client_start_pending;
static char network_game_client_start_pending_map[0x80];
static long network_game_client_start_pending_games;

/* whether the automated tests' join has told the player why a host cannot
be joined (network_game_client_join_first_available_game) */
static boolean network_game_client_incompatibility_told;

/* when each controller last asked the host for its player in the pregame
(network_game_client_add_player): the pregame screen asks every frame, and
is answered once every half second. A removal asked for since lets the next
ask go at once, so that a quick join again is not taken for a repeat. */
static unsigned long network_game_client_add_player_request_times[MAXIMUM_NUMBER_OF_LOCAL_PLAYERS];
static boolean network_game_client_add_player_requested[MAXIMUM_NUMBER_OF_LOCAL_PLAYERS];

/* since when the network has been down in a game (while
network_game_client_link_down is set): network_game_client_network_lost */
static unsigned long network_game_client_link_down_time;
static unsigned long network_game_client_link_checked_time;
static boolean network_game_client_link_down;

/* each advertised game's host's network version and netcode, and the game's
kind (game_modes_kind), by its place in the client's available_games
(HALO_PORT_NETWORK_VERSION) */
static struct
{
	word version;
	byte flags;
	byte kind;
} network_game_client_advertised_versions[MAXIMUM_NETWORK_ADVERTISED_GAMES];

struct network_game_client network_game_client_dont_use_directly;
boolean allow_out_of_sync = FALSE;
boolean network_game_client_dont_use_directly_in_use = FALSE;

/* ---------- public code */

/* transport_network_available asks the system for its interfaces
(getifaddrs): asked at most once a second, not every frame */
static boolean network_game_client_network_available(
	void)
{
	static boolean asked = FALSE;
	static boolean available;
	static unsigned long ask_time;
	unsigned long now = system_milliseconds();

	if (!asked || now - ask_time >= 1000)
	{
		available = transport_network_available();
		ask_time = now;
		asked = TRUE;
	}

	return available;
}

/* port: whether the network has been down too long for a game to go on. In
a game, a network that goes down for a moment (the IPv4 address lost while
Wi-Fi roams or DHCP renews) does not end it at once: only one down for as
long as the host waits for a machine it hears nothing from
(NETWORK_GAME_CLIENT_LINK_DOWN_TIMEOUT). Out of a game it ends at once
(check_networking_and_generate_error). */
static boolean network_game_client_network_lost(
	void)
{
	unsigned long now = system_milliseconds();
	boolean lost = FALSE;

	/* (a time down from before the last check a moment ago is not counted:
	the network was not asked in between) */
	if (now - network_game_client_link_checked_time > 2000)
	{
		network_game_client_link_down = FALSE;
	}
	network_game_client_link_checked_time = now;

	if (network_game_client_network_available())
	{
		if (network_game_client_link_down)
		{
			network_event(
				"the network is back after %lu ms",
				now - network_game_client_link_down_time);
			network_game_client_link_down = FALSE;
		}
	}
	else if (!network_game_client_link_down)
	{
		network_event("the network went down (in game); waiting for it to come back");
		network_game_client_link_down = TRUE;
		network_game_client_link_down_time = now;
	}
	else if (now - network_game_client_link_down_time >= NETWORK_GAME_CLIENT_LINK_DOWN_TIMEOUT)
	{
		lost = TRUE;
	}

	return lost;
}

static boolean check_networking_and_generate_error(
	void)
{
	boolean connected = TRUE;

	if (!network_game_is_splitscreen_local())
	{
		connected = network_game_client_network_available();

		if (!connected)
		{
			error(_error_silent, "network connection went down!");
			display_error_when_main_menu_loaded(6);
		}
	}

	return connected;
}

void network_game_client_dispose(
	struct network_game_client *client)
{
	if (client)
	{
		if (client->connection)
			network_connection_delete(client->connection);

		match_assert(
			"c:\\halo\\SOURCE\\networking\\network_client_manager.c",
			0xB2,
			network_game_client_dont_use_directly_in_use);

		network_game_client_dont_use_directly_in_use = FALSE;
	}

	network_event("network client disposed");

	return;
}

long unstrip_player_index(
	long player_index)
{
	struct data_iterator iterator;
	long result = NONE;

	data_iterator_new(&iterator, player_data);

	while (data_iterator_next(&iterator))
	{
		if (DATUM_INDEX_TO_ABSOLUTE_INDEX(iterator.datum_index) ==
			DATUM_INDEX_TO_ABSOLUTE_INDEX(player_index))
		{
			result = iterator.datum_index;
			break;
		}
	}

	return result;
}

short network_game_client_get_state(
	struct network_game_client *client,
	short *state_data)
{
	unsigned long elapsed_time;

	match_assert(
		"c:\\halo\\SOURCE\\networking\\network_client_manager.c",
		0xF9,
		client);

	if (state_data)
	{
		*state_data = 0;

		if (client->state == _network_game_client_state_joining)
		{
			elapsed_time = system_milliseconds() * 100 -
				client->connection_attempt_time * 100;
			*state_data = (short)(elapsed_time / 120000);
		}
	}

	return client->state;
}

boolean network_game_client_set_machine(
	struct network_game_client *client,
	struct network_machine *machine)
{
	match_assert(
		"c:\\halo\\SOURCE\\networking\\network_client_manager.c",
		0x1E1,
		client && (client->machine_index<MAXIMUM_NETWORK_MACHINE_COUNT) && network_machine_is_valid(machine));

	/* port: the assert is not checked in release builds */
	if (client->machine_index >= MAXIMUM_NETWORK_MACHINE_COUNT)
		return FALSE;
	csmemcpy(
		&client->game.machines[client->machine_index],
		machine,
		sizeof(*machine));

	return TRUE;
}

void network_game_client_switch_to_postgame(
	struct network_game_client *client)
{
	match_assert(
		"c:\\halo\\SOURCE\\networking\\network_client_manager.c",
		0x48C,
		client);

	game_engine_switch_to_postgame();
	client->state = _network_game_client_state_postgame;
	network_event("switching to postgame");

	return;
}

boolean network_game_client_switch_to_pregame(
	struct network_game_client *client)
{
	match_assert(
		"c:\\halo\\SOURCE\\networking\\network_client_manager.c",
		0x499,
		client);

	if (client->state != _network_game_client_state_pregame)
	{
		network_game_reset_for_next_round(&client->game, TRUE);
		network_connection_keep_alive(client->connection);
		client->next_update_number = 0;
		client->join_in_progress = TRUE;
		client->connection_silent = FALSE;
		client->state = _network_game_client_state_pregame;
		network_game_client_late_join_time = 0;
		network_game_client_late_join_clock_pending = FALSE;
		/* port: a start put off is not this game's */
		network_game_client_start_pending = FALSE;
		network_event("switching to pregame");
		network_game_reset_to_pregame_ui();
		network_connection_keep_alive(client->connection);
	}

	return TRUE;
}

void network_game_client_countdown_timer_update(
	struct network_game_client *client,
	short seconds_to_game_start)
{
	match_assert(
		"c:\\halo\\SOURCE\\networking\\network_client_manager.c",
		0x5C3,
		client);

	client->seconds_to_game_start = seconds_to_game_start;

	return;
}

boolean network_game_client_advertised_game_is_valid(
	struct network_advertised_game *advertised_game)
{
	boolean valid = TRUE;

	if (!advertised_game->valid ||
		(long)(system_milliseconds() - advertised_game->update_time) > 6000)
	{
		valid = FALSE;
	}

	return valid;
}

void network_game_client_game_shutdown(
	struct network_game_client *client)
{
	match_assert(
		"c:\\halo\\SOURCE\\networking\\network_client_manager.c",
		0x3FC,
		client);

	network_game_client_set_error(
		client,
		_network_game_client_error_server_shutdown);

	network_event("the game host is shutting down");
	network_game_client_all_local_players_have_quit();

	return;
}

struct network_machine *network_game_client_get_machine(
	struct network_game_client *client)
{
	if (client && client->machine_index < MAXIMUM_NETWORK_MACHINE_COUNT)
		return &client->game.machines[client->machine_index];

	return NULL;
}

short network_game_client_get_machine_index(
	struct network_game_client *client)
{
	match_assert(
		"c:\\halo\\SOURCE\\networking\\network_client_manager.c",
		0x1FD,
		client);

	return client->machine_index;
}

struct network_advertised_game *network_game_client_get_available_games(
	struct network_game_client *client)
{
	match_assert(
		"c:\\halo\\SOURCE\\networking\\network_client_manager.c",
		0x2AC,
		client);

	return client->available_games;
}

short network_game_client_get_error(
	struct network_game_client *client)
{
	match_assert(
		"c:\\halo\\SOURCE\\networking\\network_client_manager.c",
		0x2B4,
		client);

	return client->error;
}

short network_game_client_get_seconds_to_game_start(
	struct network_game_client *client)
{
	match_assert(
		"c:\\halo\\SOURCE\\networking\\network_client_manager.c",
		0x2BC,
		client);

	return client->seconds_to_game_start;
}

void network_game_client_keep_alive(
	struct network_game_client *client)
{
	network_connection_keep_alive(client->connection);

	return;
}

struct network_connection *network_game_client_get_connection(
	struct network_game_client *client)
{
	match_assert(
		"c:\\halo\\SOURCE\\networking\\network_client_manager.c",
		0x4B3,
		client);

	return client->connection;
}

void network_game_client_get_remote_server_address(
	struct network_game_client *client,
	struct transport_address *address)
{
	match_assert(
		"c:\\halo\\SOURCE\\networking\\network_client_manager.c",
		0x4BC,
		client);

	network_connection_get_address(client->connection, address, NULL);

	return;
}

struct network_game *network_game_client_get_game(
	struct network_game_client *client)
{
	match_assert(
		"c:\\halo\\SOURCE\\networking\\network_client_manager.c",
		0x4CD,
		client);

	return &client->game;
}

boolean network_game_client_server_has_started_game(
	struct network_game_client *client)
{
	match_assert(
		"c:\\halo\\SOURCE\\networking\\network_client_manager.c",
		0x4D5,
		client);

	return client->next_update_number > 0;
}

long network_game_client_get_next_update_number(
	struct network_game_client *client)
{
	match_assert(
		"c:\\halo\\SOURCE\\networking\\network_client_manager.c",
		0x4DD,
		client);

	return client->next_update_number;
}

boolean network_game_client_write(
	struct network_connection *connection,
	message_header *message,
	word message_size,
	struct transport_address *address,
	boolean reliable)
{
	return network_connection_write(
		connection,
		message,
		message_size,
		address,
		reliable);
}

boolean network_game_client_idle(
	struct network_game_client *client)
{
	boolean success = FALSE;

	match_assert(
		"c:\\halo\\SOURCE\\networking\\network_client_manager.c",
		0xC6,
		client);

	switch (client->state)
	{
	case _network_game_client_state_searching:
		if (!(success = network_game_client_idle_searching(client)))
			network_event("network_game_client_idle_searching() failed");
		break;

	case _network_game_client_state_joining:
		if (!(success = network_game_client_idle_joining(client)))
			network_event("network_game_client_idle_joining() failed");
		break;

	case _network_game_client_state_pregame:
		if (!(success = network_game_client_idle_pregame(client)))
			network_event("network_game_client_idle_pregame() failed");
		break;

	case _network_game_client_state_ingame:
		if (!(success = network_game_client_idle_ingame(client)))
			network_event("network_game_client_idle_ingame() failed");
		break;

	case _network_game_client_state_postgame:
		if (!(success = network_game_client_idle_postgame(client)))
			network_event("network_game_client_idle_postgame() failed");
		break;

	default:
		match_assert(
			"c:\\halo\\SOURCE\\networking\\network_client_manager.c",
			0xEE,
			!"unknown client state");
		break;
	}

	return success;
}

boolean network_game_client_game_settings_updated(
	struct network_game_client *client,
	struct network_game *message_packet)
{
	match_assert(
		"c:\\halo\\SOURCE\\networking\\network_client_manager.c",
		0x375,
		client && message_packet);

	if (message_packet->machine_count >= 0 &&
		message_packet->machine_count <= MAXIMUM_NETWORK_MACHINE_COUNT &&
		message_packet->player_count >= 0 &&
		message_packet->player_count <= MAXIMUM_NUMBER_OF_PLAYERS &&
		/* port: and the most players, which caps player_count as players are
		added (network_game_add_player) */
		message_packet->maximum_players > 0 &&
		message_packet->maximum_players <= MAXIMUM_NUMBER_OF_PLAYERS &&
		network_game_client_map_name_is_valid(message_packet->map.name, sizeof(message_packet->map.name)) &&
		VALID_INDEX(message_packet->difficulty, NUMBER_OF_GAME_DIFFICULTY_LEVELS))
	{
		struct network_game previous_game;

		/* port: the record's players: no two the same machine's same
		controller, no machine with more than its local players; the
		strings ending in their fields */
		if (!network_game_client_game_record_is_valid(message_packet))
		{
			network_event("invalid message_server_game_settings_update message received: its players");
			return FALSE;
		}
		if (csstrcmp(message_packet->map.name, client->game.map.name) ||
			message_packet->map.version != client->game.map.version)
		{
			char build[0x20];

			/* port: a map this machine has not (a Custom Edition map not in
			its custom_maps folder, or another version of it than the host's,
			say): the player told which and where to copy it (the main menu's
			error, in place of the failed join's), and the game left, rather
			than precaching it, which would give the damaged disc error
			(cache_files.c) */
			if (!network_game_is_splitscreen_local() &&
				!cache_files_map_present(message_packet->map.name, (unsigned long)message_packet->map.version))
			{
				return FALSE;
			}

			/* port: a map of a build this version does not play with others
			(its objects would not be the host's): said, and the game left */
			if (!network_game_is_splitscreen_local() &&
				!cache_files_map_plays_multiplayer(message_packet->map.name, build))
			{
				cache_files_show_multiplayer_unavailable(message_packet->map.name, build);
				/* (the menu's error the join's, not the connection lost that
				the failure would otherwise give) */
				display_error_when_main_menu_loaded(_error_network_failed_to_join_game);
				return FALSE;
			}
			network_event("precaching map '%s'...", message_packet->map.name);
			main_set_multiplayer_map_name(message_packet->map.name);
		}

		csmemcpy(&previous_game, &client->game, sizeof(client->game));
		csmemcpy(&client->game, message_packet, sizeof(client->game));
		csmemcpy(
			&client->game.local_data,
			&previous_game.local_data,
			sizeof(client->game.local_data));

		network_event(
			"received updated game settings from the server; there are %d players on %d machines in the game",
			message_packet->player_count,
			message_packet->machine_count);
		network_event(
			"player count %d machine count %d",
			message_packet->player_count,
			message_packet->machine_count);

		return TRUE;
	}

	network_event(
		"invalid message_server_game_settings_update message received player count %d machine count %d difficulty %d",
		message_packet->player_count,
		message_packet->machine_count,
		message_packet->difficulty);

	return FALSE;
}

void network_game_client_new_advertised_game(
	struct network_game_client *client,
	struct message_server_game_advertise *message_packet)
{
	match_assert(
		"c:\\halo\\SOURCE\\networking\\network_client_manager.c",
		0x2FC,
		client && message_packet);

	add_advertised_game(client->available_games, message_packet);

	return;
}

boolean network_game_client_request_remove_player(
	struct network_game_client *client,
	struct network_player *player)
{
	boolean success = TRUE;
	struct message_client_remove_player_request remove_player_request;
	message_header *message;

	match_assert(
		"c:\\halo\\SOURCE\\networking\\network_client_manager.c",
		0x208,
		client && network_player_is_valid(player));
	match_vassert(
		"c:\\halo\\SOURCE\\networking\\network_client_manager.c",
		0x209,
		client->machine_index < MAXIMUM_NETWORK_MACHINE_COUNT &&
			client->game.machines[client->machine_index].machine_index ==
			player->machine_index,
		"client's can only remove players from their own machines");

	network_event(
		"requesting a player removal (controller index #%d)",
		player->controller_index);

	/* (the controller's next join goes at once: network_game_client_add_player) */
	if (player->controller_index >= 0 &&
		player->controller_index < MAXIMUM_NUMBER_OF_LOCAL_PLAYERS)
	{
		network_game_client_add_player_requested[player->controller_index] = FALSE;
	}

	switch (client->state)
	{
	case _network_game_client_state_searching:
	case _network_game_client_state_joining:
		network_event("can't remove players from a game until after a game is joined");
		success = FALSE;
		break;

	case _network_game_client_state_pregame:
		csmemcpy(&remove_player_request, player, sizeof(remove_player_request));

		message = create_network_game_message(
			_message_client_remove_player_request_pregame,
			&remove_player_request,
			sizeof(remove_player_request));
		if (message)
		{
			success = network_game_client_write(
				client->connection,
				message,
				GET_MESSAGE_SIZE(*message),
				NULL,
				1);
		}
		else
		{
			network_event("failed to create a message_client_remove_player_request_pregame mesage");
			success = FALSE;
		}
		break;

	case _network_game_client_state_ingame:
		csmemcpy(&remove_player_request, player, sizeof(remove_player_request));

		message = create_network_game_message(
			_message_client_remove_player_request_ingame,
			&remove_player_request,
			sizeof(remove_player_request));
		if (message)
		{
			success = network_game_client_write(
				client->connection,
				message,
				GET_MESSAGE_SIZE(*message),
				NULL,
				1);
		}
		else
		{
			network_event("failed to create a message_client_remove_player_request_ingame message");
			success = FALSE;
		}
		break;

	case _network_game_client_state_postgame:
		csmemcpy(&remove_player_request, player, sizeof(remove_player_request));

		message = create_network_game_message(
			_message_client_remove_player_request_postgame,
			&remove_player_request,
			sizeof(remove_player_request));
		if (message)
		{
			if (!(success = network_game_client_write(
				client->connection,
				message,
				GET_MESSAGE_SIZE(*message),
				NULL,
				1)))
			{
				network_event("network_game_client_write() failed while sending a message_client_remove_player_request_postgame message");
			}
		}
		else
		{
			network_event("failed to create a message_client_remove_player_request_postgame message");
			success = FALSE;
		}
		break;

	default:
		network_event("client is in an unknown state");
		break;
	}

	return success;
}

boolean network_game_client_add_player(
	struct network_game_client *client,
	short local_player_index)
{
	boolean success = TRUE;
	struct network_player player;
	struct message_client_add_player_request add_player_request;
	struct player_profile profile;
	message_header *message;

	match_assert(
		"c:\\halo\\SOURCE\\networking\\network_client_manager.c",
		0x530,
		client && (local_player_index>=0) && (local_player_index<MAXIMUM_NUMBER_OF_LOCAL_PLAYERS));

	/* port: not until the host's settings name the game's map, and a game
	in progress whose map is being downloaded not until it is (map sharing:
	port/linux/game/map_share.c); asked again each frame */
	if (map_share_holds_players())
		return TRUE;

	/* port: the pregame screen asks each frame until the host's settings
	have the player: once every half second, not each frame (the host
	refused and logged each repeat) */
	if (client->state == _network_game_client_state_pregame)
	{
		unsigned long now = system_milliseconds();

		if (network_game_client_add_player_requested[local_player_index] &&
			now - network_game_client_add_player_request_times[local_player_index] < 500)
		{
			return TRUE;
		}
		network_game_client_add_player_requested[local_player_index] = TRUE;
		network_game_client_add_player_request_times[local_player_index] = now;
	}

	player_ui_get_active_player_profile(local_player_index, &profile);

	player.controller_index = (char)local_player_index;
	player.machine_index = (char)client->machine_index;
	ustrncpy(player.name, profile.player_name, NETWORK_PLAYER_NAME_LENGTH - 1);
	player.name[NETWORK_PLAYER_NAME_LENGTH - 1] = 0;
	player.primary_color_index = profile.primary_color_index;
	player.icon_index = NONE;
	player.team_index = NONE;
	player.player_list_index = NONE;

	network_event(
		"requesting a player addition (controller index #%d)",
		player.controller_index);

	switch (client->state)
	{
	case _network_game_client_state_searching:
	case _network_game_client_state_joining:
		network_event("can't add players to a game until after a game is joined");
		success = FALSE;
		break;

	case _network_game_client_state_pregame:
		csmemcpy(&add_player_request, &player, sizeof(add_player_request));

		message = create_network_game_message(
			_message_client_add_player_request_pregame,
			&add_player_request,
			sizeof(add_player_request));
		if (message)
		{
			if (!(success = network_game_client_write(
				client->connection,
				message,
				GET_MESSAGE_SIZE(*message),
				NULL,
				1)))
			{
				network_event("network_game_client_write() failed while sending a message_client_add_player_request_pregame message");
			}
		}
		else
		{
			network_event("failed to create a message_client_add_player_request_pregame message");
		}
		break;

	case _network_game_client_state_ingame:
		csmemcpy(&add_player_request, &player, sizeof(add_player_request));

		message = create_network_game_message(
			_message_client_add_player_request_ingame,
			&add_player_request,
			sizeof(add_player_request));
		if (message)
		{
			if (!(success = network_game_client_write(
				client->connection,
				message,
				GET_MESSAGE_SIZE(*message),
				NULL,
				1)))
			{
				network_event("network_game_client_write() failed while sending a message_client_add_player_request_ingame message");
			}
		}
		else
		{
			network_event("failed to create a message_client_add_player_request_ingame message");
		}
		break;

	case _network_game_client_state_postgame:
		network_event("client tried to add a new player in post-game");
		success = FALSE;
		break;

	default:
		network_event("client is in an unknown state");
		break;
	}

	return success;
}

boolean network_game_client_handle_game_update(
	struct network_game_client *client,
	struct message_server_game_update *message_packet)
{
	match_assert(
		"c:\\halo\\SOURCE\\networking\\network_client_manager.c",
		0x40D,
		client && message_packet);

	/* (every machine ticks on its own clock with the inputs the host relays,
	network_distributed.c: the host's game update carries no actions, and
	keeps only the count of updates, which a machine that joined the game in
	progress takes up where it is) */
	client->next_update_number = message_packet->update_number + 1;
	/* (the host's time at the start, and a game in progress's past 16 bits
	of ticks: the host's whole time, if it is ahead; never back, which the
	host would take for old messages) */
	/* port: and never one the game's arithmetic on its time (a second more,
	a time limit) could take past a long */
	if (message_packet->game_time < 0 || message_packet->game_time > 0x3FFFFFFF)
	{
		network_event("ignoring the host's game tick #%ld", message_packet->game_time);
	}
	else if (network_game_client_late_join_clock_pending)
	{
		network_game_client_late_join_clock_pending = FALSE;
		if (message_packet->game_time > game_time_get())
		{
			game_time_set_distributed(message_packet->game_time);
			network_event("the game in progress is at game tick #%ld", message_packet->game_time);
		}
	}
	/* port: and whenever the host's time is more than a second ahead: the
	clock catches up no more than a second (game_time_update), so a stall
	longer than that (the app in the background, a hitch) left this machine
	behind the host for the rest of the game. The host's time is when it
	sent the update, before this machine's by the way there, so a clock in
	step is never moved; one behind is, at the next update (once a second at
	least) */
	else if (message_packet->game_time > game_time_get() + TICKS_PER_SECOND)
	{
		network_event(
			"the host's clock is %ld ticks ahead of this machine's; taking its game tick #%ld",
			message_packet->game_time - game_time_get(),
			message_packet->game_time);
		game_time_set_distributed(message_packet->game_time);
	}

	return TRUE;
}

/* port: not while the map is still being downloaded (map sharing): loaded
then, it is not there (the damaged disc error, for good). The game starts
once it is there (network_game_client_idle_pregame). A host of this version
starts no machine before its download is done; one of before may. */
boolean network_game_client_begin_game(
	struct network_game_client *client)
{
	if (client && map_share_downloading(client->game.map.name))
	{
		network_game_client_start_pending = TRUE;
		csstrncpy(network_game_client_start_pending_map, client->game.map.name,
			sizeof(network_game_client_start_pending_map) - 1);
		network_game_client_start_pending_map[sizeof(network_game_client_start_pending_map) - 1] = 0;
		network_game_client_start_pending_games = client->game.number_of_games_played;
		network_event("the host started the game while its map is being downloaded: joining it once the map is there");
		return TRUE;
	}
	return network_game_client_game_has_started(client);
}

boolean network_game_client_game_has_started(
	struct network_game_client *client)
{
	/* (the host's time came with the start: the loading below is counted) */
	unsigned long loading_started = system_milliseconds();

	match_assert(
		"c:\\halo\\SOURCE\\networking\\network_client_manager.c",
		0x3B0,
		client && (client->state == _network_game_client_state_pregame));

	client->seconds_to_game_start = NONE;
	network_connection_keep_alive(client->connection);

	if (network_game_create_game_objects(&client->game))
	{
		long network_player_index;

		/* (this machine's players need not be in adjacent slots) */
		for (network_player_index = 0;
			network_player_index < MAXIMUM_NUMBER_OF_PLAYERS;
			network_player_index++)
		{
			struct network_player *player = &client->game.players[network_player_index];

			if (network_player_is_valid(player) &&
				player->machine_index == client->machine_index)
			{
				local_player_set_player_index(
					player->controller_index,
					unstrip_player_index(player->player_list_index));
			}
		}

		network_connection_keep_alive(client->connection);

		{
			struct message_client_loaded loaded = {0};
			message_header *message = create_network_game_message(
				_message_client_loaded,
				&loaded,
				sizeof(loaded));

			if (message)
			{
				if (network_game_client_write(
					client->connection,
					message,
					GET_MESSAGE_SIZE(*message),
					NULL,
					1))
				{
					network_event("local machine is loaded & ready to play");

					client->state = _network_game_client_state_ingame;
					client->next_update_number = 0;
					client->connection_silent = FALSE;

					ui_widgets_close_all();
					game_time_start();
					/* (a game in progress: the host's time when it said to start,
					and the ticks this machine spent loading it) */
					if (network_game_client_late_join_time > 0)
					{
						unsigned long elapsed = system_milliseconds() - loading_started;
						/* (no longer than a minute: a stalled clock counts nothing) */
						long ticks = elapsed < 60000UL ? (long)(elapsed * TICKS_PER_SECOND / 1000) : 0;

						game_time_set_distributed(network_game_client_late_join_time + ticks);
						network_event("joined the game in progress at game tick #%ld",
							network_game_client_late_join_time + ticks);
					}
					network_game_client_late_join_time = 0;
					/* (the clock waits for the host's first game update, which
					brings the host's time, game_time_update: at a start the host
					ticks only once every machine has loaded) */
					network_game_client_late_join_clock_pending =
						game_connection() == _game_connection_network_client;
					game_initial_pulse();
				}
				else
				{
					network_event("network_game_client_write() failed while sending a message_client_loaded message");
				}
			}
			else
			{
				network_event("failed to create a message_client_loaded message");
			}
		}
	}
	else
	{
		network_event("failed to load the necessary game data");
	}

	return client->state == _network_game_client_state_ingame;
}

boolean network_game_client_remove_player(
	struct network_game_client *client,
	struct network_player *player,
	long reason)
{
	boolean success = FALSE;
	long player_index = NONE;
	long network_player_index;

	match_assert(
		"c:\\halo\\SOURCE\\networking\\network_client_manager.c",
		0x273,
		client && player);

	for (network_player_index = 0;
		network_player_index < MAXIMUM_NUMBER_OF_PLAYERS;
		network_player_index++)
	{
		if (network_player_is_valid(&client->game.players[network_player_index]) &&
			client->game.players[network_player_index].machine_index == player->machine_index &&
			client->game.players[network_player_index].controller_index == player->controller_index)
		{
			player_index = unstrip_player_index(
				client->game.players[network_player_index].player_list_index);
			success = network_game_remove_player(&client->game, player);
			break;
		}
	}

	if (success && client->game.local_data.game_objects_loaded)
	{
		if (player_index && player_index != NONE)
		{
			struct player_datum *player_datum = player_get(player_index);

			if (reason != NONE)
			{
				/* (a quit time this clock has passed would never be reached,
				game.c) */
				reason = MAX(reason, game_time_get() + 1);
				error(
					_error_silent,
					"%x quit of of game at tick %d (now %d)",
					player_index,
					reason,
					game_time_get());
				player_datum->quit_out_of_game_time = reason;
				/* (and is no longer its machine's: a machine that joins the
				game in progress at its index fills the list afresh,
				players.c) */
				machine_remove_player(player_index);
			}

			for (network_player_index = 0;
				network_player_index < MAXIMUM_NUMBER_OF_PLAYERS;
				network_player_index++)
			{
				if (network_player_is_valid(&client->game.players[network_player_index]) &&
					client->game.players[network_player_index].machine_index == client->machine_index)
				{
					break;
				}
			}

			/* port: a local player who leaves while another remains no longer has
			its slot: its entry in the local players is cleared here, as the datum
			stays until a joiner takes the slot (network_game_spawn_player), which
			then deletes it and makes another with a new identifier, and the entry
			would name a player that no longer exists (player_get halts on it). The
			last to leave ends the game, whose sounds and views still name it until
			then, so its entry stays */
			if (network_player_index != MAXIMUM_NUMBER_OF_PLAYERS &&
				player->machine_index == client->machine_index &&
				player->controller_index >= 0 && player->controller_index < MAXIMUM_LOCAL_PLAYERS &&
				local_player_get_player_index(player->controller_index) == player_index)
			{
				local_player_set_player_index(player->controller_index, NONE);
				/* (and its control, which names its unit: the datum no longer has a
				local player index, so the host deleting the unit does not reset it
				(network_player_detach_unit), and player_control_update would get a
				unit that is gone) */
				player_control_new_unit(player->controller_index, NONE);
			}

			/* port: a dedicated server's own machine never has a player
			(port/linux/game/server_director.c): it leaves only when a player
			of its own goes, which none is */
			if (network_player_index == MAXIMUM_NUMBER_OF_PLAYERS &&
				(!halo_server_mode() || player->machine_index == client->machine_index))
			{
				network_game_client_all_local_players_have_quit();
				network_event("no local players remain in the game, exiting the game now");
			}
		}
		else
		{
			error(
				_error_silent,
				"network game tried to delete a player with a phony player index (#0x%08lX)",
				player_index);
			success = FALSE;
		}
	}

	return success;
}

void network_game_client_accepted_into_game(
	struct network_game_client *client,
	struct transport_address *source_address,
	struct message_server_machine_accepted *message_packet)
{
	match_assert(
		"c:\\halo\\SOURCE\\networking\\network_client_manager.c",
		0x327,
		client && source_address && message_packet && (client->state == _network_game_client_state_joining));

	if (message_packet->machine_index >= 0 &&
		message_packet->machine_index < MAXIMUM_NETWORK_MACHINE_COUNT)
	{
		struct message_client_settings_request settings_request;
		message_header *message;

		client->machine_index = message_packet->machine_index;
		client->game.machines[message_packet->machine_index].machine_index =
			(char)message_packet->machine_index;
		client->state = _network_game_client_state_pregame;
		/* port: the next join shows no Discord pass until asked */
		player_link_join_reset();

		network_game_set_random_seed(message_packet->random_seed);
		network_event(
			"successfully joined a net game; our machine is #%d",
			message_packet->machine_index);

		network_game_generate_local_machine_name(settings_request.machine_name);
		settings_request.machine_index = (char)message_packet->machine_index;

		message = create_network_game_message(
			_message_client_settings_request,
			&settings_request,
			sizeof(settings_request));
		if (message)
		{
			if (!network_game_client_write(
				client->connection,
				message,
				GET_MESSAGE_SIZE(*message),
				NULL,
				1))
			{
				network_event("network_game_client_write() failed while sending a message_client_settings_request message");
			}
		}
		else
		{
			network_event("failed to create a message_client_settings_request message");
		}
	}
	else
	{
		network_event("received a message_server_machine_accepted message with a bad machine_index");
	}

	return;
}

boolean network_game_client_update_local_player_data(
	struct network_game_client *client,
	struct network_player *player)
{
	boolean success = FALSE;
	struct network_player player_settings;
	message_header *message;

	match_assert(
		"c:\\halo\\SOURCE\\networking\\network_client_manager.c",
		0x587,
		client && player);
	match_assert(
		"c:\\halo\\SOURCE\\networking\\network_client_manager.c",
		0x588,
		player->machine_index==client->machine_index);
	match_assert(
		"c:\\halo\\SOURCE\\networking\\network_client_manager.c",
		0x589,
		network_player_is_valid(player));

	csmemcpy(&player_settings, player, sizeof(player_settings));

	if (player_settings.team_index == NONE)
		player_settings.team_index = 0;

	message = create_network_game_message(
		_message_client_player_settings_request,
		&player_settings,
		sizeof(player_settings));
	if (message)
	{
		if (network_game_client_write(
			client->connection,
			message,
			GET_MESSAGE_SIZE(*message),
			NULL,
			1))
		{
			success = TRUE;
		}
		else
		{
			network_event("network_game_client_update_local_player_data() failed while sending a message_client_player_settings_request message");
		}
	}

	return success;
}

boolean network_game_client_add_player_to_game(
	struct network_game_client *client,
	struct network_player *player)
{
	boolean success = FALSE;

	match_assert(
		"c:\\halo\\SOURCE\\networking\\network_client_manager.c",
		0x462,
		client && player);

	if (network_player_is_valid(player))
	{
		success = network_game_add_player(&client->game, player);

		if (success)
		{
			if (client->state == _network_game_client_state_ingame)
			{
				/* port: the slot it went in, which in the distributed netcode's
				games need not be the last (network_game_add_player) */
				{
					struct network_player const *added = player;
					long slot;

					/* (the slot network_game_add_player gave it, not one
					worked out from player_count) */
					player = VALID_INDEX(added->player_list_index, MAXIMUM_NUMBER_OF_PLAYERS) ?
						&client->game.players[added->player_list_index] : NULL;
					for (slot = 0; slot < MAXIMUM_NUMBER_OF_PLAYERS; slot++)
					{
						if (network_player_is_valid(&client->game.players[slot]) &&
							client->game.players[slot].machine_index == added->machine_index &&
							client->game.players[slot].controller_index == added->controller_index)
						{
							player = &client->game.players[slot];
							break;
						}
					}
				}

				success = player && network_game_spawn_player(player);

				if (success)
				{
					long player_index = unstrip_player_index(player->player_list_index);

					/* port: a player added to the game in progress gets its team
					and the game type's data, as the players at the start have
					(game_initial_pulse); otherwise it keeps player_new's team 1,
					in free for all the team of the player in slot 1, whose kills
					of it and its of them count as betrayals */
					{
						extern void game_engine_player_added(long player_index);

						game_engine_player_added(player_index);
					}
					if (player->machine_index == client->machine_index)
					{
						local_player_set_player_index(
							player->controller_index,
							player_index);
					}

					update_client_add_player(player_index);

					if (global_network_game_server_get())
						update_server_add_player(player_index);
				}
			}

			if (success)
			{
				network_event(
					"added new player to the game (machine #%d / controller #%d)",
					player->machine_index,
					player->controller_index);
			}
		}
	}

	return success;
}

boolean network_game_client_initiate_join_game(
	struct network_game_client *client,
	struct network_advertised_game *game,
	struct network_join_parameters *join_parameters,
	struct transport_address *address)
{
	boolean success;

	match_assert(
		"c:\\halo\\SOURCE\\networking\\network_client_manager.c",
		0x157,
		client && (client->state == _network_game_client_state_searching) && game && join_parameters && client->connection && !network_connection_connected(client->connection) && (game->platform == network_game_get_local_platform()));

	client->join_in_progress = TRUE;
	client->connect_process = 0;
	client->connection_attempt_time = system_milliseconds();
	/* port: a new join shows no Discord pass until its host asks for one */
	if (!network_game_client_joining_with_link)
		player_link_join_reset();

	csmemcpy(
		&client->join_parameters,
		join_parameters,
		sizeof(*join_parameters));

	success = network_connection_connect(client->connection, address, 0);

	if (success == TRUE)
	{
		/* port: the join's wait counted from the connection made, not from
		before a connect that took seconds (idle_joining) */
		network_connection_keep_alive(client->connection);
		client->state = _network_game_client_state_joining;
		network_event(
			"attempting to connect to game @ %s",
			transport_address_to_string(address));
	}
	else
	{
		client->join_in_progress = FALSE;
		display_error_when_main_menu_loaded(7);
		network_event(
			"failed attempt to initiate a connection to game @ %s",
			transport_address_to_string(address));
	}

	return success;
}

void network_game_client_ponged(
	struct network_game_client *client,
	struct transport_address *source_address,
	long timestamp)
{
	match_assert(
		"c:\\halo\\SOURCE\\networking\\network_client_manager.c",
		0x307,
		client && source_address);

	if (client->pinging &&
		client->ping_address.address.long_words[0] ==
		source_address->address.long_words[0])
	{
		unsigned long now = system_milliseconds();

		if ((long)(now - (unsigned long)timestamp) >= 0)
		{
			client->average_ping = (word)((client->average_ping *
				client->ping_sample_count + now - timestamp) /
				(client->ping_sample_count + 1));
			client->ping_sample_count += 1;
		}
		else
		{
			network_event("received a pong from the future");
		}
	}
	else
	{
		network_event("received a pong from a system we aren't interested in");
	}

	return;
}

boolean network_game_client_address_matches_server(
	struct network_game_client *client,
	struct transport_address *address)
{
	struct transport_address server_address;
	struct transport_address server_unreliable_address;
	boolean address_matches;

	match_assert(
		"c:\\halo\\SOURCE\\networking\\network_client_manager.c",
		0x2D2,
		client != NULL);
	match_assert(
		"c:\\halo\\SOURCE\\networking\\network_client_manager.c",
		0x2D3,
		client->connection);
	match_assert(
		"c:\\halo\\SOURCE\\networking\\network_client_manager.c",
		0x2D4,
		address != NULL);
	match_vassert(
		"c:\\halo\\SOURCE\\networking\\network_client_manager.c",
		0x2D5,
		address->address.long_words[0],
		"address->address.ipv4_address");

	network_connection_get_address(client->connection, &server_address, &server_unreliable_address);

	/* port: the host's stream's peer, or its datagrams' (internet play's
	tunnel has them at different local addresses: the client's datagram
	socket is connected to the host, so what comes to it is the host's) */
	address_matches = server_address.address.long_words[0] == address->address.long_words[0] ||
		(server_unreliable_address.address.long_words[0] &&
			server_unreliable_address.address.long_words[0] == address->address.long_words[0]);

	return address_matches;
}

boolean network_game_client_request_start_time_change(
	struct network_game_client *client,
	short request_type)
{
	match_assert(
		"c:\\halo\\SOURCE\\networking\\network_client_manager.c",
		0x5A5,
		client);
	match_assert(
		"c:\\halo\\SOURCE\\networking\\network_client_manager.c",
		0x5A6,
		(request_type>=0) && (request_type<NUMBER_OF_GAME_START_REQUESTS));

	if (client->state == _network_game_client_state_pregame)
	{
		struct message_client_game_start_request game_start_request;
		message_header *message;

		game_start_request.request_type = request_type;

		message = create_network_game_message(
			_message_client_game_start_request,
			&game_start_request,
			sizeof(game_start_request));
		if (message)
		{
			if (!network_game_client_write(
				client->connection,
				message,
				GET_MESSAGE_SIZE(*message),
				NULL,
				1))
			{
				network_event("network_game_client_request_start_time_change() failed to send a message_client_game_start_request message");
			}
		}
	}
	else
	{
		network_event("failed to send a message_client_game_start_request because we are not in the pregame state");
	}

	return TRUE;
}

void network_game_client_reset(
	struct network_game_client *client,
	boolean teardown_connection)
{
	match_assert(
		"c:\\halo\\SOURCE\\networking\\network_client_manager.c",
		0x4EE,
		client);

	network_game_invalidate(&client->game);

	client->machine_index = NONE;
	client->state = _network_game_client_state_searching;

	/* (port: or one that was lost, whose socket stays until it is torn down) */
	if (teardown_connection && client->connection &&
		((boolean)network_connection_connected(client->connection) ||
			!network_connection_active(client->connection)))
	{
		if (network_connection_disconnect(client->connection))
		{
			SET_FLAG(
				client->flags,
				_network_game_client_connection_established_bit,
				FALSE);
		}
		else
		{
			network_game_client_set_error(
				client,
				_network_game_client_error_unknown);
			network_event("failed to reinitialize network game client");
		}
	}

	SET_FLAG(client->flags, _network_game_client_join_request_sent_bit, FALSE);
	client->error = _network_game_client_error_none;
	client->join_in_progress = FALSE;
	client->last_broadcast_search_time = 0;
	client->next_update_number = 0;
	client->connection_silent = FALSE;
	client->seconds_to_game_start = NONE;
	network_game_client_late_join_time = 0;
	network_game_client_late_join_clock_pending = FALSE;
	/* port: no start put off */
	network_game_client_start_pending = FALSE;
	network_game_client_incompatibility_told = FALSE;
	csmemset(network_game_client_add_player_requested, 0, sizeof(network_game_client_add_player_requested));

	return;
}

struct network_game_client *network_game_client_create(
	void)
{
	struct network_game_client *client = &network_game_client_dont_use_directly;

	match_assert(
		"c:\\halo\\SOURCE\\networking\\network_client_manager.c",
		0x94,
		!network_game_client_dont_use_directly_in_use);

	network_game_client_dont_use_directly_in_use = TRUE;

	csmemset(
		&network_game_client_dont_use_directly,
		0,
		sizeof(network_game_client_dont_use_directly));

	network_game_client_dont_use_directly.connection = network_connection_new(
		_network_connection_type_client,
		NETWORK_GAME_CLIENT_PORT);

	if (network_game_client_dont_use_directly.connection)
	{
		network_game_client_reset(&network_game_client_dont_use_directly, FALSE);
	}
	else
	{
		network_event("network_game_create_client() failed; could not create network connection");
		network_game_client_dispose(&network_game_client_dont_use_directly);
		client = NULL;
	}

	return client;
}

void network_game_client_rejected_by_game(
	struct network_game_client *client,
	struct transport_address *source_address,
	word rejection_code)
{
	const char *reason = "<unknown>";

	match_assert(
		"c:\\halo\\SOURCE\\networking\\network_client_manager.c",
		0x35A,
		client && source_address);

	client->state = _network_game_client_state_searching;

	switch (rejection_code)
	{
	case _rejection_code_version_too_old:
		reason = "_rejection_code_version_too_old";
		break;

	case _rejection_code_version_too_new:
		reason = "_rejection_code_version_too_new";
		break;

	case _rejection_code_bad_join_token:
		reason = "_rejection_code_bad_join_token";
		break;

	case _rejection_code_bad_password:
		reason = "_rejection_code_bad_password";
		break;

	case _rejection_code_game_is_full:
		reason = "_rejection_code_game_is_full";
		break;

	case _rejection_code_game_is_closed:
		reason = "_rejection_code_game_is_closed";
		break;

	case _rejection_code_blacklisted_machine:
		reason = "_rejection_code_blacklisted_machine";
		break;

	/* port: the port's own */
	case _rejection_code_name_refused:
		reason = "_rejection_code_name_refused";
		break;

	case _rejection_code_vpn_link_required:
		reason = "_rejection_code_vpn_link_required";
		break;

	case _rejection_code_vpn_blocked:
		reason = "_rejection_code_vpn_blocked";
		break;

	case _rejection_code_vpn_link_invalid:
		reason = "_rejection_code_vpn_link_invalid";
		break;

	case _rejection_code_vpn_link_too_new:
		reason = "_rejection_code_vpn_link_too_new";
		break;

	case _rejection_code_vpn_link_in_use:
		reason = "_rejection_code_vpn_link_in_use";
		break;

	case _rejection_code_address_full:
		reason = "_rejection_code_address_full";
		break;
	}

	network_event(
		"unable to join game: reason= #%d/%s",
		rejection_code,
		reason);
	/* port: a dedicated server that refused this machine for its VPN asked
	for a Discord account's pass (or refused the one shown): the join made
	again at once with this machine's (the next it holds), if it holds one
	(player_link.c), on the pregame screen it is on */
	if (player_link_join_refused(rejection_code))
	{
		struct transport_address address;
		struct network_join_parameters join_parameters;
		struct network_advertised_game game;
		boolean joining;

		csmemset(&address, 0, sizeof(address));
		network_connection_get_address(client->connection, &address, NULL);
		csmemcpy(&join_parameters, &client->join_parameters, sizeof(join_parameters));
		network_game_client_reset(client, TRUE);
		csmemset(&game, 0, sizeof(game));
		game.platform = network_game_get_local_platform();
		network_game_client_joining_with_link = TRUE;
		joining = network_game_client_initiate_join_game(client, &game, &join_parameters, &address);
		network_game_client_joining_with_link = FALSE;
		if (joining)
		{
			network_event("joining again with a Discord account's pass");
			network_game_client_joined_again_with_link = TRUE;
			return;
		}
	}
	/* port: why, for the server browser's line (p2p_join_progress) */
	p2p_join_refused(rejection_code);
	/* port: the join went to the pregame screen at once (and a machine
	refused a game in progress, network_game_server_refuse_late_joiner, is in
	it): with no game behind it, it is left for the main menu, which says
	why, rather than sitting in a lobby that will never start */
	display_error_when_main_menu_loaded(
		rejection_code == _rejection_code_game_is_full || rejection_code == _rejection_code_game_is_closed ?
			_error_network_join_game_closed :
			_error_network_failed_to_join_game);
	network_game_abort();
	network_game_client_reset(client, TRUE);

	return;
}

/* ---------- private code */

/* a map name from the host ends within its field and names a map in the maps
folder: it goes into the map's path (cache_files_windows.c) */
static boolean network_game_client_map_name_is_valid(
	char const *map_name,
	long size)
{
	/* (a scenario's tag path, of which the cache takes the name after the
	last backslash: letters, digits and a few more, none that a path reads
	otherwise; the [ ] ( ) + that Custom Edition maps' names use, as in
	[H2]_Lockout, too) */
	char const *character;
	char const *leaf;

	if (!memchr(map_name, '\0', size))
		return FALSE;
	for (character = map_name; *character; character++)
	{
		if (!((*character >= 'a' && *character <= 'z') || (*character >= 'A' && *character <= 'Z') ||
			(*character >= '0' && *character <= '9') || *character == '_' || *character == '-' ||
			*character == '.' || *character == ' ' || *character == '\\' ||
			*character == '[' || *character == ']' || *character == '(' || *character == ')' || *character == '+'))
		{
			return FALSE;
		}
	}
	if (strstr(map_name, ".."))
		return FALSE;
	leaf = strrchr(map_name, '\\');
	leaf = leaf ? leaf + 1 : map_name;
	/* port: and not one of Windows's devices (con, nul, com1...), whatever
	follows it: maps\com1.map opens the serial port there */
	{
		static char const *const devices[] = { "con", "prn", "aux", "nul", "com", "lpt" };
		long stem = (long)strcspn(leaf, ". ");
		short device;

		for (device = 0; device < (short)NUMBEROF(devices); device++)
		{
			if (!_strnicmp(leaf, devices[device], 3) &&
				((device < 4 && stem == 3) || (device >= 4 && stem == 4 && leaf[3] >= '0' && leaf[3] <= '9')))
			{
				return FALSE;
			}
		}
	}
	return *leaf && leaf[strspn(leaf, ". ")] != 0;
}

/* port: the players of a settings record the host sends: each valid one
the only one of its machine's controller (so no machine has more than its
local players); the record's strings made to end in their fields */
static boolean network_game_client_game_record_is_valid(
	struct network_game *game)
{
	short machine_players[HALO_PORT_MAXIMUM_NETWORK_MACHINES][MAXIMUM_LOCAL_PLAYERS];
	short index;

	game->name[NUMBEROF(game->name) - 1] = 0;
	game->variant.human_readable_game_description[NUMBEROF(game->variant.human_readable_game_description) - 1] = 0;
	for (index = 0; index < NUMBEROF(game->machines); index++)
		game->machines[index].name[NUMBEROF(game->machines[index].name) - 1] = 0;
	csmemset(machine_players, 0, sizeof(machine_players));
	for (index = 0; index < NUMBEROF(game->players); index++)
	{
		struct network_player *player = &game->players[index];

		player->name[NUMBEROF(player->name) - 1] = 0;
		if (!network_player_is_valid(player))
			continue;
		if (machine_players[player->machine_index][player->controller_index]++)
			return FALSE;
	}

	return TRUE;
}

static boolean add_advertised_game(
	struct network_advertised_game *available_games,
	struct message_server_game_advertise *advertisement)
{
	boolean success = FALSE;
	boolean open = TEST_FLAG(advertisement->flags, _game_advertisement_open_bit) &&
		advertisement->machine_count < MAXIMUM_NETWORK_MACHINE_COUNT;
	struct network_advertised_game *advertised_game = NULL;
	long game_index;

	for (game_index = 0; game_index < MAXIMUM_NETWORK_ADVERTISED_GAMES; game_index++)
	{
		struct network_advertised_game *current = available_games + game_index;

		if (!network_game_client_advertised_game_is_valid(current))
		{
			csmemset(current, 0, sizeof(*current));
		}
	}

	for (game_index = 0; game_index < MAXIMUM_NETWORK_ADVERTISED_GAMES; game_index++)
	{
		struct network_advertised_game *current = available_games + game_index;

		if (transport_nonce_is_equal(current->nonce, advertisement->nonce))
		{
			advertised_game = current;
			break;
		}
	}

	if (!advertised_game)
	{
		for (game_index = 0; game_index < MAXIMUM_NETWORK_ADVERTISED_GAMES; game_index++)
		{
			struct network_advertised_game *current = available_games + game_index;

			if (!current->valid)
			{
				advertised_game = current;
				break;
			}
		}

		if (!advertised_game && open)
		{
			for (game_index = 0; game_index < MAXIMUM_NETWORK_ADVERTISED_GAMES; game_index++)
			{
				struct network_advertised_game *current = available_games + game_index;

				match_assert(
					"c:\\halo\\SOURCE\\networking\\network_client_manager.c",
					0x61F,
					current->valid);

				if (!current->open)
				{
					advertised_game = current;
					csmemset(advertised_game, 0, sizeof(*advertised_game));
					break;
				}
			}
		}
	}

	if (advertised_game)
	{
		advertised_game->valid = TRUE;
		advertised_game->key = advertisement->key;
		advertised_game->key_id = advertisement->key_id;
		advertised_game->xnaddr = advertisement->xnaddr;

		csmemcpy(
			advertised_game->nonce,
			advertisement->nonce,
			sizeof(advertisement->nonce));

		advertised_game->update_time = system_milliseconds();
		advertised_game->platform = advertisement->platform;
		/* (a host built before there was a version sends zeros: 0) */
		{
			long game_index = advertised_game - available_games;

			network_game_client_advertised_versions[game_index].version = (word)(
				advertisement->__unknown5A[HALO_PORT_ADVERTISED_VERSION_OFFSET] |
				(advertisement->__unknown5A[HALO_PORT_ADVERTISED_VERSION_OFFSET + 1] << 8));
			network_game_client_advertised_versions[game_index].flags =
				advertisement->__unknown5A[HALO_PORT_ADVERTISED_FLAGS_OFFSET];
			network_game_client_advertised_versions[game_index].kind =
				advertisement->__unknown5A[HALO_PORT_ADVERTISED_KIND_OFFSET];
		}

		if (advertisement->game_name[0] != L'\0')
		{
			ustrncpy(
				advertised_game->game_name,
				advertisement->game_name,
				NETWORK_GAME_NAME_LENGTH - 1);
		}
		else
		{
			ustrncpy(
				advertised_game->game_name,
				L"???",
				NETWORK_GAME_NAME_LENGTH - 1);
		}
		advertised_game->game_name[NETWORK_GAME_NAME_LENGTH - 1] = L'\0';

		advertised_game->engine_type = advertisement->engine_type;

		csmemcpy(
			&advertised_game->map,
			&advertisement->map,
			sizeof(advertisement->map));
		/* (from any machine on the network: not trusted to end) */
		advertised_game->map.name[NUMBEROF(advertised_game->map.name) - 1] = '\0';

		advertised_game->machine_count = advertisement->machine_count;
		advertised_game->player_count = advertisement->player_count;
		advertised_game->maximum_player_count = advertisement->maximum_player_count;
		advertised_game->unknown100 = advertisement->unknown100;
		advertised_game->open = open;
		advertised_game->has_teams = TEST_FLAG(
			advertisement->flags,
			_game_advertisement_has_teams_bit);
		advertised_game->oddball_variant =
			advertised_game->engine_type == game_engine_oddball &&
			TEST_FLAG(advertisement->flags, _game_advertisement_oddball_variant_bit);

		success = TRUE;

		network_event(
			"there is %s %s net game with %d players and %d machines",
			advertised_game->open ? "an open" : "a closed",
			advertised_game->platform == _network_game_platform_xbox ? "XBox" :
				(advertised_game->platform == _network_game_platform_pc ? "PC" : "<unknown platform>"),
			advertised_game->player_count,
			advertised_game->machine_count);
	}
	else
	{
		error(
			_error_silent,
			"not fatal, but we have to many active network games cannot add more to the list");
	}

	return success;
}

static boolean network_game_client_process_incoming_messages(
	struct network_game_client *client)
{
	return network_game_client_process_messages(client, TRUE);
}

/* (budgeted: no longer than MAXIMUM_MESSAGE_MILLISECONDS_PER_IDLE, else
every message queued) */
static boolean network_game_client_process_messages(
	struct network_game_client *client,
	boolean budgeted)
{
	boolean success = TRUE;
	word message_packet_size;
	struct transport_address source_address;
	word message_packet[MAXIMUM_NETWORK_MESSAGE_SIZE / sizeof(word)];
	unsigned long start_time = system_milliseconds();

	message_packet_size = sizeof(message_packet);

	while (success && network_connection_read(
		client->connection,
		message_packet,
		&message_packet_size,
		&source_address))
	{
		if (!(success = network_game_client_handle_message(
			client,
			message_packet,
			message_packet_size,
			&source_address)))
		{
			network_event("network_game_client_handle_message() failed in network_game_client_process_incoming_messages()");
		}

		message_packet_size = sizeof(message_packet);
		if (budgeted && system_milliseconds() - start_time >= MAXIMUM_MESSAGE_MILLISECONDS_PER_IDLE)
			break;
	}

	return success;
}

/* port: the messages the connection has queued when it has just failed or
closed: the host's last before it closed the connection (a rejection, which
it follows with the close) came with the close, and the queue holds only
whole messages. TRUE when they took the client out of its state (a
rejection, network_game_client_rejected_by_game, which says why and ends the
game): the connection's failure is then no failure of the client's. */
static boolean network_game_client_process_last_messages(
	struct network_game_client *client)
{
	short state = client->state;

	/* (all of them: the connection goes) */
	if (!network_game_client_process_messages(client, FALSE))
	{
		network_event("network_game_client_process_incoming_messages() failed after the connection failed");
	}

	return client->state != state;
}

static void network_game_client_update_precache_status(
	struct network_game_client *client)
{
	unsigned long now = system_milliseconds();

	if (now - client->last_precache_time > 1000)
	{
		char *map_name = main_get_multiplayer_map_name();

		client->last_precache_time = now;

		if (cache_files_give_time_to_precache(map_name))
		{
			struct message_client_map_is_precached_pregame map_is_precached = {0};
			message_header *message;

			csstrncpy(
				map_is_precached.map_name,
				map_name,
				sizeof(map_is_precached.map_name));

			message = create_network_game_message(
				_message_client_map_is_precached_pregame,
				&map_is_precached,
				sizeof(map_is_precached));
			if (message)
			{
				if (!network_game_client_write(
					client->connection,
					message,
					GET_MESSAGE_SIZE(*message),
					NULL,
					1))
				{
					network_event("network_game_client_write() failed while sending a message_client_graceful_game_exit_pregame message");
				}
			}
		}
	}

	return;
}

static boolean network_game_client_idle_searching(
	struct network_game_client *client)
{
	unsigned long now = system_milliseconds();
	boolean success;

	network_connection_keep_alive(client->connection);

	success = check_networking_and_generate_error();

	if (success == TRUE)
	{
		if (global_network_game_server_get())
		{
			struct network_advertised_game local_game = {0};
			struct network_join_parameters join_parameters;
			struct transport_address server_address;

			server_address.address.long_words[0] = IPV4_LOOPBACK_ADDRESS;
			server_address.port = NETWORK_GAME_SERVER_PORT;
			server_address.address_length = IPV4_ADDRESS_LENGTH;
			local_game.platform = network_game_get_local_platform();

			transport_get_nonce(local_game.nonce, sizeof(local_game.nonce));

			join_parameters.__unknown2 = 0;
			network_game_generate_join_game_token(join_parameters.join_token);

			if (!network_game_client_initiate_join_game(
				client,
				&local_game,
				&join_parameters,
				&server_address))
			{
				success = FALSE;
				display_error_when_main_menu_loaded(7);
				network_event("network_game_client_initiate_join_game() failed");
			}
		}
		else if (!(success = network_connection_idle(client->connection, 5000, NULL)))
		{
			display_error_when_main_menu_loaded(7);
			network_event("network_connection_idle() failed in network_game_client_idle_searching()");
		}
		else if (!(success = network_game_client_process_incoming_messages(client)))
		{
			network_event("network_game_client_process_incoming_messages() failed in network_game_client_idle_searching()");
		}
		else if (now - client->last_broadcast_search_time > 2000)
		{
			if (!global_network_game_server_get())
			{
				struct message_client_broadcast_game_search broadcast_game_search;
				struct transport_address broadcast_address;
				message_header *message;

				broadcast_game_search.port = NETWORK_GAME_CLIENT_PORT;
				/* the search version: the native builds' games are not the Xbox's */
				broadcast_game_search.__unknown2 = HALO_PORT_NETWORK_GAME_MESSAGE_VERSION;
				transport_get_nonce(
					broadcast_game_search.nonce,
					sizeof(broadcast_game_search.nonce));
				broadcast_address.address_length = IPV4_ADDRESS_LENGTH;
				broadcast_address.address.long_words[0] = BROADCAST_IPV4_ADDRESS;
				broadcast_address.port = NETWORK_GAME_SERVER_PORT;

				message = create_network_game_message(
					_message_client_broadcast_game_search,
					&broadcast_game_search,
					sizeof(broadcast_game_search));
				if (message)
				{
					success = network_game_client_write(
						client->connection,
						message,
						GET_MESSAGE_SIZE(*message),
						&broadcast_address,
						0);
					if (success == TRUE)
					{
						network_event("sent out a broadcast game search packet");
						client->last_broadcast_search_time = now;
					}
					else
					{
						network_event("network_game_client_write() failed while sending a message_client_broadcast_game_search message");
					}
				}
				else
				{
					network_event("failed to create a message_client_broadcast_game_search message");
				}
			}
		}
		else if (client->pinging == TRUE && now - client->last_ping_time > 1000)
		{
			struct message_client_ping ping;
			message_header *message;

			ping.timestamp = now;
			ping.port = NETWORK_GAME_CLIENT_PORT;

			message = create_network_game_message(
				_message_client_ping,
				&ping,
				sizeof(ping));
			if (message)
			{
				if (network_game_client_write(
					client->connection,
					message,
					GET_MESSAGE_SIZE(*message),
					&client->ping_address,
					0))
				{
					client->last_ping_time = now;
				}
				else
				{
					network_event("network_game_client_write() failed while sending a message_client_ping message");
				}
			}
			else
			{
				network_event("failed to create a message_client_ping message");
			}
		}
	}

	return success;
}

static boolean network_game_client_idle_joining(
	struct network_game_client *client)
{
	boolean success = check_networking_and_generate_error();

	if (success == TRUE)
	{
		if ((boolean)network_connection_connected(client->connection))
		{
			if (!TEST_FLAG(client->flags, _network_game_client_join_request_sent_bit))
			{
				struct message_client_join_game_request join_game_request;
				message_header *message;

				csmemset(&join_game_request, 0, sizeof(join_game_request));
				network_game_generate_local_machine_name(join_game_request.machine_name);
				csmemcpy(
					join_game_request.join_token,
					client->join_parameters.join_token,
					sizeof(join_game_request.join_token));
				/* (port: and its hardware id, which a host logs and may have
				banned: all 32 digits, unended) */
				{
					char hardware_id[sizeof(join_game_request.hardware_id) + 1];

					p2p_hardware_id(hardware_id, sizeof(hardware_id));
					csmemcpy(join_game_request.hardware_id, hardware_id, csstrlen(hardware_id));
				}
				/* (port: and a Discord account's pass, when the host asked
				for one: bound to this connection, the internet play tunnel's
				with the host's stand-in in 100.64.0.0/10, in network byte
				order) */
				{
					struct transport_address host_address;
					unsigned long host;

					csmemset(&host_address, 0, sizeof(host_address));
					network_connection_get_address(client->connection, &host_address, NULL);
					host = host_address.address.long_words[0];
					player_link_join_fill((host & 0xFFC00000) == 0x64400000 ?
						(host >> 24) | ((host >> 8) & 0xFF00) | ((host << 8) & 0xFF0000) | (host << 24) : 0,
						join_game_request.link_pass, join_game_request.link_proof);
				}

				message = create_network_game_message(
					_message_client_join_game_request,
					&join_game_request,
					sizeof(join_game_request));
				if (message)
				{
					if (network_game_client_write(
						client->connection,
						message,
						GET_MESSAGE_SIZE(*message),
						NULL,
						1))
					{
						SET_FLAG(
							client->flags,
							_network_game_client_join_request_sent_bit,
							TRUE);
					}
					else
					{
						network_event("network_game_client_write() failed to send a message_client_join_game_request message");
					}
				}
				else
				{
					network_event("failed to create a message_client_join_game_request message");
				}
			}

			client->connect_process = 0;
		}
		else if (client->connect_process &&
			system_milliseconds() - client->connection_attempt_time > 120000)
		{
			network_event("client connection process has timed out; aborting connection attempt");
			cancel_connect_process(client->connect_process);
			client->connect_process = 0;

			return FALSE;
		}

		success = network_connection_idle(client->connection, 5000, NULL);

		if (success)
		{
			if (!(success = network_game_client_process_incoming_messages(client)))
			{
				network_event("network_game_client_process_incoming_messages() failed in network_game_client_idle_joining()");
			}
		}
		else
		{
			network_event("network_connection_idle() failed in network_game_client_idle_joining()");
			success = network_game_client_process_last_messages(client);
		}
		/* port: the host's refusal asked for a Discord account's pass and the
		join was made again with it: the refused connection's end, which the
		host closed, is not this join's failure */
		if (network_game_client_joined_again_with_link)
		{
			network_game_client_joined_again_with_link = FALSE;
			success = TRUE;
		}
	}

	return success;
}

static boolean network_game_client_idle_pregame(
	struct network_game_client *client)
{
	boolean success = check_networking_and_generate_error();

	if (success)
	{
		if (network_connection_active(client->connection) &&
			(boolean)network_connection_connected(client->connection))
		{
			network_game_client_update_precache_status(client);

			if (!(success = network_connection_idle(client->connection, 15000, NULL)))
			{
				network_event("network_connection_idle() failed in network_game_client_idle_pregame()");
				if (network_game_client_process_last_messages(client))
				{
					return TRUE;
				}
			}
			else if (!(success = network_game_client_process_incoming_messages(client)))
			{
				network_event("network_game_client_process_incoming_messages() failed in network_game_client_idle_pregame()");
			}
			/* port: a start put off while the map was downloaded (map
			sharing), once the download is over: the game loaded if the map
			is there (a failed download leaves the game: map_share.c) */
			else if (network_game_client_start_pending && client->state == _network_game_client_state_pregame &&
				!map_share_downloading(client->game.map.name))
			{
				network_game_client_start_pending = FALSE;
				if (csstrcmp(client->game.map.name, network_game_client_start_pending_map) ||
					client->game.number_of_games_played != network_game_client_start_pending_games)
				{
					network_event("the game put off is over: waiting for the host's next start");
				}
				else if (!custom_edition_level_name(client->game.map.name) ||
					custom_edition_cache_playable(client->game.map.name))
				{
					network_event("the map is downloaded: joining the game the host started");
					if (!network_game_client_game_has_started(client))
						network_event("network_game_client_game_has_started() failed");
				}
			}
		}
		else
		{
			success = FALSE;
			if (network_game_client_process_last_messages(client))
			{
				return TRUE;
			}
		}
	}

	if (!success)
	{
		if (!network_connection_active(client->connection))
		{
			display_error_when_main_menu_loaded(4);

			return FALSE;
		}
	}

	return success;
}

static boolean network_game_client_idle_ingame(
	struct network_game_client *client)
{
	boolean success = TRUE;
	/* port: until the host's first update the host may still be loading the
	map (its main thread, which sends nothing meanwhile), as long as it waits
	for the other machines to load (network_server_manager.c's
	NETWORK_GAME_SERVER_MAXIMUM_WAIT_TIME_FOR_LEVEL_LOADING) */
	boolean started = network_game_client_server_has_started_game(client);

	if (!network_connection_active(client->connection) ||
		!network_connection_connected(client->connection))
	{
		error(_error_silent, "new idle in game abort hit");
		display_error_when_main_menu_loaded(_error_network_server_shut_down);
		success = FALSE;
	}
	else if (!network_game_is_splitscreen_local())
	{
		boolean connection_stale = network_connection_going_stale(client->connection);

		/* (port: not at once, network_game_client_network_lost) */
		if (network_game_client_network_lost())
		{
			display_error_when_main_menu_loaded(_error_network_connection_lost);
			network_event("network connection went down (idle in game)!");
			success = FALSE;
		}
		else if (connection_stale && !client->connection_silent && started)
		{
			short local_player_index;

			for (local_player_index = local_player_get_next(NONE);
				local_player_index != NONE;
				local_player_index = local_player_get_next(local_player_index))
			{
				display_error(
					_error_network_trouble_is_brewing,
					local_player_index,
					FALSE,
					FALSE);
			}

			network_event("network client connection has been silent for a dangerously long amount of time");
		}

		client->connection_silent = connection_stale;
	}

	if (success == TRUE)
	{
		success = network_connection_idle(client->connection, started ? 15000 : 75000, NULL);

		if (success)
		{
			success = network_game_client_process_incoming_messages(client);

			if (!success)
			{
				network_event("network_game_client_process_incoming_messages() failed in network_game_client_idle_ingame()");
			}
		}
		else
		{
			if (!network_connection_active(client->connection) ||
				!network_connection_connected(client->connection))
			{
				error(_error_silent, "new2 idle in game abort hit");
				display_error_when_main_menu_loaded(_error_network_server_shut_down);
				success = FALSE;
			}

			network_event("network_connection_idle() failed in network_game_client_idle_ingame()");
		}
	}

	return success;
}

static boolean network_game_client_idle_postgame(
	struct network_game_client *client)
{
	/* (the scores of the game just over: the network may be down for a
	moment, as in the game, network_game_client_network_lost) */
	boolean success = network_game_is_splitscreen_local() || !network_game_client_network_lost();

	if (!success)
	{
		error(_error_silent, "network connection went down!");
		display_error_when_main_menu_loaded(_error_network_connection_lost);
	}
	else
	{
		if (!(success = network_connection_idle(client->connection, 15000, NULL)))
		{
			network_event("network_connection_idle() failed in network_game_client_idle_postgame()");
			if (network_game_client_process_last_messages(client))
			{
				return TRUE;
			}
		}
		else if (!(success = network_game_client_process_incoming_messages(client)))
		{
			network_event("network_game_client_process_incoming_messages() failed in network_game_client_idle_postgame()");
		}
	}

	if (!success)
	{
		if (!network_connection_active(client->connection))
		{
			display_error_when_main_menu_loaded(4);

			return FALSE;
		}
	}

	return success;
}


static void network_game_client_set_error(
	struct network_game_client *client,
	word error)
{
	match_assert(
		"c:\\halo\\SOURCE\\networking\\network_client_manager.c",
		0x662,
		client);

	if (error >= NUMBER_OF_NETWORK_GAME_CLIENT_ERRORS)
		error = _network_game_client_error_unknown;

	if (client->error == _network_game_client_error_none)
		client->error = error;

	return;
}

/* the native ports' automated network tests (port/linux/game/network_test.c):
joins the first open game the client's search has found, as picking it in
the system link list does (network_game_join_game_from_server_list) */
/* the platform layer's (sdl_platform.c) */
void platform_show_message(char const *title, char const *message);

/* whether this client can join the advertised game: its host's network
version is this machine's (HALO_PORT_NETWORK_VERSION), and it plays the
distributed netcode (a host of this version built before the lockstep
netcode was removed may play that). If not the player is told why (when
tell), and nothing is joined. */
boolean network_game_client_advertised_game_compatible(
	struct network_game_client *client,
	struct network_advertised_game const *game,
	boolean tell)
{
	long game_index = client ? game - client->available_games : NONE;
	unsigned int ours = HALO_PORT_NETWORK_VERSION;
	unsigned int theirs;
	boolean distributed;

	if (game_index < 0 || game_index >= MAXIMUM_NETWORK_ADVERTISED_GAMES)
		return FALSE;
	theirs = network_game_client_advertised_versions[game_index].version;
	distributed = (network_game_client_advertised_versions[game_index].flags &
		HALO_PORT_ADVERTISED_DISTRIBUTED_FLAG) != 0;
	if (theirs == ours && distributed)
	{
		network_event("joining a host of network version %u", theirs);
		return TRUE;
	}
	if (tell)
		network_game_client_tell_version(theirs, distributed);
	return FALSE;
}

/* port: the player told why a host of this network version (and netcode)
cannot be joined: an advertised game's, or a server browser's row's */
void network_game_client_tell_version(
	unsigned int theirs,
	boolean distributed)
{
	unsigned int ours = HALO_PORT_NETWORK_VERSION;
	char message[400];

	if (theirs == ours)
	{
		csprintf(message,
			"The host is using the lockstep network code, which this version no longer has.\n\n"
			"Ask the host to update the game.");
	}
	else if (theirs > ours)
	{
		csprintf(message,
			"The host is using a newer version of the network code than you.\n\n"
			"You are on version %u. The host is on version %u.\n\n"
			"Update the game to join this host.",
			ours, theirs);
	}
	else
	{
		csprintf(message,
			"The host is using an older version of the network code than you.\n\n"
			"You are on version %u. The host is on version %u.\n\n"
			"Ask the host to update the game.",
			ours, theirs);
	}
	network_event("not joining a host of network version %u%s (this machine's is %u)", theirs,
		distributed ? "" : " with the lockstep netcode", ours);
	platform_show_message("Halo: cannot join this game", message);
}

/* port: whether the advertised game is under way (HALO_PORT_ADVERTISED_IN_PROGRESS_FLAG),
not in its lobby */
boolean network_game_client_advertised_game_in_progress(
	struct network_game_client *client,
	struct network_advertised_game const *game)
{
	long game_index = client ? game - client->available_games : NONE;

	return game_index >= 0 && game_index < MAXIMUM_NETWORK_ADVERTISED_GAMES &&
		(network_game_client_advertised_versions[game_index].flags & HALO_PORT_ADVERTISED_IN_PROGRESS_FLAG) != 0;
}

/* port: whether the advertised game's host is a dedicated server
(HALO_PORT_ADVERTISED_DEDICATED_FLAG), for the lobby's line */
boolean network_game_client_advertised_game_dedicated(
	struct network_game_client *client,
	struct network_advertised_game const *game)
{
	long game_index = client ? game - client->available_games : NONE;

	return game_index >= 0 && game_index < MAXIMUM_NETWORK_ADVERTISED_GAMES &&
		(network_game_client_advertised_versions[game_index].flags & HALO_PORT_ADVERTISED_DEDICATED_FLAG) != 0;
}

/* port: the client's index'th advertised game, as a probe tells it (halo
--probe: port/linux/game/network_test.c) */
boolean network_game_client_port_advertised_game_summary(
	long index,
	struct network_advertised_game_summary *summary)
{
	struct network_game_client *client = global_network_game_client_get();
	struct network_advertised_game *game;

	if (!client || index < 0 || index >= MAXIMUM_NETWORK_ADVERTISED_GAMES)
		return FALSE;
	game = &client->available_games[index];
	if (!network_game_client_advertised_game_is_valid(game))
		return FALSE;
	csmemset(summary, 0, sizeof(*summary));
	csmemcpy(summary->name, game->game_name, sizeof(summary->name));
	summary->name[NUMBEROF(summary->name) - 1] = 0;
	csmemcpy(summary->map_name, game->map.name, sizeof(summary->map_name));
	summary->map_name[sizeof(summary->map_name) - 1] = 0;
	summary->engine_type = game->engine_type;
	summary->player_count = (short)game->player_count;
	summary->maximum_player_count = game->maximum_player_count;
	summary->open = game->open;
	summary->has_teams = game->has_teams;
	summary->in_progress = network_game_client_advertised_game_in_progress(client, game);
	summary->dedicated = network_game_client_advertised_game_dedicated(client, game);
	summary->version = network_game_client_advertised_versions[index].version;
	summary->kind = network_game_client_advertised_versions[index].kind;
	return TRUE;
}

/* port: the advertised game's kind (an OpenCE gametype's mode, and Hardcore:
port/linux/game/game_modes.h), for the server browser's name of it */
byte network_game_client_advertised_game_kind(
	struct network_game_client *client,
	struct network_advertised_game const *game)
{
	long game_index = client ? game - client->available_games : NONE;

	return game_index >= 0 && game_index < MAXIMUM_NETWORK_ADVERTISED_GAMES ?
		network_game_client_advertised_versions[game_index].kind : 0;
}

boolean network_game_client_join_first_available_game(
	void)
{
	struct network_game_client *client = global_network_game_client_get();
	long game_index;

	if (!client || client->state != _network_game_client_state_searching || client->join_in_progress ||
		!client->connection || network_connection_connected(client->connection))
	{
		return FALSE;
	}
	for (game_index = 0; game_index < MAXIMUM_NETWORK_ADVERTISED_GAMES; game_index++)
	{
		struct network_advertised_game *game = &client->available_games[game_index];

		if (network_game_client_advertised_game_is_valid(game) &&
			game->platform == network_game_get_local_platform() && game->open)
		{
			struct transport_address address = { { { 0 } } };
			struct network_join_parameters join_parameters;

			/* (the automated tests try every frame: told once until the client resets) */
			if (!network_game_client_advertised_game_compatible(client, game,
				!network_game_client_incompatibility_told))
			{
				network_game_client_incompatibility_told = TRUE;
				return FALSE;
			}

			csmemset(&join_parameters, 0, sizeof(join_parameters));
			transport_client_start((XNADDR const *)&game->xnaddr, (XNKEY const *)&game->key,
				(XNKID const *)&game->key_id, NETWORK_GAME_SERVER_PORT, &address);
			if (!address.address.long_words[0] || !address.port)
				return FALSE;
			network_game_generate_join_game_token(join_parameters.join_token);
			return network_game_client_initiate_join_game(client, game, &join_parameters, &address);
		}
	}
	return FALSE;
}

/* ... and puts this machine's players on a team (a team game needs both
teams), as the pregame screen's team choice does; NONE: the other team from
another machine's player */
boolean network_game_client_set_team(
	char team_index)
{
	struct network_game_client *client = global_network_game_client_get();
	boolean success = FALSE;
	short player_index;

	if (!client || client->state != _network_game_client_state_pregame)
		return FALSE;
	for (player_index = 0; player_index < MAXIMUM_NUMBER_OF_PLAYERS && team_index == NONE; player_index++)
	{
		struct network_player const *player = &client->game.players[player_index];

		if (network_player_is_valid(player) && player->machine_index != (char)client->machine_index)
			team_index = player->team_index == 1 ? 0 : 1;
	}
	if (team_index == NONE)
		team_index = 1;
	for (player_index = 0; player_index < MAXIMUM_NUMBER_OF_PLAYERS; player_index++)
	{
		struct network_player player = client->game.players[player_index];

		if (network_player_is_valid(&player) && player.machine_index == (char)client->machine_index)
		{
			player.team_index = team_index;
			success |= network_game_client_update_local_player_data(client, &player);
		}
	}
	return success;
}

