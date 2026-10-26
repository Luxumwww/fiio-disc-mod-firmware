/* Exact stock I2C clients and CS43131 register protocol, not a bus scan.
 * Used only inside a verified DoP configuration or native health callback. */
struct DopMessage {unsigned short address,flags,length;unsigned char *buffer;};
struct DopTransfer {struct DopMessage *messages;unsigned count;};
static int (*dop_real_ioctl)(int,unsigned,void *);
static unsigned dop_bus_writes,dop_health_checks,dop_health_avoided,dop_bus_failures,dop_frame_repairs;
static int dop_register(int fd,unsigned address,unsigned reg,unsigned char *value,int writing){
 unsigned char bytes[5]={reg>>16,reg>>8,reg,0,*value};
 struct DopMessage m[2]={{address,0,writing?5:4,bytes},{address,1,1,value}};
 struct DopTransfer x={m,writing?1u:2u};
 return dop_real_ioctl(fd,0x707,&x)==(writing?1:2);
}
static int dop_bus_open(void){
 char name[20];const char *paths[2]={"/sys/bus/i2c/devices/3-0030/name","/sys/bus/i2c/devices/3-0032/name"};
 const char *names[2]={"cs43131\n","cs43131c\n"};
 for(unsigned i=0;i<2;i++){int f=open(paths[i],0),n=f<0?0:read(f,name,sizeof(name)-1);if(f>=0)close(f);if(n<=0)return -1;name[n]=0;if(strcmp(name,names[i]))return -1;}
 return open("/dev/i2c-3",2);
}
static int dop_bus_check(unsigned rate,int change){
 int f=dop_bus_open();if(f<0){dop_bus_failures++;return 0;}
 if(rate!=176400&&rate!=352800){close(f);return 0;}
 /* #236 DoP_init uses 24-bit slots (48 clocks), while the verified native
  * S32 stereo BAIC3 output sends 32-bit slots (64 clocks). Keep the original
  * serial polarity/delay/channel mapping; change only the expected period.
  * LCHI/LCPR may only be written while ASP is powered down. */
 const unsigned regs[8]={0x1000b,0x70004,0x40014,0x40015,0x40016,0x40017,0x40018,0x40019};
 unsigned char old[2][8],power[2],wanted_rate=rate==176400?5:7,wanted_speed=rate==176400?0:4;
 int ok=1;
 for(unsigned c=0;c<2&&ok;c++){
  unsigned char id[3]={0};unsigned address=0x30+2*c;
  for(unsigned i=0;i<3&&ok;i++)ok=dop_register(f,address,0x10000+i,&id[i],0);
  if(id[0]!=0x43||id[1]!=0x13||id[2]!=0x10)ok=0;
  for(unsigned i=0;i<8&&ok;i++)ok=dop_register(f,address,regs[i],&old[c][i],0);
  if(ok)ok=dop_register(f,address,0x20000,&power[c],0);
  if(ok&&(old[c][1]&0x70)!=0x50)ok=0; /* Existing stock ASP DoP path only. */
  if(ok&&(old[c][6]!=0x0c||old[c][7]!=0x0a||old[c][3]||old[c][5]||
      !((old[c][2]==23&&old[c][4]==47)||(old[c][2]==31&&old[c][4]==63))))ok=0;
  if(ok&&old[c][4]!=63&&(!change||!(power[c]&0x40)))ok=0;
 }
 unsigned changed[2]={0};
 if(ok)for(unsigned c=0;c<2&&ok;c++){
  unsigned char v[5]={(old[c][0]&0xf0)|wanted_rate,(old[c][1]&~0x0c)|wanted_speed,31,0,63};
  for(unsigned i=0;i<5&&ok;i++)if(v[i]!=old[c][i]){
   if(!change){ok=0;break;}
   /* Track attempted writes too: a bus error may occur after the chip latched
    * data. Compensate within this same muted transaction on failure. */
   changed[c]|=1u<<i;ok=dop_register(f,0x30+2*c,regs[i],&v[i],1);
   unsigned char verify=0;if(ok){dop_bus_writes++;ok=dop_register(f,0x30+2*c,regs[i],&verify,0)&&verify==v[i];}
  }
 }
 if(!ok&&change)for(unsigned c=0;c<2;c++)for(unsigned i=0;i<5;i++)if(changed[c]&(1u<<i))dop_register(f,0x30+2*c,regs[i],&old[c][i],1);
 if(ok&&change)for(unsigned c=0;c<2;c++)if(changed[c]&0x14)dop_frame_repairs++;
 close(f);if(!ok&&change)dop_bus_failures++;return ok;
}
