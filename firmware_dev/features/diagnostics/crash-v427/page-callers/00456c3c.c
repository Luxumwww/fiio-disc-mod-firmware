
void FUN_00456c3c(undefined4 param_1)

{
  undefined4 uVar1;
  undefined *puVar2;
  undefined4 *puVar3;
  undefined4 *puVar4;
  undefined **ppuVar5;
  
  DAT_00826078 = param_1;
  DAT_008c69d0 = FUN_004d992c(0);
  FUN_004d9a88(DAT_008c69d0,0x10);
  FUN_004e6d80(DAT_008c69d0,0xff000000,0);
  FUN_004e6d90(DAT_008c69d0,0xff,0);
  puVar3 = &DAT_008c8a9c;
  ppuVar5 = &PTR_s_D__usr_project_config_img_main_s_00825d60;
  do {
    puVar4 = puVar3 + 1;
    uVar1 = FUN_0051af10(DAT_008c69d0);
    puVar2 = *ppuVar5;
    *puVar3 = uVar1;
    FUN_0051af44(uVar1,puVar2);
    FUN_004dbe84(*puVar3,9,0,0);
    FUN_004d9960(*puVar3,1);
    puVar3 = puVar4;
    ppuVar5 = ppuVar5 + 1;
  } while (puVar4 != &DAT_008c8ac8);
  FUN_004d9a88(DAT_008c8a9c,1);
  DAT_008c69cd = 1;
  FUN_004d3c7c(DAT_008c69d0,&LAB_00455be0,7,0);
  FUN_004d3c7c(DAT_008c69d0,&LAB_00455be0,0xc,0);
  thunk_FUN_00952500(DAT_008c69d0);
  return;
}

