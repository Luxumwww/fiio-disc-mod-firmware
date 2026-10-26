/* Shared production policy: module loading belongs to the initialized native
 * UI loop, never ELF constructors, other usleep callers or a recursive pump. */
typedef struct {unsigned constructor_ready,loading,finished;} DiscStartupGate;
static int startup_claim(DiscStartupGate *s,unsigned caller,int display_ready,int lock_held){
 if(!s->constructor_ready||s->loading||s->finished||caller!=0x40397c||!display_ready||!lock_held)return 0;
 s->loading=1;return 1;
}
static void startup_complete(DiscStartupGate *s){s->loading=0;s->finished=1;}
