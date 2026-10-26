#ifndef DISC_THERMAL_FRAME_H
#define DISC_THERMAL_FRAME_H
#define THERMAL_W 180u
#define THERMAL_H 240u
#define THERMAL_MAGIC 0x54485231u
typedef struct {
 unsigned magic,ui_pid,token;
 volatile unsigned sequence;
 unsigned width,height,frames,reserved;
 unsigned char pixels[THERMAL_W*THERMAL_H*4u];
} ThermalFrame;
#endif
