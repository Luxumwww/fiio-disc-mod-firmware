
void FUN_00433388(void)

{
  int iVar1;
  undefined4 uVar2;
  undefined4 uVar3;
  undefined4 *puVar4;
  char cVar5;
  
  DAT_008c593c = FUN_004d992c(0);
  FUN_004db8f0(DAT_008c593c,0x168,0x168);
  FUN_004dbe7c(DAT_008c593c,9);
  FUN_004d9a88(DAT_008c593c,0x10);
  FUN_004d9960(DAT_008c593c,8);
  FUN_004e6d80(DAT_008c593c,0xff8c8c8c,0);
  FUN_004e6d90(DAT_008c593c,0xff,0);
  FUN_004e6e50(DAT_008c593c,0xff000000,0);
  FUN_004e6e60(DAT_008c593c,0,0);
  FUN_004e7090(DAT_008c593c,0,0);
  FUN_004e6e70(DAT_008c593c,0,0);
  uVar2 = DAT_008c593c;
  FUN_004e6d40(DAT_008c593c,0,0);
  FUN_004e6d50(uVar2,0,0);
  FUN_004e6d20(uVar2,0,0);
  FUN_004e6d30(uVar2,0,0);
  thunk_FUN_00952500(DAT_008c593c);
  DAT_008c5938 = FUN_004323b4(DAT_008c593c);
  FUN_004d9960(DAT_008c5938,0x4000);
  DAT_008c58b8 = FUN_0051af10(DAT_008c5938);
  FUN_004dbe84(DAT_008c58b8,9,0,0);
  FUN_004d9a88(DAT_008c58b8,0x10);
  FUN_0051af44(DAT_008c58b8,"D:/usr/project/config/img/main_screen/main_1/local_play_bg.png");
  iVar1 = FUN_00467f0c();
  if (iVar1 == 0) {
    FUN_004db8f0(DAT_008c58b8,0x16c,0x16c);
  }
  else {
    FUN_004db8f0(DAT_008c58b8,0x168,0x168);
  }
  FUN_004db8f0(DAT_008c58b8,0x16c,0x16c);
  FUN_004e6f40(DAT_008c58b8,0xff,0);
  DAT_008c592c = FUN_004d992c(DAT_008c5938);
  FUN_004db8f0(DAT_008c592c,0x168,0x168);
  FUN_004dbe84(DAT_008c592c,9,0,0);
  FUN_004d9a88(DAT_008c592c,0x10);
  FUN_004e6d90(DAT_008c592c,0,0);
  FUN_004e6e70(DAT_008c592c,0,0);
  uVar2 = FUN_004d992c(DAT_008c592c);
  FUN_004db9cc(uVar2,0x168);
  FUN_004dba4c(uVar2,0x168);
  FUN_004dbe84(uVar2,9,0,0);
  FUN_004e6d80(uVar2,0xff000000,0);
  FUN_004e6d90(uVar2,0x7f,0);
  FUN_004e6d40(uVar2,0,0);
  FUN_004e6d50(uVar2,0,0);
  FUN_004e6d20(uVar2,0,0);
  FUN_004e6d30(uVar2,0,0);
  FUN_004e6e70(uVar2,0,0);
  FUN_004d9960(uVar2,0x4000);
  DAT_008c5924 = FUN_0051c364(DAT_008c592c);
  uVar2 = FUN_0046f51c();
  iVar1 = FUN_00428900("/usr/project/config/ui/set_menu/others.json",uVar2);
  if (iVar1 == 0) {
    thunk_FUN_0097a000(DAT_008c5924,&DAT_0068a3a0);
  }
  else {
    thunk_FUN_0097a000(DAT_008c5924,*(undefined4 *)(*(int *)(iVar1 + 8) + 0x38));
  }
  FUN_00428dd0(iVar1);
  FUN_004e4820(DAT_008c5924,&_fbss,0);
  FUN_004e7020(DAT_008c5924,0xffffffff,0);
  FUN_004dbe84(DAT_008c5924,9,0,0);
  FUN_00458970(DAT_008c592c);
  FUN_004d9960(DAT_008c592c,1);
  DAT_008c5930 = FUN_004d992c(DAT_008c5938);
  FUN_004db8f0(DAT_008c5930,0x168,0x168);
  FUN_004dbe84(DAT_008c5930,5,0,0);
  FUN_004d9a88(DAT_008c5930,0x10);
  FUN_004e6d90(DAT_008c5930,0,0);
  FUN_004e6e70(DAT_008c5930,0,0);
  DAT_008c5914 = 0;
  uVar2 = FUN_00467f0c();
  FUN_00431e74(uVar2);
  DAT_008c58f4 = FUN_004d992c(DAT_008c58b8);
  FUN_004db9cc(DAT_008c58f4,0x168);
  FUN_004dba4c(DAT_008c58f4,0x168);
  FUN_004dbe84(DAT_008c58f4,9,0,0);
  FUN_004e6d80(DAT_008c58f4,0xff000000,0);
  FUN_004e6d90(DAT_008c58f4,0xb2,0);
  uVar2 = DAT_008c58f4;
  FUN_004e6d40(DAT_008c58f4,0,0);
  FUN_004e6d50(uVar2,0,0);
  FUN_004e6d20(uVar2,0,0);
  FUN_004e6d30(uVar2,0,0);
  FUN_004e6e70(DAT_008c58f4,0,0);
  FUN_004324bc(DAT_008c5938,&DAT_008c58bc,&DAT_008c505c,&DAT_008c5054);
  iVar1 = 0xa3;
  FUN_00432b68(DAT_008c5938);
  FUN_00432cc8(DAT_008c5930);
  uVar2 = DAT_008c593c;
  puVar4 = &DAT_008c5034;
  cVar5 = '\0';
  do {
    uVar3 = FUN_004d992c(uVar2);
    *puVar4 = uVar3;
    FUN_004db8f0(uVar3,5,5);
    FUN_004e7090(*puVar4,0x7fff,0);
    FUN_004e6d80(*puVar4,0xff696969,0);
    if (cVar5 == '\0') {
      FUN_004e6d80(DAT_008c5034,0xffffffff,0);
      FUN_004e6d90(*puVar4,0xff,0);
      FUN_004e6e70(*puVar4,0,0);
      FUN_004dbe84(*puVar4,1,iVar1,0x14e);
    }
    else {
      FUN_004e6d90(*puVar4,0xff);
      FUN_004e6e70(*puVar4,0,0);
      FUN_004dbe84(*puVar4,1,iVar1,0x14e);
      if ((char)(cVar5 + '\x01') == '\x04') {
        FUN_004d9960(DAT_008c5928,0x4000);
        FUN_004d9960(DAT_008c58b8,0x4000);
        FUN_004d9960(DAT_008c58f4,0x4000);
        iVar1 = FUN_00467f0c();
        if (iVar1 == 0) {
          FUN_0043040c();
          FUN_0051af44(DAT_008c58b8,
                       "D:/usr/project/config/img/main_screen/main_1/theme_local_play_1.png");
        }
        else {
          FUN_0051af44(DAT_008c58b8,"D:/usr/project/config/img/main_screen/main_1/local_play_bg.png"
                      );
        }
        FUN_00431964(DAT_008c58b8,DAT_008c5928,DAT_008c5098);
        FUN_004d3c7c(DAT_008c58b8,&LAB_00431184,1,0);
        FUN_004d3c7c(DAT_008c58b8,&LAB_00431184,7,0);
        FUN_004d3c7c(DAT_008c58b8,&LAB_00431184,8,0);
        FUN_004d3c7c(DAT_008c5934,&LAB_00431184,1,0);
        FUN_004d3c7c(DAT_008c5934,&LAB_00431184,7,0);
        FUN_004d3c7c(DAT_008c5934,&LAB_00431184,8,0);
        FUN_004d3c7c(DAT_008c5930,&LAB_00431184,1,0);
        FUN_004d3c7c(DAT_008c5930,&LAB_00431184,7,0);
        FUN_004d3c7c(DAT_008c5930,&LAB_00431184,8,0);
        FUN_004d3c7c(DAT_008c5930,&LAB_00431184,2,0);
        FUN_004d3c7c(DAT_008c592c,FUN_004308fc,0,0);
        return;
      }
    }
    puVar4 = puVar4 + 1;
    iVar1 = iVar1 + 10;
    cVar5 = cVar5 + '\x01';
  } while( true );
}

