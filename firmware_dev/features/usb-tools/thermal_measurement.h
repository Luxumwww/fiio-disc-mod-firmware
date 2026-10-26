/* Verified against this APK's HCUSBSDK ConvertData/ParseRecvData functions.
 * Group3 command1 = basic measurement (47-byte known prefix), command2 =
 * normal/expert mode. Preserve the complete returned configuration. */
static int measurement_select(unsigned command){unsigned char p[2]={3,(unsigned char)command};return control(0x21,1,5<<8,10<<8,p,2)==2;}
static void measurement_trace(unsigned command,int length_reply,unsigned requested,int received,const unsigned char *data){
 char report[1200],path[100];int used=snprintf(report,sizeof(report),"command=%u length_reply=%d requested=%u received=%d data=",command,length_reply,requested,received);
 for(int i=0;data&&i<received&&i<512;i++)used+=snprintf(report+used,sizeof(report)-(unsigned)used,"%02x",data[i]);snprintf(report+used,sizeof(report)-(unsigned)used,"\n");
 snprintf(path,sizeof(path),RAM "thermal.measurement-read-%u",command);atomic_text(path,report);
}
static int measurement_read(unsigned command,unsigned minimum,unsigned char data[512]){
 if(thermal_protocol==1){
  unsigned n=command==1?47u:3u;unsigned char p[3]={3,0,(unsigned char)command},reply[50]={0};
  if(control(0x21,1,3<<8,10<<8,p,3)!=3||control(0xa1,0x81,3<<8,10<<8,reply,n+3)!=(int)(n+3)||reply[0]!=3||reply[2]!=command)return 0;
  memcpy(data,reply+3,n);return data[0]==1&&n>=minimum?(int)n:0;
 }
 if(thermal_protocol!=2||!measurement_select(command)){measurement_trace(command,-1,0,-1,0);return 0;}
 unsigned char size[4]={0};int l=control(0xa1,0x85,3<<8,10<<8,size,4);if(l!=2&&l!=4){measurement_trace(command,l,0,-1,0);return 0;}
 unsigned n=l==2?u16(size):u32(size);if(!n||n>512){measurement_trace(command,l,n,-1,0);return 0;}
 int got=control(0xa1,0x81,3<<8,10<<8,data,n);measurement_trace(command,l,n,got,data);return n>=minimum&&got==(int)n&&data[0]==1?got:0;
}
static int measurement_write(unsigned command,const unsigned char *data,unsigned n){
 if(thermal_protocol==1){if(n!=(command==1?47u:3u))return 0;unsigned char p[50]={3,0x88,(unsigned char)command};memcpy(p+3,data,n);return control(0x21,1,3<<8,10<<8,p,n+3)==(int)(n+3);}
 return thermal_protocol==2&&measurement_select(command)&&control(0x21,1,3<<8,10<<8,(void *)data,n)==(int)n;
}
static int measurement_matches(const unsigned char *p,unsigned n){return p[1]==1&&p[2]==1&&p[3]==1&&p[4]==0&&p[30]==1&&p[31]==2&&(n<45||p[44]==1);}
static void measurement_pause(void){
#ifndef DISC_THERMAL_MEASURE_TEST
 usleep(120000);
#endif
}
static int measurement_verify(unsigned command,int n,unsigned char *check){
 for(unsigned i=0;i<3;i++){if(measurement_read(command,command==1?32u:3u,check)==n&&(command==1?measurement_matches(check,(unsigned)n):(check[1]==1&&check[2]==0)))return 1;if(i<2)measurement_pause();}return 0;
}
static int measurement_apply(void){
 unsigned char basic[512],mode[512],changed[512],check[512];
 int n=measurement_read(1,32,basic),m=measurement_read(2,3,mode);thermal_measurement=-1;
 if(!n||!m||(n!=32&&n<47)||basic[1]>1||basic[2]>1||basic[3]>1||basic[4]>1||(n>=45&&basic[44]>1)||(mode[1]!=1&&mode[1]!=2)||mode[2]>1){atomic_text(RAM "thermal.measurement","verified=0\nresult=read_or_validation_failed\n");return 0;}
 /* Save the original payload before any change, in this session's RAM.
  * No emissivity, range, compensation or calibration field is synthesized. */
 char report[2300];int used=snprintf(report,sizeof(report),"protocol=%d basic_length=%d mode_length=%d\nbasic=",thermal_protocol,n,m);
 for(int i=0;i<n;i++)used+=snprintf(report+used,sizeof(report)-(unsigned)used,"%02x",basic[i]);
 used+=snprintf(report+used,sizeof(report)-(unsigned)used,"\nmode=");for(int i=0;i<m;i++)used+=snprintf(report+used,sizeof(report)-(unsigned)used,"%02x",mode[i]);snprintf(report+used,sizeof(report)-(unsigned)used,"\n");atomic_text(RAM "thermal.measurement-original",report);
 int changed_mode=mode[1]!=1||mode[2]!=0,changed_basic=!measurement_matches(basic,(unsigned)n),attempted_mode=0,attempted_basic=0;
 if(changed_mode){memcpy(changed,mode,(unsigned)m);changed[1]=1;changed[2]=0;attempted_mode=1;if(!measurement_write(2,changed,(unsigned)m)||!measurement_verify(2,m,check))goto restore;}
 if(changed_basic){memcpy(changed,basic,(unsigned)n);changed[1]=changed[2]=changed[3]=1;if(n>=45)changed[44]=1;changed[4]=0;changed[30]=1;changed[31]=2;
  attempted_basic=1;if(!measurement_write(1,changed,(unsigned)n))goto restore;
 }
 if(!measurement_verify(1,n,check)||!measurement_verify(2,m,check))goto restore;
 thermal_measurement=1;atomic_text(RAM "thermal.measurement",n>=45?"verified=1\ncenter=1\nmaximum=1\nminimum=1\nposition=tracking\n":"verified=1\ncenter=native_normal_mode\nmaximum=1\nminimum=1\nposition=tracking\n");return 1;
restore:
 if(attempted_basic)measurement_write(1,basic,(unsigned)n);
 if(attempted_mode)measurement_write(2,mode,(unsigned)m);
 atomic_text(RAM "thermal.measurement","verified=0\nresult=failed_and_restore_requested\n");return 0;
}
#ifndef DISC_THERMAL_MEASURE_TEST
static void measurement_command(void){
 char b[128],op[24];unsigned p,c,v;if(!text(RAM "thermal.command",b,sizeof(b))||sscanf(b,"%u %u %23s %u",&p,&c,op,&v)!=4||p!=owner||c!=token)return;
 /* A targeted regions read is diagnostic only. The old zero argument keeps
  * its basic/mode readback behavior. Never invent a short legacy response. */
 if(!strcmp(op,"measurement-query")){unsigned char data[512];unlink(RAM "thermal.command");if(v==3&&thermal_protocol==2)measurement_read(3,1,data);else if(v==0){measurement_read(1,1,data);measurement_read(2,1,data);}}
 else if(!strcmp(op,"measurement")){unlink(RAM "thermal.command");measurement_apply();}
}
#endif
