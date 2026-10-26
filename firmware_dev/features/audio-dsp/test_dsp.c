#include <assert.h>
#include "dsp.c"
int main(int argc,char **argv){
 assert(argc==2);const char *root=argv[1];
 Dsp *d=create(48000,0,root);assert(d);int a[256],orig[256];for(int i=0;i<256;i++)a[i]=orig[i]=(i&1)?INT32_MIN+i:INT32_MAX-i;
 process(d,a,128);assert(!memcmp(a,orig,sizeof(a)));destroy(d);
 d=create(768000,1,root);assert(d&&d->hall&&d->hall->len[7]+23<FV_MAXCOMB&&d->hall->alen[0]+23<FV_MAXALLPASS);destroy(d);
 d=create(48000,2,"/path/absent");assert(d&&!(d->available&2));destroy(d);
 const unsigned rates[]={44100,48000,96000,192000,384000,768000};
 for(unsigned r=0;r<6;r++){
  d=create(rates[r],2,root);assert(d&&(d->available&2));d->mix[0]=1;
  unsigned frames=rates[r]*2;int *pcm=calloc(frames*2,sizeof(int));assert(pcm);pcm[0]=1073741824;pcm[2*137+1]=536870912;
  for(unsigned pos=0;pos<frames;){unsigned n=1+(pos*17+83)%777;if(n>frames-pos)n=frames-pos;assert(process(d,pcm+2*pos,n));pos+=n;}
  char path[512];snprintf(path,sizeof(path),"%s/../build/impulse-%u.i32",root,rates[r]);FILE *f=fopen(path,"wb");assert(f);assert(fwrite(pcm,8,frames,f)==frames);fclose(f);free(pcm);destroy(d);
 }
 d=create(48000,7,root);assert(d&&d->available==7);int *pcm=calloc(48000*2,sizeof(int));assert(pcm);for(unsigned i=0;i<48000;i++){pcm[2*i]=(i&1)?-1700000000:1700000000;pcm[2*i+1]=pcm[2*i];}process(d,pcm,48000);for(unsigned i=0;i<96000;i++)assert(pcm[i]!=INT32_MIN);free(pcm);destroy(d);
 puts("PASS: off bit-exact, 768k buffers, missing IR fallback, six-rate stereo convolution, all three effects");
}
