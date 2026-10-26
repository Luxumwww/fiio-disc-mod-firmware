INFO  SettingsDump.java> 
void FUN_00465260(undefined4 param_1)

{
  undefined4 uVar1;
  int iVar2;
  
  DAT_008dd940 = param_1;
  DAT_008dd944 = FUN_004e969c(0);
  FUN_004eb680(DAT_008dd944,0x168,0x168);
  uVar1 = DAT_008dd944;
  FUN_004f6b10(DAT_008dd944,0,0);
  FUN_004f6b20(uVar1,0,0);
  FUN_004f6af0(uVar1,0,0);
  FUN_004f6b00(uVar1,0,0);
  FUN_004f6c40(DAT_008dd944,0,0);
  FUN_004f6b50(DAT_008dd944,0xff000000,0);
  FUN_004ebc14(DAT_008dd944,1,0,0);
  FUN_004f6e60(DAT_008dd944,0,0);
  FUN_004f6b30(DAT_008dd944,5,0);
  FUN_004f0268(DAT_008dd944,0);
  FUN_004e39ec(DAT_008dd944,FUN_004650d0,1,0);
  FUN_004e39ec(DAT_008dd944,FUN_004650d0,2,0);
  FUN_004e39ec(DAT_008dd944,FUN_004650d0,8,0);
  FUN_004e2734(DAT_008dd944);
  uVar1 = FUN_00473fa8();
  iVar2 = FUN_0042e640("/usr/project/config/ui/set_menu/system_setting.json",uVar1);
  if (iVar2 == 0) {
    FUN_0046d88c(DAT_008dd944,&DAT_0069e76c,0xb4fb);
  }
  else {
    FUN_0046d88c(DAT_008dd944,*(undefined4 *)(*(int *)(iVar2 + 8) + 0x38),0xb4fb);
  }
  FUN_0042eb10(iVar2);
  FUN_00487738(DAT_008dd944,0xad,0x29,1,0x5e,0xb4,0xff212121,0xff00b4fb,1,FUN_0046512c);
  FUN_00487738(DAT_008dd944,0xad,0x29,1,0x5e,0xfd,0xfffe0156,0xffffffff,0,&LAB_00465214);
  FUN_004e2734(DAT_008dd944);
  return;
}

 (GhidraScript)  
