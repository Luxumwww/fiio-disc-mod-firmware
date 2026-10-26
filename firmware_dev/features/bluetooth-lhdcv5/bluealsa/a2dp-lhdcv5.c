/* Experimental glue against upstream BlueALSA v4.0.0. This is a reference
 * patch, NOT a replacement for the vendor daemon; its private extensions
 * and LDAC decoder must be preserved before device installation. */
#include "a2dp-lhdcv5.h"
#if ENABLE_LHDCV5
#include <errno.h>
#include <pthread.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include "disc_lhdcv5.h"
#include "io.h"
#include "shared/defs.h"
#include "shared/log.h"
#ifndef DISC_LHDCV5_ALLOW_192K
#define DISC_LHDCV5_ALLOW_192K 0
#endif
#define RATE_MASK (DISC_LHDCV5_ALLOW_192K ? 0x35 : 0x34)
struct a2dp_codec a2dp_lhdcv5_sink={
 .dir=A2DP_SINK,.codec_id=A2DP_CODEC_VENDOR_LHDC_V5,
 .capabilities.lhdcv5={.info=A2DP_SET_VENDOR_ID_CODEC_ID(BT_COMPID_SAVITECH,0x4c35),.data={RATE_MASK,0x86,0xd1,0,0}},
 .capabilities_size=sizeof(a2dp_lhdcv5_t),
};
uint32_t a2dp_lhdcv5_check(const struct a2dp_codec *codec,const void *data,size_t size){
 disc_lhdcv5_config cfg;const uint8_t *p=data;
 if(size!=11)return A2DP_CHECK_ERR_SIZE;
 if(disc_lhdcv5_parse_config(p,size,&cfg))return A2DP_CHECK_ERR_SAMPLING;
 if(!(p[6]&codec->capabilities.lhdcv5.data[0]))return A2DP_CHECK_ERR_SAMPLING;
 return A2DP_CHECK_OK;
}
int a2dp_lhdcv5_filter(const struct a2dp_codec *codec,void *data,size_t size){
 uint8_t *p=data;const uint8_t *local=(const uint8_t *)&codec->capabilities.lhdcv5;
 if(size!=11||memcmp(p,local,6)){errno=EINVAL;return -1;}
 p[6]&=local[6];p[7]=(p[7]&0xf0)|(p[7]&local[7]&6);
 /* Require the observed V5 version; optional effects are not implemented. */
 if((p[8]&15)!=1){errno=ENOTSUP;return -1;}
 p[8]=0x11;p[9]=p[10]=0;return 0;
}
int a2dp_lhdcv5_select(const struct a2dp_codec *codec,void *data,size_t size){
 if(a2dp_lhdcv5_filter(codec,data,size))return -1;
 uint8_t *p=data;unsigned rate=p[6]&0x35,bits=p[7]&6;
 p[6]=rate&1?1:rate&4?4:rate&0x10?0x10:rate&0x20?0x20:0;
 p[7]=(p[7]&0xf0)|(bits&2?2:bits&4?4:0);
 disc_lhdcv5_config cfg;return disc_lhdcv5_parse_config(p,size,&cfg);
}
void a2dp_lhdcv5_transport_init(struct ba_transport *t){
 disc_lhdcv5_config cfg;
 if(disc_lhdcv5_parse_config((const uint8_t *)&t->a2dp.configuration.lhdcv5,11,&cfg))return;
 t->a2dp.pcm.format=BA_TRANSPORT_PCM_FORMAT_S32_4LE;
 t->a2dp.pcm.channels=2;t->a2dp.pcm.sampling=cfg.rate;
}
static int emit_pcm(void *user,const int32_t *pcm,size_t frames){
 struct ba_transport_pcm *out=user;int32_t scaled[1920];size_t samples=frames*2;
 if(samples>ARRAYSIZE(scaled)){errno=EOVERFLOW;return -1;}
 memcpy(scaled,pcm,samples*sizeof(*pcm));io_pcm_scale(out,scaled,samples);
 size_t sent=0;
 while(sent<samples){ssize_t n=io_pcm_write(out,scaled+sent,samples-sent);if(n<=0)return -1;sent+=(size_t)n;}
 return 0;
}
static void *decoder_thread(struct ba_transport_thread *th){
 pthread_setcancelstate(PTHREAD_CANCEL_DISABLE,NULL);
 pthread_cleanup_push(PTHREAD_CLEANUP(ba_transport_thread_cleanup),th);
 struct ba_transport *t=th->t;struct io_poll io={.timeout=-1};uint8_t packet[8192];
 disc_lhdcv5 *dec=disc_lhdcv5_open((const uint8_t *)&t->a2dp.configuration.lhdcv5,11);
 if(!dec)goto fail_open;
 pthread_cleanup_push(PTHREAD_CLEANUP(disc_lhdcv5_close),dec);
 for(ba_transport_thread_set_state_running(th);;){
  ssize_t n=io_poll_and_read_bt(&io,th,packet,sizeof(packet));if(n<=0)break;
  if(!ba_transport_pcm_is_active(&t->a2dp.pcm)){disc_lhdcv5_flush(dec);continue;}
  if(disc_lhdcv5_packet(dec,packet,(size_t)n,emit_pcm,&t->a2dp.pcm)<0){
   /* Packet errors are counted by the adapter. No synchronous per-packet log.
    * A closing PCM consumer can recover on its next open without killing BT. */
   if(errno==EIO)disc_lhdcv5_flush(dec);
  }
 }
 ba_transport_thread_set_state_stopping(th);
 pthread_setcancelstate(PTHREAD_CANCEL_DISABLE,NULL);
 pthread_cleanup_pop(1);
fail_open:
 pthread_cleanup_pop(1);return NULL;
}
int a2dp_lhdcv5_transport_start(struct ba_transport *t){
 if(t->type.profile&BA_TRANSPORT_PROFILE_A2DP_SINK)
  return ba_transport_thread_create(&t->thread_dec,decoder_thread,"ba-a2dp-lhdcv5",true);
 errno=ENOTSUP;return -1;
}
#endif
