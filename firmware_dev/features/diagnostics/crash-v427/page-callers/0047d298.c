
void FUN_0047d298(void)

{
  int iVar1;
  DIR *__dirp;
  dirent *pdVar2;
  int iVar3;
  char *pcVar4;
  undefined4 *puVar5;
  uint uVar6;
  char acStack_128 [256];
  
  iVar1 = access("/tmp/sdcard/CustomCover",0);
  if (iVar1 == 0) {
    if (DAT_008c7794 == 0) {
      FUN_0047cae4();
    }
    else {
      thunk_FUN_00952500();
    }
    __dirp = opendir("/tmp/sdcard/CustomCover");
    if (__dirp != (DIR *)0x0) {
      iVar1 = 0;
      while (pdVar2 = readdir(__dirp), pdVar2 != (dirent *)0x0) {
        while( true ) {
          pcVar4 = pdVar2->d_name;
          iVar3 = strcmp(pcVar4,".");
          if ((iVar3 == 0) || (iVar3 = strcmp(pcVar4,".."), iVar3 == 0)) break;
          iVar3 = FUN_0047c910(pcVar4);
          iVar1 = iVar1 + (uint)(iVar3 != 0);
          pdVar2 = readdir(__dirp);
          if (pdVar2 == (dirent *)0x0) goto LAB_0047d380;
        }
      }
LAB_0047d380:
      printf("total_found:%d\n",iVar1);
      DAT_00826080 = (ushort)iVar1;
      rewinddir(__dirp);
      DAT_00826084 = malloc((uint)DAT_00826080 << 3);
      iVar1 = 0;
LAB_0047d3c8:
      pdVar2 = readdir(__dirp);
      if (pdVar2 != (dirent *)0x0) {
        while( true ) {
          pcVar4 = pdVar2->d_name;
          iVar3 = strcmp(pcVar4,".");
          if ((iVar3 == 0) || (iVar3 = strcmp(pcVar4,".."), iVar3 == 0)) break;
          iVar3 = FUN_0047c910(pcVar4);
          if (iVar3 == 0) break;
          snprintf(acStack_128,0x100,"%s/%s","/tmp/sdcard/CustomCover",pcVar4);
          puVar5 = (undefined4 *)((int)DAT_00826084 + iVar1 * 8);
          pcVar4 = strdup(pcVar4);
          *puVar5 = pcVar4;
          pcVar4 = strdup(acStack_128);
          puVar5[1] = pcVar4;
          pdVar2 = readdir(__dirp);
          iVar1 = iVar1 + 1;
          if (pdVar2 == (dirent *)0x0) goto LAB_0047d468;
        }
        goto LAB_0047d3c8;
      }
LAB_0047d468:
      closedir(__dirp);
    }
    if (DAT_00826080 != 0) {
      uVar6 = 0;
      do {
        FUN_0047cf44(DAT_008c7790,*(undefined4 *)((int)DAT_00826084 + uVar6 * 8),uVar6);
        uVar6 = uVar6 + 1 & 0xffff;
      } while (uVar6 < DAT_00826080);
      return;
    }
  }
  else {
    system("mkdir /tmp/sdcard/CustomCover");
    if (DAT_008c7794 == 0) {
      FUN_0047cae4();
      return;
    }
    thunk_FUN_00952500();
  }
  return;
}

