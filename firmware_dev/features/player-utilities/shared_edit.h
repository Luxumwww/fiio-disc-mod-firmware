#ifndef DISC_SHARED_EDIT_H
#define DISC_SHARED_EDIT_H
#include "../audio-dsp/shared_profile.h"
#include "param_schema.h"
/* Update one node/field in the freshly locked configuration. Other instances,
 * master, ordering and concurrent edits to other fields remain intact.
 * 1=OK, -1=stale field/removed node, -2=invalid, -3=capacity. */
static int disc_shared_edit(DiscChain *c,unsigned *id,unsigned type,unsigned param,float expected,float value){
 if(!disc_chain_valid(c)||type>16||type==1||param>=disc_native_effects[type].count||!isfinite(value))return -2;
 const DiscNativeParam *p=&disc_native_effects[type].params[param];
 if(value<p->low||value>p->high)return -2;
 DiscChainNode *node=0;
 for(unsigned i=0;i<c->count;i++)if(c->nodes[i].id==*id)node=&c->nodes[i];
 if(*id&&(!node||node->type!=type||node->params[param]!=expected))return -1;
 if(!*id){
  if(c->count>=DISC_CHAIN_MAX)return -3;
  unsigned next=1;for(unsigned i=0;i<c->count;i++)if(c->nodes[i].id>=next)next=c->nodes[i].id+1;
  if(next>2147483647u)return -3;
  DiscChainNode n={0};n.id=next;n.type=type;n.enabled=0;
  for(unsigned j=0;j<disc_native_effects[type].count;j++)n.params[j]=disc_native_effects[type].params[j].initial;
  if(!disc_chain_node_valid(&n))return -2;
  c->nodes[c->count++]=n;node=&c->nodes[c->count-1];*id=next;
 }
 float old=node->params[param];node->params[param]=value;
 if(!disc_chain_node_valid(node)){node->params[param]=old;return -2;}return 1;
}
/* Reset one exact instance in a freshly locked chain. The confirmation's
 * parameter snapshot rejects concurrent changes; switches and other nodes
 * are read fresh and retained. 0=already default/absent, 1=changed. */
static int disc_shared_reset(DiscChain *c,unsigned id,unsigned type,const float expected[12]){
 if(!disc_chain_valid(c)||type>16||type==1||!expected)return -2;
 if(!id)return 0; /* Merely restoring an unadded effect must not create it. */
 DiscChainNode *node=0;for(unsigned i=0;i<c->count;i++)if(c->nodes[i].id==id)node=&c->nodes[i];
 if(!node||node->type!=type)return -1;
 for(unsigned j=0;j<12;j++)if(node->params[j]!=expected[j])return -1;
 DiscChainNode next=*node;unsigned changed=0;
 for(unsigned j=0;j<12;j++){next.params[j]=j<disc_native_effects[type].count?disc_native_effects[type].params[j].initial:0;if(next.params[j]!=node->params[j])changed=1;}
 if(!disc_chain_node_valid(&next))return -2;if(!changed)return 0;
 *node=next;return 1;
}
#endif
