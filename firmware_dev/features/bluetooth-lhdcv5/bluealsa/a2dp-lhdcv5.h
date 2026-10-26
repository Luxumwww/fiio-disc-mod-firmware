/* DISC experimental BlueALSA 4.0.0 sink integration. MIT integration code. */
#pragma once
#include "a2dp.h"
#include "ba-transport.h"
#if ENABLE_LHDCV5
extern struct a2dp_codec a2dp_lhdcv5_sink;
uint32_t a2dp_lhdcv5_check(const struct a2dp_codec *,const void *,size_t);
int a2dp_lhdcv5_filter(const struct a2dp_codec *,void *,size_t);
int a2dp_lhdcv5_select(const struct a2dp_codec *,void *,size_t);
void a2dp_lhdcv5_transport_init(struct ba_transport *);
int a2dp_lhdcv5_transport_start(struct ba_transport *);
#endif
