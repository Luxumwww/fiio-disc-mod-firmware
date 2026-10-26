/* Offline only: loaded into stock ffmpeg under explicitly invoked qemu-user.
 * No device access, codec registration or replacement of original libraries.
 * Function signatures follow arkq/openaptx include/openaptx.h.
 */
extern int printf(const char *, ...);
extern void *calloc(unsigned, unsigned);
extern void free(void *);
extern void *dlopen(const char *, int);
extern void *dlsym(void *, const char *);
extern char *dlerror(void);
extern int dlclose(void *);
extern void *avcodec_find_decoder_by_name(const char *);

static void wrapper(const char *file, const char *size_name,
                    const char *init_name, const char *destroy_name) {
    void *lib = dlopen(file, 2);
    if (!lib) { printf("DISC_PROBE load_error=%s\n", dlerror()); return; }
    unsigned (*size_fn)(void) = (unsigned (*)(void))dlsym(lib, size_name);
    int (*init_fn)(void *, short) = (int (*)(void *, short))dlsym(lib, init_name);
    void (*destroy_fn)(void *) = (void (*)(void *))dlsym(lib, destroy_name);
    if (!size_fn || !init_fn || !destroy_fn) {
        printf("DISC_PROBE missing_api=%s\n", file);
    } else {
        unsigned size = size_fn();
        if (size == 0 || size > 1048576) {
            printf("DISC_PROBE invalid_size=%u\n", size);
        } else {
            void *ctx = calloc(1, size);
            if (!ctx) { printf("DISC_PROBE allocation_failed\n"); dlclose(lib); return; }
            int status = init_fn(ctx, 0);
            printf("DISC_PROBE wrapper=%s size=%u init_status=%d\n", init_name, size, status);
            if (status == 0) destroy_fn(ctx);
            free(ctx);
        }
    }
    dlclose(lib);
}

__attribute__((constructor)) static void probe(void) {
    printf("DISC_PROBE ffmpeg_decoder_aptx=%d ffmpeg_decoder_aptx_hd=%d\n",
           avcodec_find_decoder_by_name("aptx") != 0,
           avcodec_find_decoder_by_name("aptx_hd") != 0);
    wrapper("/root/disc-firmware-dev-v257/stock/usr/lib/libaptx.so",
            "SizeofAptxbtdec", "aptxbtdec_init", "aptxbtdec_destroy");
    wrapper("/root/disc-firmware-dev-v257/stock/usr/lib/libaptxHD.so",
            "SizeofAptxhdbtdec", "aptxhdbtdec_init", "aptxhdbtdec_destroy");
}
