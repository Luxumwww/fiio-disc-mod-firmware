
void FUN_0046dc90(undefined4 param_1)

{
  int iVar1;
  void *pvVar2;
  undefined4 in_stack_00000018;
  
  if (DAT_008c73f0 == 0) {
    FUN_0046d860();
  }
  thunk_FUN_00952500(DAT_008c73f0);
  DAT_008c73e4 = param_1;
  if (DAT_008c73e1 == '\0') {
    iVar1 = FUN_0042403c();
    DAT_008c73f8 = iVar1;
    if (iVar1 != 0) {
      DAT_008c73fc = 0;
      in_stack_00000018 = 0;
      pvVar2 = malloc(iVar1 * 0xc);
      DAT_008c7400 = pvVar2;
      DAT_008c73f4 = malloc(iVar1 << 2);
      FUN_00422dd4(pvVar2,iVar1,0,&stack0x00000018);
      iVar1 = FUN_004245cc(DAT_008c507c);
      if (iVar1 < 1) {
        iVar1 = DAT_008c507c;
      }
      DAT_008c73dc = iVar1 + -1;
      FUN_0046d240();
    }
    DAT_008c73e1 = 1;
    return;
  }
  return;
}

