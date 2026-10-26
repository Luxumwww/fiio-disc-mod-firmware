/* Stereo-linked lookahead sample-peak limiter. Two 1 ms moving averages
 * smooth a held minimum gain. Hold covers both averages' full support:
 * delayed peaks cannot escape during recovery. No audio-loop allocation,
 * transcendental functions or scanning. Before native EQ, not true-peak. */
#define OUTPUT_GUARD_WINDOW_MAX 768
#define OUTPUT_GUARD_DELAY_MAX (2*OUTPUT_GUARD_WINDOW_MAX)
typedef struct {
 float gain,peak;unsigned limited,invalid;
 unsigned window,delay,pos,p1,p2,hold,active;
 double recovery,envelope,sum1,sum2,inverse_window;
 float audio[OUTPUT_GUARD_DELAY_MAX][2];
 float average1[OUTPUT_GUARD_WINDOW_MAX],average2[OUTPUT_GUARD_WINDOW_MAX];
} OutputGuard;
static void output_guard_reset(OutputGuard *g){
 g->gain=1;g->envelope=1;g->sum1=g->sum2=g->window;
 g->pos=g->p1=g->p2=g->hold=g->active=0;
 memset(g->audio,0,g->delay*sizeof(g->audio[0]));
 for(unsigned i=0;i<g->window;i++)g->average1[i]=g->average2[i]=1;
}
static void output_guard_init(OutputGuard *g,unsigned rate){
 memset(g,0,sizeof(*g));g->window=(rate+999)/1000;
 if(g->window<2)g->window=2;if(g->window>OUTPUT_GUARD_WINDOW_MAX)g->window=OUTPUT_GUARD_WINDOW_MAX;
 g->delay=2*(g->window-1);g->inverse_window=1.0/g->window;g->recovery=1-exp(-1.0/(rate*.150));output_guard_reset(g);
}
static void output_guard_sanitize(OutputGuard *g,float io[2]){for(unsigned c=0;c<2;c++)if(!isfinite(io[c])){io[c]=0;g->invalid++;}}
static void output_guard_process(OutputGuard *g,float io[2]){
 output_guard_sanitize(g,io);float left=fabsf(io[0]),right=fabsf(io[1]),peak=left>right?left:right;if(peak>g->peak)g->peak=peak;
 const float ceiling=.99999f;
 if(peak<=ceiling&&g->envelope==1&&g->gain==1){
  float out[2]={g->audio[g->pos][0],g->audio[g->pos][1]};
  g->audio[g->pos][0]=io[0];g->audio[g->pos][1]=io[1];if(++g->pos==g->delay)g->pos=0;
  io[0]=out[0];io[1]=out[1];g->active=1;return;
 }
 double target=peak>ceiling?ceiling/peak:1;
 if(target<1){if(target<g->envelope)g->envelope=target;g->hold=g->delay;}
 else if(g->hold)g->hold--;
 else {g->envelope+=g->recovery*(1-g->envelope);if(g->envelope>1-1e-8)g->envelope=1;}
 float control=(float)g->envelope;
 g->sum1+=(double)control-g->average1[g->p1];g->average1[g->p1]=control;if(++g->p1==g->window)g->p1=0;
 float first=(float)(g->sum1*g->inverse_window);
 g->sum2+=(double)first-g->average2[g->p2];g->average2[g->p2]=first;if(++g->p2==g->window)g->p2=0;
 g->gain=(float)(g->sum2*g->inverse_window);if(g->gain>1)g->gain=1;
 if(g->gain<.999999f)g->limited++;
 float out[2]={g->audio[g->pos][0],g->audio[g->pos][1]};
 g->audio[g->pos][0]=io[0];g->audio[g->pos][1]=io[1];if(++g->pos==g->delay)g->pos=0;
 io[0]=out[0]*g->gain;io[1]=out[1]*g->gain;g->active=1;
}
