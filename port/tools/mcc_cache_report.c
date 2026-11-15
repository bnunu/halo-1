/* Standalone audit of a Halo 1 MCC cache. A successful audit is not a claim
 * that the cache has been converted for the Xbox runtime. */
#include "mcc_cache_format.h"
#include <stdio.h>
#include <stdlib.h>

static int read_file(void *context, uint32_t offset, uint32_t size, void *out) {
    FILE *file=(FILE *)context;
    return fseek(file,(long)offset,SEEK_SET)==0 && fread(out,1,size,file)==size;
}

int main(int argc, char **argv) {
    struct mcc_cache_source source;
    struct mcc_cache_report report;
    enum mcc_cache_status status;
    FILE *file;
    long length;
    unsigned i;
    if (argc!=2) { fprintf(stderr,"usage: mcc_cache_report FILE.map\n"); return 2; }
    file=fopen(argv[1],"rb");
    if (!file) { fprintf(stderr,"cannot open map\n"); return 2; }
    if (fseek(file,0,SEEK_END)!=0 || (length=ftell(file))<0) {
        fclose(file); fprintf(stderr,"cannot measure map\n"); return 2;
    }
    source.context=file; source.read=read_file; source.size=(uint64_t)length;
    status=mcc_cache_inspect(&source,&report);
    printf("status=%s\n",mcc_cache_status_string(status));
    printf("problem_tag=%u\nproblem_location=0x%08X\n",report.problem_tag,report.problem_location);
    printf("problem_path=%s\nproblem_detail=%s\n",report.problem_path,report.problem_detail);
    if (report.identity.tag_data_size) {
        printf("name=%s\nbuild=%s\nscenario_type=%u\nflags=0x%04X\n",
            report.identity.name,report.identity.build,report.identity.scenario_type,report.identity.flags);
        printf("tag_base=0x%08X\ntag_bytes=%u\ntag_count=%u\n",report.tag_base,
            report.identity.tag_data_size,report.tag_count);
        printf("models=%u\nmodel_bytes=%u\nbsps=%u\nbsp_materials=%u\n",
            report.model_count,report.model_bytes,report.bsp_count,report.bsp_material_count);
        printf("bitmaps=%u\nbc7_bitmaps=%u\nexternal_bitmaps=%u\n",
            report.bitmap_count,report.bc7_bitmap_count,report.external_bitmap_count);
        printf("sound_permutations=%u\nexternal_sounds=%u\ngeneric_shaders=%u\n",
            report.sound_permutation_count,report.external_sound_count,report.generic_shader_count);
        printf("scripts=%u\nglobals=%u\nscript_node_capacity=%u\nparameterized_scripts=%u\n",
            report.script_count,report.global_count,report.script_node_count,report.parameterized_script_count);
        for (i=0;i<report.bsp_count && i<MCC_CACHE_MAX_BSPS;i++) {
            struct mcc_cache_bsp const *b=&report.bsps[i];
            printf("bsp_%u=offset:0x%08X size:0x%08X address:0x%08X vertices:0x%08X+0x%08X\n",
                i,b->file_offset,b->size,b->address,b->vertex_file_offset,b->vertex_bytes);
        }
    }
    fclose(file);
    return status==MCC_CACHE_OK ? 0 : 1;
}
