/* Small, stereo-linked music effects. Coefficients/allocation are prepared
 * outside the audio callback. Per-sample paths contain no transcendental math.
 * Dynamic EQ is a parallel band-pass bell, not a moving RBJ peaking filter. */
typedef struct {float b0,b2,a1,a2,z1[2],z2[2];} DiscBand;
static void creative_band(DiscBand *b,unsigned rate,float hz,float q){
 if(hz>rate*.45f)hz=rate*.45f;
 float w=6.28318530718f*hz/rate,alpha=sinf(w)/(2*q),a=1+alpha;
 b->b0=alpha/a;b->b2=-alpha/a;b->a1=-2*cosf(w)/a;b->a2=(1-alpha)/a;
}
static float creative_filter(DiscBand *b,float x,unsigned ch){
 float y=b->b0*x+b->z1[ch];b->z1[ch]=-b->a1*y+b->z2[ch];b->z2[ch]=b->b2*x-b->a2*y;return y;
}
typedef struct {float width,cross,basspole,feedpole,output,side_low,feed[2];int bass_on;} DiscStereo;
static void creative_stereo_init(DiscStereo *s,unsigned rate,const float *p){
 s->width=p[0];s->cross=p[1];s->bass_on=p[2]>0;s->basspole=expf(-6.28318530718f*p[2]/rate);
 s->feedpole=expf(-6.28318530718f*700/rate);s->output=powf(10,p[3]/20);
}
static void creative_stereo(DiscStereo *s,float *io){
 float l=io[0],r=io[1],mid=(l+r)*.5f,side=(l-r)*.5f;
 s->side_low=s->basspole*s->side_low+(1-s->basspole)*side;
 if(s->bass_on)side-=s->side_low;side*=s->width;
 s->feed[0]=s->feedpole*s->feed[0]+(1-s->feedpole)*l;
 s->feed[1]=s->feedpole*s->feed[1]+(1-s->feedpole)*r;
 io[0]=(mid+side+s->cross*s->feed[1])/(1+s->cross)*s->output;
 io[1]=(mid-side+s->cross*s->feed[0])/(1+s->cross)*s->output;
}
typedef struct {float *buffer,feedback,mix,pole,low[2];unsigned length,pos;int pingpong;} DiscDelay;
static DiscDelay *creative_delay_create(unsigned rate,const float *p){
 DiscDelay *d=calloc(1,sizeof(*d));if(!d)return 0;d->length=(unsigned)(rate*p[0]*.001f+.5f);
 if(!d->length)d->length=1;d->buffer=calloc(d->length*2,sizeof(float));if(!d->buffer){free(d);return 0;}
 d->feedback=p[1];d->mix=p[2];d->pole=expf(-6.28318530718f*p[3]/rate);d->pingpong=p[4]>=.5f;return d;
}
static void creative_delay_reset(DiscDelay *d){memset(d->buffer,0,d->length*2*sizeof(float));d->pos=0;d->low[0]=d->low[1]=0;}
static void creative_delay_destroy(DiscDelay *d){if(d){free(d->buffer);free(d);}}
static void creative_delay(DiscDelay *d,float *io){
 float dry[2]={io[0],io[1]},wet[2]={d->buffer[d->pos*2],d->buffer[d->pos*2+1]};
 for(unsigned c=0;c<2;c++)d->low[c]=d->pole*d->low[c]+(1-d->pole)*wet[c];
 for(unsigned c=0;c<2;c++){
  d->buffer[d->pos*2+c]=dry[c]+d->feedback*d->low[d->pingpong?1-c:c];
  io[c]=dry[c]*(1-d->mix)+wet[c]*d->mix;
 }
 if(++d->pos==d->length)d->pos=0;
}
typedef struct {DiscBand band;float threshold,amount,attack,release,mix,envelope,gain;int deesser;} DiscDynamic;
static void creative_dynamic_init(DiscDynamic *d,unsigned rate,const float *p,int deesser){
 creative_band(&d->band,rate,p[0],p[1]);d->threshold=powf(10,p[2]/20);d->amount=powf(10,(deesser?-p[3]:p[3])/20);
 d->attack=expf(-1/(rate*p[4]*.001f));d->release=expf(-1/(rate*p[5]*.001f));d->mix=p[6];d->gain=1;d->deesser=deesser;
}
static void creative_dynamic_reset(DiscDynamic *d){memset(d->band.z1,0,sizeof(d->band.z1));memset(d->band.z2,0,sizeof(d->band.z2));d->gain=1;d->envelope=0;}
static void creative_dynamic(DiscDynamic *d,float *io){
 float band[2]={creative_filter(&d->band,io[0],0),creative_filter(&d->band,io[1],1)};
 float peak=fabsf(band[0]);if(fabsf(band[1])>peak)peak=fabsf(band[1]);
 float a=peak>d->envelope?d->attack:d->release;d->envelope=a*d->envelope+(1-a)*peak;
 float target=1;
 if(d->envelope>d->threshold){
  if(d->deesser){target=d->threshold/d->envelope;if(target<d->amount)target=d->amount;}
  else {float activity=(d->envelope-d->threshold)/(d->envelope+d->threshold);target=1+(d->amount-1)*activity;}
 }
 a=fabsf(target-1)>fabsf(d->gain-1)?d->attack:d->release;d->gain=a*d->gain+(1-a)*target;
 for(unsigned c=0;c<2;c++)io[c]+=band[c]*(d->gain-1)*d->mix;
}
