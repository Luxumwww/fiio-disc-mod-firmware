
void FUN_00470b4c(undefined4 param_1)

{
  undefined4 *puVar1;
  undefined4 uVar2;
  undefined4 local_18 [3];
  
  puVar1 = (undefined4 *)FUN_004d3bdc();
  FUN_004428bc(&DAT_0068f0f0,0,*puVar1);
  uVar2 = FUN_004d3bb4(param_1);
  local_18[0] = FUN_004e8a68(uVar2);
  thunk_FUN_00952500(DAT_008c74d8);
  FUN_004e80c8(local_18);
  FUN_004e80c8(puVar1 + 4);
  DAT_008c74c1 = DAT_008c74c1 - 1;
  FUN_004db8f0(DAT_008c74c8,0x168,(uint)DAT_008c74c1 * 0x47 + 0x14);
  return;
}

