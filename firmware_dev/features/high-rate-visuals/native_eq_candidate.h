/* Experimental MSA stereo equivalent of pinned stock45b960, not installed.
 * The factory remains responsible for coefficients, pre-gain and EQ settings.
 * Retain every double operation/order, overflow reset and MIPS S32 conversion. */
#include <stdint.h>
#include <math.h>
#ifdef EQ_DEBUG
#include <stdio.h>
#endif
typedef double NativeEqV2d __attribute__((vector_size(16)));
typedef uint64_t NativeEqV2u __attribute__((vector_size(16)));
void native_eq_candidate(int *pcm,unsigned frames,double *buffer,const unsigned *active,const float coefficients[10][5],double left[10][5],double right[10][5]){
#pragma clang fp contract(off)
 for(unsigned i=0;i<frames*2;i++)buffer[i]=pcm[i];
 for(unsigned band=0;band<10;band++)if(active[band]){
  NativeEqV2d b0={coefficients[band][0],coefficients[band][0]},b1={coefficients[band][1],coefficients[band][1]},b2={coefficients[band][2],coefficients[band][2]},b3={coefficients[band][3],coefficients[band][3]},b4={coefficients[band][4],coefficients[band][4]};
  NativeEqV2d x0={left[band][0],right[band][0]},x1={left[band][1],right[band][1]},x2={left[band][2],right[band][2]},y1={left[band][3],right[band][3]},y2={left[band][4],right[band][4]},zero={0,0};
  NativeEqV2d *v=(void *)buffer;
  for(unsigned i=0;i<frames;i++){
   NativeEqV2d x=v[i],a=b0*x;a=a+zero;NativeEqV2d b=b1*x1;b=b+a;a=b2*x2;b=a+b;a=b3*y1;a=a+b;NativeEqV2d y=b4*y2;y=y+a;
   x0=x;x2=x1;x1=x;y2=y1;y1=y;
   if(!isfinite(y[0])){x0[0]=x1[0]=x2[0]=y1[0]=y2[0]=0;}
   if(!isfinite(y[1])){x0[1]=x1[1]=x2[1]=y1[1]=y2[1]=0;}
   v[i]=y1;
  }
  NativeEqV2d states[5]={x0,x1,x2,y1,y2};for(unsigned i=0;i<5;i++){left[band][i]=states[i][0];right[band][i]=states[i][1];}
 }
 for(unsigned i=0;i<frames*2;i++){
  double v;__asm__ volatile("trunc.w.d %0,%1":"=f"(v):"f"(buffer[i]));int x;__asm__ volatile("mfc1 %0,%1":"=r"(x):"f"(v));pcm[i]=x;
 }
}
