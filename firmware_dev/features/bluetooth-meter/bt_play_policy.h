/* Explicit transport intent: a retry must never toggle its direction. */
typedef struct {unsigned deadline,next,attempts;int intent,pending,result;} DiscBtPlay;
static int bt_play_state(const char *s){if(!strcmp(s,"playing"))return 1;if(!strcmp(s,"paused")||!strcmp(s,"stopped"))return 0;return -1;}
static void bt_play_begin(DiscBtPlay *p,unsigned t,int state){*p=(DiscBtPlay){t+1800,t,0,state==1?2:1,1,0};}
static int bt_play_due(DiscBtPlay *p,unsigned t,int state){
 if(!p->pending)return 0;
 if((int)(t-p->deadline)>=0){p->pending=0;p->result=-2;return 0;}
 if(p->attempts&&state==(p->intent==1?1:0)){p->pending=0;p->result=1;return 0;}
 if((int)(t-p->next)<0)return 0;
 if(p->attempts>=3){p->pending=0;p->result=-3;return 0;}
 p->attempts++;p->next=t+450;return p->intent;
}
