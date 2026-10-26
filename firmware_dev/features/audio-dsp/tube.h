/* Asymmetric tube-style coloration, not a model of a particular physical tube.
 * Strong drive, first-order antiderivative antialiasing of a piecewise-linear
 * curve, stereo-independent DC removal and a warm high-frequency roll-off.
 * All nonlinear tables are prepared outside the audio callback. */
typedef struct {
 float curve[4097];double integral[4097];
 float previous[2],dc_input[2],dc_output[2],low[2],dc_r,low_a,color,output;
} Tube;
static Tube *tube_create(unsigned rate){
 Tube *t=calloc(1,sizeof(*t));if(!t)return 0;
 const float drive=3.2f,bias=.4f,b=tanhf(bias),normal=drive*(1-b*b);
 for(unsigned i=0;i<=4096;i++){float x=(float)i/512-4;t->curve[i]=(tanhf(drive*x+bias)-b)/normal;if(i)t->integral[i]=t->integral[i-1]+(t->curve[i-1]+t->curve[i])*(1.0/1024);}
 t->dc_r=expf(-6.28318530718f*5/rate);t->low_a=1-expf(-6.28318530718f*7000/rate);t->color=.75f;t->output=1.4f;return t;
}
static void tube_configure(Tube *t,unsigned rate,const float *p){
 float b=tanhf(p[1]),normal=p[0]*(1-b*b);t->integral[0]=0;
 for(unsigned i=0;i<=4096;i++){float x=(float)i/512-4;t->curve[i]=(tanhf(p[0]*x+p[1])-b)/normal;if(i)t->integral[i]=t->integral[i-1]+(t->curve[i-1]+t->curve[i])*(1.0/1024);}
 t->low_a=1-expf(-6.28318530718f*p[2]/rate);t->color=p[3];t->output=p[4];
}
static void tube_reset(Tube *t){memset(t->previous,0,sizeof(t->previous));memset(t->dc_input,0,sizeof(t->dc_input));memset(t->dc_output,0,sizeof(t->dc_output));memset(t->low,0,sizeof(t->low));}
static float tube_curve(Tube *t,float x){
 if(x<=-4)return t->curve[0];if(x>=4)return t->curve[4096];
 float p=(x+4)*512;unsigned i=(unsigned)p;if(i>=4096)return t->curve[4096];float f=p-i;return t->curve[i]+(t->curve[i+1]-t->curve[i])*f;
}
static double tube_integral(Tube *t,float x){
 if(x<=-4)return t->integral[0]+(double)(x+4)*t->curve[0];if(x>=4)return t->integral[4096]+(double)(x-4)*t->curve[4096];
 float p=(x+4)*512;unsigned i=(unsigned)p;if(i>=4096)return t->integral[4096];double f=p-i;
 return t->integral[i]+(f*t->curve[i]+.5*f*f*(t->curve[i+1]-t->curve[i]))/512;
}
static float tube_sample(Tube *t,float x,unsigned c){
 float old=t->previous[c];
#ifdef DISC_TUBE_NO_ADAA
 float shaped=tube_curve(t,x);
#else
 float difference=x-old;
 float shaped=fabsf(difference)<.00001f?tube_curve(t,.5f*(x+old)):(float)((tube_integral(t,x)-tube_integral(t,old))/difference);
#endif
 t->previous[c]=x;
 float dc=shaped-t->dc_input[c]+t->dc_r*t->dc_output[c];t->dc_input[c]=shaped;t->dc_output[c]=dc;
 t->low[c]+=t->low_a*(dc-t->low[c]);
 // Match the ADAA half-sample phase in the internal dry component.
 return t->output*(t->color*t->low[c]+(.5f*(1-t->color))*(x+old));
}
