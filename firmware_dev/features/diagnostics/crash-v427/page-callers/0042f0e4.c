
void FUN_0042f0e4(void)

{
  char *__s1;
  int iVar1;
  undefined4 uVar2;
  uint uVar3;
  
  if (DAT_008c4fe0 != 0) {
    __s1 = (char *)FUN_0051c3a8(DAT_008c4fdc);
    iVar1 = strcmp(__s1,&DAT_008c569c);
    DAT_008c4fd5 = iVar1 == 0;
    if ((iVar1 == 0) && (DAT_008c4ff0 != 0)) {
      uVar3 = 0;
      iVar1 = 0;
      do {
        uVar2 = FUN_004e8a80(*(undefined4 *)((int)&DAT_008c4fa4 + iVar1),1);
        uVar2 = FUN_004e8a80(uVar2,0);
        uVar3 = uVar3 + 1 & 0xffff;
        FUN_0046685c(uVar2,&DAT_008c549c,
                     *(undefined4 *)(DAT_008c4ff4 + *(int *)((int)&DAT_008c4fa8 + iVar1) * 0xc + 4))
        ;
        iVar1 = uVar3 << 3;
      } while (uVar3 < DAT_008c4ff0);
    }
    thunk_FUN_00952500(DAT_008c4fe0);
    return;
  }
  FUN_00465744();
  return;
}

