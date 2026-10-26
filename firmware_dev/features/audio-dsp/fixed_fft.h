/* Block-floating Q31 transform on the X2000 MSA integer lanes. The exponent
 * follows each block; output remains FP32 and the PCM rate is unchanged. */
#ifdef DISC_DSP_FIXED_FFT
typedef struct {int r,i;} ICx;
static void fft_fixed(Conv *v,Cx *a,int inverse){
 unsigned peak=0,bits=0;for(unsigned k=v->n;k>1;k>>=1)bits++;
 for(unsigned i=0;i<v->n*2;i++){unsigned x=((unsigned *)(void *)a)[i]&0x7fffffff;if(x>peak)peak=x;}
 int exponent=peak?(int)(peak>>23)-127+3:0;if(exponent< -90)exponent=-90;
 float input_scale=ldexpf(1.f,31-exponent);
 for(unsigned i=0;i<v->n*2;i+=4){v4f32 f=(v4f32)__msa_ld_w((void *)((float *)a+i),0);__msa_st_w(__msa_ftint_s_w(f*input_scale),(void *)((int *)a+i),0);}
 ICx *z=(ICx *)(void *)a;
 for(unsigned i=0;i<v->n;i++){unsigned j=v->reverse[i];if(j>i){ICx t=z[i];z[i]=z[j];z[j]=t;}}
 for(unsigned len=2;len<=v->n;len*=2){unsigned half=len/2,step=v->n/len;v4i32 magnitude={0,0,0,0};
  for(unsigned s=0;s<v->n;s+=len){unsigned j=0;
   for(;j+1<half;j+=2){
    unsigned x=s+j,y=x+half;int *w0=v->fixed_twiddle+2*j*step,*w1=v->fixed_twiddle+2*(j+1)*step;
    v4i32 wr={w0[0],w0[0],w1[0],w1[0]},wi={w0[1],w0[1],w1[1],w1[1]};if(inverse)wi=(v4i32){0,0,0,0}-wi;
    v4i32 p=__msa_ld_w(z+x,0),q=__msa_ld_w(z+y,0),swap=__builtin_shufflevector(q,q,1,0,3,2),im=__msa_mul_q_w(swap,wi),sign={-1,0,-1,0};
    im=(im^sign)-sign;v4i32 t=__msa_mul_q_w(q,wr)+im,u=p+t,k=p-t;
    __msa_st_w(u,z+x,0);__msa_st_w(k,z+y,0);magnitude|=__msa_add_a_w(u,(v4i32){0,0,0,0})|__msa_add_a_w(k,(v4i32){0,0,0,0});
   }
   for(;j<half;j++){
    unsigned x=s+j,y=x+half;ICx p=z[x],q=z[y],t=q;
    if(j){int *w=v->fixed_twiddle+2*j*step,im=inverse?-w[1]:w[1];t.r=(int)(((int64_t)q.r*w[0]-(int64_t)q.i*im)>>31);t.i=(int)(((int64_t)q.r*im+(int64_t)q.i*w[0])>>31);}
    ICx u={p.r+t.r,p.i+t.i},k={p.r-t.r,p.i-t.i};z[x]=u;z[y]=k;
    unsigned m=(unsigned)abs(u.r)|(unsigned)abs(u.i)|(unsigned)abs(k.r)|(unsigned)abs(k.i);magnitude[0]|=m;
   }
  }
  unsigned m=(unsigned)magnitude[0]|(unsigned)magnitude[1]|(unsigned)magnitude[2]|(unsigned)magnitude[3];unsigned shift=(m&0x40000000)?2:(m&0x20000000)?1:0;
  if(shift&&len<v->n){v4i32 amount={shift,shift,shift,shift};for(unsigned i=0;i<v->n*2;i+=4){v4i32 q=__msa_ld_w((int *)a+i,0);__msa_st_w(__msa_sra_w(q,amount),(int *)a+i,0);}exponent+=shift;}
 }
 float output_scale=ldexpf(1.f,exponent-31-(inverse?(int)bits:0));
 for(unsigned i=0;i<v->n*2;i+=4){v4i32 q=__msa_ld_w((int *)a+i,0);v4f32 f=__msa_ffint_s_w(q)*output_scale;__msa_st_w((v4i32)f,(float *)a+i,0);}
}
#endif
