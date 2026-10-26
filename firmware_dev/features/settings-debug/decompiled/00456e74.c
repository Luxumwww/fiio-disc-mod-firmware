INFO  SettingsDump.java> 
void FUN_00456e74(undefined4 param_1)

{
  undefined4 uVar1;
  int iVar2;
  undefined *puVar3;
  
  DAT_008dd4b8 = param_1;
  DAT_008dd4c8 = FUN_004e969c(0);
  FUN_004eb680(DAT_008dd4c8,0x168,0x168);
  uVar1 = DAT_008dd4c8;
  FUN_004f6b10(DAT_008dd4c8,0,0);
  FUN_004f6b20(uVar1,0,0);
  FUN_004f6af0(uVar1,0,0);
  FUN_004f6b00(uVar1,0,0);
  FUN_004f6c40(DAT_008dd4c8,0,0);
  FUN_004f6b50(DAT_008dd4c8,0xff000000,0);
  FUN_004ebc14(DAT_008dd4c8,1,0,0);
  FUN_004f6e60(DAT_008dd4c8,0,0);
  FUN_004f6b30(DAT_008dd4c8,5,0);
  FUN_004f0268(DAT_008dd4c8,0);
  FUN_004e39ec(DAT_008dd4c8,FUN_004568f0,1,0);
  FUN_004e39ec(DAT_008dd4c8,FUN_004568f0,2,0);
  FUN_004e39ec(DAT_008dd4c8,FUN_004568f0,8,0);
  FUN_004e2734(DAT_008dd4c8);
  FUN_005030a0(DAT_008dd4c8,1);
  FUN_005030e4(DAT_008dd4c8,0,0,2);
  uVar1 = FUN_00473fa8();
  iVar2 = FUN_0042e640("/usr/project/config/ui/set_menu/system_setting.json",uVar1);
  if (iVar2 == 0) {
    FUN_0046d88c(DAT_008dd4c8,&DAT_0069d8bc,0xb4fb);
  }
  else {
    FUN_0046d88c(DAT_008dd4c8,*(undefined4 *)(*(int *)(iVar2 + 8) + 0x18),0xb4fb);
  }
  FUN_0042eb10(iVar2);
  uVar1 = FUN_00473fa8();
  iVar2 = FUN_0042e640("/usr/project/config/ui/set_menu/others.json",uVar1);
  if (iVar2 == 0) {
    DAT_008dd4bc = FUN_0046d5c0(DAT_008dd4c8,&DAT_0069d8c4,0,FUN_00456a10);
    DAT_008dd4c0 = FUN_0046d5c0(DAT_008dd4c8,&DAT_0069d8cc,1,FUN_00456a10);
    puVar3 = &DAT_0069d8d4;
  }
  else {
    DAT_008dd4bc = FUN_0046d5c0(DAT_008dd4c8,**(undefined4 **)(iVar2 + 8),0,FUN_00456a10);
    DAT_008dd4c0 = FUN_0046d5c0(DAT_008dd4c8,*(undefined4 *)(*(int *)(iVar2 + 8) + 4),1,FUN_00456a10
                               );
    puVar3 = *(undefined **)(*(int *)(iVar2 + 8) + 0x180);
  }
  DAT_008dd4c4 = FUN_0046d5c0(DAT_008dd4c8,puVar3,2,FUN_00456a10);
  FUN_0042eb10(iVar2);
  uVar1 = FUN_004e969c(DAT_008dd4c8);
  FUN_004eb680(uVar1,0x140,0x28);
  FUN_004f6c40(uVar1,0,0);
  FUN_004f6c20(uVar1,0xff000000,0);
  FUN_004f6b60(uVar1,0,0);
  FUN_004f0268(uVar1,0);
  FUN_004f02c8(uVar1,0);
  FUN_004e96d0(uVar1,0x4000);
  FUN_0046d848((&DAT_008dd4bc)[DAT_008e172e],1);
  return;
}

 (GhidraScript)  
