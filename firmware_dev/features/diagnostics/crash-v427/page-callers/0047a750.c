
void FUN_0047a750(void)

{
  size_t sVar1;
  undefined4 *puVar2;
  size_t sVar3;
  bool bVar4;
  undefined4 uVar5;
  uint uVar6;
  char *pcVar7;
  int iVar8;
  int *piVar9;
  int iVar10;
  uint uVar11;
  int iVar12;
  int iVar13;
  int iVar14;
  uint uVar15;
  char *local_38;
  
  uVar15 = (uint)DAT_008c59a0;
  iVar8 = uVar15 * 8;
  iVar13 = (&DAT_008c7e98)[uVar15 * 0x25];
  if (iVar13 == 0) {
    return;
  }
  sVar1 = strlen(&DAT_008c509c);
  uVar6 = *(uint *)(&DAT_008c7f24 + uVar15 * 0x94);
  if ((sVar1 == 0) || (*(int *)(&DAT_008c7ebc + uVar15 * 0x94) == 0)) {
    iVar12 = 0;
    if (uVar6 != 0) {
      local_38 = "D:/usr/project/config/img/main_screen/main_1/folder_n.png";
      do {
        iVar13 = (iVar8 * 4 + uVar15 * 5) * 4;
        iVar8 = (iVar12 + 5) * 8 + iVar13;
        iVar14 = *(int *)(&DAT_008c7e9c + iVar8);
        if (((iVar14 == 0) || (*(int *)(&DAT_008c7ebc + iVar13) == 0)) ||
           (uVar11 = *(uint *)(&DAT_008c7ea0 + iVar8), *(uint *)(&DAT_008c7eac + iVar13) <= uVar11))
        goto LAB_0047ab8c;
        if ((uVar11 < *(uint *)(&DAT_008c7ec0 + iVar13)) &&
           (piVar9 = (int *)(*(int *)(&DAT_008c7ebc + iVar13) + uVar11 * 0xc), *piVar9 != 0)) {
          iVar8 = FUN_004e8a80(iVar14,0);
          if (iVar8 != 0) {
            pcVar7 = local_38;
            if ((char)piVar9[2] == '\0') {
              pcVar7 = "D:/usr/project/config/img/main_screen/main_1/song_n.png";
            }
            FUN_0051af44(iVar8,pcVar7);
          }
          iVar8 = FUN_004e8a80(iVar14,1);
          if ((iVar8 != 0) && (iVar13 = FUN_004e8a80(iVar8,0), iVar13 != 0)) {
            thunk_FUN_0097a000(iVar13,*piVar9);
            FUN_004e2364(iVar8,0,0);
          }
          if ((*(char *)((int)piVar9 + 9) == '\0') || (DAT_008c579c == '\0')) {
            pcVar7 = (char *)piVar9[1];
            if (pcVar7 == (char *)0x0) goto LAB_0047af10;
            sVar1 = strlen(pcVar7);
            sVar3 = strlen(&DAT_008c509c);
            bVar4 = false;
            if ((int)sVar1 <= (int)sVar3) {
              iVar8 = strncmp(pcVar7,&DAT_008c509c,sVar1);
              bVar4 = false;
              if (iVar8 == 0) {
                if ((&DAT_008c509c)[sVar1] == '/') {
                  bVar4 = true;
                }
                else {
                  bVar4 = (&DAT_008c509c)[sVar1] == '\0';
                }
              }
            }
          }
          else if ((char *)*piVar9 == (char *)0x0) {
LAB_0047af10:
            bVar4 = false;
          }
          else {
            iVar8 = strcmp((char *)*piVar9,&DAT_008c579c);
            bVar4 = iVar8 == 0;
          }
          FUN_0047a1c0(iVar14,bVar4);
          iVar8 = FUN_004e8a80(iVar14,2);
          uVar15 = (uint)DAT_008c59a0;
          if (iVar8 == 0) goto LAB_0047ac5c;
          if ((DAT_008c7765 == '\0') || ((char)piVar9[2] != '\0')) {
            FUN_004d9960(iVar8,1);
            FUN_00465ba0(iVar14,0xfa);
            FUN_004d9ba8(iVar8,1);
            FUN_004e6e50(iVar8,0xff747474,0);
            uVar15 = (uint)DAT_008c59a0;
            uVar6 = *(uint *)(&DAT_008c7f24 + uVar15 * 0x94);
          }
          else {
            uVar15 = (uint)DAT_008c59a0;
            iVar13 = uVar15 * 0x94;
            iVar10 = *(int *)(&DAT_008c7eb4 + iVar13);
            if (uVar11 - iVar10 < *(uint *)(&DAT_008c7eb0 + iVar13)) {
              FUN_004d9a88(iVar8,1);
              FUN_00465ba0(iVar14,200);
              if (*(char *)(*(int *)(&DAT_008c7eb8 + (uint)DAT_008c59a0 * 0x94) + (uVar11 - iVar10))
                  == '\0') {
                FUN_004d9ba8(iVar8,1);
                uVar5 = 0xff747474;
              }
              else {
                FUN_004d9b88(iVar8,1);
                uVar5 = 0xffff0000;
              }
              FUN_004e6e50(iVar8,uVar5,0);
              uVar15 = (uint)DAT_008c59a0;
              goto LAB_0047ac5c;
            }
            uVar6 = *(uint *)(&DAT_008c7f24 + iVar13);
          }
LAB_0047ab8c:
          if (uVar6 <= iVar12 + 1U) goto LAB_0047ac8c;
        }
        else {
          iVar8 = FUN_004e8a80(iVar14,0);
          if (iVar8 != 0) {
            FUN_0051af44(iVar8,"D:/usr/project/config/img/main_screen/main_1/song_n.png");
          }
          iVar8 = FUN_004e8a80(iVar14,1);
          if ((iVar8 != 0) && (iVar8 = FUN_004e8a80(iVar8,0), iVar8 != 0)) {
            thunk_FUN_0097a000(iVar8,"");
          }
          iVar8 = FUN_004e8a80(iVar14,0);
          if (iVar8 == 0) {
            printf("Error:img is null");
            uVar15 = (uint)DAT_008c59a0;
          }
          else {
            iVar13 = FUN_004e8a80(iVar14,1);
            if (iVar13 == 0) {
              printf("Error:child is null",0);
              uVar15 = (uint)DAT_008c59a0;
            }
            else {
              iVar13 = FUN_004e8a80(iVar13);
              if (iVar13 == 0) {
                printf("Error:label is null");
                uVar15 = (uint)DAT_008c59a0;
              }
              else {
                FUN_004e6f50(iVar8,0xffffffff,0);
                FUN_004e6f60(iVar8,0,0);
                FUN_004e7020(iVar13,0xffffffff,0);
                uVar15 = (uint)DAT_008c59a0;
              }
            }
          }
LAB_0047ac5c:
          uVar6 = *(uint *)(&DAT_008c7f24 + uVar15 * 0x94);
          if (uVar6 <= iVar12 + 1U) goto LAB_0047ac8c;
        }
        iVar12 = iVar12 + 1;
        iVar8 = uVar15 << 3;
      } while( true );
    }
    goto LAB_0047aca4;
  }
  if (uVar6 == 0) {
    uVar6 = 0;
  }
  else {
    uVar11 = 0;
    bVar4 = false;
    iVar13 = uVar15 * 9;
    do {
      while( true ) {
        iVar12 = (uVar11 + 5) * 8;
        iVar8 = iVar12 + (iVar13 * 4 + uVar15) * 4;
        FUN_0047a2f4(*(undefined4 *)(&DAT_008c7e9c + iVar8),*(undefined4 *)(&DAT_008c7ea0 + iVar8));
        uVar15 = (uint)DAT_008c59a0;
        iVar8 = uVar15 * 8;
        if (!bVar4) break;
LAB_0047aa4c:
        uVar6 = *(uint *)(&DAT_008c7f24 + uVar15 * 0x94);
        uVar11 = uVar11 + 1;
        iVar13 = uVar15 * 9;
        if (uVar6 <= uVar11) goto LAB_0047aa70;
      }
      iVar13 = uVar15 * 0x94;
      if (*(uint *)(&DAT_008c7ec0 + iVar13) <= *(uint *)(&DAT_008c7ea0 + iVar12 + iVar13))
      goto LAB_0047aa4c;
      puVar2 = (undefined4 *)
               (*(int *)(&DAT_008c7ebc + iVar13) + *(uint *)(&DAT_008c7ea0 + iVar12 + iVar13) * 0xc)
      ;
      if ((char *)*puVar2 == (char *)0x0) goto LAB_0047aa4c;
      if (*(char *)((int)puVar2 + 9) == '\0') {
        pcVar7 = (char *)puVar2[1];
        if (((pcVar7 != (char *)0x0) && (sVar3 = strlen(pcVar7), (int)sVar3 <= (int)sVar1)) &&
           (iVar13 = strncmp(pcVar7,&DAT_008c509c,sVar3), iVar13 == 0)) {
          if ((&DAT_008c509c)[sVar3] == '/') {
            bVar4 = true;
          }
          else {
            bVar4 = (&DAT_008c509c)[sVar3] == '\0';
          }
        }
        goto LAB_0047aa4c;
      }
      uVar11 = uVar11 + 1;
      iVar13 = strcmp((char *)*puVar2,&DAT_008c579c);
      bVar4 = iVar13 == 0;
      uVar6 = *(uint *)(&DAT_008c7f24 + uVar15 * 0x94);
      iVar13 = uVar15 * 9;
    } while (uVar11 < uVar6);
LAB_0047aa70:
    if (bVar4) goto LAB_0047aa78;
  }
  iVar12 = *(int *)(&DAT_008c7ec0 + uVar15 * 0x94);
  iVar13 = 0;
  if (iVar12 != 0) {
    puVar2 = *(undefined4 **)(&DAT_008c7ebc + uVar15 * 0x94);
    do {
      if ((char *)*puVar2 != (char *)0x0) {
        if (*(char *)((int)puVar2 + 9) == '\0') {
          pcVar7 = (char *)puVar2[1];
          if (((pcVar7 != (char *)0x0) && (sVar3 = strlen(pcVar7), (int)sVar3 <= (int)sVar1)) &&
             ((iVar14 = strncmp(pcVar7,&DAT_008c509c,sVar3), iVar14 == 0 &&
              (((&DAT_008c509c)[sVar3] == '/' || ((&DAT_008c509c)[sVar3] == '\0')))))) {
LAB_0047ae28:
            iVar12 = iVar8 * 0x10 + uVar15 * 0x14;
            iVar13 = iVar13 - (uVar6 >> 1);
            if (iVar13 < 0) {
              iVar13 = 0;
            }
            if (*(uint *)(&DAT_008c7eac + iVar12) < iVar13 + uVar6) {
              iVar13 = *(uint *)(&DAT_008c7eac + iVar12) - uVar6;
            }
            uVar6 = 0;
            if (*(int *)(&DAT_008c7f24 + iVar12) != 0) {
              do {
                iVar8 = (uVar6 + 5) * 8 + iVar8 * 0x10 + uVar15 * 0x14;
                *(int *)(&DAT_008c7ea0 + iVar8) = iVar13;
                FUN_0047a2f4(*(undefined4 *)(&DAT_008c7e9c + iVar8),iVar13);
                uVar15 = (uint)DAT_008c59a0;
                uVar6 = uVar6 + 1;
                iVar8 = uVar15 * 8;
                iVar13 = iVar13 + 1;
              } while (uVar6 < *(uint *)(&DAT_008c7f24 + uVar15 * 0x94));
            }
            FUN_004e2644(*(undefined4 *)(&DAT_008c7ea0 + iVar8 * 0x10 + uVar15 * 0x14),0,0);
            uVar15 = (uint)DAT_008c59a0;
            iVar8 = uVar15 << 3;
            break;
          }
        }
        else {
          iVar14 = strcmp((char *)*puVar2,&DAT_008c579c);
          if (iVar14 == 0) goto LAB_0047ae28;
        }
      }
      iVar13 = iVar13 + 1;
      puVar2 = puVar2 + 3;
    } while (iVar13 != iVar12);
  }
LAB_0047aa78:
  thunk_FUN_00952500((&DAT_008c7e98)[uVar15 * 5 + iVar8 * 4]);
  return;
LAB_0047ac8c:
  iVar13 = (&DAT_008c7e98)[uVar15 * 0x25];
LAB_0047aca4:
  thunk_FUN_00952500(iVar13);
  return;
}

