
void FUN_0044eb88(int param_1)

{
  int iVar1;
  undefined4 uVar2;
  
  iVar1 = FUN_0042b6b0();
  if (iVar1 == 0) {
    if (param_1 == 0) {
      if (DAT_008c6808 == 0) {
        if (DAT_008c8a95 == '\0') {
          DAT_00825d40 = FUN_0046c7b0();
          FUN_0047c870();
          FUN_00442298();
          FUN_0046c770();
          FUN_0044c6d4();
          FUN_0044ced4();
          if (DAT_008c6800 == 0) {
            iVar1 = FUN_0042b6a4();
            if (iVar1 == 0) {
              uVar2 = FUN_0050a7f0();
              DAT_008c6800 = FUN_004d28b4(uVar2);
            }
            else {
              DAT_008c6800 = 0;
            }
          }
          if ((DAT_008c6804 != 0) && (iVar1 = FUN_004d9d00(), iVar1 != 0)) {
            FUN_004e7f34(DAT_008c6804);
          }
          DAT_008c6804 = 0;
          DAT_008c6804 = FUN_004d992c(0);
          FUN_004e4c28(DAT_008c6804,0,0xfffff);
          FUN_004dbe84(DAT_008c6804,9,0,0);
          FUN_004db8f0(DAT_008c6804,0x168,0x168);
          FUN_004e6d80(DAT_008c6804,0xff000000,0);
          FUN_004e6d90(DAT_008c6804,0xff,0);
          thunk_FUN_00952500(DAT_008c6804);
        }
      }
      else {
        if ((DAT_008c6804 != 0) && (iVar1 = FUN_004d9d00(), iVar1 != 0)) {
          thunk_FUN_00952500(DAT_008c6804);
        }
        FUN_0043e370();
        FUN_004e80c8(&DAT_008c6808);
      }
    }
    else {
      FUN_0043e24c();
      FUN_0044ea78();
      FUN_0043e404(DAT_008c6808);
      FUN_0043e908(DAT_008c6808);
      FUN_0043eb5c(DAT_008c6808);
      FUN_0043f4d4(DAT_008c6808);
      FUN_0043e56c();
      FUN_0043e30c();
      thunk_FUN_00952500(DAT_008c6808);
    }
    DAT_008c8a95 = '\x01';
    DAT_008c8a74 = (undefined1)param_1;
  }
  return;
}

