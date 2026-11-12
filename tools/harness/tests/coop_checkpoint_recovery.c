/* Real replica/create/delete and player-state code against a small fake world.
   Rendering, tag lookup and object construction are fixture boundaries. Datum
   allocation, lookup and deletion use memory/data.c, including full identifiers. */
#define _CRT_SECURE_NO_WARNINGS
#define data_array harness_data_array
#define datum_header harness_datum_header
#define game_state_data_new harness_game_state_data_new
#define datum_new harness_datum_new
#define datum_get harness_datum_get
#define datum_delete harness_datum_delete
#include "harness.h"
#undef data_array
#undef datum_header
#undef game_state_data_new
#undef datum_new
#undef datum_get
#undef datum_delete
#include <math.h>

typedef unsigned short word;
typedef word message_header;
typedef struct { real red, green, blue; } real_rgb_color;
#define MIN(a,b) ((a)<(b)?(a):(b))
#define MAX(a,b) ((a)>(b)?(a):(b))
#define PIN(v,a,b) MIN(MAX(v,a),b)
#define NUMBEROF(a) (sizeof(a)/sizeof((a)[0]))
#define csmemset memset
#define csmemcpy memcpy
#define csmemcmp memcmp
#define csstrncpy strncpy
#define SET_FLAG(v,b,on) ((v) = (on) ? (v)|FLAG(b) : (v)&~FLAG(b))
#define DATUM_INDEX_TO_IDENTIFIER(i) ((short)((i)>>16))
#define DATUM_INDEX_NEW(i,id) (((long)(unsigned short)(id)<<16)|(i))
#define match_vassert(f,l,c,m) CHECK(c,"data invariant")
#define platform_log(...) ((void)0)
/* Only world capacities are reduced. Behavior limits and flags come from the
   owning production definitions, including the inventory-to-player capacity. */
#define MAXIMUM_TRACKED_OBJECTS 128
#define MAXIMUM_TRACKED_PLAYERS 2
#define HALO_PORT_MAXIMUM_NETWORK_MACHINES 2
enum {_game_connection_network_client=1,_game_connection_network_server=2};
enum {_distributed_to_host_reliably};
#include "config.inc"
enum { _player_action_result_reload, _netgame_flag_teleporter_source };
enum { OBJECT_DEFINITION_TAG=1, BIPED_DEFINITION_TAG, VEHICLE_DEFINITION_TAG,
       WEAPON_DEFINITION_TAG, EQUIPMENT_DEFINITION_TAG, game_engine_ctf, DAMAGE_EFFECT_DEFINITION_TAG };
#define unit_datum object_datum
#define weapon_datum object_datum
#include "types.inc"

struct weapon_magazine { short rounds_total,rounds_loaded,state; };
struct object_datum {
    long definition_index;
    struct {
        short type, owner_team_index, forced_shader_permutation_index;
        unsigned flags, damage_flags;
        long owner_player_index, parent_object_index, first_child_object_index, next_object_index;
        real_point3d position;
        real_vector3d forward, up, translational_velocity, angular_velocity;
        real_rgb_color base_change_colors[NUMBER_OF_OBJECT_CHANGE_COLORS], outgoing_change_colors[NUMBER_OF_OBJECT_CHANGE_COLORS];
        byte region_permutations[MAXIMUM_REGIONS_PER_OBJECT];
    } object;
    struct {
        long player_index, driver_object_index, weapon_object_indices[MAXIMUM_WEAPONS_PER_UNIT];
        long weapon_last_used_at_game_time[MAXIMUM_WEAPONS_PER_UNIT];
        short parent_seat_index,current_weapon_index,desired_weapon_index;
        short grenade_throw_state,current_grenade_index,desired_grenade_index;
        char grenade_counts[NUMBER_OF_UNIT_GRENADE_TYPES];
        unsigned flags;
        real active_camouflage;
        real_vector3d desired_facing_vector,desired_aiming_vector,desired_looking_vector;
        real_vector3d aiming_vector,looking_vector;
    } unit;
    struct {struct weapon_magazine magazines[2];real age;long game_time_last_fired;} weapon;
};
struct object_header { short identifier; struct object_datum *datum; };
struct player_datum {
    short identifier, local_player_index;
    long unit_index, team_index, action_object_index, teleporter_index;
    int action_result;
    short powerup_durations[NUMBER_OF_PLAYER_POWERUPS];
};
struct object_placement_data {
    long definition_index, owner_player_index;
    short owner_team_index, variant_number;
    real_point3d position;
    real_vector3d forward,up,translational_velocity,angular_velocity;
    real_rgb_color change_colors[NUMBER_OF_OBJECT_CHANGE_COLORS];
};
struct object_definition { struct { short type; struct {long index;} model,collision_model; } object; };
struct weapon_magazine_definition {short rounds_total_initial,rounds_total_maximum,rounds_loaded_maximum;};
struct weapon_definition { struct { unsigned flags;
    struct {short count;struct weapon_magazine_definition *elements;} magazines;} weapon; };
struct model_region { struct { short count; } permutations; };
struct model { struct { short count; struct model_region *elements; } regions; };
#define TAG_BLOCK_GET_ELEMENT(block,index,type) (&(block)->elements[index])
struct damage_network_state { boolean shield_depleted,shield_charging,shield_over_charging;
    real body_vitality,shield_vitality,current_body_damage,recent_body_damage,current_shield_damage,recent_shield_damage; };
struct distributed_death { boolean valid,friendly_fire,killed_by_vehicle; long killing_player_index,dead_since; };
struct object_iterator { long index; int next,mask; };
struct damage_data {
    long definition_index,owner_player_index,owner_object_index;
    unsigned long flags;short owner_team_index,material_type;
    real_point3d origin,epicenter;real_vector3d direction;
    real scale,multiplier,material_effect_scale;int location;
};
struct collision_model {struct {struct {short count;} regions,materials;} resistance;};
struct damage_effect_definition {real cutoff_radius;};

static struct data_array *object_header_data,*player_data;
static struct object_datum world[MAXIMUM_TRACKED_OBJECTS];
static int world_capacity=16;
static struct object_definition definitions[4];
static struct weapon_definition weapon_definition;
static struct weapon_magazine_definition magazine_definition={60,120,12};
static struct model empty_model;
static struct { int game_engine_index; } variant;
static struct distributed_death distributed_deaths[2];
static short distributed_seat_disagreements[2];
static real distributed_own_round_trip;
static long objects_client_has[MAXIMUM_TRACKED_OBJECTS],objects_client_creating_index=NONE;
static boolean objects_client_checkpoint_pending[MAXIMUM_TRACKED_OBJECTS];
static int objects_client_deleting,objects_client_creating;
static int objects_client_failed_time=NONE,objects_client_ready_time=NONE;
static int objects_client_ready_interval,objects_client_retry_ticks=CLIENT_RETRY_TICKS;
static int objects_client_ask_again,objects_client_synchronized=1;
static int objects_client_new_object_count,objects_client_check_all;
static struct { int creates,deletes,create_failures; } objects_statistics;
static int client=1,coop=1,now=1000,constructed,deleted,controls_changed;
static unsigned long distributed_checkpoint_epoch=1;
static boolean distributed_checkpoint_synchronized;
static boolean distributed_handling_batch,distributed_handling_stream_message;
static boolean distributed_identity_sent,distributed_build_sent;
static int recovery_barriers;
static long control_unit=NONE;
static boolean fail_allocation;
static boolean require_detached,detached_before_construction;
static int damage_replayed_events,kill_calls,damage_replaying_kill,damage_replaying_killer_score;
static long damage_replaying_killer=NONE;
/* Narrow storage views for the real damage reset and report sender. The
   source controls queue consumption; serialization/delivery ends at capture. */
static short damage_event_count,damage_report_count;
static int damage_dealing_report,damage_sent_reports,hit_reports_sent;
static long last_hit_target=NONE;
static struct distributed_hit_report damage_reports[MAXIMUM_HIT_REPORTS_PER_TICK];
static struct {
    long definition_indices[MAXIMUM_RECENT_WEAPONS],unarmed_melee_damage_index;
    long throw_times[MAXIMUM_GRENADE_THROWS],driven_time;
    struct {long time;} stuck[MAXIMUM_STUCK_PROJECTILES];
    real hit_seconds;
} damage_players[MAXIMUM_TRACKED_PLAYERS];
static struct {long time;} damage_history[TARGET_HISTORY_TICKS];
static struct {long time;short count;} damage_explosions[MAXIMUM_TRACKED_PLAYERS];
static struct {long time;} damage_trails[MAXIMUM_TRACKED_PLAYERS][PLAYER_TRAIL_POINTS];
static struct {long time;} damage_fire_origins[MAXIMUM_TRACKED_PLAYERS],damage_fire_origins_sent[MAXIMUM_LOCAL_PLAYERS];
static struct {long source_index;} damage_rate_cache[DAMAGE_RATE_CACHE_SIZE];
static int checkpoint_prediction_resets;
static void distributed_reset_checkpoint_state(boolean reset_received_times) {
    CHECK(!reset_received_times,"completion preserves ordered snapshot receive history");checkpoint_prediction_resets++;
}
static void distributed_send_fire_origins(void) { }
static struct object_datum *object_get(long index);
static struct object_datum *object_try_and_get(long index);
static void object_delete_recursive(long index,boolean siblings);
static void network_player_detach_unit(long index);
static boolean distributed_player_is_local(long index);
static long distributed_player_from_byte(byte index);
static void distributed_client_create_failed(void);
static void unit_kill(long index);
static void unit_kill_silent(long index);
static void unit_kill_no_statistics(long index);
static short objects_client_retried_inventory_count;
static long objects_host_told[MAXIMUM_TRACKED_OBJECTS],objects_host_told_count;
static struct {long time,player_indices[MAXIMUM_LOCAL_PLAYERS];} objects_host_machines[2];
static struct {unsigned long checksum,weapons_checksum;} objects_host_inventories[MAXIMUM_TRACKED_OBJECTS];
static union {long alignment;byte bytes[MAXIMUM_MESSAGE_SIZE];} snapshot_packets[32];
static word snapshot_sizes[32];
static int snapshot_count;
static struct distributed_object_change description(long index);

#include "data.inc"

static struct object_datum *object_try_and_get(long index) {
    struct object_header *h=datum_try_and_get(object_header_data,index);
    return h?h->datum:NULL;
}
static struct object_datum *object_get(long index) {
    struct object_datum *o=object_try_and_get(index);CHECK(o,"invalid object %lx",index);return o;
}
static struct object_datum *object_try_and_get_and_verify_type(long index,int mask) {
    struct object_datum *o=object_try_and_get(index);return o&&TEST_FLAG(mask,o->object.type)?o:NULL;
}
#define unit_get object_get
#define unit_try_and_get(index) object_try_and_get_and_verify_type(index,_object_mask_unit)
#define weapon_get object_get
static struct object_datum *weapon_try_and_get(long index) {return object_try_and_get_and_verify_type(index,_object_mask_weapon);}
static struct player_datum *player_try_and_get(long index) {return datum_try_and_get(player_data,index);}
static struct player_datum *player_get(long index) {return datum_get(player_data,index);}
static struct object_definition *object_definition_get(long index) {CHECK(index>=0&&index<4,"tag");return &definitions[index];}
static struct weapon_definition *weapon_definition_get(long index) {(void)index;return &weapon_definition;}
static struct model *model_definition_get(long index) {(void)index;return &empty_model;}
static boolean tag_index_is_group(long index,long group) {
    if(group==DAMAGE_EFFECT_DEFINITION_TAG)return index==10;
    return index>=0&&index<4&&(group==OBJECT_DEFINITION_TAG||group==BIPED_DEFINITION_TAG+definitions[index].object.type);
}
static int game_connection(void) {return client?_game_connection_network_client:_game_connection_network_server;}
static int network_connection_last_read_was_unreliable(void) {return 0;}
/* Explicit fake reset boundary; the real network_objects_reverted reset is
   exercised separately with its ownership and transient histories. */
static void distributed_restore_checkpoint(void) {
    int i;
    recovery_barriers++;memset(objects_client_checkpoint_pending,TRUE,sizeof(objects_client_checkpoint_pending));
    for(i=0;i<MAXIMUM_LOCAL_PLAYERS;i++)objects_client_own_inventories[i].unit_index=NONE;
    objects_client_synchronized=FALSE;objects_client_ready_time=NONE;
}
static int effect_cleanups;
static void effects_discard_gameplay(void) {effect_cleanups++;}
static int network_game_distributed_client(void) {return client;}
static int network_coop_active(void) {return coop;}
static long game_time_get(void) {return now;}
static int game_engine_running(void) {return 0;}
static int game_engine_has_teams(void) {return 0;}
static void *game_engine_get_variant(void) {return &variant;}
#define game_engine_get_variant() (&variant)
static int weapon_is_flag(long index) {(void)index;return 0;}
static void game_engine_objective_weapon_deleted(long index) {(void)index;CHECK(0,"unexpected objective");}
static void object_iterator_new(struct object_iterator *it,int mask,int flags) {it->next=0;it->mask=mask;(void)flags;}
static void *object_iterator_next(struct object_iterator *it) {
    while(it->next<world_capacity){int slot=it->next++;struct object_header *h=(struct object_header *)object_header_data->data+slot;
        if(h->identifier&&TEST_FLAG(it->mask,h->datum->object.type)){it->index=DATUM_INDEX_NEW(slot,h->identifier);return h->datum;}}
    return NULL;
}
/* Bounded fake-world graph operations; the network's decisions about which
   children survive are the real distributed_client_delete implementation. */
static void object_detach(long index) {
    struct object_datum *o=object_get(index);long parent=o->object.parent_object_index;
    if(parent!=NONE){long *link=&object_get(parent)->object.first_child_object_index;int steps=0;
        while(*link!=NONE&&*link!=index){CHECK(++steps<16,"fixture attachment cycle");link=&object_get(*link)->object.next_object_index;}
        CHECK(*link==index,"fixture child is attached");*link=o->object.next_object_index;}
    o->object.parent_object_index=o->object.next_object_index=NONE;
}
static void attach_child(long parent,long child) {
    struct object_datum *p=object_get(parent),*c=object_get(child);
    CHECK(c->object.parent_object_index==NONE,"fixture child already attached");
    c->object.next_object_index=p->object.first_child_object_index;p->object.first_child_object_index=child;
    c->object.parent_object_index=parent;
}
static void item_in_unit_inventory(long item,long unit) {attach_child(unit,item);}
static void unit_drop_item(long unit,long item) {(void)unit;object_detach(item);SET_FLAG(object_get(item)->object.flags,_object_connected_to_map_bit,TRUE);}
static short unit_weapon_next_index(long index,long previous,int direction) {
    int i;(void)previous;(void)direction;for(i=0;i<MAXIMUM_WEAPONS_PER_UNIT;i++)if(unit_get(index)->unit.weapon_object_indices[i]!=NONE)return (short)i;return NONE;
}
static void first_person_weapon_message_from_unit(long index,int message) {(void)index;CHECK(message==13,"weapon removal");}
static void unit_exit_seat_end(long index) {object_detach(index);unit_get(index)->unit.parent_seat_index=NONE;}
static void unit_set_actively_controlled(long index,int active) {(void)index;(void)active;}
static void player_control_new_unit(short local,long index) {CHECK(local==0,"local controller");control_unit=index;controls_changed++;}
static void observer_obsolete_position(short local) {CHECK(local==0,"local observer");}
static void infection_reconcile_player(long index) {(void)index;}
static void object_set_garbage(long index,int value) {SET_FLAG(object_get(index)->object.flags,_object_garbage_bit,value);}
static void object_disconnect_from_map(long index) {SET_FLAG(object_get(index)->object.flags,_object_connected_to_map_bit,FALSE);}
static void object_set_visibility(long index,int visible) {(void)index;(void)visible;}
static void object_damage_update(long index) {
    struct object_datum *o=object_get(index);
    CHECK(o->object.damage_flags&(FLAG(_object_die_act_of_god_bit)|FLAG(_object_die_act_of_god_silent_bit)|
        FLAG(_object_die_act_of_god_no_statistics_bit)),"damage update requires a real kill request");
    SET_FLAG(o->object.damage_flags,_object_dead_bit,TRUE);
}
static void object_delete_immediately(long index) {
    CHECK(objects_client_deleting || (client&&object_get(index)->object.type==_object_type_projectile),
        "delete must be authorized host replica work or client projectile cleanup");
    object_delete_recursive(index,FALSE);
}
static void object_deleted_procs_call(long index) {(void)index;}
static void object_deactivate(long index) {(void)index;}
static void object_delete_attachments(long index) {(void)index;}
static void object_type_delete(long index) {object_detach(index);}
static void object_header_delete(struct data_array *data,long index) {datum_delete(data,index);deleted++;}
static void object_placement_data_new(struct object_placement_data *p,long definition,long owner) {
    memset(p,0,sizeof(*p));p->definition_index=definition;p->owner_player_index=owner;
}
static long object_new(struct object_placement_data *p) {
    long index=objects_client_creating_index;
    struct object_header *h;
    struct object_datum *o;
    int i;
    CHECK(objects_client_creating,"host construction");
    if(require_detached){CHECK(control_unit==NONE&&player_get(DATUM_INDEX_NEW(0,7))->unit_index==NONE,
        "old player binding and control cleared before construction");detached_before_construction=TRUE;}
    if(fail_allocation)return NONE;
    CHECK(datum_new_at_index(object_header_data,index)==index,"exact handle allocation");
    h=datum_get(object_header_data,index);o=&world[DATUM_INDEX_TO_ABSOLUTE_INDEX(index)];
    memset(o,0,sizeof(*o));h->datum=o;
    o->definition_index=p->definition_index;o->object.type=definitions[p->definition_index].object.type;
    o->object.flags=FLAG(_object_connected_to_map_bit);o->object.position=p->position;
    o->object.owner_player_index=p->owner_player_index;o->object.owner_team_index=p->owner_team_index;
    o->object.parent_object_index=o->object.first_child_object_index=o->object.next_object_index=NONE;
    o->unit.player_index=o->unit.driver_object_index=NONE;o->unit.parent_seat_index=NONE;
    o->unit.current_weapon_index=o->unit.desired_weapon_index=NONE;
    o->unit.current_grenade_index=o->unit.desired_grenade_index=NONE;
    for(i=0;i<MAXIMUM_WEAPONS_PER_UNIT;i++)o->unit.weapon_object_indices[i]=NONE;
    constructed++;return index;
}
static void distributed_object_move(long index,real_point3d const *position,real_vector3d const *forward,
    real_vector3d const *up,real_vector3d const *velocity,real_vector3d const *angular,short kind) {
    struct object_datum *o=object_get(index);
    CHECK(kind==_network_vehicle_write_change,"object-change write");o->object.position=*position;o->object.forward=*forward;
    o->object.up=*up;o->object.translational_velocity=*velocity;o->object.angular_velocity=*angular;
}
static boolean mcc_script_effects_preserve_body(void) {return FALSE;}
static void network_objects_set_seat(long index,long vehicle,short seat) {(void)index;CHECK(vehicle==NONE&&seat==NONE,"no seat fixture");}
static real distributed_vitality_unpack(word v) {return v/16384.0f;}
static real distributed_own_round_trip_ticks(void) {return 0;}
static void damage_set_network_state(long index,struct damage_network_state const *state) {(void)index;CHECK(state->body_vitality==1.0f,"host vitality");}
static void distributed_correct_own_unit(struct player_datum *p,long index,struct distributed_unit_state const *s) {(void)p;(void)index;(void)s;CHECK(0,"movement outside fixture");}
static void distributed_apply_remote_unit(long index,struct distributed_unit_state const *s)
    {(void)index;(void)s;CHECK(0,"movement outside fixture");}
static real distance_squared3d(real_point3d const *a,real_point3d const *b) {return (a->x-b->x)*(a->x-b->x)+(a->y-b->y)*(a->y-b->y)+(a->z-b->z)*(a->z-b->z);}
static long find_netgame_flag(real_point3d const *p,real a,real b,int kind,long previous) {(void)p;(void)a;(void)b;(void)kind;(void)previous;return NONE;}

/* Damage effects are the fake-world boundary. The real handler and validators
   decide whether to reach this callback. No packet-age decision is mocked here. */
static void damage_data_new(struct damage_data *data,long definition) {memset(data,0,sizeof(*data));data->definition_index=definition;}
static void scenario_location_from_point(int *location,real_point3d const *point) {(void)point;*location=0;}
static struct collision_model *collision_model_definition_get(long index) {(void)index;CHECK(0,"fixture has no collision model");return NULL;}
static boolean object_has_node(long index,short node) {(void)index;return node==0;}
static void damage_replay_player_effect(long player,struct damage_data *data,real total) {(void)player;(void)data;(void)total;CHECK(0,"not a player-effect fixture");}
static void damage_replay_aftermath(long object,struct damage_data *data,unsigned flags,real shield,real body,real multiplier,short part) {
    (void)object;(void)data;(void)flags;(void)shield;(void)body;(void)multiplier;(void)part;CHECK(0,"not an aftermath fixture");
}
static void distributed_set_death(short player,byte killer,boolean friendly,boolean vehicle) {(void)killer;(void)friendly;(void)vehicle;CHECK(player==0,"victim slot");}
static void damage_replay_kill(long index,struct damage_data *data,short node,short region,short material) {
    long player=unit_get(index)->unit.player_index;
    CHECK(damage_replaying_kill&&damage_replaying_killer==NONE&&damage_replaying_killer_score==0,"real replay context");
    CHECK(data->definition_index==10&&node==NONE&&region==NONE&&material==NONE,"validated fixture kill");
    kill_calls++;unit_kill_no_statistics(index);object_damage_update(index);
    if(player!=NONE)network_player_detach_unit(player);
}
static void players_show_telefragged(long player) {(void)player;CHECK(0,"not a telefrag fixture");}
static boolean static_target(long index) {(void)index;return FALSE;}
static long static_target_near(real_point3d const *point,real reach) {(void)point;(void)reach;CHECK(0,"not a static fixture");return NONE;}
static struct damage_effect_definition *damage_effect_definition_get(long index) {(void)index;CHECK(0,"not a static fixture");return NULL;}
static void damage_replay_static(long index,struct damage_data *data,short node,short region,short material) {
    (void)index;(void)data;(void)node;(void)region;(void)material;CHECK(0,"not a static fixture");
}

/* Transport capture, host membership and descriptor serialization are fixture
   boundaries. The ordered snapshot sender and inventory serializer/replay are
   production functions; central transport stamping has separate tests. */
static long *machine_get_player_list(long machine) {
    static long list[MAXIMUM_LOCAL_PLAYERS];CHECK(machine==1,"snapshot recipient");return list;
}
static void distributed_host_update_objects(void) { }
static void distributed_change_from_object(long index,struct distributed_object_change *change) {
    struct object_datum *object=object_get(index);
    *change=description(index);change->definition_index=object->definition_index;
    SET_FLAG(change->flags,_distributed_object_dead_bit,TEST_FLAG(object->object.damage_flags,_object_dead_bit));
}
static void distributed_send_to_machine_reliably(long machine,void *message,short type,short count,word size) {
    struct distributed_message_header *header=message;
    CHECK(machine==1&&snapshot_count<32&&size<=MAXIMUM_MESSAGE_SIZE,"bounded reliable capture");
    header->type=(byte)type;header->count=(byte)count;header->checkpoint_epoch=distributed_checkpoint_epoch;
    memcpy(snapshot_packets[snapshot_count].bytes,message,size);snapshot_sizes[snapshot_count++]=size;
}
static void distributed_checkpoint_to_machine(long machine) {
    struct distributed_message_header header={0};
    distributed_send_to_machine_reliably(machine,&header,_distributed_message_checkpoint,0,sizeof(header));
}
static long local_player_get_player_index(short local) {return local==0?DATUM_INDEX_NEW(0,7):NONE;}
static int client_ready_sends,prediction_ticks;
static void distributed_send(void *message,short type,short count,word size,short destination) {
    if(type==_distributed_message_hit_reports){
        struct distributed_hit_report_message *reports=message;
        CHECK(distributed_checkpoint_synchronized&&destination==_distributed_to_host_reliably&&count>0&&
            size==sizeof(reports->header)+count*sizeof(reports->reports[0]),"current-world bounded hit report capture");
        hit_reports_sent+=count;last_hit_target=reports->reports[count-1].object_index;return;
    }
    (void)message;CHECK(type==_distributed_message_client_ready&&count<=1&&size==sizeof(struct distributed_message_header)&&
        destination==_distributed_to_host_reliably,"client tick keeps bounded readiness alive");client_ready_sends++;
}
static void distributed_client_remove_own_objects(void) { }
static void distributed_client_ready_picked_up_weapons(void) {prediction_ticks++;}
static void mcc_network_client_tick(void) {prediction_ticks++;}
static void distributed_client_send_vehicles(void) {prediction_ticks++;}
static void distributed_client_carry_unsteered_vehicles(void) {prediction_ticks++;}
static int identity_sends,build_sends;
static void distributed_client_send_identity(void) {prediction_ticks++;identity_sends++;}
static void distributed_client_send_build(void) {prediction_ticks++;build_sends++;}

#include "engine.inc"
#include "under_test.inc"

static void begin_checkpoint(void) {
    struct distributed_message_header barrier={0};
    int before=recovery_barriers;
    barrier.type=_distributed_message_checkpoint;barrier.checkpoint_epoch=distributed_checkpoint_epoch+1;
    distributed_identity_sent=distributed_build_sent=TRUE;
    CHECK(!distributed_message_epoch_valid(NONE,&barrier,sizeof(barrier)),"barrier consumed before gameplay dispatch");
    CHECK(distributed_checkpoint_epoch==barrier.checkpoint_epoch&&recovery_barriers==before+1,"host barrier begins new lifetime");
    CHECK(!distributed_identity_sent&&!distributed_build_sent,"initial identity may be sent again");
}
static void finish_snapshot(void) {
    struct distributed_message_header marker={0};
    marker.type=_distributed_message_objects_synchronized;marker.checkpoint_epoch=distributed_checkpoint_epoch;
    CHECK(distributed_message_epoch_valid(NONE,&marker,sizeof(marker))&&distributed_checkpoint_synchronized,
        "ordered snapshot marker admits current world gameplay");
}

static struct data_array *new_array(char const *name,int capacity,int size) {
    struct data_array *array=calloc(1,sizeof(*array)+(size_t)capacity*size);
    CHECK(array,"array allocation");data_initialize(array,name,(short)capacity,(short)size);
    array->valid=TRUE;array->next_identifier=1;return array;
}
static struct distributed_object_change description(long index) {
    struct distributed_object_change c;memset(&c,0,sizeof(c));
    c.change=_object_change_create;c.object_index=index;c.definition_index=0;
    c.owner_player_index=0;c.owner_team_index=0;c.forward.i=1;c.up.k=1;return c;
}
static struct distributed_unit_state live_state(long index) {
    struct distributed_unit_state state;memset(&state,0,sizeof(state));
    state.flags=FLAG(_distributed_unit_alive_bit);state.unit_index=index;
    state.vehicle_index=NONE;state.seat_index=NONE;state.body_vitality=16384;return state;
}
static void setup(void) {
    int i;long player_index=DATUM_INDEX_NEW(0,7);struct player_datum *player;
    object_header_data=new_array("objects",world_capacity,sizeof(struct object_header));
    player_data=new_array("players",2,sizeof(struct player_datum));
    CHECK(datum_new_at_index(player_data,player_index)==player_index,"player allocation");
    player=player_get(player_index);player->unit_index=NONE;player->local_player_index=0;
    player->action_object_index=NONE;
    for(i=0;i<4;i++){definitions[i].object.type=(short)i;
        definitions[i].object.model.index=definitions[i].object.collision_model.index=NONE;}
    weapon_definition.weapon.magazines.count=1;weapon_definition.weapon.magazines.elements=&magazine_definition;
    for(i=0;i<MAXIMUM_TRACKED_OBJECTS;i++){
        objects_client_has[i]=objects_host_told[i]=NONE;objects_client_checkpoint_pending[i]=TRUE;
    }
    for(i=0;i<MAXIMUM_TRACKED_PLAYERS;i++)distributed_deaths[i].dead_since=NONE;
    for(i=0;i<MAXIMUM_LOCAL_PLAYERS;i++)objects_client_own_inventories[i].unit_index=NONE;
}
static long add_replica(short slot,short type) {
    struct distributed_object_change c=description(DATUM_INDEX_NEW(slot,0x1234));
    c.definition_index=type;network_objects_handle_changes(&c,1);return c.object_index;
}
static struct distributed_inventory inventory_for(long unit,long weapon) {
    struct distributed_inventory inv;int i;memset(&inv,0,sizeof(inv));inv.unit_index=unit;
    for(i=0;i<MAXIMUM_WEAPONS_PER_UNIT;i++)inv.weapon_indices[i]=NONE;
    inv.weapon_indices[0]=weapon;inv.current_weapon_index=0;inv.grenade_counts[0]=2;
    inv.rounds_total[0][0]=77;inv.rounds_loaded[0][0]=11;return inv;
}
static struct distributed_damage_event kill_for(long index) {
    struct distributed_damage_event event;
    memset(&event,0,sizeof(event));event.kind=_damage_event_kill;
    event.player_index=NO_PLAYER;event.object_index=index;event.damage.definition_index=10;
    event.damage.owner_player_index=NO_PLAYER;event.damage.owner_object_index=NONE;
    event.damage.scale=event.damage.multiplier=event.damage.material_effect_scale=1.0f;
    event.node_index=event.region_index=event.material_index=NONE;return event;
}
static int snapshot_inventory_case(long unit,boolean many) {
    long weapon=add_replica(4,_object_type_weapon);
    struct distributed_inventory future=inventory_for(unit,weapon);
    unsigned long old_epoch=distributed_checkpoint_epoch;
    int i,received=0,batches=0,described=0,phase=0;
    int expected=many?world_capacity-5:1;
    int limit=MIN(MAXIMUM_ENTRIES_PER_MESSAGE,RELIABLE_ENTRIES(struct distributed_inventory));
    objects_host_told[3]=unit;objects_host_told[4]=weapon;objects_host_told_count=5;
    if(many){
        for(i=5;i<world_capacity;i++)objects_host_told[i]=add_replica((short)i,_object_type_biped);
        /* One host corpse must not acquire an inventory or change rank counts. */
        SET_FLAG(object_get(objects_host_told[5])->object.damage_flags,_object_dead_bit,TRUE);
        objects_host_told_count=world_capacity;
    }
    client=0;distributed_checkpoint_epoch++;objects_host_machines[1].time=NONE;
    network_objects_client_asked(1,TRUE);
    CHECK(snapshot_count>2,"full snapshot captured");
    /* The retained client has future-world equipment; no sender timing or
       checksum state may prevent the checkpoint's explicit empty inventory. */
    client=1;distributed_checkpoint_epoch=old_epoch;
    distributed_client_apply_inventory(&future);
    {
        struct distributed_unit_state state=live_state(unit);
        distributed_handle_unit_state(now,&state);
        CHECK(player_get(DATUM_INDEX_NEW(0,7))->unit_index==unit,"retained healthy unit remains locally bound");
    }
    CHECK(object_get(unit)->unit.grenade_counts[0]==2&&object_get(unit)->unit.weapon_object_indices[0]==weapon,"future equipment established");
    now+=4*TICKS_PER_SECOND;
    for(i=0;i<snapshot_count;i++){
        struct distributed_message_header *header=(struct distributed_message_header *)snapshot_packets[i].bytes;
        void *entries=snapshot_packets[i].bytes+sizeof(*header);
        if(header->type==_distributed_message_checkpoint){
            int ticks;
            CHECK(i==0&&!distributed_message_epoch_valid(NONE,header,snapshot_sizes[i]),"ordered host barrier first");
            objects_client_new_object_count=MAXIMUM_CLIENT_NEW_OBJECTS;
            for(ticks=0;ticks<3;ticks++){
                now++;network_objects_client_tick();
                CHECK(objects_client_own_inventories[0].unit_index==NONE&&!prediction_ticks&&objects_client_check_all,
                    "old bound replica does not seed predictions; deferred own-object cleanup remains requested");
            }
            CHECK(client_ready_sends>0,"partial snapshot does not stall ready handshake");continue;
        }
        CHECK(distributed_message_epoch_valid(NONE,header,snapshot_sizes[i]),"ordered snapshot admitted while gameplay is frozen");
        if(header->type==_distributed_message_object_changes){
            CHECK(!phase&&!distributed_checkpoint_synchronized,"descriptions precede inventory and marker");
            described+=header->count;network_objects_handle_changes(entries,header->count);
        }else if(header->type==_distributed_message_inventories){
            struct distributed_inventory *inventories=entries;int j;
            CHECK(phase<=1&&!distributed_checkpoint_synchronized&&header->count>0&&header->count<=limit,"inventory within reliable batch bounds before marker");
            phase=1;batches++;received+=header->count;
            CHECK(snapshot_sizes[i]==sizeof(*header)+header->count*sizeof(*inventories),"exact inventory bytes");
            for(j=0;j<header->count;j++)distributed_client_apply_inventory(&inventories[j]);
        }else{
            CHECK(header->type==_distributed_message_objects_synchronized&&i==snapshot_count-1&&phase==1&&distributed_checkpoint_synchronized,"marker follows complete inventories");
            network_objects_handle_synchronized();
            phase=2;
        }
    }
    CHECK(phase==2&&described==(many?world_capacity-3:2),"all known descriptions precede completion");
    CHECK(received==expected&&batches==(expected+limit-1)/limit,"every live inventory exactly once, including partial final batch");
    CHECK(object_get(unit)->unit.grenade_counts[0]==0&&object_get(unit)->unit.weapon_object_indices[0]==NONE&&
        object_get(weapon)->object.parent_object_index==NONE,"empty snapshot clears future grenades and detaches future weapon");
    network_objects_client_tick();
    CHECK(prediction_ticks==6&&objects_client_own_inventories[0].unit_index==unit,"ordinary prediction history resumes after ordered marker");
    snapshot_count=0;client=0;network_objects_client_asked(1,FALSE);
    CHECK(snapshot_count==2&&((struct distributed_message_header *)snapshot_packets[1].bytes)->type==_distributed_message_objects_synchronized,
        "already-requested snapshot retains bounded synchronized-only path");
    return 0;
}
static int exercise(char const *name) {
    long index=DATUM_INDEX_NEW(3,0x1234),player_index=DATUM_INDEX_NEW(0,7);
    struct distributed_object_change change;
    struct distributed_unit_state state;
    struct object_datum *corpse;
    long weapon=NONE,rider=NONE,cosmetic=NONE;
    struct distributed_inventory inventory;
    struct distributed_damage_event saved_kill;
    struct distributed_message_header kill_header={0};
    int constructed_before,deleted_before,controls_before;
    boolean bound_before;
    if(!strcmp(name,"snapshot-batches"))world_capacity=MAXIMUM_TRACKED_OBJECTS;
    setup();change=description(index);
    network_objects_handle_changes(&change,1);finish_snapshot();CHECK(constructed==1,"initial replica");
    corpse=object_get(index);state=live_state(index);
    if(!strncmp(name,"snapshot-",9))return snapshot_inventory_case(index,!strcmp(name,"snapshot-batches"));
    if(!strcmp(name,"identity-after-boundary")){
        int identity_before,build_before;
        objects_client_synchronized=FALSE;network_objects_client_tick();
        CHECK(client_ready_sends==1&&objects_client_ready_time==now,"a ready went");
        begin_checkpoint();CHECK(objects_client_ready_time==NONE,"the boundary cleared the ready's time");
        /* the snapshot's end came in the same read: no tick between to ask again */
        finish_snapshot();network_objects_handle_synchronized();
        identity_before=identity_sends;build_before=build_sends;
        now++;network_objects_client_tick();
        CHECK(objects_client_synchronized&&identity_sends==identity_before+1&&build_sends==build_before+1,
            "identity and build go once the snapshot is whole, though the boundary cleared the ready's time");
        return 0;
    }
    if(!strcmp(name,"partial-producers")){
        long projectile,child;
        int prediction_resets;
        weapon=add_replica(4,_object_type_weapon);inventory=inventory_for(index,weapon);
        distributed_handle_unit_state(now,&state);
        begin_checkpoint();
        distributed_client_apply_inventory(&inventory);
        CHECK(!distributed_checkpoint_synchronized&&corpse->unit.weapon_object_indices[0]==weapon,
            "healthy restored inventory arrives before the ordered marker");
        /* game_tick still updates objects while the reliable snapshot arrives.
           Object creation is a fixture boundary; gate and deletion are real. */
        projectile=add_replica(5,_object_type_equipment);child=add_replica(6,_object_type_equipment);
        object_get(projectile)->object.type=_object_type_projectile;attach_child(projectile,child);
        damage_report_count=damage_event_count=1;damage_reports[0].object_index=child;
        damage_history[0].time=now;damage_players[0].definition_indices[0]=weapon;
        prediction_resets=checkpoint_prediction_resets;
        deleted_before=deleted;finish_snapshot();
        CHECK(!object_try_and_get(projectile)&&!object_try_and_get(child)&&deleted==deleted_before+2,
            "first snapshot marker retires partial-world projectile through normal deletion");
        CHECK(object_get(index)==corpse&&player_get(player_index)->unit_index==index&&control_unit==index&&
            corpse->unit.player_index==player_index&&network_objects_client_has(index),
            "healthy authoritative unit and reciprocal ownership survive final producer cleanup");
        CHECK(corpse->unit.weapon_object_indices[0]==weapon&&object_get(weapon)->object.parent_object_index==index&&
            corpse->unit.grenade_counts[0]==2&&object_get(weapon)->weapon.magazines[0].rounds_loaded==11,
            "restored inventory is not reset by final producer cleanup");
        CHECK(!damage_report_count&&!damage_event_count&&!damage_dealing_report&&damage_history[0].time==NONE&&
            damage_players[0].definition_indices[0]==NONE&&checkpoint_prediction_resets==prediction_resets+1,
            "first marker clears partial-world damage and prediction histories");
        network_damage_client_tick();CHECK(!hit_reports_sent,"partial-world queued hit is never sent after marker");
        projectile=add_replica(7,_object_type_equipment);object_get(projectile)->object.type=_object_type_projectile;
        damage_report_count=1;damage_reports[0].object_index=index;damage_history[0].time=now;
        prediction_resets=checkpoint_prediction_resets;
        deleted_before=deleted;finish_snapshot();
        CHECK(object_try_and_get(projectile)&&deleted==deleted_before&&damage_report_count==1&&
            damage_history[0].time==now&&checkpoint_prediction_resets==prediction_resets,
            "duplicate marker preserves current-world projectile, queued hit and history");
        network_damage_client_tick();CHECK(hit_reports_sent==1&&last_hit_target==index&&!damage_report_count&&damage_sent_reports==1,
            "fresh current-world hit still reaches the ordinary sender");
        return 0;
    }
    if(!strcmp(name,"projectiles")){
        long projectile=add_replica(4,_object_type_equipment);
        long child=add_replica(5,_object_type_equipment);
        int initial_effect_cleanups=effect_cleanups;
        object_get(projectile)->object.type=_object_type_projectile;
        attach_child(projectile,child);
        client=0;network_objects_discard_client_effects();
        CHECK(object_try_and_get(projectile)&&object_try_and_get(child)&&!deleted&&effect_cleanups==initial_effect_cleanups,"host restored projectiles and effects are retained");
        client=1;network_objects_discard_client_effects();
        CHECK(!object_try_and_get(projectile)&&!object_try_and_get(child)&&deleted==2,"client old shot follows real recursive object deletion");
        CHECK(object_try_and_get(index)&&constructed==3,"unrelated live unit survives");
        network_objects_discard_client_effects();CHECK(deleted==2&&effect_cleanups==initial_effect_cleanups+2,"repeated cleanup is harmless; effect lifetime also retires");return 0;
    }
    if(!strcmp(name,"live")){
        distributed_handle_unit_state(now,&state);network_objects_handle_changes(&change,1);
        CHECK(constructed==1&&!deleted,"unchanged live replica is reused");
        CHECK(player_get(player_index)->unit_index==index&&control_unit==index,"live binding retained");return 0;
    }
    if(!strcmp(name,"host-kills-live")){
        SET_FLAG(change.flags,_distributed_object_dead_bit,TRUE);network_objects_handle_changes(&change,1);
        CHECK(constructed==1&&!deleted,"host death reuses the living object");
        CHECK(TEST_FLAG(corpse->object.damage_flags,_object_dead_bit)&&
            TEST_FLAG(corpse->object.damage_flags,_object_die_act_of_god_silent_bit)&&
            TEST_FLAG(corpse->object.damage_flags,_object_die_act_of_god_no_statistics_bit),"real silent/no-statistics kill requests applied");
        return 0;
    }
    if(!strcmp(name,"bound-corpse")){
        distributed_handle_unit_state(now,&state);
        CHECK(player_get(player_index)->unit_index==index&&corpse->unit.player_index==player_index&&control_unit==index,
            "old life begins with a reciprocal player binding");
        require_detached=TRUE;
    }
    if(!strcmp(name,"unknown")||!strcmp(name,"unknown-player")){
        if(!strcmp(name,"unknown"))state.unit_index=DATUM_INDEX_NEW(4,0x1234);else state.player_index=255;
        distributed_handle_unit_state(now,&state);
        CHECK(!objects_client_ask_again&&!deleted&&constructed==1,"unknown references do not reconstruct");
        CHECK(player_get(player_index)->unit_index==NONE&&control_unit==NONE,"unknown references do not bind");return 0;
    }
    if(!strcmp(name,"stale-delete")){
        change.change=_object_change_delete;change.object_index=DATUM_INDEX_NEW(3,0x1233);
        network_objects_handle_changes(&change,1);
        CHECK(!deleted&&network_objects_client_has(index),"old generation delete must not remove current object");return 0;
    }
    if(!strcmp(name,"inventory")||!strcmp(name,"hierarchy")){
        weapon=add_replica(4,_object_type_weapon);inventory=inventory_for(index,weapon);
        distributed_client_apply_inventory(&inventory);
        CHECK(corpse->unit.weapon_object_indices[0]==weapon&&object_get(weapon)->object.parent_object_index==index,"initial inventory attached");
        if(!strcmp(name,"hierarchy")){
            rider=add_replica(5,_object_type_biped);unit_get(rider)->unit.parent_seat_index=0;attach_child(index,rider);
            cosmetic=add_replica(6,_object_type_equipment);attach_child(index,cosmetic);
        }
    }
    if(!strcmp(name,"late-kill")){
        distributed_handle_unit_state(now,&state);saved_kill=kill_for(index);
        kill_header.type=_distributed_message_damage_events;kill_header.count=1;
        kill_header.checkpoint_epoch=distributed_checkpoint_epoch;
        CHECK(distributed_message_epoch_valid(NONE,&kill_header,sizeof(kill_header)+sizeof(saved_kill)),"old life was current at first delivery");
        network_damage_handle_events(&saved_kill,1);
        CHECK(kill_calls==1&&damage_replayed_events==1,"first delivery legitimately kills old bound life");
        CHECK(TEST_FLAG(corpse->object.damage_flags,_object_dead_bit)&&player_get(player_index)->unit_index==NONE&&
            control_unit==NONE,"first delivery detaches the old life");
    }else SET_FLAG(corpse->object.damage_flags,_object_dead_bit,TRUE);
    begin_checkpoint();
    if(!strncmp(name,"invalid-",8)||!strcmp(name,"outside-pool")){
        struct object_datum before=*corpse;
        if(!strcmp(name,"invalid-tag"))change.definition_index=99;
        else if(!strcmp(name,"invalid-team"))change.owner_team_index=MAXIMUM_TRACKED_PLAYERS;
        else if(!strcmp(name,"invalid-color"))change.change_colors[2].red=NAN;
        else if(!strcmp(name,"invalid-position"))change.position.x=NAN;
        else if(!strcmp(name,"invalid-axes"))change.up=change.forward;
        else if(!strcmp(name,"invalid-identifier"))change.object_index=3;
        else change.object_index=DATUM_INDEX_NEW(16,0x1234);
        network_objects_handle_changes(&change,1);
        CHECK(constructed==1&&!deleted&&network_objects_client_has(index),"invalid description must not destroy replica");
        CHECK(!memcmp(&before,corpse,sizeof(before))&&objects_client_checkpoint_pending[3],"invalid description preserves object and recovery eligibility");
        change=description(index);network_objects_handle_changes(&change,1);distributed_handle_unit_state(now,&state);
        CHECK(constructed==2&&deleted==1&&control_unit==index&&!objects_client_checkpoint_pending[3],"next valid descriptor still reconstructs");return 0;
    }
    if(!strcmp(name,"host-dead")){
        SET_FLAG(change.flags,_distributed_object_dead_bit,TRUE);network_objects_handle_changes(&change,1);
        CHECK(constructed==1&&!deleted&&TEST_FLAG(corpse->object.damage_flags,_object_dead_bit),"host corpse stays dead");return 0;
    }
    if(!strcmp(name,"different-handle")){
        change.object_index=DATUM_INDEX_NEW(3,0x1235);state.unit_index=change.object_index;
        network_objects_handle_changes(&change,1);distributed_handle_unit_state(now,&state);
        CHECK(!object_try_and_get(index)&&network_objects_client_has(change.object_index),"identifier replacement");
        CHECK(constructed==2&&deleted==1&&player_get(player_index)->unit_index==change.object_index,"new generation binds");return 0;
    }
    distributed_handle_unit_state(now,&state);
    CHECK(player_get(player_index)->unit_index==(require_detached?index:NONE),"alive state alone cannot replace a corpse");
    CHECK(objects_client_ask_again&&!objects_client_synchronized,"authoritative recovery request");
    CHECK(objects_client_ready_interval==CLIENT_RETRY_TICKS,"ordinary retry delay");
    if(!strcmp(name,"retry")){
        int i;long interval=objects_client_ready_interval;
        for(i=0;i<20;i++){now++;distributed_handle_unit_state(now,&state);}
        CHECK(objects_client_ready_interval==interval&&objects_client_retry_ticks==2*CLIENT_RETRY_TICKS,"one pending request coalesces");
        network_objects_handle_synchronized();CHECK(!objects_client_synchronized,"premature completion cannot clear pending failure");
        for(i=0;i<20;i++){objects_client_ready_time=++now;now++;distributed_handle_unit_state(now,&state);}
        CHECK(objects_client_retry_ticks==CLIENT_RETRY_MAXIMUM_TICKS,"bounded retry growth");
        objects_client_ready_time=++now;network_objects_handle_synchronized();
        CHECK(objects_client_synchronized&&objects_client_failed_time==NONE&&objects_client_retry_ticks==CLIENT_RETRY_TICKS,"completed request resets retry");return 0;
    }
    if(!strcmp(name,"allocation-failure"))fail_allocation=TRUE;
    if(!strcmp(name,"late-inventory")){
        weapon=DATUM_INDEX_NEW(4,0x1234);inventory=inventory_for(index,weapon);
        distributed_client_apply_inventory(&inventory);
        CHECK(objects_client_retried_inventory_count==1,"inventory waits for missing weapon");
    }
    constructed_before=constructed;deleted_before=deleted;controls_before=controls_changed;
    bound_before=player_get(player_index)->unit_index==index;
    network_objects_handle_changes(&change,1);distributed_handle_unit_state(now,&state);
    if(fail_allocation){
        CHECK(deleted==1&&constructed==1&&!object_try_and_get(index),"failed replacement leaves no partial object");
        CHECK(!network_objects_client_has(index)&&objects_client_has[3]==NONE,"failed replacement is not known");
        CHECK(player_get(player_index)->unit_index==NONE&&control_unit==NONE,"failed replacement never binds");
        CHECK(objects_statistics.create_failures==1&&objects_client_ask_again,"failed allocation retains retry");return 0;
    }
    CHECK(constructed==constructed_before+1&&deleted==deleted_before+(cosmetic!=NONE?2:1),"replacement constructor and destructor");
    CHECK(!TEST_FLAG(object_get(index)->object.damage_flags,_object_dead_bit),"new live replica");
    CHECK(player_get(player_index)->unit_index==index&&object_get(index)->unit.player_index==player_index,"reciprocal binding");
    CHECK(control_unit==index&&controls_changed==controls_before+1+(bound_before?1:0),"local control restored");
    CHECK(!require_detached||detached_before_construction,"bound corpse cleanup precedes reconstruction");
    CHECK(objects_client_has[3]==index&&network_objects_client_has(index),"exact authoritative handle");
    CHECK(!objects_client_checkpoint_pending[3],"validated snapshot consumes recovery eligibility");
    finish_snapshot();
    if(weapon!=NONE){
        CHECK(object_get(index)->unit.grenade_counts[0]==0,"replacement waits for host grenades");
        if(!strcmp(name,"late-inventory")){
            CHECK(add_replica(4,_object_type_weapon)==weapon,"delayed weapon identity");
            distributed_client_apply_retried_inventories();
            CHECK(!objects_client_retried_inventory_count,"completed inventory consumed");
        }else{
            CHECK(network_objects_client_has(weapon)&&object_get(weapon)->object.parent_object_index==NONE,"host weapon survives unit replacement");
            distributed_client_apply_inventory(&inventory);
        }
        CHECK(object_get(index)->unit.weapon_object_indices[0]==weapon&&object_get(weapon)->object.parent_object_index==index,"inventory returns to replacement");
        CHECK(object_get(index)->unit.grenade_counts[0]==2&&object_get(index)->unit.desired_weapon_index==0,"host equipment restored");
        CHECK(object_get(weapon)->weapon.magazines[0].rounds_total==77&&object_get(weapon)->weapon.magazines[0].rounds_loaded==11,"host ammunition restored");
        if(rider!=NONE){CHECK(network_objects_client_has(rider)&&object_get(rider)->object.parent_object_index==NONE,"seated child survives replacement");
            CHECK(!object_try_and_get(cosmetic),"non-preserved child follows real recursive deletion");}
    }
    if(!strcmp(name,"repeat")){
        SET_FLAG(object_get(index)->object.damage_flags,_object_dead_bit,TRUE);
        player_get(player_index)->action_object_index=index;
        begin_checkpoint();
        network_objects_handle_changes(&change,1);distributed_handle_unit_state(now,&state);
        CHECK(constructed==3&&deleted==2,"second restore reconstructs again");
        CHECK(control_unit==index&&controls_changed==3,"bound corpse detaches then rebinds");
        CHECK(player_get(player_index)->action_object_index==NONE,"discarded action target cleared");
    }
    if(!strncmp(name,"kill-",5)||!strcmp(name,"late-kill")||!strcmp(name,"same-epoch-create")){
        struct distributed_damage_event event=!strcmp(name,"late-kill")?saved_kill:kill_for(index);
        int kills_before=kill_calls,events_before=damage_replayed_events;
        if(!strcmp(name,"kill-unknown"))event.object_index=DATUM_INDEX_NEW(4,0x1234);
        if(!strcmp(name,"kill-invalid-number"))event.damage.scale=NAN;
        if(!strcmp(name,"kill-invalid-tag"))event.damage.definition_index=99;
        if(!strcmp(name,"kill-invalid-node"))event.node_index=1;
        /* Deliver the identical old payload through the production lifetime
           gate. The complete dispatcher/transport has separate coverage. */
        if(strcmp(name,"late-kill")){
            kill_header.type=_distributed_message_damage_events;kill_header.count=1;
            kill_header.checkpoint_epoch=distributed_checkpoint_epoch;
        }
        if(distributed_message_epoch_valid(NONE,&kill_header,sizeof(kill_header)+sizeof(event)))
            network_damage_handle_events(&event,1);
        if(!strcmp(name,"late-kill")){
            CHECK(kill_calls==kills_before&&damage_replayed_events==events_before,"old kill never reaches replay");
            CHECK(!TEST_FLAG(object_get(index)->object.damage_flags,_object_dead_bit)&&
                player_get(player_index)->unit_index==index&&object_get(index)->unit.player_index==player_index&&
                control_unit==index&&network_objects_client_has(index),"rejected old-life kill preserves reconstructed life");
            return 0;
        }
        if(!strcmp(name,"kill-current")||!strcmp(name,"same-epoch-create")){
            CHECK(kill_calls==kills_before+1&&damage_replayed_events==events_before+1,"valid event reached real replay dispatch");
            CHECK(TEST_FLAG(object_get(index)->object.damage_flags,_object_dead_bit)&&
                player_get(player_index)->unit_index==NONE,"accepted kill affects reconstructed life");
            network_damage_handle_events(&event,1);CHECK(kill_calls==kills_before+1,"already-dead duplicate not replayed");
            if(!strcmp(name,"same-epoch-create")){
                int count=constructed;
                network_objects_handle_changes(&change,1);distributed_handle_unit_state(now,&state);
                CHECK(constructed==count&&TEST_FLAG(object_get(index)->object.damage_flags,_object_dead_bit)&&
                    player_get(player_index)->unit_index==NONE,"later live description cannot revive a same-epoch death");
                begin_checkpoint();network_objects_handle_changes(&change,1);distributed_handle_unit_state(now,&state);
                CHECK(constructed==count+1&&!TEST_FLAG(object_get(index)->object.damage_flags,_object_dead_bit)&&
                    control_unit==index,"next checkpoint explicitly permits same-handle recovery");
            }
        }else{
            CHECK(!kill_calls&&!TEST_FLAG(object_get(index)->object.damage_flags,_object_dead_bit),"refused kill leaves replica alive");
            CHECK(player_get(player_index)->unit_index==index&&control_unit==index,"refused kill leaves binding intact");
            CHECK(damage_replayed_events==(!strcmp(name,"kill-unknown")||!strcmp(name,"kill-invalid-node")),"actual validation/lookup dispatch boundary");
        }
    }
    return 0;
}
int main(int argc,char **argv) {
    int result;CHECK(argc==2,"case");result=exercise(argv[1]);
    free(object_header_data);free(player_data);return result;
}
