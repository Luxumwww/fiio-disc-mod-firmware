/* Explicit five-second RAM-only diagnostic; dormant during ordinary use.
 * Measures native refresh callbacks/layout/draw, not panel vsync. */
static unsigned frame_probe_active,frame_probe_count,frame_probe_until;
static struct {unsigned start,duration,period,mode;int shade_y,grid_h;} frame_probe_rows[384];
static unsigned frame_probe_us(void){long t[2];if(clock_gettime(1,t))return 0;return (unsigned)t[0]*1000000u+(unsigned)t[1]/1000u;}
static unsigned frame_probe_begin(void){return frame_probe_active?frame_probe_us():0;}
static void frame_probe_end(unsigned start,void *timer){
 if(!frame_probe_active||frame_probe_count>=384)return;
 unsigned duration=frame_probe_us()-start;LvObj *shade=LV_GLOBAL(0x8deee4),*grid=LV_GLOBAL(0x8dee94);
 unsigned i=frame_probe_count++;frame_probe_rows[i].start=start;frame_probe_rows[i].duration=duration;
 frame_probe_rows[i].period=timer?*(unsigned *)timer:0;frame_probe_rows[i].mode=*(unsigned *)0x8e16d4;
 frame_probe_rows[i].shade_y=shade&&lv_valid(shade)?*(int *)((char *)shade+24):-999;
 frame_probe_rows[i].grid_h=grid&&lv_valid(grid)?*(int *)((char *)grid+32)-*(int *)((char *)grid+24)+1:0;
}
static void frame_probe_poll(void){
 if(!access("/tmp/disc-runtime/frame-probe.request",0)){
  unlink("/tmp/disc-runtime/frame-probe.request");frame_probe_count=0;frame_probe_until=milliseconds()+5000;frame_probe_active=1;
 }
 if(!frame_probe_active||milliseconds()<frame_probe_until)return;
 frame_probe_active=0;int f=open("/tmp/disc-runtime/frame-probe.csv",1|0x100|0x200,0600);if(f<0)return;
 const char heading[]="start_us,duration_us,period_ms,mode,shade_y,grid_h\n";write(f,heading,sizeof(heading)-1);
 for(unsigned i=0;i<frame_probe_count;i++){char b[120];int n=snprintf(b,sizeof(b),"%u,%u,%u,%u,%d,%d\n",frame_probe_rows[i].start,frame_probe_rows[i].duration,frame_probe_rows[i].period,frame_probe_rows[i].mode,frame_probe_rows[i].shade_y,frame_probe_rows[i].grid_h);write(f,b,n);}close(f);
}
