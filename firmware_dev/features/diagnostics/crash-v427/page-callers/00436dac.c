
void FUN_00436dac(void)

{
  undefined4 *puVar1;
  int iVar2;
  void *__ptr;
  uint uVar3;
  uint uVar4;
  int iVar5;
  uint uVar6;
  
  uVar3 = (uint)DAT_008c59a0;
  if (uVar3 != 0) {
    uVar6 = uVar3 - 1 & 0xff;
    iVar2 = uVar6 * 8;
    DAT_008c59a0 = (byte)(uVar3 - 1);
    if (*(int *)(&DAT_008c7f24 + uVar6 * 0x94) != 0) {
      uVar4 = 0;
      do {
        iVar2 = (uVar4 + 5) * 8 + iVar2 * 0x10 + uVar6 * 0x14;
        uVar4 = uVar4 + 1;
        FUN_0047a2f4(*(undefined4 *)(&DAT_008c7e9c + iVar2),*(undefined4 *)(&DAT_008c7ea0 + iVar2));
        uVar6 = (uint)DAT_008c59a0;
        iVar2 = uVar6 * 8;
      } while (uVar4 < *(uint *)(&DAT_008c7f24 + uVar6 * 0x94));
    }
    thunk_FUN_00952500((&DAT_008c7e98)[uVar6 * 5 + iVar2 * 4]);
  }
  if ((&DAT_008c7e98)[uVar3 * 0x25] != 0) {
    FUN_004e7f34();
    (&DAT_008c7e98)[uVar3 * 0x25] = 0;
  }
  if (*(void **)(&DAT_008c7eb8 + uVar3 * 0x94) != (void *)0x0) {
    free(*(void **)(&DAT_008c7eb8 + uVar3 * 0x94));
    *(undefined4 *)(&DAT_008c7eb8 + uVar3 * 0x94) = 0;
  }
  iVar2 = uVar3 * 0x94;
  __ptr = *(void **)(&DAT_008c7ebc + iVar2);
  if (__ptr != (void *)0x0) {
    iVar5 = 0;
    if (*(int *)(&DAT_008c7ec0 + iVar2) != 0) {
      uVar6 = 0;
      do {
        while( true ) {
          puVar1 = (undefined4 *)((int)__ptr + iVar5);
          if ((void *)*puVar1 != (void *)0x0) {
            free((void *)*puVar1);
            __ptr = *(void **)(&DAT_008c7ebc + iVar2);
            puVar1 = (undefined4 *)((int)__ptr + iVar5);
          }
          if ((void *)puVar1[1] == (void *)0x0) break;
          uVar6 = uVar6 + 1;
          free((void *)puVar1[1]);
          __ptr = *(void **)(&DAT_008c7ebc + iVar2);
          iVar5 = iVar5 + 0xc;
          if (*(uint *)(&DAT_008c7ec0 + iVar2) <= uVar6) goto LAB_00436eb4;
        }
        uVar6 = uVar6 + 1;
        iVar5 = iVar5 + 0xc;
      } while (uVar6 < *(uint *)(&DAT_008c7ec0 + iVar2));
    }
LAB_00436eb4:
    free(__ptr);
    *(undefined4 *)(&DAT_008c7ebc + uVar3 * 0x94) = 0;
  }
  iVar2 = uVar3 * 0x94;
  *(undefined4 *)(&DAT_008c7e9c + iVar2) = 0;
  *(undefined4 *)(&DAT_008c7ea0 + iVar2) = 0;
  *(undefined4 *)(&DAT_008c7eac + iVar2) = 0;
  *(undefined4 *)(&DAT_008c7ec0 + iVar2) = 0;
  *(undefined4 *)(&DAT_008c7f24 + iVar2) = 0;
  return;
}

