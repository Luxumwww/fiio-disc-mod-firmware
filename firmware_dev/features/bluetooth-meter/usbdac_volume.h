/* Native UI V2.57 selected output volume; player analogue cache can be stale.
 * Refs: 4208a0 / 4208c8 setters and 4844d8 native popup update. */
static unsigned disc_uac_volume(unsigned internal,unsigned external,unsigned selected){return selected==1?external:internal;}
