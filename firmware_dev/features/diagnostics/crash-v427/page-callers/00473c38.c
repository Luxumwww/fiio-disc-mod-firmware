
void FUN_00473c38(undefined4 param_1)

{
  undefined4 uVar1;
  int iVar2;
  undefined *puVar3;
  
  DAT_008c74e4 = param_1;
  DAT_008c74f4 = FUN_004d992c(0);
  FUN_004db8f0(DAT_008c74f4,0x168,0x168);
  uVar1 = DAT_008c74f4;
  FUN_004e6d40(DAT_008c74f4,0,0);
  FUN_004e6d50(uVar1,0,0);
  FUN_004e6d20(uVar1,0,0);
  FUN_004e6d30(uVar1,0,0);
  FUN_004e6e70(DAT_008c74f4,0,0);
  FUN_004e6d80(DAT_008c74f4,0xff000000,0);
  FUN_004dbe84(DAT_008c74f4,1,0,0);
  FUN_004e7090(DAT_008c74f4,0,0);
  FUN_004e6d60(DAT_008c74f4,5,0);
  FUN_004e04b8(DAT_008c74f4,0);
  FUN_004d3c7c(DAT_008c74f4,FUN_00473b24,1,0);
  FUN_004d3c7c(DAT_008c74f4,FUN_00473b24,2,0);
  FUN_004d3c7c(DAT_008c74f4,FUN_00473b24,8,0);
  thunk_FUN_00952500(DAT_008c74f4);
  FUN_004f3160(DAT_008c74f4,1);
  FUN_004f31a4(DAT_008c74f4,0,0,2);
  uVar1 = FUN_0046f51c();
  iVar2 = FUN_00428900("/usr/project/config/ui/set_menu/audio_setting.json",uVar1);
  if (iVar2 == 0) {
    FUN_0045a5bc(DAT_008c74f4,&DAT_0068ba9c,0xfe0156);
    DAT_008c74e8 = FUN_0045a33c(DAT_008c74f4,&DAT_00687bd8,0,FUN_00473b80);
    DAT_008c74ec = FUN_0045a33c(DAT_008c74f4,&DAT_0068f240,1,FUN_00473b80);
    puVar3 = &DAT_0068f248;
  }
  else {
    FUN_0045a5bc(DAT_008c74f4,*(undefined4 *)(*(int *)(iVar2 + 8) + 4),0xfe0156);
    DAT_008c74e8 = FUN_0045a33c(DAT_008c74f4,*(undefined4 *)(*(int *)(iVar2 + 8) + 0x20),0,
                                FUN_00473b80);
    DAT_008c74ec = FUN_0045a33c(DAT_008c74f4,*(undefined4 *)(*(int *)(iVar2 + 8) + 0x24),1,
                                FUN_00473b80);
    puVar3 = *(undefined **)(*(int *)(iVar2 + 8) + 0x28);
  }
  DAT_008c74f0 = FUN_0045a33c(DAT_008c74f4,puVar3,2,FUN_00473b80);
  FUN_00428dd0(iVar2);
  uVar1 = FUN_004e8a80((&DAT_008c74e8)[DAT_008c8a68],1);
  FUN_004d9b88(uVar1,1);
  return;
}

