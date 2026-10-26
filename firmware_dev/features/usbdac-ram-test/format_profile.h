/* Explicit one-shot receiver format experiment; no PLL, HP power, GPIO,
 * PCM/DMA setup, channel phase or audio payload changes. */
struct FormatMessage {unsigned short address,flags,length;unsigned char *data;};
struct FormatTransfer {struct FormatMessage *messages;unsigned count;};
static unsigned format_attempts,format_applied,format_failed,format_writes,format_compensated;
static int format_result,format_mute_result,format_unmute_result;
static int format_reg(int fd,unsigned chip,unsigned reg,unsigned char *v,int writing){
 unsigned char b[5]={reg>>16,reg>>8,reg,0,*v};
 struct FormatMessage msgs[2]={{chip,0,writing?5:4,b},{chip,1,1,v}};
 struct FormatTransfer t={msgs,writing?1u:2u};
 return native_ioctl(fd,0x707,&t)==(writing?1:2);
}
static int format_set(int fd,unsigned chip,unsigned reg,unsigned char v){
 if(!format_reg(fd,chip,reg,&v,1))return 0;
 format_writes++;unsigned char check=0;
 return format_reg(fd,chip,reg,&check,0)&&check==v;
}
static int format_clients(void){
 const char *path[2]={"/sys/bus/i2c/devices/3-0030/name","/sys/bus/i2c/devices/3-0032/name"};
 const char *name[2]={"cs43131\n","cs43131c\n"};
 for(unsigned i=0;i<2;i++){char b[24];int fd=open(path[i],0),n=fd<0?0:read(fd,b,23);if(fd>=0)close(fd);if(n<=0)return 0;b[n]=0;if(strcmp(b,name[i]))return 0;}
 return 1;
}
static int format_transaction(int fd,int dac,unsigned hz){
 const unsigned regs[5]={0x1000c,0x5000a,0x5000b,0x40014,0x40016};
 unsigned char old[2][5],power[2];unsigned changed[2]={0},powered=0;
 for(unsigned c=0;c<2;c++){
  unsigned char v=0;for(unsigned i=0;i<3;i++)if(!format_reg(fd,0x30+2*c,0x10000+i,&v,0)||v!=(i==0?0x43:i==1?0x13:0x10))return 1;
  for(unsigned i=0;i<5;i++)if(!format_reg(fd,0x30+2*c,regs[i],&old[c][i],0))return 1;
  if(!format_reg(fd,0x30+2*c,0x20000,&power[c],0)||(power[c]&0x50))return 2;
  if(old[c][0]!=4||old[c][1]!=(c?15:7)||old[c][2]!=15||old[c][3]!=23||old[c][4]!=47)return 3;
  if(!format_reg(fd,0x30+2*c,0x1000b,&v,0)||v!=(hz==176400?5:7))return 3;
  if(!format_reg(fd,0x30+2*c,0x70004,&v,0)||v!=(hz==176400?0x50:0x54))return 3;
  if(!format_reg(fd,0x30+2*c,0x40015,&v,0)||v)return 3;
  if(!format_reg(fd,0x30+2*c,0x40017,&v,0)||v)return 3;
  if(!format_reg(fd,0x30+2*c,0x40018,&v,0)||v!=12)return 3;
  if(!format_reg(fd,0x30+2*c,0x40019,&v,0)||v!=10)return 3;
 }
 format_mute_result=native_ioctl(dac,0x20004d2d,0);
 if(format_mute_result<0)return 4;
 int result=0;
 for(unsigned c=0;c<2;c++){
  powered|=1u<<c;
  if(!format_set(fd,0x30+2*c,0x20000,power[c]|0x40)){result=5;break;}
 }
 if(!result)for(unsigned c=0;c<2&&!result;c++){
  unsigned char want[5]={(old[c][0]&~3u)|1u,old[c][1]&~1u,old[c][2]&~1u,31,63};
  for(unsigned i=0;i<5;i++){
   changed[c]|=1u<<i;
   if(!format_set(fd,0x30+2*c,regs[i],want[i])){result=6;break;}
  }
 }
 if(result)for(unsigned c=0;c<2;c++)for(unsigned i=0;i<5;i++)if(changed[c]&(1u<<i)){
  if(format_set(fd,0x30+2*c,regs[i],old[c][i]))format_compensated++;
 }
 for(unsigned c=0;c<2;c++)if(powered&(1u<<c))if(!format_set(fd,0x30+2*c,0x20000,power[c]))result=7;
 /* Existing native unmute always runs, even if a bus write failed. */
 format_unmute_result=native_ioctl(dac,0x20004d2e,0);
 if(format_unmute_result<0)result=8;
 return result;
}
static void format_apply(void){
 format_attempts++;unsigned sink=W(0x83dfe4),p=W(0x83dfc4),hz=p?W(p+0xc):0;
 if(!enabled||phase!=IDLE||control_active||capture_active||!sink||
    (hz!=176400&&hz!=352800)||!scope(hz)){format_result=9;format_failed++;return;}
 if(!format_clients()){format_result=10;format_failed++;return;}
 int fd=open("/dev/i2c-3",2);if(fd<0){format_result=11;format_failed++;return;}
 pthread_mutex_lock((void *)(unsigned long)(sink+0x1c));
 format_result=scope(hz)?format_transaction(fd,(int)W(0x83a7d0),hz):9;
 pthread_mutex_unlock((void *)(unsigned long)(sink+0x1c));close(fd);
 if(!format_result)format_applied++;else format_failed++;
}
static void format_poll(void){
 static unsigned checked;unsigned now=seconds();if(checked==now)return;checked=now;
 if(!access("/tmp/disc-runtime/dop-format-profile-start",0)){
  unlink("/tmp/disc-runtime/dop-format-profile-start");format_apply();
 }
}
