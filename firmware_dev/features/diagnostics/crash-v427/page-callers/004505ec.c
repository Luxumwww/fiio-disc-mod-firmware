
void FUN_004505ec(undefined4 param_1)

{
  undefined4 uVar1;
  int iVar2;
  
  DAT_008c68a0 = param_1;
  DAT_008c68a4 = FUN_004d992c(0);
  FUN_004db8f0(DAT_008c68a4,0x168,0x168);
  FUN_004e7090(DAT_008c68a4,0xb4,0);
  uVar1 = DAT_008c68a4;
  FUN_004e6d40(DAT_008c68a4,0,0);
  FUN_004e6d50(uVar1,0,0);
  FUN_004e6d20(uVar1,0,0);
  FUN_004e6d30(uVar1,0,0);
  FUN_004e6e70(DAT_008c68a4,0,0);
  FUN_004e6d80(DAT_008c68a4,0xff000000,0);
  FUN_004f3160(DAT_008c68a4,1);
  FUN_004f31a4(DAT_008c68a4,0,0,2);
  FUN_004dbe84(DAT_008c68a4,1,0,0);
  FUN_004e7090(DAT_008c68a4,0,0);
  FUN_004e6d60(DAT_008c68a4,5,0);
  FUN_004e04b8(DAT_008c68a4,0);
  FUN_004e0518(DAT_008c68a4,0xc);
  FUN_004d3c7c(DAT_008c68a4,FUN_00450340,1,0);
  FUN_004d3c7c(DAT_008c68a4,FUN_00450340,2,0);
  FUN_004d3c7c(DAT_008c68a4,FUN_00450340,8,0);
  uVar1 = FUN_0046f51c();
  iVar2 = FUN_00428900("/usr/project/config/ui/set_menu/system_setting.json",uVar1);
  if (iVar2 == 0) {
    FUN_0045a5bc(DAT_008c68a4,&DAT_0068c6d0,0xb4fb);
  }
  else {
    FUN_0045a5bc(DAT_008c68a4,*(undefined4 *)(*(int *)(iVar2 + 8) + 0x24),0xb4fb);
  }
  FUN_00428dd0(iVar2);
  uVar1 = FUN_0046f51c();
  iVar2 = FUN_00428900("/usr/project/config/ui/set_menu/others.json",uVar1);
  if (iVar2 == 0) {
    FUN_0045039c(DAT_008c68a4,&DAT_0068c9a4,0);
    FUN_0045039c(DAT_008c68a4,&DAT_0068c9ac,1);
    FUN_0045039c(DAT_008c68a4,&DAT_0068aea8,2);
  }
  else {
    FUN_0045039c(DAT_008c68a4,*(undefined4 *)(*(int *)(iVar2 + 8) + 0x16c),0);
    FUN_0045039c(DAT_008c68a4,*(undefined4 *)(*(int *)(iVar2 + 8) + 0x170),1);
    FUN_0045039c(DAT_008c68a4,*(undefined4 *)(*(int *)(iVar2 + 8) + 0x174),2);
  }
  FUN_00428dd0(iVar2);
  thunk_FUN_00952500(DAT_008c68a4);
  return;
}

