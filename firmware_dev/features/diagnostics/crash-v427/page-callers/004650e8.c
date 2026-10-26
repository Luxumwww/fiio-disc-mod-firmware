
void FUN_004650e8(void)

{
  undefined4 uVar1;
  int iVar2;
  undefined4 uVar3;
  undefined4 uVar4;
  int iVar5;
  undefined4 *puVar6;
  int iVar7;
  int iVar8;
  
  if (DAT_008c71e8 != 0) {
    uVar1 = FUN_0046f51c();
    iVar2 = FUN_00428900("/usr/project/config/ui/set_menu/classify_playlist.json",uVar1);
    if (iVar2 == 0) {
      SYNC(0);
      SYNC(0);
      if (DAT_00846390 != 0) {
        FUN_0048a0ec(DAT_00846390,"process/ui/snowsky_disc/view/src/class_play_screen.c",0x34,
                     "refresh_language_label_ui",0x19,0x6d,100,"Failed to parse UI config file\n");
      }
      FUN_00428dd0(0);
    }
    else {
      uVar1 = FUN_004e8a80(DAT_008c71e8,0);
      iVar8 = 0x825dc4;
      iVar7 = 0;
      thunk_FUN_0097a000(uVar1,*(undefined4 *)(*(int *)(iVar2 + 8) + 0x18));
      do {
        puVar6 = (undefined4 *)(iVar8 + 8);
        iVar8 = iVar8 + 0xc;
        uVar1 = FUN_004e8a80(*puVar6,1);
        puVar6 = (undefined4 *)(*(int *)(iVar2 + 8) + iVar7);
        iVar7 = iVar7 + 4;
        thunk_FUN_0097a000(uVar1,*puVar6);
      } while (iVar7 != 0x18);
      FUN_00428dd0(iVar2);
    }
    thunk_FUN_00952500(DAT_008c71e8);
    return;
  }
  DAT_008c71e8 = FUN_004d992c(0);
  FUN_004e6d80(DAT_008c71e8,0xff000000,0);
  FUN_004e6d90(DAT_008c71e8,0xff,0);
  FUN_004db8f0(DAT_008c71e8,0x168,400);
  FUN_004e04b8(DAT_008c71e8,0);
  FUN_004e6e50(DAT_008c71e8,0xff000000,0);
  FUN_004e6e60(DAT_008c71e8,0,0);
  uVar1 = DAT_008c71e8;
  FUN_004e6d40(DAT_008c71e8,0,0);
  FUN_004e6d50(uVar1,0,0);
  FUN_004e6d20(uVar1,0,0);
  FUN_004e6d30(uVar1,0,0);
  FUN_004d3c7c(DAT_008c71e8,FUN_00464b00,1,0);
  FUN_004d3c7c(DAT_008c71e8,FUN_00464b00,2,0);
  FUN_004d3c7c(DAT_008c71e8,FUN_00464b00,8,0);
  uVar1 = FUN_0051c364(DAT_008c71e8);
  uVar3 = FUN_0046f51c();
  iVar2 = FUN_00428900("/usr/project/config/ui/set_menu/classify_playlist.json",uVar3);
  if (iVar2 == 0) {
    FUN_00428dd0(0);
    return;
  }
  iVar8 = 0x34;
  iVar7 = 0;
  thunk_FUN_0097a000(uVar1,*(undefined4 *)(*(int *)(iVar2 + 8) + 0x18));
  FUN_00428dd0(iVar2);
  FUN_004e7020(uVar1,0xffff0000,0);
  FUN_004e4820(uVar1,&DAT_00826054,0);
  FUN_004e6d90(uVar1,0,0);
  FUN_004e6e70(uVar1,0,0);
  FUN_004dbe84(uVar1,2,0,10);
  uVar1 = DAT_008c71e8;
  iVar2 = 0x825dc4;
  do {
    uVar3 = FUN_00518ca4(uVar1);
    *(undefined4 *)(iVar2 + 8) = uVar3;
    FUN_004db8f0(uVar3,0x168,0x36);
    FUN_004e6d90(*(undefined4 *)(iVar2 + 8),0,0);
    FUN_004e6e60(*(undefined4 *)(iVar2 + 8),0,0);
    FUN_004e6ee0(*(undefined4 *)(iVar2 + 8),0,0);
    uVar3 = *(undefined4 *)(iVar2 + 8);
    FUN_004e6d40(uVar3,0,0);
    FUN_004e6d50(uVar3,0,0);
    FUN_004e6d20(uVar3,0,0);
    FUN_004e6d30(uVar3,0,0);
    FUN_004e6d90(*(undefined4 *)(iVar2 + 8),0xb2,0x20);
    FUN_004e6d80(*(undefined4 *)(iVar2 + 8),0xff444444,0x20);
    FUN_004e6f20(*(undefined4 *)(iVar2 + 8),0xff888888,0x20);
    FUN_004d9960(*(undefined4 *)(iVar2 + 8),0x4000);
    FUN_004dbe84(*(undefined4 *)(iVar2 + 8),0x10,0,iVar8);
    FUN_004d3c7c(*(undefined4 *)(iVar2 + 8),FUN_00464a30,7,iVar7);
    uVar3 = FUN_0051af10(*(undefined4 *)(iVar2 + 8));
    FUN_0051af44(uVar3,*(undefined4 *)(iVar2 + 4));
    FUN_004dbe84(uVar3,7,0x2c,0);
    uVar3 = FUN_0051c364(*(undefined4 *)(iVar2 + 8));
    uVar4 = FUN_0046f51c();
    iVar5 = FUN_00428900("/usr/project/config/ui/set_menu/classify_playlist.json",uVar4);
    if (iVar5 == 0) {
      SYNC(0);
      SYNC(0);
      if (DAT_00846390 != 0) {
        FUN_0048a0ec(DAT_00846390,"process/ui/snowsky_disc/view/src/class_play_screen.c",0x34,
                     "init_main_menu",0xe,0x99,100,"Failed to parse UI config file\n");
      }
      FUN_00428dd0(0);
      break;
    }
    iVar8 = iVar8 + 0x32;
    thunk_FUN_0097a000(uVar3,*(undefined4 *)(*(int *)(iVar5 + 8) + iVar7 * 4));
    iVar7 = iVar7 + 1;
    FUN_00428dd0(iVar5);
    FUN_004db9cc(uVar3,200);
    FUN_0051d5d8(uVar3,3);
    FUN_004e7080(uVar3,1,0);
    FUN_004e4820(uVar3,&_fbss,0);
    FUN_004dbe84(uVar3,7,0x5f,0);
    uVar3 = FUN_0051af10(*(undefined4 *)(iVar2 + 8));
    FUN_0051af44(uVar3,"D:/usr/project/config/img/main_screen/main_1/in_right.png");
    FUN_004dbe84(uVar3,8,0xffffffde,0);
    iVar2 = iVar2 + 0xc;
  } while (iVar8 != 0x160);
  uVar1 = FUN_004d992c(DAT_008c71e8);
  FUN_004dbe84(uVar1,1,0,0x168);
  FUN_004db8f0(uVar1,0x140,0x28);
  FUN_004e6e70(uVar1,0,0);
  FUN_004e6e50(uVar1,0xffffffff,0);
  FUN_004e6d90(uVar1,0,0);
  FUN_004e04b8(uVar1,0);
  FUN_004e0518(uVar1,0);
  FUN_004d9960(uVar1,0x4000);
  thunk_FUN_00952500(DAT_008c71e8);
  return;
}

