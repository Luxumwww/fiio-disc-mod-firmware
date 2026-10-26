#ifndef DISC_DSP_PROFILE_FORMAT_H
#define DISC_DSP_PROFILE_FORMAT_H
#include "parameters.h"
/* Version 1 had eight effect rows. Missing new rows inherit defaults; the
 * retired DX8 row stays reserved. Commit only a complete, validated profile. */
static inline int disc_profile_parse(const char *text,float out[12][12]){
 if(!text||(text[0]!='1'&&text[0]!='2')||(text[1]!=' '&&text[1]!='\n'))return 0;
 unsigned rows=text[0]=='1'?8:12;float next[12][12];
 for(unsigned i=0;i<12;i++)for(unsigned j=0;j<12;j++)next[i][j]=disc_param_default[i][j];
 const char *at=text+1;char *end;
 for(unsigned i=0;i<rows;i++)for(unsigned j=0;j<12;j++){
  next[i][j]=strtof(at,&end);if(end==at)return 0;at=end;
 }
 while(*at==' '||*at=='\t'||*at=='\r'||*at=='\n')at++;
 if(*at||!disc_params_valid(next))return 0;
 for(unsigned i=0;i<12;i++)for(unsigned j=0;j<12;j++)out[i][j]=next[i][j];return 1;
}
#endif
