#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "usb_port_state.h"
static char state[1024];
static const char *make(unsigned mode,unsigned updated,const char *transition,const char *profile,const char *extra){
 snprintf(state,sizeof(state),"mode=%s\nusb_active=0\nwireless_active=1\npause_reason=%s\nrescue=0\nwork_mode=%u\nupdated=%u\ntransition=%s\nusb_priority=\n%s",profile,mode==0?"pure_usb_audio":"bt_usb_audio",mode,updated,transition,extra);return state;
}
int main(void){
 for(unsigned m=0;m<=3;m+=3){
  assert(usb_port_state_ready(make(m,102,"","wireless",""),m,100999,103250));
  assert(usb_port_state_ready(make(m,102,"","off",""),m,100999,103250));
  assert(!usb_port_state_ready(make(m,100,"","off",""),m,100999,103250));
  assert(!usb_port_state_ready(make(m,102,"debug","off",""),m,100999,103250));
  assert(!usb_port_state_ready(make(m,102,"wireless","wireless",""),m,100999,103250));
  assert(!usb_port_state_ready(make(m,102,"","both",""),m,100999,103250));
  assert(!usb_port_state_ready(make(m,102,"","off","usb_active=1\n"),m,100999,103250));
  assert(!usb_port_state_ready(make(m,102,"","off","usb_priority=active\n"),m,100999,103250));
  assert(!usb_port_state_ready(make(m,102,"","off","rescue=1\n"),m,100999,103250));
  assert(!usb_port_state_ready(make(m,102,"","off","pause_reason=\n"),m,100999,103250));
  assert(!usb_port_state_ready(make(m,102,"","off",""),m,100999,108000));
  assert(!usb_port_state_ready(make(m,104,"","off",""),m,100999,103250));
  assert(!usb_port_state_ready(make(m,102,"","off",""),3-m,100999,103250));
 }
 assert(!usb_port_state_ready(make(1,102,"","off",""),1,100999,103250));
 assert(!usb_port_state_ready(make(2,102,"","off",""),2,100999,103250));
 assert(!usb_state_is("xusb_active=0\n","usb_active","0"));
 assert(!usb_state_is("usb_active=0","usb_active","0"));
 assert(!usb_state_is("usb_active=0\nusb_active=0\n","usb_active","0"));
 unsigned n=0;assert(usb_state_unsigned("x=4294967295\n","x",&n)&&n==4294967295u);
 assert(!usb_state_unsigned("x=4294967296\n","x",&n));
 assert(!usb_state_unsigned("x=-1\n","x",&n));
 assert(!usb_state_unsigned("x=\n","x",&n));
 assert(!usb_state_unsigned("x=1junk\n","x",&n));
 puts("PASS: completed/fresh audio handoff, both modes, stale/transition/rescue/duplicate/truncated states rejected");
 return 0;
}
