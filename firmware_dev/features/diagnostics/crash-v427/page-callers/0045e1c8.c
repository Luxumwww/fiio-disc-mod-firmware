
void FUN_0045e1c8(undefined4 param_1)

{
  undefined4 uVar1;
  int iVar2;
  int iVar3;
  
  DAT_008c6fb4 = param_1;
  uVar1 = FUN_0046f51c();
  iVar2 = FUN_00428900("/usr/project/config/ui/set_menu/system.json",uVar1);
  if (iVar2 != 0) {
    uVar1 = FUN_004e8a80(DAT_008c6fe0,0);
    uVar1 = FUN_004e8a80(uVar1,0);
    thunk_FUN_0097a000(uVar1,*(undefined4 *)(*(int *)(iVar2 + 8) + 8));
  }
  FUN_00428dd0(iVar2);
  uVar1 = FUN_0046f51c();
  iVar2 = FUN_00428900("/usr/project/config/ui/set_menu/wifi.json",uVar1);
  if (iVar2 == 0) {
    puts("screen_cfg_2 is null");
    FUN_00428dd0(0);
    thunk_FUN_00952500(DAT_008c6fe0);
    return;
  }
  iVar3 = FUN_004e8a80(DAT_008c6fe0,1);
  if (iVar3 == 0) {
LAB_0045e364:
    puts("button is null");
    thunk_FUN_00952500(DAT_008c6fe0);
    return;
  }
  iVar3 = FUN_004e8a80(iVar3,1);
  if (iVar3 != 0) {
    thunk_FUN_0097a000(iVar3,**(undefined4 **)(iVar2 + 8));
    iVar3 = FUN_004e8a80(DAT_008c6fe0,3);
    if (iVar3 == 0) goto LAB_0045e364;
    iVar3 = FUN_004e8a80(iVar3,0);
    if (iVar3 != 0) {
      thunk_FUN_0097a000(iVar3,*(undefined4 *)(*(int *)(iVar2 + 8) + 4));
      if ((DAT_008c6fc8 != 0) && (iVar3 = FUN_004e8a80(DAT_008c6fc8,0), iVar3 != 0)) {
        thunk_FUN_0097a000(iVar3,*(undefined4 *)(*(int *)(iVar2 + 8) + 0x30));
      }
      if (((DAT_008c6fd8 != 0) && (iVar3 = FUN_004e8a80(DAT_008c6fd8,0), iVar3 != 0)) &&
         (iVar3 = FUN_004e8a80(iVar3,2), iVar3 != 0)) {
        thunk_FUN_0097a000(iVar3,*(undefined4 *)(*(int *)(iVar2 + 8) + 0xc));
      }
      FUN_00428dd0(iVar2);
      thunk_FUN_00952500(DAT_008c6fe0);
      return;
    }
  }
  puts("title_label is null");
  thunk_FUN_00952500(DAT_008c6fe0);
  return;
}

