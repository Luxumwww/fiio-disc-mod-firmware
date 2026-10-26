
void FUN_00456388(undefined4 param_1)

{
  undefined4 uVar1;
  undefined4 *puVar2;
  uint uVar3;
  
  DAT_00826078 = param_1;
  DAT_008c6e54 = FUN_004d992c(0);
  FUN_004db8f0(DAT_008c6e54,0x168,0x168);
  FUN_004dbe84(DAT_008c6e54,9,0,0);
  FUN_004e6e70(DAT_008c6e54,0,0);
  uVar1 = DAT_008c6e54;
  FUN_004e6d40(DAT_008c6e54,0,0);
  FUN_004e6d50(uVar1,0,0);
  FUN_004e6d20(uVar1,0,0);
  FUN_004e6d30(uVar1,0,0);
  uVar3 = 0;
  FUN_004d9a88(DAT_008c6e54,0x10);
  puVar2 = &DAT_008c6e00;
  do {
    uVar1 = FUN_004d992c(DAT_008c6e54);
    *puVar2 = uVar1;
    FUN_004db8f0(uVar1,0x5a,0x5a);
    FUN_004e6d80(*puVar2,0xff000000,0);
    FUN_004e6d90(*puVar2,0xff,0);
    uVar1 = *puVar2;
    FUN_004e6d40(uVar1,0,0);
    FUN_004e6d50(uVar1,0,0);
    FUN_004e6d20(uVar1,0,0);
    FUN_004e6d30(uVar1,0,0);
    FUN_004e7090(*puVar2,0,0);
    FUN_004dbe84(*puVar2,1,(uVar3 & 3) * 0x5a,((int)uVar3 >> 2) * 0x5a);
    uVar3 = uVar3 + 1;
    FUN_004d3c7c(*puVar2,&LAB_00455734,0,0);
    puVar2 = puVar2 + 1;
  } while (uVar3 != 0x10);
  thunk_FUN_00952500();
  return;
}

