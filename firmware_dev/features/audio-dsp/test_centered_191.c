#define main legacy_checks_190
#include "test_haas_190.c"
#undef main
static unsigned random_state=191;
static float sample(void){random_state=random_state*1664525u+1013904223u;return (int)random_state*(.8f/2147483648.f);}
static void centered(unsigned rate){
 for(unsigned bass=0;bass<2;bass++)for(unsigned strength=0;strength<2;strength++){
  float p[12];memcpy(p,disc_haas_default,sizeof(p));p[0]=20;p[1]=strength?1:.35f;p[3]=bass?500:0;
  DiscHaas *mono=disc_haas_create(rate,p),*a=disc_haas_create(rate,p),*b=disc_haas_create(rate,p);assert(mono&&a&&b);
  for(unsigned i=0;i<rate/4;i++){
   float dry=sample(),m[2]={dry,dry};disc_haas_sample(mono,m);assert(m[0]==dry&&m[1]==dry);
   float l=sample(),r=sample(),x[2]={l,r},y[2]={r,l};disc_haas_sample(a,x);disc_haas_sample(b,y);
   assert(fabsf(x[0]-y[1])<1e-6f&&fabsf(x[1]-y[0])<1e-6f);
   assert(fabsf((x[0]+x[1])-(l+r))<5e-7f);
  }disc_haas_destroy(mono);disc_haas_destroy(a);disc_haas_destroy(b);
 }
 DiscChain c=spec();Dsp *a=disc_dsp_create_chain(rate,&c),*b=disc_dsp_create_chain(rate,&c);assert(a&&b);int x[1024],y[1024];
 for(unsigned k=0;k<64;k++){
  for(unsigned i=0;i<512;i++){int l=(int)(sample()*1000000000.f),r=(int)(sample()*1000000000.f);x[2*i]=y[2*i+1]=l;x[2*i+1]=y[2*i]=r;}
  process(a,x,512);process(b,y,512);for(unsigned i=0;i<512;i++)assert(x[2*i]==y[2*i+1]&&x[2*i+1]==y[2*i]);
 }destroy(a);destroy(b);a=disc_dsp_create_chain(rate,&c);assert(a);
 for(unsigned k=0;k<64;k++){for(unsigned i=0;i<512;i++)x[2*i]=x[2*i+1]=(int)(sample()*2000000000.f);process(a,x,512);for(unsigned i=0;i<512;i++)assert(x[2*i]==x[2*i+1]);assert(!a->clipped&&!a->guard.invalid);}destroy(a);
}
int main(void){assert(disc_haas_default[2]==2);assert(!legacy_checks_190());unsigned rates[]={8000,44100,48000,96000,192000,384000};for(unsigned r=0;r<6;r++)centered(rates[r]);puts("PASS centered differential: exact mono channel equality, raw mid preservation, swap symmetry including complete chain/guard at six rates; default mode2");return 0;}
