#include "disc_lhdcv5.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <inttypes.h>
static uint64_t frames;static uint32_t hash=2166136261u;static FILE *pcm_file;
static int output(void *u,const int32_t *pcm,size_t n){(void)u;frames+=n;const uint8_t *p=(const uint8_t *)pcm;for(size_t i=0;i<n*8;i++)hash=(hash^p[i])*16777619u;if(pcm_file&&fwrite(p,1,n*8,pcm_file)!=n*8)return 1;return 0;}
static int fail_sink(void *u,const int32_t *p,size_t n){(void)u;(void)p;(void)n;return 1;}
static unsigned little(const uint8_t *b){return b[0]|b[1]<<8|b[2]<<16|b[3]<<24;}
#define CHECK(x) do{if(!(x)){fprintf(stderr,"FAIL line %u: %s\n",__LINE__,#x);return 1;}}while(0)
int main(int argc,char **argv){
 CHECK(argc==3||argc==4);unsigned rate=(unsigned)atoi(argv[1]);if(argc==4){pcm_file=fopen(argv[3],"wb");CHECK(pcm_file);}
 uint8_t config[11]={0x3a,5,0,0,0x35,0x4c,0x10,0x82,0x11,0,0};config[6]=rate==48000?0x10:rate==96000?4:rate==192000?1:0;
 disc_lhdcv5_config cfg;CHECK(!disc_lhdcv5_parse_config(config,11,&cfg)&&cfg.rate==rate&&cfg.bits==24);
 CHECK(disc_lhdcv5_parse_config(config,10,&cfg)<0);config[6]|=0x20;CHECK(disc_lhdcv5_parse_config(config,11,&cfg)<0);config[6]&=~0x20;
 disc_lhdcv5 *s=disc_lhdcv5_open(config,11);CHECK(s);CHECK(!disc_lhdcv5_open(config,11)&&errno==EBUSY);
 FILE *f=fopen(argv[2],"rb");CHECK(f);uint8_t b[8192],head[8];CHECK(fread(head,1,8,f)==8&&!memcmp(head,"DLHDC001",8));
 uint8_t first[8192];size_t first_size=0;unsigned packets=0;
 while(fread(head,1,4,f)==4){unsigned n=little(head);CHECK(n<=sizeof(b)&&fread(b,1,n,f)==n);if(!first_size){first_size=n;memcpy(first,b,n);}CHECK(disc_lhdcv5_packet(s,b,n,output,NULL)>=0);packets++;}
 fclose(f);if(pcm_file){CHECK(!fclose(pcm_file));pcm_file=NULL;}disc_lhdcv5_stats st;disc_lhdcv5_get_stats(s,&st);CHECK(packets==512&&st.decode_errors==0&&st.malformed==0&&!st.pending_bytes&&st.pcm_frames==frames);
 printf("{\"rate\":%u,\"packets\":%u,\"frames\":%"PRIu64",\"pcm_frames\":%"PRIu64",\"pcm_fnv\":%u,\"decode_errors\":%"PRIu64",\"gaps\":%"PRIu64",\"max_us\":%u}\n",rate,packets,st.frames,st.pcm_frames,hash,st.decode_errors,st.gaps,st.max_decode_us);
 /* Truncation/invalid RTP; duplicate suppression and wrap-aware sequence state. */
 CHECK(disc_lhdcv5_packet(s,first,5,output,NULL)<0);
 disc_lhdcv5_flush(s);CHECK(disc_lhdcv5_packet(s,first,first_size,output,NULL)>0);uint64_t old=frames;
 CHECK(disc_lhdcv5_packet(s,first,first_size,output,NULL)==0&&frames==old);
 uint8_t wrong[8192];memcpy(wrong,first,first_size);wrong[0]=0x90;wrong[12]=0;wrong[13]=1;wrong[14]=255;wrong[15]=255;CHECK(disc_lhdcv5_packet(s,wrong,first_size,output,NULL)<0);
 disc_lhdcv5_flush(s);CHECK(disc_lhdcv5_packet(s,first,first_size,fail_sink,NULL)<0&&errno==EIO);
 disc_lhdcv5_close(s);s=disc_lhdcv5_open(config,11);CHECK(s);
 /* A frame split across two transport records must produce identical PCM. */
 size_t split=20;uint8_t half[8192];memcpy(half,first,first_size);size_t second_size=14+first_size-split;memcpy(half+14,first+split,first_size-split);unsigned seq=(first[2]*256+first[3]+1)&65535;half[2]=seq>>8;half[3]=seq;
 uint64_t before=frames;CHECK(disc_lhdcv5_packet(s,first,split,output,NULL)==0);CHECK(disc_lhdcv5_packet(s,half,second_size,output,NULL)>0&&frames>before);
 disc_lhdcv5_get_stats(s,&st);CHECK(!st.pending_bytes);disc_lhdcv5_close(s);
 puts("PASS adapter: real capture, exact configuration, singleton guard, truncation, extension bounds, duplicate, callback failure, frame carry and teardown");return 0;
}
