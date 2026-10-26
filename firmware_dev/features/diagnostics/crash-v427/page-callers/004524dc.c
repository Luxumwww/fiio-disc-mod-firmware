
void FUN_004524dc(undefined4 param_1,undefined4 param_2,undefined4 param_3,undefined4 param_4,
                 undefined4 param_5)

{
  undefined1 uVar1;
  
  FUN_00451ee0();
  thunk_FUN_00952500(DAT_008c68fc);
  FUN_00465790(FUN_00451ce4);
  uVar1 = (undefined1)param_2;
  DAT_008c6910 = param_4;
  switch(param_2) {
  case 1:
    FUN_0045217c(1,param_3,param_1,0);
    DAT_008c68b0 = uVar1;
    return;
  case 2:
    FUN_0045217c(2,param_3,param_1,0);
    DAT_008c68b0 = uVar1;
    return;
  case 3:
    FUN_0045217c(3,param_3,param_1,0);
    DAT_008c68b0 = uVar1;
    return;
  case 6:
    FUN_0045217c(6,param_3,param_1,param_5);
    DAT_008c68b0 = uVar1;
    return;
  case 7:
    FUN_0045217c(7,param_3,param_1,param_5);
  }
  DAT_008c68b0 = uVar1;
  return;
}

