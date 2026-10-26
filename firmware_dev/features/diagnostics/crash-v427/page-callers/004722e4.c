
void FUN_004722e4(int param_1,char *param_2)

{
  undefined4 *puVar1;
  undefined4 uVar2;
  int iVar3;
  int iVar4;
  undefined4 uVar5;
  
  DAT_008c74b0 = 0;
  uVar2 = FUN_0046f51c();
  iVar3 = FUN_00428900("/usr/project/config/ui/set_menu/bt.json",uVar2);
  puVar1 = DAT_008c74b4;
  if (DAT_008c74bd == '\x01') {
    uVar2 = *(undefined4 *)(DAT_008c74b8 + 0x14);
    if (param_1 == 0) {
      if (iVar3 == 0) {
        thunk_FUN_0097a000(uVar2,&DAT_0068f1bc);
      }
      else {
        thunk_FUN_0097a000(uVar2,*(undefined4 *)(*(int *)(iVar3 + 8) + 0x44));
      }
      uVar2 = DAT_008c74c8;
      iVar4 = DAT_008c74b8;
      *(undefined1 *)(DAT_008c74b8 + 0xc) = 0;
      uVar5 = *(undefined4 *)(iVar4 + 4);
      FUN_004e8224(uVar2);
      DAT_008c74c1 = 0;
      FUN_004db8f0(DAT_008c74c8,0x168,0x14);
      DAT_008c74d0 = FUN_004d992c(0);
      FUN_004db8f0(DAT_008c74d0,0x168,0x168);
      uVar2 = DAT_008c74d0;
      FUN_004e6d40(DAT_008c74d0,0,0);
      FUN_004e6d50(uVar2,0,0);
      FUN_004e6d20(uVar2,0,0);
      FUN_004e6d30(uVar2,0,0);
      FUN_004e6e70(DAT_008c74d0,0,0);
      FUN_004e6d80(DAT_008c74d0,0xff000000,0);
      uVar2 = FUN_0051af10(DAT_008c74d0);
      FUN_0051af44(uVar2,"D:/usr/project/config/img/main_screen/bt_set/bt_connect_fail.png");
      FUN_004dbe84(uVar2,2,0,0x4d);
      uVar2 = FUN_0051c364(DAT_008c74d0);
      thunk_FUN_0097a000(uVar2,uVar5);
      FUN_004dbe84(uVar2,2,0,0xa2);
      FUN_004e7020(uVar2,0xffffffff,0);
      FUN_004e4820(uVar2,&DAT_00826054,0);
      uVar2 = FUN_0051c364(DAT_008c74d0);
      uVar5 = FUN_0046f51c();
      iVar4 = FUN_00428900("/usr/project/config/ui/set_menu/bt.json",uVar5);
      if (iVar4 == 0) {
        thunk_FUN_0097a000(uVar2,&DAT_0068f1bc);
      }
      else {
        thunk_FUN_0097a000(uVar2,*(undefined4 *)(*(int *)(iVar4 + 8) + 0x44));
      }
      FUN_00428dd0(iVar4);
      FUN_004dbe84(uVar2,2,0,0xcd);
      FUN_004e7020(uVar2,0xffffffff,0);
      FUN_004e4820(uVar2,&DAT_00826054,0);
      FUN_0042ae68(DAT_008c74d0,0x7f,0x29,2,0,0x11b,0xff2d83ff,0xffffffff,1,FUN_00470788);
      thunk_FUN_00952500(DAT_008c74d0);
      DAT_008c74bd = '\x03';
      DAT_008c8a62 = 0;
    }
    else {
      if (iVar3 == 0) {
        thunk_FUN_0097a000(uVar2,&DAT_0068de70);
      }
      else {
        thunk_FUN_0097a000(uVar2,*(undefined4 *)(*(int *)(iVar3 + 8) + 0x24));
      }
      uVar2 = DAT_008c74c8;
      iVar4 = DAT_008c74b8;
      *(undefined1 *)(DAT_008c74b8 + 0xc) = 2;
      uVar5 = *(undefined4 *)(iVar4 + 4);
      FUN_004e8224(uVar2);
      DAT_008c74c1 = 0;
      FUN_004db8f0(DAT_008c74c8,0x168,0x14);
      DAT_008c74d0 = FUN_004d992c(0);
      FUN_004db8f0(DAT_008c74d0,0x168,0x168);
      uVar2 = DAT_008c74d0;
      FUN_004e6d40(DAT_008c74d0,0,0);
      FUN_004e6d50(uVar2,0,0);
      FUN_004e6d20(uVar2,0,0);
      FUN_004e6d30(uVar2,0,0);
      FUN_004e6e70(DAT_008c74d0,0,0);
      FUN_004e6d80(DAT_008c74d0,0xff000000,0);
      uVar2 = FUN_0051af10(DAT_008c74d0);
      FUN_0051af44(uVar2,"D:/usr/project/config/img/main_screen/bt_set/bt_connect_ok.png");
      FUN_004dbe84(uVar2,2,0,0x4d);
      uVar2 = FUN_0051c364(DAT_008c74d0);
      thunk_FUN_0097a000(uVar2,uVar5);
      FUN_004dbe84(uVar2,2,0,0xa2);
      FUN_004e7020(uVar2,0xffffffff,0);
      FUN_004e4820(uVar2,&DAT_00826054,0);
      uVar2 = FUN_0051c364(DAT_008c74d0);
      uVar5 = FUN_0046f51c();
      iVar4 = FUN_00428900("/usr/project/config/ui/set_menu/bt.json",uVar5);
      if (iVar4 == 0) {
        thunk_FUN_0097a000(uVar2,&DAT_0068f1ac);
      }
      else {
        thunk_FUN_0097a000(uVar2,*(undefined4 *)(*(int *)(iVar4 + 8) + 0x40));
      }
      FUN_00428dd0(iVar4);
      FUN_004dbe84(uVar2,2,0,0xcd);
      FUN_004e7020(uVar2,0xffffffff,0);
      FUN_004e4820(uVar2,&DAT_00826054,0);
      FUN_0042ae68(DAT_008c74d0,0x7f,0x29,2,0,0x11b,0xff2d83ff,0xffffffff,1,FUN_00470788);
      thunk_FUN_00952500(DAT_008c74d0);
      DAT_008c74bd = '\x03';
      DAT_008c8a62 = 1;
    }
  }
  else if (DAT_008c74bd == '\x02') {
    iVar4 = strcmp((char *)*DAT_008c74b4,param_2);
    if (iVar4 != 0) {
      return;
    }
    uVar2 = puVar1[7];
    if (param_1 == 0) {
      *(undefined1 *)(puVar1 + 3) = 0;
      DAT_008c8a62 = 0;
      if (iVar3 == 0) {
        thunk_FUN_0097a000(uVar2,&DAT_0068d8b8);
        thunk_FUN_0097a000(DAT_008c74b4[6],&DAT_0068f094);
        thunk_FUN_0097a000(DAT_008c74b4[5],&DAT_0068f0a4);
      }
      else {
        thunk_FUN_0097a000(uVar2,*(undefined4 *)(*(int *)(iVar3 + 8) + 0x1c));
        thunk_FUN_0097a000(DAT_008c74b4[6],*(undefined4 *)(*(int *)(iVar3 + 8) + 0x3c));
        thunk_FUN_0097a000(DAT_008c74b4[5],*(undefined4 *)(*(int *)(iVar3 + 8) + 0x20));
      }
    }
    else {
      *(undefined1 *)(puVar1 + 3) = 2;
      DAT_008c8a62 = 1;
      if (iVar3 == 0) {
        thunk_FUN_0097a000(uVar2,&DAT_0068de70);
        thunk_FUN_0097a000(DAT_008c74b4[6],&DAT_0068f0b8);
        thunk_FUN_0097a000(DAT_008c74b4[5],&DAT_0068de70);
      }
      else {
        thunk_FUN_0097a000(uVar2,*(undefined4 *)(*(int *)(iVar3 + 8) + 0x24));
        thunk_FUN_0097a000(DAT_008c74b4[6],*(undefined4 *)(*(int *)(iVar3 + 8) + 0x14));
        thunk_FUN_0097a000(DAT_008c74b4[5],*(undefined4 *)(*(int *)(iVar3 + 8) + 0x24));
      }
    }
  }
  FUN_0046c680(DAT_008c8a62);
  FUN_00428dd0(iVar3);
  return;
}

