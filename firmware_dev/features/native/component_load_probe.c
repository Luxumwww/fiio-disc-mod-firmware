/* Computer-only load probe: use the stock executable's CRT/loader, then run the
 * native core self-test instead of starting its application main. Not shipped. */
extern void *dlsym(void *,const char *);
extern int disc_native_selftest(void);
static int probe_main(int argc,char **argv,char **env){
 (void)argc;(void)argv;(void)env;return disc_native_selftest();
}
int __libc_start_main(int (*mainfn)(int,char **,char **),int argc,char **argv,
 void (*init)(void),void (*fini)(void),void (*rtld_fini)(void),void *stack){
 (void)mainfn;
 int (*next)(int (*)(int,char **,char **),int,char **,void (*)(void),void (*)(void),void (*)(void),void *)=dlsym((void *)-1,"__libc_start_main");
 if(!next)return 90;
 return next(probe_main,argc,argv,init,fini,rtld_fini,stack);
}
