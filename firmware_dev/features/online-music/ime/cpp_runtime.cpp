#include <stdlib.h>
#include <stddef.h>
void* operator new(size_t n){return malloc(n?n:1);}
void* operator new[](size_t n){return malloc(n?n:1);}
void operator delete(void*p)noexcept{free(p);}
void operator delete[](void*p)noexcept{free(p);}
void operator delete(void*p,size_t)noexcept{free(p);}
void operator delete[](void*p,size_t)noexcept{free(p);}
extern "C" void __cxa_pure_virtual(){abort();}
extern "C" unsigned long long __udivdi3(unsigned long long a,unsigned long long b){if(!b)abort();unsigned long long q=0,r=0;for(int i=63;i>=0;i--){bool carry=(r>>63)!=0;r=(r<<1)|((a>>i)&1);if(carry||r>=b){r-=b;q|=1ULL<<i;}}return q;}
