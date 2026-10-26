/* Own APK SDK: USB_TEMPERATURE_CORRECT (2040/2041), UVC2 group3/cmd6.
 * This command includes correction fields: preserve them byte-for-byte and
 * refuse writes while correction is enabled. Only change the spot position. */
static int center_read(unsigned group,unsigned char data[512]){
 unsigned char select[2]={(unsigned char)group,6},size[4]={0};
 if(thermal_protocol!=2||control(0x21,1,5<<8,10<<8,select,2)!=2)return 0;
 int l=control(0xa1,0x85,group<<8,10<<8,size,4);unsigned n=l==2?u16(size):l==4?u32(size):0;
 if(n<1||n>512)return 0;
 int got=control(0xa1,0x81,group<<8,10<<8,data,n);
 char path[100],report[1200];int used=snprintf(report,sizeof(report),"group=%u command=6 length=%u received=%d data=",group,n,got);
 for(int i=0;i<got;i++)used+=snprintf(report+used,sizeof(report)-(unsigned)used,"%02x",data[i]);
 snprintf(report+used,sizeof(report)-(unsigned)used,"\n");snprintf(path,sizeof(path),RAM "thermal.center-read-%u",group);atomic_text(path,report);
 return got==(int)n&&data[0]==1?got:0;
}
static int center_write(const unsigned char *data,unsigned n){unsigned char p[2]={3,6};return thermal_protocol==2&&control(0x21,1,5<<8,10<<8,p,2)==2&&control(0x21,1,3<<8,10<<8,(void *)data,n)==(int)n;}
static int center_apply(void){
 unsigned char original[512],changed[512],check[512];int n=center_read(3,original);
 if(n!=28||original[1]!=1||original[2]>2||original[3]!=0||u32(original+16)>1000||u32(original+20)>1000){atomic_text(RAM "thermal.center","verified=0\nreason=unsupported_or_correction_enabled\n");return 0;}
 if(u32(original+16)==500&&u32(original+20)==500){atomic_text(RAM "thermal.center","verified=1\nx=500\ny=500\nchanged=0\n");return 1;}
 memcpy(changed,original,(unsigned)n);put32(changed+16,500);put32(changed+20,500);
 char report[1200];int used=snprintf(report,sizeof(report),"data=");for(int i=0;i<n;i++)used+=snprintf(report+used,sizeof(report)-(unsigned)used,"%02x",original[i]);snprintf(report+used,sizeof(report)-(unsigned)used,"\n");atomic_text(RAM "thermal.center-original",report);
 if(center_write(changed,(unsigned)n))for(unsigned i=0;i<3;i++){usleep(120000);if(center_read(3,check)==n&&!memcmp(check,changed,(unsigned)n)){atomic_text(RAM "thermal.center","verified=1\nx=500\ny=500\nchanged=1\n");return 1;}}
 center_write(original,(unsigned)n);atomic_text(RAM "thermal.center","verified=0\nreason=readback_failed_restore_requested\n");return 0;
}
static void center_command(void){
 char b[128],op[24];unsigned p,c,v;if(!text(RAM "thermal.command",b,sizeof(b))||sscanf(b,"%u %u %23s %u",&p,&c,op,&v)!=4||p!=owner||c!=token)return;
 if(!strcmp(op,"center-query")){unsigned char data[512];unlink(RAM "thermal.command");center_read(3,data);center_read(2,data);}
 else if(!strcmp(op,"center")&&v==0){unlink(RAM "thermal.command");center_apply();}
}
