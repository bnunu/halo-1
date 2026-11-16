#include "harness.h"

#include "config.inc"
enum { OBJECTS = 8, SENT = 32 };
struct object_datum { struct { unsigned short damage_flags; } object; };
struct distributed_coop_event { long type, value; };
static struct { boolean joined, refresh; } host_resend;
#include "state.inc"
static struct object_datum objects[OBJECTS];
static boolean exists[OBJECTS];
static long sent_markers[SENT];
static int sent_count;
static struct object_datum *object_try_and_get(long index)
{
	return index >= 0 && index < OBJECTS && exists[index] ? &objects[index] : NULL;
}
static struct distributed_coop_event *event_new(long type)
{
	static struct distributed_coop_event event;
	return &event;
}
static void send_nav_point(short kind, short nav_index, long target, long marker, real vertical_offset)
{
	CHECK(sent_count < SENT, "too many sent");
	sent_markers[sent_count++] = marker;
}
#include "under_test.inc"

static void add(short kind, long marker)
{
	short index = host_hud_state.nav_point_count++;

	host_hud_state.nav_points[index].kind = kind;
	host_hud_state.nav_points[index].nav_index = 1;
	host_hud_state.nav_points[index].target = 0;
	host_hud_state.nav_points[index].marker = marker;
}

static boolean sent(long marker)
{
	int index;

	for (index = 0; index < sent_count; index++)
		if (sent_markers[index] == marker)
			return TRUE;
	return FALSE;
}

int main(int argc, char **argv)
{
	const char *case_name = argv[1];
	long index;

	host_hud_state.objective = NONE;
	host_resend.refresh = TRUE;
	for (index = 0; index < OBJECTS; index++)
		exists[index] = TRUE;
	objects[2].object.damage_flags = FLAG(_object_dead_bit);
	exists[3] = FALSE;
	CASE("alive")
	{
		add(_coop_nav_point_team_object, 1);
		host_send_hud_state();
		CHECK(sent_count == 1 && sent(1), "a living object's nav point not resent");
		CHECK(host_hud_state.nav_point_count == 1, "a living object's nav point forgotten");
	}
	CASE("dead")
	{
		/* an enemy a script marked, killed since: not sent again, forgotten */
		add(_coop_nav_point_team_object, 2);
		host_send_hud_state();
		CHECK(!sent_count, "a dead object's nav point resent");
		CHECK(!host_hud_state.nav_point_count, "a dead object's nav point kept");
	}
	CASE("gone")
	{
		add(_coop_nav_point_unit_object, 3);
		host_send_hud_state();
		CHECK(!sent_count && !host_hud_state.nav_point_count, "a deleted object's nav point kept");
	}
	CASE("flag")
	{
		/* a flag's marker is a cutscene flag's index, no object */
		add(_coop_nav_point_team_flag, 3);
		add(_coop_nav_point_unit_flag, 2);
		host_send_hud_state();
		CHECK(sent_count == 2 && host_hud_state.nav_point_count == 2, "a flag's nav point dropped");
	}
	CASE("mixed")
	{
		add(_coop_nav_point_team_object, 2);
		add(_coop_nav_point_team_object, 3);
		add(_coop_nav_point_team_object, 1);
		add(_coop_nav_point_team_object, 2);
		add(_coop_nav_point_team_flag, 5);
		host_send_hud_state();
		CHECK(sent_count == 2 && sent(1) && sent(5), "the living ones not all resent (%d sent)", sent_count);
		CHECK(host_hud_state.nav_point_count == 2, "%d nav points kept", host_hud_state.nav_point_count);
	}
	CASE("no-refresh")
	{
		host_resend.refresh = FALSE;
		add(_coop_nav_point_team_object, 2);
		host_send_hud_state();
		CHECK(!sent_count && host_hud_state.nav_point_count == 1, "changed outside a refresh");
	}
	return 0;
}
