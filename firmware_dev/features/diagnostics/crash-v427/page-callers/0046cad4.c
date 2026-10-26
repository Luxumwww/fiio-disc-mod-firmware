
void FUN_0046cad4(undefined4 param_1)

{
  undefined4 uVar1;
  int iVar2;
  undefined *puVar3;
  
  DAT_008c7380 = param_1;
  DAT_008c7388 = FUN_004d992c(0);
  FUN_004db8f0(DAT_008c7388,0x168,0x168);
  uVar1 = DAT_008c7388;
  FUN_004e6d40(DAT_008c7388,0,0);
  FUN_004e6d50(uVar1,0,0);
  FUN_004e6d20(uVar1,0,0);
  FUN_004e6d30(uVar1,0,0);
  FUN_004e6e70(DAT_008c7388,0,0);
  FUN_004e6d80(DAT_008c7388,0xff000000,0);
  FUN_004dbe84(DAT_008c7388,1,0,0);
  FUN_004e7090(DAT_008c7388,0,0);
  FUN_004e6d60(DAT_008c7388,5,0);
  FUN_004e04b8(DAT_008c7388,0);
  FUN_004d3c7c(DAT_008c7388,FUN_0046c920,1,0);
  FUN_004d3c7c(DAT_008c7388,FUN_0046c920,2,0);
  FUN_004d3c7c(DAT_008c7388,FUN_0046c920,8,0);
  thunk_FUN_00952500(DAT_008c7388);
  FUN_004f3160(DAT_008c7388,1);
  FUN_004f31a4(DAT_008c7388,0,0,2);
  uVar1 = FUN_0046f51c();
  iVar2 = FUN_00428900("/usr/project/config/ui/set_menu/others.json",uVar1);
  if (iVar2 == 0) {
    FUN_0045a5bc(DAT_008c7388,&DAT_0068c9a4,0xb4fb);
  }
  else {
    FUN_0045a5bc(DAT_008c7388,*(undefined4 *)(*(int *)(iVar2 + 8) + 0x16c),0xb4fb);
  }
  FUN_00428dd0(iVar2);
  uVar1 = FUN_0046f51c();
  iVar2 = FUN_00428900("/usr/project/config/ui/set_menu/others.json",uVar1);
  if (iVar2 == 0) {
    DAT_008c738c = FUN_0045a33c(DAT_008c7388,&DAT_0068aeb0,0,FUN_0046c97c);
    puVar3 = &DAT_0068aec0;
  }
  else {
    DAT_008c738c = FUN_0045a33c(DAT_008c7388,*(undefined4 *)(*(int *)(iVar2 + 8) + 0x178),0,
                                FUN_0046c97c);
    puVar3 = *(undefined **)(*(int *)(iVar2 + 8) + 0x17c);
  }
  DAT_008c7390 = FUN_0045a33c(DAT_008c7388,puVar3,1,FUN_0046c97c);
  FUN_00428dd0(iVar2);
  FUN_0045a578((&DAT_008c738c)[DAT_008c8a8f],1);
  return;
}

