/* Pinned glibc2.29 exports the original atexit only at GLIBC_2.0. */
#ifdef atexit
#undef atexit
#endif
#ifdef __mips__
extern int soxr_atexit_compat(void (*)(void));
__asm__(".symver soxr_atexit_compat,atexit@GLIBC_2.0");
int disc_soxr_register_exit(void (*fn)(void)){return soxr_atexit_compat(fn);}
#else
extern int atexit(void (*)(void));
int disc_soxr_register_exit(void (*fn)(void)){return atexit(fn);}
#endif
