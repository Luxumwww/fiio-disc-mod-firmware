
void FUN_00470ac8(void)

{
  undefined4 local_10 [2];
  
  if (DAT_008c74be != '\x02') {
    return;
  }
  local_10[0] = FUN_004d3bb4();
  thunk_FUN_00952500(DAT_008c74d8);
  FUN_004e80c8(local_10);
  DAT_008c74be = 0;
  DAT_008c74bd = 0;
  return;
}

