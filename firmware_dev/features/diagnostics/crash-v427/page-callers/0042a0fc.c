
void FUN_0042a0fc(undefined4 param_1)

{
  undefined4 uVar1;
  int iVar2;
  undefined *puVar3;
  
  puts("creat img_anim_brightness screen");
  DAT_008c4e50 = param_1;
  DAT_008c4e60 = FUN_004d992c(0);
  FUN_004db8f0(DAT_008c4e60,0x168,0x168);
  uVar1 = DAT_008c4e60;
  FUN_004e6d40(DAT_008c4e60,0,0);
  FUN_004e6d50(uVar1,0,0);
  FUN_004e6d20(uVar1,0,0);
  FUN_004e6d30(uVar1,0,0);
  FUN_004e6e70(DAT_008c4e60,0,0);
  FUN_004e6d80(DAT_008c4e60,0xff000000,0);
  FUN_004dbe84(DAT_008c4e60,1,0,0);
  FUN_004e7090(DAT_008c4e60,0,0);
  FUN_004e6d60(DAT_008c4e60,5,0);
  FUN_004e04b8(DAT_008c4e60,0);
  FUN_004d3c7c(DAT_008c4e60,FUN_00429ff0,1,0);
  FUN_004d3c7c(DAT_008c4e60,FUN_00429ff0,2,0);
  FUN_004d3c7c(DAT_008c4e60,FUN_00429ff0,8,0);
  thunk_FUN_00952500(DAT_008c4e60);
  FUN_004f3160(DAT_008c4e60,1);
  FUN_004f31a4(DAT_008c4e60,0,0,2);
  uVar1 = FUN_0046f51c();
  iVar2 = FUN_00428900("/usr/project/config/ui/set_menu/system_setting.json",uVar1);
  if (iVar2 == 0) {
    FUN_0045a5bc(DAT_008c4e60,&DAT_00687b90,0xb4fb);
  }
  FUN_00428dd0(iVar2);
  uVar1 = FUN_0046f51c();
  iVar2 = FUN_00428900("/usr/project/config/ui/set_menu/others.json",uVar1);
  if (iVar2 == 0) {
    DAT_008c4e58 = FUN_0045a33c(DAT_008c4e60,&DAT_00687bd0,0,FUN_0042a04c);
    puVar3 = &DAT_00687bd8;
  }
  else {
    DAT_008c4e58 = FUN_0045a33c(DAT_008c4e60,*(undefined4 *)(*(int *)(iVar2 + 8) + 0x100),0,
                                FUN_0042a04c);
    puVar3 = *(undefined **)(*(int *)(iVar2 + 8) + 0x104);
  }
  DAT_008c4e5c = FUN_0045a33c(DAT_008c4e60,puVar3,1,FUN_0042a04c);
  FUN_00428dd0(iVar2);
  uVar1 = FUN_004d992c(DAT_008c4e60);
  FUN_004db8f0(uVar1,0x140,0x28);
  FUN_004e6e70(uVar1,0,0);
  FUN_004e6e50(uVar1,0xff000000,0);
  FUN_004e6d90(uVar1,0,0);
  FUN_004e04b8(uVar1,0);
  FUN_004e0518(uVar1,0);
  FUN_004d9960(uVar1,0x4000);
  if (DAT_008c8a8d == '\0') {
    FUN_0045a578(DAT_008c4e5c,1);
    puts("creat img_anim_brightness screen success");
    return;
  }
  FUN_0045a578(DAT_008c4e58,1);
  puts("creat img_anim_brightness screen success");
  return;
}

