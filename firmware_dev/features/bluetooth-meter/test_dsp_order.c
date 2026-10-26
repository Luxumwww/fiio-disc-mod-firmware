#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "dsp_order.h"
static unsigned count;
static void permutations(unsigned a[8],unsigned n){if(n<8){for(unsigned i=n;i<8;i++){unsigned t=a[n];a[n]=a[i];a[i]=t;permutations(a,n+1);t=a[n];a[n]=a[i];a[i]=t;}return;}count++;char buf[96];snprintf(buf,sizeof(buf),"%u %u %u %u %u %u %u %u\n",a[0],a[1],a[2],a[3],a[4],a[5],a[6],a[7]);unsigned b[8];assert(dsp_order_parse(buf,b)&&!memcmp(a,b,sizeof(b)));for(unsigned k=0;k<8;k++)for(int to=-1;to<=8;to++){unsigned id=a[k],seen=0;memcpy(b,a,sizeof(b));dsp_order_move(b,id,to);for(unsigned j=0;j<8;j++){assert(b[j]!=1&&(b[j]<8||b[j]==13)&&!(seen&(1u<<b[j])));seen|=1u<<b[j];}assert(seen==8445&&b[to<0?0:to>7?7:to]==id);}}
int main(void){unsigned a[8],ref[8];dsp_order_default(a);permutations(a,0);assert(count==40320);assert(dsp_order_parse("7 1 3 0 6 2 5 4",a));unsigned expected[]={7,3,0,6,2,5,4,13};assert(!memcmp(a,expected,sizeof(a)));memcpy(ref,a,sizeof(a));assert(!dsp_order_parse("0 1 2 3 4 5 6",a)&&!memcmp(a,ref,sizeof(a)));assert(!dsp_order_parse("0 2 3 4 5 6 6",a)&&!memcmp(a,ref,sizeof(a)));assert(!dsp_order_move(a,1,3)&&!memcmp(a,ref,sizeof(a)));assert(dsp_order_slot(900)==7);puts("PASS40320 native8 permutations/moves; old relative order retained and Haas appended; removed ID rejected");}
