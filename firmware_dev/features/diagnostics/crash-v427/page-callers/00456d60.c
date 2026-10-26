
void FUN_00456d60(undefined4 param_1)

{
  undefined4 uVar1;
  
  DAT_00826078 = param_1;
  DAT_008c69c8 = FUN_004d992c(0);
  FUN_004d9a88(DAT_008c69c8,0x10);
  FUN_004e6d80(DAT_008c69c8,0xff000000,0);
  FUN_004e6d90(DAT_008c69c8,0xff,0);
  uVar1 = FUN_004d992c(DAT_008c69c8);
  FUN_004db8f0(uVar1,2,0x168);
  FUN_004e6d80(uVar1,0xffffffff,0);
  FUN_004dbe84(uVar1,9,0,0);
  uVar1 = FUN_004d992c(DAT_008c69c8);
  FUN_004db8f0(uVar1,0x168,2);
  FUN_004e6d80(uVar1,0xffffffff,0);
  FUN_004dbe84(uVar1,9,0,0);
  DAT_008c69cd = 1;
  FUN_004d3c7c(DAT_008c69c8,&LAB_00455db4,7,0);
  thunk_FUN_00952500(DAT_008c69c8);
  return;
}

