#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "split_write.h"
static unsigned char audio[112896];
static unsigned consumed,limit,call,fail_call;
static int fail_result;
static int writer(void *pcm,const void *p,unsigned bytes){
 assert(pcm==audio&&bytes&&bytes<=4096&&!(bytes&7));call++;
 assert(p==audio+consumed*8);
 unsigned count=bytes/8;
 if(call==fail_call)return fail_result;
 if(limit&&count>limit)count=limit;
 for(unsigned i=0;i<count;i++){
  const unsigned char *q=(const unsigned char *)p+i*8;
  assert(!memcmp(q,audio+(consumed+i)*8,8));
  assert(q[3]==(((consumed+i)&1)?250:5)&&q[7]==q[3]);
 }
 consumed+=count;return (int)count;
}
static void reset(void){memset(&split_stats,0,sizeof(split_stats));consumed=limit=call=fail_call=0;fail_result=0;}
int main(void){
 for(unsigned i=0;i<sizeof(audio)/8;i++)for(unsigned j=0;j<8;j++)
  audio[i*8+j]=(j==3||j==7)?((i&1)?250:5):(unsigned char)(i*17+j);
 reset();assert(split_write_block(audio,audio,sizeof(audio),writer)==14112);
 assert(consumed==14112&&call==28&&split_stats.max_request_frames==512&&!split_stats.fault);
 reset();limit=13;assert(split_write_block(audio,audio,sizeof(audio),writer)==14112);
 assert(consumed==14112&&split_stats.partials>0&&!split_stats.fault);
 /* Odd partial counts must retain phase and channel order across calls. */
 reset();assert(split_write_block(audio,audio,8,writer)==1&&consumed==1);
 reset();fail_call=2;fail_result=-32;
 assert(split_write_block(audio,audio,sizeof(audio),writer)==-32);
 assert(consumed==512&&call==2&&split_stats.errors==1&&split_stats.fault);
 assert(split_stats.incomplete_frames==14112-512);
 reset();fail_call=2;fail_result=0;
 assert(split_write_block(audio,audio,sizeof(audio),writer)==0);
 assert(consumed==512&&call==2&&split_stats.zero_results==1&&split_stats.fault);
 reset();fail_call=1;fail_result=513;
 assert(split_write_block(audio,audio,sizeof(audio),writer)==-5);
 assert(!consumed&&call==1&&split_stats.invalid_results==1&&split_stats.fault);
 puts("PASS: realtime 40ms payload splitting, odd partial continuation, exact phase/channel/data order, tail, error and ambiguous-zero no retry");
}
