/* Independent PEQ instances. Double coefficients/state preserve stability
 * at low frequency and high Q with 192/384kHz streams. No per-sample trig. */
typedef struct {double b0,b1,b2,a1,a2,z1[2],z2[2];} DiscPeq;
static void chain_peq_init(DiscPeq *f,unsigned rate,const float *p){
 double hz=p[0];if(hz>rate*.45)hz=rate*.45;
 double w=6.283185307179586*hz/rate,c=cos(w),s=sin(w),alpha=s/(2*p[2]),A=pow(10,p[1]/40.0),root=sqrt(A),b0,b1,b2,a0,a1,a2;
 unsigned kind=(unsigned)p[3];
 if(kind==0){b0=1+alpha*A;b1=-2*c;b2=1-alpha*A;a0=1+alpha/A;a1=-2*c;a2=1-alpha/A;}
 else if(kind==3){b0=(1-c)/2;b1=1-c;b2=b0;a0=1+alpha;a1=-2*c;a2=1-alpha;}
 else if(kind==4){b0=(1+c)/2;b1=-(1+c);b2=b0;a0=1+alpha;a1=-2*c;a2=1-alpha;}
 else {
  /* Shelf slope is fixed at 1; UI disables Q for shelf kinds. */
  alpha=s*0.7071067811865476;
  if(kind==1){b0=A*((A+1)-(A-1)*c+2*root*alpha);b1=2*A*((A-1)-(A+1)*c);b2=A*((A+1)-(A-1)*c-2*root*alpha);a0=(A+1)+(A-1)*c+2*root*alpha;a1=-2*((A-1)+(A+1)*c);a2=(A+1)+(A-1)*c-2*root*alpha;}
  else {b0=A*((A+1)+(A-1)*c+2*root*alpha);b1=-2*A*((A-1)+(A+1)*c);b2=A*((A+1)+(A-1)*c-2*root*alpha);a0=(A+1)-(A-1)*c+2*root*alpha;a1=2*((A-1)-(A+1)*c);a2=(A+1)-(A-1)*c-2*root*alpha;}
 }
 f->b0=b0/a0;f->b1=b1/a0;f->b2=b2/a0;f->a1=a1/a0;f->a2=a2/a0;
}
static void chain_peq_reset(DiscPeq *f){f->z1[0]=f->z1[1]=f->z2[0]=f->z2[1]=0;}
static void chain_peq_sample(DiscPeq *f,float *io){for(unsigned c=0;c<2;c++){double x=io[c],y=f->b0*x+f->z1[c];f->z1[c]=f->b1*x-f->a1*y+f->z2[c];f->z2[c]=f->b2*x-f->a2*y;io[c]=(float)y;}}
