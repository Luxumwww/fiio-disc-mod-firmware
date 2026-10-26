INFO  SettingsDump.java> 
void FUN_0045777c(void)

{
  undefined4 uVar1;
  int iVar2;
  
  DAT_008dd4f0 = FUN_004e969c(0);
  FUN_00486620(DAT_008dd4f0);
  FUN_004eb680(DAT_008dd4f0,0x168,0x168);
  FUN_004f6e60(DAT_008dd4f0,0xb4,0);
  uVar1 = DAT_008dd4f0;
  FUN_004f6b10(DAT_008dd4f0,0,0);
  FUN_004f6b20(uVar1,0,0);
  FUN_004f6af0(uVar1,0,0);
  FUN_004f6b00(uVar1,0,0);
  FUN_004f6c40(DAT_008dd4f0,0,0);
  FUN_004f6b50(DAT_008dd4f0,0xff000000,0);
  FUN_005030a0(DAT_008dd4f0,1);
  FUN_005030e4(DAT_008dd4f0,0,0,2);
  FUN_004ebc14(DAT_008dd4f0,1,0,0);
  FUN_004f6e60(DAT_008dd4f0,0,0);
  FUN_004f6b30(DAT_008dd4f0,5,0);
  FUN_004f0268(DAT_008dd4f0,0);
  FUN_004f02c8(DAT_008dd4f0,0xc);
  FUN_004e39ec(DAT_008dd4f0,FUN_00457738,1,0);
  FUN_004e39ec(DAT_008dd4f0,FUN_00457738,2,0);
  FUN_004e39ec(DAT_008dd4f0,FUN_00457738,8,0);
  uVar1 = FUN_00473fa8();
  iVar2 = FUN_0042e640("/usr/project/config/ui/set_menu/system.json",uVar1);
  if (iVar2 == 0) {
    FUN_0046d88c(DAT_008dd4f0,&DAT_0069da4c,0xb4fb);
  }
  else {
    FUN_0046d88c(DAT_008dd4f0,*(undefined4 *)(*(int *)(iVar2 + 8) + 4),0xb4fb);
  }
  FUN_0042eb10(iVar2);
  uVar1 = FUN_00473fa8();
  iVar2 = FUN_0042e640("/usr/project/config/ui/set_menu/system_setting.json",uVar1);
  if (iVar2 == 0) {
    FUN_00485ef8(DAT_008dd4f0,&DAT_0069da5c,0,&LAB_004575f0,0);
    FUN_00485ef8(DAT_008dd4f0,&DAT_0069da6c,1,&LAB_004575f0,0);
    FUN_00485ef8(DAT_008dd4f0,&DAT_0069da74,2,&LAB_004575f0,0);
    FUN_00485ef8(DAT_008dd4f0,&DAT_0069da84,3,&LAB_004575f0,0);
    FUN_00485ef8(DAT_008dd4f0,&DAT_0069da94,4,&LAB_004575f0,0);
    FUN_00485ef8(DAT_008dd4f0,&DAT_0069daa4,5,&LAB_004575f0,0);
    FUN_00485ef8(DAT_008dd4f0,&DAT_0069dab4,9,&LAB_004575f0,0);
    FUN_00485ef8(DAT_008dd4f0,&DAT_0069dad0,10,&LAB_004575f0,0);
    FUN_00485ef8(DAT_008dd4f0,&DAT_0069dae4,0xb,&LAB_004575f0,0);
    FUN_00485ef8(DAT_008dd4f0,&DAT_0069daf4,6,&LAB_004575f0,0);
    FUN_00485ef8(DAT_008dd4f0,&DAT_0069db04,7,&LAB_004575f0,0);
    FUN_00485ef8(DAT_008dd4f0,&DAT_0069db14,8,&LAB_004575f0,0);
    FUN_00485ef8(DAT_008dd4f0,&DAT_0069db24,0xc,&LAB_004575f0,0);
    FUN_00485ef8(DAT_008dd4f0,&DAT_0069db2c,0xd,&LAB_004575f0,0);
    FUN_00485ef8(DAT_008dd4f0,&DAT_0069db3c,0xe,&LAB_004575f0,0);
    FUN_00485ef8(DAT_008dd4f0,&DAT_0069db50,0xf,&LAB_004575f0,0);
  }
  else {
    FUN_00485ef8(DAT_008dd4f0,**(undefined4 **)(iVar2 + 8),0,&LAB_004575f0,0);
    FUN_00485ef8(DAT_008dd4f0,*(undefined4 *)(*(int *)(iVar2 + 8) + 4),1,&LAB_004575f0,0);
    FUN_00485ef8(DAT_008dd4f0,*(undefined4 *)(*(int *)(iVar2 + 8) + 8),2,&LAB_004575f0,0);
    FUN_00485ef8(DAT_008dd4f0,*(undefined4 *)(*(int *)(iVar2 + 8) + 0xc),3,&LAB_004575f0,0);
    FUN_00485ef8(DAT_008dd4f0,*(undefined4 *)(*(int *)(iVar2 + 8) + 0x10),4,&LAB_004575f0,0);
    FUN_00485ef8(DAT_008dd4f0,*(undefined4 *)(*(int *)(iVar2 + 8) + 0x14),5,&LAB_004575f0,0);
    FUN_00485ef8(DAT_008dd4f0,*(undefined4 *)(*(int *)(iVar2 + 8) + 0x18),6,&LAB_004575f0,0);
    FUN_00485ef8(DAT_008dd4f0,*(undefined4 *)(*(int *)(iVar2 + 8) + 0x1c),7,&LAB_004575f0,0);
    FUN_00485ef8(DAT_008dd4f0,*(undefined4 *)(*(int *)(iVar2 + 8) + 0x20),8,&LAB_004575f0,0);
    FUN_00485ef8(DAT_008dd4f0,*(undefined4 *)(*(int *)(iVar2 + 8) + 0x24),9,&LAB_004575f0,0);
    FUN_00485ef8(DAT_008dd4f0,*(undefined4 *)(*(int *)(iVar2 + 8) + 0x28),10,&LAB_004575f0,0);
    FUN_00485ef8(DAT_008dd4f0,*(undefined4 *)(*(int *)(iVar2 + 8) + 0x2c),0xb,&LAB_004575f0,0);
    FUN_00485ef8(DAT_008dd4f0,*(undefined4 *)(*(int *)(iVar2 + 8) + 0x30),0xc,&LAB_004575f0,0);
    FUN_00485ef8(DAT_008dd4f0,*(undefined4 *)(*(int *)(iVar2 + 8) + 0x34),0xd,&LAB_004575f0,0);
    FUN_00485ef8(DAT_008dd4f0,*(undefined4 *)(*(int *)(iVar2 + 8) + 0x38),0xe,&LAB_004575f0,0);
    FUN_00485ef8(DAT_008dd4f0,*(undefined4 *)(*(int *)(iVar2 + 8) + 0x3c),0xf,&LAB_004575f0,0);
  }
  FUN_0042eb10(iVar2);
  uVar1 = FUN_004e969c(DAT_008dd4f0);
  FUN_004eb680(uVar1,0x140,0x28);
  FUN_004f6c40(uVar1,0,0);
  FUN_004f6c20(uVar1,0xff000000,0);
  FUN_004f6b60(uVar1,0,0);
  FUN_004f0268(uVar1,0);
  FUN_004f02c8(uVar1,0);
  FUN_004e96d0(uVar1,0x4000);
  return;
}

 (GhidraScript)  
