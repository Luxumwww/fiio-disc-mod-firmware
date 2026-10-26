
void FUN_0045c300(void)

{
  undefined4 uVar1;
  int iVar2;
  int iVar3;
  int iVar4;
  
  if (DAT_008c6f60 == 0) {
    FUN_0045a6d0();
  }
  else {
    uVar1 = FUN_0046f51c();
    iVar2 = FUN_00428900("/usr/project/config/ui/set_menu/app.json",uVar1);
    if (iVar2 != 0) {
      uVar1 = FUN_004e8a80(DAT_008c6f60,0);
      uVar1 = FUN_004e8a80(uVar1,0);
      thunk_FUN_0097a000(uVar1,*(undefined4 *)(*(int *)(iVar2 + 8) + 4));
    }
    FUN_00428dd0(iVar2);
    iVar4 = 1;
    uVar1 = FUN_0046f51c();
    iVar2 = FUN_00428900("/usr/project/config/ui/set_menu/system.json",uVar1);
    if (iVar2 == 0) {
      puts("screen_cfg is null");
      FUN_00428dd0(0);
    }
    else {
      do {
        while (iVar3 = FUN_004e8a80(DAT_008c6f60,iVar4), iVar3 != 0) {
          iVar3 = FUN_004e8a80(iVar3,1);
          if (iVar3 == 0) {
            puts("title_label is null");
          }
          else {
            thunk_FUN_0097a000(iVar3,*(undefined4 *)(*(int *)(iVar2 + 8) + (iVar4 + 0x3fffffff) * 4)
                              );
          }
          iVar4 = iVar4 + 1;
          if (iVar4 == 8) goto LAB_0045c418;
        }
        puts("button is null");
        iVar4 = iVar4 + 1;
      } while (iVar4 != 8);
LAB_0045c418:
      FUN_00428dd0(iVar2);
    }
  }
  FUN_0046c770();
  thunk_FUN_00952500(DAT_008c6f60);
  return;
}

