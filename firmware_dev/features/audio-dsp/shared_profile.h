#ifndef DISC_SHARED_PROFILE_H
#define DISC_SHARED_PROFILE_H
#include "chain_format.h"
/* Master and all nodes publish in one atomic file; chain hash excludes master. */
static inline const char *disc_shared_payload(const char *text,unsigned *enabled){
 const char *prefix="DSP-SHARED-1 ";unsigned i=0;while(prefix[i]){if(text[i]!=prefix[i])return 0;i++;}
 if((text[i]!='0'&&text[i]!='1')||text[i+1]!='\n')return 0;*enabled=(unsigned)(text[i]-'0');return text+i+2;
}
static inline int disc_shared_parse(const char *text,DiscChain *chain,unsigned *enabled){const char *payload=disc_shared_payload(text,enabled);return payload&&disc_chain_parse(payload,chain);}
static inline int disc_shared_format(const DiscChain *c,unsigned enabled,char *out,unsigned cap){
 if(enabled>1||!disc_chain_valid(c))return 0;int n=snprintf(out,cap,"DSP-SHARED-1 %u\n1 %u\n",enabled,c->count);if(n<0||(unsigned)n>=cap)return 0;
 for(unsigned i=0;i<c->count;i++){const DiscChainNode *p=&c->nodes[i];int count=snprintf(out+n,cap-(unsigned)n,"%u %u %u",p->id,p->type,p->enabled);if(count<0||(unsigned)count>=cap-(unsigned)n)return 0;n+=count;
  for(unsigned j=0;j<12;j++){count=snprintf(out+n,cap-(unsigned)n," %.6g",(double)p->params[j]);if(count<0||(unsigned)count>=cap-(unsigned)n)return 0;n+=count;}
  if((unsigned)n+1>=cap)return 0;out[n++]='\n';out[n]=0;
 }return n;
}
#endif
