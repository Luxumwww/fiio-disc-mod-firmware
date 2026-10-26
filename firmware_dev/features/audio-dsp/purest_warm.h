/* PurestWarm3-inspired mild asymmetric coloration. MIT upstream retained.
 * Revision: reduce cubic asymmetry strength 8x, bound coloration (not dry
 * audio), remove coloration DC. No treble roll-off or dry-path clipping.
 * Original polynomial/BIP is adapted rather than a bit-exact upstream preset. */
typedef struct {float asym,dc_alpha,dc[2],previous[2];} PurestWarm3State;
static void purest_warm_reset(PurestWarm3State *s,unsigned rate){
 memset(s,0,sizeof(*s));s->asym=.5f;s->dc_alpha=expf(-6.28318530718f*5.0f/(float)rate);
}
static PurestWarm3State *purest_warm_create(unsigned rate){PurestWarm3State *s=calloc(1,sizeof(*s));if(s)purest_warm_reset(s,rate);return s;}
static void purest_warm_sample(PurestWarm3State *s,float io[2]){
 for(unsigned ch=0;ch<2;ch++){
  float x=io[ch];
  /* Smooth bounded quartic: approximately the original x - asym^3*x^4/6
   * at ordinary levels, but never runs away after an upstream reverb peak.
   * reciprocal form also avoids overflow for unusually large finite input. */
  float a=fabsf(x),r=a>1.0f?1.0f/a:a,r2=r*r,r4=r2*r2;
  float color=-(s->asym*s->asym*s->asym/6.0f)*(a>1.0f?1.0f/(r4+.0625f):r4/(1.0f+.0625f*r4));
  float ac=color-s->previous[ch]+s->dc_alpha*s->dc[ch];
  s->previous[ch]=color;s->dc[ch]=ac;io[ch]=x+ac;
 }
}
