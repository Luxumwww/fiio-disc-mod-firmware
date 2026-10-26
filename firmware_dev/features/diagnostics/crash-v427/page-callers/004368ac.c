
void FUN_004368ac(int param_1)

{
  undefined4 uVar1;
  undefined4 uVar2;
  int iVar3;
  undefined4 *puVar4;
  
  iVar3 = param_1 * 0x94;
  uVar1 = FUN_004d992c(0);
  puVar4 = &DAT_008c7e98 + param_1 * 0x25;
  *puVar4 = uVar1;
  *(undefined4 *)(&DAT_008c7ea4 + iVar3) = 0;
  *(undefined4 *)(&DAT_008c7ea8 + iVar3) = 0;
  *(undefined4 *)(&DAT_008c7eb0 + iVar3) = 0;
  *(undefined4 *)(&DAT_008c7eb4 + iVar3) = 0;
  *(undefined4 *)(&DAT_008c7eac + iVar3) = 0;
  *(undefined4 *)(&DAT_008c7ebc + iVar3) = 0;
  *(undefined4 *)(&DAT_008c7ec0 + iVar3) = 0;
  *(undefined4 *)(&DAT_008c7f24 + iVar3) = 0;
  *(undefined4 *)(&DAT_008c7eb8 + iVar3) = 0;
  memset(&DAT_008c7ec4 + iVar3,0,0x60);
  FUN_004e6d80(*puVar4,0xff000000,0);
  FUN_004e6d90(*puVar4,0xff,0);
  FUN_004db9cc(*puVar4,0x168);
  FUN_004dba4c(*puVar4,0x168);
  FUN_004dbe7c(*puVar4,1);
  FUN_004e7090(*puVar4,0,0);
  FUN_004e6e70(*puVar4,0,0);
  uVar1 = *puVar4;
  FUN_004e6d40(uVar1,0,0);
  FUN_004e6d50(uVar1,0,0);
  FUN_004e6d20(uVar1,0,0);
  FUN_004e6d30(uVar1,0,0);
  FUN_004e04b8(*puVar4,0);
  FUN_004d9a88(*puVar4,0x10);
  FUN_004d3c7c(*puVar4,&LAB_004365b0,1,0);
  FUN_004d3c7c(*puVar4,&LAB_004365b0,2,0);
  FUN_004d3c7c(*puVar4,&LAB_004365b0,8,0);
  uVar1 = FUN_0051c364(*puVar4);
  FUN_004e4820(uVar1,&DAT_00826054,0);
  FUN_004e7020(uVar1,0xffff0000,0);
  uVar2 = FUN_0046f51c();
  iVar3 = FUN_00428900("/usr/project/config/ui/set_menu/app.json",uVar2);
  if (iVar3 == 0) {
    thunk_FUN_0097a000(uVar1,&DAT_0068ad58);
  }
  else {
    thunk_FUN_0097a000(uVar1,*(undefined4 *)(*(int *)(iVar3 + 8) + 8));
  }
  FUN_00428dd0(iVar3);
  FUN_004dbe84(uVar1,2,0,0xb);
  FUN_004db9cc(uVar1,0xb4);
  FUN_0051d5d8(uVar1,2);
  FUN_004d9a88(uVar1,0x10);
  iVar3 = param_1 * 0x94;
  puVar4 = &DAT_008c7e98 + param_1 * 0x25;
  FUN_004d9960(uVar1,0x4000);
  uVar1 = FUN_0051c364(*puVar4);
  *(undefined4 *)(&DAT_008c7e9c + iVar3) = uVar1;
  FUN_004e4820(uVar1,&DAT_0082604c,0);
  FUN_004e7020(*(undefined4 *)(&DAT_008c7e9c + iVar3),0xff9f9f9f,0);
  thunk_FUN_0097a000(*(undefined4 *)(&DAT_008c7e9c + iVar3),&DAT_006819b8);
  FUN_004dbe84(*(undefined4 *)(&DAT_008c7e9c + iVar3),2,0,0x2b);
  FUN_004db9cc(*(undefined4 *)(&DAT_008c7e9c + iVar3),0x10e);
  FUN_0051d5d8(*(undefined4 *)(&DAT_008c7e9c + iVar3),3);
  FUN_004d9a88(*(undefined4 *)(&DAT_008c7e9c + iVar3),0x10);
  FUN_004d9960(*(undefined4 *)(&DAT_008c7e9c + iVar3),0x4000);
  uVar1 = FUN_00508c10(*puVar4);
  *(undefined4 *)(&DAT_008c7ea0 + iVar3) = uVar1;
  FUN_004f3160(uVar1,1);
  FUN_004db8f0(*(undefined4 *)(&DAT_008c7ea0 + iVar3),0x168,0x132);
  FUN_004dbe84(*(undefined4 *)(&DAT_008c7ea0 + iVar3),5,0,5);
  FUN_004e6d90(*(undefined4 *)(&DAT_008c7ea0 + iVar3),0,0);
  FUN_004e7090(*(undefined4 *)(&DAT_008c7ea0 + iVar3),0,0);
  FUN_004e6e70(*(undefined4 *)(&DAT_008c7ea0 + iVar3),0,0);
  uVar1 = *(undefined4 *)(&DAT_008c7ea0 + iVar3);
  FUN_004e6d40(uVar1,0,0);
  FUN_004e6d50(uVar1,0,0);
  FUN_004e6d20(uVar1,0,0);
  FUN_004e6d30(uVar1,0,0);
  FUN_004e0518(*(undefined4 *)(&DAT_008c7ea0 + iVar3),0xc);
  FUN_004d9960(*(undefined4 *)(&DAT_008c7ea0 + iVar3),0x4000);
  FUN_004e04b8(*(undefined4 *)(&DAT_008c7ea0 + iVar3),0);
  FUN_004d3c7c(*(undefined4 *)(&DAT_008c7ea0 + iVar3),FUN_00435dd0,0xb,0);
  thunk_FUN_00952500(*puVar4);
  return;
}

