
void FUN_0043ce24(void)

{
  undefined4 uVar1;
  int iVar2;
  uint uVar3;
  
  if (DAT_008c5af8 == 0) {
    FUN_0043c8c8();
    thunk_FUN_00952500(DAT_008c5af8);
  }
  else {
    if (DAT_008c5af4 != 0) {
      uVar1 = FUN_0046f51c();
      iVar2 = FUN_00428900("/usr/project/config/ui/set_menu/classify_playlist.json",uVar1);
      if (iVar2 == 0) {
        SYNC(0);
        SYNC(0);
        if (DAT_00846390 != 0) {
          FUN_0048a0ec(DAT_00846390,"process/ui/snowsky_disc/view/src/class_custom_list.c",0x34,
                       "refresh_language_text",0x15,0x1b2,100,"Failed to parse UI config file\n");
        }
        FUN_00428dd0(0);
      }
      else {
        thunk_FUN_0097a000(DAT_008c5af4,*(undefined4 *)(*(int *)(iVar2 + 8) + 0x14));
        FUN_00428dd0(iVar2);
      }
    }
    thunk_FUN_00952500(DAT_008c5af8);
  }
  if (DAT_008c5aed != '\0') {
    if (DAT_008c5b04 != 0) {
      uVar3 = 0;
      iVar2 = 0;
      do {
        uVar1 = FUN_004e8a80(*(undefined4 *)((int)&DAT_008c5ab4 + iVar2),1);
        uVar1 = FUN_004e8a80(uVar1,0);
        uVar3 = uVar3 + 1 & 0xffff;
        FUN_0046685c(uVar1,&DAT_008c579c,
                     *(undefined4 *)(DAT_008c5b08 + *(int *)((int)&DAT_008c5ab8 + iVar2) * 0xc + 4))
        ;
        iVar2 = uVar3 << 3;
      } while (uVar3 < DAT_008c5b04);
    }
    return;
  }
  DAT_008c5aed = 1;
  FUN_0043cc14();
  return;
}

