
void FUN_004709c4(undefined4 param_1)

{
  undefined4 local_18 [2];
  
  if (DAT_008c74a0 != '\x02') {
    return;
  }
  if (DAT_008c74b1 == '\0') {
    if (DAT_008c8a61 != '\0') {
      FUN_004428bc(&DAT_0068f0c8,0,0);
    }
    local_18[0] = FUN_004d3bb4(param_1);
    thunk_FUN_00952500(DAT_008c74d8);
    FUN_004e80c8(local_18);
    FUN_004428bc(&DAT_0068f0d0,0,0);
    DAT_008c74a0 = 0;
    DAT_008c74bd = 0;
    return;
  }
  FUN_004d9960(0,1);
  FUN_004d9a88(0,1);
  DAT_008c74a0 = 0;
  DAT_008c74b1 = 0;
  return;
}

