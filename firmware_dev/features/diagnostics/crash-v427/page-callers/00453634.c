
void FUN_00453634(undefined4 param_1)

{
  undefined4 uVar1;
  int iVar2;
  undefined *puVar3;
  
  DAT_008c692c = param_1;
  DAT_008c6944 = FUN_004d992c(0);
  FUN_004db8f0(DAT_008c6944,0x168,0x168);
  uVar1 = DAT_008c6944;
  FUN_004e6d40(DAT_008c6944,0,0);
  FUN_004e6d50(uVar1,0,0);
  FUN_004e6d20(uVar1,0,0);
  FUN_004e6d30(uVar1,0,0);
  FUN_004e6e70(DAT_008c6944,0,0);
  FUN_004e6d80(DAT_008c6944,0xff000000,0);
  FUN_004dbe84(DAT_008c6944,1,0,0);
  FUN_004e7090(DAT_008c6944,0,0);
  FUN_004e6d60(DAT_008c6944,5,0);
  FUN_004d3c7c(DAT_008c6944,&LAB_00452778,1,0);
  FUN_004d3c7c(DAT_008c6944,&LAB_00452778,2,0);
  FUN_004d3c7c(DAT_008c6944,&LAB_00452778,8,0);
  thunk_FUN_00952500(DAT_008c6944);
  FUN_004f3160(DAT_008c6944,1);
  FUN_004f31a4(DAT_008c6944,0,0,2);
  uVar1 = FUN_0046f51c();
  iVar2 = FUN_00428900("/usr/project/config/ui/set_menu/system.json",uVar1);
  if (iVar2 == 0) {
    FUN_0045a5bc(DAT_008c6944,&DAT_0068cc1c,0xff7e00);
  }
  else {
    FUN_0045a5bc(DAT_008c6944,*(undefined4 *)(*(int *)(iVar2 + 8) + 0x18),0xff7e00);
  }
  FUN_00428dd0(iVar2);
  uVar1 = FUN_0046f51c();
  iVar2 = FUN_00428900("/usr/project/config/ui/set_menu/others.json",uVar1);
  if (iVar2 == 0) {
    DAT_008c693c = FUN_0045a33c(DAT_008c6944,&DAT_0068cc2c,0,FUN_0045348c);
    puVar3 = &DAT_0068cc3c;
  }
  else {
    DAT_008c693c = FUN_0045a33c(DAT_008c6944,*(undefined4 *)(*(int *)(iVar2 + 8) + 0x5c),0,
                                FUN_0045348c);
    puVar3 = *(undefined **)(*(int *)(iVar2 + 8) + 0x60);
  }
  DAT_008c6940 = FUN_0045a33c(DAT_008c6944,puVar3,1,FUN_0045348c);
  FUN_00428dd0(iVar2);
  uVar1 = FUN_004e8a80(DAT_008c693c,1);
  FUN_004d9960(uVar1,1);
  if (DAT_00825d50 == 0) {
    return;
  }
  FUN_0045a578((&DAT_008c693c)[DAT_00825d50],1);
  return;
}

