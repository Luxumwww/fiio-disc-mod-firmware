
void FUN_00434c30(undefined4 param_1)

{
  FUN_0046c770();
  if ((DAT_008c8a8e != '\0') && (DAT_008c8a8e != '\x01')) {
    if (DAT_008c8a8e != '\x02') {
      return;
    }
    FUN_004460b8(param_1);
    return;
  }
  DAT_008c5908 = param_1;
  if (DAT_008c593c != 0) {
    thunk_FUN_00952500();
    FUN_00430114();
    return;
  }
  FUN_00433388();
  FUN_00430114();
  return;
}

