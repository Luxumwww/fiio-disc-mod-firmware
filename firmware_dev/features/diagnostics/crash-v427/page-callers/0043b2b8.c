
void FUN_0043b2b8(undefined4 param_1)

{
  undefined4 uVar1;
  int iVar2;
  
  DAT_008c5a90 = param_1;
  DAT_008c5aa4 = FUN_004d992c(0);
  FUN_004db8f0(DAT_008c5aa4,0x168,0x168);
  uVar1 = DAT_008c5aa4;
  FUN_004e6d40(DAT_008c5aa4,0,0);
  FUN_004e6d50(uVar1,0,0);
  FUN_004e6d20(uVar1,0,0);
  FUN_004e6d30(uVar1,0,0);
  FUN_004e6e70(DAT_008c5aa4,0,0);
  FUN_004e6d80(DAT_008c5aa4,0xff000000,0);
  FUN_004dbe84(DAT_008c5aa4,1,0,0);
  FUN_004e7090(DAT_008c5aa4,0,0);
  FUN_004e6d60(DAT_008c5aa4,5,0);
  FUN_004e04b8(DAT_008c5aa4,0);
  FUN_004d3c7c(DAT_008c5aa4,&LAB_0043aff0,1,0);
  FUN_004d3c7c(DAT_008c5aa4,&LAB_0043aff0,2,0);
  FUN_004d3c7c(DAT_008c5aa4,&LAB_0043aff0,8,0);
  thunk_FUN_00952500(DAT_008c5aa4);
  FUN_004f3160(DAT_008c5aa4,1);
  FUN_004f31a4(DAT_008c5aa4,0,0,2);
  uVar1 = FUN_0046f51c();
  iVar2 = FUN_00428900("/usr/project/config/ui/set_menu/system_setting.json",uVar1);
  if (iVar2 == 0) {
    FUN_0045a5bc(DAT_008c5aa4,&DAT_0068b238,0xb4fb);
  }
  else {
    FUN_0045a5bc(DAT_008c5aa4,*(undefined4 *)(*(int *)(iVar2 + 8) + 0xc),0xb4fb);
  }
  FUN_00428dd0(iVar2);
  DAT_008c5a94 = FUN_0045a33c(DAT_008c5aa4,"0",0,FUN_0043b04c);
  DAT_008c5a98 = FUN_0045a33c(DAT_008c5aa4,&DAT_0068b248,1,FUN_0043b04c);
  DAT_008c5a9c = FUN_0045a33c(DAT_008c5aa4,&DAT_0068b24c,2,FUN_0043b04c);
  DAT_008c5aa0 = FUN_0045a33c(DAT_008c5aa4,&DAT_0068b250,3,FUN_0043b04c);
  uVar1 = FUN_004d992c(DAT_008c5aa4);
  FUN_004db8f0(uVar1,0x140,0x28);
  FUN_004e6e70(uVar1,0,0);
  FUN_004e6e50(uVar1,0xff000000,0);
  FUN_004e6d90(uVar1,0,0);
  FUN_004e04b8(uVar1,0);
  FUN_004e0518(uVar1,0);
  FUN_004d9960(uVar1,0x4000);
  FUN_0045a578((&DAT_008c5a94)[DAT_008c8a7b],1);
  return;
}

