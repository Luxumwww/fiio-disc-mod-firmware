
void FUN_00453078(void)

{
  undefined4 uVar1;
  undefined4 uVar2;
  int iVar3;
  undefined4 local_60;
  undefined1 *local_5c;
  undefined4 local_40;
  undefined4 local_3c;
  undefined4 local_38;
  undefined4 local_34;
  undefined2 local_20;
  
  FUN_0046c770();
  DAT_008c6920 = 0;
  thunk_FUN_00953000(1);
  if (DAT_008c6924 != 0) {
    FUN_005137bc();
    DAT_008c6924 = 0;
  }
  DAT_008c6930 = 1;
  DAT_008c6928 = FUN_004d992c(0);
  FUN_004db8f0(DAT_008c6928,0x20000064);
  FUN_004e6d80(DAT_008c6928,0xff000000,0);
  FUN_004d3c7c(DAT_008c6928,&LAB_004527d8,0xc,0);
  uVar1 = FUN_0051c364(DAT_008c6928);
  uVar2 = FUN_0046f51c();
  iVar3 = FUN_00428900("/usr/project/config/ui/set_menu/popup.json",uVar2);
  if (iVar3 == 0) {
    thunk_FUN_0097a000(uVar1,&DAT_0068ca84);
  }
  else {
    thunk_FUN_0097a000(uVar1,**(undefined4 **)(iVar3 + 8));
  }
  FUN_00428dd0(iVar3);
  FUN_004e4820(uVar1,&DAT_00826054,0);
  FUN_004e7020(uVar1,0xffe67b10,0);
  FUN_004dbe84(uVar1,2,0,0x11);
  uVar1 = FUN_0051c364(DAT_008c6928);
  thunk_FUN_0097a000(uVar1,&DAT_0068ca94);
  FUN_004e4820(uVar1,&DAT_00826054,0);
  FUN_004e7020(uVar1,0xffe67b10,0);
  FUN_004dbe84(uVar1,5,0,0xffffffee);
  iVar3 = DAT_008c6928;
  *(undefined4 *)(DAT_008c6928 + 0x10) = uVar1;
  uVar1 = FUN_0051af10(iVar3);
  FUN_0051af44(uVar1,"D:/usr/project/config/img/main_screen/main_1/scan_bg.png");
  FUN_004dbe84(uVar1,9,0,0);
  thunk_FUN_00952500(DAT_008c6928);
  uVar1 = FUN_0051af10(DAT_008c6928);
  FUN_004dbe84(uVar1,1,0x6a,0xd2);
  FUN_004d9a88(uVar1,0x10);
  FUN_0051af44(uVar1,"D:/usr/project/config/img/main_screen/main_1/scan_music_1.png");
  FUN_004db8f0(uVar1,0x1a,0x1a);
  uVar1 = FUN_0051af10(DAT_008c6928);
  FUN_004dbe84(uVar1,1,0xd6,0xe4);
  FUN_004d9a88(uVar1,0x10);
  FUN_0051af44(uVar1,"D:/usr/project/config/img/main_screen/main_1/scan_music_2.png");
  FUN_004db8f0(uVar1,0x15,0x15);
  uVar1 = FUN_0051af10(DAT_008c6928);
  FUN_004dbe84(uVar1,1,0xe5,0x7b);
  FUN_004d9a88(uVar1,0x10);
  FUN_0051af44(uVar1,"D:/usr/project/config/img/main_screen/main_1/scan_music_3.png");
  FUN_004db8f0(uVar1,0x11,0x12);
  uVar1 = FUN_0051af10(DAT_008c6928);
  FUN_004dbe84(uVar1,1,0x88,0x67);
  FUN_004d9a88(uVar1,0x10);
  FUN_0051af44(uVar1,"D:/usr/project/config/img/main_screen/main_1/scan_bg_music.png");
  FUN_004db8f0(uVar1,0x13,0x19);
  uVar1 = FUN_0051af10(DAT_008c6928);
  FUN_0051af44(uVar1,"D:/usr/project/config/img/main_screen/main_1/scan_white.png");
  FUN_004dbe84(uVar1,1,0xb2,0x43);
  FUN_0051bf20(uVar1,0,0x72);
  FUN_0050b3f0(&local_60);
  local_5c = &LAB_00452770;
  local_38 = 0xe10;
  local_34 = 2000;
  local_20 = 0xffff;
  local_40 = 0;
  local_3c = 0;
  local_60 = uVar1;
  thunk_FUN_00976000(&local_60);
  FUN_00465b54();
  FUN_004428bc(&DAT_006827fc,0,0);
  FUN_00434248();
  return;
}

