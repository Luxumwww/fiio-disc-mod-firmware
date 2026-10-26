#ifndef DISC_WHITEPOINT_CONFIG_H
#define DISC_WHITEPOINT_CONFIG_H
/* Bounded strict setting format. Zero is off; cap avoids an invisible UI. */
#define DS_WHITEPOINT_MAX 80
static int ds_parse_config(const char *text,unsigned length,int *value){
 static const char prefix[]="version=1\nreduction=";
 unsigned i=0;int n=0,digits=0;
 if(!text||!value||length<sizeof(prefix))return 0;
 for(;i<sizeof(prefix)-1;i++)if(text[i]!=prefix[i])return 0;
 while(i<length&&text[i]>='0'&&text[i]<='9'){
  if(++digits>2)return 0;n=n*10+text[i++]-'0';
 }
 if(!digits||n>DS_WHITEPOINT_MAX)return 0;
 if(i<length&&text[i]=='\n')i++;
 if(i!=length)return 0;*value=n;return 1;
}
#endif
