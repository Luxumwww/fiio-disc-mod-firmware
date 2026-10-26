
void FUN_004684a8(void)

{
  int iVar1;
  undefined4 uVar2;
  
  thunk_FUN_00953000(1);
  if (DAT_008c729c == 0) {
    DAT_008c7284 = (undefined4 *)FUN_004430f8(0x46);
    DAT_008c7280 = (undefined4 *)FUN_004430f8(0x28);
    DAT_008c729c = FUN_004d992c(0);
    FUN_004db9cc(DAT_008c729c,0x168);
    FUN_004dba4c(DAT_008c729c,0x168);
    FUN_004dbe7c(DAT_008c729c,1);
    FUN_004d9a88(DAT_008c729c,0x10);
    FUN_004d9960(DAT_008c729c,8);
    FUN_004e6d80(DAT_008c729c,0xff000000,0);
    FUN_004e6d90(DAT_008c729c,0xff,0);
    FUN_004e6e50(DAT_008c729c,0xff000000,0);
    FUN_004e6e60(DAT_008c729c,0,0);
    FUN_004e7090(DAT_008c729c,0x14,0);
    FUN_004e6e70(DAT_008c729c,0,0);
    iVar1 = DAT_008c729c;
    FUN_004e6d40(DAT_008c729c,0,0);
    FUN_004e6d50(iVar1,0,0);
    FUN_004e6d20(iVar1,0,0);
    FUN_004e6d30(iVar1,0,0);
    DAT_008c7298 = FUN_004d992c(DAT_008c729c);
    FUN_004db9cc(DAT_008c7298,0x168);
    FUN_004dba4c(DAT_008c7298,0x168);
    FUN_004dbe84(DAT_008c7298,9,0,0);
    FUN_004e6d80(DAT_008c7298,0xff000000,0);
    FUN_004e6d90(DAT_008c7298,0xff,0);
    FUN_004d9a88(DAT_008c7298,0x10);
    FUN_004e7090(DAT_008c7298,0xf,0);
    FUN_004e6e70(DAT_008c7298,0,0);
    uVar2 = DAT_008c7298;
    FUN_004e6d40(DAT_008c7298,0,0);
    FUN_004e6d50(uVar2,0,0);
    FUN_004e6d20(uVar2,0,0);
    FUN_004e6d30(uVar2,0,0);
    FUN_004e6e50(DAT_008c7298,0xff616161,0);
    FUN_004e6e60(DAT_008c7298,0,0);
    FUN_004d3c7c(DAT_008c7298,&LAB_00468470,1,0);
    FUN_004d3c7c(DAT_008c7298,&LAB_00468470,2,0);
    FUN_004d3c7c(DAT_008c7298,&LAB_00468470,8,0);
    DAT_008c7294 = FUN_0051af10(DAT_008c7298);
    FUN_004dbe84(DAT_008c7294,9,0,0);
    FUN_004d9a88(DAT_008c7294,0x10);
    FUN_0051c0ac(DAT_008c7294,0x80);
    FUN_0051af44(DAT_008c7294,"D:/usr/project/config/img/main_screen/main_1/media_local_play_bg.png"
                );
    DAT_008c7290 = FUN_0051c364(DAT_008c7298);
    uVar2 = FUN_0046f51c();
    iVar1 = FUN_00428900("/usr/project/config/ui/set_menu/bt.json",uVar2);
    if (iVar1 == 0) {
      thunk_FUN_0097a000(DAT_008c7290,&DAT_0068d8b8);
    }
    else {
      thunk_FUN_0097a000(DAT_008c7290,*(undefined4 *)(*(int *)(iVar1 + 8) + 0x1c));
    }
    FUN_00428dd0(iVar1);
    FUN_004e7020(DAT_008c7290,0xff000000,0);
    FUN_004e4820(DAT_008c7290,*DAT_008c7280,0);
    FUN_004e7020(DAT_008c7290,0xff035572,0);
    FUN_004dbe84(DAT_008c7290,2,0,0x23);
    DAT_008c728c = FUN_0051c364(DAT_008c7298);
    thunk_FUN_0097a000(DAT_008c728c,&PTR_LAB_0068e9a8);
    FUN_004e7020(DAT_008c728c,0xff000000,0);
    FUN_004e4820(DAT_008c728c,*DAT_008c7284,0);
    FUN_004e7020(DAT_008c728c,0xff035572,0);
    FUN_004dbe84(DAT_008c728c,2,0,0x55);
    FUN_004d9960(DAT_008c728c,1);
  }
  else {
    if (DAT_008c7288 != '\0') {
      thunk_FUN_00952500();
      return;
    }
    iVar1 = FUN_004d9d00(DAT_008c7290);
    if (iVar1 == 0) {
      thunk_FUN_00952500(DAT_008c729c);
      return;
    }
    uVar2 = FUN_0046f51c();
    iVar1 = FUN_00428900("/usr/project/config/ui/set_menu/bt.json",uVar2);
    if (iVar1 == 0) {
      thunk_FUN_0097a000(DAT_008c7290,&DAT_0068d8b8);
    }
    else {
      thunk_FUN_0097a000(DAT_008c7290,*(undefined4 *)(*(int *)(iVar1 + 8) + 0x1c));
    }
    FUN_00428dd0(iVar1);
  }
  thunk_FUN_00952500(DAT_008c729c);
  return;
}

