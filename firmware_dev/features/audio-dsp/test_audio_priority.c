#include <assert.h>
#include <stdio.h>
static unsigned fake_tid=101;
static int nice_values[256],writes,fail;
long syscall(long n,...){assert(n==4222);return fake_tid;}
int getpriority(int which,unsigned tid){assert(which==0);return nice_values[tid];}
int setpriority(int which,unsigned tid,int value){assert(which==0);writes++;if(fail)return -1;nice_values[tid]=value;return 0;}
int access(const char *path,int mode){(void)mode;unsigned tid=0;assert(sscanf(path,"/proc/self/task/%u",&tid)==1);return tid==201?-1:0;}
#include "audio_priority.h"
int main(void){
 dsp_audio_priority(1);assert(nice_values[101]==-5&&dsp_priority_owned&&writes==1);
 dsp_audio_priority(1);assert(writes==1);
 dsp_audio_priority(0);assert(nice_values[101]==0&&!dsp_priority_owned);
 nice_values[101]=-10;dsp_audio_priority(1);assert(nice_values[101]==-10&&!dsp_priority_owned);
 fake_tid=102;dsp_audio_priority(1);assert(nice_values[102]==-5);
 nice_values[102]=-8;fake_tid=103;dsp_audio_priority(1);assert(nice_values[102]==-8&&nice_values[103]==-5);
 fake_tid=201;dsp_audio_priority(1);fake_tid=104;dsp_audio_priority(1);assert(nice_values[201]==-5);
 dsp_audio_priority(0);fail=1;dsp_audio_priority(1);assert(!dsp_priority_owned&&dsp_priority_result==-1&&nice_values[104]==0);
 puts("PASS audio nice boost, no per-buffer writes, off restores, existing policy preserved, old-thread ownership, vanished thread and failure fallback");
}
