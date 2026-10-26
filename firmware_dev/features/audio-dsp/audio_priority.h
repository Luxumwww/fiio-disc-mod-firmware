/* Fair-scheduler preference for the existing DSP caller, no RT policy. */
extern int getpriority(int,unsigned),setpriority(int,unsigned,int);
static unsigned dsp_audio_tid;
static int dsp_audio_nice,dsp_priority_active,dsp_priority_owned,dsp_priority_result;
static void dsp_audio_priority(int active){
 unsigned tid=(unsigned)syscall(4222); /* Linux MIPS o32 gettid. */
 if(!tid||tid>0x7fffffffu)return;
 if(tid==dsp_audio_tid&&active==dsp_priority_active)return;
 if(dsp_priority_owned){char path[80];snprintf(path,sizeof(path),"/proc/self/task/%u",dsp_audio_tid);
  if(!access(path,0)&&getpriority(0,dsp_audio_tid)==-5)setpriority(0,dsp_audio_tid,0);
 }
 dsp_audio_tid=tid;dsp_priority_active=active;dsp_priority_owned=0;dsp_priority_result=0;dsp_audio_nice=getpriority(0,tid);
 /* Preserve every existing non-default priority. Only undo our own value. */
 if(active&&dsp_audio_nice==0){dsp_priority_result=setpriority(0,tid,-5);dsp_audio_nice=getpriority(0,tid);dsp_priority_owned=dsp_priority_result==0&&dsp_audio_nice==-5;}
}
