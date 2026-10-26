#ifndef DISC_PURE_TOOLBAR_ORDER_H
#define DISC_PURE_TOOLBAR_ORDER_H
static void pure_order_default(unsigned order[11]){const unsigned initial[11]={0,1,2,3,4,9,5,6,7,8,10};for(unsigned i=0;i<11;i++)order[i]=initial[i];}
static int pure_order_parse(const char *s,unsigned order[11]){
 unsigned v[11],seen=0;int count=sscanf(s,"%u %u %u %u %u %u %u %u %u %u %u",v,v+1,v+2,v+3,v+4,v+5,v+6,v+7,v+8,v+9,v+10);if(count<10)return 0;
 for(int i=0;i<count;i++){if(v[i]>=(unsigned)count||(seen&(1u<<v[i])))return 0;seen|=1u<<v[i];}
 if(count==10)v[10]=10;
 for(unsigned i=0;i<11;i++){order[i]=v[i];}
 return 1;
}
static unsigned pure_order_position(const unsigned order[11],unsigned id){for(unsigned i=0;i<11;i++)if(order[i]==id)return i;return 0;}
static int pure_order_move(unsigned order[11],unsigned id,unsigned slot){
 unsigned from=pure_order_position(order,id);if(id>10||slot>10||from==slot)return 0;
 if(from<slot)for(unsigned i=from;i<slot;i++)order[i]=order[i+1];else for(unsigned i=from;i>slot;i--)order[i]=order[i-1];
 order[slot]=id;return 1;
}
static int pure_slot_x(unsigned slot){return 49+99*(int)(slot%3);}
static int pure_slot_y(unsigned slot){return 84*(int)(slot/3);}
static unsigned pure_nearest_slot(int x,int y){unsigned chosen=0;int distance=0x7fffffff;
 for(unsigned i=0;i<11;i++){int dx=x-pure_slot_x(i),dy=y-pure_slot_y(i),d=dx*dx+dy*dy;if(d<distance){distance=d;chosen=i;}}
 return chosen;
}
#endif
