/* Presentation of requested/observed transports, with a bounded pending state. */
typedef struct {int request[2],waiting[2];unsigned since[2];} DiscDebugView;
static int disc_debug_value(DiscDebugView *v,unsigned now,int i,int requested,int active,int fresh,int paused,int rescue,int reconnect){
 int value=requested<0?-1:!fresh?3:requested==active?active:3;
 if(i==0&&requested==1&&paused&&!rescue&&(!fresh||!active))value=2;
 else if(i==0&&fresh&&rescue&&active)value=4;
 else if(i==0&&requested==1&&fresh&&!active&&!paused&&reconnect)value=6;
 if(i==1&&fresh&&rescue)value=active;
 if(value==3||value==6){
  if(!v->waiting[i]||v->request[i]!=requested){v->waiting[i]=1;v->since[i]=now;}
  if(now-v->since[i]>=(fresh?30u:10u))value=5;
 }else v->waiting[i]=0;
 v->request[i]=requested;return value;
}
