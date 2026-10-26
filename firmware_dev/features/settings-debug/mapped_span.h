/* Bound provider results by actual mappings before dereferencing/calling.
 * Caller supplies a complete maps line; no libc parser or map allocations. */
#ifndef DISC_MAPPED_SPAN_H
#define DISC_MAPPED_SPAN_H
typedef struct {unsigned low,high;int readable,executable;} DiscMappedSpan;
static int mapped_hex(const char **p,unsigned *out){
 unsigned value=0,count=0;while(count<8){unsigned char c=(unsigned char)**p;unsigned d=c>='0'&&c<='9'?c-'0':c>='a'&&c<='f'?c-'a'+10:c>='A'&&c<='F'?c-'A'+10:16;
  if(d>15)break;value=(value<<4)|d;(*p)++;count++;
 }if(!count)return 0;*out=value;return 1;
}
static int mapped_parse(const char *line,DiscMappedSpan *span){
 const char *p=line;if(!mapped_hex(&p,&span->low)||*p++!='-'||!mapped_hex(&p,&span->high)||*p++!=' '||span->high<=span->low)return 0;
 if(!p[0]||!p[1]||!p[2]||!p[3])return 0;
 span->readable=p[0]=='r';span->executable=p[2]=='x';return 1;
}
static int mapped_contains(const DiscMappedSpan *span,unsigned address,unsigned size,int executable){
 return size&&span->readable&&(!executable||span->executable)&&address>=span->low&&address<span->high&&size<=span->high-address;
}
#endif
