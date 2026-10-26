
void FUN_00458098(undefined4 param_1)

{
  undefined4 uVar1;
  int iVar2;
  char *pcVar3;
  
  DAT_008c6ea0 = param_1;
  DAT_008c6ec4 = FUN_004d992c(0);
  FUN_004db8f0(DAT_008c6ec4,0x168,0x168);
  uVar1 = DAT_008c6ec4;
  FUN_004e6d40(DAT_008c6ec4,0,0);
  FUN_004e6d50(uVar1,0,0);
  FUN_004e6d20(uVar1,0,0);
  FUN_004e6d30(uVar1,0,0);
  FUN_004e6e70(DAT_008c6ec4,0,0);
  FUN_004e6d80(DAT_008c6ec4,0xff000000,0);
  FUN_004dbe84(DAT_008c6ec4,1,0,0);
  FUN_004e7090(DAT_008c6ec4,0,0);
  FUN_004e6d60(DAT_008c6ec4,5,0);
  FUN_004e04b8(DAT_008c6ec4,0);
  FUN_004d3c7c(DAT_008c6ec4,FUN_0045803c,1,0);
  FUN_004d3c7c(DAT_008c6ec4,FUN_0045803c,2,0);
  FUN_004d3c7c(DAT_008c6ec4,FUN_0045803c,8,0);
  thunk_FUN_00952500(DAT_008c6ec4);
  FUN_004f3160(DAT_008c6ec4,1);
  FUN_004f31a4(DAT_008c6ec4,2,0,2);
  uVar1 = FUN_004d992c(DAT_008c6ec4);
  FUN_004db8f0(uVar1,0x168,0x50);
  FUN_004e6e70(uVar1,0,0);
  FUN_004e6e50(uVar1,0xff000000,0);
  FUN_004e6d90(uVar1,0,0);
  FUN_004e04b8(uVar1,0);
  FUN_004e0518(uVar1,0);
  FUN_004d9960(uVar1,0x4000);
  uVar1 = FUN_0046f51c();
  iVar2 = FUN_00428900("/usr/project/config/ui/set_menu/audio_setting.json",uVar1);
  if (iVar2 == 0) {
    FUN_0045a5bc(DAT_008c6ec4,&DAT_0068bd94,0xe67b10);
  }
  else {
    FUN_0045a5bc(DAT_008c6ec4,*(undefined4 *)(*(int *)(iVar2 + 8) + 0x4c),0xe67b10);
  }
  FUN_00428dd0(iVar2);
  uVar1 = FUN_0046f51c();
  iVar2 = FUN_00428900("/usr/project/config/ui/set_menu/others.json",uVar1);
  if (iVar2 == 0) {
    DAT_008c6ea8 = FUN_0045a33c(DAT_008c6ec4,"FAST_LL",0,&LAB_00457f20);
    DAT_008c6eac = FUN_0045a33c(DAT_008c6ec4,"SLOW_LL",1,&LAB_00457f20);
    DAT_008c6eb0 = FUN_0045a33c(DAT_008c6ec4,"SLOW_PC",2,&LAB_00457f20);
    DAT_008c6eb4 = FUN_0045a33c(DAT_008c6ec4,"FAST_PC",3,&LAB_00457f20);
    DAT_008c6eb8 = FUN_0045a33c(DAT_008c6ec4,"NON_OS",4,&LAB_00457f20);
    pcVar3 = "HIGH_PASS";
  }
  else {
    DAT_008c6ea8 = FUN_0045a33c(DAT_008c6ec4,*(undefined4 *)(*(int *)(iVar2 + 8) + 0x118),0,
                                &LAB_00457f20);
    DAT_008c6eac = FUN_0045a33c(DAT_008c6ec4,*(undefined4 *)(*(int *)(iVar2 + 8) + 0x11c),1,
                                &LAB_00457f20);
    DAT_008c6eb0 = FUN_0045a33c(DAT_008c6ec4,*(undefined4 *)(*(int *)(iVar2 + 8) + 0x120),2,
                                &LAB_00457f20);
    DAT_008c6eb4 = FUN_0045a33c(DAT_008c6ec4,*(undefined4 *)(*(int *)(iVar2 + 8) + 0x124),3,
                                &LAB_00457f20);
    DAT_008c6eb8 = FUN_0045a33c(DAT_008c6ec4,*(undefined4 *)(*(int *)(iVar2 + 8) + 0x128),4,
                                &LAB_00457f20);
    pcVar3 = *(char **)(*(int *)(iVar2 + 8) + 300);
  }
  DAT_008c6ebc = FUN_0045a33c(DAT_008c6ec4,pcVar3,5,&LAB_00457f20);
  FUN_00428dd0(iVar2);
  DAT_008c6ec0 = FUN_004d992c(DAT_008c6ec4);
  FUN_004db8f0(DAT_008c6ec0,0x168,0x28);
  FUN_004e6e70(DAT_008c6ec0,0,0);
  FUN_004e6e50(DAT_008c6ec0,0xff000000,0);
  FUN_004e6d90(DAT_008c6ec0,0,0);
  FUN_004e04b8(DAT_008c6ec0,0);
  FUN_004e0518(DAT_008c6ec0,0);
  FUN_0045a578((&DAT_008c6ea8)[DAT_008c8a63],1);
  FUN_004e298c((&DAT_008c6ea8)[DAT_008c8a63 + 1],1);
  return;
}

