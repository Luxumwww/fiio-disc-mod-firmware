#ifndef DISC_PURE_CONTROL_CALLBACKS_H
#define DISC_PURE_CONTROL_CALLBACKS_H
/* Two different native callbacks share one edit guard. Restore by event
 * filter, keeping every descriptor's original user data/filter bytes. */
static void pure_control_event_patch(unsigned *events,unsigned count,int block,unsigned replacement){
 if(count>32)return;
 for(unsigned j=0;events&&j<count;j++){unsigned code=events[j*3+2]&255,original=code==5?0x47f8f0:code==7?0x480228:0;
  if(original&&events[j*3]==(block?original:replacement))events[j*3]=block?replacement:original;
 }
}
#endif
