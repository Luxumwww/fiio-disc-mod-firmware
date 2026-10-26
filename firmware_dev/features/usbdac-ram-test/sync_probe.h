/* Exact o32 ALSA SYNC_PTR: flags6 queries state/control; flags7 also
 * refreshes hardware pointer. Both GET appl_ptr and avail_min, so neither
 * writes either control value. No timestamp helper with flags3 is called. */
struct SyncProbe {unsigned flags,status[16],control[16];};
_Static_assert(sizeof(struct SyncProbe)==132,"o32 SYNC_PTR ABI");
struct SyncRecord {unsigned before_state,after_state,before_hw,after_hw,before_app,after_app,before_min,after_min;};
static int sync_read_pair(int fd,int (*call)(int,unsigned,void *),struct SyncRecord *r){
 struct SyncProbe first={0},last={0};first.flags=6;
 int rc=call(fd,0xc0844123u,&first);if(rc<0)return 1;
 r->before_state=first.status[0];r->before_hw=first.status[2];r->before_app=first.control[0];r->before_min=first.control[1];
 if(first.status[0]!=3)return 2;
 last.flags=7;rc=call(fd,0xc0844123u,&last);if(rc<0)return 3;
 r->after_state=last.status[0];r->after_hw=last.status[2];r->after_app=last.control[0];r->after_min=last.control[1];
 if(last.status[0]!=3)return 4;
 if(first.control[0]!=last.control[0]||first.control[1]!=last.control[1])return 5;
 return 0;
}
