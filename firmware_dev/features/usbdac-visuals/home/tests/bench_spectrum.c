/* Isolated arithmetic benchmark. No audio/USB/device handles or settings.
 * Clock stamps for analysis scheduling are synthetic 40ms steps; measured
 * wall/CPU clocks are real. Run once at nice19, 16 warm-ups +100 measures. */
#include <math.h>
#include <stdio.h>
#include <time.h>
#include <sys/resource.h>
#include "../spectrum.h"

static DiscDacHomeSpectrum state;
static short pcm[DISC_DAC_HOME_SPECTRUM_SAMPLES][2];
static volatile unsigned sink;
static double seconds(const struct timespec *value){return value->tv_sec+value->tv_nsec*1.0e-9;}
static void prepare_pcm(void){
 unsigned random=180;
 for(unsigned i=0;i<DISC_DAC_HOME_SPECTRUM_SAMPLES;i++){
  random=random*1664525u+1013904223u;
  float noise=((int)(random>>16)-32768)*(0.04f/32768.0f);
  double phase=6.2831853071795864769*i/48000.0;
  float left=0.39f*sinf((float)(phase*79))+0.28f*sinf((float)(phase*1300))+0.12f*sinf((float)(phase*11500))+noise;
  float right=0.31f*sinf((float)(phase*127))-0.25f*sinf((float)(phase*1300))+0.17f*sinf((float)(phase*7300))-noise;
  pcm[i][0]=(short)(left*32767);pcm[i][1]=(short)(right*32767);
 }
}
static int once(unsigned stamp){
 disc_dac_home_spectrum_gate(&state,1,DISC_DAC_HOME_SPECTRUM_PCM,stamp);
 if(!disc_dac_home_spectrum_process(&state,pcm,DISC_DAC_HOME_SPECTRUM_SAMPLES,48000,stamp))return 0;
 disc_dac_home_spectrum_tick(&state,stamp);
 for(unsigned i=0;i<DISC_DAC_HOME_SPECTRUM_BANDS;i++)sink+=state.height[i]+disc_dac_home_spectrum_bar_x(i);
 return 1;
}
int main(void){
 int nice_result=setpriority(PRIO_PROCESS,0,19);
 prepare_pcm();disc_dac_home_spectrum_reset(&state);
 unsigned stamp=1000;
 for(unsigned i=0;i<16;i++,stamp+=40)if(!once(stamp))return 2;
 double cpu_sum=0,cpu_max=0,wall_sum=0,wall_max=0;
 for(unsigned i=0;i<100;i++,stamp+=40){
  struct timespec wall_start,cpu_start,cpu_end,wall_end;
  if(clock_gettime(CLOCK_MONOTONIC,&wall_start)||clock_gettime(CLOCK_THREAD_CPUTIME_ID,&cpu_start))return 3;
  if(!once(stamp))return 4;
  if(clock_gettime(CLOCK_THREAD_CPUTIME_ID,&cpu_end)||clock_gettime(CLOCK_MONOTONIC,&wall_end))return 5;
  double cpu=seconds(&cpu_end)-seconds(&cpu_start),wall=seconds(&wall_end)-seconds(&wall_start);
  if(cpu<0||wall<0)return 6;cpu_sum+=cpu;wall_sum+=wall;if(cpu>cpu_max)cpu_max=cpu;if(wall>wall_max)wall_max=wall;
 }
 unsigned analyses=state.analyses;
 disc_dac_home_spectrum_gate(&state,0,DISC_DAC_HOME_SPECTRUM_PCM,stamp);
 if(disc_dac_home_spectrum_process(&state,pcm,4096,48000,stamp)||state.analyses!=analyses)return 7;
 disc_dac_home_spectrum_gate(&state,1,DISC_DAC_HOME_SPECTRUM_DOP,stamp);
 if(disc_dac_home_spectrum_process(&state,pcm,4096,48000,stamp)||state.analyses!=analyses)return 8;
 printf("{\"benchmark\":\"dac_home_spectrum_gate_process_tick\",\"effective_rate\":48000,\"samples\":4096,\"bands\":14,\"warmup\":16,\"iterations\":100,\"analyses\":%u,\"nice19_result\":%d,\"cpu_avg_ms\":%.6f,\"cpu_max_ms\":%.6f,\"wall_avg_ms\":%.6f,\"wall_max_ms\":%.6f,\"cpu_avg_percent_of_40ms\":%.6f,\"sink\":%u,\"input\":\"synthetic_pcm_only\",\"audio_opened\":false,\"settings_written\":false,\"drawing_included\":false}\n",
        analyses,nice_result,cpu_sum*10.0,cpu_max*1000.0,wall_sum*10.0,wall_max*1000.0,cpu_sum*25.0,sink);
 fflush(stdout);return 0;
}

#ifdef DISC_DAC_HOME_STANDALONE_MIPS
/* Match the already used probe executable entry ABI, with no parent process
 * interposition. Dynamic loader initializes libc; exit only this process. */
__asm__(".text\n.set noreorder\n.globl _start\n_start:\naddiu $sp,$sp,-32\njal main\nnop\nmove $a0,$v0\nli $v0,4001\nsyscall\n");
#endif
