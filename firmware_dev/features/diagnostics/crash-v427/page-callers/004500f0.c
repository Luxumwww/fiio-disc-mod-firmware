
void FUN_004500f0(void)

{
  undefined4 uVar1;
  int iVar2;
  undefined4 *puVar3;
  int iVar4;
  undefined4 *puVar5;
  
  thunk_FUN_00953000(0);
  uVar1 = FUN_0046f51c();
  iVar2 = FUN_00428900("/usr/project/config/ui/set_menu/app.json",uVar1);
  if (iVar2 == 0) {
    SYNC(0);
    SYNC(0);
    if (DAT_00846390 != 0) {
      FUN_0048a0ec(DAT_00846390,"process/ui/snowsky_disc/view/src/app_screen.c",0x2d,
                   "refresh_language_label_ui",0x19,0x12d,100,"Failed to parse UI config file\n");
    }
    FUN_00428dd0(0);
  }
  else {
    puVar5 = &DAT_008c6840;
    iVar4 = 0;
    do {
      uVar1 = *puVar5;
      puVar5 = puVar5 + 5;
      puVar3 = (undefined4 *)(*(int *)(iVar2 + 8) + iVar4);
      iVar4 = iVar4 + 4;
      thunk_FUN_0097a000(uVar1,*puVar3);
    } while (iVar4 != 0x14);
    FUN_00428dd0(iVar2);
  }
  thunk_FUN_00952500(DAT_008c689c);
  FUN_0046c790();
  return;
}

