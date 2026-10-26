
void FUN_00439228(void)

{
  undefined4 uVar1;
  int iVar2;
  uint uVar3;
  
  if (DAT_008c5a28 == 0) {
    FUN_00438c0c();
    thunk_FUN_00952500(DAT_008c5a28);
  }
  else {
    if (DAT_008c5a24 != 0) {
      uVar1 = FUN_0046f51c();
      iVar2 = FUN_00428900("/usr/project/config/ui/set_menu/classify_playlist.json",uVar1);
      if (iVar2 == 0) {
        SYNC(0);
        SYNC(0);
        if (DAT_00846390 != 0) {
          FUN_0048a0ec(DAT_00846390,"process/ui/snowsky_disc/view/src/class_album.c",0x2e,
                       "refresh_language_text",0x15,0x18d,100,"Failed to parse UI config file\n");
        }
        FUN_00428dd0(0);
      }
      else {
        thunk_FUN_0097a000(DAT_008c5a24,*(undefined4 *)(*(int *)(iVar2 + 8) + 8));
        FUN_00428dd0(iVar2);
      }
    }
    thunk_FUN_00952500(DAT_008c5a28);
  }
  if (DAT_008c5a1c != '\0') {
    if (DAT_008c5a38 != 0) {
      uVar3 = 0;
      iVar2 = 0;
      do {
        uVar1 = FUN_004e8a80(*(undefined4 *)((int)&DAT_008c59e4 + iVar2),1);
        uVar1 = FUN_004e8a80(uVar1,0);
        uVar3 = uVar3 + 1 & 0xffff;
        FUN_0046685c(uVar1,&DAT_008c549c,
                     *(undefined4 *)(DAT_008c5a3c + *(int *)((int)&DAT_008c59e8 + iVar2) * 0xc + 4))
        ;
        iVar2 = uVar3 << 3;
      } while (uVar3 < DAT_008c5a38);
    }
    FUN_00465790(FUN_00439228);
    return;
  }
  DAT_008c5a1c = 1;
  FUN_00438f58();
  FUN_00465790(FUN_00439228);
  return;
}

