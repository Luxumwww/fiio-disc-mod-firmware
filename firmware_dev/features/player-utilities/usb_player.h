/* Explicit UI request, consumed only by the existing control worker while
 * holding pc_usb_mutex. It resets the bounded native recovery, not volume,
 * player transport, Bluetooth input or USB power negotiation. */
static unsigned pu_usb_checked,pu_usb_token;static int pu_usb_result,pu_usb_was_owned=-1;
static void pu_usb_request_poll(int owned){
 if(!owned){if(pu_usb_was_owned)unlink("/tmp/disc-runtime/usb-reconnect.request");pu_usb_was_owned=0;return;}pu_usb_was_owned=1;
 long t[2];if(clock_gettime(1,t))return;unsigned ms=(unsigned)t[0]*1000u+(unsigned)t[1]/1000000u;if(ms-pu_usb_checked<250)return;pu_usb_checked=ms;
 char b[120];int fd=open("/tmp/disc-runtime/usb-reconnect.request",0);if(fd<0)return;int n=read(fd,b,sizeof b-1);close(fd);unlink("/tmp/disc-runtime/usb-reconnect.request");if(n<=0)return;b[n]=0;
 unsigned pid,mode,generation,stamp,token;int at=0;if(sscanf(b,"%u %u %u %u %u %n",&pid,&mode,&generation,&stamp,&token,&at)!=5||b[at]||pid!=(unsigned)getpid()||mode!=(unsigned)pc_workmode()||stamp>(unsigned)t[0]||(unsigned)t[0]-stamp>5||!token)return;
 pu_usb_token=token;pu_usb_result=-1;pc_usb_observe();
 if(!owned||!pc_usb_owned()||generation!=pc_usb_generation||!pc_usb_present||!pc_usb_epoch_valid)return;
 pc_usb_minimum();pc_usb_need_rebind=1;pc_usb_ready=0;pc_usb_bound_generation=pc_usb_pcm_generation=0;pc_usb_attempts=pc_usb_retry_at=0;pc_usb_error=0;pc_usb_recovering=1;pu_usb_result=0;
}
