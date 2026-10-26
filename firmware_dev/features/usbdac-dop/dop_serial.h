/* Narrow DoP serial profile. Reuse native DAC handles and mute sequencing;
 * no PCM write/read wrapper, audio packet mutation, silence policy or reopen. */
static unsigned dop_serial_applied,dop_serial_failed,dop_serial_health_forwarded;
static int dop_serial_bind(void);
static int dop_serial_apply(unsigned rate){
 int ok=dop_bus_check(rate,1);
 if(ok)dop_serial_applied++;else dop_serial_failed++;
 return ok;
}
static int dop_serial_ioctl(int fd,unsigned request,void *arg){
 unsigned p=PC_WORD(0x83dfc4),rate=p?PC_WORD(p+0xc):0;
 int stream=dop_serial_profile&&dop_stream(rate,32,1)&&PC_BYTE(0x899060)==1;
 int left=fd==(int)PC_WORD(0x83a7d0),right=fd==(int)PC_WORD(0x83a7cc);
 if(stream&&((left&&request==0x20004d28)||(right&&request==0x20004d2b))){
  /* #236's DSD health branch compares ASP rate with 0x0b, although valid
   * DoP64/128 is 0x05/0x07. Suppress that reset only after both actual DAC
   * configurations pass the exact rate/speed/format/frame validation. */
  dop_health_checks++;
  if(dop_bus_check(rate,0)){dop_health_avoided++;return 0;}
  dop_serial_health_forwarded++;
 }
 if(stream&&left&&request==0x20004d2e)dop_serial_apply(rate);
 /* Native unmute must run even if validation/I2C fails. Do not introduce
  * the previous experiment's positive-return failure or muted latch. */
 return dop_real_ioctl(fd,request,arg);
}
static void dop_serial_finish(unsigned rate){
 if(!dop_serial_profile||!dop_stream(rate,32,1)||PC_BYTE(0x899060)!=1)return;
 if(dop_bus_check(rate,0))return;
 /* Covers a same-format native callback that did not run mute/unmute.
  * The existing left ioctl controls both chips. Never open /dev/cs43131. */
 int fd=(int)PC_WORD(0x83a7d0);
 int muted=dop_real_ioctl(fd,0x20004d2d,0);
 if(muted>=0)dop_serial_apply(rate);else dop_serial_failed++;
 dop_real_ioctl(fd,0x20004d2e,0);
}
static int dop_serial_bind(void){
 if(!dop_real_ioctl)dop_real_ioctl=(int (*)(int,unsigned,void *))dlsym((void *)0,"ioctl");
 if(!dop_real_ioctl)return 0;
 unsigned original=(unsigned)(unsigned long)dop_real_ioctl,wrapper=(unsigned)(unsigned long)dop_serial_ioctl;
 if(PC_WORD(PLAYER_IOCTL_GOT)!=original&&PC_WORD(PLAYER_IOCTL_GOT)!=wrapper)return 0;
 PC_WORD(PLAYER_IOCTL_GOT)=wrapper;return 1;
}
