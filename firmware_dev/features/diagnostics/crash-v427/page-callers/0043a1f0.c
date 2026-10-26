
void FUN_0043a1f0(int param_1)

{
  undefined4 uVar1;
  undefined4 uVar2;
  int iVar3;
  
  FUN_004e80c8(&DAT_008c5a80);
  DAT_008c5a80 = FUN_004d992c(0);
  FUN_004db8f0(DAT_008c5a80,0x168,0x168);
  uVar1 = DAT_008c5a80;
  FUN_004e6d40(DAT_008c5a80,0,0);
  FUN_004e6d50(uVar1,0,0);
  FUN_004e6d20(uVar1,0,0);
  FUN_004e6d30(uVar1,0,0);
  FUN_004e6e70(DAT_008c5a80,0,0);
  FUN_004e6d80(DAT_008c5a80,0xff000000,0);
  uVar1 = FUN_0051af10(DAT_008c5a80);
  FUN_0051af44(uVar1,"D:/usr/project/config/img/main_screen/bt_set/bt_connect_ok.png");
  FUN_004dbe84(uVar1,2,0,0x4d);
  uVar1 = FUN_0051c364(DAT_008c5a80);
  if (param_1 == 0) {
    uVar2 = FUN_0046f51c();
    iVar3 = FUN_00428900("/usr/project/config/ui/set_menu/others.json",uVar2);
    if (iVar3 == 0) {
      thunk_FUN_0097a000(uVar1,&DAT_0068aff0);
    }
    else {
      thunk_FUN_0097a000(uVar1,*(undefined4 *)(*(int *)(iVar3 + 8) + 0x94));
    }
    FUN_00428dd0(iVar3);
  }
  FUN_004dbe84(uVar1,2,0,0xb9);
  FUN_004e7020(uVar1,0xffffffff,0);
  FUN_004e4820(uVar1,&DAT_00826054,0);
  FUN_0042ae68(DAT_008c5a80,0x7f,0x29,2,0,0x11b,0xff2d83ff,0xffffffff,1,FUN_00439964);
  thunk_FUN_00952500(DAT_008c5a80);
  return;
}

