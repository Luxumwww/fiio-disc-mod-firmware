
void FUN_00462db0(int param_1)

{
  if (param_1 != 0) {
    FUN_0043d464();
    FUN_004428bc(&DAT_006833d8,8,0);
    DAT_008c70ec = 0;
    thunk_FUN_00953000(0);
    return;
  }
  FUN_0046c770();
  thunk_FUN_00953000(1);
  if (DAT_008c70f8 == 0) {
    FUN_00462580();
  }
  thunk_FUN_00952500(DAT_008c70f8);
  return;
}

