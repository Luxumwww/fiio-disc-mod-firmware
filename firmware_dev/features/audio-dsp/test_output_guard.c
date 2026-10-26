#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <assert.h>
#include "output_guard.h"
int main(void){
 const unsigned rates[]={8000,44100,48000,96000,176400,192000,384000,768000};
 for(unsigned ri=0;ri<8;ri++){
  unsigned rate=rates[ri];OutputGuard *g=malloc(sizeof(*g));assert(g);output_guard_init(g,rate);
  // Smooth steady signal interrupted by one isolated high transient.
  float previous=1;double largest=0;for(unsigned n=0;n<g->delay*4+1;n++){
   float io[2]={.4f,-.2f};if(n==g->delay*2)io[0]=4,io[1]=-2;
   output_guard_process(g,io);assert(fabsf(io[0])<=.999991f&&io[1]==-.5f*io[0]);
   double jump=fabsf(g->gain-previous);if(jump>largest)largest=jump;previous=g->gain;
  }
  // Old attack jumps .75 in one sample. New bound is .75/window.
  assert(largest<.751/g->window);
  printf("rate=%u latency_frames=%u isolated_peak_gain_step=%.9f old_step=.75\n",rate,g->delay,largest);
  output_guard_reset(g);unsigned rng=7;
  // Single-frame overloads, opposing stereo peaks, changing signs and levels.
  for(unsigned n=0;n<rate*2;n++){
   rng=rng*1664525+1013904223;float x=((int)(rng>>8)-8388608)*(20.f/8388608);
   rng=rng*1664525+1013904223;float y=((int)(rng>>8)-8388608)*(20.f/8388608);
   if(n%193<150){x*=.01f;y*=.01f;}float io[2]={x,y};output_guard_process(g,io);
   assert(isfinite(io[0])&&isfinite(io[1])&&fabsf(io[0])<1&&fabsf(io[1])<1);
  }
  // Recovery must actually reach unity, not stall in single-precision.
  for(unsigned n=0;n<rate*4;n++){float io[2]={.2f,-.1f};output_guard_process(g,io);}assert(g->gain==1);
  output_guard_reset(g);for(unsigned n=0;n<g->delay+1;n++){float io[2]={.4f,-.2f};output_guard_process(g,io);if(n<g->delay)assert(!io[0]&&!io[1]);else assert(io[0]==.4f&&io[1]==-.2f);}
  unsigned invalid=g->invalid;float io[2]={NAN,INFINITY};output_guard_process(g,io);assert(g->invalid==invalid+2);
  output_guard_reset(g);for(unsigned n=0;n<g->delay+1;n++){io[0]=io[1]=0;output_guard_process(g,io);assert(!io[0]&&!io[1]);}free(g);
 }
 puts("PASS: 8 rates, smooth attack, arbitrary stereo peaks bounded, exact delay/unity, full recovery, reset, invalid values");return 0;
}
