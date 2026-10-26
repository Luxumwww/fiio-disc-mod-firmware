
void FUN_00464628(undefined4 param_1)

{
  undefined4 uVar1;
  int iVar2;
  char *pcVar3;
  
  DAT_008c71a4 = param_1;
  DAT_008c71bc = FUN_004d992c(0);
  FUN_004db8f0(DAT_008c71bc,0x168,0x168);
  uVar1 = DAT_008c71bc;
  FUN_004e6d40(DAT_008c71bc,0,0);
  FUN_004e6d50(uVar1,0,0);
  FUN_004e6d20(uVar1,0,0);
  FUN_004e6d30(uVar1,0,0);
  FUN_004e6e70(DAT_008c71bc,0,0);
  FUN_004e6d80(DAT_008c71bc,0xff000000,0);
  FUN_004dbe84(DAT_008c71bc,1,0,0);
  FUN_004e7090(DAT_008c71bc,0,0);
  FUN_004e6d60(DAT_008c71bc,5,0);
  FUN_004d3c7c(DAT_008c71bc,&DAT_004644f8,1,0);
  FUN_004d3c7c(DAT_008c71bc,&DAT_004644f8,2,0);
  FUN_004d3c7c(DAT_008c71bc,&DAT_004644f8,8,0);
  thunk_FUN_00952500(DAT_008c71bc);
  FUN_004e04b8(DAT_008c71bc,0);
  FUN_004f3160(DAT_008c71bc,1);
  FUN_004f31a4(DAT_008c71bc,0,0,2);
  uVar1 = FUN_004d992c(DAT_008c71bc);
  FUN_004db8f0(uVar1,0x140,0x28);
  FUN_004e6e70(uVar1,0,0);
  FUN_004e6e50(uVar1,0xff000000,0);
  FUN_004e6d90(uVar1,0,0);
  FUN_004e04b8(uVar1,0);
  FUN_004d9960(uVar1,1);
  uVar1 = FUN_0046f51c();
  iVar2 = FUN_00428900("/usr/project/config/ui/set_menu/audio_setting.json",uVar1);
  if (iVar2 == 0) {
    FUN_0045a5bc(DAT_008c71bc,&DAT_0068bd78,0xe67b10);
  }
  else {
    FUN_0045a5bc(DAT_008c71bc,*(undefined4 *)(*(int *)(iVar2 + 8) + 0x40),0xe67b10);
  }
  FUN_00428dd0(iVar2);
  DAT_008c71a8 = FUN_0045a33c(DAT_008c71bc,&DAT_0068e3e0,0,&DAT_00464554);
  DAT_008c71ac = FUN_0045a33c(DAT_008c71bc,&DAT_0068e3e4,1,&DAT_00464554);
  uVar1 = FUN_0046f51c();
  iVar2 = FUN_00428900("/usr/project/config/ui/set_menu/others.json",uVar1);
  if (iVar2 == 0) {
    DAT_008c71b0 = FUN_0045a33c(DAT_008c71bc,"LDAC_MOBILE",2,&DAT_00464554);
    DAT_008c71b4 = FUN_0045a33c(DAT_008c71bc,"LDAC_STANDARD",3,&DAT_00464554);
    pcVar3 = "LDAC_HIGH";
  }
  else {
    DAT_008c71b0 = FUN_0045a33c(DAT_008c71bc,*(undefined4 *)(*(int *)(iVar2 + 8) + 0x130),2,
                                &DAT_00464554);
    DAT_008c71b4 = FUN_0045a33c(DAT_008c71bc,*(undefined4 *)(*(int *)(iVar2 + 8) + 0x134),3,
                                &DAT_00464554);
    pcVar3 = *(char **)(*(int *)(iVar2 + 8) + 0x138);
  }
  DAT_008c71b8 = FUN_0045a33c(DAT_008c71bc,pcVar3,4,&DAT_00464554);
  FUN_00428dd0(iVar2);
  uVar1 = FUN_004d992c(DAT_008c71bc);
  FUN_004db8f0(uVar1,0x140,0x28);
  FUN_004e6e70(uVar1,0,0);
  FUN_004e6e50(uVar1,0xff000000,0);
  FUN_004e6d90(uVar1,0,0);
  FUN_004e04b8(uVar1,0);
  FUN_004e0518(uVar1,0);
  FUN_004d9960(uVar1,0x4000);
  FUN_00969000((&DAT_008c71a8)[DAT_008c8a60],1);
  return;
}

