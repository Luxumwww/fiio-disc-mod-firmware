/* Read-only native PEQ encoder; called on the existing control tick, never PCM. */
static void remote_eq_player_tick(void){
 if(access("/usr/data/disc-runtime/components/disc-remote-control/enabled",0))return;
 char b[128];int f=open("/tmp/disc-runtime/remote-eq.request",0);if(f<0)return;int n=read(f,b,sizeof(b)-1);close(f);if(n<=0)return;b[n]=0;
 unsigned seq=0;char *at=b;while(*at>='0'&&*at<='9'){unsigned next=seq*10+(*at++-'0');if(next<seq)return;seq=next;}if(!seq||*at++!='\n')return;
 if((unsigned)(b+n-at)!=8||memcmp(at+4,"0008",4))return;
 int kind=!memcmp(at,"0639",4)?1:!memcmp(at,"0628",4)?2:!memcmp(at,"0629",4)?3:0;if(!kind)return;
 unlink("/tmp/disc-runtime/remote-eq.request");char value[192];int size=0,result=0;
 if(kind==1)size=snprintf(value,sizeof(value),"%04X",PC_WORD(0x898a0c));
 else {void *profile=0;char *bands=PC_FN(0x4ed0e0,char *(*)(void **))(&profile);
  if(!bands)result=-2;
  else if(kind==2){unsigned len=0;while(bands[len]&&len<145)len++;if(len!=144)result=-2;else size=snprintf(value,sizeof(value),"0000%s",bands);}
  else {float master=profile?*(const float *)((char *)profile+4):*(const float *)0x8a1470;size=snprintf(value,sizeof(value),"%04X",(unsigned)(unsigned short)(short)(int)(master*10.f));}
  if(bands)free(bands);
 }
 char out[256];int header=snprintf(out,sizeof(out),"%u\n",seq);if(!result){memcpy(out+header,value,size);remote_eq_file("/tmp/disc-runtime/remote-eq.reply","/tmp/disc-runtime/remote-eq.reply.next",out,header+size);}
 int count=snprintf(out,sizeof(out),"%u\n%d\n",seq,result);remote_eq_file("/tmp/disc-runtime/remote-eq.ack","/tmp/disc-runtime/remote-eq.ack.next",out,count);
}
