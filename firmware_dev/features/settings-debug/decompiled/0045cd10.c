INFO  SettingsDump.java> 
void FUN_0045cd10(undefined4 param_1)

{
  undefined4 uVar1;
  int iVar2;
  char acStack_38 [36];
  
  DAT_008dd5b0 = param_1;
  DAT_008dd5d4 = FUN_004e969c(0);
  FUN_004eb680(DAT_008dd5d4,0x168,0x168);
  uVar1 = DAT_008dd5d4;
  FUN_004f6b10(DAT_008dd5d4,0,0);
  FUN_004f6b20(uVar1,0,0);
  FUN_004f6af0(uVar1,0,0);
  FUN_004f6b00(uVar1,0,0);
  FUN_004f6c40(DAT_008dd5d4,0,0);
  FUN_004f6b50(DAT_008dd5d4,0xff000000,0);
  FUN_004ebc14(DAT_008dd5d4,1,0,0);
  FUN_004f6e60(DAT_008dd5d4,0,0);
  FUN_004f6b30(DAT_008dd5d4,5,0);
  FUN_004f0268(DAT_008dd5d4,0);
  FUN_004e39ec(DAT_008dd5d4,FUN_0045cb48,1,0);
  FUN_004e39ec(DAT_008dd5d4,FUN_0045cb48,2,0);
  FUN_004e39ec(DAT_008dd5d4,FUN_0045cb48,8,0);
  FUN_004e2734(DAT_008dd5d4);
  FUN_005030a0(DAT_008dd5d4,1);
  FUN_005030e4(DAT_008dd5d4,0,0,2);
  uVar1 = FUN_00473fa8();
  iVar2 = FUN_0042e640("/usr/project/config/ui/set_menu/system_setting.json",uVar1);
  if (iVar2 == 0) {
    FUN_0046d88c(DAT_008dd5d4,&DAT_0069da94,0xb4fb);
  }
  else {
    FUN_0046d88c(DAT_008dd5d4,*(undefined4 *)(*(int *)(iVar2 + 8) + 0x10),0xb4fb);
  }
  FUN_0042eb10(iVar2);
  uVar1 = FUN_00473fa8();
  iVar2 = FUN_0042e640("/usr/project/config/ui/set_menu/others.json",uVar1);
  if (iVar2 == 0) {
    DAT_008dd5b8 = FUN_0046d5c0(DAT_008dd5d4,&DAT_0069dbb0,0,&LAB_0045ca70);
    DAT_008dd5bc = FUN_0046d5c0(DAT_008dd5d4,"10min",1,&LAB_0045ca70);
    DAT_008dd5c0 = FUN_0046d5c0(DAT_008dd5d4,"30min",2,&LAB_0045ca70);
    DAT_008dd5c4 = FUN_0046d5c0(DAT_008dd5d4,"60min",3,&LAB_0045ca70);
    DAT_008dd5c8 = FUN_0046d5c0(DAT_008dd5d4,"90min",4,&LAB_0045ca70);
    DAT_008dd5cc = FUN_0046d5c0(DAT_008dd5d4,"120min",5,&LAB_0045ca70);
  }
  else {
    DAT_008dd5b8 = FUN_0046d5c0(DAT_008dd5d4,*(undefined4 *)(*(int *)(iVar2 + 8) + 0x104),0,
                                &LAB_0045ca70);
    sprintf(acStack_38,"10%s",*(undefined4 *)(*(int *)(iVar2 + 8) + 0x140));
    DAT_008dd5bc = FUN_0046d5c0(DAT_008dd5d4,acStack_38,1,&LAB_0045ca70);
    sprintf(acStack_38,"30%s",*(undefined4 *)(*(int *)(iVar2 + 8) + 0x140));
    DAT_008dd5c0 = FUN_0046d5c0(DAT_008dd5d4,acStack_38,2,&LAB_0045ca70);
    sprintf(acStack_38,"60%s",*(undefined4 *)(*(int *)(iVar2 + 8) + 0x140));
    DAT_008dd5c4 = FUN_0046d5c0(DAT_008dd5d4,acStack_38,3,&LAB_0045ca70);
    sprintf(acStack_38,"90%s",*(undefined4 *)(*(int *)(iVar2 + 8) + 0x140));
    DAT_008dd5c8 = FUN_0046d5c0(DAT_008dd5d4,acStack_38,4,&LAB_0045ca70);
    sprintf(acStack_38,"120%s",*(undefined4 *)(*(int *)(iVar2 + 8) + 0x140));
    DAT_008dd5cc = FUN_0046d5c0(DAT_008dd5d4,acStack_38,5,&LAB_0045ca70);
  }
  FUN_0042eb10(iVar2);
  DAT_008dd5d0 = FUN_004e969c(DAT_008dd5d4);
  FUN_004eb680(DAT_008dd5d0,0x168,0x28);
  FUN_004f6c40(DAT_008dd5d0,0,0);
  FUN_004f6c20(DAT_008dd5d0,0xff000000,0);
  FUN_004f6b60(DAT_008dd5d0,0,0);
  FUN_004f0268(DAT_008dd5d0,0);
  FUN_004f02c8(DAT_008dd5d0,0);
  FUN_0046d848((&DAT_008dd5b8)[DAT_008e1725],1);
  FUN_004f273c((&DAT_008dd5b8)[DAT_008e1725 + 1],1);
  return;
}

 (GhidraScript)  
