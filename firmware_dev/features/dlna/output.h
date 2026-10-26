/* Native output handler closes PLAYER5, starts a new renderer, but leaves
 * the work mode at0. A following input command then cannot reap that renderer.
 * Stock network back sends output before input8: skip its redundant output. */
static int dlna_output_request(void *request,int *result){
 if(!request||!pc_enabled()||pc_workmode()!=5)return 0;
 unsigned output=((unsigned *)request)[4];
 if(output==PC_BYTE(0x83a75a)){*result=0;return 1;}
 *result=PC_FN(0x4f1294,int (*)(void *))(request);
 if(!*result&&PC_BYTE(0x83a759)==12&&pc_workmode()==0)
  PC_FN(0x453674,void (*)(unsigned))(5);
 return 1;
}
