/* usb_active=0 is also published at the BEGINNING of teardown. Wait for a
 * completed off/wireless profile before changing role or enabling VBUS. */
static int thermal_port_state_ready(const char *s){
 return strstr(s,"\nusb_active=0\n")&&strstr(s,"\ntransition=\n")&&
  (!strncmp(s,"mode=off\n",9)||(!strncmp(s,"mode=wireless\n",14)&&strstr(s,"\nwireless_active=1\n")));
}
