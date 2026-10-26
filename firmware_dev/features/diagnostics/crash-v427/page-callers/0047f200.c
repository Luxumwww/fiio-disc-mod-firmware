
void FUN_0047f200(undefined4 param_1)

{
  undefined4 uVar1;
  int iVar2;
  
  DAT_008c780c = param_1;
  DAT_008c7808 = FUN_004d992c(0);
  FUN_004db8f0(DAT_008c7808,0x168,0x168);
  FUN_004e7090(DAT_008c7808,0xb4,0);
  uVar1 = DAT_008c7808;
  FUN_004e6d40(DAT_008c7808,0,0);
  FUN_004e6d50(uVar1,0,0);
  FUN_004e6d20(uVar1,0,0);
  FUN_004e6d30(uVar1,0,0);
  FUN_004e6e70(DAT_008c7808,0,0);
  FUN_004e6d80(DAT_008c7808,0xff000000,0);
  FUN_004f3160(DAT_008c7808,1);
  FUN_004f31a4(DAT_008c7808,0,0,2);
  FUN_004dbe84(DAT_008c7808,1,0,0);
  FUN_004e7090(DAT_008c7808,0,0);
  FUN_004e6d60(DAT_008c7808,5,0);
  FUN_004e04b8(DAT_008c7808,0);
  FUN_004e0518(DAT_008c7808,0xc);
  FUN_004d3c7c(DAT_008c7808,FUN_0047ef4c,1,0);
  FUN_004d3c7c(DAT_008c7808,FUN_0047ef4c,2,0);
  FUN_004d3c7c(DAT_008c7808,FUN_0047ef4c,8,0);
  uVar1 = FUN_0046f51c();
  iVar2 = FUN_00428900("/usr/project/config/ui/set_menu/system_setting.json",uVar1);
  if (iVar2 == 0) {
    FUN_0045a5bc(DAT_008c7808,&DAT_0068c6f4,0xb4fb);
  }
  else {
    FUN_0045a5bc(DAT_008c7808,*(undefined4 *)(*(int *)(iVar2 + 8) + 0x30),0xb4fb);
  }
  FUN_00428dd0(iVar2);
  uVar1 = FUN_0046f51c();
  iVar2 = FUN_00428900("/usr/project/config/ui/set_menu/others.json",uVar1);
  if (iVar2 == 0) {
    FUN_0047efa8(DAT_008c7808,&DAT_0068f85c,0);
    FUN_0047efa8(DAT_008c7808,&DAT_0068f86c,1);
  }
  else {
    FUN_0047efa8(DAT_008c7808,*(undefined4 *)(*(int *)(iVar2 + 8) + 0x1c),0);
    FUN_0047efa8(DAT_008c7808,*(undefined4 *)(*(int *)(iVar2 + 8) + 0x20),1);
  }
  FUN_00428dd0(iVar2);
  uVar1 = FUN_004d992c(DAT_008c7808);
  FUN_004db8f0(uVar1,0x140,0x28);
  FUN_004e6e70(uVar1,0,0);
  FUN_004e6e50(uVar1,0xff000000,0);
  FUN_004e6d90(uVar1,0,0);
  FUN_004e04b8(uVar1,0);
  FUN_004e0518(uVar1,0);
  FUN_004d9960(uVar1,0x4000);
  FUN_00513720(FUN_0047ef10,5,0);
  thunk_FUN_00952500(DAT_008c7808);
  FUN_0045c6a0(DAT_008c8a79);
  FUN_004749d4(DAT_008c8a85);
  thunk_FUN_00953000(0);
  return;
}

