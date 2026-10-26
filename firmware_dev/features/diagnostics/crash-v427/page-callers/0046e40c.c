
void FUN_0046e40c(undefined4 param_1)

{
  undefined4 uVar1;
  undefined4 uVar2;
  int iVar3;
  
  DAT_00826078 = param_1;
  DAT_008c7418 = FUN_004d992c(0);
  FUN_004db8f0(DAT_008c7418,0x168,0x168);
  FUN_004dbe84(DAT_008c7418,9,0,0);
  FUN_004e6d80(DAT_008c7418,0xff000000,0);
  FUN_004e6e70(DAT_008c7418,0,0);
  FUN_004e7090(DAT_008c7418,0,0);
  FUN_004e04b8(DAT_008c7418,0);
  FUN_004d3c7c(DAT_008c7418,&LAB_0046e160,1,0);
  FUN_004d3c7c(DAT_008c7418,&LAB_0046e160,2,0);
  FUN_004d3c7c(DAT_008c7418,&LAB_0046e160,8,0);
  uVar1 = FUN_0051c364(DAT_008c7418);
  uVar2 = FUN_0046f51c();
  iVar3 = FUN_00428900("/usr/project/config/ui/set_menu/others.json",uVar2);
  if (iVar3 == 0) {
    thunk_FUN_0097a000(uVar1,&DAT_0068eee8);
  }
  else {
    thunk_FUN_0097a000(uVar1,*(undefined4 *)(*(int *)(iVar3 + 8) + 0x28));
  }
  FUN_00428dd0(iVar3);
  FUN_004db9cc(uVar1,0x140);
  FUN_0051d5d8(uVar1,0);
  FUN_004e4820(uVar1,&_fbss,0);
  FUN_004e7020(uVar1,0xffffffff,0);
  FUN_004dbe84(uVar1,2,0,0x66);
  FUN_0042ae68(DAT_008c7418,0xad,0x29,1,0x5e,0xb4,0xff212121,0xfffe0156,1,FUN_0046e1d0);
  FUN_0042ae68(DAT_008c7418,0xad,0x29,1,0x5e,0xfd,0xfffe0156,0xffffffff,0,&LAB_0046e3b8);
  thunk_FUN_00952500(DAT_008c7418);
  return;
}

