
void FUN_0046ec88(int param_1)

{
  int iVar1;
  undefined4 uVar2;
  uint uVar3;
  undefined4 uVar4;
  
  if (param_1 != 0) {
    DAT_008c7420 = param_1;
    DAT_008c7450 = FUN_004d992c(0);
    FUN_004db8f0(DAT_008c7450,0x168,0x168);
    uVar2 = DAT_008c7450;
    FUN_004e6d40(DAT_008c7450,0,0);
    FUN_004e6d50(uVar2,0,0);
    FUN_004e6d20(uVar2,0,0);
    FUN_004e6d30(uVar2,0,0);
    FUN_004e6e70(DAT_008c7450,0,0);
    FUN_004e6d80(DAT_008c7450,0xff000000,0);
    FUN_004dbe84(DAT_008c7450,1,0,0);
    FUN_004e7090(DAT_008c7450,0,0);
    FUN_004e6d60(DAT_008c7450,5,0);
    FUN_004e04b8(DAT_008c7450,0);
    FUN_004d3c7c(DAT_008c7450,FUN_0046ebc4,1,0);
    FUN_004d3c7c(DAT_008c7450,FUN_0046ebc4,2,0);
    FUN_004d3c7c(DAT_008c7450,FUN_0046ebc4,8,0);
    thunk_FUN_00952500(DAT_008c7450);
    FUN_004f3160(DAT_008c7450,1);
    FUN_004f31a4(DAT_008c7450,0,0,2);
    iVar1 = FUN_00428900("/usr/project/config/ui/set_menu/system_setting.json",PTR_DAT_00825e60);
    if (iVar1 == 0) {
      FUN_0045a5bc(DAT_008c7450,&DAT_0068ef2c,0xb4fb);
    }
    else {
      FUN_0045a5bc(DAT_008c7450,*(undefined4 *)(*(int *)(iVar1 + 8) + 0x2c),0xb4fb);
    }
    FUN_00428dd0(iVar1);
    DAT_008c7424 = FUN_0045a33c(DAT_008c7450,&DAT_0068ef3c,0,&LAB_0046f760);
    DAT_008c7428 = FUN_0045a33c(DAT_008c7450,&DAT_0068ef4c,1,&LAB_0046f760);
    DAT_008c742c = FUN_0045a33c(DAT_008c7450,"English",2,&LAB_0046f760);
    DAT_008c7430 = FUN_0045a33c(DAT_008c7450,&DAT_0068ef64,3,&LAB_0046f760);
    DAT_008c7434 = FUN_0045a33c(DAT_008c7450,&DAT_0068ef70,4,&LAB_0046f760);
    DAT_008c7438 = FUN_0045a33c(DAT_008c7450,&DAT_0068ef7c,5,&LAB_0046f760);
    DAT_008c743c = FUN_0045a33c(DAT_008c7450,"Italiano",6,&LAB_0046f760);
    DAT_008c7440 = FUN_0045a33c(DAT_008c7450,"Deutsch",7,&LAB_0046f760);
    DAT_008c7444 = FUN_0045a33c(DAT_008c7450,&DAT_0068ef9c,8,&LAB_0046f760);
    DAT_008c7448 = FUN_0045a33c(DAT_008c7450,&DAT_0068efa8,9,&LAB_0046f760);
    uVar2 = FUN_004d992c(DAT_008c7450);
    FUN_004db8f0(uVar2,0x140,0x28);
    FUN_004e6e70(uVar2,0,0);
    FUN_004e6e50(uVar2,0xff000000,0);
    FUN_004e6d90(uVar2,0,0);
    FUN_004e04b8(uVar2,0);
    FUN_004e0518(uVar2,0);
    FUN_004d9960(uVar2,0x4000);
    uVar3 = (uint)DAT_008c8a7f;
    if (uVar3 == 100) {
      uVar3 = 0;
    }
    FUN_0045a578((&DAT_008c7424)[uVar3],1);
    return;
  }
  FUN_0046c770();
  DAT_008c7450 = FUN_004d992c(0);
  FUN_004db8f0(DAT_008c7450,0x168,0x168);
  uVar2 = DAT_008c7450;
  FUN_004e6d40(DAT_008c7450,0,0);
  FUN_004e6d50(uVar2,0,0);
  FUN_004e6d20(uVar2,0,0);
  FUN_004e6d30(uVar2,0,0);
  FUN_004e6e70(DAT_008c7450,0,0);
  FUN_004e6d80(DAT_008c7450,0xff000000,0);
  FUN_004dbe84(DAT_008c7450,1,0,0);
  FUN_004e7090(DAT_008c7450,0,0);
  FUN_004e04b8(DAT_008c7450,0);
  FUN_004d3c7c(DAT_008c7450,FUN_0046ebc4,1,0);
  FUN_004d3c7c(DAT_008c7450,FUN_0046ebc4,2,0);
  FUN_004d3c7c(DAT_008c7450,FUN_0046ebc4,8,0);
  thunk_FUN_00952500(DAT_008c7450);
  uVar2 = FUN_004d992c(DAT_008c7450);
  FUN_004db8f0(uVar2,0x168,300);
  FUN_004dbe84(uVar2,1,0,0);
  FUN_004e6e70(uVar2,0,0);
  FUN_004e6d90(uVar2,0,0);
  FUN_004e6d40(uVar2,0,0);
  FUN_004e6d50(uVar2,0,0);
  FUN_004e6d20(uVar2,0,0);
  FUN_004e6d30(uVar2,0,0);
  FUN_004f3160(uVar2,1);
  FUN_004f31a4(uVar2,0,0,2);
  FUN_004e6d60(uVar2,5,0);
  FUN_004e70b0(uVar2,0,0x10000);
  iVar1 = FUN_00428900("/usr/project/config/ui/set_menu/system_setting.json",PTR_DAT_00825e60);
  if (iVar1 == 0) {
    FUN_0045a5bc(uVar2,&DAT_0068ef2c,0xb4fb);
  }
  else {
    FUN_0045a5bc(uVar2,*(undefined4 *)(*(int *)(iVar1 + 8) + 0x2c),0xb4fb);
  }
  FUN_00428dd0(iVar1);
  DAT_008c7424 = FUN_0045a33c(uVar2,&DAT_0068ef3c,0,&LAB_0046f760);
  DAT_008c7428 = FUN_0045a33c(uVar2,&DAT_0068ef4c,1,&LAB_0046f760);
  DAT_008c742c = FUN_0045a33c(uVar2,"English",2,&LAB_0046f760);
  DAT_008c7430 = FUN_0045a33c(uVar2,&DAT_0068ef64,3,&LAB_0046f760);
  DAT_008c7434 = FUN_0045a33c(uVar2,&DAT_0068ef70,4,&LAB_0046f760);
  DAT_008c7438 = FUN_0045a33c(uVar2,&DAT_0068ef7c,5,&LAB_0046f760);
  DAT_008c743c = FUN_0045a33c(uVar2,"Italiano",6,&LAB_0046f760);
  DAT_008c7440 = FUN_0045a33c(uVar2,"Deutsch",7,&LAB_0046f760);
  DAT_008c7444 = FUN_0045a33c(uVar2,&DAT_0068ef9c,8,&LAB_0046f760);
  DAT_008c7448 = FUN_0045a33c(uVar2,&DAT_0068efa8,9,&LAB_0046f760);
  uVar2 = FUN_004d992c(uVar2);
  FUN_004db8f0(uVar2,0x140,0x28);
  FUN_004e6e70(uVar2,0,0);
  FUN_004e6e50(uVar2,0xff000000,0);
  FUN_004e6d90(uVar2,0,0);
  FUN_004e04b8(uVar2,0);
  FUN_004e0518(uVar2,0);
  DAT_008c8a7f = 0;
  FUN_0045a578(DAT_008c7424,1);
  uVar2 = FUN_00518ca4(DAT_008c7450);
  FUN_004db8f0(uVar2,0x7f,0x29);
  FUN_004dbe84(uVar2,5,0,0xfffffff6);
  FUN_004e6e70(uVar2,0,0);
  FUN_004e6ee0(uVar2,0,0);
  FUN_004e7090(uVar2,0x14,0);
  uVar4 = FUN_0051c364(uVar2);
  iVar1 = FUN_00428900("/usr/project/config/ui/set_menu/popup.json",PTR_DAT_00825e60);
  if (iVar1 == 0) {
    thunk_FUN_0097a000(uVar4,&DAT_00687c78);
  }
  else {
    thunk_FUN_0097a000(uVar4,*(undefined4 *)(*(int *)(iVar1 + 8) + 0x68));
  }
  FUN_00428dd0(iVar1);
  FUN_004dbe84(uVar4,9,0,0);
  FUN_004e7020(uVar4,0xffffffff,0);
  FUN_004e4820(uVar4,&DAT_00826054,0);
  FUN_004d3c7c(uVar2,&LAB_0046ec20,7,0);
  FUN_004d9a88(uVar2,0x10);
  uVar4 = FUN_004e8a68(uVar2);
  iVar1 = FUN_004e8acc(uVar4);
  FUN_004e8534(uVar2,iVar1 + -1);
  return;
}

