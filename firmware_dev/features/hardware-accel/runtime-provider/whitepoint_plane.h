/* One provider owns the complete four-layer configuration. fb2 is a static
 * RGB black plane above both the native UI and the existing ARGB fb1 layer.
 * This has no LVGL object/input handler, no extra lease, no per-frame pixels.
 * Only the exact reviewed 360x360 stock layout may enter this path. */
static unsigned wp_original[72],wp_expected[72],wp_checkat;
static int wp_value,wp_armed,wp_fault,wp_fd=-1,wp_recovered_visual;
static unsigned char *wp_pixels;
static int wp_native_ok(const unsigned *cfg){
 static const unsigned words[4]={0x003fc119,0x003fc110,0x003fc108,0x003fc100};
 for(unsigned l=0;l<4;l++)for(unsigned j=0;j<18;j++){
  unsigned value=j==0?words[l]:(j<=3?360u:0u);if(cfg[l*18+j]!=value)return 0;
 }return 1;
}
static void wp_external(void){if(wp_armed)wp_fault=1;}
static void wp_adopt(const unsigned *cfg){if(wp_armed&&!wp_fault)memcpy(wp_expected,cfg,sizeof wp_expected);}
static int wp_base_ok(const unsigned *cfg){
 if(wp_armed)return !wp_fault&&!memcmp(cfg,wp_expected,sizeof wp_expected);
 return wp_native_ok(cfg);
}
static void wp_visual(unsigned *cfg,int masked){
 cfg[0]=(cfg[0]&~(7u<<3))|((masked?1u:2u)<<3);
 cfg[18]=(cfg[18]&~((7u<<3)|(15u<<6)))|((masked?2u:3u)<<3)|(5u<<6)|1u;
}
static void wp_mask(unsigned *cfg,unsigned reduction){
 unsigned alpha=(reduction*255u+50u)/100u;
 for(unsigned l=0;l<4;l++)cfg[l*18]=(cfg[l*18]&~(7u<<3))|((l==0?1u:l==1?2u:l==2?3u:0u)<<3);
 cfg[36]=(cfg[36]&~0x003fffffu)|1u|(3u<<3)|(4u<<6)|(1u<<13)|(alpha<<14);
}
/* The display controller can outlive mq_ui. Recognize only our exact static
 * topology and an exactly representable 1..80 setting, rebuild the canonical
 * native baseline, and never stack a second black plane after a UI reload. */
static int wp_recover(const unsigned *cfg){
 if(wp_armed||wp_fault)return 0;
 unsigned canonical[72],trial[72];memset(canonical,0,sizeof canonical);
 static const unsigned words[4]={0x003fc119,0x003fc110,0x003fc108,0x003fc100};
 for(unsigned l=0;l<4;l++){canonical[l*18]=words[l];canonical[l*18+1]=canonical[l*18+2]=canonical[l*18+3]=360;}
 for(unsigned r=1;r<=80;r++){
  memcpy(trial,canonical,sizeof trial);if(cfg[18]&1u)wp_visual(trial,0);wp_mask(trial,r);
  if(!memcmp(cfg,trial,sizeof trial)){
   memcpy(wp_original,canonical,sizeof canonical);memcpy(wp_expected,cfg,sizeof wp_expected);
   wp_value=(int)r;wp_armed=1;wp_recovered_visual=1;return 1;
  }
 }return 0;
}
static int wp_map(void){
 if(wp_pixels)return 1;
 unsigned fix[32],basefix[32];int fd=open("/dev/fb2",2);if(fd<0)return 0;
 int basefd=(int)DU(0x8e091c);
 if(ioctl(fd,0x4602,fix)||ioctl(basefd,0x4602,basefix)||!basefix[4]||fix[4]<basefix[4]||fix[4]-basefix[4]!=4147200u||(fix[4]&3)||fix[5]!=1555200||fix[11]!=1440||basefix[5]!=1555200||basefix[11]!=1440){close(fd);return 0;}
 unsigned bias=fix[4]&4095u;void *mapping=mmap(0,1555200u+bias,3,1,fd,0);
 if(mapping==(void *)-1){close(fd);return 0;}
 wp_pixels=(unsigned char *)mapping+bias;wp_fd=fd;
 memset(wp_pixels,0,1555200);__sync_synchronize();return 1;
}
/* Roll back only a configuration that is still ours. An unrelated writer is
 * never overwritten, including after a failed PUT or a failed readback. */
static int wp_rollback(const unsigned *before,const unsigned *next){
 unsigned current[72];int fd=(int)DU(0x8e091c);
 if(ioctl(fd,0xc0044702u,current))return 0;
 if(!memcmp(current,before,sizeof current))return 1;
 if(memcmp(current,next,sizeof current))return 0;
 if(ioctl(fd,0xc0044701u,(void *)before)||ioctl(fd,0xc0044702u,current)||memcmp(current,before,sizeof current))return 0;
 if(DU(0x83abd0)&&dpu_shape())dpu_pan();return 1;
}
static int wp_write_checked(const unsigned *before,const unsigned *next){
 unsigned readback[72];int fd=(int)DU(0x8e091c);
 if(ioctl(fd,0xc0044701u,(void *)next)||ioctl(fd,0xc0044702u,readback)||memcmp(readback,next,sizeof readback)){
  if(!wp_rollback(before,next))wp_fault=1;return 0;
 }wp_adopt(next);return 1;
}
int disc_accel_whitepoint_get(void){
 unsigned current[72];if(!provider_guarded||wp_fault)return -1;
 if(ioctl((int)DU(0x8e091c),0xc0044702u,current))return -1;
 if(!wp_armed)wp_recover(current);
 if(wp_armed){if(memcmp(current,wp_expected,sizeof current)){wp_fault=1;return -1;}return wp_value;}
 if(dpu_on)return !memcmp(current,dpu_trial,sizeof current)?0:-1;
 return wp_native_ok(current)?0:-1;
}
int disc_accel_whitepoint_set(unsigned reduction){
 unsigned before[72],next[72],base[72],readback[72];int fd=(int)DU(0x8e091c);
 if(reduction>80u)reduction=80u;
 if(!provider_guarded||wp_fault||(reduction&&(!provider_enabled||!dpu_shape()||!DU(0x83abd0))))return -1;
 if(ioctl(fd,0xc0044702u,before))return -1;
 if(!wp_armed)wp_recover(before);
 if(wp_armed){if(memcmp(before,wp_expected,sizeof before)){wp_fault=1;return -1;}memcpy(base,wp_original,sizeof base);}
 else if(dpu_on){if(memcmp(before,dpu_trial,sizeof before)||!wp_native_ok(dpu_saved))return -1;memcpy(base,dpu_saved,sizeof base);}
 else {if(!wp_native_ok(before))return -1;memcpy(base,before,sizeof base);}
 if(reduction==(unsigned)wp_value&&!wp_recovered_visual)return (int)reduction;
 if(reduction&&!wp_map())return -1;
 memcpy(next,base,sizeof next);if(dpu_on)wp_visual(next,0);
 if(reduction)wp_mask(next,reduction);
 if(ioctl(fd,0xc0044701u,next)||ioctl(fd,0xc0044702u,readback)||memcmp(readback,next,sizeof next)){
  if(!wp_rollback(before,next))wp_fault=1;return -1;
 }
 /* An unchanged LVGL frame still needs a pan for the new descriptor config.
  * Keep the current front and native var offsets, never swap to stale pixels. */
 if(DU(0x83abd0)&&dpu_shape()&&!dpu_pan()){
  if(!wp_rollback(before,next))wp_fault=1;return -1;
 }
 if(ioctl(fd,0xc0044702u,readback)||memcmp(readback,next,sizeof next)){
  if(!wp_rollback(before,next))wp_fault=1;return -1;
 }
 if(reduction&&!wp_armed)memcpy(wp_original,base,sizeof base);
 if(dpu_on){memcpy(dpu_saved,base,sizeof base);if(reduction)wp_mask(dpu_saved,reduction);memcpy(dpu_trial,next,sizeof next);}
 wp_value=(int)reduction;wp_armed=reduction!=0;wp_fault=wp_recovered_visual=0;
 if(wp_armed)memcpy(wp_expected,next,sizeof next);wp_checkat=now();return wp_value;
}
static void wp_tick(void){
 if(!wp_armed||wp_fault)return;unsigned t=now();if(t-wp_checkat<200u)return;wp_checkat=t;
 /* Rotation and screen power do not alter a full-screen constant black layer.
  * Readback stays read-only; native suspend/resume owns power and timing. */
 (void)disc_accel_whitepoint_get();
}
