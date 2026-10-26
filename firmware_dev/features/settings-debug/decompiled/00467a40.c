INFO  SettingsDump.java> 
void FUN_00467a40(undefined4 param_1)

{
  undefined4 uVar1;
  int iVar2;
  undefined *puVar3;
  
  DAT_008dda00 = param_1;
  DAT_008dda10 = FUN_004e969c(0);
  FUN_004eb680(DAT_008dda10,0x168,0x168);
  uVar1 = DAT_008dda10;
  FUN_004f6b10(DAT_008dda10,0,0);
  FUN_004f6b20(uVar1,0,0);
  FUN_004f6af0(uVar1,0,0);
  FUN_004f6b00(uVar1,0,0);
  FUN_004f6c40(DAT_008dda10,0,0);
  FUN_004f6b50(DAT_008dda10,0xff000000,0);
  FUN_004ebc14(DAT_008dda10,1,0,0);
  FUN_004f6e60(DAT_008dda10,0,0);
  FUN_004f6b30(DAT_008dda10,5,0);
  FUN_004f0268(DAT_008dda10,0);
  FUN_004e39ec(DAT_008dda10,FUN_00467934,1,0);
  FUN_004e39ec(DAT_008dda10,FUN_00467934,2,0);
  FUN_004e39ec(DAT_008dda10,FUN_00467934,8,0);
  FUN_004e2734(DAT_008dda10);
  FUN_005030a0(DAT_008dda10,1);
  FUN_005030e4(DAT_008dda10,0,0,2);
  uVar1 = FUN_00473fa8();
  iVar2 = FUN_0042e640("/usr/project/config/ui/set_menu/system_setting.json",uVar1);
  if (iVar2 == 0) {
    FUN_0046d88c(DAT_008dda10,&DAT_0069da5c,0xb4fb);
  }
  else {
    FUN_0046d88c(DAT_008dda10,**(undefined4 **)(iVar2 + 8),0xb4fb);
  }
  FUN_0042eb10(iVar2);
  uVar1 = FUN_00473fa8();
  iVar2 = FUN_0042e640("/usr/project/config/ui/set_menu/others.json",uVar1);
  if (iVar2 == 0) {
    DAT_008dda08 = FUN_0046d5c0(DAT_008dda10,"ON",0,FUN_00467990);
    puVar3 = &DAT_0069dbb0;
  }
  else {
    DAT_008dda08 = FUN_0046d5c0(DAT_008dda10,*(undefined4 *)(*(int *)(iVar2 + 8) + 0x100),0,
                                FUN_00467990);
    puVar3 = *(undefined **)(*(int *)(iVar2 + 8) + 0x104);
  }
  DAT_008dda0c = FUN_0046d5c0(DAT_008dda10,puVar3,1,FUN_00467990);
  FUN_0042eb10(iVar2);
  uVar1 = FUN_004e969c(DAT_008dda10);
  FUN_004eb680(uVar1,0x140,0x28);
  FUN_004f6c40(uVar1,0,0);
  FUN_004f6c20(uVar1,0xff000000,0);
  FUN_004f6b60(uVar1,0,0);
  FUN_004f0268(uVar1,0);
  FUN_004f02c8(uVar1,0);
  FUN_004e96d0(uVar1,0x4000);
  if (DAT_0083a5e0 == '\0') {
    FUN_0046d848(DAT_008dda0c,1);
    return;
  }
  FUN_0046d848(DAT_008dda08,1);
  return;
}

 (GhidraScript)  
