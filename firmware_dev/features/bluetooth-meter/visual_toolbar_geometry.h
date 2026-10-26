#ifndef DISC_VISUAL_TOOLBAR_GEOMETRY_H
#define DISC_VISUAL_TOOLBAR_GEOMETRY_H
/* Keep Bluetooth's existing spacing as the reference. These are offsets
 * inside the native brightness holder, not absolute screen coordinates. */
static int visual_toolbar_brightness_offset(int folded){return folded?100:184;}
#endif
