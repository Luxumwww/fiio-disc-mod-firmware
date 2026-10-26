
void FUN_00468098(undefined4 param_1)

{
  undefined4 uVar1;
  int iVar2;
  undefined *puVar3;
  
  DAT_008c7264 = param_1;
  DAT_008c727c = FUN_004d992c(0);
  FUN_004db8f0(DAT_008c727c,0x168,0x168);
  uVar1 = DAT_008c727c;
  FUN_004e6d40(DAT_008c727c,0,0);
  FUN_004e6d50(uVar1,0,0);
  FUN_004e6d20(uVar1,0,0);
  FUN_004e6d30(uVar1,0,0);
  FUN_004e6e70(DAT_008c727c,0,0);
  FUN_004e6d80(DAT_008c727c,0xff000000,0);
  FUN_004dbe84(DAT_008c727c,1,0,0);
  FUN_004e7090(DAT_008c727c,0,0);
  FUN_004e6d60(DAT_008c727c,5,0);
  FUN_004d3c7c(DAT_008c727c,FUN_00467f74,1,0);
  FUN_004d3c7c(DAT_008c727c,FUN_00467f74,2,0);
  FUN_004d3c7c(DAT_008c727c,FUN_00467f74,8,0);
  FUN_004e04b8(DAT_008c727c,0);
  thunk_FUN_00952500(DAT_008c727c);
  FUN_004f3160(DAT_008c727c,1);
  FUN_004f31a4(DAT_008c727c,2,0,2);
  uVar1 = FUN_004d992c(DAT_008c727c);
  FUN_004db8f0(uVar1,0x140,0x28);
  FUN_004e6e70(uVar1,0,0);
  FUN_004e6e50(uVar1,0xff000000,0);
  FUN_004e6d90(uVar1,0,0);
  FUN_004e04b8(uVar1,0);
  FUN_004e0518(uVar1,0);
  uVar1 = FUN_0046f51c();
  iVar2 = FUN_00428900("/usr/project/config/ui/set_menu/audio_setting.json",uVar1);
  if (iVar2 == 0) {
    FUN_0045a5bc(DAT_008c727c,&DAT_0068ba8c,0xfe0156);
    DAT_008c7268 = FUN_0045a33c(DAT_008c727c,&DAT_0068e910,0,&DAT_00467fd0);
    DAT_008c726c = FUN_0045a33c(DAT_008c727c,&DAT_0068e920,1,&DAT_00467fd0);
    DAT_008c7270 = FUN_0045a33c(DAT_008c727c,&DAT_0068e930,2,&DAT_00467fd0);
    DAT_008c7274 = FUN_0045a33c(DAT_008c727c,&DAT_0068e940,3,&DAT_00467fd0);
    puVar3 = &DAT_0068e950;
  }
  else {
    FUN_0045a5bc(DAT_008c727c,**(undefined4 **)(iVar2 + 8),0xfe0156);
    DAT_008c7268 = FUN_0045a33c(DAT_008c727c,*(undefined4 *)(*(int *)(iVar2 + 8) + 0xc),0,
                                &DAT_00467fd0);
    DAT_008c726c = FUN_0045a33c(DAT_008c727c,*(undefined4 *)(*(int *)(iVar2 + 8) + 0x10),1,
                                &DAT_00467fd0);
    DAT_008c7270 = FUN_0045a33c(DAT_008c727c,*(undefined4 *)(*(int *)(iVar2 + 8) + 0x14),2,
                                &DAT_00467fd0);
    DAT_008c7274 = FUN_0045a33c(DAT_008c727c,*(undefined4 *)(*(int *)(iVar2 + 8) + 0x18),3,
                                &DAT_00467fd0);
    puVar3 = *(undefined **)(*(int *)(iVar2 + 8) + 0x1c);
  }
  DAT_008c7278 = FUN_0045a33c(DAT_008c727c,puVar3,4,&DAT_00467fd0);
  FUN_00428dd0(iVar2);
  uVar1 = FUN_004d992c(DAT_008c727c);
  FUN_004db8f0(uVar1,0x140,0x28);
  FUN_004e6e70(uVar1,0,0);
  FUN_004e6e50(uVar1,0xff000000,0);
  FUN_004e6d90(uVar1,0,0);
  FUN_004e04b8(uVar1,0);
  FUN_004e0518(uVar1,0);
  FUN_004d9960(uVar1,0x4000);
  FUN_0045a578((&DAT_008c7268)[DAT_008c58fc],1);
  return;
}

