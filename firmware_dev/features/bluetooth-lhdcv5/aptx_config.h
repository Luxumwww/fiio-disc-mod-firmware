/* DISC receiver integration, MIT. Decoder remains a separate LGPL library. */
#define APTX_ID 0x4fffu
#define APTX_HD_ID 0x24ffu
static uint8_t aptx_codecs[2][64] __attribute__((aligned(8)));
static uint32_t expanded_codecs[10];
static const uint8_t aptx_caps[2][11]={
 {0x4f,0,0,0,1,0,0x32,0,0,0,0},
 {0xd7,0,0,0,0x24,0,0x32,0,0,0,0}};
static int is_aptx_id(unsigned id){return id==APTX_ID||id==APTX_HD_ID;}
static int is_aptx(const void *c){return c&&is_aptx_id(U16(c,4));}
static unsigned aptx_rate(const uint8_t *cfg){return (cfg[6]&0xf0)==0x10?48000:(cfg[6]&0xf0)==0x20?44100:0;}
static int aptx_filter(const void *c,uint8_t *p,size_t n){
 unsigned hd=U16(c,4)==APTX_HD_ID;
 if(n!=(hd?11u:7u)||memcmp(p,aptx_caps[hd],6)){errno=EINVAL;return -1;}
 p[6]&=0x32;if(!(p[6]&0x30)||!(p[6]&2)){errno=ENOTSUP;return -1;}
 if(hd)memset(p+7,0,4);
 return 0;
}
static unsigned aptx_check(const void *c,const uint8_t *p,size_t n){
 unsigned hd=U16(c,4)==APTX_HD_ID;
 if(n!=(hd?11u:7u)||memcmp(p,aptx_caps[hd],6))return UINT32_MAX;
 if(!aptx_rate(p)||(p[6]&15)!=2)return 4;
 if(hd&&(p[7]||p[8]||p[9]||p[10]))return 4;
 return 0;
}
static int aptx_select(const void *c,uint8_t *p,size_t n){
 if(aptx_filter(c,p,n))return -1;
 p[6]=(p[6]&0x10?0x10:0x20)|2;return 0;
}
/* Every absolute stock table reference was enumerated and guarded. Use a
 * separate ten-pointer table, leaving all ROM table storage untouched. */
static void aptx_prepare_table(void){
 for(unsigned i=0;i<6;i++)expanded_codecs[i]=WORD(0x41acc8+i*4);
 expanded_codecs[6]=(uintptr_t)codec;
 for(unsigned i=0;i<2;i++){
  U32(aptx_codecs[i],0)=1;U16(aptx_codecs[i],4)=i?APTX_HD_ID:APTX_ID;
  memcpy(aptx_codecs[i]+7,aptx_caps[i],i?11:7);
  U32(aptx_codecs[i],24)=i?11:7;aptx_codecs[i][60]=1;
  expanded_codecs[7+i]=(uintptr_t)aptx_codecs[i];
 }
 uintptr_t p=(uintptr_t)expanded_codecs;unsigned hi=(p+0x8000)>>16,lo=p&65535;
 for(size_t i=0;i<sizeof(table_refs)/sizeof(table_refs[0]);i++){
  unsigned imm=table_refs[i].high?hi:lo;
  WORD(table_refs[i].address)=(table_refs[i].word&0xffff0000)|imm;
 }
}
