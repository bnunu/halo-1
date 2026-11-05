/* Both sides of the cancellation boundary use their actual structures/functions. */
#include "harness.h"
#include "types.inc"
static struct sdl_stream stream;
static struct sdl_stream *streams=&stream;
static struct dsound_globals dsound_globals;
static int refills,unlocks,events,resets,lock_depth;
static unsigned long status[8],completed[8];
#define mixer_lock 0
#define pthread_mutex_lock(...) ((void)lock_depth++)
#define pthread_mutex_unlock(...) ((void)lock_depth--)
#define channel_get(i) (&dsound_globals.channels[i])
#define sound_cache_sound_hardware_unlock(...) ((void)unlocks++)
#define interrupt_time_error(...) CHECK(FALSE,"invalid completion")
#define SetEvent(...) ((void)events++)
#define resampler_reset(...) ((void)resets++)
/* (the fade of what the voice would have played next: the mixer's, test_mixer.py) */
#define voice_cut(...) ((void)0)
static void dsound_channel_fill(short i) {
 (void)i; refills++;
 CHECK(refills<10,"a flush repeatedly refills itself");
 stream.packet_count++; dsound_globals.channels[0].packet_count++;
}
static HRESULT stream_flush(IDirectSoundStream *object);
#define DirectSoundStopStream(s) ((void)stream_flush(s))
/* The real callback's Xbox pointer-sized context is cast to a short channel number. */
#pragma clang diagnostic ignored "-Wvoid-pointer-to-int-cast"
#include "under_test.inc"
static void callback(void *context,void *packet,DWORD result) {
 CHECK(lock_depth==0,"callback runs without the mixer lock");
 dsound_channel_callback(context,packet,result);
}
static void reset(int count,int finished,int wrapped) {
 memset(&stream,0,sizeof(stream)); memset(&dsound_globals,0,sizeof(dsound_globals));
 stream.callback=callback; stream.packet_head=wrapped?MAXIMUM_STREAM_PACKETS-2:0;
 stream.packet_count=count; stream.cursor=71;
 dsound_globals.actual_channel_count=1;
 struct sound_channel *channel=channel_get(0);
 channel->stream=&stream.object; channel->state=_sound_channel_playing; channel->packet_count=count;
 channel->playing_permutation=channel->queued_permutation=(struct sound_permutation *)&stream;
 for(int i=0;i<count;i++) {
  struct voice_packet *p=&stream.packets[(stream.packet_head+i)%MAXIMUM_STREAM_PACKETS];
  p->finished=finished==2 || (finished==1 && i==0);
  p->samples=malloc(16); p->packet.dwMaxSize=512;
  status[i]=XMEDIAPACKET_STATUS_PENDING; completed[i]=999;
  p->packet.pdwStatus=&status[i]; p->packet.pdwCompletedSize=&completed[i];
  p->packet.pContext=(void *)(long)(i+1);
 }
 refills=unlocks=events=resets=lock_depth=0;
}
int main(int argc,char **argv) {
 CHECK(argc==2,"case name"); const char *case_name=argv[1];
 int count=!strcmp(case_name,"empty")?0:3;
 int finished=!strcmp(case_name,"unfinished")?0:(!strcmp(case_name,"all-finished")?2:1);
 reset(count,finished,!strcmp(case_name,"wrapped"));
 CASE("ordinary") {
  streams_complete_finished();
  CHECK(status[0]==XMEDIAPACKET_STATUS_SUCCESS && completed[0]==512,"ordinary completion succeeds");
  CHECK(refills==1 && unlocks==1 && stream.packet_count==3,"ordinary completion refills");
  stream_flush(&stream.object); return 0;
 }
 CASE("paused") { dsound_globals.paused=TRUE; }
 CASE("no-callback") { stream.callback=NULL; }
 channel_stop(0);
 CHECK(stream.packet_count==0 && stream.cursor==0 && !lock_depth,"flush empties stream and resets cursor");
 CHECK(refills==0 && resets==1,"cancellation never refills and resets resampling");
 for(int i=0;i<count;i++) {
  CHECK(status[i]==XMEDIAPACKET_STATUS_FLUSHED,"even finished packets are cancelled");
  CHECK(completed[i]==((finished==2 || (finished==1 && i==0))?512:0),"preserve completed byte count");
 }
 CASE("no-callback") { CHECK(events==count,"event completions fire"); return 0; }
 CHECK(unlocks==count && dsound_globals.channels[0].packet_count==0,"cache ownership released once");
 CHECK(!dsound_globals.channels[0].playing_permutation && !dsound_globals.channels[0].queued_permutation,"producers cleared");
 CHECK(dsound_globals.channels[0].state==_sound_channel_idle,"channel idle");
 CASE("repeat") { channel_stop(0); CHECK(resets==1,"repeated stop is harmless"); }
 return 0;
}
