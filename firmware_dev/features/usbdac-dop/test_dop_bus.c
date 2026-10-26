#include <assert.h>
#include <stdio.h>
#include <string.h>
static unsigned char regs[2][12]={{0x43,0x13,0x10,7,0x54,31,0,63,0,0x0c,0x0a,0xf2},{0x43,0x13,0x10,7,0x54,31,0,63,0,0x0c,0x0a,0xf2}};
static unsigned writes,opens,write_attempts,fail_write;static int fail_bus,bad_device;
static int open(const char *p,int mode,...){(void)mode;opens++;return strstr(p,"i2c-3")?3:strstr(p,"0030")?1:2;}
static int read(int f,void *b,unsigned size){const char *s=bad_device?"unknown\n":f==1?"cs43131\n":"cs43131c\n";assert(strlen(s)<size);memcpy(b,s,strlen(s));return strlen(s);}
static int close(int f){(void)f;return 0;}
#include "dop_bus.h"
static int ioctl_mock(int fd,unsigned request,void *arg){
 assert(fd==3&&request==0x707);if(fail_bus)return -1;struct DopTransfer *x=arg;
 struct DopMessage *m=x->messages;assert(m[0].address==0x30||m[0].address==0x32);unsigned c=(m[0].address-0x30)/2;
 unsigned char *b=m[0].buffer;assert(b[3]==0);unsigned reg=(b[0]<<16)|(b[1]<<8)|b[2],i;
 const unsigned rr[]={0x10000,0x10001,0x10002,0x1000b,0x70004,0x40014,0x40015,0x40016,0x40017,0x40018,0x40019,0x20000};
 i=12;for(unsigned n=0;n<12;n++)if(rr[n]==reg)i=n;assert(i<12);
 if(x->count==2){assert(m[0].length==4&&m[1].flags==1&&m[1].length==1&&m[1].address==m[0].address);*m[1].buffer=regs[c][i];return 2;}
 assert(x->count==1&&m[0].length==5&&i>=3);write_attempts++;regs[c][i]=b[4];
 if(write_attempts==fail_write)return -1; /* may latch before failed ACK */
 writes++;return 1;
}
int main(void){
 dop_real_ioctl=ioctl_mock;assert(dop_bus_check(352800,0)&&writes==0);
 assert(!dop_bus_check(176400,0)&&writes==0);assert(dop_bus_check(176400,1)&&writes==4);
 assert(regs[0][3]==5&&regs[1][3]==5&&regs[0][4]==0x50&&regs[1][4]==0x50);
 for(int i=0;i<100;i++)assert(dop_bus_check(176400,1));assert(writes==4);
 assert(dop_bus_check(352800,1)&&writes==8);regs[0][4]=0x56;regs[1][4]=0x56;
 assert(dop_bus_check(176400,1)&&regs[0][4]==0x52&&regs[1][4]==0x52); /* Preserve unrelated detector flags. */
 unsigned old=writes;regs[1][0]=0;assert(!dop_bus_check(352800,1)&&writes==old);regs[1][0]=0x43;
 fail_bus=1;assert(!dop_bus_check(352800,1)&&writes==old);fail_bus=0;bad_device=1;assert(!dop_bus_check(352800,1)&&writes==old);
 bad_device=0;
 for(unsigned c=0;c<2;c++){regs[c][5]=23;regs[c][7]=47;regs[c][11]=0xa2;}
 assert(!dop_bus_check(176400,0)&&writes==old);
 assert(!dop_bus_check(176400,1)&&writes==old);
 regs[0][11]=0xf2;assert(!dop_bus_check(176400,1)&&writes==old);
 regs[1][11]=0xf2;assert(dop_bus_check(176400,1)&&writes==old+4&&dop_frame_repairs==2);
 for(unsigned c=0;c<2;c++){assert(regs[c][5]==31&&regs[c][7]==63);regs[c][11]=0xa2;}
 assert(dop_bus_check(176400,0));assert(dop_bus_check(176400,1)&&writes==old+4);
 for(unsigned c=0;c<2;c++){regs[c][5]=23;regs[c][7]=47;regs[c][11]=0xf2;}
 unsigned char saved[2][12];memcpy(saved,regs,sizeof regs);fail_write=write_attempts+3;
 assert(!dop_bus_check(176400,1)&&!memcmp(saved,regs,sizeof regs));fail_write=0;
 regs[1][9]=0x1c;old=writes;assert(!dop_bus_check(176400,1)&&writes==old);regs[1][9]=0x0c;
 assert(!dop_bus_check(705600,1)&&writes==old);
 puts("PASS: two-chip preflight; 64/128 rate/speed; 48-to-64 clock frame repaired only while both ASPs powered down; unchanged healthy frames; foreign framing rejected; failed/latched writes restored within muted transaction");
}
