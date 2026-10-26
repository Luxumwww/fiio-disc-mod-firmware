/* Native set_y updates a style first; absolute coordinates remain from the
 * previous frame until update_layout (also called by native refresh). Resolve
 * it BEFORE deciding visibility, frozen-background mask or lease retirement. */
static unsigned shade_visible_height(LvObj *shade){
 if(!shade||!lv_valid(shade)||(*(unsigned *)((char *)shade+36)&1))return 0;
 FN(0x4ef6f4,void (*)(LvObj *))(shade);
 int height=*(int *)((char *)shade+24)+360;
 return height<0?0u:height>360?360u:(unsigned)height;
}
