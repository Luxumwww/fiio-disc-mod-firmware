INFO  SettingsDump.java> 
void FUN_00457e6c(void)

{
  undefined4 uVar1;
  int iVar2;
  int iVar3;
  int iVar4;
  
  if (DAT_008dd4f0 == 0) {
    FUN_0045777c();
    FUN_004e2734(DAT_008dd4f0);
    return;
  }
  uVar1 = FUN_00473fa8();
  iVar2 = FUN_0042e640("/usr/project/config/ui/set_menu/system.json",uVar1);
  if (iVar2 != 0) {
    uVar1 = FUN_004f8850(DAT_008dd4f0,0);
    uVar1 = FUN_004f8850(uVar1,0);
    FUN_0052f2bc(uVar1,*(undefined4 *)(*(int *)(iVar2 + 8) + 4));
  }
  FUN_0042eb10(iVar2);
  iVar4 = 1;
  uVar1 = FUN_00473fa8();
  iVar2 = FUN_0042e640("/usr/project/config/ui/set_menu/system_setting.json",uVar1);
  if (iVar2 == 0) {
    puts("screen_cfg is null");
    FUN_0042eb10(0);
    FUN_004e2734(DAT_008dd4f0);
    return;
  }
  do {
    while (iVar3 = FUN_004f8850(DAT_008dd4f0,iVar4), iVar3 != 0) {
      iVar3 = FUN_004f8850(iVar3,0);
      if (iVar3 == 0) {
        puts("title_label is null");
      }
      else {
        FUN_0052f2bc(iVar3,*(undefined4 *)(*(int *)(iVar2 + 8) + (iVar4 + 0x3fffffff) * 4));
      }
      iVar4 = iVar4 + 1;
      if (iVar4 == 0x11) goto LAB_00457f8c;
    }
    puts("button is null");
    iVar4 = iVar4 + 1;
  } while (iVar4 != 0x11);
LAB_00457f8c:
  FUN_0042eb10(iVar2);
  FUN_004e2734(DAT_008dd4f0);
  return;
}

 (GhidraScript)  
