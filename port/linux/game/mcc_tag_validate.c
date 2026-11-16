/* MCC tags use the engine's published schema descriptions, with their own
 * traversal, ownership bitmap, stream limits and callback context. Neither
 * the Xbox validator state nor any Custom Edition validation mode is used. */
#include "mcc_tag_validate.h"
#include "mcc_runtime.h"
#include "mcc_loading.h"
#include "mcc_syntax.h"
#include "mcc_object_names.h"
#include "tag_schema.h"

#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define MCC_VALIDATION_DEPTH 32
#define MCC_VALIDATION_CAPACITY 0x04000000UL

struct mcc_validation_instance {
    unsigned long group, parents[2];
    long handle;
    char *name;
    void *root;
    unsigned long unused[2];
};
struct mcc_validation_header {
    struct mcc_validation_instance *instances;
    long scenario;
    unsigned long checksum;
    long count;
    long vertex_count;
    byte *vertices;
    long index_count;
    byte *indices;
    unsigned long signature;
};
struct mcc_validation_bsp_header {
    void *root;
    long vertex_count;
    byte *vertices;
    long index_count;
    byte *indices;
    unsigned long signature;
};
struct mcc_validation_frame {
    byte *address;
    struct tag_schema_definition const *schema;
};
struct mcc_validation_context {
    byte *region;
    unsigned long length;
    struct mcc_validation_frame stack[MCC_VALIDATION_DEPTH];
    int depth;
    byte *structure;
    long handle;
    char const *field;
    char path[192];
    boolean rejected;
    boolean geometry_access;
    unsigned long visits;
};
struct mcc_allocation {
    struct mcc_allocation *next;
    void const *address;
    unsigned long size;
    struct tag_schema_definition const *schema;
};
static struct {
    struct mcc_runtime *runtime;
    struct mcc_validation_header *header;
    byte *claims;
    byte *aliases;
    struct mcc_allocation *allocations[1024];
    unsigned long claim_bytes;
    long corrections;
    struct mcc_validation_context *active;
} mcc_validation;
static char mcc_validation_empty_name[] = "";
static struct mcc_validation_dispatch const mcc_callbacks = {
    mcc_validation_owns, mcc_validation_message, mcc_validation_file_contains,
    mcc_validation_root, mcc_validation_contains, mcc_validation_tag_get,
    mcc_validation_buffer_data, mcc_validation_any_claimed
};

/* MCC retains tag-collection loader lists that the Xbox schema inventory
 * does not describe. Wire format: a 12-byte block of 16-byte references.
 * Reference: Invader src/tag/hek/definition/tag_collection.json. */
struct mcc_collection_entry { struct tag_reference reference; };
struct mcc_collection_root { struct tag_block tags; };
static struct tag_schema_field const mcc_collection_entry_fields[] = {
    TAG_SCHEMA_REFERENCE(struct mcc_collection_entry, reference, NULL),
    TAG_SCHEMA_END
};
static struct tag_schema_definition const mcc_collection_entry_schema =
    TAG_SCHEMA_DEFINITION(mcc_collection_entry, struct mcc_collection_entry, mcc_collection_entry_fields);
static struct tag_schema_field const mcc_collection_fields[] = {
    TAG_SCHEMA_TOOL_BLOCK(struct mcc_collection_root, tags, mcc_collection_entry_schema, 200),
    TAG_SCHEMA_END
};
static struct tag_schema_definition const mcc_collection_schema =
    TAG_SCHEMA_DEFINITION(mcc_collection, struct mcc_collection_root, mcc_collection_fields);
static struct tag_schema_group const mcc_collection_group = {'tagc', {NONE, NONE}, &mcc_collection_schema};

/* Input-device defaults are retained PC/MCC menu data: enum16, flags16 and
 * two raw-data descriptors (44 bytes total). Format evidence: Reclaimer
 * reclaimer/hek/defs/devc.py; payloads never control the native input layer. */
struct mcc_device_defaults {
    short device_kind;
    unsigned short flags;
    struct tag_data identifier;
    struct tag_data profile;
};
static struct tag_schema_field const mcc_device_fields[] = {
    TAG_SCHEMA_ENUM(struct mcc_device_defaults, device_kind, 3, 0),
    TAG_SCHEMA_DATA(struct mcc_device_defaults, identifier, 16),
    TAG_SCHEMA_DATA(struct mcc_device_defaults, profile, 41984),
    TAG_SCHEMA_END
};
static struct tag_schema_definition const mcc_device_schema =
    TAG_SCHEMA_DEFINITION(mcc_device_defaults, struct mcc_device_defaults, mcc_device_fields);
static struct tag_schema_group const mcc_device_group = {'devc', {NONE, NONE}, &mcc_device_schema};

static int mcc_range(void const *base, unsigned long length, void const *address, unsigned long bytes)
{
    uintptr_t origin = (uintptr_t)base, point = (uintptr_t)address;
    return point >= origin && point - origin <= length && bytes <= length - (unsigned long)(point - origin);
}

static int mcc_claim(void const *address, unsigned long size)
{
    unsigned long first, bit;
    if (!size) return 1;
    if (!mcc_range(mcc_validation.runtime->tags, mcc_validation.runtime->capacity, address, size)) return 0;
    first = (unsigned long)((uintptr_t)address - (uintptr_t)mcc_validation.runtime->tags);
    for (bit = first; bit < first + size; ++bit) {
        byte mask = (byte)(1U << (bit & 7));
        if (mcc_validation.claims[bit >> 3] & mask) return 0;
        mcc_validation.claims[bit >> 3] |= mask;
    }
    return 1;
}

/* BioHalo shares whole blocks and same-schema prefixes. Each use is walked
 * and checked against its own parent bounds; cycles and cross-schema aliases
 * remain invalid. Raw data must retain its exact extent. */
static int mcc_claim_allocation(void const *address,unsigned long size,struct tag_schema_definition const *schema)
{
    unsigned bucket=((uintptr_t)address>>2)%1024;
    struct mcc_allocation *entry;
    for (entry=mcc_validation.allocations[bucket];entry;entry=entry->next)
        if (entry->address==address) {
            if (entry->schema!=schema) return 0;
            if (entry->size==size) return 1;
            if (!schema) return 0;
            if (size>entry->size) {
                if (!mcc_claim((byte const *)address+entry->size,size-entry->size)) return 0;
                entry->size=size;
            }
            return 1;
        }
    if (!mcc_claim(address,size)) return 0;
    entry=malloc(sizeof(*entry));
    if (!entry) return 0;
    entry->address=address;entry->size=size;entry->schema=schema;
    entry->next=mcc_validation.allocations[bucket];mcc_validation.allocations[bucket]=entry;
    return 1;
}

static void mcc_allocations_dispose(void)
{
    unsigned bucket;
    for (bucket=0;bucket<1024;bucket++) {
        struct mcc_allocation *entry=mcc_validation.allocations[bucket];
        while (entry) {struct mcc_allocation *next=entry->next;free(entry);entry=next;}
        mcc_validation.allocations[bucket]=NULL;
    }
}

static int mcc_zero_block(void const *address, unsigned long bytes)
{
    unsigned long i;
    for (i=0;i<bytes;i++) if (((byte const *)address)[i]) return 0;
    return 1;
}

static int mcc_interned_name(struct tag_schema_definition const *schema)
{
    struct tag_schema_field const *field;
    if (schema->size!=32) return 0;
    for (field=schema->fields;field->type!=_tag_schema_terminator;field++)
        if (field->type!=_tag_schema_string || field->offset || field->size!=32) return 0;
    return 1;
}

static int mcc_value_block(struct tag_schema_definition const *schema)
{
    struct tag_schema_field const *field;
    for (field=schema->fields;field->type!=_tag_schema_terminator;field++) {
        if (field->type==_tag_schema_block || field->type==_tag_schema_data ||
            field->type==_tag_schema_file_data || field->type==_tag_schema_reference ||
            field->type==_tag_schema_struct || field->type==_tag_schema_check) return 0;
    }
    return 1;
}

static void mcc_problem(struct mcc_validation_context *context, int rejected, char const *message)
{
    char text[512];
    if (rejected && mcc_validation.runtime && !mcc_validation.runtime->failure[0])
        snprintf(mcc_validation.runtime->failure,sizeof(mcc_validation.runtime->failure),
            "Invalid data: tag %08lx, %.24s. See debug.txt.",
            context->handle,context->field ? context->field : "root");
    if (rejected && mcc_validation.runtime) {
        mcc_diagnostic_tag(mcc_validation.runtime,mcc_bitmaps_source_handle(mcc_validation.runtime,(uint32_t)context->handle));
        mcc_diagnostic_at(mcc_validation.runtime,"%s",context->path[0] ? context->path : "root");
        mcc_diagnostic_fail(mcc_validation.runtime,"%s",message);
    }
    if (rejected) context->rejected = TRUE;
    else ++mcc_validation.corrections;
    if (rejected || mcc_validation.corrections <= 64) {
        snprintf(text, sizeof(text), "MCC tag %08lx (%s): %s: %s", context->handle,
            context->path[0] ? context->path : "root", rejected ? "rejected" : "corrected", message);
        tag_validate_report(text);
    }
}

static struct tag_schema_group const *mcc_group(unsigned long group)
{
    struct tag_schema_group const *const *list;
    if (group == 'tagc') return &mcc_collection_group;
    if (group == 'devc') return &mcc_device_group;
    for (list = tag_schema_group_lists; *list; ++list) {
        struct tag_schema_group const *entry;
        for (entry = *list; entry->group_tag; ++entry)
            if (entry->group_tag == group) return entry;
    }
    return NULL;
}

static struct mcc_validation_instance *mcc_instance(long handle)
{
    unsigned long slot = (unsigned long)handle & 0xffffUL;
    if (handle == NONE || !mcc_validation.header || slot >= (unsigned long)mcc_validation.header->count) return NULL;
    return mcc_validation.header->instances[slot].handle == handle ? &mcc_validation.header->instances[slot] : NULL;
}

static int mcc_is_group(struct mcc_validation_instance const *instance, unsigned long const *groups)
{
    if (!groups) return 1;
    for (; *groups; ++groups)
        if (instance->group == *groups || instance->parents[0] == *groups || instance->parents[1] == *groups) return 1;
    return 0;
}

static int mcc_name_valid(struct mcc_validation_context *context, char const *name)
{
    unsigned long remaining;
    if (name == mcc_validation_empty_name) return 1;
    if (mcc_range(context->region, context->length, name, 1))
        remaining = context->length - (unsigned long)((uintptr_t)name - (uintptr_t)context->region);
    else if (mcc_range(mcc_validation.runtime->tags, mcc_validation.runtime->used, name, 1))
        remaining = mcc_validation.runtime->used - (unsigned long)((uintptr_t)name - (uintptr_t)mcc_validation.runtime->tags);
    else return 0;
    return memchr(name, 0, remaining < 256 ? remaining : 256) != NULL;
}

static long mcc_integer(byte const *address, short size, int unsign)
{
    if (size == 1) return unsign ? (long)*address : (long)*(signed char const *)address;
    if (size == 2) { short value; memcpy(&value, address, 2); return unsign ? (long)(unsigned short)value : value; }
    { long value; memcpy(&value, address, 4); return value; }
}
static void mcc_set_integer(byte *address, short size, long value)
{
    if (size == 1) *address = (byte)value;
    else if (size == 2) { short small = (short)value; memcpy(address, &small, 2); }
    else memcpy(address, &value, 4);
}

static void mcc_walk(struct mcc_validation_context *context, byte *address,
    struct tag_schema_definition const *schema, int checks);

static struct tag_schema_field const *mcc_field(struct tag_schema_definition const *schema, char const *name)
{
    struct tag_schema_field const *field;
    for (field = schema->fields; field->type != _tag_schema_terminator; ++field)
        if (field->name && !strcmp(field->name, name)) return field;
    return NULL;
}

/* Some MCC jet types carry velocity and spread only. The January runtime
 * also reads a third, rotates-up switch. Keep the authored pair and provide
 * an explicit false switch in MCC-owned tag storage; never read a following
 * allocation as that switch. Complete constant blocks retain their values. */
static int mcc_particle_type_constants(struct mcc_validation_context *context,
    byte *address, struct tag_schema_definition const *schema)
{
    struct tag_schema_field const *cf=mcc_field(schema,"physics_constants");
    struct tag_schema_field const *pf=mcc_field(schema,"initial_particle_creation_physics");
    struct tag_schema_field const *sf=mcc_field(schema,"type_states"),*mode;
    struct tag_schema_definition const *states;
    struct tag_block *constants,*block;
    int jet,required;
    long i;
    real *copy;
    if (!cf || !pf || !sf || !sf->definition) return 0;
    constants=(struct tag_block *)(address+cf->offset);
    block=(struct tag_block *)(address+sf->offset);
    states=sf->definition;mode=mcc_field(states,"particle_creation_physics");
    if (!mode) return 0;
    jet=mcc_integer(address+pf->offset,pf->size,0)==2;
    required=mcc_integer(address+pf->offset,pf->size,0)!=0;
    for (i=0;i<block->count;i++) {
        long value=mcc_integer((byte *)block->address+i*states->size+mode->offset,mode->size,0);
        if (value==2) jet=1;
        if (value) required=1;
    }
    if (!required || constants->count>=3) return 1;
    if (!jet || constants->count!=2 || context->region!=mcc_validation.runtime->tags) {
        mcc_problem(context,1,"particle creation physics has too few constants");return 0;
    }
    copy=mcc_runtime_allocate(mcc_validation.runtime,3*sizeof(*copy));
    if (!copy) {mcc_problem(context,1,"no room for MCC jet constants");return 0;}
    memcpy(copy,constants->address,2*sizeof(*copy));copy[2]=0.0f;
    if (!mcc_claim_allocation(copy,3*sizeof(*copy),cf->definition)) return 0;
    constants->address=copy;constants->count=3;
    context->length=mcc_validation.runtime->used;
    mcc_problem(context,0,"jet's absent rotates-up constant defaults to false");
    return 1;
}

/* Recover a misplaced start edge only from the surface's complete, unique
 * authored ring. Super Wacky Funtime's Flood model swaps two start edges;
 * no vertices, planes, edge links or surface ownership are changed. */
static int mcc_collision_surface_edges(struct mcc_validation_context *context,
    byte *address, struct tag_schema_definition const *schema)
{
    struct tag_schema_field const *sf=mcc_field(schema,"surfaces"),*ef=mcc_field(schema,"edges");
    struct tag_block *surfaces,*edges;
    long *first,*counts,i,j;
    int result=0;
    if (!sf || !ef || !sf->definition || !ef->definition ||
        ((struct tag_schema_definition const *)sf->definition)->size!=12 ||
        ((struct tag_schema_definition const *)ef->definition)->size!=24) return 0;
    surfaces=(struct tag_block *)(address+sf->offset);
    edges=(struct tag_block *)(address+ef->offset);
    if (!surfaces->count) return 1;
    if (surfaces->count<0 || surfaces->count>131072 || edges->count<0 || edges->count>262144) return 0;
    /* Valid BSPs need no ownership table or allocation. */
    for (i=0;i<surfaces->count;i++) {
        long start=mcc_integer((byte *)surfaces->address+i*12+4,4,0);
        byte const *edge;
        if (start<0 || start>=edges->count) break;
        edge=(byte *)edges->address+start*24;
        if (mcc_integer(edge+16,4,0)!=i && mcc_integer(edge+20,4,0)!=i) break;
    }
    if (i==surfaces->count) return 1;
    first=malloc(surfaces->count*2*sizeof(*first));
    if (!first) return 0;
    counts=first+surfaces->count;
    for (i=0;i<surfaces->count;i++) {first[i]=NONE;counts[i]=0;}
    for (i=0;i<edges->count;i++) {
        byte const *edge=(byte *)edges->address+i*24;
        for (j=0;j<2;j++) {
            long surface=mcc_integer(edge+16+j*4,4,0);
            if (surface==NONE) continue;
            if (surface<0 || surface>=surfaces->count) goto done;
            if (first[surface]==NONE) first[surface]=i;
            counts[surface]++;
        }
    }
    for (i=0;i<surfaces->count;i++) {
        byte *surface=(byte *)surfaces->address+i*12;
        long start=mcc_integer(surface+4,4,0),cursor,steps=0;
        if (start>=0 && start<edges->count) {
            byte const *edge=(byte *)edges->address+start*24;
            if (mcc_integer(edge+16,4,0)==i || mcc_integer(edge+20,4,0)==i) continue;
        }
        start=cursor=first[i];
        if (start==NONE || counts[i]<3 || counts[i]>8) goto done;
        do {
            byte const *edge;
            long side;
            if (cursor<0 || cursor>=edges->count || steps++>=counts[i]) goto done;
            edge=(byte *)edges->address+cursor*24;
            side=mcc_integer(edge+20,4,0)==i;
            if (mcc_integer(edge+16+side*4,4,0)!=i) goto done;
            cursor=mcc_integer(edge+8+side*4,4,0);
        } while (cursor!=start);
        if (steps!=counts[i]) goto done;
        mcc_set_integer(surface+4,4,start);
        mcc_problem(context,0,"collision surface start restored from its complete owned edge ring");
    }
    result=1;
done:
    if (!result) mcc_problem(context,1,"misplaced collision start has no complete unique owned edge ring");
    free(first);
    return result;
}

/* Some compiled graphs omit their parent hierarchy. Recover it only from
 * the model referenced by the same object, with matching ordered names.
 * Unnamed, unlinked slots can be filled without changing node indices.
 * Multiple associated models must agree; the normal tree check follows. */
static int mcc_animation_model_tree(struct mcc_validation_context *context,
    struct tag_block const *nodes,struct tag_schema_definition const *schema)
{
    struct mcc_validation_header *header=mcc_validation.header;
    short links[64][3];
    char names[64][32];
    long i,j;
    int pruned,incomplete=0;
    int found=0,parents_omitted=1,links_omitted=1,holes=0,partial=1,missing=0;
    if (!schema->name || schema->size!=64 || nodes->count<2 || nodes->count>64) return 0;
    for (j=0;j<nodes->count;j++) {
        byte const *node=(byte *)nodes->address+j*64;
        if (mcc_integer(node+36,2,0)!=0) parents_omitted=0;
        if (mcc_integer(node+32,2,0)!=NONE || mcc_integer(node+34,2,0)!=NONE ||
            mcc_integer(node+36,2,0)!=NONE) links_omitted=0;
    }
    /* Descent's Pelican retains node slots for absent landing-gear bones.
     * Only completely unnamed, unlinked slots qualify as holes. */
    for (j=0;j<nodes->count;j++) {
        byte const *node=(byte *)nodes->address+j*64;
        if (!node[0]) {
            unsigned k;
            for (k=0;k<32;k++) if (node[k]) return 0;
            if (!j || mcc_integer(node+32,2,0)!=NONE || mcc_integer(node+34,2,0)!=NONE ||
                mcc_integer(node+36,2,0)!=NONE) return 0;
            holes=1;
        }
    }
    if (!links_omitted && !holes) {
        byte referenced[64];
        memset(referenced,0,sizeof(referenced));referenced[0]=1;
        for (j=0;j<nodes->count;j++) {
            byte const *node=(byte *)nodes->address+j*64;
            long sibling=mcc_integer(node+32,2,0),child=mcc_integer(node+34,2,0);
            if (j && sibling>0 && sibling<nodes->count) referenced[sibling]=1;
            if (child>0 && child<nodes->count) referenced[child]=1;
        }
        for (j=1;j<nodes->count && referenced[j];j++) {}
        incomplete=j!=nodes->count;
        if (!parents_omitted && !incomplete) return 0;
    }
    if (mcc_integer((byte *)nodes->address+32,2,0)!=NONE) return 0;
    for (i=0;i<header->count;i++) {
        struct mcc_validation_instance *object=header->instances+i,*model;
        struct tag_block const *model_nodes;
        long model_handle,graph_handle;
        if (object->parents[0]!='obje' && object->parents[1]!='obje') continue;
        if (!mcc_range(context->region,context->length,object->root,72)) return 0;
        memcpy(&graph_handle,(byte *)object->root+68,4);
        if (graph_handle!=context->handle) continue;
        memcpy(&model_handle,(byte *)object->root+52,4);
        model=mcc_instance(model_handle);
        if (!model || model->group!='mode' || !mcc_range(context->region,context->length,model->root,232)) return 0;
        model_nodes=(struct tag_block const *)((byte *)model->root+184);
        if (model_nodes->count!=nodes->count ||
            !mcc_range(context->region,context->length,model_nodes->address,nodes->count*156)) return 0;
        for (j=0;j<nodes->count;j++) {
            byte const *source=(byte const *)model_nodes->address+j*156;
            byte const *target=(byte const *)nodes->address+j*64;
            if (!source[0] || !memchr(source,0,32) || !memchr(target,0,32) ||
                (target[0] && strcmp((char const *)source,(char const *)target))) return 0;
            if (found && (memcmp(links[j],source+32,6) || strcmp(names[j],(char const *)source))) return 0;
            memcpy(links[j],source+32,6);memcpy(names[j],source,32);
        }
        found=1;
    }
    if (!found) return 0;
    /* Bot Bootcamp's cyborg omits the neck's child; Arbiter's Demise's
     * Ghost omits siblings and all parents. Recover only absent edges;
     * every retained edge must match the associated model exactly. */
    for (j=0;j<nodes->count;j++) {
        byte const *node=(byte *)nodes->address+j*64;
        long k;
        for (k=0;k<3;k++) {
            long link=mcc_integer(node+32+k*2,2,0);
            if (k==2 && (!j || parents_omitted)) continue;
            if (link==NONE && links[j][k]!=NONE) missing=1;
            else if (link!=links[j][k]) partial=0;
        }
    }
    partial=partial && missing && !holes;
    /* Ruined Halo's Warthog retains disconnected sibling chains and some
     * zeroed parents. Names and nonzero parents must match the associated
     * model; retained edges may skip siblings, never cross parent groups.
     * This also covers Exile's root pointing at its last child. */
    pruned=incomplete && !holes && !links_omitted && !partial;
    if (!holes && !links_omitted && !partial && !pruned) return 0;
    if (holes || pruned) {
        /* Named nodes must still describe the same parent relationships.
         * Sibling order may differ, but every retained edge must agree with
         * the model. No animation or vertex node indices are remapped. */
        for (j=0;j<nodes->count;j++) {
            byte const *node=(byte *)nodes->address+j*64;
            long sibling,child,parent,step,cursor;
            if (!node[0]) continue;
            sibling=mcc_integer(node+32,2,0);child=mcc_integer(node+34,2,0);
            parent=mcc_integer(node+36,2,0);
            if (j && parent!=links[j][2] && !(pruned && parent==0)) return 0;
            if (sibling!=NONE && (sibling<=0 || sibling>=nodes->count || sibling==j ||
                links[sibling][2]!=links[j][2])) return 0;
            if (child!=NONE && (child<=0 || child>=nodes->count || child==j || links[child][2]!=j)) return 0;
            cursor=j;step=0;
            while (cursor!=NONE) {
                if (cursor<0 || cursor>=nodes->count || step++>=nodes->count) return 0;
                cursor=mcc_integer((byte *)nodes->address+cursor*64+32,2,0);
            }
        }
    }
    for (j=0;j<nodes->count;j++) {
        byte *node=(byte *)nodes->address+j*64;
        if (holes && !node[0]) memcpy(node,names[j],32);
        memcpy(node+32,links[j],6);
    }
    return 1;
}

/* MCC graph indices are signed shorts, rather than the Xbox authoring
 * tool's 256-animation limit. Keep node-tree and permutation-list checks
 * independent, using the published schema offsets after structural checks. */
static int mcc_animation_graph_check(struct mcc_validation_context *context,
    byte *address, struct tag_schema_definition const *schema)
{
    struct tag_schema_field const *nodes_field = mcc_field(schema, "nodes");
    struct tag_schema_field const *animations_field = mcc_field(schema, "animations");
    struct tag_schema_definition const *nodes_schema, *animations_schema;
    struct tag_schema_field const *sibling, *child, *parent, *next;
    struct tag_block const *nodes, *animations;
    byte reached[64], *states;
    short queue[64];
    long head, tail, first;
    int recovered,parents_recovered=0;
    if (!nodes_field || !animations_field || !nodes_field->definition || !animations_field->definition) return 0;
    nodes_schema = nodes_field->definition; animations_schema = animations_field->definition;
    sibling = mcc_field(nodes_schema, "next_sibling_node_index");
    child = mcc_field(nodes_schema, "first_child_node_index");
    parent = mcc_field(nodes_schema, "parent_node_index");
    next = mcc_field(animations_schema, "next_animation_index");
    if (!sibling || !child || !parent || !next) return 0;
    nodes = (struct tag_block const *)(address + nodes_field->offset);
    animations = (struct tag_block const *)(address + animations_field->offset);
    if (nodes->count < 0 || nodes->count > 64 || animations->count < 0 || animations->count > 32767) return 0;
    recovered=mcc_animation_model_tree(context,nodes,nodes_schema);
    memset(reached, 0, sizeof(reached)); head = tail = 0;
    if (nodes->count) {
        byte const *root = nodes->address;
        long root_sibling=mcc_integer(root + sibling->offset, 2, 0);
        /* Validate the complete child tree before normalizing the unused
         * root sibling. Some MCC graphs point it back into that tree. */
        if (root_sibling!=NONE && (root_sibling<0 || root_sibling>=nodes->count)) return 0;
        queue[tail++] = 0; reached[0] = 1;
    }
    while (head < tail) {
        long index = queue[head++];
        byte const *node = (byte const *)nodes->address + index * nodes_schema->size;
        long child_index = mcc_integer(node + child->offset, 2, 0);
        while (child_index != NONE) {
            byte *descendant;
            if (child_index < 0 || child_index >= nodes->count || reached[child_index] || tail >= 64) return 0;
            descendant = (byte *)nodes->address + child_index * nodes_schema->size;
            /* An explicit, uniquely reached child supplies an omitted
             * parent. Conflicting parents and cyclic links still fail. */
            if (mcc_integer(descendant + parent->offset, 2, 0)==NONE) {
                mcc_set_integer(descendant+parent->offset,2,index);
                parents_recovered=1;
            }
            if (mcc_integer(descendant + parent->offset, 2, 0) != index) return 0;
            reached[child_index] = 1; queue[tail++] = (short)child_index;
            child_index = mcc_integer(descendant + sibling->offset, 2, 0);
        }
    }
    if (tail != nodes->count) return 0;
    if (parents_recovered) mcc_problem(context,0,"omitted animation parents restored from the complete child tree");
    if (recovered) mcc_problem(context,0,"animation hierarchy restored from its associated model's matching nodes");
    if (nodes->count && mcc_integer((byte *)nodes->address+sibling->offset,2,0)!=NONE) {
        short none=NONE;
        memcpy((byte *)nodes->address+sibling->offset,&none,sizeof(none));
        mcc_problem(context,0,"unused root sibling cleared after validating the complete node tree");
    }
    if (!animations->count) return 1;
    states = malloc(animations->count);
    if (!states) return 0;
    memset(states, 0, animations->count);
    for (first = 0; first < animations->count; ++first) {
        long index = first;
        while (index != NONE && !states[index]) {
            byte const *animation = (byte const *)animations->address + index * animations_schema->size;
            states[index] = 1;
            index = mcc_integer(animation + next->offset, 2, 0);
            if (index != NONE && (index < 0 || index >= animations->count)) { free(states); return 0; }
        }
        if (index != NONE && states[index] == 1) { free(states); return 0; }
        index = first;
        while (index != NONE && states[index] == 1) {
            byte const *animation = (byte const *)animations->address + index * animations_schema->size;
            states[index] = 2; index = mcc_integer(animation + next->offset, 2, 0);
        }
    }
    free(states); (void)context;
    return 1;
}

/* Every container's extents are verified before any of its indices or child
 * elements are visited. Claims reject overlap and cycles before dereference. */
static void mcc_fields(struct mcc_validation_context *context, byte *address,
    struct tag_schema_definition const *schema, int stage, int nesting)
{
    struct tag_schema_field const *field;
    byte *previous_structure = context->structure;
    size_t prefix=strlen(context->path);
    if (!schema || schema->size <= 0 || nesting >= MCC_VALIDATION_DEPTH ||
        !mcc_range(context->region, context->length, address, (unsigned long)schema->size)) {
        mcc_problem(context, 1, "schema structure is outside loaded data or excessively nested"); return;
    }
    context->structure = address;
    for (field = schema->fields; field->type != _tag_schema_terminator && !context->rejected; ++field) {
        int index;
        uint64_t extent = (uint64_t)(unsigned short)field->size * (unsigned short)field->count;
        context->field = field->name;
        snprintf(context->path+prefix,sizeof(context->path)-prefix,"%s%s",prefix ? "." : "",field->name ? field->name : "(unnamed field)");
        /* Inline structures may deliberately omit their trailing unused
         * fields; size is still the array stride, not the bytes visited. */
        if (field->type == _tag_schema_struct && field->definition && field->count > 0) {
            struct tag_schema_definition const *inner = field->definition;
            extent = (uint64_t)(unsigned short)field->size * (field->count - 1) + (unsigned long)inner->size;
            if (inner->size <= 0 || inner->size > field->size) extent = UINT64_MAX;
        }
        if (field->count <= 0 || field->size < 0 || field->offset < 0 || field->offset > schema->size ||
            extent > (unsigned long)(schema->size - field->offset)) {
            mcc_problem(context, 1, "schema field leaves its structure"); break;
        }
        for (index = 0; index < field->count && !context->rejected; ++index) {
            byte *member = address + field->offset + index * field->size;
            size_t field_length;
            snprintf(context->path+prefix,sizeof(context->path)-prefix,"%s%s",prefix ? "." : "",field->name ? field->name : "(unnamed field)");
            field_length=strlen(context->path);
            if (field->count>1) snprintf(context->path+field_length,sizeof(context->path)-field_length,"[%d]",index);
            if (field->type == _tag_schema_struct) {
                mcc_fields(context, member, field->definition, stage, nesting + 1);
            } else if (field->type == _tag_schema_block) {
                struct tag_block *block = (struct tag_block *)member;
                struct tag_schema_definition const *element = field->definition;
                if (stage == 0) {
                    unsigned long bytes;
                    long maximum = field->maximum;
                    block->definition = NULL;
                    /* The MCC table owns extended names; never truncate indices
                     * embedded in scripts, placements and saved objects. */
                    if (schema->name && !strcmp(schema->name, "scenario") &&
                        !strcmp(field->name, "object_names")) {
                        maximum = MCC_OBJECT_NAMES_MAXIMUM;
                        if (block->count > maximum) {
                            mcc_problem(context, 1, "object-name count exceeds MCC's 32767-name capacity; names cannot be truncated");
                            break;
                        }
                    }
                    /* Scenery palettes use dynamic tag blocks at runtime.
                     * Preserve MCC entries beyond the Xbox authoring limit;
                     * placement indices and native iteration remain signed shorts. */
                    if (schema->name && !strcmp(schema->name, "scenario") &&
                        !strcmp(field->name, "scenery_palette")) {
                        maximum = 32767;
                        if (block->count > maximum) {
                            mcc_problem(context, 1, "scenery palette exceeds MCC's 32767-entry capacity; entries cannot be truncated");
                            break;
                        }
                    }
                    if (!element || element->size <= 0 || block->count < 0 ||
                        (unsigned long)block->count > context->length / (unsigned long)element->size) {
                        mcc_problem(context, 1, "invalid block count"); break;
                    }
                    bytes = (unsigned long)block->count * (unsigned long)element->size;
                    /* BioHalo interns scalar records in other allocations.
                     * Isolate overlapping values before correcting them. A
                     * pointer-bearing block is eligible only when entirely
                     * zero (no pointers or counts). Its checks still apply. */
                    if (bytes && context->region==mcc_validation.runtime->tags &&
                        mcc_range(context->region,context->length,block->address,bytes) &&
                        (mcc_interned_name(element) ||
                         (mcc_validation_any_claimed(block->address,bytes) &&
                          (mcc_value_block(element) || mcc_zero_block(block->address,bytes))))) {
                        void *copy;
                        if (!(copy=mcc_runtime_allocate(mcc_validation.runtime,bytes))) {
                            mcc_problem(context,1,"no room to isolate interned block");break;
                        }
                        memcpy(copy,block->address,bytes);block->address=copy;
                        context->length=mcc_validation.runtime->used;
                    }
                    if (bytes && (!mcc_range(context->region, context->length, block->address, bytes) ||
                        !mcc_claim_allocation(block->address, bytes, element))) {
                        char message[192];
                        snprintf(message,sizeof(message),"block %s at +%08lx (%lu bytes) leaves loaded data or overlaps another allocation",
                            element->name,(unsigned long)((uintptr_t)block->address-(uintptr_t)context->region),bytes);
                        mcc_problem(context, 1, message); break;
                    }
                    if (schema->name && !strcmp(schema->name, "game_globals") && !strcmp(field->name, "grenades")) {
                        maximum = 4;
                        if (block->count < 2 || block->count > maximum) {
                            mcc_problem(context, 1, "MCC requires two to four grenade kinds"); break;
                        }
                    }
                    if (maximum > 0 && block->count > maximum &&
                        !(field->flags & FLAG(_tag_schema_tool_maximum_bit)) &&
                        /* MCC appends first-person animation slots. Native
                         * callers index known slots, with no fixed storage. */
                        !(schema->name && !strcmp(schema->name, "animation_graph_first_person_weapon_animations") &&
                          !strcmp(field->name, "animations"))) {
                        block->count = maximum;
                        mcc_problem(context, 0, "block reduced to engine capacity");
                    }
                    if (!block->count) block->address = NULL;
                } else {
                    long item;
                    size_t block_length=strlen(context->path);
                    for (item = 0; item < block->count && !context->rejected; ++item) {
                        snprintf(context->path+block_length,sizeof(context->path)-block_length,"[%ld]",item);
                        mcc_walk(context, (byte *)block->address + item * element->size, element, stage == 2);
                    }
                    context->path[block_length]=0;
                }
            } else if (field->type == _tag_schema_data || field->type == _tag_schema_file_data) {
                if (stage == 0) {
                    struct tag_data *data = (struct tag_data *)member;
                    long maximum = field->maximum;
                    /* The shared scenario schema describes Xbox/CE's 19001
                     * slots. MCC's separately validated syntax arena keeps
                     * its own complete span through this later schema pass. */
                    if (field->type == _tag_schema_data && schema->name &&
                        !strcmp(schema->name, "scenario") && field->name &&
                        !strcmp(field->name, "hs_syntax_data"))
                        maximum = MCC_SYNTAX_MAXIMUM_DATA_BYTES;
                    data->definition = NULL;
                    if (data->size < 0) { mcc_problem(context, 1, "negative data size"); break; }
                    if (field->type == _tag_schema_file_data) {
                        if (!mcc_validation_file_contains((struct tag_validation *)context, data->file_offset, data->size))
                            mcc_problem(context, 1, "file data leaves MCC stream");
                    } else if (data->size) {
                        if (mcc_range(context->region, context->length, data->address, data->size)) {
                            /* Optimized animation, text and unused input-device
                             * metadata can share prefixes or padding. Isolate
                             * these bytes before the normal format callbacks. */
                            if (schema->name && (!strcmp(schema->name,"animation") ||
                                !strcmp(schema->name,"string_list_entry") ||
                                !strcmp(schema->name,"unicode_string_list_entry") ||
                                !strcmp(schema->name,"mcc_device_defaults")) &&
                                context->region==mcc_validation.runtime->tags &&
                                mcc_validation_any_claimed(data->address,data->size)) {
                                void *copy=mcc_runtime_allocate(mcc_validation.runtime,data->size);
                                if (!copy) {mcc_problem(context,1,"no room to isolate shared data");break;}
                                memcpy(copy,data->address,data->size);data->address=copy;
                                context->length=mcc_validation.runtime->used;
                            }
                            if (!mcc_claim_allocation(data->address, (unsigned long)data->size, NULL))
                                mcc_problem(context, 1, "data overlaps another allocation");
                        } else if (!mcc_geometry_contains(mcc_validation.runtime, data->address, data->size)) {
                            mcc_problem(context, 1, "data leaves loaded bytes and owned geometry");
                        }
                    }
                    if (maximum > 0 && data->size > maximum) {
                        data->size = maximum; mcc_problem(context, 0, "data reduced to engine capacity");
                    }
                    if (!data->size && field->type == _tag_schema_data) data->address = NULL;
                }
            } else if (field->type == _tag_schema_check) {
                if (stage == 2) {
                    int accepted;
                    context->geometry_access = FALSE;
                    mcc_validation.active = context;
                    mcc_validation_callbacks = &mcc_callbacks;
                    if (!strcmp(field->name, "bitmap_data_check"))
                        accepted = mcc_bitmaps_valid(mcc_validation.runtime, (struct bitmap_data *)address);
                    else if (!strcmp(field->name, "model_geometry_part_check"))
                        accepted = mcc_geometry_part_valid(mcc_validation.runtime, address);
                    else if (!strcmp(field->name, "bitmap_group_check"))
                        accepted = mcc_bitmaps_group_valid(mcc_validation.runtime, address);
                    else if (!strcmp(field->name, "collision_model_bsp_check"))
                        accepted = mcc_collision_surface_edges(context,address,schema) &&
                            field->check((struct tag_validation *)context,address);
                    else if (!strcmp(field->name, "animation_graph_check"))
                        accepted = mcc_animation_graph_check(context, address, schema);
                    else if (!strcmp(field->name, "particle_system_type_check"))
                        accepted = mcc_particle_type_constants(context,address,schema) &&
                            field->check((struct tag_validation *)context,address);
                    else accepted = field->check((struct tag_validation *)context, address);
                    if (!accepted && !context->rejected)
                        mcc_problem(context, 1, "schema callback rejected data");
                    mcc_validation.active = NULL;
                    mcc_validation_callbacks = NULL;
                }
            } else if (stage == 1) {
                int unsign = (field->flags & FLAG(_tag_schema_unsigned_bit)) != 0;
                int optional = (field->flags & FLAG(_tag_schema_none_bit)) != 0;
                if (field->type == _tag_schema_reference || field->type == _tag_schema_tag_index) {
                    struct tag_reference *reference = (struct tag_reference *)member;
                    long *handle = field->type == _tag_schema_reference ? &reference->index : (long *)member;
                    struct mcc_validation_instance *instance = mcc_instance(*handle);
                    if (*handle != NONE && (!instance || !mcc_is_group(instance, field->definition))) {
                        *handle = NONE; mcc_problem(context, 0, "invalid tag reference removed");
                    }
                    if (field->type == _tag_schema_reference) {
                        if (*handle != NONE && instance) {
                            reference->group_tag = instance->group;
                            reference->name = instance->name;
                            reference->name_length = (long)strlen(instance->name);
                        } else {
                            reference->name = mcc_validation_empty_name;
                            reference->name_length = 0;
                        }
                    }
                } else if (field->type == _tag_schema_enum || field->type == _tag_schema_block_index) {
                    long value = mcc_integer(member, field->size, unsign);
                    long absent = unsign && field->size == 1 ? 255 : unsign && field->size == 2 ? 65535 : NONE;
                    long maximum = field->maximum;
                    if (field->type == _tag_schema_enum && schema->name &&
                        (!strcmp(schema->name, "unit") || !strcmp(schema->name, "equipment")) &&
                        !strcmp(field->name, "grenade_type")) maximum = 4;
                    if (field->type == _tag_schema_enum && schema->name &&
                        !strcmp(schema->name, "hud_absolute_placement") &&
                        !strcmp(field->name, "corner")) maximum = 9;
                    if (field->type == _tag_schema_block_index) {
                        byte *target = NULL;
                        if (field->target_level == TAG_SCHEMA_ROOT) target = context->stack[0].address;
                        else if (field->target_level == TAG_SCHEMA_STRUCTURE) target = context->structure;
                        else if (field->target_level >= 0 && field->target_level < context->depth)
                            target = context->stack[context->depth - 1 - field->target_level].address;
                        if (!target || field->target_offset < 0 ||
                            !mcc_range(context->region, context->length, target + field->target_offset, sizeof(struct tag_block))) {
                            mcc_problem(context, 1, "invalid schema index target"); break;
                        }
                        maximum = ((struct tag_block *)(target + field->target_offset))->count;
                        if (!maximum) optional = 1;
                    }
                    if (!((value >= 0 && value < maximum && value != absent) || (optional && value == absent))) {
                        mcc_set_integer(member, field->size, optional ? NONE : 0);
                        mcc_problem(context, 0, "index or enumeration corrected");
                    }
                } else if (field->type == _tag_schema_string && field->size) {
                    long width = field->size;
                    /* MCC flag names occupy 32 bytes. The native declaration
                     * calls the last byte padding; it can hold the terminator. */
                    if (schema->name && !strcmp(schema->name, "cutscene_flag") &&
                        !strcmp(field->name, "name") && width == 31 &&
                        field->offset + 32 <= schema->size) width = 32;
                    if (!memchr(member, 0, width)) {
                        member[width - 1] = 0; mcc_problem(context, 0, "string terminated");
                    }
                } else if (field->type == _tag_schema_reset) {
                    if (field->size <= 4) mcc_set_integer(member, field->size, field->target_offset);
                    else memset(member, 0, field->size);
                }
            }
        }
    }
    context->structure = previous_structure;
    context->path[prefix]=0;
}

static void mcc_walk(struct mcc_validation_context *context, byte *address,
    struct tag_schema_definition const *schema, int checks)
{
    unsigned long alignment;
    int ancestor;
    if (!schema || schema->size <= 0) { mcc_problem(context, 1, "invalid schema size"); return; }
    if (++context->visits>4000000UL) {mcc_problem(context,1,"tag traversal exceeds validation budget");return;}
    for (ancestor=0;ancestor<context->depth;ancestor++)
        if (context->stack[ancestor].address==address) {mcc_problem(context,1,"cyclic tag block");return;}
    alignment = schema->size % 4 == 0 ? 4 : schema->size % 2 == 0 ? 2 : 1;
    if ((uintptr_t)address & (alignment - 1)) { mcc_problem(context, 1, "misaligned tag or block element"); return; }
    if (context->depth >= MCC_VALIDATION_DEPTH) { mcc_problem(context, 1, "block nesting exceeds limit"); return; }
    context->stack[context->depth].address = address;
    context->stack[context->depth++].schema = schema;
    if (checks) mcc_fields(context, address, schema, 2, 0);
    else {
        mcc_fields(context, address, schema, 0, 0);
        if (!context->rejected) mcc_fields(context, address, schema, 1, 0);
    }
    --context->depth;
}

static int mcc_root_compare(void const *a, void const *b)
{
    struct mcc_validation_instance const *x=*(struct mcc_validation_instance const *const *)a;
    struct mcc_validation_instance const *y=*(struct mcc_validation_instance const *const *)b;
    uintptr_t first=(uintptr_t)x->root, second=(uintptr_t)y->root;
    return first<second ? -1 : first>second;
}

int mcc_tags_validate(struct mcc_runtime *runtime)
{
    struct mcc_validation_context context;
    struct mcc_validation_header *header;
    long i;
    struct mcc_validation_instance **roots;
    int pass;
    if (!runtime || !runtime->tags || ((uintptr_t)runtime->tags & 3) ||
        runtime->used < sizeof(*header) || runtime->used > runtime->capacity ||
        runtime->capacity > MCC_VALIDATION_CAPACITY) return mcc_diagnostic_fail(runtime,"tag validation requires an aligned loaded tag region within the MCC capacity");
    mcc_validation_dispose(mcc_validation.runtime);
    mcc_validation.runtime = runtime;
    header = mcc_validation.header = (struct mcc_validation_header *)runtime->tags;
    mcc_validation.claim_bytes = (runtime->capacity + 7) / 8;
    mcc_validation.claims = malloc(mcc_validation.claim_bytes);
    if (!mcc_validation.claims) {mcc_diagnostic_tag(runtime,UINT32_MAX);return mcc_diagnostic_fail(runtime,"not enough memory for tag validation ownership tracking");}
    memset(mcc_validation.claims, 0, mcc_validation.claim_bytes);
    memset(&context, 0, sizeof(context)); context.region = runtime->tags; context.length = runtime->used;
    context.handle = NONE;
    if (header->signature != 'tags' || header->count <= 0 || header->count > 65535 ||
        ((uintptr_t)header->instances & 3) ||
        !mcc_range(runtime->tags, runtime->used, header->instances, (unsigned long)header->count * sizeof(*header->instances)) ||
        !mcc_claim(header, sizeof(*header)) || !mcc_claim(header->instances, header->count * sizeof(*header->instances))) {
        mcc_problem(&context, 1, "invalid tag header or instance table"); return 0;
    }
    roots=malloc(header->count*sizeof(*roots));
    mcc_validation.aliases=malloc(header->count);
    if (!roots || !mcc_validation.aliases) {free(roots);mcc_diagnostic_tag(runtime,UINT32_MAX);return mcc_diagnostic_fail(runtime,"not enough memory for tag validation root/alias tables");}
    memset(mcc_validation.aliases,0,header->count);
    for (i = 0; i < header->count && !context.rejected; ++i) {
        struct mcc_validation_instance *instance = &header->instances[i];
        struct tag_schema_group const *group = mcc_group(instance->group);
        context.handle = instance->handle;
        if (((unsigned long)instance->handle & 65535UL) != (unsigned long)i || !group) {
            mcc_problem(&context, 1, "invalid tag handle or unsupported tag group"); break;
        }
        instance->parents[0] = group->parent_group_tags[0]; instance->parents[1] = group->parent_group_tags[1];
        if (!mcc_name_valid(&context, instance->name)) instance->name = mcc_validation_empty_name;
        if (instance->group == 'sbsp') { instance->root = NULL; continue; }
        if (!group->definition) {
            if (!mcc_range(context.region, context.length, instance->root, 1)) mcc_problem(&context, 1, "tag root outside cache");
            continue;
        }
        if (!mcc_range(context.region, context.length, instance->root, group->definition->size))
            mcc_problem(&context, 1, "tag root outside cache");
    }
    for (i=0;i<header->count;i++) roots[i]=header->instances+i;
    qsort(roots,header->count,sizeof(*roots),mcc_root_compare);
    for (i=0;i<header->count && !context.rejected;i++) {
        struct mcc_validation_instance *instance=roots[i];
        struct tag_schema_group const *group=mcc_group(instance->group);
        context.handle=instance->handle;
        if (!instance->root || !group || !group->definition) continue;
        /* Compiled MCC caches can deduplicate complete tags (BioHalo's
         * landing-beacon lens flares). Validate each same-group root once;
         * partial overlaps and roots interpreted as different groups fail. */
        if (i && roots[i-1]->root==instance->root && roots[i-1]->group==instance->group)
            mcc_validation.aliases[instance-header->instances]=1;
        else if (!mcc_claim(instance->root,group->definition->size))
            mcc_problem(&context,1,"tag root overlaps another allocation");
    }
    free(roots);
    for (pass = 0; pass < 2 && !context.rejected; ++pass)
        for (i = 0; i < header->count && !context.rejected; ++i) {
            struct mcc_validation_instance *instance = &header->instances[i];
            struct tag_schema_group const *group = mcc_group(instance->group);
            context.handle = instance->handle;
            mcc_loading_update(MCC_LOADING_VALIDATION,
                (unsigned long)pass * header->count + i, (unsigned long)header->count * 2);
            if (instance->root && group && group->definition && !mcc_validation.aliases[i])
                mcc_walk(&context, instance->root, group->definition, pass);
        }
    if (!mcc_instance(header->scenario) || mcc_instance(header->scenario)->group != 'scnr') {
        context.handle=header->scenario;context.path[0]=0;
        mcc_problem(&context, 1, "scenario handle does not identify a scenario");
    }
    return !context.rejected;
}

int mcc_bsp_validate(struct mcc_runtime *runtime, long handle, void *address, long size)
{
    struct mcc_validation_context context;
    struct mcc_validation_bsp_header *header = address;
    struct mcc_validation_instance *instance;
    struct tag_schema_group const *schema = mcc_group('sbsp');
    unsigned long first, bit;
    mcc_diagnostic_tag(runtime,(uint32_t)handle);
    mcc_diagnostic_at(runtime,"BSP validation");
    if (mcc_validation.runtime != runtime || !mcc_validation.claims || ((uintptr_t)address & 3) ||
        size < (long)sizeof(*header) ||
        !mcc_range(runtime->tags, runtime->capacity, address, (unsigned long)size)) return mcc_diagnostic_fail(runtime,"BSP validation state is missing, or the loaded BSP range is invalid/misaligned");
    first = (unsigned long)((uintptr_t)address - (uintptr_t)runtime->tags);
    if (first < runtime->used) return mcc_diagnostic_fail(runtime,"BSP allocation overlaps loaded tag data");
    instance = mcc_instance(handle);
    if (!instance || instance->group != 'sbsp' || !schema || !schema->definition) return mcc_diagnostic_fail(runtime,"BSP handle is stale, has the wrong tag group, or has no validation schema");
    mcc_allocations_dispose();
    for (bit = runtime->used; bit < runtime->capacity; ++bit) mcc_validation.claims[bit >> 3] &= (byte)~(1U << (bit & 7));
    memset(&context, 0, sizeof(context)); context.region = address; context.length = size; context.handle = handle;
    if (header->signature != 'sbsp' || !mcc_claim(header, sizeof(*header)) ||
        !mcc_range(address, size, header->root, schema->definition->size) || !mcc_claim(header->root, schema->definition->size)) {
        mcc_problem(&context, 1, "invalid BSP header or root"); return 0;
    }
    mcc_walk(&context, header->root, schema->definition, 0);
    if (!context.rejected) mcc_walk(&context, header->root, schema->definition, 1);
    return !context.rejected;
}

void mcc_validation_dispose(struct mcc_runtime *runtime)
{
    if (runtime != mcc_validation.runtime) return;
    mcc_allocations_dispose();
    if (mcc_validation.claims) free(mcc_validation.claims);
    if (mcc_validation.aliases) free(mcc_validation.aliases);
    memset(&mcc_validation, 0, sizeof(mcc_validation));
}
boolean mcc_validation_owns(struct tag_validation *validation)
{
    return validation && validation == (struct tag_validation *)mcc_validation.active;
}
boolean mcc_validation_callback_active(void) { return mcc_validation.active != NULL; }
void mcc_validation_message(struct tag_validation *validation, boolean refuse, char const *format, va_list arguments)
{
    char text[384]; vsnprintf(text, sizeof(text), format, arguments);
    mcc_problem((struct mcc_validation_context *)validation, refuse, text);
}
boolean mcc_validation_file_contains(struct tag_validation *validation, long offset, long size)
{
    struct mcc_runtime *runtime = mcc_validation.runtime;
    struct mcc_validation_context *context=(struct mcc_validation_context *)validation;
    if (!context || offset < 0 || size < 0 || !runtime) return FALSE;
    return ((unsigned long)offset <= runtime->source.size &&
        (unsigned long)size <= runtime->source.size - (unsigned long)offset) ||
        mcc_audio_contains(runtime, context->handle, (uint32_t)offset, (uint32_t)size) ||
        mcc_bitmaps_contains(runtime, (uint32_t)offset, (uint32_t)size);
}
void *mcc_validation_root(struct tag_validation *validation)
{
    struct mcc_validation_context *context = (struct mcc_validation_context *)validation;
    return context->depth ? context->stack[0].address : NULL;
}
boolean mcc_validation_contains(struct tag_validation *validation, void const *address, unsigned long size)
{
    struct mcc_validation_context *context = (struct mcc_validation_context *)validation;
    return mcc_range(context->region, context->length, address, size) ||
        (context->geometry_access && mcc_geometry_contains(mcc_validation.runtime, address, size));
}
void *mcc_validation_tag_get(struct tag_validation *validation, long handle, unsigned long group)
{
    struct mcc_validation_instance *instance = mcc_instance(handle);
    unsigned long groups[2]; (void)validation; groups[0] = group; groups[1] = 0;
    return instance && mcc_is_group(instance, groups) ? instance->root : NULL;
}
void *mcc_validation_buffer_data(struct tag_validation *validation, void const *buffer, boolean indices)
{
    struct mcc_validation_context *context = (struct mcc_validation_context *)validation;
    unsigned long bytes;
    /* The BSP header's second table contains lightmap vertex buffers; only
     * the model tag header's second table contains index buffers. */
    int index_buffer = context->region == mcc_validation.runtime->tags && indices;
    void *data = mcc_geometry_buffer_data(mcc_validation.runtime, buffer, index_buffer, &bytes);
    if (data) context->geometry_access = TRUE;
    return data;
}
boolean mcc_validation_any_claimed(void const *address, unsigned long size)
{
    unsigned long first, bit;
    if (!mcc_validation.runtime) return TRUE;
    if (mcc_geometry_contains(mcc_validation.runtime, address, size)) return FALSE;
    if (!mcc_range(mcc_validation.runtime->tags, mcc_validation.runtime->capacity, address, size)) return TRUE;
    first = (unsigned long)((uintptr_t)address - (uintptr_t)mcc_validation.runtime->tags);
    for (bit = first; bit < first + size; ++bit)
        if (mcc_validation.claims[bit >> 3] & (1U << (bit & 7))) return TRUE;
    return FALSE;
}
