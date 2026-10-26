
void FUN_0047cae4(void)

{
  undefined4 uVar1;
  undefined4 uVar2;
  int iVar3;
  
  DAT_008c7794 = FUN_004d992c(0);
  FUN_004e6d80(DAT_008c7794,0xff000000,0);
  FUN_004e6d90(DAT_008c7794,0xff,0);
  FUN_004db9cc(DAT_008c7794,0x168);
  FUN_004dba4c(DAT_008c7794,0x168);
  FUN_004dbe7c(DAT_008c7794,1);
  FUN_004e7090(DAT_008c7794,0,0);
  FUN_004e6e70(DAT_008c7794,0,0);
  uVar1 = DAT_008c7794;
  FUN_004e6d40(DAT_008c7794,0,0);
  FUN_004e6d50(uVar1,0,0);
  FUN_004e6d20(uVar1,0,0);
  FUN_004e6d30(uVar1,0,0);
  FUN_004e04b8(DAT_008c7794,0);
  FUN_004d9a88(DAT_008c7794,0x10);
  FUN_004d3c7c(DAT_008c7794,&LAB_0047ce40,1,0);
  FUN_004d3c7c(DAT_008c7794,&LAB_0047ce40,2,0);
  FUN_004d3c7c(DAT_008c7794,&LAB_0047ce40,8,0);
  uVar1 = FUN_0051c364(DAT_008c7794);
  FUN_004e4820(uVar1,&DAT_00826054,0);
  FUN_004e7020(uVar1,0xffff0000,0);
  uVar2 = FUN_0046f51c();
  iVar3 = FUN_00428900("/usr/project/config/ui/set_menu/others.json",uVar2);
  if (iVar3 == 0) {
    thunk_FUN_0097a000(uVar1,&DAT_0068f6ac);
  }
  else {
    thunk_FUN_0097a000(uVar1,*(undefined4 *)(*(int *)(iVar3 + 8) + 0x164));
  }
  FUN_00428dd0(iVar3);
  FUN_004dbe84(uVar1,2,0,0xb);
  FUN_004db9cc(uVar1,0x15e);
  FUN_0051d5d8(uVar1,3);
  FUN_004d9a88(uVar1,0x10);
  FUN_004d9960(uVar1,0x4000);
  DAT_008c778c = FUN_0051c364(DAT_008c7794);
  FUN_004e4820(DAT_008c778c,&DAT_0082604c,0);
  FUN_004e7020(DAT_008c778c,0xff9f9f9f,0);
  thunk_FUN_0097a000(DAT_008c778c,"/tmp/sdcard/CustomCover");
  FUN_004dbe84(DAT_008c778c,2,0,0x2b);
  FUN_004db9cc(DAT_008c778c,0x10e);
  FUN_0051d5d8(DAT_008c778c,3);
  FUN_004d9a88(DAT_008c778c,0x10);
  FUN_004d9960(DAT_008c778c,0x4000);
  DAT_008c7790 = FUN_00508c10(DAT_008c7794);
  FUN_004db8f0(DAT_008c7790,0x168,0x132);
  FUN_004dbe84(DAT_008c7790,5,0,5);
  FUN_004e6d90(DAT_008c7790,0,0);
  FUN_004e7090(DAT_008c7790,0,0);
  FUN_004e6e70(DAT_008c7790,0,0);
  uVar1 = DAT_008c7790;
  FUN_004e6d40(DAT_008c7790,0,0);
  FUN_004e6d50(uVar1,0,0);
  FUN_004e6d20(uVar1,0,0);
  FUN_004e6d30(uVar1,0,0);
  FUN_004e0518(DAT_008c7790,0xc);
  FUN_004d9960(DAT_008c7790,0x4000);
  FUN_004e04b8(DAT_008c7790,0);
  thunk_FUN_00952500(DAT_008c7794);
  return;
}

