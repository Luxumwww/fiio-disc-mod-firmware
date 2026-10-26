#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "dsp_order.h"
int main(void){unsigned a[11],b[11],rng=714;dsp_order_default(a);assert(dsp_order_slot(0)==0&&dsp_order_slot(999)==10);assert(dsp_order_parse("7 1 3 0 6 2 5 4",a));unsigned expected[]={7,3,0,6,2,5,4,8,9,10,11};assert(!memcmp(a,expected,sizeof(a)));
 for(unsigned n=0;n<10000;n++){for(unsigned i=10;i;i--){rng=rng*1664525+1013904223;unsigned j=rng%(i+1),v=a[i];a[i]=a[j];a[j]=v;}char text[128];int pos=0;for(unsigned i=0;i<11;i++)pos+=snprintf(text+pos,sizeof(text)-pos,"%u ",a[i]);assert(dsp_order_parse(text,b)&&!memcmp(a,b,sizeof(a)));for(unsigned i=0;i<11;i++)for(int to=-1;to<12;to++){memcpy(b,a,sizeof(a));unsigned id=a[i];dsp_order_move(b,id,to);unsigned seen=0;for(unsigned j=0;j<11;j++){assert(b[j]<12&&b[j]!=1&&!(seen&(1u<<b[j])));seen|=1u<<b[j];}assert(seen==4093&&b[to<0?0:to>10?10:to]==id);}}
 memcpy(b,a,sizeof(a));assert(!dsp_order_parse("0 2 3 4 5 6 7 8 9 10 10",a)&&!memcmp(a,b,sizeof(a)));assert(!dsp_order_parse("999999999999999999999999",a));assert(!dsp_order_move(a,1,4));puts("PASS10000 order/move cases; legacy7/8 relative order, append4, multi-digit IDs, invalid input rejection");return 0;}
