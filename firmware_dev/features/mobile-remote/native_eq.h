/* Uses the stock local FiiO message queue, never starts LAN servers. */
static int (*remote_mq_open)(const char *,int,...),(*remote_mq_send)(int,const char *,unsigned,unsigned),(*remote_mq_close)(int);
static void remote_eq_file(const char *path,const char *tmp,const char *data,unsigned size){int fd=open(tmp,1|0x100|0x200,0600);if(fd<0)return;int ok=write(fd,data,size)==(int)size;close(fd);if(ok)rename(tmp,path);else unlink(tmp);}
static void remote_eq_install(void){
 remote_mq_open=dlsym((void *)0,"mq_open");remote_mq_send=dlsym((void *)0,"mq_send");remote_mq_close=dlsym((void *)0,"mq_close");
 if(remote_mq_open&&remote_mq_send&&remote_mq_close){char ready[32];int n=snprintf(ready,sizeof(ready),"%d\n",getpid());remote_eq_file("/tmp/disc-runtime/remote-eq.ready","/tmp/disc-runtime/remote-eq.ready.next",ready,n);}
}
static void remote_eq_tick(void){
 if(!remote_mq_send||access("/tmp/disc-runtime/remote-eq.ready",0))remote_eq_install();
 if(!remote_mq_send||access("/usr/data/disc-runtime/components/disc-remote-control/enabled",0))return;
 char b[4096];int f=open("/tmp/disc-runtime/remote-eq.request",0);if(f<0)return;int n=read(f,b,sizeof(b)-1);close(f);if(n<=0)return;b[n]=0;
 unsigned seq=0;char *at=b;while(*at>='0'&&*at<='9'){unsigned next=seq*10+(*at++-'0');if(next<seq)return;seq=next;}if(!seq||*at++!='\n')return;
 char *frame=at;unsigned len=(unsigned)(b+n-at);if(len<8||len>2048)return;
 const char *tag=0,*expected=0;
 if(!memcmp(frame,"0639",4)){tag="0639";expected="a639";}else if(!memcmp(frame,"0628",4)){tag="0628";expected="a628";}else if(!memcmp(frame,"0629",4)){tag="0629";expected="a629";}
 else if(!memcmp(frame,"0690",4))tag="0690";else if(!memcmp(frame,"0678",4))tag="0678";else if(!memcmp(frame,"0630",4))tag="0630";
 if(!tag||expected)return;unlink("/tmp/disc-runtime/remote-eq.request");
 int q=remote_mq_open("/player",1|0x80),result=-1;if(q>=0){result=remote_mq_send(q,frame,len,0);remote_mq_close(q);}
 char ack[80];int count=snprintf(ack,sizeof(ack),"%u\n%d\n",seq,result);remote_eq_file("/tmp/disc-runtime/remote-eq.ack","/tmp/disc-runtime/remote-eq.ack.next",ack,count);
}
