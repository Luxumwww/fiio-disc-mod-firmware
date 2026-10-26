INFO  SettingsDump.java> 
void FUN_0045ebc0(undefined4 param_1)

{
  undefined4 uVar1;
  int iVar2;
  
  DAT_008dd79c = param_1;
  DAT_008dd798 = FUN_004e969c(0);
  FUN_004eb680(DAT_008dd798,0x168,0x168);
  FUN_004f6e60(DAT_008dd798,0xb4,0);
  uVar1 = DAT_008dd798;
  FUN_004f6b10(DAT_008dd798,0,0);
  FUN_004f6b20(uVar1,0,0);
  FUN_004f6af0(uVar1,0,0);
  FUN_004f6b00(uVar1,0,0);
  FUN_004f6c40(DAT_008dd798,0,0);
  FUN_004f6b50(DAT_008dd798,0xff000000,0);
  FUN_005030a0(DAT_008dd798,1);
  FUN_005030e4(DAT_008dd798,0,0,2);
  FUN_004ebc14(DAT_008dd798,1,0,0);
  FUN_004f6e60(DAT_008dd798,0,0);
  FUN_004f6b30(DAT_008dd798,5,0);
  FUN_004f0268(DAT_008dd798,0);
  FUN_004f02c8(DAT_008dd798,0xc);
  FUN_004e39ec(DAT_008dd798,FUN_0045eaec,1,0);
  FUN_004e39ec(DAT_008dd798,FUN_0045eaec,2,0);
  FUN_004e39ec(DAT_008dd798,FUN_0045eaec,8,0);
  uVar1 = FUN_00473fa8();
  iVar2 = FUN_0042e640("/usr/project/config/ui/set_menu/system_setting.json",uVar1);
  if (iVar2 == 0) {
    FUN_0046d88c(DAT_008dd798,&DAT_0069db2c,0xb4fb);
  }
  else {
    FUN_0046d88c(DAT_008dd798,*(undefined4 *)(*(int *)(iVar2 + 8) + 0x34),0xb4fb);
  }
  FUN_0042eb10(iVar2);
  uVar1 = FUN_00473fa8();
  iVar2 = FUN_0042e640("/usr/project/config/ui/set_menu/others.json",uVar1);
  if (iVar2 == 0) {
    FUN_00485ef8(DAT_008dd798,&DAT_0069e10c,0,&LAB_0045eefc,0);
    FUN_00485ef8(DAT_008dd798,&DAT_0069e11c,1,&LAB_0045eefc,0);
  }
  else {
    FUN_00485ef8(DAT_008dd798,*(undefined4 *)(*(int *)(iVar2 + 8) + 0x1c),0,&LAB_0045eefc,0);
    FUN_00485ef8(DAT_008dd798,*(undefined4 *)(*(int *)(iVar2 + 8) + 0x20),1,&LAB_0045eefc,0);
  }
  FUN_0042eb10(iVar2);
  uVar1 = FUN_004e969c(DAT_008dd798);
  FUN_004eb680(uVar1,0x140,0x28);
  FUN_004f6c40(uVar1,0,0);
  FUN_004f6c20(uVar1,0xff000000,0);
  FUN_004f6b60(uVar1,0,0);
  FUN_004f0268(uVar1,0);
  FUN_004f02c8(uVar1,0);
  FUN_004e96d0(uVar1,0x4000);
  FUN_00525320(&LAB_0045eab0,5,0);
  FUN_004e2734(DAT_008dd798);
  FUN_0046e390(DAT_008e1719);
  FUN_0045cc84(DAT_008e1725);
  FUN_00481a78(0);
  return;
}

 (GhidraScript)  
