
void FUN_004717c0(void)

{
  undefined4 uVar1;
  int iVar2;
  undefined4 uVar3;
  undefined4 uVar4;
  
  DAT_008c74d8 = FUN_004d992c(0);
  FUN_004db8f0(DAT_008c74d8,0x168,0x168);
  uVar1 = DAT_008c74d8;
  FUN_004e6d40(DAT_008c74d8,0,0);
  FUN_004e6d50(uVar1,0,0);
  FUN_004e6d20(uVar1,0,0);
  FUN_004e6d30(uVar1,0,0);
  FUN_004e6e70(DAT_008c74d8,0,0);
  FUN_004e6d80(DAT_008c74d8,0xff000000,0);
  FUN_004f3160(DAT_008c74d8,1);
  FUN_004dbe84(DAT_008c74d8,1,0,0);
  FUN_004e7090(DAT_008c74d8,0,0);
  FUN_004e6d60(DAT_008c74d8,0,0);
  FUN_004d3c7c(DAT_008c74d8,&LAB_00470530,1,0);
  FUN_004d3c7c(DAT_008c74d8,&LAB_00470530,2,0);
  FUN_004d3c7c(DAT_008c74d8,&LAB_00470530,8,0);
  thunk_FUN_00952500(DAT_008c74d8);
  uVar1 = FUN_0046f51c();
  iVar2 = FUN_00428900("/usr/project/config/ui/set_menu/system.json",uVar1);
  if (iVar2 == 0) {
    FUN_0045a5bc(DAT_008c74d8,&DAT_0068d9f8,0x2d83ff);
  }
  else {
    FUN_0045a5bc(DAT_008c74d8,*(undefined4 *)(*(int *)(iVar2 + 8) + 0xc),0x2d83ff);
  }
  FUN_00428dd0(iVar2);
  uVar1 = FUN_004d992c(DAT_008c74d8);
  FUN_004db8f0(uVar1,0x168,0x3c);
  FUN_004e6e70(uVar1,2,0);
  FUN_004e6e50(uVar1,0xff000000,0);
  FUN_004e6d90(uVar1,0,0);
  FUN_004e6d40(uVar1,5,0);
  FUN_004e6d50(uVar1,5,0);
  FUN_004e6d20(uVar1,5,0);
  FUN_004e6d30(uVar1,5,0);
  FUN_004dbe84(uVar1,2,0,0x28);
  FUN_004e04b8(uVar1,0);
  FUN_004d9960(uVar1,0x4000);
  DAT_008c74cc = FUN_00521a50(uVar1);
  FUN_004db714(DAT_008c74cc,0x14,6);
  FUN_004db8f0(DAT_008c74cc,0x3c,0x21);
  FUN_004d9b88(DAT_008c74cc,0);
  if (DAT_008c8a61 == '\0') {
    FUN_004d9ba8(DAT_008c74cc,1);
  }
  else {
    FUN_004d9b88();
  }
  DAT_008c8a7c = 0;
  FUN_004dbe84(DAT_008c74cc,9,0x46,0);
  FUN_004e6d80(DAT_008c74cc,0xffff0000,0x20001);
  FUN_004e6d80(DAT_008c74cc,0xff565656,0);
  FUN_004d9a88(DAT_008c74cc,2);
  uVar3 = FUN_0051c364(uVar1);
  uVar4 = FUN_0046f51c();
  iVar2 = FUN_00428900("/usr/project/config/ui/set_menu/system.json",uVar4);
  if (iVar2 == 0) {
    thunk_FUN_0097a000(uVar3,&DAT_0068d9f8);
  }
  else {
    thunk_FUN_0097a000(uVar3,*(undefined4 *)(*(int *)(iVar2 + 8) + 0xc));
  }
  FUN_00428dd0(iVar2);
  FUN_004dbe84(uVar3,9,0xffffffb0,0);
  FUN_004e7020(uVar3,0xff9d9d9d,0);
  FUN_004e4820(uVar3,&_fbss,0);
  FUN_004d3c7c(uVar1,&LAB_00471448,7,0);
  uVar1 = FUN_004d992c(DAT_008c74d8);
  FUN_004db8f0(uVar1,0x168,0x28);
  FUN_004e6e70(uVar1,2,0);
  FUN_004e6e50(uVar1,0xff000000,0);
  FUN_004e6d90(uVar1,0,0);
  FUN_004e04b8(uVar1,0);
  FUN_004e0518(uVar1,0);
  FUN_004d9960(uVar1,0x4000);
  uVar1 = FUN_0051c364(uVar1);
  uVar3 = FUN_0046f51c();
  iVar2 = FUN_00428900("/usr/project/config/ui/set_menu/bt.json",uVar3);
  if (iVar2 == 0) {
    thunk_FUN_0097a000(uVar1,&DAT_0068f17c);
  }
  else {
    thunk_FUN_0097a000(uVar1,**(undefined4 **)(iVar2 + 8));
  }
  FUN_00428dd0(iVar2);
  FUN_004dbe84(uVar1,7,10,0);
  FUN_004e7020(uVar1,0xffffffff,0);
  FUN_004e4820(uVar1,&DAT_00826054,0);
  DAT_008c74c8 = FUN_004d992c(DAT_008c74d8);
  FUN_004db8f0(DAT_008c74c8,0x168,(uint)DAT_008c74c1 * 0x47 + 0x14);
  FUN_004dbe84(DAT_008c74c8,9,0,0x28);
  uVar1 = DAT_008c74c8;
  FUN_004e6d40(DAT_008c74c8,0,0);
  FUN_004e6d50(uVar1,0,0);
  FUN_004e6d20(uVar1,0,0);
  FUN_004e6d30(uVar1,0,0);
  FUN_004e6e70(DAT_008c74c8,0,0);
  FUN_004f3160(DAT_008c74c8,1);
  FUN_004f31a4(DAT_008c74c8,0,0,2);
  FUN_004e6d90(DAT_008c74c8,0,0);
  FUN_004d9960(DAT_008c74c8,0x4000);
  FUN_004e04b8(DAT_008c74c8,0);
  FUN_004e0518(DAT_008c74c8,0);
  uVar1 = FUN_004d992c(DAT_008c74d8);
  FUN_004db8f0(uVar1,0x168,0x46);
  FUN_004e6e70(uVar1,2,0);
  FUN_004e6e50(uVar1,0xff000000,0);
  FUN_004e6d90(uVar1,0,0);
  FUN_004e04b8(uVar1,0);
  FUN_004e0518(uVar1,0);
  FUN_004d9960(uVar1,0x4000);
  uVar1 = FUN_00518ca4(uVar1);
  FUN_004db8f0(uVar1,0xad,0x29);
  FUN_004dbe84(uVar1,9,0,0);
  FUN_004e6e70(uVar1,0,0);
  FUN_004e6ee0(uVar1,0,0);
  FUN_004e7090(uVar1,0x14,0);
  FUN_004d9960(uVar1,0x4000);
  uVar3 = FUN_0051c364(uVar1);
  uVar4 = FUN_0046f51c();
  iVar2 = FUN_00428900("/usr/project/config/ui/set_menu/bt.json",uVar4);
  if (iVar2 == 0) {
    thunk_FUN_0097a000(uVar3,&DAT_0068f18c);
  }
  else {
    thunk_FUN_0097a000(uVar3,*(undefined4 *)(*(int *)(iVar2 + 8) + 4));
  }
  FUN_00428dd0(iVar2);
  FUN_004db9cc(uVar3,0xa5);
  FUN_0051d5d8(uVar3,2);
  FUN_004dbe84(uVar3,9,0,0);
  FUN_004e7020(uVar3,0xffffffff,0);
  FUN_004e4820(uVar3,&DAT_00826054,0);
  FUN_004d3c7c(uVar1,&LAB_004711ac,7,0);
  uVar1 = FUN_004d992c(DAT_008c74d8);
  FUN_004db8f0(uVar1,0x168,0x28);
  FUN_004e6e70(uVar1,0,0);
  FUN_004e6e50(uVar1,0xff000000,0);
  FUN_004e6d90(uVar1,0,0);
  FUN_004e04b8(uVar1,0);
  FUN_004e0518(uVar1,0);
  FUN_004d9960(uVar1,0x4000);
  return;
}

