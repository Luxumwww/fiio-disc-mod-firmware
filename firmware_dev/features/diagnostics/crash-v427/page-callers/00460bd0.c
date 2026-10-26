
void FUN_00460bd0(undefined4 param_1)

{
  undefined4 uVar1;
  int iVar2;
  
  DAT_008c6ff0 = param_1;
  DAT_008c6ff4 = FUN_004d992c(0);
  FUN_004db8f0(DAT_008c6ff4,0x168,0x168);
  uVar1 = DAT_008c6ff4;
  FUN_004e6d40(DAT_008c6ff4,0,0);
  FUN_004e6d50(uVar1,0,0);
  FUN_004e6d20(uVar1,0,0);
  FUN_004e6d30(uVar1,0,0);
  FUN_004e6e70(DAT_008c6ff4,0,0);
  FUN_004e6d80(DAT_008c6ff4,0xff000000,0);
  FUN_004dbe84(DAT_008c6ff4,1,0,0);
  FUN_004e7090(DAT_008c6ff4,0,0);
  FUN_004e6d60(DAT_008c6ff4,5,0);
  FUN_004e04b8(DAT_008c6ff4,0);
  FUN_004d3c7c(DAT_008c6ff4,&LAB_00460a40,1,0);
  FUN_004d3c7c(DAT_008c6ff4,&LAB_00460a40,2,0);
  FUN_004d3c7c(DAT_008c6ff4,&LAB_00460a40,8,0);
  thunk_FUN_00952500(DAT_008c6ff4);
  uVar1 = FUN_0046f51c();
  iVar2 = FUN_00428900("/usr/project/config/ui/set_menu/system_setting.json",uVar1);
  if (iVar2 == 0) {
    FUN_0045a5bc(DAT_008c6ff4,&DAT_0068dee8,0xb4fb);
  }
  else {
    FUN_0045a5bc(DAT_008c6ff4,*(undefined4 *)(*(int *)(iVar2 + 8) + 0x34),0xb4fb);
  }
  FUN_00428dd0(iVar2);
  FUN_0042ae68(DAT_008c6ff4,0xad,0x29,1,0x5e,0xb4,0xff212121,0xff00b4fb,1,FUN_00460a9c);
  FUN_0042ae68(DAT_008c6ff4,0xad,0x29,1,0x5e,0xfd,0xfffe0156,0xffffffff,0,&LAB_00460b84);
  thunk_FUN_00952500(DAT_008c6ff4);
  return;
}

