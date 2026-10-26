/* Actual renderer, original 32-bit ABI, no audio device or display access. */
#include "../audio-visuals/xy/xy.c"
#include <stdio.h>
#include <assert.h>
static short input[1024][2];
static unsigned lit_count;
static long sum_x,sum_y;
static int min_x,max_x,min_y,max_y;
static void measure(void){
 lit_count=0;sum_x=sum_y=0;min_x=min_y=360;max_x=max_y=-360;
 for(int y=0;y<360;y++)for(int x=0;x<360;x++)if(((canvas[y*360+x]>>8)&255)>200){
  int dx=x-180,dy=y-180;assert(dx*dx+dy*dy<172*172);
  if(dx<min_x)min_x=dx;if(dx>max_x)max_x=dx;
  if(dy<min_y)min_y=dy;if(dy>max_y)max_y=dy;
  sum_x+=dx;sum_y+=dy;lit_count++;
 }
 assert(lit_count);
}
int main(void){
 assert(sizeof(DiscXyApi)==32&&sizeof(DiscVisualImage)==12);
 for(int kind=0;kind<6;kind++){
  reset();
  for(int i=0;i<1024;i++){
   int l=kind<2?-32768+i*64:(kind<4?20000:-20000);
   int r=kind==0?l:kind==1?-l:0;if(r>32767)r=32767;
   if(kind==3||kind==5){r=l;l=0;}
   input[i][0]=l;input[i][1]=r;
  }
  feed(input,1024,768000,1000);tick(1000);measure();
  if(kind==0)assert(min_x>=-1&&max_x<=1&&max_y-min_y>180);
  else if(kind==1)assert(min_y>=-1&&max_y<=1&&max_x-min_x>180);
  else{
   assert((sum_x<0)==(kind==2||kind==5));
   assert((sum_y<0)==(kind<4));
  }
 }
 puts("PASS actual MIPS32 renderer: mono vertical, opposite polarity horizontal, positive/negative L/R direction, 768k acceptance, circular fit");fflush(stdout);return 0;
}
__asm__(".text\n.set noreorder\n.globl _start\n_start:\naddiu $sp,$sp,-32\njal main\nnop\nmove $a0,$v0\nli $v0,4001\nsyscall\n");
