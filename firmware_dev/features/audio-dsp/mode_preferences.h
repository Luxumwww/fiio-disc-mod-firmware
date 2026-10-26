#ifndef DISC_DSP_MODE_PREFERENCES_H
#define DISC_DSP_MODE_PREFERENCES_H
/* Append pure mode without changing legacy BT/USB/network preferences. */
static void disc_dsp_read_masks(const char *text,unsigned masks[4]){
 unsigned bt=0,usb=0,net=0,pure=0;
 if(sscanf(text,"%u %u %u %u",&bt,&usb,&net,&pure)>=2&&bt<=8191&&usb<=8191&&net<=8191&&pure<=8191){masks[0]=bt&8189;masks[1]=usb&8189;masks[2]=net&8189;masks[3]=pure&8189;}
 else masks[0]=masks[1]=masks[2]=masks[3]=0;
}
static inline unsigned disc_dsp_player_index(unsigned mode){return mode==1?3:mode==2?0:mode==4?1:mode==5?2:4;}
#endif
