
void FUN_00458f7c(int param_1,undefined4 param_2)

{
  undefined4 uVar1;
  undefined4 uVar2;
  int iVar3;
  
  FUN_004e80c8(&DAT_008c6f0c);
  DAT_008c6f0c = FUN_004d992c(0);
  FUN_004db8f0(DAT_008c6f0c,0x168,0x168);
  FUN_004dbe84(DAT_008c6f0c,9,0,0);
  FUN_004e6d80(DAT_008c6f0c,0xff000000,0);
  FUN_004e6e70(DAT_008c6f0c,0,0);
  FUN_004e7090(DAT_008c6f0c,0,0);
  FUN_004e04b8(DAT_008c6f0c,0);
  if (param_1 != 0) {
    DAT_008c6f10 = param_1;
  }
  DAT_008c6f08 = param_2;
  uVar1 = FUN_0051c364(DAT_008c6f0c);
  FUN_0051d5d8(uVar1,0);
  FUN_004db9cc(uVar1,0x118);
  uVar2 = FUN_0046f51c();
  iVar3 = FUN_00428900("/usr/project/config/ui/set_menu/popup.json",uVar2);
  if (iVar3 == 0) {
    switch(param_2) {
    case 0:
      thunk_FUN_0097a000(uVar1,&DAT_0068d76c);
      break;
    case 1:
      thunk_FUN_0097a000(uVar1,&DAT_0068d788);
      break;
    case 2:
      thunk_FUN_0097a000(uVar1,&DAT_0068d7a4);
      break;
    case 3:
    case 4:
    case 5:
    case 6:
    case 7:
      thunk_FUN_0097a000(uVar1,&DAT_0068d7b4);
      break;
    case 8:
      thunk_FUN_0097a000(uVar1,&DAT_0068d7c4);
    }
  }
  else {
    switch(param_2) {
    case 0:
      thunk_FUN_0097a000(uVar1,*(undefined4 *)(*(int *)(iVar3 + 8) + 0x5c));
      break;
    case 1:
      thunk_FUN_0097a000(uVar1,*(undefined4 *)(*(int *)(iVar3 + 8) + 0x60));
      break;
    case 2:
      thunk_FUN_0097a000(uVar1,*(undefined4 *)(*(int *)(iVar3 + 8) + 0x10));
      break;
    case 3:
    case 4:
    case 5:
    case 6:
    case 7:
      thunk_FUN_0097a000(uVar1,*(undefined4 *)(*(int *)(iVar3 + 8) + 0x24));
      break;
    case 8:
      thunk_FUN_0097a000(uVar1,*(undefined4 *)(*(int *)(iVar3 + 8) + 0x74));
    }
  }
  FUN_00428dd0(iVar3);
  FUN_004e4820(uVar1,&_fbss,0);
  FUN_004e7020(uVar1,0xffffffff,0);
  FUN_004dbe84(uVar1,2,0,0x66);
  FUN_0042ae68(DAT_008c6f0c,0xad,0x29,2,0,0xb4,0xff212121,0xfffe0156,1,&LAB_00458e90);
  FUN_0042ae68(DAT_008c6f0c,0xad,0x29,2,0,0xfd,0xfffe0156,0xffffffff,0,FUN_00458ef0);
  thunk_FUN_00952500(DAT_008c6f0c);
  return;
}

