INFO  SettingsDump.java> 
void FUN_004647f8(undefined4 param_1)

{
  undefined4 uVar1;
  int iVar2;
  
  DAT_008dd900 = param_1;
  DAT_008dd914 = FUN_004e969c(0);
  FUN_004eb680(DAT_008dd914,0x168,0x168);
  uVar1 = DAT_008dd914;
  FUN_004f6b10(DAT_008dd914,0,0);
  FUN_004f6b20(uVar1,0,0);
  FUN_004f6af0(uVar1,0,0);
  FUN_004f6b00(uVar1,0,0);
  FUN_004f6c40(DAT_008dd914,0,0);
  FUN_004f6b50(DAT_008dd914,0xff000000,0);
  FUN_004ebc14(DAT_008dd914,1,0,0);
  FUN_004f6e60(DAT_008dd914,0,0);
  FUN_004f6b30(DAT_008dd914,5,0);
  FUN_004f0268(DAT_008dd914,0);
  FUN_004e39ec(DAT_008dd914,&LAB_00464530,1,0);
  FUN_004e39ec(DAT_008dd914,&LAB_00464530,2,0);
  FUN_004e39ec(DAT_008dd914,&LAB_00464530,8,0);
  FUN_004e2734(DAT_008dd914);
  FUN_005030a0(DAT_008dd914,1);
  FUN_005030e4(DAT_008dd914,0,0,2);
  uVar1 = FUN_00473fa8();
  iVar2 = FUN_0042e640("/usr/project/config/ui/set_menu/system_setting.json",uVar1);
  if (iVar2 == 0) {
    FUN_0046d88c(DAT_008dd914,&DAT_0069da84,0xb4fb);
  }
  else {
    FUN_0046d88c(DAT_008dd914,*(undefined4 *)(*(int *)(iVar2 + 8) + 0xc),0xb4fb);
  }
  FUN_0042eb10(iVar2);
  DAT_008dd904 = FUN_0046d5c0(DAT_008dd914,"0",0,FUN_0046458c);
  DAT_008dd908 = FUN_0046d5c0(DAT_008dd914,&DAT_0069e748,1,FUN_0046458c);
  DAT_008dd90c = FUN_0046d5c0(DAT_008dd914,&DAT_0069e74c,2,FUN_0046458c);
  DAT_008dd910 = FUN_0046d5c0(DAT_008dd914,&DAT_0069e750,3,FUN_0046458c);
  uVar1 = FUN_004e969c(DAT_008dd914);
  FUN_004eb680(uVar1,0x140,0x28);
  FUN_004f6c40(uVar1,0,0);
  FUN_004f6c20(uVar1,0xff000000,0);
  FUN_004f6b60(uVar1,0,0);
  FUN_004f0268(uVar1,0);
  FUN_004f02c8(uVar1,0);
  FUN_004e96d0(uVar1,0x4000);
  FUN_0046d848((&DAT_008dd904)[DAT_008e171b],1);
  return;
}

 (GhidraScript)  
