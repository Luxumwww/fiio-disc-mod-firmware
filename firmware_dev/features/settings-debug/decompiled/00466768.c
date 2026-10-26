INFO  SettingsDump.java> 
void FUN_00466768(undefined4 param_1)

{
  undefined4 uVar1;
  int iVar2;
  
  puts("creat theme screen");
  DAT_008dd980 = param_1;
  DAT_008dd984 = FUN_004e969c(0);
  FUN_004eb680(DAT_008dd984,0x168,0x168);
  uVar1 = DAT_008dd984;
  FUN_004f6b10(DAT_008dd984,0,0);
  FUN_004f6b20(uVar1,0,0);
  FUN_004f6af0(uVar1,0,0);
  FUN_004f6b00(uVar1,0,0);
  FUN_004f6c40(DAT_008dd984,0,0);
  FUN_004f6b50(DAT_008dd984,0xff000000,0);
  FUN_004ebc14(DAT_008dd984,1,0,0);
  FUN_004f6e60(DAT_008dd984,0,0);
  FUN_004f6b30(DAT_008dd984,5,0);
  FUN_004f0268(DAT_008dd984,0);
  FUN_004e39ec(DAT_008dd984,FUN_0046670c,1,0);
  FUN_004e39ec(DAT_008dd984,FUN_0046670c,2,0);
  FUN_004e39ec(DAT_008dd984,FUN_0046670c,8,0);
  FUN_004e2734(DAT_008dd984);
  FUN_005030a0(DAT_008dd984,1);
  FUN_005030e4(DAT_008dd984,0,0,2);
  uVar1 = FUN_00473fa8();
  iVar2 = FUN_0042e640("/usr/project/config/ui/set_menu/system_setting.json",uVar1);
  if (iVar2 == 0) {
    FUN_0046d88c(DAT_008dd984,&DAT_0069dae4,0xb4fb);
  }
  else {
    FUN_0046d88c(DAT_008dd984,*(undefined4 *)(*(int *)(iVar2 + 8) + 0x2c),0xb4fb);
  }
  FUN_0042eb10(iVar2);
  uVar1 = FUN_00473fa8();
  iVar2 = FUN_0042e640("/usr/project/config/ui/set_menu/others.json",uVar1);
  if (iVar2 == 0) {
    FUN_00485ef8(DAT_008dd984,&DAT_0069eeb0,0,FUN_004666d0,0);
    FUN_00485ef8(DAT_008dd984,&DAT_0069e4d0,1,FUN_004666d0,0);
    FUN_00485ef8(DAT_008dd984,&DAT_0069e598,2,FUN_004666d0,0);
  }
  else {
    FUN_00485ef8(DAT_008dd984,*(undefined4 *)(*(int *)(iVar2 + 8) + 8),0,FUN_004666d0,0);
    FUN_00485ef8(DAT_008dd984,*(undefined4 *)(*(int *)(iVar2 + 8) + 0xc),1,FUN_004666d0,0);
    FUN_00485ef8(DAT_008dd984,*(undefined4 *)(*(int *)(iVar2 + 8) + 0x50),2,FUN_004666d0,0);
  }
  FUN_0042eb10(iVar2);
  uVar1 = FUN_004e969c(DAT_008dd984);
  FUN_004eb680(uVar1,0x140,0x28);
  FUN_004f6c40(uVar1,0,0);
  FUN_004f6c20(uVar1,0xff000000,0);
  FUN_004f6b60(uVar1,0,0);
  FUN_004f0268(uVar1,0);
  FUN_004f02c8(uVar1,0);
  FUN_004e96d0(uVar1,0x4000);
  puts("creat theme screen success");
  return;
}

 (GhidraScript)  
