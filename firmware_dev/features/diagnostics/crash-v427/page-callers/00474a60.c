
void FUN_00474a60(undefined4 param_1)

{
  undefined4 uVar1;
  int iVar2;
  char acStack_38 [36];
  
  DAT_008c7520 = param_1;
  DAT_008c7544 = FUN_004d992c(0);
  FUN_004db8f0(DAT_008c7544,0x168,0x168);
  uVar1 = DAT_008c7544;
  FUN_004e6d40(DAT_008c7544,0,0);
  FUN_004e6d50(uVar1,0,0);
  FUN_004e6d20(uVar1,0,0);
  FUN_004e6d30(uVar1,0,0);
  FUN_004e6e70(DAT_008c7544,0,0);
  FUN_004e6d80(DAT_008c7544,0xff000000,0);
  FUN_004dbe84(DAT_008c7544,1,0,0);
  FUN_004e7090(DAT_008c7544,0,0);
  FUN_004e6d60(DAT_008c7544,5,0);
  FUN_004e04b8(DAT_008c7544,0);
  FUN_004d3c7c(DAT_008c7544,FUN_00474898,1,0);
  FUN_004d3c7c(DAT_008c7544,FUN_00474898,2,0);
  FUN_004d3c7c(DAT_008c7544,FUN_00474898,8,0);
  thunk_FUN_00952500(DAT_008c7544);
  FUN_004f3160(DAT_008c7544,1);
  FUN_004f31a4(DAT_008c7544,0,0,2);
  uVar1 = FUN_0046f51c();
  iVar2 = FUN_00428900("/usr/project/config/ui/set_menu/system_setting.json",uVar1);
  if (iVar2 == 0) {
    FUN_0045a5bc(DAT_008c7544,&DAT_0068c6b0,0xb4fb);
  }
  else {
    FUN_0045a5bc(DAT_008c7544,*(undefined4 *)(*(int *)(iVar2 + 8) + 0x10),0xb4fb);
  }
  FUN_00428dd0(iVar2);
  uVar1 = FUN_0046f51c();
  iVar2 = FUN_00428900("/usr/project/config/ui/set_menu/others.json",uVar1);
  if (iVar2 == 0) {
    DAT_008c7528 = FUN_0045a33c(DAT_008c7544,&DAT_0068b094,0,&LAB_004747c0);
    DAT_008c752c = FUN_0045a33c(DAT_008c7544,"10min",1,&LAB_004747c0);
    DAT_008c7530 = FUN_0045a33c(DAT_008c7544,"30min",2,&LAB_004747c0);
    DAT_008c7534 = FUN_0045a33c(DAT_008c7544,"60min",3,&LAB_004747c0);
    DAT_008c7538 = FUN_0045a33c(DAT_008c7544,"90min",4,&LAB_004747c0);
    DAT_008c753c = FUN_0045a33c(DAT_008c7544,"120min",5,&LAB_004747c0);
  }
  else {
    DAT_008c7528 = FUN_0045a33c(DAT_008c7544,*(undefined4 *)(*(int *)(iVar2 + 8) + 0x104),0,
                                &LAB_004747c0);
    sprintf(acStack_38,"10%s",*(undefined4 *)(*(int *)(iVar2 + 8) + 0x140));
    DAT_008c752c = FUN_0045a33c(DAT_008c7544,acStack_38,1,&LAB_004747c0);
    sprintf(acStack_38,"30%s",*(undefined4 *)(*(int *)(iVar2 + 8) + 0x140));
    DAT_008c7530 = FUN_0045a33c(DAT_008c7544,acStack_38,2,&LAB_004747c0);
    sprintf(acStack_38,"60%s",*(undefined4 *)(*(int *)(iVar2 + 8) + 0x140));
    DAT_008c7534 = FUN_0045a33c(DAT_008c7544,acStack_38,3,&LAB_004747c0);
    sprintf(acStack_38,"90%s",*(undefined4 *)(*(int *)(iVar2 + 8) + 0x140));
    DAT_008c7538 = FUN_0045a33c(DAT_008c7544,acStack_38,4,&LAB_004747c0);
    sprintf(acStack_38,"120%s",*(undefined4 *)(*(int *)(iVar2 + 8) + 0x140));
    DAT_008c753c = FUN_0045a33c(DAT_008c7544,acStack_38,5,&LAB_004747c0);
  }
  FUN_00428dd0(iVar2);
  DAT_008c7540 = FUN_004d992c(DAT_008c7544);
  FUN_004db8f0(DAT_008c7540,0x168,0x28);
  FUN_004e6e70(DAT_008c7540,0,0);
  FUN_004e6e50(DAT_008c7540,0xff000000,0);
  FUN_004e6d90(DAT_008c7540,0,0);
  FUN_004e04b8(DAT_008c7540,0);
  FUN_004e0518(DAT_008c7540,0);
  FUN_0045a578((&DAT_008c7528)[DAT_008c8a85],1);
  FUN_004e298c((&DAT_008c7528)[DAT_008c8a85 + 1],1);
  return;
}

