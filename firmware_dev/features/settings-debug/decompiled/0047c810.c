INFO  SettingsDump.java> 
void FUN_0047c810(undefined4 param_1)

{
  undefined4 uVar1;
  int iVar2;
  undefined *puVar3;
  
  if (DAT_008de4c0 != 0) {
    FUN_004e2734(DAT_008de4c0);
    return;
  }
  DAT_008de4a0 = param_1;
  DAT_008de4c0 = FUN_004e969c(0);
  FUN_004eb680(DAT_008de4c0,0x168,0x168);
  uVar1 = DAT_008de4c0;
  FUN_004f6b10(DAT_008de4c0,0,0);
  FUN_004f6b20(uVar1,0,0);
  FUN_004f6af0(uVar1,0,0);
  FUN_004f6b00(uVar1,0,0);
  FUN_004f6c40(DAT_008de4c0,0,0);
  FUN_004f6b50(DAT_008de4c0,0xff000000,0);
  FUN_004ebc14(DAT_008de4c0,1,0,0);
  FUN_004f6e60(DAT_008de4c0,0,0);
  FUN_004f6b30(DAT_008de4c0,5,0);
  FUN_004f0268(DAT_008de4c0,0);
  FUN_004e39ec(DAT_008de4c0,&LAB_0047c6a0,1,0);
  FUN_004e39ec(DAT_008de4c0,&LAB_0047c6a0,2,0);
  FUN_004e39ec(DAT_008de4c0,&LAB_0047c6a0,8,0);
  FUN_004e2734(DAT_008de4c0);
  FUN_005030a0(DAT_008de4c0,1);
  FUN_005030e4(DAT_008de4c0,0,0,2);
  uVar1 = FUN_00473fa8();
  iVar2 = FUN_0042e640("/usr/project/config/ui/set_menu/system_setting.json",uVar1);
  if (iVar2 == 0) {
    FUN_0046d88c(DAT_008de4c0,&DAT_0069d8bc,0xb4fb);
  }
  else {
    FUN_0046d88c(DAT_008de4c0,*(undefined4 *)(*(int *)(iVar2 + 8) + 0x1c),0xb4fb);
  }
  FUN_0042eb10(iVar2);
  uVar1 = FUN_00473fa8();
  iVar2 = FUN_0042e640("/usr/project/config/ui/set_menu/others.json",uVar1);
  if (iVar2 == 0) {
    DAT_008de4a8 = FUN_0046d5c0(DAT_008de4c0,&DAT_0069d8c4,0,FUN_0047c6fc);
    DAT_008de4ac = FUN_0046d5c0(DAT_008de4c0,&DAT_0069d8cc,1,FUN_0047c6fc);
    DAT_008de4b0 = FUN_0046d5c0(DAT_008de4c0,&DAT_0069d8d4,2,FUN_0047c6fc);
    DAT_008de4b4 = FUN_0046d5c0(DAT_008de4c0,&DAT_006a0020,3,FUN_0047c6fc);
    DAT_008de4b8 = FUN_0046d5c0(DAT_008de4c0,&DAT_006a0028,4,FUN_0047c6fc);
    puVar3 = &DAT_006a0030;
  }
  else {
    DAT_008de4a8 = FUN_0046d5c0(DAT_008de4c0,*(undefined4 *)(*(int *)(iVar2 + 8) + 0x3c),0,
                                FUN_0047c6fc);
    DAT_008de4ac = FUN_0046d5c0(DAT_008de4c0,*(undefined4 *)(*(int *)(iVar2 + 8) + 0x40),1,
                                FUN_0047c6fc);
    DAT_008de4b0 = FUN_0046d5c0(DAT_008de4c0,*(undefined4 *)(*(int *)(iVar2 + 8) + 0x44),2,
                                FUN_0047c6fc);
    DAT_008de4b4 = FUN_0046d5c0(DAT_008de4c0,*(undefined4 *)(*(int *)(iVar2 + 8) + 0x48),3,
                                FUN_0047c6fc);
    DAT_008de4b8 = FUN_0046d5c0(DAT_008de4c0,*(undefined4 *)(*(int *)(iVar2 + 8) + 0x4c),4,
                                FUN_0047c6fc);
    puVar3 = *(undefined **)(*(int *)(iVar2 + 8) + 0x164);
  }
  DAT_008de4bc = FUN_0046d5c0(DAT_008de4c0,puVar3,5,FUN_0047c6fc);
  FUN_0042eb10(iVar2);
  uVar1 = FUN_004e969c(DAT_008de4c0);
  FUN_004eb680(uVar1,0x140,0x28);
  FUN_004f6c40(uVar1,0,0);
  FUN_004f6c20(uVar1,0xff000000,0);
  FUN_004f6b60(uVar1,0,0);
  FUN_004f0268(uVar1,0);
  FUN_004f02c8(uVar1,0);
  FUN_004e96d0(uVar1,0x4000);
  FUN_0046d848((&DAT_008de4a8)[DAT_008de4a4],1);
  return;
}

 (GhidraScript)  
