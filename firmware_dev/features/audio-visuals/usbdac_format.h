/* V2.57 digital worker input ring format, before resampling/DoP conversion. */
static int disc_uac_format(const unsigned *header,unsigned workmode,unsigned digital_state,unsigned *rate,unsigned *bits,unsigned *channels){
 if(workmode!=4||digital_state!=1||header[6]!=0||header[11]==2)return 0;
 if(header[3]<8000||header[3]>768000||(header[4]!=1&&header[4]!=2)||(header[5]!=16&&header[5]!=24&&header[5]!=32))return 0;
 *rate=header[3];*channels=header[4];*bits=header[5];return 1;
}
static int disc_uac_is_dop(const unsigned char *data,unsigned bits,unsigned channels,unsigned samples){
 if(bits==16||samples<16)return 0;unsigned stride=bits/8;
 for(unsigned marker=2;marker<stride;marker++){
  unsigned first=data[marker];if(first!=5&&first!=250)continue;int valid=1;
  for(unsigned i=0;i<16&&valid;i++)for(unsigned c=0;c<channels;c++)if(data[(i*channels+c)*stride+marker]!=(i&1?(first^255):first)){valid=0;break;}
  if(valid)return 1;
 }return 0;
}
static int disc_visual_mode(unsigned mode,int usbdac_enabled){return mode==3||(mode==1&&usbdac_enabled);}
static __attribute__((unused)) short disc_visual_sample(const unsigned char *data,unsigned bits,unsigned index){
 if(bits==24){unsigned sample=(unsigned)data[index*3]|((unsigned)data[index*3+1]<<8)|((unsigned)data[index*3+2]<<16);return (short)((int)(sample<<8)>>16);}
 return (short)(bits==32?((const int *)(const void *)data)[index]>>16:((const short *)(const void *)data)[index]);
}
