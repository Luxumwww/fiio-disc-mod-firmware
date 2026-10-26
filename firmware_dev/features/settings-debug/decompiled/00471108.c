INFO  SettingsDump.java> 
void FUN_00471108(undefined4 param_1)

{
  undefined4 uVar1;
  undefined4 uVar2;
  int iVar3;
  
  DAT_008de034 = param_1;
  DAT_008de04c = FUN_004e969c(0);
  FUN_004eb680(DAT_008de04c,0x168,0x168);
  uVar1 = DAT_008de04c;
  FUN_004f6b10(DAT_008de04c,0,0);
  FUN_004f6b20(uVar1,0,0);
  FUN_004f6af0(uVar1,0,0);
  FUN_004f6b00(uVar1,0,0);
  FUN_004f6c40(DAT_008de04c,0,0);
  FUN_004f6b50(DAT_008de04c,0xff000000,0);
  FUN_004ebc14(DAT_008de04c,1,0,0);
  FUN_004f6e60(DAT_008de04c,0,0);
  FUN_004f6b30(DAT_008de04c,5,0);
  FUN_004f0268(DAT_008de04c,0);
  FUN_004e39ec(DAT_008de04c,&LAB_00470be0,1,0);
  FUN_004e39ec(DAT_008de04c,&LAB_00470be0,2,0);
  FUN_004e39ec(DAT_008de04c,&LAB_00470be0,8,0);
  FUN_004e2734(DAT_008de04c);
  uVar1 = FUN_0052df64(DAT_008de04c);
  uVar2 = FUN_00473fa8();
  iVar3 = FUN_0042e640("/usr/project/config/ui/set_menu/system_setting.json",uVar2);
  if (iVar3 == 0) {
    FUN_0052f2bc(uVar1,&DAT_0069fe60);
  }
  else {
    FUN_0052f2bc(uVar1,*(undefined4 *)(*(int *)(iVar3 + 8) + 4));
  }
  FUN_0042eb10(iVar3);
  FUN_004eb75c(uVar1,0xb4);
  FUN_0052f4f0(uVar1,2);
  FUN_004ebc14(uVar1,2,0,0x10);
  FUN_004f6df0(uVar1,0xff00b4fb,0);
  FUN_004f45d0(uVar1,&DAT_0083ac5c,0);
  DAT_008de048 = FUN_0053342c(DAT_008de04c);
  FUN_004eb680(DAT_008de048,0xfa,0x2f);
  FUN_0052a754(DAT_008de048,1,0x28);
  FUN_0052a4ec(DAT_008de048,DAT_008e1704,0);
  FUN_0052a7c8(DAT_008de048,0);
  FUN_004f6e80(DAT_008de048,0,0x30000);
  FUN_004ebc14(DAT_008de048,9,0,0xfffffff6);
  FUN_004e39ec(DAT_008de048,FUN_00471010,0x1c,0);
  DAT_008de040 = FUN_00470ea8(DAT_008de04c,0x3a,
                              "D:/usr/project/config/img/apps/settings/decrease.png");
  DAT_008de044 = FUN_00470ea8(DAT_008de04c,0x104,"D:/usr/project/config/img/apps/settings/add.png");
  return;
}

 (GhidraScript)  
