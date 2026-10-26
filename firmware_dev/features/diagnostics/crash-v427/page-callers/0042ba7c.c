
void FUN_0042ba7c(undefined4 param_1)

{
  undefined4 uVar1;
  int iVar2;
  undefined *puVar3;
  
  puts("creat local_img_anim screen");
  DAT_008c4ee0 = param_1;
  DAT_008c4ef0 = FUN_004d992c(0);
  FUN_004db8f0(DAT_008c4ef0,0x168,0x168);
  uVar1 = DAT_008c4ef0;
  FUN_004e6d40(DAT_008c4ef0,0,0);
  FUN_004e6d50(uVar1,0,0);
  FUN_004e6d20(uVar1,0,0);
  FUN_004e6d30(uVar1,0,0);
  FUN_004e6e70(DAT_008c4ef0,0,0);
  FUN_004e6d80(DAT_008c4ef0,0xff000000,0);
  FUN_004dbe84(DAT_008c4ef0,1,0,0);
  FUN_004e7090(DAT_008c4ef0,0,0);
  FUN_004e6d60(DAT_008c4ef0,5,0);
  FUN_004e04b8(DAT_008c4ef0,0);
  FUN_004d3c7c(DAT_008c4ef0,FUN_0042b970,1,0);
  FUN_004d3c7c(DAT_008c4ef0,FUN_0042b970,2,0);
  FUN_004d3c7c(DAT_008c4ef0,FUN_0042b970,8,0);
  thunk_FUN_00952500(DAT_008c4ef0);
  FUN_004f3160(DAT_008c4ef0,1);
  FUN_004f31a4(DAT_008c4ef0,0,0,2);
  uVar1 = FUN_0046f51c();
  iVar2 = FUN_00428900("/usr/project/config/ui/set_menu/system_setting.json",uVar1);
  if (iVar2 == 0) {
    FUN_0045a5bc(DAT_008c4ef0,&DAT_0068a0b0,0xb4fb);
  }
  else {
    FUN_0045a5bc(DAT_008c4ef0,*(undefined4 *)(*(int *)(iVar2 + 8) + 0x20),0xb4fb);
  }
  FUN_00428dd0(iVar2);
  uVar1 = FUN_0046f51c();
  iVar2 = FUN_00428900("/usr/project/config/ui/set_menu/others.json",uVar1);
  if (iVar2 == 0) {
    DAT_008c4ee8 = FUN_0045a33c(DAT_008c4ef0,&DAT_0068a0c0,0,FUN_0042b9cc);
    puVar3 = &DAT_0068a0c8;
  }
  else {
    DAT_008c4ee8 = FUN_0045a33c(DAT_008c4ef0,*(undefined4 *)(*(int *)(iVar2 + 8) + 0x144),0,
                                FUN_0042b9cc);
    puVar3 = *(undefined **)(*(int *)(iVar2 + 8) + 0x148);
  }
  DAT_008c4eec = FUN_0045a33c(DAT_008c4ef0,puVar3,1,FUN_0042b9cc);
  FUN_00428dd0(iVar2);
  uVar1 = FUN_004d992c(DAT_008c4ef0);
  FUN_004db8f0(uVar1,0x140,0x28);
  FUN_004e6e70(uVar1,0,0);
  FUN_004e6e50(uVar1,0xff000000,0);
  FUN_004e6d90(uVar1,0,0);
  FUN_004e04b8(uVar1,0);
  FUN_004e0518(uVar1,0);
  FUN_004d9960(uVar1,0x4000);
  if (DAT_008c8a8b == '\0') {
    FUN_0045a578(DAT_008c4eec,1);
    puts("creat local_img_anim screen success");
    return;
  }
  FUN_0045a578(DAT_008c4ee8,1);
  puts("creat local_img_anim screen success");
  return;
}

