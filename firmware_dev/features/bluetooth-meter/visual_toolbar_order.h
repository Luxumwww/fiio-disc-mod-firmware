#ifndef DISC_VISUAL_TOOLBAR_ORDER_H
#define DISC_VISUAL_TOOLBAR_ORDER_H
#define VT_COUNT 6u
static void vt_order_default(unsigned o[6]){for(unsigned i=0;i<6;i++)o[i]=i;}
static int vt_space(char c){return c==' '||c=='\t'||c=='\r'||c=='\n';}
static int vt_order_parse(const char *s,unsigned o[6]){unsigned v[6],seen=0;for(unsigned i=0;i<6;i++){while(vt_space(*s))s++;if(*s<'0'||*s>'5'||(s[1]&&!vt_space(s[1])))return 0;v[i]=(unsigned)(*s++-'0');if(seen&(1u<<v[i]))return 0;seen|=1u<<v[i];}while(vt_space(*s))s++;if(*s)return 0;for(unsigned i=0;i<6;i++)o[i]=v[i];return 1;}
static unsigned vt_order_position(const unsigned o[6],unsigned id){for(unsigned i=0;i<6;i++)if(o[i]==id)return i;return 0;}
static int vt_order_move(unsigned o[6],unsigned id,unsigned slot){if(id>=6||slot>=6)return 0;unsigned from=vt_order_position(o,id);if(from==slot)return 0;if(from<slot)for(unsigned i=from;i<slot;i++)o[i]=o[i+1];else for(unsigned i=from;i>slot;i--)o[i]=o[i-1];o[slot]=id;return 1;}
static int vt_slot_x(unsigned slot){return 49+(int)(slot%3)*99;}
static int vt_slot_y(unsigned slot){return slot<3?0:98;}
static unsigned vt_nearest_slot(int x,int y){unsigned best=0;int distance=0x7fffffff;for(unsigned i=0;i<6;i++){int dx=x-vt_slot_x(i),dy=y-vt_slot_y(i),d=dx*dx+dy*dy;if(d<distance){distance=d;best=i;}}return best;}
#endif
