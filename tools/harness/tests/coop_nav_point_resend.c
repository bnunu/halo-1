#include "harness.h"

#include "config.inc"
enum { OBJECTS = 40, SENT = 64 };
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
static boolean coop_host(void) { return TRUE; }
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
	network_coop_note_nav_point(kind, 1, 0, marker, 0.f);
}

static boolean sent(long marker)
{
	int index;

	for (index = 0; index < sent_count; index++)
		if (sent_markers[index] == marker)
			return TRUE;
	return FALSE;
}

static boolean kept(long marker)
{
	short index;

	for (index = 0; index < host_hud_state.nav_point_count; index++)
		if (host_hud_state.nav_points[index].marker == marker)
			return TRUE;
	return FALSE;
}

/* a resend: what the host sends again every OBJECT_REFRESH_TICKS */
static void resend(void)
{
	sent_count = 0;
	host_send_hud_state();
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
		resend();
		CHECK(sent_count == 1 && sent(1), "a living object's nav point not resent");
		CHECK(kept(1), "a living object's nav point forgotten");
	}
	CASE("dead")
	{
		/* an enemy a script marked, killed since: not sent again */
		add(_coop_nav_point_team_object, 2);
		resend();
		CHECK(!sent_count, "a dead object's nav point resent");
	}
	CASE("gone")
	{
		add(_coop_nav_point_unit_object, 3);
		resend();
		CHECK(!sent_count, "a deleted object's nav point resent");
	}
	CASE("flag")
	{
		/* a flag's marker is a cutscene flag's index, no object */
		add(_coop_nav_point_team_flag, 3);
		add(_coop_nav_point_unit_flag, 2);
		resend();
		CHECK(sent_count == 2, "a flag's nav point not resent");
	}
	CASE("mixed")
	{
		add(_coop_nav_point_team_object, 2);
		add(_coop_nav_point_team_object, 3);
		add(_coop_nav_point_team_object, 1);
		add(_coop_nav_point_unit_object, 2);
		add(_coop_nav_point_team_flag, 5);
		resend();
		CHECK(sent_count == 2 && sent(1) && sent(5), "the living ones not all resent (%d sent)", sent_count);
	}
	CASE("revert")
	{
		/* marked alive at a checkpoint, killed, then the checkpoint reverted
		(the list is outside it): the enemy is back, and so is its marker,
		for the clients there and one that joins after */
		objects[2].object.damage_flags = 0;
		add(_coop_nav_point_team_object, 2);
		resend();
		CHECK(sent(2), "the living enemy's nav point not resent");
		objects[2].object.damage_flags = FLAG(_object_dead_bit);
		resend();
		CHECK(!sent_count, "the dead enemy's nav point resent");
		objects[2].object.damage_flags = 0;
		resend();
		CHECK(sent(2), "the reverted enemy's nav point not resent");
		host_resend.joined = TRUE;
		resend();
		CHECK(sent(2), "a late joiner not sent the reverted enemy's nav point");
	}
	CASE("full")
	{
		/* sixteen, one of them on a dead enemy: a new one takes that place */
		for (index = 0; index < MAXIMUM_NAV_POINTS; index++)
			add(_coop_nav_point_team_object, 10 + index);
		objects[12].object.damage_flags = FLAG(_object_dead_bit);
		add(_coop_nav_point_team_object, 30);
		CHECK(host_hud_state.nav_point_count == MAXIMUM_NAV_POINTS, "%d nav points", host_hud_state.nav_point_count);
		CHECK(kept(30) && !kept(12), "the new nav point did not take the dead one's place");
		/* all sixteen living: a seventeenth is not kept, as before */
		objects[12].object.damage_flags = 0;
		add(_coop_nav_point_team_object, 12);
		CHECK(!kept(12), "a seventeenth nav point replaced a living one");
	}
	CASE("no-refresh")
	{
		host_resend.refresh = FALSE;
		add(_coop_nav_point_team_object, 1);
		sent_count = 0;
		host_send_hud_state();
		CHECK(!sent_count, "resent outside a refresh");
	}
	return 0;
}
