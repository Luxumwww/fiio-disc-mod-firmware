
void FUN_0044e6c4(void)

{
  undefined4 uVar1;
  int iVar2;
  uint uVar3;
  
  if (DAT_008c67e8 == 0) {
    FUN_0044e08c();
    thunk_FUN_00952500(DAT_008c67e8);
  }
  else {
    if (DAT_008c67e4 != 0) {
      uVar1 = FUN_0046f51c();
      iVar2 = FUN_00428900("/usr/project/config/ui/set_menu/classify_playlist.json",uVar1);
      if (iVar2 == 0) {
        SYNC(0);
        SYNC(0);
        if (DAT_00846390 != 0) {
          FUN_0048a0ec(DAT_00846390,"process/ui/snowsky_disc/view/src/class_artist.c",0x2f,
                       "refresh_language_text",0x15,0x1a9,100,"Failed to parse UI config file\n");
        }
        FUN_00428dd0(0);
      }
      else {
        thunk_FUN_0097a000(DAT_008c67e4,*(undefined4 *)(*(int *)(iVar2 + 8) + 4));
        FUN_00428dd0(iVar2);
      }
    }
    thunk_FUN_00952500(DAT_008c67e8);
  }
  if (DAT_008c67dc != '\0') {
    if (DAT_008c67f8 != 0) {
      uVar3 = 0;
      iVar2 = 0;
      do {
        uVar1 = FUN_004e8a80(*(undefined4 *)((int)&DAT_008c67a4 + iVar2),1);
        uVar1 = FUN_004e8a80(uVar1,0);
        uVar3 = uVar3 + 1 & 0xffff;
        FUN_0046685c(uVar1,&DAT_008c559c,
                     *(undefined4 *)(DAT_008c67fc + *(int *)((int)&DAT_008c67a8 + iVar2) * 0xc + 4))
        ;
        iVar2 = uVar3 << 3;
      } while (uVar3 < DAT_008c67f8);
    }
    return;
  }
  DAT_008c67dc = 1;
  FUN_0044e3d8();
  return;
}

