INFO  SettingsDump.java> 
void FUN_00473714(int param_1)

{
  int iVar1;
  undefined4 uVar2;
  uint uVar3;
  undefined4 uVar4;
  
  if (param_1 != 0) {
    DAT_008de150 = param_1;
    DAT_008de180 = FUN_004e969c(0);
    FUN_004eb680(DAT_008de180,0x168,0x168);
    uVar2 = DAT_008de180;
    FUN_004f6b10(DAT_008de180,0,0);
    FUN_004f6b20(uVar2,0,0);
    FUN_004f6af0(uVar2,0,0);
    FUN_004f6b00(uVar2,0,0);
    FUN_004f6c40(DAT_008de180,0,0);
    FUN_004f6b50(DAT_008de180,0xff000000,0);
    FUN_004ebc14(DAT_008de180,1,0,0);
    FUN_004f6e60(DAT_008de180,0,0);
    FUN_004f6b30(DAT_008de180,5,0);
    FUN_004f0268(DAT_008de180,0);
    FUN_004e39ec(DAT_008de180,FUN_00473640,1,0);
    FUN_004e39ec(DAT_008de180,FUN_00473640,2,0);
    FUN_004e39ec(DAT_008de180,FUN_00473640,8,0);
    FUN_004e2734(DAT_008de180);
    FUN_005030a0(DAT_008de180,1);
    FUN_005030e4(DAT_008de180,0,0,2);
    iVar1 = FUN_0042e640("/usr/project/config/ui/set_menu/system_setting.json",PTR_DAT_0083a660);
    if (iVar1 == 0) {
      FUN_0046d88c(DAT_008de180,&DAT_0069ff78,0xb4fb);
    }
    else {
      FUN_0046d88c(DAT_008de180,*(undefined4 *)(*(int *)(iVar1 + 8) + 0x30),0xb4fb);
    }
    FUN_0042eb10(iVar1);
    DAT_008de154 = FUN_0046d5c0(DAT_008de180,&DAT_0069ff88,0,FUN_00474270);
    DAT_008de158 = FUN_0046d5c0(DAT_008de180,&DAT_0069ff98,1,FUN_00474270);
    DAT_008de15c = FUN_0046d5c0(DAT_008de180,"English",2,FUN_00474270);
    DAT_008de160 = FUN_0046d5c0(DAT_008de180,&DAT_0069ffb0,3,FUN_00474270);
    DAT_008de164 = FUN_0046d5c0(DAT_008de180,&DAT_0069ffbc,4,FUN_00474270);
    DAT_008de168 = FUN_0046d5c0(DAT_008de180,&DAT_0069ffc8,5,FUN_00474270);
    DAT_008de16c = FUN_0046d5c0(DAT_008de180,"Italiano",6,FUN_00474270);
    DAT_008de170 = FUN_0046d5c0(DAT_008de180,"Deutsch",7,FUN_00474270);
    DAT_008de174 = FUN_0046d5c0(DAT_008de180,&DAT_0069ffe8,8,FUN_00474270);
    DAT_008de178 = FUN_0046d5c0(DAT_008de180,&DAT_0069fff4,9,FUN_00474270);
    uVar2 = FUN_004e969c(DAT_008de180);
    FUN_004eb680(uVar2,0x140,0x28);
    FUN_004f6c40(uVar2,0,0);
    FUN_004f6c20(uVar2,0xff000000,0);
    FUN_004f6b60(uVar2,0,0);
    FUN_004f0268(uVar2,0);
    FUN_004f02c8(uVar2,0);
    FUN_004e96d0(uVar2,0x4000);
    uVar3 = (uint)DAT_008e171f;
    if (uVar3 == 100) {
      uVar3 = 0;
    }
    FUN_0046d848((&DAT_008de154)[uVar3],1);
    return;
  }
  FUN_00479090();
  DAT_008de180 = FUN_004e969c(0);
  FUN_004eb680(DAT_008de180,0x168,0x168);
  uVar2 = DAT_008de180;
  FUN_004f6b10(DAT_008de180,0,0);
  FUN_004f6b20(uVar2,0,0);
  FUN_004f6af0(uVar2,0,0);
  FUN_004f6b00(uVar2,0,0);
  FUN_004f6c40(DAT_008de180,0,0);
  FUN_004f6b50(DAT_008de180,0xff000000,0);
  FUN_004ebc14(DAT_008de180,1,0,0);
  FUN_004f6e60(DAT_008de180,0,0);
  FUN_004f0268(DAT_008de180,0);
  FUN_004e39ec(DAT_008de180,FUN_00473640,1,0);
  FUN_004e39ec(DAT_008de180,FUN_00473640,2,0);
  FUN_004e39ec(DAT_008de180,FUN_00473640,8,0);
  FUN_004e2734(DAT_008de180);
  uVar2 = FUN_004e969c(DAT_008de180);
  FUN_004eb680(uVar2,0x168,300);
  FUN_004ebc14(uVar2,1,0,0);
  FUN_004f6c40(uVar2,0,0);
  FUN_004f6b60(uVar2,0,0);
  FUN_004f6b10(uVar2,0,0);
  FUN_004f6b20(uVar2,0,0);
  FUN_004f6af0(uVar2,0,0);
  FUN_004f6b00(uVar2,0,0);
  FUN_005030a0(uVar2,1);
  FUN_005030e4(uVar2,0,0,2);
  FUN_004f6b30(uVar2,5,0);
  FUN_004f6e80(uVar2,0,0x10000);
  iVar1 = FUN_0042e640("/usr/project/config/ui/set_menu/system_setting.json",PTR_DAT_0083a660);
  if (iVar1 == 0) {
    FUN_0046d88c(uVar2,&DAT_0069ff78,0xb4fb);
  }
  else {
    FUN_0046d88c(uVar2,*(undefined4 *)(*(int *)(iVar1 + 8) + 0x30),0xb4fb);
  }
  FUN_0042eb10(iVar1);
  DAT_008de154 = FUN_0046d5c0(uVar2,&DAT_0069ff88,0,&LAB_004741ec);
  DAT_008de158 = FUN_0046d5c0(uVar2,&DAT_0069ff98,1,&LAB_004741ec);
  DAT_008de15c = FUN_0046d5c0(uVar2,"English",2,&LAB_004741ec);
  DAT_008de160 = FUN_0046d5c0(uVar2,&DAT_0069ffb0,3,&LAB_004741ec);
  DAT_008de164 = FUN_0046d5c0(uVar2,&DAT_0069ffbc,4,&LAB_004741ec);
  DAT_008de168 = FUN_0046d5c0(uVar2,&DAT_0069ffc8,5,&LAB_004741ec);
  DAT_008de16c = FUN_0046d5c0(uVar2,"Italiano",6,&LAB_004741ec);
  DAT_008de170 = FUN_0046d5c0(uVar2,"Deutsch",7,&LAB_004741ec);
  DAT_008de174 = FUN_0046d5c0(uVar2,&DAT_0069ffe8,8,&LAB_004741ec);
  DAT_008de178 = FUN_0046d5c0(uVar2,&DAT_0069fff4,9,&LAB_004741ec);
  uVar2 = FUN_004e969c(uVar2);
  FUN_004eb680(uVar2,0x140,0x28);
  FUN_004f6c40(uVar2,0,0);
  FUN_004f6c20(uVar2,0xff000000,0);
  FUN_004f6b60(uVar2,0,0);
  FUN_004f0268(uVar2,0);
  FUN_004f02c8(uVar2,0);
  DAT_008e171f = 0;
  FUN_0046d848(DAT_008de154,1);
  uVar2 = FUN_0052a8a4(DAT_008de180);
  FUN_004eb680(uVar2,0x7f,0x29);
  FUN_004ebc14(uVar2,5,0,0xfffffff6);
  FUN_004f6c40(uVar2,0,0);
  FUN_004f6cb0(uVar2,0,0);
  FUN_004f6e60(uVar2,0x14,0);
  uVar4 = FUN_0052df64(uVar2);
  iVar1 = FUN_0042e640("/usr/project/config/ui/set_menu/popup.json",PTR_DAT_0083a660);
  if (iVar1 == 0) {
    FUN_0052f2bc(uVar4,&DAT_006a0004);
  }
  else {
    FUN_0052f2bc(uVar4,*(undefined4 *)(*(int *)(iVar1 + 8) + 0x68));
  }
  FUN_0042eb10(iVar1);
  FUN_004ebc14(uVar4,9,0,0);
  FUN_004f6df0(uVar4,0xffffffff,0);
  FUN_004f45d0(uVar4,&DAT_0083ac5c,0);
  FUN_004e39ec(uVar2,&LAB_0047369c,7,0);
  FUN_004e97f8(uVar2,0x10);
  uVar4 = FUN_004f8838(uVar2);
  iVar1 = FUN_004f889c(uVar4);
  FUN_004f8304(uVar2,iVar1 + -1);
  return;
}

 (GhidraScript)  
