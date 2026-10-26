
void FUN_00456114(undefined4 param_1)

{
  undefined4 uVar1;
  undefined *puVar2;
  undefined4 *puVar3;
  undefined4 *puVar4;
  undefined **ppuVar5;
  
  DAT_00826078 = param_1;
  DAT_008c6e64 = FUN_004d992c(0);
  FUN_004d9a88(DAT_008c6e64,0x10);
  FUN_004e6d80(DAT_008c6e64,0xff000000,0);
  FUN_004e6d90(DAT_008c6e64,0xff,0);
  puVar3 = &DAT_008c8ac8;
  ppuVar5 = &PTR_s_D__usr_project_config_img_main_s_00825d8c;
  do {
    puVar4 = puVar3 + 1;
    uVar1 = FUN_0051af10(DAT_008c6e64);
    puVar2 = *ppuVar5;
    *puVar3 = uVar1;
    FUN_0051af44(uVar1,puVar2);
    FUN_004db8f0(*puVar3,0x168,0x168);
    FUN_0051c0ac(*puVar3,0x100);
    FUN_004dbe84(*puVar3,9,0,0);
    FUN_004d9960(*puVar3,1);
    puVar3 = puVar4;
    ppuVar5 = ppuVar5 + 1;
  } while (puVar4 != (undefined4 *)0x8c8af0);
  FUN_004d9a88(DAT_008c8ac8,1);
  DAT_008c6e50 = 1;
  DAT_0082607c = FUN_00513720(&LAB_00455490,1000,DAT_008c6e64);
  FUN_004d3c7c(DAT_008c6e64,FUN_00455518,7,0);
  thunk_FUN_00952500(DAT_008c6e64);
  return;
}

