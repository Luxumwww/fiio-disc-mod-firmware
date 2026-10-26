
void FUN_00453f94(void)

{
  undefined4 uVar1;
  int iVar2;
  undefined4 uVar3;
  
  DAT_008c69a8 = FUN_004d992c(0);
  FUN_004db8f0(DAT_008c69a8,0x168,0x168);
  FUN_004dbe84(DAT_008c69a8,9,0,0);
  FUN_004e6d80(DAT_008c69a8,0xff000000,0);
  FUN_004e6e70(DAT_008c69a8,0,0);
  FUN_004e7090(DAT_008c69a8,0,0);
  FUN_004e04b8(DAT_008c69a8,0);
  FUN_004d3c7c(DAT_008c69a8,&LAB_00453efc,1,0);
  FUN_004d3c7c(DAT_008c69a8,&LAB_00453efc,2,0);
  FUN_004d3c7c(DAT_008c69a8,&LAB_00453efc,8,0);
  DAT_008c69a4 = FUN_0051c364(DAT_008c69a8);
  uVar1 = FUN_0046f51c();
  iVar2 = FUN_00428900("/usr/project/config/ui/set_menu/others.json",uVar1);
  if (iVar2 == 0) {
    thunk_FUN_0097a000(DAT_008c69a4,&DAT_0068cc88);
  }
  else {
    thunk_FUN_0097a000(DAT_008c69a4,*(undefined4 *)(*(int *)(iVar2 + 8) + 100));
  }
  FUN_004db9cc(DAT_008c69a4,0x140);
  FUN_0051d5d8(DAT_008c69a4,0);
  FUN_004e4820(DAT_008c69a4,&DAT_00826064,0);
  FUN_004e7020(DAT_008c69a4,0xffffffff,0);
  FUN_004dbe84(DAT_008c69a4,9,0,0);
  if (DAT_008c69a4 != 0) {
    DAT_008c6998 = DAT_008c69a4;
  }
  DAT_008c69a0 = FUN_0051c364(DAT_008c69a8);
  if (iVar2 == 0) {
    thunk_FUN_0097a000(DAT_008c69a0,&DAT_0068cc98);
  }
  else {
    thunk_FUN_0097a000(DAT_008c69a0,*(undefined4 *)(*(int *)(iVar2 + 8) + 0x68));
  }
  FUN_00428dd0(iVar2);
  FUN_004db9cc(DAT_008c69a0,0x140);
  FUN_0051d5d8(DAT_008c69a0,0);
  FUN_004e4820(DAT_008c69a0,&DAT_00826054,0);
  FUN_004e7020(DAT_008c69a0,0xff696969,0);
  FUN_004dbe84(DAT_008c69a0,2,0,0x106);
  FUN_004d9960(DAT_008c69a0,1);
  if (DAT_008c69a0 != 0) {
    DAT_008c6994 = DAT_008c69a0;
  }
  DAT_008c699c = FUN_0051c364(DAT_008c69a8);
  thunk_FUN_0097a000(DAT_008c699c,&DAT_0068cc84);
  FUN_004db9cc(DAT_008c699c,0x140);
  FUN_0051d5d8(DAT_008c699c,0);
  FUN_004e4820(DAT_008c699c,&_fbss,0);
  FUN_004e7020(DAT_008c699c,0xffffffff,0);
  FUN_004dbe84(DAT_008c699c,2,0,0x19);
  FUN_004d9960(DAT_008c699c,1);
  DAT_008c6988 = FUN_0042ae68(DAT_008c69a8,0xad,0x29,1,0x5e,0xb4,0xff212121,0xfffe0156,1,
                              &LAB_00453d00);
  FUN_004d9960(DAT_008c6988,1);
  uVar1 = FUN_004e8a80(DAT_008c6988,0);
  uVar3 = FUN_0046f51c();
  iVar2 = FUN_00428900("/usr/project/config/ui/set_menu/others.json",uVar3);
  if (iVar2 == 0) {
    thunk_FUN_0097a000(uVar1,&DAT_0068ccbc);
  }
  else {
    thunk_FUN_0097a000(uVar1,*(undefined4 *)(*(int *)(iVar2 + 8) + 0x15c));
  }
  FUN_00428dd0(iVar2);
  DAT_008c6984 = FUN_0042ae68(DAT_008c69a8,0xad,0x29,1,0x5e,0xfd,0xfffe0156,0xffffffff,0,
                              &LAB_0045439c);
  FUN_004d9960(DAT_008c6984,1);
  thunk_FUN_00952500(DAT_008c69a8);
  FUN_004428bc(&DAT_00683470,2,0);
  return;
}

