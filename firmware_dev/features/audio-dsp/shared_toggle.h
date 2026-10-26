#ifndef DISC_SHARED_TOGGLE_H
#define DISC_SHARED_TOGGLE_H
#include "shared_profile.h"
/* Native rows toggle every instance of a type without replacing its parameters. */
/* 1=invalid/config IO,2=saved capacity,3=active capacity; preflight is read-only. */
static inline unsigned disc_shared_toggle_error(const DiscChain *c,unsigned enabled,unsigned type){
 if(!c||enabled>1||!disc_chain_valid(c)||type==1||(type>=12&&type!=DISC_HAAS_TYPE&&type!=DISC_FFHAAS_TYPE&&type!=DISC_STEREOFX_TYPE&&type!=DISC_WARMGLOSS_TYPE))return 1;
 unsigned present=0,on=0,instances=0,other=0;
 for(unsigned i=0;i<c->count;i++){if(c->nodes[i].type==type){present=1;instances++;on|=c->nodes[i].enabled;}else other+=c->nodes[i].enabled;}
 if(!present&&c->count>=DISC_CHAIN_MAX)return 2;
 if(!enabled)on=0;
 if(!on&&other+(present?instances:1)>DISC_CHAIN_ACTIVE_MAX)return 3;
 return 0;
}
static inline int disc_shared_toggle_type(DiscChain *c,unsigned *enabled,unsigned type){
 if(!enabled||disc_shared_toggle_error(c,*enabled,type))return 0;
 unsigned present=0,on=0,id=1;
 for(unsigned i=0;i<c->count;i++){if(c->nodes[i].id>=id)id=c->nodes[i].id+1;if(c->nodes[i].type==type){present=1;on|=c->nodes[i].enabled;}}
 if(!present){
  if(c->count>=DISC_CHAIN_MAX||id>2147483647u)return 0;
  DiscChainNode *n=&c->nodes[c->count++];n->id=id;n->type=type;n->enabled=1;
  for(unsigned j=0;j<12;j++)n->params[j]=type==DISC_WARMGLOSS_TYPE?disc_warmgloss_default[j]:type==DISC_STEREOFX_TYPE?disc_stereofx_default[j]:type==DISC_FFHAAS_TYPE?disc_ffhaas_default[j]:type==DISC_HAAS_TYPE?disc_haas_default[j]:disc_param_default[type][j];
 }else{if(!*enabled)on=0;for(unsigned i=0;i<c->count;i++)if(c->nodes[i].type==type)c->nodes[i].enabled=!on;}
 *enabled=*enabled||!on;return 1;
}
#endif
