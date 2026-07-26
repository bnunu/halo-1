/*
NETWORK_CONNECTION.C

symbols in this file:
00117B30 0010:
	_network_connection_initialize (0000)
00117B40 0070:
	_network_server_allow_client_connections (0000)
00117BB0 0060:
	_network_connection_connected (0000)
00117C10 00a0:
	_network_connection_get_address (0000)
00117CB0 0120:
	_network_connection_connect (0000)
00117DD0 0040:
	_network_connection_set_connection_rejection_procedure (0000)
00117E10 00a0:
	_network_connection_server_accept_client_connection (0000)
00117EB0 0040:
	_network_connection_active (0000)
00117EF0 0040:
	_network_connection_going_stale (0000)
00117F30 0200:
	_code_00117f30 (0000)
00118130 0440:
	_code_00118130 (0000)
00118570 0010:
	_network_connection_keep_alive (0000)
00118580 00d0:
	_network_connection_delete (0000)
00118650 0330:
	_network_connection_write (0000)
00118980 0140:
	_network_server_close_client_connection (0000)
00118AC0 0080:
	_code_00118ac0 (0000)
00118B40 01e0:
	_code_00118b40 (0000)
00118D20 01e0:
	_code_00118d20 (0000)
00118F00 0240:
	_network_connection_new (0000)
00119140 0090:
	_network_connection_read (0000)
001191D0 00b0:
	_network_connection_disconnect (0000)
00119280 02c0:
	_code_00119280 (0000)
00119540 0310:
	_network_connection_idle (0000)
00282D74 003d:
	??_C@_0DN@LALHHLDL@server_connection?9?$DOflags?$CGFLAG?$CI_c@ (0000)
00282DB4 0012:
	??_C@_0BC@MBGFNKLH@server_connection?$AA@ (0000)
00282DC8 002f:
	??_C@_0CP@PHHPIMDF@c?3?2halo?2SOURCE?2networking?2networ@ (0000)
00282DF8 000b:
	??_C@_0L@IJAFCDGM@connection?$AA@ (0000)
00282E04 003c:
	??_C@_0DM@JFKHFKGL@connect_endpoint?$CI?$CJ?5on?5reliable?5e@ (0000)
00282E40 003e:
	??_C@_0DO@PHFNEMHH@connect_endpoint?$CI?$CJ?5on?5unreliable@ (0000)
00282E80 002d:
	??_C@_0CN@MMKPPGGM@connect_endpoint_async?$CI?$CJ?5returne@ (0000)
00282EB0 000f:
	??_C@_0P@GENNJCOH@remote_address?$AA@ (0000)
00282EC0 0012:
	??_C@_0BC@IIEOAGFH@client_connection?$AA@ (0000)
00282ED4 003e:
	??_C@_0DO@KJEOOHFJ@partial?5datagram?5in?5queue?5?$CI?$CD?$CFd?5o@ (0000)
00282F14 0020:
	??_C@_0CA@COBEMAD@encryption?5should?5not?5be?5active?$AA@ (0000)
00282F38 0065:
	??_C@_0GF@CMFKGJHL@packet?5in?5queue?5is?5?$CD?$CFd?5bytes?0?5bu@ (0000)
00282FA0 0050:
	??_C@_0FA@BBOJNMPA@got?5an?5unusually?5large?5datagram?5@ (0000)
00282FF0 0024:
	??_C@_0CE@GAKKKNNF@?$CKbuffer_size?$DOsizeof?$CImessage_head@ (0000)
00283014 000c:
	??_C@_0M@JLCCDMKC@buffer_size?$AA@ (0000)
00283020 0008:
	??_C@_07ONPBMBOP@message?$AA@ (0000)
00283028 007b:
	??_C@_0HL@CDIBBPO@connection?5?$CG?$CG?5connection?9?$DOunreli@ (0000)
002830A4 0019:
	??_C@_0BJ@POEPMLNA@?$CB?$CCunknown?5traffic?5event?$CC?$AA@ (0000)
002830C0 0014:
	??_C@_0BE@HPLCINIF@?$CFg?7?$CFld?7?$CFld?7?$CFld?7?$CFld?6?$AA@ (0000)
002830D4 0025:
	??_C@_0CF@EIHEBDPA@connection?8s?5remote?5address?5was?3@ (0000)
002830FC 0020:
	??_C@_0CA@JMADEGFJ@connection?5lifetime?7?$CFg?7seconds?6?$AA@ (0000)
00283120 0008:
	__real@3f50624dd2f1a9fc (0000)
00283128 0042:
	??_C@_0EC@HNHLBFPB@NOTE?3?5header?5overhead?5is?5not?5inc@ (0000)
0028316C 002f:
	??_C@_0CP@CCJIMFN@stream?5overhead?5?$CIheaders?$CJ?7?$CFld?7by@ (0000)
0028319C 0032:
	??_C@_0DC@JMBOAMCF@datagram?5overhead?5?$CIheaders?$CJ?7?$CFld?7@ (0000)
002831D0 001e:
	??_C@_0BO@BKOHNOBI@stream?5messages?5received?7?$CFld?6?$AA@ (0000)
002831F0 001a:
	??_C@_0BK@PLGDKDKM@stream?5messages?5sent?7?$CFld?6?$AA@ (0000)
0028320C 0018:
	??_C@_0BI@CKENGKMK@datagrams?5received?7?$CFld?6?$AA@ (0000)
00283224 0014:
	??_C@_0BE@MACKCCIC@datagrams?5sent?7?$CFld?6?$AA@ (0000)
00283238 0003:
	??_C@_02PHMGELLB@?6?6?$AA@ (0000)
00283240 0045:
	??_C@_0EF@PFDCDHNI@time?0?5seconds?7udp?5bytes?5out?7udp?5@ (0000)
00283288 0011:
	??_C@_0BB@KBOPNJJN@_traffic_log?4xls?$AA@ (0000)
0028329C 0034:
	??_C@_0DE@OCDLPLKI@client?5call?5to?5write_endpoint?$CI?$CJ?5@ (0000)
002832D0 0089:
	??_C@_0IJ@ILIPJAEL@?$CIconnection?9?$DOflags?$CGFLAG?$CI_connect@ (0000)
0028335C 002a:
	??_C@_0CK@OKGABJCB@message?5size?5exceeds?5maximum?5all@ (0000)
00283388 0025:
	??_C@_0CF@KHJDIFFG@buffer_size?5?$DM?$DN?5DATAGRAM_MAXIMUM_@ (0000)
002833B0 001d:
	??_C@_0BN@GKPOMGCO@buffer?5size?5was?5?$CFd?5max?5is?5?$CFd?$AA@ (0000)
002833D0 000d:
	??_C@_0N@JGCLEACH@dest_address?$AA@ (0000)
002833E0 000a:
	??_C@_09PFGDAODL@?$CBreliable?$AA@ (0000)
002833EC 0025:
	??_C@_0CF@HBNJJBN@bad?5message?5or?5buffer_size?5param@ (0000)
00283418 0061:
	??_C@_0GB@CPNIJODL@failed?5to?5remove?5a?5client?5endpoi@ (0000)
0028347C 0014:
	??_C@_0BE@LJCKNEPN@server?9?$DOclient_list?$AA@ (0000)
00283490 0015:
	??_C@_0BF@OMPKMEKG@server?9?$DOendpoint_set?$AA@ (0000)
002834A8 003f:
	??_C@_0DP@KMHECGJA@server_connection?9?$DOflags?5?$CG?5FLAG?$CI@ (0000)
002834E8 0012:
	??_C@_0BC@EKDFJKMJ@incoming?9reliable?$AA@ (0000)
002834FC 0012:
	??_C@_0BC@LBJNKONM@reliable_endpoint?$AA@ (0000)
00283510 0063:
	??_C@_0GD@KDKCLKAJ@packet?5in?5queue?5is?5?$CD?$CFd?5bytes?0?5bu@ (0000)
00283578 004d:
	??_C@_0EN@FFFBCEEK@got?5an?5unusually?5large?5message?5?$CI@ (0000)
002835C8 0032:
	??_C@_0DC@NIPJCDFJ@connection?5?$CG?$CG?5connection?9?$DOreliab@ (0000)
002835FC 003c:
	??_C@_0DM@BHKHEGGG@blocked?5in?5network_connection_id@ (0000)
00283638 0020:
	??_C@_0CA@CDMGOOMC@client?5reliable?5connection?5lost?$AA@ (0000)
00283658 0031:
	??_C@_0DB@HLCKNJA@error?5?8?$CFs?8?5reading?5from?5client?5r@ (0000)
0028368C 0023:
	??_C@_0CD@EEPBCOBK@circular_queue_queue_data?$CI?$CJ?5fail@ (0000)
002836B0 0024:
	??_C@_0CE@ELOAAFGD@connection?9?$DOreliable_incoming_qu@ (0000)
002836D4 001e:
	??_C@_0BO@GNMCHDOP@connection?9?$DOreliable_endpoint?$AA@ (0000)
002836F4 0014:
	??_C@_0BE@NPIEPOBN@incoming?9unreliable?$AA@ (0000)
00283708 0030:
	??_C@_0DA@ONKNJAHC@well_known_port?5?$DO?5MAXIMUM_RESERV@ (0000)
00283738 0065:
	??_C@_0GF@LBGPIAAM@?$CIflags?$CGFLAG?$CI_connection_create_s@ (0000)
002837A0 0085:
	??_C@_0IF@LOPDJAMN@connection?9?$DOflags?$CGFLAG?$CI_connecti@ (0000)
00283828 0028:
	??_C@_0CI@GBLKHCFP@poll_endpoint_set?$CI?$CJ?5returned?5err@ (0000)
00283850 0027:
	??_C@_0CH@EFPCNJNH@rogue?5endpoint?5connected?5to?5the?5@ (0000)
00283878 0042:
	??_C@_0EC@FCONNFAI@failed?5to?5remove?5a?5client?5endpoi@ (0000)
002838BC 0020:
	??_C@_0CA@KMGAFGOG@accept_endpoint?$CI?$CJ?5returned?5NULL?$AA@ (0000)
002838DC 0018:
	??_C@_0BI@LPNBCLGA@error?5adding?5new?5client?$AA@ (0000)
002838F4 0016:
	??_C@_0BG@BDEGOGDF@new_client_connection?$AA@ (0000)
0028390C 0019:
	??_C@_0BJ@CNCMMBDB@connection?9?$DOendpoint_set?$AA@ (0000)
00283928 0029:
	??_C@_0CJ@LIBJNIBK@connection?9?$DOconnection?4reliable_@ (0000)
00283954 0013:
	??_C@_0BD@NCEDJJHP@connection?5?$CB?$DN?5NULL?$AA@ (0000)
00283968 0027:
	??_C@_0CH@PJEDOOFH@datagram?5received?5from?5unknown?5a@ (0000)
00283990 0049:
	??_C@_0EJ@EBOPBIBA@circular_queue_queue_data?$CI?$CJ?5fail@ (0000)
002839DC 0020:
	??_C@_0CA@NHHJOLOM@endpoint?5read?5buffer?5overflowed?$AA@ (0000)
002839FC 0038:
	??_C@_0DI@GAIEALCC@network_connection_idle_client_r@ (0000)
00283A34 0023:
	??_C@_0CD@EAHAMPLA@timeout?5in?5network_connection_id@ (0000)
00283A58 0038:
	??_C@_0DI@JINGJIOI@network_connection_idle_server_r@ (0000)
00283A90 0039:
	??_C@_0DJ@ODICILDO@dont?5timeout?5is?5active?5so?5not?5ti@ (0000)
004566DA 0001:
	_global_connection_dont_timeout (0000)
*/

/* ---------- headers */

#include "cseries/cseries.h"
#include "cseries/cseries_windows.h"
#include "network_connection.h"

/* ---------- constants */

enum
{
	_connection_create_server_bit = 0,
};

/* ---------- macros */

/* ---------- structures */

struct network_connection
{
	byte __unknown0[8];
	unsigned long keep_alive_time;
	void *connection_rejection_procedure;
	byte __unknown10[0x20];
	unsigned long flags;
	byte __unknown34[0x18];
	boolean allow_client_connections;
};

/* ---------- prototypes */

/* ---------- globals */

/* ---------- public code */

void network_connection_initialize(
	void)
{
	return;
}

void network_connection_set_connection_rejection_procedure(
	struct network_connection *connection,
	void *connection_rejection_procedure)
{
	match_assert(
		"c:\\halo\\SOURCE\\networking\\network_connection.c",
		0x318,
		connection);

	connection->connection_rejection_procedure = connection_rejection_procedure;

	return;
}

boolean network_connection_active(
	struct network_connection *connection)
{
	match_assert(
		"c:\\halo\\SOURCE\\networking\\network_connection.c",
		0x330,
		connection);

	return !TEST_FLAG(connection->flags, 4);
}

boolean network_connection_going_stale(
	struct network_connection *connection)
{
	match_assert(
		"c:\\halo\\SOURCE\\networking\\network_connection.c",
		0x338,
		connection);

	return TEST_FLAG(connection->flags, 5);
}

void network_connection_keep_alive(
	struct network_connection *connection)
{
	connection->keep_alive_time = system_milliseconds();

	return;
}

void network_server_allow_client_connections(
	struct network_connection *server_connection,
	boolean allow_client_connections)
{
	match_assert(
		"c:\\halo\\SOURCE\\networking\\network_connection.c",
		0x151,
		server_connection);
	match_assert(
		"c:\\halo\\SOURCE\\networking\\network_connection.c",
		0x152,
		server_connection->flags&FLAG(_connection_create_server_bit));

	server_connection->allow_client_connections = allow_client_connections;

	return;
}

/* ---------- private code */
