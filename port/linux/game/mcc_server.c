#include "cseries.h"
#include "mcc_server.h"
#include "mcc_package.h"
#include "mcc_cache_format.h"
#include "mcc_maps.h"
#include "../src/server_rotation.h"
#include <stdio.h>
#include <string.h>

static int mcc_server_rescan_pending;

wchar_t const *mcc_server_map_title(char const *level)
{
    char canonical[64];
    short index;
    wchar_t const *title;
    if (!mcc_package_name(level,canonical)) return NULL;
    index=mcc_maps_find(canonical);
    if (index<0) return NULL;
    title=mcc_maps_name(index);
    return title && title[0] ? title : NULL;
}

void mcc_server_request_rescan(void)
{
    mcc_server_rescan_pending=1;
}

void mcc_server_update(int pregame)
{
    if (!pregame || !mcc_server_rescan_pending) return;
    mcc_server_rescan_pending=0;
    mcc_maps_rescan();
    mcc_maps_count(FALSE);
}

/* The HTTP command bridge must preserve an MCC package's internal spaces.
 * Validate the local namespace and keep the whole entry one console word. */
int mcc_server_play_line(char const *map,char const *gametype,char *line,int size)
{
    char level[64],canonical[64];
    size_t i,length;
    int written;
    if (strlen(map)<9 || (map[8]!='/' && map[8]!='\\')) return 0;
    for (i=0;i<8;i++) {
        unsigned char c=(unsigned char)map[i];
        if (c>='A' && c<='Z') c+='a'-'A';
        if (c!=(unsigned char)"mcc_maps"[i]) return 0;
    }
    length=strcspn(map,":");
    if (length>=sizeof(level) || size<=0) return -1;
    memcpy(level,map,length);level[length]=0;
    if (!mcc_package_name(level,canonical)) return -1;
    for (i=0;map[i];i++)
        if ((unsigned char)map[i]<32 || (unsigned char)map[i]>=127 || map[i]=='"') return -1;
    for (i=0;gametype[i];i++)
        if ((unsigned char)gametype[i]<=32 || (unsigned char)gametype[i]>=127 || gametype[i]=='"') return -1;
    written=snprintf(line,(size_t)size,"sv_map \"%s\" %s",map,gametype);
    return written>=0 && written<size ? 1 : -1;
}

void mcc_server_resolve(struct server_entry *entry)
{
    char canonical[64],path[128];
    unsigned char header[MCC_CACHE_HEADER_BYTES];
    struct mcc_cache_identity identity;
    enum mcc_cache_status status;
    FILE *file;
    long size;
    if (!mcc_package_name(entry->map,canonical) || !mcc_package_map_path(canonical,path)) {
        snprintf(entry->rejected,sizeof(entry->rejected),"Invalid MCC map path: %s",entry->map);
        return;
    }
    strcpy(entry->level,canonical);
    strcpy(entry->map_name,strrchr(canonical,'\\')+1);
    file=fopen(path,"rb");
    if (!file) {
        entry->map_missing=1;
        snprintf(entry->rejected,sizeof(entry->rejected),"Cannot read MCC map %s",canonical);
        return;
    }
    status=MCC_CACHE_READ_FAILED;
    if (!fseek(file,0,SEEK_END) && (size=ftell(file))>=0 && !fseek(file,0,SEEK_SET) &&
        fread(header,1,sizeof(header),file)==sizeof(header))
        status=mcc_cache_identify(header,sizeof(header),(uint64_t)size,&identity);
    fclose(file);
    if (status!=MCC_CACHE_OK)
        snprintf(entry->rejected,sizeof(entry->rejected),"Invalid MCC map %s: %s",canonical,mcc_cache_status_string(status));
    else if ((identity.scenario_type==0)!=(!strcmp(entry->gametype,"coop")))
        snprintf(entry->rejected,sizeof(entry->rejected),"%s requires %s",canonical,
            identity.scenario_type==0 ? "the coop gametype" : "a multiplayer gametype");
}

int mcc_server_maps_each(void (*each)(void *, const char *, const char *, const char *),void *context)
{
    short row,count,index;
    /* Read the cached catalog; an HTTP refresh must not rescan mid-game. */
    count=mcc_maps_count(FALSE);
    for (row=0;row<count;row++) {
        char const *level;
        index=mcc_maps_index(row,FALSE);
        level=mcc_maps_level_name(index);
        if (level) each(context,level+9,level,"mcc");
    }
    return count;
}
