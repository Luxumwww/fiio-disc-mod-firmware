/* Device-side palette control. Wire layout checked against this APK's SDK:
 * CUsbCommandConfig<2026/2027>, ConvertData<USB_IMAGE_ENHANCEMENT>.
 * UVC2 selector=2, command=5; palette is payload byte 5. All other bytes
 * are preserved and every requested change is read back before saving. */
static int thermal_protocol;
static unsigned palette_checked;
static int palette_valid(unsigned p){return p==1||p==2||(p>=10&&p<=22);}
static int palette_select(void){unsigned char p[2]={2,5};return control(0x21,1,5<<8,10<<8,p,2)==2;}
static int palette_read(unsigned char data[512]){
 unsigned char size[4]={0},version[4]={0};
 if(!thermal_protocol){
  control(0xa1,0x85,4<<8,10<<8,size,4);
  if(control(0xa1,0x81,4<<8,10<<8,version,4)<3)return 0;
  if(!memcmp(version,"2.0",3))thermal_protocol=2;
  else if(!memcmp(version,"1.0",3))thermal_protocol=1;
  else return 0;
 }
 if(thermal_protocol==1){
  unsigned char select[3]={3,0,5},reply[27]={0};
  if(control(0x21,1,2<<8,10<<8,select,3)!=3||control(0xa1,0x81,2<<8,10<<8,reply,27)!=27||reply[0]!=3||reply[2]!=5||reply[3]!=1||!palette_valid(reply[8]))return 0;
  memset(data,0,512);memcpy(data,reply+3,24);thermal_palette=data[5];return 24;
 }
 if(!palette_select())return 0;
 int got=control(0xa1,0x85,2<<8,10<<8,size,4);if(got!=2&&got!=4)return 0;
 unsigned n=got==2?u16(size):u32(size);if(n<6||n>512)return 0;
 memset(data,0,512);int received=control(0xa1,0x81,2<<8,10<<8,data,n);
 {char report[256];int used=snprintf(report,sizeof(report),"protocol=%d length_reply=%d requested=%u received=%d data=",thermal_protocol,got,n,received);for(unsigned i=0;i<n&&i<24&&used<220;i++)used+=snprintf(report+used,sizeof(report)-(unsigned)used,"%02x",data[i]);snprintf(report+used,sizeof(report)-(unsigned)used,"\n");atomic_text(RAM "thermal.palette",report);}
 if(received!=(int)n||data[0]!=1||!palette_valid(data[5]))return 0;
 thermal_palette=data[5];return (int)n;
}
static int palette_write(const unsigned char *data,unsigned n){
 if(thermal_protocol==1){if(n!=24)return 0;unsigned char packet[27]={3,0x88,5};memcpy(packet+3,data,24);return control(0x21,1,2<<8,10<<8,packet,27)==27;}
 return palette_select()&&control(0x21,1,2<<8,10<<8,(void *)data,n)==(int)n;
}
static int palette_set(unsigned requested){
 if(!palette_valid(requested))return 0;
 unsigned char original[512],changed[512],check[512];int n=palette_read(original);if(!n)return 0;
 memcpy(changed,original,(unsigned)n);changed[5]=(unsigned char)requested;
 if(palette_write(changed,(unsigned)n)&&palette_read(check)==n&&check[5]==requested)return 1;
 /* Readback failure never becomes a successful saved preference. */
 palette_write(original,(unsigned)n);palette_read(check);return 0;
}
static void palette_command(void){
 char b[128];if(!text(RAM "thermal.command",b,sizeof(b)))return;
 unsigned p,c,value;char op[24];if(sscanf(b,"%u %u %23s %u",&p,&c,op,&value)!=4||p!=owner||c!=token){unlink(RAM "thermal.command");return;}
 unlink(RAM "thermal.command");
 if(!strcmp(op,"palette")){
  if(palette_set(value)){
   mkdir("/usr/data/disc-runtime/module-data/disc-usb-tools",0700);
   snprintf(b,sizeof(b),"%u\n",value);atomic_text("/usr/data/disc-runtime/module-data/disc-usb-tools/palette.conf",b);state("streaming","配色已切换");
  }else state("streaming","该配色暂不可用");
 }
}
