/* Shared by the actual UI and the offline transition tests. The runtime
 * publishes usb_active=0 before teardown; only a fresh, completed audio
 * reservation may transfer the physical port to the native player. */
static int usb_state_value(const char *s,const char *key,char *out,unsigned capacity){
 unsigned found=0;
 if(!capacity)return 0;
 while(*s){
  const char *row=s;unsigned n=0,k=0;
  while(s[n]&&s[n]!='\n')n++;
  if(s[n]!='\n')return 0;
  while(key[k]&&k<n&&row[k]==key[k])k++;
  if(!key[k]&&k<n&&row[k]=='='){
   if(found++||n-k>=capacity)return 0;
   unsigned j=0;for(k++;k<n;k++)out[j++]=row[k];out[j]=0;
  }
  s+=n+1;
 }
 return found==1;
}
static int usb_state_is(const char *s,const char *key,const char *value){
 char b[80];return usb_state_value(s,key,b,sizeof(b))&&!strcmp(b,value);
}
static int usb_state_unsigned(const char *s,const char *key,unsigned *value){
 char b[24];if(!usb_state_value(s,key,b,sizeof(b))||!b[0])return 0;
 unsigned n=0;for(unsigned i=0;b[i];i++){
  if(b[i]<'0'||b[i]>'9'||n>429496729u||(n==429496729u&&b[i]>'5'))return 0;
  n=n*10+(unsigned)(b[i]-'0');
 }
 *value=n;return 1;
}
static int usb_port_state_ready(const char *s,unsigned mode,unsigned requested_ms,unsigned now_ms){
 unsigned updated,workmode;
 if(mode!=0&&mode!=3)return 0;
 if(!usb_state_unsigned(s,"updated",&updated)||!usb_state_unsigned(s,"work_mode",&workmode)||workmode!=mode)return 0;
 /* Require a publication in a later second than the request, rather than
  * accepting an old state that happened to be written in the same second. */
 if(updated<=requested_ms/1000u||updated>now_ms/1000u||now_ms/1000u-updated>5)return 0;
 if(!usb_state_is(s,"pause_reason",mode==0?"pure_usb_audio":"bt_usb_audio")||
    !usb_state_is(s,"usb_active","0")||!usb_state_is(s,"transition","")||
    !usb_state_is(s,"rescue","0")||!usb_state_is(s,"usb_priority",""))return 0;
 return usb_state_is(s,"mode","off")||
  (usb_state_is(s,"mode","wireless")&&usb_state_is(s,"wireless_active","1"));
}
