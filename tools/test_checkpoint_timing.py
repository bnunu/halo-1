"""Compile the game's checkpoint wait (game_save and game_save_no_timeout)
and check that it counts game ticks, not frames: a checkpoint waits as long
at 30, 60 or 1000 frames a second."""
import argparse
import os
import shutil
import subprocess
import tempfile
from pathlib import Path

from test_death_timing import function

ROOT = Path(__file__).resolve().parent.parent

PRELUDE = r'''
#include <assert.h>
#include <stdio.h>
#include <string.h>
typedef int boolean;
#define TRUE 1
#define FALSE 0
static struct {boolean saving_map,save_map_safely,save_map_timeout,save_map_completed;
    short ticks_until_next_save_check,safe_intervals;long ticks_unable_to_save;} main_globals;
static long main_save_last_tick;
static boolean debug_game_save;
static long tick;
static int paused,safe,saves,checks;
static long game_time_get(void) {return tick;}
static int game_time_get_paused(void) {return paused;}
static boolean coop_scripts_safe_to_save(void) {checks++;return safe;}
static boolean cinematic_port_holds_checkpoints(void) {return FALSE;}
static void players_note_checkpoint(void) {}
static void hud_autosave(boolean on) {saves+=on;}
static void console_printf(boolean error,const char *text) {(void)error;(void)text;}
'''

TESTS = r'''
/* runs the game for some ticks at some frames a tick; how many ticks the
checkpoint took, or -1 if it gave up */
static long run(int frames_per_tick,int timeout,int safe_after,long most_ticks) {
    long start;
    memset(&main_globals,0,sizeof(main_globals));
    saves=checks=0;safe=FALSE;tick=1000;start=tick;
    if (timeout) main_save_map_safe(); else main_save_map_no_timeout();
    while (tick-start<most_ticks) {
        int frame;
        for (frame=0;frame<frames_per_tick;frame++) {
            if (!main_globals.saving_map) return saves ? tick-start : -1;
            main_save_map_private();
        }
        tick++;
        safe=tick-start>=safe_after;
    }
    return saves ? tick-start : -1;
}

int main(void) {
    int rates[]={1,2,4,33};
    int i;
    for (i=0;i<4;i++) {
        long safe_now=run(rates[i],TRUE,0,1000);
        long safe_later=run(rates[i],TRUE,150,1000);
        long too_late=run(rates[i],TRUE,300,1000);
        long no_timeout=run(rates[i],FALSE,600,2000);
        printf("%4d frames a second: saved after %ld, %ld ticks; gave up: %s; no timeout: %ld\n",
            rates[i]*30,safe_now,safe_later,too_late<0?"yes":"no",no_timeout);
        assert(safe_now>=30&&safe_now<=45);
        assert(safe_later>=180&&safe_later<=200);
        assert(too_late<0);
        assert(no_timeout>=630&&no_timeout<=645);
    }
    /* a revert moves the clock back: no ticks counted, no crash */
    memset(&main_globals,0,sizeof(main_globals));
    tick=500;main_save_map_safe();tick=100;main_save_map_private();
    assert(main_globals.saving_map);
    /* paused: nothing counted */
    tick=1000;main_save_map_safe();paused=TRUE;tick+=1000;main_save_map_private();
    assert(main_globals.saving_map&&main_globals.ticks_unable_to_save==0);
    puts("PASS: checkpoints wait in game ticks at any frame rate");
    return 0;
}
'''


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--cc', default='clang')
    args = parser.parse_args()
    main_source = (ROOT / 'source/main/main.c').read_text()
    unit = PRELUDE
    for name in ('main_save_map_safe', 'main_save_map_no_timeout', 'main_save_map_private'):
        unit += function(main_source, name)
    unit += TESTS
    cc = shutil.which(args.cc) or args.cc
    with tempfile.TemporaryDirectory(prefix='halo-checkpoint-timing-') as temp:
        c = Path(temp) / 'checkpoint.c'
        exe = Path(temp) / ('checkpoint.exe' if os.name == 'nt' else 'checkpoint')
        c.write_text(unit)
        flags = ['--target=i686-pc-windows-msvc', '-fuse-ld=lld'] if os.name == 'nt' else []
        subprocess.run([cc, *flags, '-std=c11', '-O2', str(c), '-o', str(exe)], check=True)
        subprocess.run([str(exe)], check=True)


if __name__ == '__main__':
    main()
