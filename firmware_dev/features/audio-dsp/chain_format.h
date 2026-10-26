#ifndef DISC_DSP_CHAIN_FORMAT_H
#define DISC_DSP_CHAIN_FORMAT_H
#include "parameters.h"
#include "haas_params.h"
#include "ffhaas_params.h"
#include "stereofx_params.h"
#include "warmgloss_params.h"
#define DISC_CHAIN_MAX 16
#define DISC_CHAIN_ACTIVE_MAX 8
#define DISC_DSP_CUSTOM 4096u
typedef struct {unsigned id,type,enabled;float params[12];} DiscChainNode;
typedef struct {unsigned count;DiscChainNode nodes[DISC_CHAIN_MAX];} DiscChain;
static const float disc_chain_eq_default[12]={1000,0,0.70710678f,0,0,0,0,0,0,0,0,0};
static inline int disc_chain_node_valid(const DiscChainNode *n){
 if(!n->id||n->id>2147483647u||n->type>DISC_WARMGLOSS_TYPE||n->type==1||n->enabled>1)return 0;
 if(n->type==DISC_WARMGLOSS_TYPE)return disc_warmgloss_params_valid(n->params);
 if(n->type==DISC_STEREOFX_TYPE)return disc_stereofx_params_valid(n->params);
 if(n->type==DISC_FFHAAS_TYPE)return disc_ffhaas_params_valid(n->params);
 if(n->type==DISC_HAAS_TYPE)return disc_haas_params_valid(n->params);
 for(unsigned j=0;j<12;j++){
  float x=n->params[j];if(!isfinite(x))return 0;
  if(n->type<12){if(x<disc_param_range[n->type][j][0]||x>disc_param_range[n->type][j][1])return 0;}
  else if((j==0&&(x<20||x>20000))||(j==1&&(x<-24||x>12))||(j==2&&(x<.1f||x>20))||(j==3&&(x<0||x>4||x!=(float)(unsigned)x))||(j>3&&x!=0))return 0;
 }return 1;
}
static inline unsigned disc_chain_enabled_count(const DiscChain *s){unsigned n=0;for(unsigned i=0;i<s->count;i++)n+=s->nodes[i].enabled;return n;}
static inline int disc_chain_valid(const DiscChain *s){
 if(!s||s->count>DISC_CHAIN_MAX||disc_chain_enabled_count(s)>DISC_CHAIN_ACTIVE_MAX)return 0;
 for(unsigned i=0;i<s->count;i++){if(!disc_chain_node_valid(&s->nodes[i]))return 0;for(unsigned j=0;j<i;j++)if(s->nodes[i].id==s->nodes[j].id)return 0;}return 1;
}
static inline int disc_chain_uint(const char **p,unsigned *v){
 while(**p==' '||**p=='\n'||**p=='\r'||**p=='\t')(*p)++;
 if(**p<'0'||**p>'9')return 0;unsigned x=0;
 do{unsigned d=(unsigned)(*(*p)++-'0');if(x>214748364u||(x==214748364u&&d>7))return 0;x=x*10+d;}while(**p>='0'&&**p<='9');
 if(**p&&**p!=' '&&**p!='\n'&&**p!='\r'&&**p!='\t')return 0;*v=x;return 1;
}
static inline int disc_chain_parse(const char *text,DiscChain *out){
 const char *at=text;unsigned version;DiscChain next={0};
 if(!disc_chain_uint(&at,&version)||version!=1||!disc_chain_uint(&at,&next.count)||next.count>DISC_CHAIN_MAX)return 0;
 for(unsigned i=0;i<next.count;i++){
  DiscChainNode *n=&next.nodes[i];if(!disc_chain_uint(&at,&n->id)||!disc_chain_uint(&at,&n->type)||!disc_chain_uint(&at,&n->enabled))return 0;
  for(unsigned j=0;j<12;j++){char *end;n->params[j]=strtof(at,&end);if(end==at||(*end&&*end!=' '&&*end!='\n'&&*end!='\r'&&*end!='\t'))return 0;at=end;}
 }
 while(*at==' '||*at=='\n'||*at=='\r'||*at=='\t')at++;
 if(*at||!disc_chain_valid(&next))return 0;*out=next;return 1;
}
static inline unsigned disc_chain_hash(const char *text){unsigned hash=2166136261u;while(*text)hash=(hash^(unsigned char)*text++)*16777619u;return hash;}
#endif
