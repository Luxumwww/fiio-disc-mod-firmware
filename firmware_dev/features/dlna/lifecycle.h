/* V2.57's audio stop sets renderer_quit before close_player. The stock
 * stop_dlna then returns early, skipping join and cleanup. Re-entering can
 * leave the old renderer shutting down the newly initialized UPnP service.
 * Both native stop call sites must reap the thread exactly once. */
static unsigned dlna_stop_lock[6] __attribute__((aligned(8)));
static int dlna_lifecycle_ready;
static void dlna_stop_complete(void){
 pthread_mutex_lock(dlna_stop_lock);
 if(PC_WORD(0x83e010)){
  /* Native stop owns signal, join, UPnP and playlist cleanup. Force its
   * cleanup branch even when audio_track_stop already signaled quit. */
  PC_WORD(0x89ae64)=0;
  PC_FN(0x5e9eb8,void (*)(void))();
  PC_WORD(0x83e010)=0;
 }
 pthread_mutex_unlock(dlna_stop_lock);
}
#ifndef DISC_DLNA_LIFECYCLE_TEST
static int dlna_lifecycle_install(void){
 const unsigned sites[2]={0x457ec0,0x466f1c};
 if(dlna_lifecycle_ready)return 1;
 for(unsigned i=0;i<2;i++)if(PC_WORD(sites[i])!=0x0c17a7ae)return 0;
 unsigned *stub=mmap((void *)0x07300000,4096,3,0x802,-1,0);
 unsigned address=(unsigned)(unsigned long)stub,target=(unsigned)(unsigned long)dlna_stop_complete;
 if(address<0x10000||address>=0x10000000u||(address&4095)){if(stub!=(void *)-1)munmap(stub,4096);return 0;}
 stub[0]=0x3c190000|(target>>16);stub[1]=0x37390000|(target&65535);stub[2]=0x03200008;stub[3]=0;
 if(cacheflush(stub,16,3)||mprotect(stub,4096,5)){munmap(stub,4096);return 0;}
 if(mprotect((void *)0x457000,4096,7)){munmap(stub,4096);return 0;}
 if(mprotect((void *)0x466000,4096,7)){mprotect((void *)0x457000,4096,5);munmap(stub,4096);return 0;}
 for(unsigned i=0;i<2;i++)PC_WORD(sites[i])=0x0c000000|(address>>2);
 __sync_synchronize();int rc=cacheflush((void *)0x457000,4096,3)|cacheflush((void *)0x466000,4096,3);
 if(rc){for(unsigned i=0;i<2;i++)PC_WORD(sites[i])=0x0c17a7ae;cacheflush((void *)0x457000,4096,3);cacheflush((void *)0x466000,4096,3);}
 int protection=mprotect((void *)0x457000,4096,5)|mprotect((void *)0x466000,4096,5);
 dlna_lifecycle_ready=!rc&&!protection;return dlna_lifecycle_ready;
}
#endif
