#include <stddef.h>
#include "cseries.h"
#include "mcc_workshop.h"
#include "mcc_maps.h"
#include "mcc_package.h"
#include "../src/mcc_manifest.h"
#include "../src/posix.h"
#include "../src/server_json.h"
#include "../src/steam_workshop.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int mcc_workshop_digits(char const *text, unsigned length)
{
    unsigned i;
    if (!length || length>20) return 0;
    for (i=0;i<length;i++) if (text[i]<'0' || text[i]>'9') return 0;
    return 1;
}

/* Printable ASCII only: the title is shown in an error box and sent as bytes. */
static void mcc_workshop_title_copy(char const *text, unsigned length, char title[MCC_WORKSHOP_TITLE_SIZE])
{
    unsigned i,used=0;
    for (i=0;i<length && used<MCC_WORKSHOP_TITLE_SIZE-1;i++) {
        unsigned char c=(unsigned char)text[i];
        if (c>=32 && c<127) title[used++]=(char)c;
    }
    while (used && title[used-1]==' ') used--;
    title[used]=0;
}

int mcc_workshop_parse_sidecar(char const *text, struct mcc_workshop_item *item)
{
    unsigned length=0,title_start,title_length=0;
    memset(item,0,sizeof(*item));
    while (text[length] && text[length]!='\r' && text[length]!='\n') length++;
    if (!mcc_workshop_digits(text,length)) return 0;
    memcpy(item->id,text,length);
    title_start=length;
    while (text[title_start]=='\r' || text[title_start]=='\n') title_start++;
    while (text[title_start+title_length] && text[title_start+title_length]!='\r' &&
        text[title_start+title_length]!='\n') title_length++;
    mcc_workshop_title_copy(text+title_start,title_length,item->title);
    return 1;
}

int mcc_workshop_parse_modinfo(char const *text, int length, struct mcc_workshop_item *item)
{
    struct server_json_document document;
    char engine[32],title[MCC_WORKSHOP_TITLE_SIZE*2];
    int token,id_length,valid=0;
    memset(item,0,sizeof(*item));
    /* (a byte order mark, as Windows editors leave one) */
    if (text && length>=3 && (unsigned char)text[0]==0xEF && (unsigned char)text[1]==0xBB &&
        (unsigned char)text[2]==0xBF) {
        text+=3;
        length-=3;
    }
    if (!text || length<=0 || !server_json_parse(&document,text,length,16)) return 0;
    if (document.count<1 || document.tokens[0].type!=_server_json_object ||
        !server_json_get_string(&document,server_json_member(&document,0,"Engine"),engine,sizeof(engine)) ||
        strcmp(engine,"Halo1")) goto done;
    token=server_json_member(&document,server_json_member(&document,
        server_json_member(&document,0,"ModIdentifier"),"HostedModIds"),"SteamWorkshopId");
    if (token<0) goto done;
    /* a number in MCC's own (its text kept: an id may pass a double's
     * precision), a string in some hand-made ones */
    if (!server_json_get_string(&document,token,item->id,sizeof(item->id))) {
        id_length=document.tokens[token].end-document.tokens[token].start;
        if (document.tokens[token].type!=_server_json_primitive || id_length<=0 ||
            id_length>=(int)sizeof(item->id)) goto done;
        memcpy(item->id,document.text+document.tokens[token].start,(size_t)id_length);
        item->id[id_length]=0;
    }
    /* (an unpublished package's is 0 or empty) */
    id_length=(int)strlen(item->id);
    if (!mcc_workshop_digits(item->id,(unsigned)id_length) || strspn(item->id,"0")==(size_t)id_length) goto done;
    if (server_json_get_string(&document,server_json_member(&document,
        server_json_member(&document,0,"Title"),"Neutral"),title,sizeof(title)))
        mcc_workshop_title_copy(title,(unsigned)strlen(title),item->title);
    valid=1;
done:
    server_json_document_free(&document);
    if (!valid) memset(item,0,sizeof(*item));
    return valid;
}

/* the package's ModInfo.json, where its folder is resolved (beside the
 * level, or a Steam item's above its maps folder) */
static int mcc_workshop_modinfo_item(char const *name, struct mcc_workshop_item *item)
{
    char root[1024],*text;
    int length,valid=0;
    if (!mcc_manifest_package_root(name,root) || !(text=malloc(STEAM_WORKSHOP_MAXIMUM_JSON))) return 0;
    length=posix_mcc_preview_read(root,"ModInfo.json",text,STEAM_WORKSHOP_MAXIMUM_JSON);
    if (length>0) valid=mcc_workshop_parse_modinfo(text,length,item);
    free(text);
    return valid;
}

int mcc_workshop_package_folder(char const *name, char folder[64])
{
    char canonical[64],*first,*last;
    if (!mcc_package_name(name,canonical)) return 0;
    first=strchr(canonical,'\\')+1;
    last=strrchr(canonical,'\\');
    if (last<=first) return 0;
    *last=0;
    strcpy(folder,first);
    return 1;
}

int mcc_workshop_local_item(char const *name, struct mcc_workshop_item *item)
{
    char folder[64],path[160],text[128];
    FILE *file;
    size_t length;
    memset(item,0,sizeof(*item));
    if (!mcc_workshop_package_folder(name,folder)) return 0;
    snprintf(path,sizeof(path),"%s%s\\%s",MCC_MAPS_DIRECTORY,folder,MCC_WORKSHOP_SIDECAR);
    file=fopen(path,"rb");
    if (file) {
        length=fread(text,1,sizeof(text)-1,file);
        fclose(file);
        text[length]=0;
        if (mcc_workshop_parse_sidecar(text,item)) return 1;
    }
    if (mcc_workshop_digits(folder,(unsigned)strlen(folder))) {
        char title[MCC_WORKSHOP_TITLE_SIZE*2];
        strcpy(item->id,folder);
        /* the Steam item's own title when this machine has the item */
        if (mcc_maps_workshop_title(folder,title,sizeof(title)))
            mcc_workshop_title_copy(title,(unsigned)strlen(title),item->title);
        if (!item->title[0]) mcc_workshop_title_from_level(name,item->title);
        return 1;
    }
    /* a package copied in under a name of its own, with neither: the id MCC's
     * tools wrote into its ModInfo.json */
    if (mcc_workshop_modinfo_item(name,item)) {
        if (!item->title[0]) mcc_workshop_title_from_level(name,item->title);
        return 1;
    }
    memset(item,0,sizeof(*item));
    return 0;
}

void mcc_workshop_title_from_level(char const *name, char title[MCC_WORKSHOP_TITLE_SIZE])
{
    char const *leaf=strrchr(name,'\\');
    unsigned i,start_word=1;
    leaf=leaf ? leaf+1 : name;
    for (i=0;leaf[i] && i<MCC_WORKSHOP_TITLE_SIZE-1;i++) {
        unsigned char c=(unsigned char)leaf[i];
        if (c=='_') c=' ';
        else if (start_word && c>='a' && c<='z') c=(unsigned char)(c-'a'+'A');
        title[i]=(c>=32 && c<127) ? (char)c : '?';
        start_word=c==' ';
    }
    title[i]=0;
}

/* Layout: 'W' 'S' 1, the id, NUL, the title, NUL, zeros. */
void mcc_workshop_wire_pack(char name[128], struct mcc_workshop_item const *item)
{
    char *tail=name+MCC_WORKSHOP_WIRE_OFFSET;
    unsigned id_length=(unsigned)strlen(item->id),title_length=(unsigned)strlen(item->title);
    memset(tail,0,MCC_WORKSHOP_WIRE_SIZE);
    if (!mcc_workshop_digits(item->id,id_length)) return;
    if (title_length>MCC_WORKSHOP_WIRE_SIZE-3-id_length-2) title_length=MCC_WORKSHOP_WIRE_SIZE-3-id_length-2;
    tail[0]='W';
    tail[1]='S';
    tail[2]=MCC_WORKSHOP_WIRE_VERSION;
    memcpy(tail+3,item->id,id_length);
    memcpy(tail+4+id_length,item->title,title_length);
}

int mcc_workshop_wire_unpack(char const name[128], struct mcc_workshop_item *item)
{
    char const *tail=name+MCC_WORKSHOP_WIRE_OFFSET,*end;
    unsigned id_length;
    memset(item,0,sizeof(*item));
    if (tail[0]!='W' || tail[1]!='S' || tail[2]!=MCC_WORKSHOP_WIRE_VERSION) return 0;
    end=memchr(tail+3,0,MCC_WORKSHOP_WIRE_SIZE-3);
    if (!end) return 0;
    id_length=(unsigned)(end-(tail+3));
    if (!mcc_workshop_digits(tail+3,id_length)) return 0;
    memcpy(item->id,tail+3,id_length);
    if (!memchr(end+1,0,(size_t)(tail+MCC_WORKSHOP_WIRE_SIZE-(end+1)))) {
        memset(item,0,sizeof(*item));
        return 0;
    }
    mcc_workshop_title_copy(end+1,(unsigned)strlen(end+1),item->title);
    return 1;
}

void mcc_workshop_refusal_text(enum mcc_workshop_refusal kind, struct mcc_workshop_item const *item,
    char const *level, char *text, unsigned size)
{
    char title[MCC_WORKSHOP_TITLE_SIZE];
    if (item->title[0]) strcpy(title,item->title);
    else mcc_workshop_title_from_level(level,title);
    switch (kind) {
    case MCC_WORKSHOP_MISSING:
        snprintf(text,size,"No identified local copy of %s (Steam Workshop item %s). "
            "Download the host's version and copy its map and required resources into mcc_maps.",title,item->id);
        break;
    case MCC_WORKSHOP_DIFFERS:
        snprintf(text,size,"Your local package for %s (Steam Workshop item %s) does not match the host. "
            "Check versions and copy the matching map and required resources into mcc_maps.",
            title,item->id);
        break;
    default:
        snprintf(text,size,"No local MCC map %s.map and required resources match the host's SHA-256 fingerprint. "
            "Install the same complete package; its folder name may differ.",level);
        break;
    }
}
