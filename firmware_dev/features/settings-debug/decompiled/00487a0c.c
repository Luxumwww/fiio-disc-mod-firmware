INFO  SettingsDump.java> 
void FUN_00487a0c(undefined4 param_1)

{
  undefined4 uVar1;
  int iVar2;
  undefined *puVar3;
  
  puts("creat local_img_anim screen");
  DAT_008df060 = param_1;
  DAT_008df070 = FUN_004e969c(0);
  FUN_004eb680(DAT_008df070,0x168,0x168);
  uVar1 = DAT_008df070;
  FUN_004f6b10(DAT_008df070,0,0);
  FUN_004f6b20(uVar1,0,0);
  FUN_004f6af0(uVar1,0,0);
  FUN_004f6b00(uVar1,0,0);
  FUN_004f6c40(DAT_008df070,0,0);
  FUN_004f6b50(DAT_008df070,0xff000000,0);
  FUN_004ebc14(DAT_008df070,1,0,0);
  FUN_004f6e60(DAT_008df070,0,0);
  FUN_004f6b30(DAT_008df070,5,0);
  FUN_004f0268(DAT_008df070,0);
  FUN_004e39ec(DAT_008df070,FUN_00487900,1,0);
  FUN_004e39ec(DAT_008df070,FUN_00487900,2,0);
  FUN_004e39ec(DAT_008df070,FUN_00487900,8,0);
  FUN_004e2734(DAT_008df070);
  FUN_005030a0(DAT_008df070,1);
  FUN_005030e4(DAT_008df070,0,0,2);
  uVar1 = FUN_00473fa8();
  iVar2 = FUN_0042e640("/usr/project/config/ui/set_menu/system_setting.json",uVar1);
  if (iVar2 == 0) {
    FUN_0046d88c(DAT_008df070,&DAT_0069db14,0xb4fb);
  }
  else {
    FUN_0046d88c(DAT_008df070,*(undefined4 *)(*(int *)(iVar2 + 8) + 0x20),0xb4fb);
  }
  FUN_0042eb10(iVar2);
  uVar1 = FUN_00473fa8();
  iVar2 = FUN_0042e640("/usr/project/config/ui/set_menu/others.json",uVar1);
  if (iVar2 == 0) {
    DAT_008df068 = FUN_0046d5c0(DAT_008df070,&DAT_006a3534,0,FUN_0048795c);
    puVar3 = &DAT_006a353c;
  }
  else {
    DAT_008df068 = FUN_0046d5c0(DAT_008df070,*(undefined4 *)(*(int *)(iVar2 + 8) + 0x144),0,
                                FUN_0048795c);
    puVar3 = *(undefined **)(*(int *)(iVar2 + 8) + 0x148);
  }
  DAT_008df06c = FUN_0046d5c0(DAT_008df070,puVar3,1,FUN_0048795c);
  FUN_0042eb10(iVar2);
  uVar1 = FUN_004e969c(DAT_008df070);
  FUN_004eb680(uVar1,0x140,0x28);
  FUN_004f6c40(uVar1,0,0);
  FUN_004f6c20(uVar1,0xff000000,0);
  FUN_004f6b60(uVar1,0,0);
  FUN_004f0268(uVar1,0);
  FUN_004f02c8(uVar1,0);
  FUN_004e96d0(uVar1,0x4000);
  if (DAT_008e172b == '\0') {
    FUN_0046d848(DAT_008df06c,1);
    puts("creat local_img_anim screen success");
    return;
  }
  FUN_0046d848(DAT_008df068,1);
  puts("creat local_img_anim screen success");
  return;
}

 (GhidraScript)  
