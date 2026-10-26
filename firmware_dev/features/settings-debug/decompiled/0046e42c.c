INFO  SettingsDump.java> 
void FUN_0046e42c(undefined4 param_1)

{
  undefined4 uVar1;
  int iVar2;
  char acStack_38 [36];
  
  DAT_008ddfd0 = param_1;
  DAT_008ddff8 = FUN_004e969c(0);
  FUN_004eb680(DAT_008ddff8,0x168,0x168);
  uVar1 = DAT_008ddff8;
  FUN_004f6b10(DAT_008ddff8,0,0);
  FUN_004f6b20(uVar1,0,0);
  FUN_004f6af0(uVar1,0,0);
  FUN_004f6b00(uVar1,0,0);
  FUN_004f6c40(DAT_008ddff8,0,0);
  FUN_004f6b50(DAT_008ddff8,0xff000000,0);
  FUN_004ebc14(DAT_008ddff8,1,0,0);
  FUN_004f6e60(DAT_008ddff8,0,0);
  FUN_004f6b30(DAT_008ddff8,5,0);
  FUN_004f0268(DAT_008ddff8,0);
  FUN_004e39ec(DAT_008ddff8,&LAB_0046e180,1,0);
  FUN_004e39ec(DAT_008ddff8,&LAB_0046e180,2,0);
  FUN_004e39ec(DAT_008ddff8,&LAB_0046e180,8,0);
  FUN_004e2734(DAT_008ddff8);
  FUN_005030a0(DAT_008ddff8,1);
  FUN_005030e4(DAT_008ddff8,0,0,2);
  uVar1 = FUN_00473fa8();
  iVar2 = FUN_0042e640("/usr/project/config/ui/set_menu/system_setting.json",uVar1);
  if (iVar2 == 0) {
    FUN_0046d88c(DAT_008ddff8,&DAT_0069daa4,0xb4fb);
  }
  else {
    FUN_0046d88c(DAT_008ddff8,*(undefined4 *)(*(int *)(iVar2 + 8) + 0x14),0xb4fb);
  }
  FUN_0042eb10(iVar2);
  uVar1 = FUN_00473fa8();
  iVar2 = FUN_0042e640("/usr/project/config/ui/set_menu/others.json",uVar1);
  if (iVar2 == 0) {
    DAT_008ddfd8 = FUN_0046d5c0(DAT_008ddff8,&DAT_0069dbb0,0,FUN_0046e1dc);
    DAT_008ddfdc = FUN_0046d5c0(DAT_008ddff8,&DAT_0069fbc8,1,FUN_0046e1dc);
    DAT_008ddfe0 = FUN_0046d5c0(DAT_008ddff8,"10min",2,FUN_0046e1dc);
    DAT_008ddfe4 = FUN_0046d5c0(DAT_008ddff8,"30min",3,FUN_0046e1dc);
    DAT_008ddfe8 = FUN_0046d5c0(DAT_008ddff8,"60min",4,FUN_0046e1dc);
    DAT_008ddfec = FUN_0046d5c0(DAT_008ddff8,"90min",5,FUN_0046e1dc);
    DAT_008ddff0 = FUN_0046d5c0(DAT_008ddff8,"120min",6,FUN_0046e1dc);
  }
  else {
    DAT_008ddfd8 = FUN_0046d5c0(DAT_008ddff8,*(undefined4 *)(*(int *)(iVar2 + 8) + 0x104),0,
                                FUN_0046e1dc);
    sprintf(acStack_38,(char *)&PTR_DAT_0069fbd0,*(undefined4 *)(*(int *)(iVar2 + 8) + 0x140));
    DAT_008ddfdc = FUN_0046d5c0(DAT_008ddff8,acStack_38,1,FUN_0046e1dc);
    sprintf(acStack_38,"10%s",*(undefined4 *)(*(int *)(iVar2 + 8) + 0x140));
    DAT_008ddfe0 = FUN_0046d5c0(DAT_008ddff8,acStack_38,2,FUN_0046e1dc);
    sprintf(acStack_38,"30%s",*(undefined4 *)(*(int *)(iVar2 + 8) + 0x140));
    DAT_008ddfe4 = FUN_0046d5c0(DAT_008ddff8,acStack_38,3,FUN_0046e1dc);
    sprintf(acStack_38,"60%s",*(undefined4 *)(*(int *)(iVar2 + 8) + 0x140));
    DAT_008ddfe8 = FUN_0046d5c0(DAT_008ddff8,acStack_38,4,FUN_0046e1dc);
    sprintf(acStack_38,"90%s",*(undefined4 *)(*(int *)(iVar2 + 8) + 0x140));
    DAT_008ddfec = FUN_0046d5c0(DAT_008ddff8,acStack_38,5,FUN_0046e1dc);
    sprintf(acStack_38,"120%s",*(undefined4 *)(*(int *)(iVar2 + 8) + 0x140));
    DAT_008ddff0 = FUN_0046d5c0(DAT_008ddff8,acStack_38,6,FUN_0046e1dc);
  }
  FUN_0042eb10(iVar2);
  DAT_008ddff4 = FUN_004e969c(DAT_008ddff8);
  FUN_004eb680(DAT_008ddff4,0x168,0x28);
  FUN_004f6c40(DAT_008ddff4,0,0);
  FUN_004f6c20(DAT_008ddff4,0xff000000,0);
  FUN_004f6b60(DAT_008ddff4,0,0);
  FUN_004f0268(DAT_008ddff4,0);
  FUN_004f02c8(DAT_008ddff4,0);
  FUN_0046d848((&DAT_008ddfd8)[DAT_008e1719],1);
  FUN_004f273c((&DAT_008ddfd8)[DAT_008e1719 + 1],1);
  return;
}

 (GhidraScript)  
