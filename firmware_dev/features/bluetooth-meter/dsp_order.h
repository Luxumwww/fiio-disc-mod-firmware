/* Stable effect IDs. Old7/8 orders retain relative order; new IDs append. */
#define DSP_ORDER_COUNT 10
#define DSP_ROW_TOP 78
#define DSP_ROW_PITCH 54
static void dsp_order_default(unsigned order[DSP_ORDER_COUNT]){for(unsigned i=0;i<7;i++)order[i]=i?i+1:0;order[7]=13;order[8]=14;order[9]=15;}
static int dsp_order_parse(const char *text,unsigned order[DSP_ORDER_COUNT]){
 unsigned candidate[16],seen=0,n=0;const char *p=text;
 while(1){while(*p==' '||*p=='\t'||*p=='\r'||*p=='\n')p++;if(!*p)break;if(n>=16||*p<'0'||*p>'9')return 0;
 unsigned id=0;do{id=id*10+(unsigned)(*p++-'0');if(id>15)return 0;}while(*p>='0'&&*p<='9');
 if((seen&(1u<<id))||(*p&&*p!=' '&&*p!='\t'&&*p!='\r'&&*p!='\n'))return 0;seen|=1u<<id;candidate[n++]=id;}
 if(!((n==7&&seen==253)||(n==8&&seen==255)||(n==11&&(seen==4093||seen==2047))||(n==12&&seen==4095)||(n==8&&seen==8445)||(n==9&&seen==24829)||(n==10&&seen==57597)))return 0;
 unsigned dst=0;for(unsigned i=0;i<n;i++)if((candidate[i]!=1&&candidate[i]<8)||candidate[i]==13||candidate[i]==14||candidate[i]==15)order[dst++]=candidate[i];
 if(!(seen&8192u))order[dst++]=13;if(!(seen&16384u))order[dst++]=14;if(!(seen&32768u))order[dst++]=15;return dst==DSP_ORDER_COUNT;
}
static unsigned dsp_order_position(const unsigned order[DSP_ORDER_COUNT],unsigned id){for(unsigned i=0;i<DSP_ORDER_COUNT;i++)if(order[i]==id)return i;return 0;}
static int dsp_order_move(unsigned order[DSP_ORDER_COUNT],unsigned id,int to){
 if((id>=8&&id!=13&&id!=14&&id!=15)||id==1)return 0;if(to<0)to=0;if(to>=DSP_ORDER_COUNT)to=DSP_ORDER_COUNT-1;
 unsigned from=dsp_order_position(order,id);if(from==(unsigned)to)return 0;
 if(from<(unsigned)to)for(unsigned i=from;i<(unsigned)to;i++)order[i]=order[i+1];else for(unsigned i=from;i>(unsigned)to;i--)order[i]=order[i-1];order[to]=id;return 1;
}
static int dsp_order_slot(int center_y){int to=(center_y-DSP_ROW_TOP)/DSP_ROW_PITCH;return to<0?0:to>=DSP_ORDER_COUNT?DSP_ORDER_COUNT-1:to;}
