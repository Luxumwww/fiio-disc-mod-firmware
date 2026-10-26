/* Passive bounded snapshots. No PCM consumer, bus calls or audio thread. */
static int lhdcv5_snapshot(unsigned words[30]){
 int fd=open("/tmp/disc-runtime/lhdcv5.state",0);if(fd<0)return 0;
 unsigned tail[4];int n=pread(fd,words,120,0),again=pread(fd,tail,16,0);close(fd);
 if(n<96||again!=16||words[0]!=0x3543484cu||(words[1]&1)||words[1]!=tail[1]||!words[3])return 0;
 if(words[2]!=1&&words[2]!=2)return 0;
 if(words[19]!=48000&&words[19]!=96000&&words[19]!=192000)return 0;
 if(words[20]!=16&&words[20]!=24)return 0;
 if(n<120||words[2]!=2){for(unsigned i=24;i<30;i++)words[i]=0;}
 return 1;
}
