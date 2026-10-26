/* Pure bounded replay: S32_LE left-aligned 24-bit DoP, stereo. */
static int ram_packet_valid(const unsigned char *p,unsigned n){
 if(!p||n<256||(n&7))return 0;
 unsigned first=p[3];if(first!=5&&first!=250)return 0;
 for(unsigned i=0;i<n;i+=8){unsigned m=((i/8)&1)?first^255:first;
  if(p[i]||p[i+4]||p[i+3]!=m||p[i+7]!=m)return 0;
 }
 return 1;
}
static void ram_packet_copy(unsigned char *out,const unsigned char *clip,unsigned size,unsigned pos,unsigned frames,unsigned marker){
 for(unsigned i=0;i<frames;i++){
  const unsigned char *p=clip+pos;
  for(unsigned j=0;j<8;j++)out[i*8+j]=p[j];
  unsigned m=(i&1)?marker^255:marker;out[i*8+3]=(unsigned char)m;out[i*8+7]=(unsigned char)m;
  pos+=8;if(pos==size)pos=0;
 }
}
static unsigned ram_packet_hash(const unsigned char *p,unsigned n){unsigned h=2166136261u;for(unsigned i=0;i<n;i++)h=(h^p[i])*16777619u;return h;}
