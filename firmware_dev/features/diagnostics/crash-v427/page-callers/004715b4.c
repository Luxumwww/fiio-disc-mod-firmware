
void FUN_004715b4(void)

{
  undefined4 uVar1;
  int iVar2;
  int iVar3;
  
  if (((DAT_008c8a61 != '\0') && (DAT_008c74bf != DAT_008c8a7f)) && (DAT_008c74bd == '\0')) {
    FUN_004428bc(&DAT_0068f0d0,0,0);
  }
  DAT_008c74bf = DAT_008c8a7f;
  uVar1 = FUN_0046f51c();
  iVar2 = FUN_00428900("/usr/project/config/ui/set_menu/system.json",uVar1);
  if (iVar2 != 0) {
    iVar3 = FUN_004e8a80(DAT_008c74d8,0);
    if (iVar3 != 0) {
      uVar1 = FUN_004e8a80(iVar3,0);
      thunk_FUN_0097a000(uVar1,*(undefined4 *)(*(int *)(iVar2 + 8) + 0xc));
    }
    iVar3 = FUN_004e8a80(DAT_008c74d8,1);
    if (iVar3 == 0) goto LAB_00471758;
    iVar3 = FUN_004e8a80(iVar3,1);
    if (iVar3 == 0) goto LAB_00471738;
    thunk_FUN_0097a000(iVar3,*(undefined4 *)(*(int *)(iVar2 + 8) + 0xc));
  }
  FUN_00428dd0(iVar2);
  uVar1 = FUN_0046f51c();
  iVar2 = FUN_00428900("/usr/project/config/ui/set_menu/bt.json",uVar1);
  if (iVar2 == 0) {
    puts("screen_cfg_2 is null");
    FUN_00428dd0(0);
    thunk_FUN_00952500(DAT_008c74d8);
    return;
  }
  iVar3 = FUN_004e8a80(DAT_008c74d8,2);
  if (iVar3 != 0) {
    iVar3 = FUN_004e8a80(iVar3,0);
    if (iVar3 != 0) {
      thunk_FUN_0097a000(iVar3,**(undefined4 **)(iVar2 + 8));
      iVar3 = FUN_004e8a80(DAT_008c74d8,4);
      if (iVar3 == 0) goto LAB_00471758;
      iVar3 = FUN_004e8a80(iVar3,0);
      if (iVar3 != 0) {
        uVar1 = FUN_004e8a80(iVar3,0);
        thunk_FUN_0097a000(uVar1,*(undefined4 *)(*(int *)(iVar2 + 8) + 4));
        FUN_00428dd0(iVar2);
        thunk_FUN_00952500(DAT_008c74d8);
        return;
      }
    }
LAB_00471738:
    puts("title_label is null");
    thunk_FUN_00952500(DAT_008c74d8);
    return;
  }
LAB_00471758:
  puts("button is null");
  thunk_FUN_00952500(DAT_008c74d8);
  return;
}

