INFO  SettingsDump.java> 
void FUN_0045d238(undefined4 param_1)

{
  undefined4 uVar1;
  undefined4 uVar2;
  undefined4 uVar3;
  int iVar4;
  uint uVar5;
  uint uVar6;
  char acStack_30 [20];
  
  DAT_008dd5e4 = param_1;
  DAT_008dd5e8 = FUN_004e969c(0);
  FUN_004eb680(DAT_008dd5e8,0x168,0x168);
  FUN_004f6e60(DAT_008dd5e8,0xb4,0);
  uVar1 = DAT_008dd5e8;
  FUN_004f6b10(DAT_008dd5e8,0,0);
  FUN_004f6b20(uVar1,0,0);
  FUN_004f6af0(uVar1,0,0);
  FUN_004f6b00(uVar1,0,0);
  FUN_004f6c40(DAT_008dd5e8,0,0);
  FUN_004f6b50(DAT_008dd5e8,0xff000000,0);
  FUN_004ebc14(DAT_008dd5e8,1,0,0);
  FUN_004f6e60(DAT_008dd5e8,0,0);
  FUN_004f6b30(DAT_008dd5e8,5,0);
  uVar1 = FUN_004e969c(DAT_008dd5e8);
  FUN_004eb680(uVar1,0x20000064);
  FUN_004f6b10(uVar1,10,0);
  FUN_004f6b20(uVar1,10,0);
  FUN_004f6af0(uVar1,10,0);
  FUN_004f6b00(uVar1,10,0);
  FUN_004f6b50(uVar1,0xff000000,0);
  FUN_004f6c40(uVar1,0,0);
  FUN_004e97f8(uVar1,0x10);
  FUN_004e97f8(uVar1,0x8000);
  FUN_004ebc14(uVar1,9,0,0);
  FUN_004f0268(uVar1,0);
  uVar2 = FUN_0052df64(uVar1);
  uVar3 = FUN_00473fa8();
  iVar4 = FUN_0042e640("/usr/project/config/ui/set_menu/system_setting.json",uVar3);
  if (iVar4 == 0) {
    FUN_0052f2bc(uVar2,&DAT_0069db50);
  }
  else {
    FUN_0052f2bc(uVar2,*(undefined4 *)(*(int *)(iVar4 + 8) + 0x3c));
  }
  FUN_0042eb10(iVar4);
  FUN_004eb75c(uVar2,0xb4);
  FUN_0052f4f0(uVar2,2);
  FUN_004f45d0(uVar2,&DAT_0083ac54,0);
  FUN_004f6df0(uVar2,0xffffffff,0);
  FUN_004ebc14(uVar2,2,0,10);
  DAT_008dd5e0 = FUN_00535310(uVar1);
  FUN_004eb680(DAT_008dd5e0,0x135,0x20000064);
  FUN_004f6b50(DAT_008dd5e0,0xff000000,0);
  FUN_004f6c40(DAT_008dd5e0,0,0);
  iVar4 = DAT_008dd5e0;
  FUN_004f6b10(DAT_008dd5e0,10,0);
  FUN_004f6b20(iVar4,10,0);
  FUN_004f6af0(iVar4,10,0);
  FUN_004f6b00(iVar4,10,0);
  FUN_004ebc14(DAT_008dd5e0,2,5,0x35);
  FUN_004e96d0(DAT_008dd5e0,2);
  FUN_004e97f8(DAT_008dd5e0,0x8000);
  FUN_004e96d0(DAT_008dd5e0,0x10);
  FUN_004f0268(DAT_008dd5e0,0);
  FUN_004e96d0(DAT_008dd5e0,0x4000);
  FUN_004e39ec(uVar1,&LAB_0045d170,1,0);
  FUN_004e39ec(uVar1,&LAB_0045d170,2,0);
  FUN_004e39ec(uVar1,&LAB_0045d170,8,0);
  FUN_004e39ec(uVar1,&LAB_0045d170,0xc,0);
  uVar5 = 0;
  FUN_005354c4(DAT_008dd5e0,2);
  FUN_00535344(DAT_008dd5e0,7);
  FUN_00535a4c(DAT_008dd5e0,0,0x5a);
  FUN_00535a4c(DAT_008dd5e0,1,0xbe);
  do {
    uVar6 = uVar5 & 0xffff;
    FUN_00535784(DAT_008dd5e0,uVar6,0,"");
    uVar5 = uVar5 + 1;
    FUN_00535784(DAT_008dd5e0,uVar6,1,"");
  } while (uVar5 != 7);
  uVar1 = FUN_0053087c(DAT_008dd5e0);
  FUN_004eb680(uVar1,1,0x1d6);
  FUN_004f6b50(uVar1,0xff2a2a2a,0);
  FUN_004f6b60(uVar1,0xff,0);
  FUN_004eb4a4(uVar1,0x50,0);
  uVar1 = FUN_0053087c(DAT_008dd5e0);
  FUN_004eb680(uVar1,1,0x1d6);
  FUN_004f6b50(uVar1,0xff2a2a2a,0);
  FUN_004f6b60(uVar1,0xff,0);
  FUN_004eb4a4(uVar1,0,0);
  uVar1 = FUN_0053087c(DAT_008dd5e0);
  FUN_004eb680(uVar1,1,0x1d6);
  FUN_004f6b50(uVar1,0xff2a2a2a,0);
  FUN_004f6b60(uVar1,0xff,0);
  FUN_004eb4a4(uVar1,0x118,0);
  FUN_004f6b50(DAT_008dd5e0,0xff000000,0x50000);
  FUN_004f6b50(DAT_008dd5e0,0xff1a1a1a,0x50001);
  FUN_004f6c40(DAT_008dd5e0,1,0x50000);
  FUN_004f6c20(DAT_008dd5e0,0xff2a2a2a,0x50000);
  FUN_004f6df0(DAT_008dd5e0,0xffe2e2e2,0x50000);
  FUN_004f45d0(DAT_008dd5e0,&DAT_0083ac5c,0);
  iVar4 = DAT_008dd5e0;
  FUN_004f6b10(DAT_008dd5e0,7,0x50000);
  FUN_004f6b20(iVar4,7,0x50000);
  FUN_004f6af0(iVar4,7,0x50000);
  FUN_004f6b00(iVar4,7,0x50000);
  iVar4 = FUN_00414d78();
  if (iVar4 < 1) {
    sprintf(acStack_30,"V2.%d%d",1,6);
  }
  else if (iVar4 < 100) {
    sprintf(acStack_30,"V0.%d",iVar4);
  }
  else {
    sprintf(acStack_30,"V%d.%d",iVar4 / 100,iVar4 % 100);
  }
  FUN_00535784(DAT_008dd5e0,0,0,&DAT_0069df58);
  FUN_00535784(DAT_008dd5e0,1,0,&DAT_0069df5c);
  FUN_00535784(DAT_008dd5e0,2,0,&DAT_0069df60);
  FUN_00535784(DAT_008dd5e0,3,0,"WLAN MAC");
  FUN_00535784(DAT_008dd5e0,4,0,"FCC ID");
  FUN_00535784(DAT_008dd5e0,5,0,"CMIIT ID");
  FUN_00535784(DAT_008dd5e0,6,0,"BUILD TIME");
  FUN_00535784(DAT_008dd5e0,6,1,"Sep  9 2026 17:13:49");
  FUN_00535784(DAT_008dd5e0,0,1,acStack_30);
  FUN_00535784(DAT_008dd5e0,4,1,"R56-FC2002");
  FUN_00535784(DAT_008dd5e0,5,1,"25J44K42C649");
  if (DAT_008dd5e0 == 0) {
    puts("device info table is null");
  }
  else {
    FUN_00535784(DAT_008dd5e0,1,1,&DAT_008dd62c);
    FUN_00535784(DAT_008dd5e0,2,1,&DAT_008dd66c);
    FUN_00535784(DAT_008dd5e0,3,1,&DAT_008dd6ac);
  }
  FUN_004e2734(DAT_008dd5e8);
  return;
}

 (GhidraScript)  
