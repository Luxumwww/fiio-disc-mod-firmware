#include <assert.h>
#include <math.h>
#include <stdio.h>
#include "mandala_api.h"
float disc_mandala_test_power(unsigned,unsigned);
void disc_mandala_test_density(const float *);
unsigned disc_mandala_test_sequence(void);
int main(void){
 const DiscMandalaApi *api=disc_mandala_get_api(1);assert(api&&!disc_mandala_get_api(2));api->reset();
 float density[4096];for(unsigned y=0;y<64;y++)for(unsigned x=0;x<64;x++)density[y*64+x]=1+.5f*cosf(6.283185307f*(3*x+5*y)/64);
 disc_mandala_test_density(density);assert(fabsf(disc_mandala_test_power(3,5)-1024)<.03f);assert(fabsf(disc_mandala_test_power(61,59)-1024)<.03f);assert(disc_mandala_test_power(4,5)<.03f);assert(disc_mandala_test_power(0,0)<.03f);
 short pcm[960][2];for(unsigned i=0;i<960;i++){pcm[i][0]=(short)(29000*sinf(i*.055f));pcm[i][1]=(short)(27000*sinf(i*.089f+.8f));}
 api->reset();api->feed(pcm,960,48000,1000);api->tick(1000);assert(disc_mandala_test_sequence()==1);
 for(unsigned t=1016;t<1120;t+=16)api->tick(t);
 const DiscVisualImage *image=api->compose(360,0);assert(image&&image->size==518400);
 unsigned lit=0;const unsigned *pixels=image->data;for(unsigned i=0;i<129600;i++)if(pixels[i]!=0xff050a10u)lit++;assert(lit>20000);
 FILE *file=fopen("preview.ppm","wb");assert(file);fprintf(file,"P6\n360 360\n255\n");for(unsigned i=0;i<129600;i++){unsigned c=pixels[i];unsigned char p[3]={c>>16,c>>8,c};assert(fwrite(p,1,3,file)==3);}fclose(file);
 unsigned n,r,age;api->stats(&n,&r,&age,1120);assert(n==960&&r==48000&&age==120);
 unsigned background[129600];for(unsigned i=0;i<129600;i++)background[i]=0xff123456;const DiscVisualImage *mixed=api->compose(0,background);assert(((const unsigned *)mixed->data)[100]==0xff123456);
 for(unsigned t=1300;t<7000;t+=16)api->tick(t);image=api->compose(360,0);pixels=image->data;for(unsigned i=0;i<129600;i++)assert(pixels[i]==0xff050a10u);
 puts("PASS 2D Fourier plane, conjugate symmetry, DC removal, audio-driven image, stale decay, horizontal composition, ABI gate");
}
