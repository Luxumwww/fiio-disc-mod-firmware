INFO  SettingsDump.java> 
void FUN_00473054(undefined4 param_1)

{
  undefined4 uVar1;
  int iVar2;
  char *pcVar3;
  char acStack_38 [32];
  
  DAT_008de114 = param_1;
  DAT_008de144 = FUN_004e969c(0);
  FUN_004eb680(DAT_008de144,0x168,0x168);
  uVar1 = DAT_008de144;
  FUN_004f6b10(DAT_008de144,0,0);
  FUN_004f6b20(uVar1,0,0);
  FUN_004f6af0(uVar1,0,0);
  FUN_004f6b00(uVar1,0,0);
  FUN_004f6c40(DAT_008de144,0,0);
  FUN_004f6b50(DAT_008de144,0xff000000,0);
  FUN_004ebc14(DAT_008de144,1,0,0);
  FUN_004f6e60(DAT_008de144,0,0);
  FUN_004f6b30(DAT_008de144,5,0);
  FUN_004f0268(DAT_008de144,0);
  FUN_004e39ec(DAT_008de144,FUN_00472e7c,1,0);
  FUN_004e39ec(DAT_008de144,FUN_00472e7c,2,0);
  FUN_004e39ec(DAT_008de144,FUN_00472e7c,8,0);
  FUN_005030a0(DAT_008de144,1);
  FUN_005030e4(DAT_008de144,0,0,2);
  FUN_004f02c8(DAT_008de144,0xc);
  uVar1 = FUN_00473fa8();
  iVar2 = FUN_0042e640("/usr/project/config/ui/set_menu/system_setting.json",uVar1);
  if (iVar2 == 0) {
    FUN_0046d88c(DAT_008de144,&DAT_0069da74,0xb4fb);
  }
  else {
    FUN_0046d88c(DAT_008de144,*(undefined4 *)(*(int *)(iVar2 + 8) + 8),0xb4fb);
  }
  FUN_0042eb10(iVar2);
  uVar1 = FUN_00473fa8();
  iVar2 = FUN_0042e640("/usr/project/config/ui/set_menu/others.json",uVar1);
  if (iVar2 == 0) {
    DAT_008de118 = FUN_0046d5c0(DAT_008de144,&PTR_DAT_0069ff40,0,&LAB_00472cd0);
    DAT_008de11c = FUN_0046d5c0(DAT_008de144,&PTR_DAT_0069ff44,1,&LAB_00472cd0);
    DAT_008de120 = FUN_0046d5c0(DAT_008de144,&PTR_DAT_0069ff48,2,&LAB_00472cd0);
    DAT_008de124 = FUN_0046d5c0(DAT_008de144,&DAT_0069ff4c,3,&LAB_00472cd0);
    DAT_008de128 = FUN_0046d5c0(DAT_008de144,&DAT_0069fbc8,4,&LAB_00472cd0);
    DAT_008de12c = FUN_0046d5c0(DAT_008de144,"10min",5,&LAB_00472cd0);
    DAT_008de130 = FUN_0046d5c0(DAT_008de144,"30min",6,&LAB_00472cd0);
    uVar1 = 6;
    pcVar3 = "NEVER";
  }
  else {
    sprintf(acStack_38,"15%s",*(undefined4 *)(*(int *)(iVar2 + 8) + 0x13c));
    DAT_008de118 = FUN_0046d5c0(DAT_008de144,acStack_38,0,&LAB_00472cd0);
    sprintf(acStack_38,"30%s",*(undefined4 *)(*(int *)(iVar2 + 8) + 0x13c));
    DAT_008de11c = FUN_0046d5c0(DAT_008de144,acStack_38,1,&LAB_00472cd0);
    sprintf(acStack_38,"60%s",*(undefined4 *)(*(int *)(iVar2 + 8) + 0x13c));
    DAT_008de120 = FUN_0046d5c0(DAT_008de144,acStack_38,2,&LAB_00472cd0);
    sprintf(acStack_38,(char *)&PTR_DAT_0069ff64,*(undefined4 *)(*(int *)(iVar2 + 8) + 0x140));
    DAT_008de124 = FUN_0046d5c0(DAT_008de144,acStack_38,3,&LAB_00472cd0);
    sprintf(acStack_38,(char *)&PTR_DAT_0069fbd0,*(undefined4 *)(*(int *)(iVar2 + 8) + 0x140));
    DAT_008de128 = FUN_0046d5c0(DAT_008de144,acStack_38,4,&LAB_00472cd0);
    sprintf(acStack_38,"10%s",*(undefined4 *)(*(int *)(iVar2 + 8) + 0x140));
    DAT_008de12c = FUN_0046d5c0(DAT_008de144,acStack_38,5,&LAB_00472cd0);
    sprintf(acStack_38,"30%s",*(undefined4 *)(*(int *)(iVar2 + 8) + 0x140));
    DAT_008de130 = FUN_0046d5c0(DAT_008de144,acStack_38,6,&LAB_00472cd0);
    pcVar3 = *(char **)(*(int *)(iVar2 + 8) + 0x184);
    uVar1 = 7;
  }
  DAT_008de134 = FUN_0046d5c0(DAT_008de144,pcVar3,uVar1,&LAB_00472cd0);
  FUN_0042eb10(iVar2);
  DAT_008de138 = FUN_004e969c(DAT_008de144);
  FUN_004eb680(DAT_008de138,0x168,0x28);
  FUN_004f6c40(DAT_008de138,0,0);
  FUN_004f6c20(DAT_008de138,0xff000000,0);
  FUN_004f6b60(DAT_008de138,0,0);
  FUN_004f0268(DAT_008de138,0);
  FUN_004f02c8(DAT_008de138,0);
  FUN_0046d848((&DAT_008de118)[DAT_008e1721],1);
  FUN_004f273c((&DAT_008de118)[DAT_008e1721 + 1],1);
  FUN_004e2734(DAT_008de144);
  return;
}

 (GhidraScript)  
