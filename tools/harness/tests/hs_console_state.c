
#include <stdint.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include "zlib_prefixed.h"
typedef unsigned char byte,boolean;
typedef unsigned short word;
typedef float real;
#define TAG_STRING_LENGTH 31
#define NONE (-1)
#define TRUE 1
#define FALSE 0
#define FLAG(b) (1u<<(b))
#define TEST_FLAG(v,b) (((v)&(1u<<(b)))!=0)
#define SET_FLAG(v,b,on) ((on)?((v)|=(1u<<(b))):((v)&=~(1u<<(b))))
#define NUMBEROF(a) (sizeof(a)/sizeof((a)[0]))
#define csmemset memset
#define csstrncpy strncpy
#define csstrcmp strcmp
#define CHECK(c) do {if(!(c)){fprintf(stderr,"check line %d: %s\n",__LINE__,#c);return 1;}}while(0)
static char assertion_text[512],temporary[512];
static char *csprintf(char *out,char const *format,...) {va_list args;va_start(args,format);vsprintf(out,format,args);va_end(args);return out;}
static void assertion(char const *file,int line,char const *text) {
    snprintf(assertion_text,sizeof(assertion_text),"%s: %s",file,text);
    fprintf(stderr,"unexpected assertion %d %s\n",line,assertion_text);exit(3);
}
#define match_assert(f,l,c) do {if(!(c))assertion(f,l,#c);}while(0)
#define match_vassert(f,l,c,t) do {if(!(c))assertion(f,l,t);}while(0)
#define match_hs_assert(f,l,i,c,t) match_vassert(f,l,c,t)
#include "types.inc"
static void platform_log(char const *format,...) {(void)format;abort();}
#include "data.inc"
static byte image[4*1024*1024],saved[sizeof(image)];
static byte syntax_memory[sizeof(struct data_array)+32767*sizeof(struct hs_syntax_node)+16];
static struct data_array *hs_thread_data,*hs_syntax_data,*hs_global_data;
static struct hs_runtime_globals hs_runtime_globals;
static int loaded,completed,standalone;
static char last_text[1024];
static long now;
static unsigned long used=160000,capacity=sizeof(image);
#define hs_thread_get(i) ((struct hs_thread_datum *)datum_get(hs_thread_data,(i)))
#define hs_syntax_get(i) ((struct hs_syntax_node *)datum_get(hs_syntax_data,(i)))
static void mcc_campaign_reset_model_lighting(void) {}
static int mcc_cache_tags_loaded(void) {return loaded;}
static void *game_state_port_cpu_region(unsigned long *u,unsigned long *c) {*u=used;*c=capacity;return image;}
#include "definitions.inc"
#include "storage.inc"
#include "valid.inc"
struct scenario {struct {long count;struct hs_script *address;} hs_scripts;
    struct {long count;void *address;} hs_globals,hs_references;
    struct {long size;void *address;} hs_string_constants;};
static struct hs_script script;
static struct scenario scenario;
#define TAG_BLOCK_GET_ELEMENT(b,i,t) ((t *)(b)->address+(i))
static struct scenario *global_scenario_get(void) {return &scenario;}
struct hs_function_definition {short return_type;void (*evaluate)(short,long,boolean);
    char const *name;boolean (*parse)(short,long);short parameter_count,parameter_types[1];};
static void test_evaluate(short,long,boolean);
static boolean hs_macro_function_parse(short f,long e) {(void)f;(void)e;return TRUE;}
static struct hs_function_definition test_function={_hs_type_void,test_evaluate,"test",hs_macro_function_parse,1,{_hs_type_string}};
static long hs_function_table_count=1;
static struct hs_function_definition *hs_function_get(short i) {if(i!=0&&!(loaded&&i==0x7000))abort();return &test_function;}
static struct hs_function_definition *mcc_script_function(short i) {return loaded&&i==0x7000 ? &test_function : NULL;}
static int hs_scenario_script_disabled(short i) {(void)i;return 0;}
static long game_time_get(void) {return now;}
static int game_in_progress(void) {return 1;}
static int hs_type_valid(short type) {return type>=_hs_type_void && type<NUMBER_OF_HS_TYPES;}
typedef void (*hs_typecasting_procedure)(void);
static void test_cast_available(void) {}
#include "casts.inc"
static long hs_cast(long i,short a,short b,long value) {(void)i;(void)a;(void)b;return value;}
static short hs_global_get_type(short i) {return i>=0 && i<1024 ? _hs_type_string : _hs_unparsed;}
static long hs_global_evaluate(short i) {(void)i;abort();return 0;}
static int mcc_hs_parameter_evaluate(long i,struct hs_syntax_node *e,long *out) {(void)i;(void)e;(void)out;return 0;}
static int mcc_hs_if_tail(short i,long t,boolean initialize) {(void)i;(void)t;(void)initialize;return 0;}
static void mcc_hs_release_thread(long i) {(void)i;}
static void hs_script_evaluate(short i,long t,boolean initialize) {(void)i;(void)t;(void)initialize;abort();}
static void hs_syntax_error(long i) {(void)i;abort();}
static void hs_stack_overflow(long i) {(void)i;abort();}
static void object_list_gc(void) {}
static void error(int severity,char const *message,...) {(void)severity;(void)message;}
#define _error_silent 0
static void hs_evaluate(long,long,long *);
static void hs_thread_main(long);
static long global_scenario_index=0;
static void *game_state_malloc(char const *name,char const *type,long size) {
    void *p=image+used;(void)name;(void)type;used+=(unsigned long)size;
    if(used>capacity)abort();return p;
}
static boolean hs_scenario_downloaded,hs_scenario_campaign;
static boolean hs_function_allowed_in_maps[1]={FALSE};
static char const *const hs_functions_denied_to_downloaded_maps[]={NULL};
static char const *const hs_globals_denied_to_downloaded_maps[]={NULL};
static boolean hs_name_listed(char const *name,char const *const *list) {(void)name;(void)list;return FALSE;}
static struct {short minimum,maximum;} hs_special_form_argument_counts[1];
#include "mcc_calls.inc"
static int mcc_parameter_set_valid(struct hs_syntax_node const *node,struct hs_syntax_node const *value) {(void)node;(void)value;return -1;}
static boolean hs_external_global_settable_by_maps(short i) {(void)i;return FALSE;}
static struct {char const *name;} unused_global;
#define hs_global_external_get(i) (&unused_global)
#define DATUM_INDEX_TO_ABSOLUTE_INDEX(i) ((unsigned short)(i))
/* The compiler and checker call the same variable bit by these names. */
#define _hs_syntax_node_variable_bit _hs_syntax_node_global_bit
#define _hs_syntax_node_permanent_bit 3
#include "policy.inc"
#include "console.inc"
#include "runtime.inc"

static struct hs_compile_globals hs_compile_globals;
static int parse_failure;
static int source_frees;
static void hs_compile_finish(void) {abort();}
static void tag_block_resize(void *block,long count) {(void)block;(void)count;abort();}
static void tag_data_resize(void *data,long bytes) {(void)data;(void)bytes;abort();}
#define match_free(file,line,pointer) ((void)(pointer),source_frees++)
#define HS_MAXIMUM_DYNAMIC_SOURCE_DATA_BYTES 1024
static void skip_whitespace(char **cursor) {while(**cursor==' ')(*cursor)++;}
static boolean hs_parse_string(long expression_index);
static long hs_tokenize(struct hs_tokenizer *tokenizer)
{
    long expression=datum_new(hs_syntax_data);
    struct hs_syntax_node *node=hs_syntax_get(expression);
    node->type=node->constant_type=_hs_type_string;node->flags=1;node->next_node_index=NONE;
    node->source_offset=(long)(tokenizer->cursor-hs_compile_globals.compiled_source);
    return expression;
}
static boolean hs_parse(long expression_index,short type)
{
    struct hs_syntax_node *root=hs_syntax_get(expression_index);
    struct hs_syntax_node *predicate=hs_syntax_get(root->data);
    root->type=type;root->index=0;
    if(parse_failure) {hs_compile_globals.error="test parse refusal";return FALSE;}
    return hs_parse_string(predicate->next_node_index);
}
#include "compile.inc"
/* One test function models a yielding call; the interpreter owns its real
 * frames, identifiers, deletion and GC. No parsing/GC algorithm is copied. */
static void test_evaluate(short function_index,long thread_index,boolean initialize) {
    (void)function_index;
    if(initialize&&!standalone)hs_thread_get(thread_index)->sleep_until=60;
    else {
        struct hs_syntax_node *root=hs_syntax_get(hs_thread_get(thread_index)->stack->expression_index);
        struct hs_syntax_node *argument=hs_syntax_get(hs_syntax_get(root->data)->next_node_index);
        strcpy(last_text,(char const *)argument->data);
        completed++;hs_return(thread_index,0);
    }
}

static long expression_new(char *text,int permanent)
{
    long root=datum_new(hs_syntax_data),predicate=datum_new(hs_syntax_data),argument=datum_new(hs_syntax_data);
    struct hs_syntax_node *node;
    if(root==NONE || predicate==NONE || argument==NONE)abort();
    node=hs_syntax_get(root);node->type=_hs_type_void;node->index=0;
    node->flags=permanent?8:0;node->next_node_index=NONE;node->data=predicate;
    node=hs_syntax_get(predicate);node->type=_hs_function_name;node->flags=permanent?9:1;
    node->next_node_index=argument;node->index=0;
    node=hs_syntax_get(argument);node->type=node->constant_type=_hs_type_string;
    node->flags=permanent?9:1;node->next_node_index=NONE;node->data=(long)text;
    return root;
}

static void initialize(int use_mcc,int syntax_count)
{
    struct data_array *native=(void *)(image+1024);
    memset(image,0,sizeof(image));
    data_initialize(native,"hs thread",256,sizeof(struct hs_thread_datum));data_make_valid(native);
    hs_thread_data=native;
    hs_global_data=(void *)(image+141000);
    data_initialize(hs_global_data,"hs globals",1024,sizeof(struct hs_global_datum));data_make_valid(hs_global_data);
    hs_console_state_initialize();
    if(use_mcc) {
        if(!mcc_script_threads_prepare())abort();
        loaded=1;mcc_hs_threads_activate();data_make_valid(hs_thread_data);
    }
    hs_syntax_data=(void *)syntax_memory;
    data_initialize(hs_syntax_data,"script node",(short)syntax_count,sizeof(struct hs_syntax_node));data_make_valid(hs_syntax_data);
    hs_runtime_globals.initialized=1;hs_runtime_globals.executing_thread_index=NONE;
}

int main(int argc,char **argv)
{
    char *text,*second;
    char const *which;
    long root,thread_index,other,i;
    struct hs_console_state *snapshot;
    struct hs_thread_datum *thread;
    int use_mcc,permanent,collect,reuse;
    CHECK(argc==3&&sizeof(long)==4&&sizeof(void *)==4);
    which=argv[2];use_mcc=!strcmp(argv[1],"mcc");
    initialize(use_mcc,!strcmp(which,"capacity_boundary") ? (use_mcc?32767:19001) :
        !strcmp(which,"long_sleep_reuse") ? 19001 : 16);
    CHECK(sizeof(*hs_console_state)==918568);
    if(!strncmp(which,"compile_",8)) {
        char const *message=NULL,*error_source=NULL;
        hs_compile_initialize(FALSE);
        root=hs_compile_expression(15,"original string",&message,&error_source);CHECK(root!=NONE);
        text=hs_compile_globals.compiled_source;
        hs_runtime_evaluate(root);
        if(!strcmp(which,"compile_dispose")) {
            hs_compile_dispose();CHECK(!hs_compile_globals.initialized&&!source_frees);
            hs_compile_initialize(FALSE);
            CHECK(!hs_compile_globals.compiled_source&&!hs_compile_globals.compiled_source_size);
        }
        if(!strcmp(which,"compile_failure"))parse_failure=1;
        other=hs_compile_expression(7,"changed",&message,&error_source);
        if(parse_failure)CHECK(other==NONE&&message);
        else CHECK(other!=NONE&&hs_compile_globals.compiled_source!=text);
        CHECK(!strcmp(text,"original string"));
        hs_compile_dispose();CHECK(!hs_compile_globals.initialized&&!source_frees);
        now=60;hs_runtime_update();CHECK(!strcmp(last_text,"original string"));
        return 0;
    }
    text=hs_console_source_acquire(20);CHECK(text!=NULL);strcpy(text,"original string");
    permanent=!strcmp(which,"scenario_gc");
    standalone=!strcmp(which,"standalone_gc");
    root=expression_new(text,permanent);hs_console_source_finish(text,root);
    hs_runtime_evaluate(root);
    thread_index=data_next_index(hs_thread_data,NONE);
    if(!standalone)CHECK(thread_index!=NONE);
    if(permanent) {
        thread=hs_thread_get(thread_index);thread->type=_hs_thread_type_script;thread->script_index=0;
        scenario.hs_scripts.count=1;scenario.hs_scripts.address=&script;
        script.script_type=_hs_script_startup;script.root_expression_index=root;
    }
    if(!strcmp(which,"valid_casts")) {
        struct hs_syntax_node literal=*hs_syntax_get(hs_syntax_get(hs_syntax_get(root)->data)->next_node_index);
        literal.constant_type=_hs_type_real;literal.type=_hs_type_boolean;
        CHECK(hs_console_syntax_node_valid(&literal));
        literal.constant_type=_hs_type_string;literal.type=_hs_type_void;
        CHECK(hs_console_syntax_node_valid(&literal));
        literal.flags=5;literal.data=0;literal.type=_hs_type_string;
        CHECK(hs_console_syntax_node_valid(&literal));
        literal.flags=1;literal.constant_type=_hs_type_short_integer;literal.type=_hs_type_long_integer;
        CHECK(hs_console_syntax_node_valid(&literal)==use_mcc);
        test_function.return_type=_hs_passthrough;hs_syntax_get(root)->type=_hs_type_string;
        CHECK(hs_console_state_capture());
        hs_syntax_get(root)->flags=2;
        scenario.hs_scripts.count=1;scenario.hs_scripts.address=&script;
        script.return_type=_hs_type_string;script.script_type=_hs_script_static;
        CHECK(hs_console_state_capture());
        script.script_type=_hs_script_stub;CHECK(hs_console_state_capture());return 0;
    }
    if(!strcmp(which,"concurrent_text")) {
        second=hs_console_source_acquire(8);CHECK(second&&second!=text);strcpy(second,"changed");
        now=60;hs_runtime_update();CHECK(!strcmp(last_text,"original string"));return 0;
    }
    if(!strcmp(which,"long_sleep_reuse")) {
        char const *message=NULL,*error_source=NULL;
        standalone=1;
        for(i=0;i<500;i++) {
            other=hs_compile_expression(7,"changed",&message,&error_source);CHECK(other!=NONE);
            CHECK(hs_compile_globals.compiled_source!=text);
            hs_runtime_evaluate(other);
        }
        standalone=0;now=60;hs_runtime_update();CHECK(completed==501);
        CHECK(!strcmp(last_text,"original string"));return 0;
    }
    if(!strcmp(which,"false_pin")) {
        thread=hs_thread_get(thread_index);hs_console_state->roots[0]=NONE;
        thread->result=(long)(text+100);
        second=hs_console_source_acquire(8);CHECK(second&&second!=text);
        thread->result=0;second=hs_console_source_acquire(8);CHECK(second==text);return 0;
    }
    if(!strcmp(which,"global_text") || !strcmp(which,"pool_full")) {
        now=60;hs_runtime_update();now=80;hs_runtime_update();
        other=datum_new(hs_global_data);CHECK(other!=NONE);
        ((struct hs_global_datum *)datum_get(hs_global_data,other))->value=(long)(text+3);
        second=hs_console_source_acquire(8);CHECK(second&&second!=text);strcpy(second,"changed");
        CHECK(!strcmp(text,"original string"));
        if(!strcmp(which,"pool_full")) {
            for(i=1;i<HS_CONSOLE_SOURCE_COUNT;i++) {
                other=datum_new(hs_global_data);CHECK(other!=NONE);
                ((struct hs_global_datum *)datum_get(hs_global_data,other))->value=(long)hs_console_state->sources[i];
            }
            CHECK(hs_console_source_acquire(8)==NULL);
        }
        data_delete_all(hs_global_data);
        second=hs_console_source_acquire(8);CHECK(second==text);
        return 0;
    }
    if(!strcmp(which,"unaligned_value") || !strcmp(which,"discarded_frame")) {
        long value=(long)(text+2);
        thread=hs_thread_get(thread_index);
        memcpy(thread->stack->data+1,&value,sizeof(value));thread->stack->size=5;
        hs_console_state->roots[0]=NONE;
        second=hs_console_source_acquire(8);CHECK(second&&second!=text);
        if(!strcmp(which,"discarded_frame")) {
            thread->stack->size=0;
            second=hs_console_source_acquire(8);CHECK(second==text);
        }
        return 0;
    }
    if(!strcmp(which,"source_bounds")) {
        CHECK(hs_console_source_acquire(-1)==NULL);
        CHECK(hs_console_source_acquire(1024)==NULL);
        CHECK(hs_console_source_acquire(1023)!=NULL);
        return 0;
    }
    if(!strcmp(which,"mcc_console_call")) {
        hs_syntax_get(root)->function_index=0x7000;
        CHECK(hs_console_state_capture()==use_mcc);
        CHECK(!mcc_script_call_valid(hs_syntax_get(root),hs_syntax_data));
        CHECK(mcc_script_call_arguments_valid(hs_syntax_get(root),hs_syntax_data)==use_mcc);
        return 0;
    }
    if(!strcmp(which,"inactive_permanent")) {
        other=datum_new(hs_syntax_data);
        hs_syntax_get(other)->flags=8;hs_syntax_get(other)->data=NONE;
        hs_syntax_get(other)->next_node_index=other;
        CHECK(hs_console_state_capture());memcpy(saved,image,sizeof(image));
        CHECK(hs_console_state_validate(saved,image,sizeof(image)));return 0;
    }
    if(!strcmp(which,"capacity_boundary")) {
        byte *canary=syntax_memory+sizeof(struct data_array)+hs_syntax_data->maximum_count*sizeof(struct hs_syntax_node);
        memset(canary,0xA5,16);memset(image+used,0x5A,16);
        while(datum_new(hs_syntax_data)!=NONE) {}
        CHECK(hs_console_state_capture());memcpy(saved,image,sizeof(image));
        CHECK(hs_console_state_validate(saved,image,sizeof(image)));
        hs_console_state_restore();
        for(i=0;i<16;i++)CHECK(canary[i]==0xA5&&image[used+i]==0x5A);
        return 0;
    }
    if(use_mcc)CHECK(mcc_script_threads_capture());
    CHECK(hs_console_state_capture());
    memcpy(saved,image,sizeof(image));
    snapshot=(void *)(saved+((byte *)hs_console_state-image));
    if(!strncmp(which,"invalid_",8)) {
        struct data_array *saved_threads=(void *)(saved+((byte *)hs_thread_data-image));
        struct hs_thread_datum *saved_thread=(void *)(saved+((byte *)hs_thread_get(thread_index)-image));
        struct hs_stack_frame *frame=(void *)(saved+((byte *)saved_thread->stack-image));
        if(!strcmp(which,"invalid_signature"))snapshot->signature=0;
        else if(!strcmp(which,"invalid_name"))memset(snapshot->syntax.name,'x',sizeof(snapshot->syntax.name));
        else if(!strcmp(which,"invalid_capacity"))snapshot->syntax.maximum_count++;
        else if(!strcmp(which,"invalid_count"))snapshot->syntax.count=17;
        else if(!strcmp(which,"invalid_actual"))snapshot->syntax.actual_count++;
        else if(!strcmp(which,"invalid_data"))snapshot->syntax.data=(void *)1;
        else if(!strcmp(which,"invalid_salt"))snapshot->nodes[(short)root].datum_header++;
        else if(!strcmp(which,"invalid_cycle"))snapshot->nodes[(short)root].next_node_index=root;
        else if(!strcmp(which,"invalid_string"))snapshot->nodes[2].data=(long)image;
        else if(!strcmp(which,"invalid_string_end"))memset(snapshot->sources[0],'x',1024);
        else if(!strcmp(which,"invalid_source_root"))snapshot->roots[0]=NONE;
        else if(!strcmp(which,"invalid_source_owner"))snapshot->nodes[2].data=(long)hs_console_state->sources[1];
        else if(!strcmp(which,"invalid_call"))snapshot->nodes[(short)root].function_index=1;
        else if(!strcmp(which,"invalid_stack"))saved_thread->stack=(void *)1;
        else if(!strcmp(which,"invalid_frame_size"))frame->size=32767;
        else if(!strcmp(which,"invalid_result")) {
            struct hs_stack_frame *previous=(void *)(saved+((byte *)frame->previous-image));previous->result=(void *)1;
        }
        else if(!strcmp(which,"invalid_previous"))frame->previous=(void *)((byte *)frame->previous+1);
        else if(!strcmp(which,"invalid_thread_array"))saved_threads->data=(void *)1;
        else if(!strcmp(which,"invalid_permanent"))snapshot->nodes[(short)root].flags|=8;
        else if(!strcmp(which,"invalid_literal_cast"))snapshot->nodes[2].constant_type=_hs_type_real;
        else if(!strcmp(which,"invalid_global_cast")) {
            snapshot->nodes[2].flags=5;snapshot->nodes[2].data=0;snapshot->nodes[2].type=_hs_type_real;
            test_function.parameter_types[0]=_hs_type_real;
        }
        else if(!strcmp(which,"invalid_return_cast"))snapshot->nodes[(short)root].type=_hs_type_string;
        else if(!strcmp(which,"invalid_unparsed"))snapshot->nodes[2].type=_hs_unparsed;
        else if(!strcmp(which,"invalid_predicate"))snapshot->nodes[1].type=_hs_type_void;
        else if(!strcmp(which,"invalid_name_argument")) {
            test_function.parse=NULL;hs_special_form_argument_counts[0].maximum=NONE;
            snapshot->nodes[2].type=_hs_function_name;
        }
        else if(!strcmp(which,"invalid_primitive_flags"))snapshot->nodes[2].flags|=2;
        else if(!strcmp(which,"invalid_global_index")) {
            snapshot->nodes[2].flags=5;snapshot->nodes[2].data=1024;
        }
        else if(!strncmp(which,"invalid_script_",15)) {
            snapshot->nodes[(short)root].flags=2;
            scenario.hs_scripts.count=1;scenario.hs_scripts.address=&script;
            script.script_type=_hs_script_static;script.return_type=_hs_type_void;
            if(!strcmp(which,"invalid_script_index"))snapshot->nodes[(short)root].script_index=1;
            else if(!strcmp(which,"invalid_script_kind"))script.script_type=_hs_script_startup;
            else if(!strcmp(which,"invalid_script_cast")) {
                script.return_type=_hs_type_real;snapshot->nodes[(short)root].type=_hs_type_string;
            } else CHECK(0);
        }
        else CHECK(0);
        CHECK(!hs_console_state_validate(saved,image,sizeof(image)));
        CHECK(!memcmp(text,"original string",16));return 0;
    }
    now=60;hs_runtime_update();CHECK(completed==1);
    collect=strcmp(which,"console_no_gc")!=0;
    reuse=!strcmp(which,"console_reuse") || !strcmp(which,"repeat_restore") || !strcmp(which,"reload");
    if(collect) {now=80;hs_runtime_update();}
    CHECK((datum_try_and_get(hs_syntax_data,root)!=NULL)==(!collect||permanent));
    if(reuse) {
        second=hs_console_source_acquire(8);CHECK(second==text);strcpy(second,"changed");
        other=expression_new(second,0);CHECK((short)other==(short)root&&other!=root);
        hs_console_source_finish(second,other);
    }
    if(!strcmp(which,"reload")) {
        hs_console_state_reset();data_delete_all(hs_syntax_data);data_delete_all(hs_thread_data);
    }
    CHECK(hs_console_state_validate(saved,image,sizeof(image)));
    CHECK(!hs_console_state_validate(NULL,image,sizeof(image)));
    CHECK(!hs_console_state_validate(saved,NULL,sizeof(image)));
    CHECK(!hs_console_state_validate(saved,image,used-1));
    if(use_mcc)CHECK(mcc_script_threads_validate(saved,image,used,capacity));
    memcpy(image,saved,sizeof(image));hs_console_state_restore();
    if(use_mcc)CHECK(mcc_script_threads_restore(image,used,capacity));
    now=60;hs_runtime_update();CHECK(completed==(standalone?1:2));
    CHECK(!strcmp(last_text,"original string"));
    if(!strcmp(which,"repeat_restore")) {
        now=80;hs_runtime_update();
        CHECK(hs_console_state_validate(saved,image,sizeof(image)));
        memcpy(image,saved,sizeof(image));hs_console_state_restore();
        if(use_mcc)CHECK(mcc_script_threads_restore(image,used,capacity));
        now=60;hs_runtime_update();CHECK(completed==3);
    }
    if(use_mcc)mcc_script_threads_dispose();
    return 0;
}
