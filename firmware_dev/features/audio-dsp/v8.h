/* Independent algorithmic reverb, not official BASS code or an exact clone.
 * Eight damped delay lines with orthogonal Householder feedback and two
 * Schroeder output diffusers per channel. No FFT, resampling or block latency.
 * Reference target: BASS default DX8 ~0.88s broad-band tail and dark late tail. */
typedef struct {float *buffer,low,gain;unsigned length,pos;} V8Line;
typedef struct {V8Line line[8],allpass[4];float pole,wet,diffusion,width,dry;unsigned bytes;} V8;
static void v8_destroy(V8 *v){if(!v)return;for(unsigned i=0;i<8;i++)free(v->line[i].buffer);for(unsigned i=0;i<4;i++)free(v->allpass[i].buffer);free(v);}
static V8 *v8_create(unsigned rate){
 static const unsigned lengths[8]={1559,1871,2137,2393,2689,2903,3163,3347},diffusers[4]={131,359,149,373};
 V8 *v=calloc(1,sizeof(*v));if(!v)return 0;v->pole=expf(-6.28318530718f*6000/rate);v->wet=2.04f;v->diffusion=.55f;v->width=1;
 for(unsigned i=0;i<12;i++){V8Line *d=i<8?&v->line[i]:&v->allpass[i-8];unsigned base=i<8?lengths[i]:diffusers[i-8];d->length=(unsigned)((double)base*rate/48000+.5);if(d->length<1)d->length=1;
  d->buffer=calloc(d->length,sizeof(float));if(!d->buffer){v8_destroy(v);return 0;}v->bytes+=d->length*sizeof(float);d->gain=powf(10.f,-3.f*d->length/(rate*1.1f));if(i>=8)d->low=.55f;
 }return v;
}
static void v8_reset(V8 *v){for(unsigned i=0;i<12;i++){V8Line *d=i<8?&v->line[i]:&v->allpass[i-8];memset(d->buffer,0,d->length*sizeof(float));d->pos=0;d->low=i<8?0:v->diffusion;}}
static float v8_allpass(V8Line *d,float x){float z=d->buffer[d->pos],y=z-d->low*x;d->buffer[d->pos]=x+d->low*y;if(++d->pos==d->length)d->pos=0;return y;}
static void v8_configure(V8 *v,unsigned rate,const float *p){v->pole=expf(-6.28318530718f*p[1]/rate);v->diffusion=p[2];v->width=p[3];v->dry=p[4];v->wet=p[5];for(unsigned i=0;i<4;i++)v->allpass[i].low=p[2];for(unsigned i=0;i<8;i++)v->line[i].gain=powf(10.f,-3.f*v->line[i].length/(rate*p[0]));}
static void v8_sample(V8 *v,float io[2]){
 float dry_l=io[0],dry_r=io[1];
 static const float in_l[8]={1,1,1,1,-1,-1,-1,-1},in_r[8]={1,-1,1,-1,1,-1,1,-1};
 static const float out_l[8]={1,1,-1,-1,1,1,-1,-1},out_r[8]={1,-1,-1,1,-1,1,1,-1};
 float value[8],sum=0,left=0,right=0;
#ifdef __clang__
#pragma clang loop unroll(full)
#endif
 for(unsigned i=0;i<8;i++){V8Line *d=&v->line[i];value[i]=d->buffer[d->pos];sum+=value[i];left+=value[i]*out_l[i];right+=value[i]*out_r[i];}
 sum*=.25f;float l=io[0]*.35355339059f,r=io[1]*.35355339059f;
#ifdef __clang__
#pragma clang loop unroll(full)
#endif
 for(unsigned i=0;i<8;i++){V8Line *d=&v->line[i];float feedback=value[i]-sum;d->low=(1-v->pole)*feedback+v->pole*d->low;if(fabsf(d->low)<1e-20f)d->low=0;
  d->buffer[d->pos]=l*in_l[i]+r*in_r[i]+d->gain*d->low;if(++d->pos==d->length)d->pos=0;
 }
 left*=.35355339059f*v->wet;right*=.35355339059f*v->wet;
 io[0]=v8_allpass(&v->allpass[1],v8_allpass(&v->allpass[0],left));io[1]=v8_allpass(&v->allpass[3],v8_allpass(&v->allpass[2],right));
 if(v->width!=1){float mid=.5f*(io[0]+io[1]),side=.5f*(io[0]-io[1])*v->width;io[0]=mid+side;io[1]=mid-side;}
 if(v->dry){io[0]+=dry_l*v->dry;io[1]+=dry_r*v->dry;}
}
