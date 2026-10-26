
void FUN_00435834(undefined4 param_1)

{
  undefined4 uVar1;
  int iVar2;
  char *pcVar3;
  char acStack_38 [32];
  
  DAT_008c5964 = param_1;
  DAT_008c5994 = FUN_004d992c(0);
  FUN_004db8f0(DAT_008c5994,0x168,0x168);
  uVar1 = DAT_008c5994;
  FUN_004e6d40(DAT_008c5994,0,0);
  FUN_004e6d50(uVar1,0,0);
  FUN_004e6d20(uVar1,0,0);
  FUN_004e6d30(uVar1,0,0);
  FUN_004e6e70(DAT_008c5994,0,0);
  FUN_004e6d80(DAT_008c5994,0xff000000,0);
  FUN_004dbe84(DAT_008c5994,1,0,0);
  FUN_004e7090(DAT_008c5994,0,0);
  FUN_004e6d60(DAT_008c5994,5,0);
  FUN_004e04b8(DAT_008c5994,0);
  FUN_004d3c7c(DAT_008c5994,FUN_0043565c,1,0);
  FUN_004d3c7c(DAT_008c5994,FUN_0043565c,2,0);
  FUN_004d3c7c(DAT_008c5994,FUN_0043565c,8,0);
  FUN_004f3160(DAT_008c5994,1);
  FUN_004f31a4(DAT_008c5994,0,0,2);
  FUN_004e0518(DAT_008c5994,0xc);
  uVar1 = FUN_0046f51c();
  iVar2 = FUN_00428900("/usr/project/config/ui/set_menu/system_setting.json",uVar1);
  if (iVar2 == 0) {
    FUN_0045a5bc(DAT_008c5994,&DAT_0068acc0,0xb4fb);
  }
  else {
    FUN_0045a5bc(DAT_008c5994,*(undefined4 *)(*(int *)(iVar2 + 8) + 8),0xb4fb);
  }
  FUN_00428dd0(iVar2);
  uVar1 = FUN_0046f51c();
  iVar2 = FUN_00428900("/usr/project/config/ui/set_menu/others.json",uVar1);
  if (iVar2 == 0) {
    DAT_008c5968 = FUN_0045a33c(DAT_008c5994,&PTR_DAT_0068acd0,0,&DAT_004354b0);
    DAT_008c596c = FUN_0045a33c(DAT_008c5994,&PTR_DAT_0068acd4,1,&DAT_004354b0);
    DAT_008c5970 = FUN_0045a33c(DAT_008c5994,&PTR_DAT_0068acd8,2,&DAT_004354b0);
    DAT_008c5974 = FUN_0045a33c(DAT_008c5994,&DAT_0068acdc,3,&DAT_004354b0);
    DAT_008c5978 = FUN_0045a33c(DAT_008c5994,&DAT_0068ace4,4,&DAT_004354b0);
    DAT_008c597c = FUN_0045a33c(DAT_008c5994,"10min",5,&DAT_004354b0);
    DAT_008c5980 = FUN_0045a33c(DAT_008c5994,"30min",6,&DAT_004354b0);
    uVar1 = 6;
    pcVar3 = "NEVER";
  }
  else {
    sprintf(acStack_38,"15%s",*(undefined4 *)(*(int *)(iVar2 + 8) + 0x13c));
    DAT_008c5968 = FUN_0045a33c(DAT_008c5994,acStack_38,0,&DAT_004354b0);
    sprintf(acStack_38,"30%s",*(undefined4 *)(*(int *)(iVar2 + 8) + 0x13c));
    DAT_008c596c = FUN_0045a33c(DAT_008c5994,acStack_38,1,&DAT_004354b0);
    sprintf(acStack_38,"60%s",*(undefined4 *)(*(int *)(iVar2 + 8) + 0x13c));
    DAT_008c5970 = FUN_0045a33c(DAT_008c5994,acStack_38,2,&DAT_004354b0);
    sprintf(acStack_38,(char *)&PTR_DAT_0068ad1c,*(undefined4 *)(*(int *)(iVar2 + 8) + 0x140));
    DAT_008c5974 = FUN_0045a33c(DAT_008c5994,acStack_38,3,&DAT_004354b0);
    sprintf(acStack_38,(char *)&PTR_DAT_0068ad20,*(undefined4 *)(*(int *)(iVar2 + 8) + 0x140));
    DAT_008c5978 = FUN_0045a33c(DAT_008c5994,acStack_38,4,&DAT_004354b0);
    sprintf(acStack_38,"10%s",*(undefined4 *)(*(int *)(iVar2 + 8) + 0x140));
    DAT_008c597c = FUN_0045a33c(DAT_008c5994,acStack_38,5,&DAT_004354b0);
    sprintf(acStack_38,"30%s",*(undefined4 *)(*(int *)(iVar2 + 8) + 0x140));
    DAT_008c5980 = FUN_0045a33c(DAT_008c5994,acStack_38,6,&DAT_004354b0);
    pcVar3 = *(char **)(*(int *)(iVar2 + 8) + 0x184);
    uVar1 = 7;
  }
  DAT_008c5984 = FUN_0045a33c(DAT_008c5994,pcVar3,uVar1,&DAT_004354b0);
  FUN_00428dd0(iVar2);
  DAT_008c5988 = FUN_004d992c(DAT_008c5994);
  FUN_004db8f0(DAT_008c5988,0x168,0x28);
  FUN_004e6e70(DAT_008c5988,0,0);
  FUN_004e6e50(DAT_008c5988,0xff000000,0);
  FUN_004e6d90(DAT_008c5988,0,0);
  FUN_004e04b8(DAT_008c5988,0);
  FUN_004e0518(DAT_008c5988,0);
  FUN_0045a578((&DAT_008c5968)[DAT_008c8a81],1);
  FUN_004e298c((&DAT_008c5968)[DAT_008c8a81 + 1],1);
  thunk_FUN_00952500(DAT_008c5994);
  return;
}

