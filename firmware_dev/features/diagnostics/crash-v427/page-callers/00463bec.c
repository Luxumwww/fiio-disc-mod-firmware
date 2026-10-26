
void FUN_00463bec(code *param_1)

{
  undefined4 uVar1;
  int iVar2;
  undefined *puVar3;
  
  if (param_1 == (code *)0x0) {
    param_1 = FUN_0044a478;
  }
  DAT_008c7134 = param_1;
  if (DAT_008c7190 != 0) {
    thunk_FUN_00952500(DAT_008c7190);
    return;
  }
  DAT_008c7190 = FUN_004d992c(0);
  FUN_004db8f0(DAT_008c7190,0x168,0x168);
  uVar1 = DAT_008c7190;
  FUN_004e6d40(DAT_008c7190,0,0);
  FUN_004e6d50(uVar1,0,0);
  FUN_004e6d20(uVar1,0,0);
  FUN_004e6d30(uVar1,0,0);
  FUN_004e6e70(DAT_008c7190,0,0);
  FUN_004e6d80(DAT_008c7190,0xff000000,0);
  FUN_004dbe84(DAT_008c7190,1,0,0);
  FUN_004e7090(DAT_008c7190,0,0);
  FUN_004e6d60(DAT_008c7190,5,0);
  FUN_004e04b8(DAT_008c7190,0);
  FUN_004d3c7c(DAT_008c7190,FUN_00463660,1,0);
  FUN_004d3c7c(DAT_008c7190,FUN_00463660,2,0);
  FUN_004d3c7c(DAT_008c7190,FUN_00463660,8,0);
  thunk_FUN_00952500(DAT_008c7190);
  FUN_004f3160(DAT_008c7190,1);
  FUN_004f31a4(DAT_008c7190,0,0,2);
  uVar1 = FUN_0046f51c();
  iVar2 = FUN_00428900("/usr/project/config/ui/set_menu/audio_setting.json",uVar1);
  if (iVar2 == 0) {
    FUN_0045a5bc(DAT_008c7190,&DAT_0068bd88,0xe67b10);
  }
  else {
    FUN_0045a5bc(DAT_008c7190,*(undefined4 *)(*(int *)(iVar2 + 8) + 0x48),0xe67b10);
  }
  FUN_00428dd0(iVar2);
  uVar1 = FUN_0046f51c();
  iVar2 = FUN_00428900("/usr/project/config/ui/set_menu/equalizer.json",uVar1);
  if (iVar2 == 0) {
    DAT_008c7138 = FUN_0045a33c(DAT_008c7190,&DAT_0068e258,0,FUN_004636bc);
    DAT_008c713c = FUN_0045a33c(DAT_008c7190,&DAT_0068e25c,1,FUN_004636bc);
    DAT_008c7140 = FUN_0045a33c(DAT_008c7190,&DAT_0068e264,2,FUN_004636bc);
    DAT_008c7144 = FUN_0045a33c(DAT_008c7190,&DAT_0068e26c,3,FUN_004636bc);
    DAT_008c7148 = FUN_0045a33c(DAT_008c7190,"Hip-Hop",4,FUN_004636bc);
    DAT_008c714c = FUN_0045a33c(DAT_008c7190,&DAT_0068e278,5,FUN_004636bc);
    DAT_008c7150 = FUN_0045a33c(DAT_008c7190,&DAT_0068e280,6,FUN_004636bc);
    DAT_008c7154 = FUN_0045a33c(DAT_008c7190,&DAT_0068e288,7,FUN_004636bc);
    DAT_008c7158 = FUN_0045a33c(DAT_008c7190,&DAT_0068e290,8,FUN_004636bc);
    DAT_008c715c = FUN_0045a33c(DAT_008c7190,&DAT_0068e298,9,FUN_004636bc);
    DAT_008c7160 = FUN_0045a33c(DAT_008c7190,&DAT_0068e2a8,10,FUN_004636bc);
    DAT_008c7164 = FUN_0045a33c(DAT_008c7190,&DAT_0068e2b8,0xb,FUN_004636bc);
    DAT_008c7168 = FUN_0045a33c(DAT_008c7190,&DAT_0068e2c4,0xc,FUN_004636bc);
    DAT_008c716c = FUN_0045a33c(DAT_008c7190,&DAT_0068e2d0,0xd,FUN_004636bc);
    DAT_008c7170 = FUN_0045a33c(DAT_008c7190,&DAT_0068e2dc,0xe,FUN_004636bc);
    DAT_008c7174 = FUN_0045a33c(DAT_008c7190,&DAT_0068e2e8,0xf,FUN_004636bc);
    DAT_008c7178 = FUN_0045a33c(DAT_008c7190,&DAT_0068e2f4,0x10,FUN_004636bc);
    DAT_008c717c = FUN_0045a33c(DAT_008c7190,&DAT_0068e300,0x11,FUN_004636bc);
    DAT_008c7180 = FUN_0045a33c(DAT_008c7190,&DAT_0068e30c,0x12,FUN_004636bc);
    DAT_008c7184 = FUN_0045a33c(DAT_008c7190,&DAT_0068e318,0x13,FUN_004636bc);
    puVar3 = &DAT_0068e324;
  }
  else {
    DAT_008c7138 = FUN_0045a33c(DAT_008c7190,**(undefined4 **)(iVar2 + 8),0,FUN_004636bc);
    DAT_008c713c = FUN_0045a33c(DAT_008c7190,*(undefined4 *)(*(int *)(iVar2 + 8) + 4),1,FUN_004636bc
                               );
    DAT_008c7140 = FUN_0045a33c(DAT_008c7190,*(undefined4 *)(*(int *)(iVar2 + 8) + 8),2,FUN_004636bc
                               );
    DAT_008c7144 = FUN_0045a33c(DAT_008c7190,*(undefined4 *)(*(int *)(iVar2 + 8) + 0xc),3,
                                FUN_004636bc);
    DAT_008c7148 = FUN_0045a33c(DAT_008c7190,*(undefined4 *)(*(int *)(iVar2 + 8) + 0x10),4,
                                FUN_004636bc);
    DAT_008c714c = FUN_0045a33c(DAT_008c7190,*(undefined4 *)(*(int *)(iVar2 + 8) + 0x14),5,
                                FUN_004636bc);
    DAT_008c7150 = FUN_0045a33c(DAT_008c7190,*(undefined4 *)(*(int *)(iVar2 + 8) + 0x18),6,
                                FUN_004636bc);
    DAT_008c7154 = FUN_0045a33c(DAT_008c7190,*(undefined4 *)(*(int *)(iVar2 + 8) + 0x1c),7,
                                FUN_004636bc);
    DAT_008c7158 = FUN_0045a33c(DAT_008c7190,*(undefined4 *)(*(int *)(iVar2 + 8) + 0x20),8,
                                FUN_004636bc);
    DAT_008c715c = FUN_0045a33c(DAT_008c7190,*(undefined4 *)(*(int *)(iVar2 + 8) + 0x24),9,
                                FUN_004636bc);
    DAT_008c7160 = FUN_0045a33c(DAT_008c7190,*(undefined4 *)(*(int *)(iVar2 + 8) + 0x28),10,
                                FUN_004636bc);
    DAT_008c7164 = FUN_0045a33c(DAT_008c7190,*(undefined4 *)(*(int *)(iVar2 + 8) + 0x2c),0xb,
                                FUN_004636bc);
    DAT_008c7168 = FUN_0045a33c(DAT_008c7190,*(undefined4 *)(*(int *)(iVar2 + 8) + 0x30),0xc,
                                FUN_004636bc);
    DAT_008c716c = FUN_0045a33c(DAT_008c7190,*(undefined4 *)(*(int *)(iVar2 + 8) + 0x34),0xd,
                                FUN_004636bc);
    DAT_008c7170 = FUN_0045a33c(DAT_008c7190,*(undefined4 *)(*(int *)(iVar2 + 8) + 0x38),0xe,
                                FUN_004636bc);
    DAT_008c7174 = FUN_0045a33c(DAT_008c7190,*(undefined4 *)(*(int *)(iVar2 + 8) + 0x3c),0xf,
                                FUN_004636bc);
    DAT_008c7178 = FUN_0045a33c(DAT_008c7190,*(undefined4 *)(*(int *)(iVar2 + 8) + 0x40),0x10,
                                FUN_004636bc);
    DAT_008c717c = FUN_0045a33c(DAT_008c7190,*(undefined4 *)(*(int *)(iVar2 + 8) + 0x44),0x11,
                                FUN_004636bc);
    DAT_008c7180 = FUN_0045a33c(DAT_008c7190,*(undefined4 *)(*(int *)(iVar2 + 8) + 0x48),0x12,
                                FUN_004636bc);
    DAT_008c7184 = FUN_0045a33c(DAT_008c7190,*(undefined4 *)(*(int *)(iVar2 + 8) + 0x4c),0x13,
                                FUN_004636bc);
    puVar3 = *(undefined **)(*(int *)(iVar2 + 8) + 0x50);
  }
  DAT_008c7188 = FUN_0045a33c(DAT_008c7190,puVar3,0x14,FUN_004636bc);
  FUN_004d3c7c(DAT_008c7164,&DAT_00463600,5,0xb);
  FUN_004d3c7c(DAT_008c7168,&DAT_00463600,5,0xc);
  FUN_004d3c7c(DAT_008c716c,&DAT_00463600,5,0xd);
  FUN_004d3c7c(DAT_008c7170,&DAT_00463600,5,0xe);
  FUN_004d3c7c(DAT_008c7174,&DAT_00463600,5,0xf);
  FUN_004d3c7c(DAT_008c7178,&DAT_00463600,5,0x10);
  FUN_004d3c7c(DAT_008c717c,&DAT_00463600,5,0x11);
  FUN_004d3c7c(DAT_008c7180,&DAT_00463600,5,0x12);
  FUN_004d3c7c(DAT_008c7184,&DAT_00463600,5,0x13);
  FUN_004d3c7c(DAT_008c7188,&DAT_00463600,5,0x14);
  FUN_00428dd0(iVar2);
  DAT_008c718c = FUN_004d992c(DAT_008c7190);
  FUN_004db8f0(DAT_008c718c,0x168,0x28);
  FUN_004e6e70(DAT_008c718c,0,0);
  FUN_004e6e50(DAT_008c718c,0xff000000,0);
  FUN_004e6d90(DAT_008c718c,0,0);
  FUN_004e04b8(DAT_008c718c,0);
  FUN_004e0518(DAT_008c718c,0);
  FUN_00978c00((&DAT_008c7138)[DAT_008c8a80],1);
  FUN_004e298c((&DAT_008c7138)[DAT_008c8a80 + 1],1);
  return;
}

