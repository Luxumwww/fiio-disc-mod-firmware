/* Independent baseline wrapper from the pre-edit 1.14.0 source snapshot. */
#include "../reports/1.15.0/pre-edit/audio-dsp/warmgloss.h"
void *old_warmgloss_create(unsigned rate,const float *p){return disc_warmgloss_create(rate,p);}
void old_warmgloss_sample(void *s,float *io){disc_warmgloss_sample(s,io);}
void old_warmgloss_reset(void *s){disc_warmgloss_reset(s);}
void old_warmgloss_destroy(void *s){disc_warmgloss_destroy(s);}
unsigned old_warmgloss_memory(void *s){return disc_warmgloss_memory(s);}
