/* D-step harness: ASan/UBSan mutation fuzzing and lifecycle checks for the
 * host LHDC V5 decoder adapter. Never touches a device. */
#define _POSIX_C_SOURCE 200809L
#include "disc_lhdcv5.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <signal.h>
#include <dirent.h>

/* ---------------- allocation failure injection ---------------- */
long g_fail_alloc_at = -1;
long g_alloc_calls = 0;
void *__real_malloc(size_t);
void *__real_calloc(size_t, size_t);
void *__real_realloc(void *, size_t);
void *__wrap_malloc(size_t n){ if(g_fail_alloc_at>=0 && ++g_alloc_calls==g_fail_alloc_at) return NULL; return __real_malloc(n); }
void *__wrap_calloc(size_t a, size_t b){ if(g_fail_alloc_at>=0 && ++g_alloc_calls==g_fail_alloc_at) return NULL; return __real_calloc(a,b); }
void *__wrap_realloc(void *p, size_t n){ if(g_fail_alloc_at>=0 && ++g_alloc_calls==g_fail_alloc_at) return NULL; return __real_realloc(p,n); }

/* ---------------- stream model ---------------- */
typedef struct { uint8_t *d; size_t n; } pkt_t;
typedef struct { pkt_t *p; size_t n, cap; } stream_t;

static void stream_init(stream_t *s){ s->p=NULL; s->n=0; s->cap=0; }
static void stream_reserve(stream_t *s, size_t want){
  if(want<=s->cap) return;
  size_t cap=s->cap?s->cap:64;
  while(cap<want) cap*=2;
  pkt_t *np=(pkt_t*)realloc(s->p,cap*sizeof(pkt_t));
  if(!np){ perror("realloc"); exit(70); }
  s->p=np; s->cap=cap;
}
static void stream_push(stream_t *s, const uint8_t *d, size_t n){
  stream_reserve(s,s->n+1);
  uint8_t *c=(uint8_t*)malloc(n?n:1);
  if(!c){ perror("malloc"); exit(70); }
  if(n) memcpy(c,d,n);
  s->p[s->n].d=c; s->p[s->n].n=n; s->n++;
}
static void stream_insert(stream_t *s, size_t idx, const uint8_t *d, size_t n){
  if(idx>s->n) idx=s->n;
  stream_reserve(s,s->n+1);
  memmove(&s->p[idx+1],&s->p[idx],(s->n-idx)*sizeof(pkt_t));
  uint8_t *c=(uint8_t*)malloc(n?n:1);
  if(!c){ perror("malloc"); exit(70); }
  if(n) memcpy(c,d,n);
  s->p[idx].d=c; s->p[idx].n=n; s->n++;
}
static void stream_remove(stream_t *s, size_t idx){
  if(idx>=s->n) return;
  free(s->p[idx].d);
  memmove(&s->p[idx],&s->p[idx+1],(s->n-idx-1)*sizeof(pkt_t));
  s->n--;
}
static void stream_free(stream_t *s){ for(size_t i=0;i<s->n;i++) free(s->p[i].d); free(s->p); stream_init(s); }
static void stream_clone(stream_t *dst, const stream_t *src){ stream_init(dst); for(size_t i=0;i<src->n;i++) stream_push(dst,src->p[i].d,src->p[i].n); }
static void stream_truncate(stream_t *s, size_t keep){ while(s->n>keep) stream_remove(s,s->n-1); }

static int stream_load(stream_t *s, const char *path){
  FILE *f=fopen(path,"rb"); if(!f) return -1;
  uint8_t magic[8];
  if(fread(magic,1,8,f)!=8||memcmp(magic,"DLHDC001",8)){ fclose(f); return -2; }
  stream_init(s);
  uint8_t h[4];
  while(fread(h,1,4,f)==4){
    size_t n=(size_t)h[0]|((size_t)h[1]<<8)|((size_t)h[2]<<16)|((size_t)h[3]<<24);
    if(n>100000){ fclose(f); return -3; }
    uint8_t *b=(uint8_t*)malloc(n?n:1);
    if(!b){ fclose(f); return -4; }
    if(n&&fread(b,1,n,f)!=n){ free(b); fclose(f); return -5; }
    stream_push(s,b,n); free(b);
  }
  fclose(f); return 0;
}
static void stream_save(const stream_t *s, const char *path){
  FILE *f=fopen(path,"wb"); if(!f) return;
  fwrite("DLHDC001",1,8,f);
  for(size_t i=0;i<s->n;i++){
    size_t n=s->p[i].n; uint8_t h[4]={(uint8_t)n,(uint8_t)(n>>8),(uint8_t)(n>>16),(uint8_t)(n>>24)};
    fwrite(h,1,4,f); if(n) fwrite(s->p[i].d,1,n,f);
  }
  fclose(f);
}

/* ---------------- playback ---------------- */
static uint64_t g_sink_total=0;
static long g_sink_fail_after=-1;
static int sink_cb(void *u,const int32_t *pcm,size_t n){
  (void)u;(void)pcm;
  g_sink_total+=n;
  if(g_sink_fail_after>=0 && g_sink_total>(uint64_t)g_sink_fail_after) return 1;
  return 0;
}
static void make_config(uint8_t cfg[11], unsigned rate, unsigned bits){
  static const uint8_t head[6]={0x3a,5,0,0,0x35,0x4c};
  memcpy(cfg,head,6);
  cfg[6]= rate==48000?0x10: rate==96000?4: rate==192000?1:0;
  cfg[7]= bits==16?0x84:0x82;
  cfg[8]=0x11; cfg[9]=0; cfg[10]=0;
}
static int play(const stream_t *s, unsigned rate, unsigned bits, long fail_after, uint64_t *frames){
  uint8_t cfg[11]; make_config(cfg,rate,bits);
  g_sink_total=0; g_sink_fail_after=fail_after;
  disc_lhdcv5 *d=disc_lhdcv5_open(cfg,11);
  if(!d) return -1;
  for(size_t i=0;i<s->n;i++) disc_lhdcv5_packet(d,s->p[i].d,s->p[i].n,sink_cb,NULL);
  disc_lhdcv5_flush(d);
  disc_lhdcv5_close(d);
  if(frames) *frames=g_sink_total;
  return 0;
}

/* ---------------- RNG ---------------- */
static uint64_t g_rng=0x9E3779B97F4A7C15ULL;
static uint64_t rnd(void){ g_rng^=g_rng<<13; g_rng^=g_rng>>7; g_rng^=g_rng<<17; return g_rng; }
static size_t rndn(size_t n){ return n?(size_t)(rnd()%n):0; }
static void pkt_ensure(pkt_t *k, size_t n){
  if(n<=k->n) return;
  uint8_t *np=(uint8_t*)realloc(k->d,n); if(!np){ perror("realloc"); exit(71); }
  memset(np+k->n,0,n-k->n); k->d=np; k->n=n;
}

/* ---------------- mutation ---------------- */
#define OP_COUNT 18
static const char *op_name(int op){
  static const char *names[OP_COUNT]={
    "truncate","extend-random-tail","overlong-gt-8192","bitflip","rtp-version",
    "payload-type","csrc-count","rtp-extension","padding","frame-length",
    "duplicate-seq","seq-wrap","ssrc-change","reorder","drop","split-record",
    "garbage-insert","zero-packet"};
  return (op>=0&&op<OP_COUNT)?names[op]:"?";
}
static void mutate(stream_t *s, unsigned ops, int *chosen, size_t maxchosen, size_t *nchosen){
  *nchosen=0;
  for(unsigned it=0;it<ops;it++){
    int op=(int)rndn(OP_COUNT);
    if(*nchosen<maxchosen) chosen[(*nchosen)++]=op;
    switch(op){
    case 0: if(s->n){ pkt_t *k=&s->p[rndn(s->n)]; if(k->n>1) k->n=1+rndn(k->n-1); } break;
    case 1: if(s->n){ pkt_t *k=&s->p[rndn(s->n)]; size_t add=rndn(80); size_t old=k->n; pkt_ensure(k,k->n+add); for(size_t i=old;i<k->n;i++) k->d[i]=(uint8_t)rnd(); } break;
    case 2: if(s->n){ pkt_t *k=&s->p[rndn(s->n)]; size_t nn=8193+rndn(400); pkt_ensure(k,nn); for(size_t i=0;i<nn;i++) k->d[i]=(uint8_t)rnd(); } break;
    case 3: if(s->n){ pkt_t *k=&s->p[rndn(s->n)]; size_t flips=1+rndn(16); for(size_t i=0;i<flips && k->n;i++) k->d[rndn(k->n)]^=(uint8_t)(1u<<rndn(8)); } break;
    case 4: if(s->n){ pkt_t *k=&s->p[rndn(s->n)]; if(k->n) k->d[0]=(uint8_t)((k->d[0]&0x3f)|((uint8_t)rndn(4)<<6)); } break;
    case 5: if(s->n){ pkt_t *k=&s->p[rndn(s->n)]; if(k->n>=2) k->d[1]=(uint8_t)rnd(); } break;
    case 6: if(s->n){ pkt_t *k=&s->p[rndn(s->n)]; if(k->n) k->d[0]=(uint8_t)((k->d[0]&0xf0)|(uint8_t)rndn(16)); } break;
    case 7: if(s->n){ pkt_t *k=&s->p[rndn(s->n)]; if(k->n){ k->d[0]|=16; pkt_ensure(k,24); for(size_t i=12;i<24;i++) k->d[i]=(uint8_t)rnd(); } } break;
    case 8: if(s->n){ pkt_t *k=&s->p[rndn(s->n)]; if(k->n){ k->d[0]|=32; k->d[k->n-1]= rndn(3)?(uint8_t)rnd():0; } } break;
    case 9: if(s->n){ pkt_t *k=&s->p[rndn(s->n)]; size_t off=k->n>16?14:0; if(k->n>=off+2){ k->d[off]=(uint8_t)rnd(); k->d[off+1]=(uint8_t)rnd(); } } break;
    case 10: if(s->n>=2){ size_t i=1+rndn(s->n-1); if(s->p[i].n>=4&&s->p[i-1].n>=4) memcpy(s->p[i].d+2,s->p[i-1].d+2,2); } break;
    case 11: if(s->n>=2){ if(s->p[0].n>=4){ s->p[0].d[2]=0xff; s->p[0].d[3]=0xff; } if(s->p[1].n>=4){ s->p[1].d[2]=0; s->p[1].d[3]=0; } } break;
    case 12: if(s->n){ pkt_t *k=&s->p[rndn(s->n)]; if(k->n>=12){ k->d[8]^=(uint8_t)rnd(); k->d[9]^=(uint8_t)rnd(); } } break;
    case 13: if(s->n>=2){ size_t i=rndn(s->n), j=rndn(s->n); pkt_t t=s->p[i]; s->p[i]=s->p[j]; s->p[j]=t; } break;
    case 14: if(s->n>=2) stream_remove(s,rndn(s->n)); break;
    case 15: if(s->n){ size_t i=rndn(s->n); pkt_t *k=&s->p[i]; if(k->n>=4){ size_t cut=1+rndn(k->n-1); size_t tn=k->n-cut; uint8_t *tail=(uint8_t*)malloc(tn?tn:1); if(!tail) exit(72); if(tn) memcpy(tail,k->d+cut,tn); k->n=cut; stream_insert(s,i+1,tail,tn); free(tail); } } break;
    case 16: { size_t nn=rndn(8500); uint8_t *b=(uint8_t*)malloc(nn?nn:1); if(!b) exit(73); for(size_t i=0;i<nn;i++) b[i]=(uint8_t)rnd(); stream_insert(s,rndn(s->n+1),b,nn); free(b); } break;
    case 17: if(s->n){ pkt_t *k=&s->p[rndn(s->n)]; if(k->n) memset(k->d,0,k->n); } break;
    }
  }
}

/* ---------------- forked case runner ---------------- */
static const stream_t *g_base;
static unsigned g_rate,g_bits;
static int g_recovery;
static void child_body_mut(const stream_t *mut,long fail_after){
  alarm(60);
  uint64_t f=0;
  play(mut,g_rate,g_bits,fail_after,&f);
  if(g_recovery){ uint64_t rf=0; if(play(g_base,g_rate,g_bits,-1,&rf)!=0||rf==0) _exit(3); }
  _exit(0);
}

static long g_crashes=0,g_recfail=0,g_hangs=0,g_san=0,g_san_saved=0;
static FILE *g_log;
static void report(const char *kind,unsigned long idx,stream_t *mut,unsigned rate,unsigned bits,
                   int *ops,size_t nops,const char *base_name,const char *outdir){
  char path[1024];
  const char *shortname=strrchr(base_name,'/'); shortname=shortname?shortname+1:base_name;
  snprintf(path,sizeof path,"%s/%s-%06lu.dlhd",outdir,kind,idx);
  stream_save(mut,path);
  printf("[%s] case=%lu rate=%u bits=%u base=%s ops=",kind,idx,rate,bits,shortname);
  for(size_t i=0;i<nops;i++) printf("%s%s",i?",":"",op_name(ops[i]));
  printf(" -> %s\n",path);
  if(g_log){ fprintf(g_log,"%s\t%lu\t%u\t%u\t%s\t",kind,idx,rate,bits,shortname);
    for(size_t i=0;i<nops;i++) fprintf(g_log,"%s ",op_name(ops[i]));
    fprintf(g_log,"\t%s\n",path); fflush(g_log); }
}

static void run_case(const char *tag,unsigned long idx,stream_t *mut,const stream_t *base,
                     unsigned rate,unsigned bits,int *ops,size_t nops,const char *base_name,
                     const char *outdir,long fail_after,int recovery){
  g_base=base; g_rate=rate; g_bits=bits; g_recovery=recovery;
  pid_t pid=fork();
  if(pid==0) child_body_mut(mut,fail_after);
  if(pid<0){ perror("fork"); exit(74); }
  int st=0; waitpid(pid,&st,0);
  if(WIFSIGNALED(st)){
    int sig=WTERMSIG(st);
    g_crashes++;
    if(sig==SIGALRM){ g_hangs++; report("HANG",idx,mut,rate,bits,ops,nops,base_name,outdir); }
    else report("CRASH",idx,mut,rate,bits,ops,nops,base_name,outdir);
    return;
  }
  if(WIFEXITED(st)){
    int rc=WEXITSTATUS(st);
    if(rc==3){ g_recfail++; report("RECOVERY-FAIL",idx,mut,rate,bits,ops,nops,base_name,outdir); }
    else if(rc!=0){ g_san++; if(g_san_saved<20){ g_san_saved++; report("SANITIZER",idx,mut,rate,bits,ops,nops,base_name,outdir); } }
  }
}

/* ---------------- process stats ---------------- */
static void proc_stats(long *rss_kb,int *fds,int *threads){
  *rss_kb=-1; *fds=-1; *threads=-1;
  FILE *f=fopen("/proc/self/statm","r");
  if(f){ long sz=0,res=0; if(fscanf(f,"%ld %ld",&sz,&res)==2) *rss_kb=res*(sysconf(_SC_PAGESIZE)/1024); fclose(f); }
  *fds=0; DIR *d=opendir("/proc/self/fd");
  if(d){ struct dirent *e; while((e=readdir(d))) if(e->d_name[0]!='.') (*fds)++; closedir(d); }
  f=fopen("/proc/self/status","r");
  if(f){ char line[256]; while(fgets(line,sizeof line,f)) if(!strncmp(line,"Threads:",8)){ *threads=atoi(line+8); break; } fclose(f); }
}

/* ---------------- modes ---------------- */
static const char *g_captures[3]={"testdata/capture-48000.packets","testdata/capture-96000.packets","testdata/capture-192000.packets"};
static unsigned g_rates[3]={48000,96000,192000};
#define CASEPACKETS 64u

static int mode_selftest(void){
  for(int i=0;i<3;i++){
    stream_t s; int rc=stream_load(&s,g_captures[i]);
    if(rc){ printf("load %s failed rc=%d\n",g_captures[i],rc); return 1; }
    uint64_t f=0; if(play(&s,g_rates[i],24,-1,&f)) { printf("open failed rate=%u\n",g_rates[i]); return 1; }
    printf("selftest rate=%u packets=%zu pcm_frames=%llu\n",g_rates[i],s.n,(unsigned long long)f);
    stream_free(&s);
  }
  return 0;
}

static int mode_fuzz(const char *outdir,unsigned long iterations){
  stream_t base[3]; int have[3]={0,0,0};
  for(int i=0;i<3;i++){
    if(stream_load(&base[i],g_captures[i])==0){ stream_truncate(&base[i],CASEPACKETS); have[i]=1; }
    else printf("warning: cannot load %s\n",g_captures[i]);
  }
  char logpath[1024]; snprintf(logpath,sizeof logpath,"%s/cases.log",outdir);
  g_log=fopen(logpath,"w");
  unsigned long reported=0;
  for(unsigned long it=0; it<iterations; it++){
    int bi=-1; do{ bi=(int)rndn(3); }while(!have[bi]);
    unsigned bits = (rndn(4)==0)?16:24;   /* mostly source 24-bit, sometimes 16-bit path */
    stream_t mut; stream_clone(&mut,&base[bi]);
    int ops[6]; size_t nops=0; unsigned count=1+(unsigned)rndn(4);
    if(rndn(8)==0){ /* whole-stream random */
      stream_free(&mut); stream_init(&mut);
      size_t np=1+rndn(12);
      for(size_t i=0;i<np;i++){ size_t nn=rndn(8600); uint8_t *b=(uint8_t*)malloc(nn?nn:1); if(!b) exit(75); for(size_t j=0;j<nn;j++) b[j]=(uint8_t)rnd(); stream_push(&mut,b,nn); free(b); }
      ops[nops++]=99;
    } else {
      mutate(&mut,count,ops,6,&nops);
    }
    long fail_after = (rndn(6)==0)? (long)rndn(2000) : -1;
    run_case("CRASH",it,&mut,&base[bi],g_rates[bi],bits,ops,nops,g_captures[bi],outdir,fail_after,1);
    if(g_crashes+g_recfail+g_hangs>reported){
      reported=g_crashes+g_recfail+g_hangs;
    }
    stream_free(&mut);
    if((it+1)%500==0) printf("... %lu/%lu cases  crashes=%ld recovery_fail=%ld hangs=%ld sanitizer=%ld\n",it+1,iterations,g_crashes,g_recfail,g_hangs,g_san);
  }
  if(g_log) fclose(g_log);
  for(int i=0;i<3;i++) if(have[i]) stream_free(&base[i]);
  printf("fuzz done: iterations=%lu crashes=%ld hangs=%ld recovery_fail=%ld sanitizer_aborts=%ld\n",iterations,g_crashes,g_hangs,g_recfail,g_san);
  return (g_crashes||g_hangs||g_recfail||g_san)?1:0;
}

static int mode_lifecycle(unsigned long cycles){
  stream_t base[3]; int have[3]={0,0,0};
  for(int i=0;i<3;i++){ if(stream_load(&base[i],g_captures[i])==0){ stream_truncate(&base[i],CASEPACKETS); have[i]=1; } }
  long rss0=0,rss=0; int rfds=0,rthreads=0,fds=0,threads=0;
  proc_stats(&rss0,&rfds,&rthreads);
  printf("lifecycle start rss_kb=%ld fds=%ld threads=%ld\n",rss0,rfds,rthreads);
  for(unsigned long i=0;i<cycles;i++){
    int bi=(int)(i%3);
    if(!have[bi]) continue;
    unsigned bits=((i/3)%2)?24:16;
    uint64_t f=0;
    if(play(&base[bi],g_rates[bi],bits,-1,&f)){ printf("lifecycle open failed at %lu\n",i); return 1; }
    if((i+1)%100==0){
      proc_stats(&rss,&fds,&threads);
      printf("cycle %6lu rate=%6u bits=%u rss_kb=%ld (delta %+ld) fds=%d threads=%d\n",
             i+1,g_rates[bi],bits,rss,rss-rss0,fds,threads);
      fflush(stdout);
    }
  }
  long rss_end=0; int fds_end=0,threads_end=0;
  proc_stats(&rss_end,&fds_end,&threads_end);
  printf("lifecycle end rss_kb=%ld (delta %+ld) fds=%d (delta %+d) threads=%d (delta %+d)\n",
         rss_end,rss_end-rss0,fds_end,fds_end-rfds,threads_end,threads_end-rthreads);
  for(int i=0;i<3;i++) if(have[i]) stream_free(&base[i]);
  return 0;
}

static int mode_openfail(void){
  uint8_t cfg[11]; make_config(cfg,96000,24);
  stream_t s; if(stream_load(&s,g_captures[1])){ printf("load failed\n"); return 1; }
  stream_truncate(&s,16);
  int stuck=0;
  for(int k=1;k<=20;k++){
    g_alloc_calls=0; g_fail_alloc_at=k;
    disc_lhdcv5 *d=disc_lhdcv5_open(cfg,11);
    int failed=(d==NULL);
    g_fail_alloc_at=-1;
    if(d){
      g_sink_total=0; g_sink_fail_after=-1;
      for(size_t i=0;i<s.n;i++) disc_lhdcv5_packet(d,s.p[i].d,s.p[i].n,sink_cb,NULL);
      disc_lhdcv5_flush(d); disc_lhdcv5_close(d);
    }
    /* singleton lock must be released no matter where allocation failed */
    disc_lhdcv5 *ok=disc_lhdcv5_open(cfg,11);
    printf("fail_at=%2d open=%-3s afterwards=%s\n",k,failed?"NULL":"ok",ok?"ok":"STUCK");
    if(!ok) stuck=1; else disc_lhdcv5_close(ok);
  }
  stream_free(&s);
  return stuck;
}

static int mode_file(const char *path,unsigned rate,unsigned bits){
  stream_t s; int rc=stream_load(&s,path);
  if(rc){ printf("load %s failed rc=%d\n",path,rc); return 2; }
  uint64_t f=0; if(play(&s,rate,bits,-1,&f)){ printf("open failed %s\n",path); stream_free(&s); return 1; }
  printf("file %s rate=%u bits=%u packets=%zu pcm_frames=%llu\n",path,rate,bits,s.n,(unsigned long long)f);
  stream_free(&s); return 0;
}

static int mode_replay(const char *mutpath,const char *basepath,unsigned rate,unsigned bits){
  stream_t m,b;
  if(stream_load(&m,mutpath)){ printf("load mut failed\n"); return 2; }
  if(stream_load(&b,basepath)){ printf("load base failed\n"); return 2; }
  uint64_t f1=0,f2=0;
  int r1=play(&m,rate,bits,-1,&f1);
  int r2=play(&b,rate,bits,-1,&f2);
  printf("replay mut: open_rc=%d frames=%llu | base: open_rc=%d frames=%llu\n",
         r1,(unsigned long long)f1,r2,(unsigned long long)f2);
  stream_free(&m); stream_free(&b);
  return (r1==0&&(int)f1>=0&&r2==0&&f2>0)?0:1;
}


static uint64_t checked_frames;
static uint32_t checked_hash;
static int checked_sink(void *u,const int32_t *pcm,size_t n){
  (void)u; checked_frames+=n;
  const uint8_t *bytes=(const uint8_t *)pcm;
  for(size_t i=0;i<n*8;i++)checked_hash=(checked_hash^bytes[i])*16777619u;
  return 0;
}
static void checked_feed(disc_lhdcv5 *d,const stream_t *s,int new_ssrc){
  for(size_t i=0;i<s->n;i++){
    pkt_t *p=&s->p[i];
    if(new_ssrc && p->n>=12)p->d[8]^=0x80;
    disc_lhdcv5_packet(d,p->d,p->n,checked_sink,NULL);
    if(new_ssrc && p->n>=12)p->d[8]^=0x80;
  }
}
static int same_instance(unsigned long count){
  for(int r=0;r<3;r++){
    stream_t base; if(stream_load(&base,g_captures[r]))return 1;
    stream_truncate(&base,CASEPACKETS);
    uint8_t cfg[11];make_config(cfg,g_rates[r],24);
    disc_lhdcv5 *d=disc_lhdcv5_open(cfg,11);if(!d)return 1;
    checked_frames=0;checked_hash=2166136261u;checked_feed(d,&base,0);
    uint64_t normal_frames=checked_frames;uint32_t normal_hash=checked_hash;
    disc_lhdcv5_close(d);if(!normal_frames)return 1;
    for(unsigned long k=0;k<count;k++){
      stream_t damaged;stream_clone(&damaged,&base);
      int ops[6];size_t nops;mutate(&damaged,1+(unsigned)rndn(4),ops,6,&nops);
      d=disc_lhdcv5_open(cfg,11);if(!d)return 1;
      checked_feed(d,&damaged,0);
      /* Same decoder object: recover on a valid new source, then flush. */
      checked_frames=0;checked_hash=2166136261u;checked_feed(d,&base,1);
      if(checked_frames!=normal_frames||checked_hash!=normal_hash){
        printf("same-instance SSRC fail rate=%u case=%lu frames=%llu expected=%llu hash=%u expected=%u\n",g_rates[r],k,(unsigned long long)checked_frames,(unsigned long long)normal_frames,checked_hash,normal_hash);return 1;
      }
      disc_lhdcv5_flush(d);
      checked_frames=0;checked_hash=2166136261u;checked_feed(d,&base,0);
      if(checked_frames!=normal_frames||checked_hash!=normal_hash){puts("same-instance flush recovery mismatch");return 1;}
      disc_lhdcv5_close(d);stream_free(&damaged);
    }
    printf("same-instance rate=%u cases=%lu exact frames/hash after damaged packets + SSRC/flush passed\n",g_rates[r],count);
    stream_free(&base);
  }
  return 0;
}

int main(int argc,char **argv){
  if(argc>1&&!strcmp(argv[1],"same-instance"))return same_instance(argc>2?strtoul(argv[2],NULL,0):100);
  if(argc<2){ printf("usage: %s selftest|fuzz|lifecycle|openfail|file|replay ...\n",argv[0]); return 2; }
  if(!strcmp(argv[1],"replay")){
    if(argc<6){ printf("usage: replay <mutfile> <basefile> <rate> <bits>\n"); return 2; }
    return mode_replay(argv[2],argv[3],(unsigned)strtoul(argv[4],NULL,0),(unsigned)strtoul(argv[5],NULL,0));
  }
  if(!strcmp(argv[1],"selftest")) return mode_selftest();
  if(!strcmp(argv[1],"file")){
    if(argc<5){ printf("usage: file <path> <rate> <bits>\n"); return 2; }
    return mode_file(argv[2],(unsigned)strtoul(argv[3],NULL,0),(unsigned)strtoul(argv[4],NULL,0));
  }
  if(!strcmp(argv[1],"fuzz")){
    if(argc<3){ printf("usage: fuzz <outdir> [iterations] [rngseed]\n"); return 2; }
    unsigned long it=(argc>=4)?strtoul(argv[3],NULL,0):2000;
    if(argc>=5) g_rng=strtoull(argv[4],NULL,0);
    return mode_fuzz(argv[2],it);
  }
  if(!strcmp(argv[1],"lifecycle")){
    unsigned long c=(argc>=3)?strtoul(argv[2],NULL,0):1000;
    return mode_lifecycle(c);
  }
  if(!strcmp(argv[1],"openfail")) return mode_openfail();
  printf("unknown mode %s\n",argv[1]);
  return 2;
}
