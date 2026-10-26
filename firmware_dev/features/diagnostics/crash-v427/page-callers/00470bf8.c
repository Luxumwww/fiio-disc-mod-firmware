
/* WARNING: Restarted to delay deadcode elimination for space: ram */

void FUN_00470bf8(void)

{
  char cVar1;
  int iVar2;
  undefined4 uVar3;
  undefined4 uVar4;
  undefined4 uVar5;
  int iVar6;
  undefined4 uVar7;
  undefined4 uVar8;
  undefined4 uVar9;
  int iVar10;
  
  iVar2 = FUN_004d3bdc();
  DAT_008c74b4 = iVar2;
  uVar3 = FUN_004d992c(0);
  FUN_004db8f0(uVar3,0x168,0x168);
  FUN_004e6d40(uVar3,0,0);
  FUN_004e6d50(uVar3,0,0);
  FUN_004e6d20(uVar3,0,0);
  FUN_004e6d30(uVar3,0,0);
  FUN_004e6e70(uVar3,0,0);
  FUN_004e6d80(uVar3,0xff000000,0);
  uVar4 = FUN_0051c364(uVar3);
  thunk_FUN_0097a000(uVar4,*(undefined4 *)(iVar2 + 4));
  FUN_004dbe84(uVar4,2,0,0x3d);
  FUN_004e7020(uVar4,0xffffffff,0);
  FUN_004e4820(uVar4,&DAT_00826054,0);
  FUN_004d9960(uVar4,0x4000);
  uVar4 = FUN_0051c364(uVar3);
  uVar5 = FUN_0046f51c();
  iVar6 = FUN_00428900("/usr/project/config/ui/set_menu/bt.json",uVar5);
  if (iVar6 == 0) {
    cVar1 = *(char *)(iVar2 + 0xc);
    if (cVar1 == '\0') {
      thunk_FUN_0097a000(uVar4,&DAT_0068d8b8);
    }
    else if (cVar1 == '\x01') {
      thunk_FUN_0097a000(uVar4,&DAT_0068f0a4);
    }
    else if (cVar1 == '\x02') {
      thunk_FUN_0097a000(uVar4,&DAT_0068de70);
    }
    else if (cVar1 == '\x03') {
      thunk_FUN_0097a000(uVar4,&DAT_0068de7c);
    }
    else {
      thunk_FUN_0097a000(uVar4,&DAT_0068f0f8);
    }
  }
  else {
    cVar1 = *(char *)(iVar2 + 0xc);
    iVar10 = *(int *)(iVar6 + 8);
    if (cVar1 == '\0') {
      thunk_FUN_0097a000(uVar4,*(undefined4 *)(iVar10 + 0x1c));
    }
    else if (cVar1 == '\x01') {
      thunk_FUN_0097a000(uVar4,*(undefined4 *)(iVar10 + 0x20));
    }
    else if (cVar1 == '\x02') {
      thunk_FUN_0097a000(uVar4,*(undefined4 *)(iVar10 + 0x24));
    }
    else if (cVar1 == '\x03') {
      thunk_FUN_0097a000(uVar4,*(undefined4 *)(iVar10 + 8));
    }
    else {
      thunk_FUN_0097a000(uVar4,*(undefined4 *)(iVar10 + 0x2c));
    }
  }
  FUN_004dbe84(uVar4,2,0,0x6e);
  FUN_004e7020(uVar4,0xffffffff,0);
  FUN_004e4820(uVar4,&DAT_00826054,0);
  FUN_004d9960(uVar4,0x4000);
  uVar5 = FUN_00518ca4(uVar3);
  FUN_004db8f0(uVar5,0xad,0x29);
  FUN_004dbe84(uVar5,2,0,0xb4);
  FUN_004e7090(uVar5,0x14,0);
  FUN_004e6e70(uVar5,0,0);
  FUN_004e6ee0(uVar5,0,0);
  FUN_004e6d80(uVar5,0xff292929,0);
  FUN_004d9960(uVar5,0x4000);
  uVar7 = FUN_0051c364(uVar5);
  if (iVar6 == 0) {
    if (*(char *)(iVar2 + 0xc) == '\x02') {
      thunk_FUN_0097a000(uVar7,&DAT_0068f0b8);
    }
    else {
      thunk_FUN_0097a000(uVar7,&DAT_0068f094);
    }
  }
  else if (*(char *)(iVar2 + 0xc) == '\x02') {
    thunk_FUN_0097a000(uVar7,*(undefined4 *)(*(int *)(iVar6 + 8) + 0x14));
  }
  else {
    thunk_FUN_0097a000(uVar7,*(undefined4 *)(*(int *)(iVar6 + 8) + 0x3c));
  }
  FUN_004db9cc(uVar7,0xa5);
  FUN_0051d5d8(uVar7,2);
  FUN_004dbe84(uVar7,9,0,0);
  FUN_004e7020(uVar7,0xff2d83ff,0);
  FUN_004e4820(uVar7,&DAT_00826054,0);
  uVar8 = FUN_00518ca4(uVar3);
  FUN_004db8f0(uVar8,0xad,0x29);
  FUN_004e7090(uVar8,0x14,0);
  FUN_004dbe84(uVar8,2,0,0x100);
  FUN_004e6e70(uVar8,0,0);
  FUN_004e6ee0(uVar8,0,0);
  FUN_004e6d80(uVar8,0xffed1c24,0);
  FUN_004d9960(uVar8,0x4000);
  uVar9 = FUN_0051c364(uVar8);
  if (iVar6 == 0) {
    thunk_FUN_0097a000(uVar9,&DAT_0068f108);
  }
  else {
    thunk_FUN_0097a000(uVar9,*(undefined4 *)(*(int *)(iVar6 + 8) + 0x18));
  }
  FUN_004db9cc(uVar9,0xa5);
  FUN_0051d5d8(uVar9,2);
  FUN_004dbe84(uVar9,9,0,0);
  FUN_004e7020(uVar9,0xffffffff,0);
  FUN_004e4820(uVar9,&DAT_00826054,0);
  FUN_004d3c7c(uVar3,FUN_00470818,0xc,0);
  FUN_004d3c7c(uVar3,FUN_00470ac8,8,0);
  FUN_004d3c7c(uVar8,&LAB_00470b30,7,iVar2);
  FUN_004d3c7c(uVar5,FUN_0047055c,7,0);
  iVar2 = DAT_008c74b4;
  *(undefined4 *)(DAT_008c74b4 + 0x18) = uVar7;
  *(undefined4 *)(iVar2 + 0x1c) = uVar4;
  thunk_FUN_00952500(uVar3);
  DAT_008c74bd = 2;
  FUN_00428dd0(iVar6);
  return;
}

