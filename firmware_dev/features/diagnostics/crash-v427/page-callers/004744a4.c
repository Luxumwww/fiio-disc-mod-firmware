
void FUN_004744a4(undefined4 param_1)

{
  undefined4 uVar1;
  undefined4 uVar2;
  int iVar3;
  
  DAT_008c7500 = param_1;
  DAT_008c751c = FUN_004d992c(0);
  FUN_004db8f0(DAT_008c751c,0x168,0x168);
  uVar1 = DAT_008c751c;
  FUN_004e6d40(DAT_008c751c,0,0);
  FUN_004e6d50(uVar1,0,0);
  FUN_004e6d20(uVar1,0,0);
  FUN_004e6d30(uVar1,0,0);
  FUN_004e6e70(DAT_008c751c,0,0);
  FUN_004e6d80(DAT_008c751c,0xff000000,0);
  FUN_004dbe84(DAT_008c751c,1,0,0);
  FUN_004e7090(DAT_008c751c,0,0);
  FUN_004e6d60(DAT_008c751c,5,0);
  FUN_004e04b8(DAT_008c751c,0);
  FUN_004d3c7c(DAT_008c751c,FUN_00473f70,1,0);
  FUN_004d3c7c(DAT_008c751c,FUN_00473f70,2,0);
  FUN_004d3c7c(DAT_008c751c,FUN_00473f70,8,0);
  thunk_FUN_00952500(DAT_008c751c);
  uVar1 = FUN_0051c364(DAT_008c751c);
  uVar2 = FUN_0046f51c();
  iVar3 = FUN_00428900("/usr/project/config/ui/set_menu/audio_setting.json",uVar2);
  if (iVar3 == 0) {
    thunk_FUN_0097a000(uVar1,&DAT_0068baac);
  }
  else {
    thunk_FUN_0097a000(uVar1,*(undefined4 *)(*(int *)(iVar3 + 8) + 8));
  }
  FUN_00428dd0(iVar3);
  FUN_004db9cc(uVar1,0xb4);
  FUN_0051d5d8(uVar1,2);
  FUN_004dbe84(uVar1,2,0,0x10);
  FUN_004e7020(uVar1,0xfffe0156,0);
  FUN_004e4820(uVar1,&DAT_00826054,0);
  DAT_008c7518 = FUN_0051c364(DAT_008c751c);
  FUN_0051d4b0(DAT_008c7518,"%d",DAT_008c8a75);
  FUN_004dbe84(DAT_008c7518,2,0,0x53);
  FUN_004e7020(DAT_008c7518,0xffffffff,0);
  FUN_004e7040(DAT_008c7518,&PTR_LAB_007bb120,0);
  DAT_008c7514 = FUN_0052151c(DAT_008c751c);
  FUN_004db8f0(DAT_008c7514,0xfa,0x2f);
  FUN_00518b54(DAT_008c7514,0,0x78);
  FUN_005188ec(DAT_008c7514,DAT_008c8a75,0);
  FUN_00518bc8(DAT_008c7514,0);
  FUN_004e70b0(DAT_008c7514,0,0x30000);
  FUN_004dbe84(DAT_008c7514,9,0,0);
  FUN_004d3c7c(DAT_008c7514,&LAB_00473ee0,0x1c,0);
  DAT_008c750c = FUN_0047424c(DAT_008c751c,0x3a,
                              "D:/usr/project/config/img/main_screen/main_1/decrease.png");
  DAT_008c7510 = FUN_0047424c(DAT_008c751c,0x104,
                              "D:/usr/project/config/img/main_screen/main_1/add.png");
  return;
}

