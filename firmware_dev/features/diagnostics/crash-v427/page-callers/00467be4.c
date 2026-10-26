
void FUN_00467be4(undefined4 param_1)

{
  undefined4 uVar1;
  int iVar2;
  undefined *puVar3;
  
  DAT_008c7248 = param_1;
  DAT_008c7258 = FUN_004d992c(0);
  FUN_004db8f0(DAT_008c7258,0x168,0x168);
  uVar1 = DAT_008c7258;
  FUN_004e6d40(DAT_008c7258,0,0);
  FUN_004e6d50(uVar1,0,0);
  FUN_004e6d20(uVar1,0,0);
  FUN_004e6d30(uVar1,0,0);
  FUN_004e6e70(DAT_008c7258,0,0);
  FUN_004e6d80(DAT_008c7258,0xff000000,0);
  FUN_004dbe84(DAT_008c7258,1,0,0);
  FUN_004e7090(DAT_008c7258,0,0);
  FUN_004e6d60(DAT_008c7258,5,0);
  FUN_004e04b8(DAT_008c7258,0);
  FUN_004d3c7c(DAT_008c7258,FUN_00467660,1,0);
  FUN_004d3c7c(DAT_008c7258,FUN_00467660,2,0);
  FUN_004d3c7c(DAT_008c7258,FUN_00467660,8,0);
  thunk_FUN_00952500(DAT_008c7258);
  FUN_004f3160(DAT_008c7258,1);
  FUN_004f31a4(DAT_008c7258,0,0,2);
  uVar1 = FUN_0046f51c();
  iVar2 = FUN_00428900("/usr/project/config/ui/set_menu/system_setting.json",uVar1);
  if (iVar2 == 0) {
    FUN_0045a5bc(DAT_008c7258,&DAT_0068a31c,0xb4fb);
  }
  else {
    FUN_0045a5bc(DAT_008c7258,*(undefined4 *)(*(int *)(iVar2 + 8) + 0x18),0xb4fb);
  }
  FUN_00428dd0(iVar2);
  uVar1 = FUN_0046f51c();
  iVar2 = FUN_00428900("/usr/project/config/ui/set_menu/others.json",uVar1);
  if (iVar2 == 0) {
    DAT_008c724c = FUN_0045a33c(DAT_008c7258,&DAT_0068a324,0,FUN_00467780);
    DAT_008c7250 = FUN_0045a33c(DAT_008c7258,&DAT_0068a32c,1,FUN_00467780);
    puVar3 = &DAT_0068a334;
  }
  else {
    DAT_008c724c = FUN_0045a33c(DAT_008c7258,**(undefined4 **)(iVar2 + 8),0,FUN_00467780);
    DAT_008c7250 = FUN_0045a33c(DAT_008c7258,*(undefined4 *)(*(int *)(iVar2 + 8) + 4),1,FUN_00467780
                               );
    puVar3 = *(undefined **)(*(int *)(iVar2 + 8) + 0x180);
  }
  DAT_008c7254 = FUN_0045a33c(DAT_008c7258,puVar3,2,FUN_00467780);
  FUN_0097d000(iVar2);
  uVar1 = FUN_004d992c(DAT_008c7258);
  FUN_004db8f0(uVar1,0x140,0x28);
  FUN_004e6e70(uVar1,0,0);
  FUN_004e6e50(uVar1,0xff000000,0);
  FUN_004e6d90(uVar1,0,0);
  FUN_004e04b8(uVar1,0);
  FUN_004e0518(uVar1,0);
  FUN_004d9960(uVar1,0x4000);
  FUN_0045a578((&DAT_008c724c)[DAT_008c8a8e],1);
  return;
}

