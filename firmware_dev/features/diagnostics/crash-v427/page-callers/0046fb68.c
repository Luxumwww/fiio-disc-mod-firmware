
void FUN_0046fb68(undefined4 param_1)

{
  undefined4 uVar1;
  undefined4 uVar2;
  int iVar3;
  
  DAT_008c7460 = param_1;
  DAT_008c7474 = FUN_004d992c(0);
  FUN_004db8f0(DAT_008c7474,0x168,0x168);
  uVar1 = DAT_008c7474;
  FUN_004e6d40(DAT_008c7474,0,0);
  FUN_004e6d50(uVar1,0,0);
  FUN_004e6d20(uVar1,0,0);
  FUN_004e6d30(uVar1,0,0);
  FUN_004e6e70(DAT_008c7474,0,0);
  FUN_004e6d80(DAT_008c7474,0xff000000,0);
  FUN_004dbe84(DAT_008c7474,1,0,0);
  FUN_004e7090(DAT_008c7474,0,0);
  FUN_004e6d60(DAT_008c7474,5,0);
  FUN_004e04b8(DAT_008c7474,0);
  FUN_004d3c7c(DAT_008c7474,&LAB_0046fa84,1,0);
  FUN_004d3c7c(DAT_008c7474,&LAB_0046fa84,2,0);
  FUN_004d3c7c(DAT_008c7474,&LAB_0046fa84,8,0);
  thunk_FUN_00952500(DAT_008c7474);
  uVar1 = FUN_0051c364(DAT_008c7474);
  uVar2 = FUN_0046f51c();
  iVar3 = FUN_00428900("/usr/project/config/ui/set_menu/audio_setting.json",uVar2);
  if (iVar3 == 0) {
    thunk_FUN_0097a000(uVar1,&DAT_0068bda0);
  }
  else {
    thunk_FUN_0097a000(uVar1,*(undefined4 *)(*(int *)(iVar3 + 8) + 0x50));
  }
  FUN_00428dd0(iVar3);
  FUN_004db9cc(uVar1,0xb4);
  FUN_0051d5d8(uVar1,2);
  FUN_004dbe84(uVar1,2,0,0x10);
  FUN_004e7020(uVar1,0xffe67b10,0);
  FUN_004e4820(uVar1,&DAT_00826054,0);
  DAT_008c7470 = FUN_0051c364(DAT_008c7474);
  iVar3 = (int)DAT_008c8a66;
  if (iVar3 == 0) {
    FUN_0051d4b0(DAT_008c7470,"0");
  }
  else if (iVar3 < 1) {
    FUN_0051d4b0(DAT_008c7470,&DAT_0068f004,-iVar3);
  }
  else {
    FUN_0051d4b0(DAT_008c7470,&DAT_0068f000);
  }
  FUN_004dbe84(DAT_008c7470,2,0,0x53);
  FUN_004e7020(DAT_008c7470,0xffffffff,0);
  FUN_004e7040(DAT_008c7470,&PTR_LAB_007bb120,0);
  DAT_008c7468 = FUN_004d992c(DAT_008c7474);
  FUN_004db8f0(DAT_008c7468,0x32,0x32);
  FUN_004e6d80(DAT_008c7468,0xffffffff,0);
  FUN_004dbe84(DAT_008c7468,1,0x3a,0xdb);
  FUN_004e70b0(DAT_008c7468,0,0x10000);
  FUN_004d9a88(DAT_008c7468,0x10);
  FUN_004e6e70(DAT_008c7468,0,0);
  FUN_004e6d90(DAT_008c7468,0,0);
  FUN_004e6d90(DAT_008c7468,0x4c,0x20);
  uVar1 = FUN_0051af10(DAT_008c7468);
  FUN_0051af44(uVar1,"D:/usr/project/config/img/main_screen/menu_set/light_down.png");
  FUN_004dbe84(uVar1,9,0,0);
  FUN_004e6d90(DAT_008c7468,0x4c,0x20);
  FUN_004d3c7c(DAT_008c7468,&LAB_0046f918,7,0);
  DAT_008c7464 = FUN_004d992c(DAT_008c7474);
  FUN_004db8f0(DAT_008c7464,0x32,0x32);
  FUN_004e6d80(DAT_008c7464,0xffffffff,0);
  FUN_004dbe84(DAT_008c7464,1,0x104,0xdb);
  FUN_004e70b0(DAT_008c7464,0,0x10000);
  FUN_004d9a88(DAT_008c7464,0x10);
  FUN_004e6e70(DAT_008c7464,0,0);
  FUN_004e6d90(DAT_008c7464,0,0);
  FUN_004e6d90(DAT_008c7464,0x4c,0x20);
  uVar1 = FUN_0051af10(DAT_008c7464);
  FUN_0051af44(uVar1,"D:/usr/project/config/img/main_screen/menu_set/light_up.png");
  FUN_004dbe84(uVar1,9,0,0);
  FUN_004d3c7c(DAT_008c7464,&LAB_0046f918,7,1);
  DAT_008c746c = FUN_0052151c(DAT_008c7474);
  FUN_004db8f0(DAT_008c746c,0x13a,0x10);
  FUN_00518b54(DAT_008c746c,0xffffffec,0x14);
  FUN_005188ec(DAT_008c746c,(int)DAT_008c8a66,0);
  FUN_00518bc8(DAT_008c746c,1);
  FUN_004e6c10(DAT_008c746c,0x24,0x30000);
  FUN_004e6c40(DAT_008c746c,0x24,0x30000);
  FUN_004e6d80(DAT_008c746c,0xffff4c2b,0x20000);
  FUN_004e6d80(DAT_008c746c,0xffff4c2b,0x30000);
  FUN_004e6d80(DAT_008c746c,0xffffffff,0);
  FUN_004dbe84(DAT_008c746c,9,0,0);
  FUN_004d3c7c(DAT_008c746c,FUN_0046f830,0x1c,0);
  return;
}

