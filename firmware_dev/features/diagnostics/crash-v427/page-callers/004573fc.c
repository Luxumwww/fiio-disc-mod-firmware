
void FUN_004573fc(void)

{
  int iVar1;
  undefined4 uVar2;
  undefined *puVar3;
  undefined4 *puVar4;
  
  if (DAT_008c6e90 != 0) {
    puVar4 = &DAT_008c6e7c;
    do {
      uVar2 = *puVar4;
      puVar4 = puVar4 + 1;
      FUN_0045a578(uVar2,0);
    } while (puVar4 != &DAT_008c6e90);
    FUN_0045a578((&DAT_008c6e7c)[DAT_008c8a34],1);
    thunk_FUN_00952500(DAT_008c6e90);
    return;
  }
  DAT_008c6e90 = FUN_004d992c(0);
  FUN_004db8f0(DAT_008c6e90,0x168,0x168);
  uVar2 = DAT_008c6e90;
  FUN_004e6d40(DAT_008c6e90,0,0);
  FUN_004e6d50(uVar2,0,0);
  FUN_004e6d20(uVar2,0,0);
  FUN_004e6d30(uVar2,0,0);
  FUN_004e6e70(DAT_008c6e90,0,0);
  FUN_004e6d80(DAT_008c6e90,0xff000000,0);
  FUN_004f3160(DAT_008c6e90,1);
  FUN_004f31a4(DAT_008c6e90,0,0,2);
  FUN_004dbe84(DAT_008c6e90,1,0,0);
  FUN_004e7090(DAT_008c6e90,0,0);
  FUN_004e6d60(DAT_008c6e90,5,0);
  FUN_004e04b8(DAT_008c6e90,0);
  FUN_004d3c7c(DAT_008c6e90,FUN_00457160,1,0);
  FUN_004d3c7c(DAT_008c6e90,FUN_00457160,2,0);
  FUN_004d3c7c(DAT_008c6e90,FUN_00457160,8,0);
  thunk_FUN_00952500(DAT_008c6e90);
  uVar2 = FUN_0046f51c();
  iVar1 = FUN_00428900("/usr/project/config/ui/set_menu/system.json",uVar2);
  if (iVar1 == 0) {
    FUN_0045a5bc(DAT_008c6e90,&DAT_0068d634,0xe67b10);
  }
  else {
    FUN_0045a5bc(DAT_008c6e90,**(undefined4 **)(iVar1 + 8),0xe67b10);
  }
  FUN_00428dd0(iVar1);
  uVar2 = FUN_0046f51c();
  iVar1 = FUN_00428900("/usr/project/config/ui/set_menu/audio_setting.json",uVar2);
  if (iVar1 == 0) {
    DAT_008c6e7c = FUN_0045a33c(DAT_008c6e90,&DAT_0068d644,0,&DAT_00457a98);
    DAT_008c6e80 = FUN_0045a33c(DAT_008c6e90,&DAT_0068d654,1,&DAT_00457a98);
    DAT_008c6e84 = FUN_0045a33c(DAT_008c6e90,"USB AUDIO",2,&DAT_00457a98);
    DAT_008c6e88 = FUN_0045a33c(DAT_008c6e90,&DAT_0068d670,3,&DAT_00457a98);
    puVar3 = &DAT_0068d684;
  }
  else {
    DAT_008c6e7c = FUN_0045a33c(DAT_008c6e90,*(undefined4 *)(*(int *)(iVar1 + 8) + 0x2c),0,
                                &DAT_00457a98);
    DAT_008c6e80 = FUN_0045a33c(DAT_008c6e90,*(undefined4 *)(*(int *)(iVar1 + 8) + 0x30),1,
                                &DAT_00457a98);
    DAT_008c6e84 = FUN_0045a33c(DAT_008c6e90,*(undefined4 *)(*(int *)(iVar1 + 8) + 0x34),2,
                                &DAT_00457a98);
    DAT_008c6e88 = FUN_0045a33c(DAT_008c6e90,*(undefined4 *)(*(int *)(iVar1 + 8) + 100),3,
                                &DAT_00457a98);
    puVar3 = *(undefined **)(*(int *)(iVar1 + 8) + 0x38);
  }
  DAT_008c6e8c = FUN_0045a33c(DAT_008c6e90,puVar3,4,&DAT_00457a98);
  FUN_00428dd0(iVar1);
  uVar2 = FUN_004d992c(DAT_008c6e90);
  FUN_004db8f0(uVar2,0x168,0x28);
  FUN_004e6e70(uVar2,0,0);
  FUN_004e6e50(uVar2,0xff000000,0);
  FUN_004e6d90(uVar2,0,0);
  FUN_004e04b8(uVar2,0);
  FUN_004e0518(uVar2,0);
  FUN_004d9960(uVar2,0x4000);
  FUN_0045a578((&DAT_008c6e7c)[DAT_008c8a34],1);
  return;
}

