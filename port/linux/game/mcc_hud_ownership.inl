/* HUD conversion writes per-consumer values into a cache which may intern
 * roots, whole blocks and scalar records. Plan from pristine bytes, isolate
 * consumers, then publish their pointers before normalizing any placement. */
enum mcc_hud_block_kind {
    MCC_HUD_AUX_OVERLAYS, MCC_HUD_AUX_METERS, MCC_HUD_STATICS,
    MCC_HUD_METERS, MCC_HUD_NUMBERS, MCC_HUD_CROSSHAIRS, MCC_HUD_OVERLAYS,
    MCC_HUD_CROSSHAIR_ITEMS, MCC_HUD_OVERLAY_ITEMS
};

/* Match the later MCC validator's bounded traversal budget. */
#define MCC_HUD_MAX_VISITS 4000000u

struct mcc_hud_use {
    unsigned char *source, *owned;
    uint32_t bytes, parent, offset, kind;
};

struct mcc_hud_plan {
    struct mcc_hud_use *uses;
    unsigned char *frames;
    uint32_t count, capacity, visits, tag_count;
};

static void mcc_hud_plan_dispose(struct mcc_hud_plan *plan)
{
    if (plan->uses) free(plan->uses);
    if (plan->frames) free(plan->frames);
}

static int mcc_hud_plan_add(struct mcc_hud_plan *plan,unsigned char *source,
    uint32_t bytes,uint32_t parent,uint32_t offset,uint32_t kind)
{
    struct mcc_hud_use *use;
    if (plan->count==plan->capacity) {
        uint32_t capacity=plan->capacity ? plan->capacity*2 : 64;
        struct mcc_hud_use *grown;
        if (capacity>MCC_HUD_MAX_VISITS) capacity=MCC_HUD_MAX_VISITS;
        if (plan->count==capacity) return 0;
        grown=malloc(capacity*sizeof(*grown));
        if (!grown) return 0;
        if (plan->count) memcpy(grown,plan->uses,plan->count*sizeof(*grown));
        if (plan->uses) free(plan->uses);
        plan->uses=grown;plan->capacity=capacity;
    }
    use=plan->uses+plan->count++;
    use->source=use->owned=source;use->bytes=bytes;
    use->parent=parent;use->offset=offset;use->kind=kind;
    return 1;
}

/* Follow ownership back to the author's tag before reporting a copied block. */
static void mcc_hud_plan_context(struct mcc_hud_plan const *plan,struct mcc_runtime *r,
    uint32_t parent,uint32_t offset,uint32_t kind)
{
    static char const *const fields[]={"auxiliary_overlays","auxiliary_meters","static_elements",
        "meter_elements","number_elements","crosshair_overlays","screen_overlays"};
    uint32_t root=parent;
    while (plan->uses[root].parent!=UINT32_MAX) root=plan->uses[root].parent;
    mcc_diagnostic_tag(r,((uint32_t *)(r->tag_index+plan->uses[root].offset*32))[3]);
    if (kind<=MCC_HUD_OVERLAYS) mcc_diagnostic_at(r,"%s",fields[kind]);
    else if (plan->uses[parent].parent==UINT32_MAX) mcc_diagnostic_at(r,"overlays.items");
    else mcc_diagnostic_at(r,"%s[%lu].items",kind==MCC_HUD_CROSSHAIR_ITEMS ? "crosshair_overlays" : "screen_overlays",
        (unsigned long)((offset-0x34)/0x68));
}

static int mcc_hud_plan_block(struct mcc_hud_plan *plan,struct mcc_runtime *r,
    uint32_t parent,uint32_t offset,uint32_t stride,uint32_t kind)
{
    uint32_t count;
    unsigned char *source=mcc_hud_block(r,plan->uses[parent].source+offset,stride,&count);
    mcc_hud_plan_context(plan,r,parent,offset,kind);
    if (!source) return mcc_diagnostic_fail(r,"HUD block count or address is outside loaded tag data");
    if (count>MCC_HUD_MAX_VISITS-plan->visits)
        return mcc_diagnostic_fail(r,"HUD block exceeds the ownership traversal budget of %lu records",(unsigned long)MCC_HUD_MAX_VISITS);
    plan->visits+=count;
    if (count && !mcc_hud_plan_add(plan,source,count*stride,parent,offset,kind))
        return mcc_diagnostic_fail(r,"not enough memory or ownership records to track the HUD block");
    return 1;
}

static int mcc_hud_use_compare(void const *a,void const *b)
{
    struct mcc_hud_use const *x=*(struct mcc_hud_use const *const *)a;
    struct mcc_hud_use const *y=*(struct mcc_hud_use const *const *)b;
    uintptr_t first=(uintptr_t)x->source,second=(uintptr_t)y->source;
    if (first!=second) return first<second ? -1 : 1;
    if (x->kind!=y->kind) return x->kind<y->kind ? -1 : 1;
    if (x->bytes!=y->bytes) return x->bytes>y->bytes ? -1 : 1;
    return x<y ? -1 : x>y;
}

static int mcc_hud_plan_prepare(struct mcc_hud_plan *plan,struct mcc_runtime *r)
{
    struct mcc_hud_use **sorted=NULL;
    uint32_t i,n,roots;
    int ok=0;
    char const *problem="not enough memory for HUD ownership planning";
    mcc_diagnostic_tag(r,UINT32_MAX);
    mcc_diagnostic_at(r,"HUD ownership planning");
    if (!r->report.tag_count || r->report.tag_count>65535)
        return mcc_diagnostic_fail(r,"HUD ownership tag count must be 1..65535");
    plan->tag_count=r->report.tag_count;
    plan->frames=malloc(plan->tag_count);
    if (!plan->frames) return mcc_diagnostic_fail(r,"not enough memory for HUD ownership frames");
    memset(plan->frames,0,plan->tag_count);
    for (i=0;i<plan->tag_count;i++) {
        uint32_t *entry=(uint32_t *)(r->tag_index+i*32),bytes=0;
        unsigned char *source;
        switch (entry[0]) {
        case 'unhi': bytes=0x56C;break;
        case 'wphi': bytes=0x17C;break;
        case 'grhi': bytes=0x1F8;break;
        case 'hudg': bytes=sizeof(struct hud_globals_definition);break;
        case 'hud#': bytes=0x64;break;
        }
        if (!bytes) continue;
        mcc_diagnostic_tag(r,entry[3]);
        mcc_diagnostic_at(r,"HUD root");
        source=mcc_runtime_pointer(r,entry[5],bytes);
        if (!source) {
            problem="HUD root is outside loaded tag data";goto done;
        }
        if (!mcc_hud_plan_add(plan,source,bytes,UINT32_MAX,i,entry[0])) goto done;
    }
    roots=plan->count;plan->visits=roots;
    if (!roots) {ok=1;goto done;}
    mcc_diagnostic_tag(r,UINT32_MAX);
    mcc_diagnostic_at(r,"HUD ownership planning");
    sorted=malloc(roots*sizeof(*sorted));
    if (!sorted) goto done;
    for (i=0;i<roots;i++) sorted[i]=plan->uses+i;
    qsort(sorted,roots,sizeof(*sorted),mcc_hud_use_compare);
    for (i=1;i<roots;i++)
        if (sorted[i]->source==sorted[i-1]->source && sorted[i]->kind==sorted[i-1]->kind)
            plan->frames[sorted[i]->offset]=2;
    free(sorted);sorted=NULL;
    for (i=0;i<roots;i++) {
        uint32_t group=plan->uses[i].kind,entry_index=plan->uses[i].offset;
        if (plan->frames[entry_index]==2) continue;
        problem=NULL; /* mcc_hud_plan_block reports its own precise failure. */
        if (group=='unhi') {
            if (!mcc_hud_plan_block(plan,r,i,0x3A4,0x84,MCC_HUD_AUX_OVERLAYS) ||
                !mcc_hud_plan_block(plan,r,i,0x3CC,0x144,MCC_HUD_AUX_METERS)) goto done;
        } else if (group=='wphi') {
            static uint32_t const strides[]={0xB4,0xB4,0xA0,0x68,0x68};
            for (n=0;n<NUMBEROF(strides);n++) {
                uint32_t first=plan->count,j,count;
                if (!mcc_hud_plan_block(plan,r,i,0x60+n*12,strides[n],MCC_HUD_STATICS+n)) goto done;
                if (n<3 || first==plan->count) continue;
                count=plan->uses[first].bytes/strides[n];
                for (j=0;j<count;j++)
                    if (!mcc_hud_plan_block(plan,r,first,j*strides[n]+0x34,
                        n==3 ? 0x6C : 0x88,n==3 ? MCC_HUD_CROSSHAIR_ITEMS : MCC_HUD_OVERLAY_ITEMS)) goto done;
            }
        } else if (group=='grhi') {
            if (!mcc_hud_plan_block(plan,r,i,0x15C,0x88,MCC_HUD_OVERLAY_ITEMS)) goto done;
        }
    }
    /* Check the traversal budget before fingerprinting. Even padding may
     * overlap another consumer's fields, so this pass never writes. */
    for (i=0;i<roots;i++) {
        uint32_t entry_index=plan->uses[i].offset;
        uint32_t *entry=(uint32_t *)(r->tag_index+entry_index*32);
        if (plan->frames[entry_index]!=2)
            plan->frames[entry_index]=(unsigned char)mcc_hud_layout_stock(mcc_hud_layout_visit(r,entry));
    }
    mcc_diagnostic_tag(r,UINT32_MAX);
    mcc_diagnostic_at(r,"HUD ownership planning");
    problem="not enough memory to sort HUD ownership records";
    sorted=malloc(plan->count*sizeof(*sorted));
    if (!sorted) goto done;
    for (i=0;i<plan->count;i++) sorted[i]=plan->uses+i;
    qsort(sorted,plan->count,sizeof(*sorted),mcc_hud_use_compare);
    for (i=0;i<plan->count;i++) {
        struct mcc_hud_use *use=sorted[i];
        int scalar=use->kind==MCC_HUD_CROSSHAIR_ITEMS || use->kind==MCC_HUD_OVERLAY_ITEMS;
        int shared=i && use->source==sorted[i-1]->source && use->kind==sorted[i-1]->kind;
        if (use->parent==UINT32_MAX || (!scalar && !shared)) continue;
        /* Pointer-free items may be interned in allocations outside the HUD
         * tree. Snapshot each consumer. For pointer-bearing aliases retain
         * the largest original extent so validation still sees bad overlaps. */
        use->owned=mcc_runtime_allocate(r,use->bytes);
        if (!use->owned) {
            mcc_hud_plan_context(plan,r,use->parent,use->offset,use->kind);
            problem="HUD block isolation exhausted runtime tag space";goto done;
        }
        memcpy(use->owned,use->source,use->bytes);
    }
    /* All snapshots precede pointer writes: a shared parent can itself hold
     * a child descriptor that another consumer still needs to copy. */
    for (i=roots;i<plan->count;i++) {
        struct mcc_hud_use *use=plan->uses+i;
        uint32_t address=(uint32_t)(uintptr_t)use->owned;
        memcpy(plan->uses[use->parent].owned+use->offset+4,&address,4);
    }
    ok=1;
done:
    if (sorted) free(sorted);
    if (!ok && problem) mcc_diagnostic_fail(r,"%s",problem);
    return ok;
}

static void mcc_hud_plan_report(struct mcc_hud_plan const *plan)
{
    uint32_t i,native=0,authored=0,aliases=0,copies=0,bytes=0;
    for (i=0;i<plan->count;i++) {
        struct mcc_hud_use const *use=plan->uses+i;
        if (use->parent==UINT32_MAX) {
            unsigned frame=plan->frames[use->offset];
            if (frame==2) aliases++;
            else if (use->kind=='unhi' || use->kind=='wphi' || use->kind=='grhi') {
                if (frame) native++;else authored++;
            }
        } else if (use->owned!=use->source) {copies++;bytes+=use->bytes;}
    }
    error(_error_silent,"mcc hud: %lu structural layouts use native frames, %lu retain authored MCC frames; "
        "%lu root aliases, %lu owned block copies (%lu bytes)",
        (unsigned long)native,(unsigned long)authored,(unsigned long)aliases,
        (unsigned long)copies,(unsigned long)bytes);
}
