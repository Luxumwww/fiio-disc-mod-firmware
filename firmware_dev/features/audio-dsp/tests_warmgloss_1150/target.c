#define run previous_run
#define __libc_start_main previous_libc_start_main
#include "../tests_stereofx_1130/target.c"
#undef run
#undef __libc_start_main
static void wg_spec(DiscChain *c){memset(c,0,sizeof *c);c->count=1;c->nodes[0].id=1;c->nodes[0].type=16;c->nodes[0].enabled=1;memcpy(c->nodes[0].params,disc_warmgloss_default,sizeof c->nodes[0].params);}
static int warm_test(const DiscDspApi *a,ChainCreate create){
 unsigned rates[]={8000,44100,48000,96000,192000,384000},rng=1150,contexts=0;int x[1024],y[1024];
 for(unsigned r=0;r<6;r++)for(unsigned v=0;v<19;v++){
  DiscChain c;wg_spec(&c);if(v>=1&&v<=16)c.nodes[0].params[(v-1)/2]=disc_warmgloss_range[(v-1)/2][(v-1)&1];
  if(v==17)for(unsigned i=0;i<8;i++)c.nodes[0].params[i]=disc_warmgloss_range[i][1];
  if(v==18)for(unsigned i=0;i<8;i++)c.nodes[0].params[i]=disc_warmgloss_range[i][0];
  void *left=create(rates[r],&c),*right=create(rates[r],&c);CHECK(left&&right);
  for(unsigned k=0;k<16;k++){
   input(x,512,&rng,k&1);for(unsigned i=0;i<512;i++){y[2*i]=x[2*i+1];y[2*i+1]=x[2*i];}
   CHECK(a->process(left,x,512)&&a->process(right,y,512));for(unsigned i=0;i<512;i++)CHECK(x[2*i]==y[2*i+1]&&x[2*i+1]==y[2*i]);
  }
  unsigned lim,bad,av,lat,clip;float peak,gain;a->diagnostics(left,&lim,&bad,&peak,&gain);a->stats(left,&av,&lat,&clip);CHECK(av==4096&&!bad&&!clip&&isfinite(peak)&&isfinite(gain));
  a->set_mask(left,0);for(unsigned k=0;k<80;k++){memset(x,0,sizeof x);a->process(left,x,512);}input(x,512,&rng,1);memcpy(y,x,sizeof x);a->process(left,x,512);CHECK(!memcmp(x,y,sizeof x));
  a->set_mask(left,4096);for(unsigned k=0;k<40;k++){memset(x,0,sizeof x);a->process(left,x,512);for(unsigned i=0;i<1024;i++)CHECK(x[i]==0);}a->destroy(left);a->destroy(right);contexts++;
 }
 printf("PASS %u type16 contexts/6 rates: all endpoints, stereo swap symmetry, finite/guard, exact OFF and silent reset\n",contexts);return 0;
}
static int warm_bench(const DiscDspApi *a,ChainCreate create){
 unsigned rates[]={48000,96000,192000,384000},rng=1150;int *x=malloc(7680*8);CHECK(x);
 for(unsigned r=0;r<4;r++)for(unsigned strong=0;strong<2;strong++){
  DiscChain c;wg_spec(&c);if(strong){c.nodes[0].params[0]=1;c.nodes[0].params[1]=1;c.nodes[0].params[2]=4;c.nodes[0].params[3]=-6;c.nodes[0].params[5]=0;c.nodes[0].params[6]=c.nodes[0].params[7]=1;}
  void *d=create(rates[r],&c);CHECK(d);for(unsigned k=0;k<4;k++){input(x,7680,&rng,strong);a->process(d,x,7680);}
  double cost=0,max=0,wall=0;for(unsigned k=0;k<3;k++){input(x,7680,&rng,strong);struct timespec t;clock_gettime(CLOCK_MONOTONIC,&t);double w=t.tv_sec+t.tv_nsec*1e-9,start=cpu();a->process(d,x,7680);double end=cpu(),step=end-start;clock_gettime(CLOCK_MONOTONIC,&t);wall+=t.tv_sec+t.tv_nsec*1e-9-w;cost+=step;if(step>max)max=step;}
  double period=7680./rates[r];printf("BENCH single_warmgloss saved=1 rate=%u strong=%u blocks=3 cpu_ms=%.3f max_cpu_ms=%.3f wall_ms=%.3f budget_per_block_ms=%.3f realtime=%.4f\n",rates[r],strong,cost*1000,max*1000,wall*1000,period*1000,cost/(3*period));fflush(stdout);CHECK(max<period*.6);a->destroy(d);
 }
 free(x);puts("PASS native single warmgloss CPU headroom (<60% block budget), no real PCM/profile/playback/volume operations");return 0;
}
static int run(int argc,char **argv,char **env){
 (void)argc;(void)argv;(void)env;setpriority(PRIO_PROCESS,0,19);
 const char *path=getenv("DSP_CANDIDATE");void *h=dlopen(path,2);CHECK(h&&module_cache_sync(path));const DiscDspApi *(*get)(void)=dlsym(h,"disc_dsp_get_api");ChainCreate create=dlsym(h,"disc_dsp_create_chain");CHECK(get&&create);
 if(getenv("DISC_OLD_COMPARE")){
  const char *old=getenv("DSP_BASELINE");void *base=dlopen(old,2);CHECK(base&&module_cache_sync(old));const DiscDspApi *(*getbase)(void)=dlsym(base,"disc_dsp_get_api");ChainCreate cb=dlsym(base,"disc_dsp_create_chain");CHECK(getbase&&cb);CHECK(!compare(getbase(),cb,get(),create));puts("PASS regression against installed DSP1.14.0 baseline");
  unsigned rates[]={8000,44100,48000,96000,192000,384000},rng=1150;int x[1024],y[1024];
  const float prior[12]={.45f,.55f,1.8f,-1.5f,160,-1,0,0,0,0,0,0};
  for(unsigned r=0;r<6;r++){DiscChain c;wg_spec(&c);memcpy(c.nodes[0].params,prior,sizeof prior);void *a=cb(rates[r],&c),*b=create(rates[r],&c);CHECK(a&&b);for(unsigned k=0;k<32;k++){input(x,512,&rng,k&1);memcpy(y,x,sizeof x);getbase()->process(a,x,512);get()->process(b,y,512);CHECK(!memcmp(x,y,sizeof x));}getbase()->destroy(a);get()->destroy(b);}
  puts("PASS old six-control type16 with zero sag byte-identical across six rates");
 }
 CHECK(!(getenv("DISC_BENCH_ONLY")?warm_bench(get(),create):warm_test(get(),create)));
 puts("PASS isolated warmgloss verification; one enabled effect at a time");return 0;
}
int __libc_start_main(int (*fn)(int,char **,char **),int argc,char **argv,void (*init)(void),void (*fini)(void),void (*rtld)(void),void *stack){(void)fn;int (*next)(int (*)(int,char **,char **),int,char **,void (*)(void),void (*)(void),void (*)(void),void *)=dlsym((void *)-1,"__libc_start_main");return next?next(run,argc,argv,init,fini,rtld,stack):90;}
