
void FUN_00441838(void)

{
  undefined4 uVar1;
  int iVar2;
  uint uVar3;
  
  if (DAT_008c6518 == 0) {
    FUN_004411fc();
    thunk_FUN_00952500(DAT_008c6518);
  }
  else {
    if (DAT_008c6514 != 0) {
      uVar1 = FUN_0046f51c();
      iVar2 = FUN_00428900("/usr/project/config/ui/set_menu/classify_playlist.json",uVar1);
      if (iVar2 == 0) {
        SYNC(0);
        SYNC(0);
        if (DAT_00846390 != 0) {
          FUN_0048a0ec(DAT_00846390,"process/ui/snowsky_disc/view/src/class_all_songs.c",0x32,
                       "refresh_language_text",0x15,0x1b6,100,"Failed to parse UI config file\n");
        }
        FUN_00428dd0(0);
      }
      else {
        thunk_FUN_0097a000(DAT_008c6514,**(undefined4 **)(iVar2 + 8));
        FUN_00428dd0(iVar2);
      }
    }
    thunk_FUN_00952500(DAT_008c6518);
  }
  if (DAT_008c650d != '\0') {
    if (DAT_008c6524 != 0) {
      uVar3 = 0;
      iVar2 = 0;
      do {
        uVar1 = FUN_004e8a80(*(undefined4 *)((int)&DAT_008c64d4 + iVar2),1);
        uVar1 = FUN_004e8a80(uVar1,0);
        uVar3 = uVar3 + 1 & 0xffff;
        FUN_00466844(uVar1,DAT_008c58a0,
                     *(undefined4 *)(DAT_008c6528 + *(int *)((int)&DAT_008c64d8 + iVar2) * 0xc));
        iVar2 = uVar3 << 3;
      } while (uVar3 < DAT_008c6524);
    }
    return;
  }
  DAT_008c650d = 1;
  FUN_00441564();
  return;
}

