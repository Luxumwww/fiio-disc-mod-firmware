#include <stdio.h>
#include <unistd.h>
#include <errno.h>
extern int snd_pcm_open(void **,const char *,int,int),snd_pcm_close(void *);
extern const char *snd_strerror(int);
int main(int argc,char **argv){
 if(argc!=2)return 2;
 for(unsigned i=0;i<80;i++){
  void *pcm=0;int r=snd_pcm_open(&pcm,argv[1],1,1);
  printf("attempt=%u result=%d error=%s errno=%d\n",i,r,snd_strerror(r),errno);fflush(stdout);
  if(!r){snd_pcm_close(pcm);puts("Opened and immediately closed; no audio read performed");return 0;}
  usleep(100000);
 }
 return 1;
}
