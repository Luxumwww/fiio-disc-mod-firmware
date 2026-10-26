
void FUN_00451ce4(void)

{
  undefined4 uVar1;
  uint uVar2;
  int iVar3;
  
  if (DAT_008c68fc == 0) {
    FUN_00465744();
    return;
  }
  if (DAT_008c6908 != 0) {
    uVar2 = 0;
    iVar3 = 0;
    do {
      uVar1 = FUN_004e8a80(*(undefined4 *)((int)&DAT_008c68b8 + iVar3),1);
      uVar1 = FUN_004e8a80(uVar1,0);
      uVar2 = uVar2 + 1 & 0xffff;
      FUN_00466844(uVar1,DAT_008c58a0,
                   *(undefined4 *)(DAT_008c690c + *(int *)((int)&DAT_008c68bc + iVar3) * 0xc));
      iVar3 = uVar2 << 3;
    } while (uVar2 < DAT_008c6908);
  }
  thunk_FUN_00952500(DAT_008c68fc);
  return;
}

