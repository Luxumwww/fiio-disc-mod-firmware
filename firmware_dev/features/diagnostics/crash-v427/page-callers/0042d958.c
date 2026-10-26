
void FUN_0042d958(void)

{
  undefined4 uVar1;
  int iVar2;
  uint uVar3;
  
  if (DAT_008c4f68 == 0) {
    FUN_0042d33c();
    thunk_FUN_00952500(DAT_008c4f68);
  }
  else {
    if (DAT_008c4f64 != 0) {
      uVar1 = FUN_0046f51c();
      iVar2 = FUN_00428900("/usr/project/config/ui/set_menu/classify_playlist.json",uVar1);
      if (iVar2 == 0) {
        SYNC(0);
        SYNC(0);
        if (DAT_00846390 != 0) {
          FUN_0048a0ec(DAT_00846390,"process/ui/snowsky_disc/view/src/class_style.c",0x2e,
                       "refresh_language_text",0x15,0x191,100,"Failed to parse UI config file\n");
        }
        FUN_00428dd0(0);
      }
      else {
        thunk_FUN_0097a000(DAT_008c4f64,*(undefined4 *)(*(int *)(iVar2 + 8) + 0xc));
        FUN_00428dd0(iVar2);
      }
    }
    thunk_FUN_00952500(DAT_008c4f68);
  }
  if (DAT_008c4f5c != '\0') {
    if (DAT_008c4f78 != 0) {
      uVar3 = 0;
      iVar2 = 0;
      do {
        uVar1 = FUN_004e8a80(*(undefined4 *)((int)&DAT_008c4f24 + iVar2),1);
        uVar1 = FUN_004e8a80(uVar1,0);
        uVar3 = uVar3 + 1 & 0xffff;
        FUN_0046685c(uVar1,&DAT_008c569c,
                     *(undefined4 *)(DAT_008c4f7c + *(int *)((int)&DAT_008c4f28 + iVar2) * 0xc + 4))
        ;
        iVar2 = uVar3 << 3;
      } while (uVar3 < DAT_008c4f78);
    }
    return;
  }
  DAT_008c4f5c = 1;
  FUN_0042d688();
  return;
}

