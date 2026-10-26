
void FUN_0045c73c(undefined4 param_1)

{
  undefined4 uVar1;
  int iVar2;
  char acStack_38 [36];
  
  DAT_008c6f70 = param_1;
  DAT_008c6f98 = FUN_004d992c(0);
  FUN_004db8f0(DAT_008c6f98,0x168,0x168);
  uVar1 = DAT_008c6f98;
  FUN_004e6d40(DAT_008c6f98,0,0);
  FUN_004e6d50(uVar1,0,0);
  FUN_004e6d20(uVar1,0,0);
  FUN_004e6d30(uVar1,0,0);
  FUN_004e6e70(DAT_008c6f98,0,0);
  FUN_004e6d80(DAT_008c6f98,0xff000000,0);
  FUN_004dbe84(DAT_008c6f98,1,0,0);
  FUN_004e7090(DAT_008c6f98,0,0);
  FUN_004e6d60(DAT_008c6f98,5,0);
  FUN_004e04b8(DAT_008c6f98,0);
  FUN_004d3c7c(DAT_008c6f98,&LAB_0045c490,1,0);
  FUN_004d3c7c(DAT_008c6f98,&LAB_0045c490,2,0);
  FUN_004d3c7c(DAT_008c6f98,&LAB_0045c490,8,0);
  thunk_FUN_00952500(DAT_008c6f98);
  FUN_004f3160(DAT_008c6f98,1);
  FUN_004f31a4(DAT_008c6f98,0,0,2);
  uVar1 = FUN_0046f51c();
  iVar2 = FUN_00428900("/usr/project/config/ui/set_menu/system_setting.json",uVar1);
  if (iVar2 == 0) {
    FUN_0045a5bc(DAT_008c6f98,&DAT_0068c6c0,0xb4fb);
  }
  else {
    FUN_0045a5bc(DAT_008c6f98,*(undefined4 *)(*(int *)(iVar2 + 8) + 0x14),0xb4fb);
  }
  FUN_00428dd0(iVar2);
  uVar1 = FUN_0046f51c();
  iVar2 = FUN_00428900("/usr/project/config/ui/set_menu/others.json",uVar1);
  if (iVar2 == 0) {
    DAT_008c6f78 = FUN_0045a33c(DAT_008c6f98,&DAT_0068b094,0,&DAT_0045c4ec);
    DAT_008c6f7c = FUN_0045a33c(DAT_008c6f98,&DAT_0068ace4,1,&DAT_0045c4ec);
    DAT_008c6f80 = FUN_0045a33c(DAT_008c6f98,"10min",2,&DAT_0045c4ec);
    DAT_008c6f84 = FUN_0045a33c(DAT_008c6f98,"30min",3,&DAT_0045c4ec);
    DAT_008c6f88 = FUN_0045a33c(DAT_008c6f98,"60min",4,&DAT_0045c4ec);
    DAT_008c6f8c = FUN_0045a33c(DAT_008c6f98,"90min",5,&DAT_0045c4ec);
    DAT_008c6f90 = FUN_0045a33c(DAT_008c6f98,"120min",6,&DAT_0045c4ec);
  }
  else {
    DAT_008c6f78 = FUN_0045a33c(DAT_008c6f98,*(undefined4 *)(*(int *)(iVar2 + 8) + 0x104),0,
                                &DAT_0045c4ec);
    sprintf(acStack_38,(char *)&PTR_DAT_0068ad20,*(undefined4 *)(*(int *)(iVar2 + 8) + 0x140));
    DAT_008c6f7c = FUN_0045a33c(DAT_008c6f98,acStack_38,1,&DAT_0045c4ec);
    sprintf(acStack_38,"10%s",*(undefined4 *)(*(int *)(iVar2 + 8) + 0x140));
    DAT_008c6f80 = FUN_0045a33c(DAT_008c6f98,acStack_38,2,&DAT_0045c4ec);
    sprintf(acStack_38,"30%s",*(undefined4 *)(*(int *)(iVar2 + 8) + 0x140));
    DAT_008c6f84 = FUN_0045a33c(DAT_008c6f98,acStack_38,3,&DAT_0045c4ec);
    sprintf(acStack_38,"60%s",*(undefined4 *)(*(int *)(iVar2 + 8) + 0x140));
    DAT_008c6f88 = FUN_0045a33c(DAT_008c6f98,acStack_38,4,&DAT_0045c4ec);
    sprintf(acStack_38,"90%s",*(undefined4 *)(*(int *)(iVar2 + 8) + 0x140));
    DAT_008c6f8c = FUN_0045a33c(DAT_008c6f98,acStack_38,5,&DAT_0045c4ec);
    sprintf(acStack_38,"120%s",*(undefined4 *)(*(int *)(iVar2 + 8) + 0x140));
    DAT_008c6f90 = FUN_0045a33c(DAT_008c6f98,acStack_38,6,&DAT_0045c4ec);
  }
  FUN_00428dd0(iVar2);
  DAT_008c6f94 = FUN_004d992c(DAT_008c6f98);
  FUN_004db8f0(DAT_008c6f94,0x168,0x28);
  FUN_004e6e70(DAT_008c6f94,0,0);
  FUN_004e6e50(DAT_008c6f94,0xff000000,0);
  FUN_004e6d90(DAT_008c6f94,0,0);
  FUN_004e04b8(DAT_008c6f94,0);
  FUN_004e0518(DAT_008c6f94,0);
  FUN_0045a578((&DAT_008c6f78)[DAT_008c8a79],1);
  FUN_004e298c((&DAT_008c6f78)[DAT_008c8a79 + 1],1);
  return;
}

