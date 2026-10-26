#include <stdio.h>
#include <stdlib.h>
int main(void){const char *value=getenv("LD_PRELOAD");printf("FIXTURE_PRELOAD=%s\n",value?value:"");return 0;}
