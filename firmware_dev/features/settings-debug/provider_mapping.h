#include "mapped_span.h"
static int provider_mapped(const void *pointer,unsigned size,int executable,const char *owner){
 unsigned address=(unsigned)pointer;char block[512],line[600];unsigned used=0;int fd=open("/proc/self/maps",0),n,match=0,overflow=0;
 if(fd<0)return 0;
 while((n=read(fd,block,sizeof(block)))>0&&!match)for(int i=0;i<n&&!match;i++){
  char c=block[i];if(c=='\n'){
   if(!overflow){line[used]=0;DiscMappedSpan span;if(mapped_parse(line,&span)&&mapped_contains(&span,address,size,executable)){
    unsigned length=0;while(owner[length])length++;
    for(unsigned j=0;j+length<=used;j++)if(!memcmp(line+j,owner,length)){match=1;break;}
   }}used=0;overflow=0;
  }else if(used<sizeof(line)-1)line[used++]=c;else overflow=1;
 }
 close(fd);return match;
}
