
void FUN_00477b68(undefined4 param_1)

{
  undefined4 uVar1;
  undefined4 uVar2;
  undefined4 uVar3;
  int iVar4;
  uint uVar5;
  uint uVar6;
  char acStack_30 [20];
  
  DAT_008c7604 = param_1;
  DAT_008c7608 = FUN_004d992c(0);
  FUN_004db8f0(DAT_008c7608,0x168,0x168);
  FUN_004e7090(DAT_008c7608,0xb4,0);
  uVar1 = DAT_008c7608;
  FUN_004e6d40(DAT_008c7608,0,0);
  FUN_004e6d50(uVar1,0,0);
  FUN_004e6d20(uVar1,0,0);
  FUN_004e6d30(uVar1,0,0);
  FUN_004e6e70(DAT_008c7608,0,0);
  FUN_004e6d80(DAT_008c7608,0xff000000,0);
  FUN_004dbe84(DAT_008c7608,1,0,0);
  FUN_004e7090(DAT_008c7608,0,0);
  FUN_004e6d60(DAT_008c7608,5,0);
  uVar1 = FUN_004d992c(DAT_008c7608);
  FUN_004db8f0(uVar1,0x20000064);
  FUN_004e6d40(uVar1,10,0);
  FUN_004e6d50(uVar1,10,0);
  FUN_004e6d20(uVar1,10,0);
  FUN_004e6d30(uVar1,10,0);
  FUN_004e6d80(uVar1,0xff000000,0);
  FUN_004e6e70(uVar1,0,0);
  FUN_004d9a88(uVar1,0x10);
  FUN_004d9a88(uVar1,0x8000);
  FUN_004dbe84(uVar1,9,0,0);
  FUN_004e04b8(uVar1,0);
  uVar2 = FUN_0051c364(uVar1);
  uVar3 = FUN_0046f51c();
  iVar4 = FUN_00428900("/usr/project/config/ui/set_menu/system_setting.json",uVar3);
  if (iVar4 == 0) {
    thunk_FUN_0097a000(uVar2,&DAT_0068f390);
  }
  else {
    thunk_FUN_0097a000(uVar2,*(undefined4 *)(*(int *)(iVar4 + 8) + 0x38));
  }
  FUN_00428dd0(iVar4);
  FUN_004db9cc(uVar2,0xb4);
  FUN_0051d5d8(uVar2,2);
  FUN_004e4820(uVar2,&_fbss,0);
  FUN_004e7020(uVar2,0xffffffff,0);
  FUN_004dbe84(uVar2,2,0,10);
  DAT_008c7600 = FUN_00523400(uVar1);
  FUN_004db8f0(DAT_008c7600,0x135,0x20000064);
  FUN_004e6d80(DAT_008c7600,0xff000000,0);
  FUN_004e6e70(DAT_008c7600,0,0);
  iVar4 = DAT_008c7600;
  FUN_004e6d40(DAT_008c7600,10,0);
  FUN_004e6d50(iVar4,10,0);
  FUN_004e6d20(iVar4,10,0);
  FUN_004e6d30(iVar4,10,0);
  FUN_004dbe84(DAT_008c7600,2,5,0x35);
  FUN_004d9960(DAT_008c7600,2);
  FUN_004d9a88(DAT_008c7600,0x8000);
  FUN_004d9960(DAT_008c7600,0x10);
  FUN_004e04b8(DAT_008c7600,0);
  FUN_004d9960(DAT_008c7600,0x4000);
  FUN_004d3c7c(uVar1,&LAB_00477aa0,1,0);
  FUN_004d3c7c(uVar1,&LAB_00477aa0,2,0);
  FUN_004d3c7c(uVar1,&LAB_00477aa0,8,0);
  FUN_004d3c7c(uVar1,&LAB_00477aa0,0xc,0);
  uVar5 = 0;
  FUN_005235b4(DAT_008c7600,2);
  FUN_00523434(DAT_008c7600,7);
  FUN_00523b3c(DAT_008c7600,0,0x5a);
  FUN_00523b3c(DAT_008c7600,1,0xbe);
  do {
    uVar6 = uVar5 & 0xffff;
    FUN_00523874(DAT_008c7600,uVar6,0,"");
    uVar5 = uVar5 + 1;
    FUN_00523874(DAT_008c7600,uVar6,1,"");
  } while (uVar5 != 7);
  uVar1 = FUN_0051e96c(DAT_008c7600);
  FUN_004db8f0(uVar1,1,0x1d6);
  FUN_004e6d80(uVar1,0xff2a2a2a,0);
  FUN_004e6d90(uVar1,0xff,0);
  FUN_004db714(uVar1,0x50,0);
  uVar1 = FUN_0051e96c(DAT_008c7600);
  FUN_004db8f0(uVar1,1,0x1d6);
  FUN_004e6d80(uVar1,0xff2a2a2a,0);
  FUN_004e6d90(uVar1,0xff,0);
  FUN_004db714(uVar1,0,0);
  uVar1 = FUN_0051e96c(DAT_008c7600);
  FUN_004db8f0(uVar1,1,0x1d6);
  FUN_004e6d80(uVar1,0xff2a2a2a,0);
  FUN_004e6d90(uVar1,0xff,0);
  FUN_004db714(uVar1,0x118,0);
  FUN_004e6d80(DAT_008c7600,0xff000000,0x50000);
  FUN_004e6d80(DAT_008c7600,0xff1a1a1a,0x50001);
  FUN_004e6e70(DAT_008c7600,1,0x50000);
  FUN_004e6e50(DAT_008c7600,0xff2a2a2a,0x50000);
  FUN_004e7020(DAT_008c7600,0xffe2e2e2,0x50000);
  FUN_004e4820(DAT_008c7600,&DAT_00826054,0);
  iVar4 = DAT_008c7600;
  FUN_004e6d40(DAT_008c7600,7,0x50000);
  FUN_004e6d50(iVar4,7,0x50000);
  FUN_004e6d20(iVar4,7,0x50000);
  FUN_004e6d30(iVar4,7,0x50000);
  iVar4 = FUN_004134dc();
  if (iVar4 < 1) {
    sprintf(acStack_30,"V2.%d%d",1,6);
  }
  else if (iVar4 < 100) {
    sprintf(acStack_30,"V0.%d",iVar4);
  }
  else {
    sprintf(acStack_30,"V%d.%d",iVar4 / 100,iVar4 % 100);
  }
  FUN_00523874(DAT_008c7600,0,0,&DAT_0068f3b8);
  FUN_00523874(DAT_008c7600,1,0,&DAT_0068f3bc);
  FUN_00523874(DAT_008c7600,2,0,&DAT_0068f3c0);
  FUN_00523874(DAT_008c7600,3,0,"WLAN MAC");
  FUN_00523874(DAT_008c7600,4,0,"FCC ID");
  FUN_00523874(DAT_008c7600,5,0,"CMIIT ID");
  FUN_00523874(DAT_008c7600,6,0,"BUILD TIME");
  FUN_00523874(DAT_008c7600,6,1,"Jul  9 2026 11:40:09");
  FUN_00523874(DAT_008c7600,0,1,acStack_30);
  FUN_00523874(DAT_008c7600,4,1,"R56-FC2002");
  FUN_00523874(DAT_008c7600,5,1,"25J44K42C649");
  if (DAT_008c7600 == 0) {
    puts("device info table is null");
  }
  else {
    FUN_00523874(DAT_008c7600,1,1,&DAT_008c764c);
    FUN_00523874(DAT_008c7600,2,1,&DAT_008c768c);
    FUN_00523874(DAT_008c7600,3,1,&DAT_008c76cc);
  }
  thunk_FUN_00952500(DAT_008c7608);
  return;
}

