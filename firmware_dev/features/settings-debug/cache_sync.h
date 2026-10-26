/* Once per newly mapped executable: no per-frame cache flush or new worker. */
extern long syscall(long,...);
#include "mapped_span.h"
static int cache_owned_span(const char *line,const char *owner,DiscMappedSpan *span){
 if(!mapped_parse(line,span)||!span->readable||!span->executable)return 0;
 unsigned n=0,length=0;while(line[n])n++;while(owner[length])length++;
 if(!length)return 0;
 for(unsigned i=0;i+length<=n;i++)if(!memcmp(line+i,owner,length)&&i>0&&line[i-1]==' '&&(i+length==n||line[i+length]==' '))return 1;
 return 0;
}
static __attribute__((always_inline)) inline int module_cache_sync(const char *owner){
 int fd=open("/proc/self/maps",0),count=0,n,ok=1,overflow=0;char block[512],line[600];unsigned used=0;
 if(fd<0)return 0;
 while(ok&&(n=read(fd,block,sizeof block))>0)for(int i=0;i<n&&ok;i++){
  char c=block[i];if(c=='\n'){
   if(!overflow){line[used]=0;DiscMappedSpan span;if(cache_owned_span(line,owner,&span)){
    /* Fault executable pages in as data before the MIPS cacheflush syscall;
     * invalidate instructions after page-in/decompression and data writes. */
    volatile unsigned char touch=0;
    for(unsigned address=span.low;address<span.high;address+=4096)touch^=*(const volatile unsigned char *)address;
    (void)touch;
    if(syscall(4147,(unsigned long)span.low,(unsigned long)(span.high-span.low),3)!=0)ok=0;else count++;
   }}used=0;overflow=0;
  }else if(used<sizeof line-1)line[used++]=c;else overflow=1;
 }
 close(fd);return ok&&count>0;
}
