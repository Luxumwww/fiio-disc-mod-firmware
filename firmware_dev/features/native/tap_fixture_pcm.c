/* Computer-only tinyalsa fixture; never packaged for a player. */
extern int *__errno_location(void);
typedef struct {unsigned channels,rate,period,count,format;} Config;
typedef struct {Config config;unsigned flags,reads;int opened;} Pcm;
static Pcm handles[8];static int read_failure;
void disc_fixture_fail_read(int error){read_failure=error;}
void *pcm_open(unsigned card,unsigned device,unsigned flags,const void *config){
 (void)device;Pcm *p=&handles[card%8];p->config=*(const Config *)config;p->flags=flags;p->reads=0;p->opened=1;*__errno_location()=73;return p;
}
int pcm_close(void *handle){((Pcm *)handle)->opened=0;*__errno_location()=74;return 0;}
int pcm_is_ready(const void *handle){return ((const Pcm *)handle)->opened;}
unsigned pcm_get_channels(const void *handle){return ((const Pcm *)handle)->config.channels;}
unsigned pcm_get_rate(const void *handle){return ((const Pcm *)handle)->config.rate;}
int pcm_get_format(const void *handle){return ((const Pcm *)handle)->config.format;}
int pcm_format_to_bits(int format){return format==0?16:(format==5||format==6)?24:format==7?32:0;}
unsigned pcm_frames_to_bytes(const void *handle,unsigned frames){const Pcm *p=handle;return frames*p->config.channels*(p->config.format==0?2:p->config.format==5?3:4);}
int pcm_read(void *handle,void *buffer,unsigned bytes){
 Pcm *p=handle;p->reads++;
 if(read_failure){*__errno_location()=75;return read_failure;}
 for(unsigned i=0;i<bytes;i++)((unsigned char *)buffer)[i]=(unsigned char)(i*3+17);
 *__errno_location()=76;return 0;
}
unsigned disc_fixture_reads(const void *handle){return ((const Pcm *)handle)->reads;}
