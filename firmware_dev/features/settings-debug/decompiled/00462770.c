INFO  SettingsDump.java> 
void FUN_00462770(undefined4 param_1)

{
  undefined4 uVar1;
  int iVar2;
  
  DAT_008dd8a0 = param_1;
  DAT_008dd8a4 = FUN_004e969c(0);
  FUN_004eb680(DAT_008dd8a4,0x168,0x168);
  FUN_004f6e60(DAT_008dd8a4,0xb4,0);
  uVar1 = DAT_008dd8a4;
  FUN_004f6b10(DAT_008dd8a4,0,0);
  FUN_004f6b20(uVar1,0,0);
  FUN_004f6af0(uVar1,0,0);
  FUN_004f6b00(uVar1,0,0);
  FUN_004f6c40(DAT_008dd8a4,0,0);
  FUN_004f6b50(DAT_008dd8a4,0xff000000,0);
  FUN_005030a0(DAT_008dd8a4,1);
  FUN_005030e4(DAT_008dd8a4,0,0,2);
  FUN_004ebc14(DAT_008dd8a4,1,0,0);
  FUN_004f6e60(DAT_008dd8a4,0,0);
  FUN_004f6b30(DAT_008dd8a4,5,0);
  FUN_004f0268(DAT_008dd8a4,0);
  FUN_004f02c8(DAT_008dd8a4,0xc);
  FUN_004e39ec(DAT_008dd8a4,&LAB_004626a0,1,0);
  FUN_004e39ec(DAT_008dd8a4,&LAB_004626a0,2,0);
  FUN_004e39ec(DAT_008dd8a4,&LAB_004626a0,8,0);
  uVar1 = FUN_00473fa8();
  iVar2 = FUN_0042e640("/usr/project/config/ui/set_menu/system_setting.json",uVar1);
  if (iVar2 == 0) {
    FUN_0046d88c(DAT_008dd8a4,&DAT_0069dab4,0xb4fb);
  }
  else {
    FUN_0046d88c(DAT_008dd8a4,*(undefined4 *)(*(int *)(iVar2 + 8) + 0x24),0xb4fb);
  }
  FUN_0042eb10(iVar2);
  uVar1 = FUN_00473fa8();
  iVar2 = FUN_0042e640("/usr/project/config/ui/set_menu/others.json",uVar1);
  if (iVar2 == 0) {
    FUN_00485ef8(DAT_008dd8a4,&DAT_0069d9b8,0,FUN_004626fc,0);
    FUN_00485ef8(DAT_008dd8a4,&DAT_0069e480,1,FUN_004626fc,0);
    FUN_00485ef8(DAT_008dd8a4,&DAT_0069e478,2,FUN_004626fc,0);
  }
  else {
    FUN_00485ef8(DAT_008dd8a4,*(undefined4 *)(*(int *)(iVar2 + 8) + 0x16c),0,FUN_004626fc,0);
    FUN_00485ef8(DAT_008dd8a4,*(undefined4 *)(*(int *)(iVar2 + 8) + 0x170),1,FUN_004626fc,0);
    FUN_00485ef8(DAT_008dd8a4,*(undefined4 *)(*(int *)(iVar2 + 8) + 0x174),2,FUN_004626fc,0);
  }
  FUN_0042eb10(iVar2);
  FUN_004e2734(DAT_008dd8a4);
  return;
}

 (GhidraScript)  
