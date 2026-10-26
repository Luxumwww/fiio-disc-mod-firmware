#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "whitepoint_config.h"
int main(void){
 char text[64];int value=-1;
 for(int i=0;i<=80;i++){
  int n=snprintf(text,sizeof text,"version=1\nreduction=%d\n",i);
  assert(ds_parse_config(text,(unsigned)n,&value)&&value==i);
  assert(ds_parse_config(text,(unsigned)n-1,&value)&&value==i);
  for(int cut=0;cut<n-2;cut++)assert(!ds_parse_config(text,(unsigned)cut,&value)||cut==n-2);
 }
 const char *bad[]={"","version=2\nreduction=30\n","version=1\nreduction=-1\n","version=1\nreduction=81\n","version=1\nreduction=99999999999\n","version=1\nreduction=10\nx=1\n","version=1\nreduction=\n","version=1\nreduction=30\n\n"};
 for(unsigned i=0;i<sizeof bad/sizeof *bad;i++)assert(!ds_parse_config(bad[i],(unsigned)strlen(bad[i]),&value));
 puts("PASS: all 81 reductions, strict range/schema, truncation and malformed data");
}
