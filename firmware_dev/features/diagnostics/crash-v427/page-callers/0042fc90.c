
void FUN_0042fc90(undefined4 param_1)

{
  undefined4 uVar1;
  int iVar2;
  undefined *puVar3;
  
  if (DAT_008c5020 != 0) {
    thunk_FUN_00952500(DAT_008c5020);
    return;
  }
  DAT_008c5000 = param_1;
  DAT_008c5020 = FUN_004d992c(0);
  FUN_004db8f0(DAT_008c5020,0x168,0x168);
  uVar1 = DAT_008c5020;
  FUN_004e6d40(DAT_008c5020,0,0);
  FUN_004e6d50(uVar1,0,0);
  FUN_004e6d20(uVar1,0,0);
  FUN_004e6d30(uVar1,0,0);
  FUN_004e6e70(DAT_008c5020,0,0);
  FUN_004e6d80(DAT_008c5020,0xff000000,0);
  FUN_004dbe84(DAT_008c5020,1,0,0);
  FUN_004e7090(DAT_008c5020,0,0);
  FUN_004e6d60(DAT_008c5020,5,0);
  FUN_004e04b8(DAT_008c5020,0);
  FUN_004d3c7c(DAT_008c5020,FUN_0042fb20,1,0);
  FUN_004d3c7c(DAT_008c5020,FUN_0042fb20,2,0);
  FUN_004d3c7c(DAT_008c5020,FUN_0042fb20,8,0);
  thunk_FUN_00952500(DAT_008c5020);
  FUN_004f3160(DAT_008c5020,1);
  FUN_004f31a4(DAT_008c5020,0,0,2);
  uVar1 = FUN_0046f51c();
  iVar2 = FUN_00428900("/usr/project/config/ui/set_menu/system_setting.json",uVar1);
  if (iVar2 == 0) {
    FUN_0045a5bc(DAT_008c5020,&DAT_0068a31c,0xb4fb);
  }
  else {
    FUN_0045a5bc(DAT_008c5020,*(undefined4 *)(*(int *)(iVar2 + 8) + 0x1c),0xb4fb);
  }
  FUN_00428dd0(iVar2);
  uVar1 = FUN_0046f51c();
  iVar2 = FUN_00428900("/usr/project/config/ui/set_menu/others.json",uVar1);
  if (iVar2 == 0) {
    DAT_008c5008 = FUN_0045a33c(DAT_008c5020,&DAT_0068a324,0,&DAT_0042fb7c);
    DAT_008c500c = FUN_0045a33c(DAT_008c5020,&DAT_0068a32c,1,&DAT_0042fb7c);
    DAT_008c5010 = FUN_0045a33c(DAT_008c5020,&DAT_0068a334,2,&DAT_0042fb7c);
    DAT_008c5014 = FUN_0045a33c(DAT_008c5020,&DAT_0068a33c,3,&DAT_0042fb7c);
    DAT_008c5018 = FUN_0045a33c(DAT_008c5020,&DAT_0068a344,4,&DAT_0042fb7c);
    puVar3 = &DAT_0068a34c;
  }
  else {
    DAT_008c5008 = FUN_0045a33c(DAT_008c5020,*(undefined4 *)(*(int *)(iVar2 + 8) + 0x3c),0,
                                &DAT_0042fb7c);
    DAT_008c500c = FUN_0045a33c(DAT_008c5020,*(undefined4 *)(*(int *)(iVar2 + 8) + 0x40),1,
                                &DAT_0042fb7c);
    DAT_008c5010 = FUN_0045a33c(DAT_008c5020,*(undefined4 *)(*(int *)(iVar2 + 8) + 0x44),2,
                                &DAT_0042fb7c);
    DAT_008c5014 = FUN_0045a33c(DAT_008c5020,*(undefined4 *)(*(int *)(iVar2 + 8) + 0x48),3,
                                &DAT_0042fb7c);
    DAT_008c5018 = FUN_0045a33c(DAT_008c5020,*(undefined4 *)(*(int *)(iVar2 + 8) + 0x4c),4,
                                &DAT_0042fb7c);
    puVar3 = *(undefined **)(*(int *)(iVar2 + 8) + 0x164);
  }
  DAT_008c501c = FUN_0045a33c(DAT_008c5020,puVar3,5,&DAT_0042fb7c);
  FUN_00963000(iVar2);
  uVar1 = FUN_004d992c(DAT_008c5020);
  FUN_004db8f0(uVar1,0x140,0x28);
  FUN_004e6e70(uVar1,0,0);
  FUN_004e6e50(uVar1,0xff000000,0);
  FUN_004e6d90(uVar1,0,0);
  FUN_004e04b8(uVar1,0);
  FUN_004e0518(uVar1,0);
  FUN_004d9960(uVar1,0x4000);
  FUN_00963600((&DAT_008c5008)[DAT_008c5004],1);
  return;
}

