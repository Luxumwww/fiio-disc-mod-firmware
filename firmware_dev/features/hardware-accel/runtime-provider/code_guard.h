/* Only 16 bytes per reviewed text entry. Volatile byte reads prevent libc
 * IFUNC/PLT/lazy binding or over-reading in this first provider call. */
static int provider_code_equal(const volatile unsigned char *live,const unsigned char *expected,unsigned count){
 for(unsigned i=0;i<count;i++)if(live[i]!=expected[i])return 0;
 return 1;
}
