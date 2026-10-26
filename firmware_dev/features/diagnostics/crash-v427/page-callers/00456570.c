
void FUN_00456570(undefined4 param_1)

{
  undefined4 uVar1;
  FILE *pFVar2;
  char *pcVar3;
  char acStack_118 [256];
  
  DAT_008c6df4 = param_1;
  DAT_008c6dfc = FUN_004d992c(0);
  FUN_004db8f0(DAT_008c6dfc,0x168,0x348);
  uVar1 = DAT_008c6dfc;
  FUN_004e6d40(DAT_008c6dfc,0,0);
  FUN_004e6d50(uVar1,0,0);
  FUN_004e6d20(uVar1,0,0);
  FUN_004e6d30(uVar1,0,0);
  FUN_004e6e70(DAT_008c6dfc,0,0);
  FUN_004e6d80(DAT_008c6dfc,0xff000000,0);
  FUN_004dbe84(DAT_008c6dfc,1,0,0);
  FUN_004e7090(DAT_008c6dfc,0,0);
  FUN_004e6d60(DAT_008c6dfc,5,0);
  FUN_004e04b8(DAT_008c6dfc,0);
  FUN_004d3c7c(DAT_008c6dfc,FUN_00455b4c,7,0);
  thunk_FUN_00952500(DAT_008c6dfc);
  uVar1 = FUN_0051c364(DAT_008c6dfc);
  thunk_FUN_0097a000(uVar1,&DAT_0068cfc0);
  FUN_004dbe84(uVar1,2,0,0x10);
  FUN_004e7020(uVar1,0xfffe0156,0);
  FUN_004e4820(uVar1,&DAT_00826054,0);
  uVar1 = FUN_0051c364(DAT_008c6dfc);
  FUN_004dbe84(uVar1,1,0x3c,0x3c);
  FUN_004e7020(uVar1,0xffffffff,0);
  FUN_004e4820(uVar1,&DAT_00826054,0);
  pFVar2 = popen("wpa_cli -i wlan0 status | grep -E \'^ssid=\' ","r");
  pcVar3 = fgets(acStack_118,0x100,pFVar2);
  if (pcVar3 != (char *)0x0) {
    FUN_0051d4b0(uVar1,acStack_118);
  }
  pclose(pFVar2);
  DAT_008c6df0 = FUN_0051c364(DAT_008c6dfc);
  FUN_004dbe84(DAT_008c6df0,1,0x3c,0x5a);
  FUN_004e7020(DAT_008c6df0,0xffffffff,0);
  FUN_004e4820(DAT_008c6df0,&DAT_00826054,0);
  uVar1 = FUN_0051c364(DAT_008c6dfc);
  FUN_004dbe84(uVar1,1,0x3c,0x78);
  FUN_004e7020(uVar1,0xffffffff,0);
  FUN_004e4820(uVar1,&DAT_00826054,0);
  pFVar2 = popen("wpa_cli -i wlan0 status | grep -E \'^bssid=\' ","r");
  pcVar3 = fgets(acStack_118,0x100,pFVar2);
  if (pcVar3 != (char *)0x0) {
    FUN_0051d4b0(uVar1,acStack_118);
  }
  pclose(pFVar2);
  uVar1 = FUN_0051c364(DAT_008c6dfc);
  FUN_004dbe84(uVar1,1,0x3c,0x96);
  FUN_004e7020(uVar1,0xffffffff,0);
  FUN_004e4820(uVar1,&DAT_00826054,0);
  pFVar2 = popen("wpa_cli -i wlan0 status | grep -E \'^ip_address=\' ","r");
  pcVar3 = fgets(acStack_118,0x100,pFVar2);
  if (pcVar3 != (char *)0x0) {
    FUN_0051d4b0(uVar1,acStack_118);
  }
  pclose(pFVar2);
  uVar1 = FUN_0051c364(DAT_008c6dfc);
  FUN_004dbe84(uVar1,1,0x3c,0xb4);
  FUN_004e7020(uVar1,0xffffffff,0);
  FUN_004e4820(uVar1,&DAT_00826054,0);
  pFVar2 = popen("wpa_cli -i wlan0 status | grep -E \'^key_mgmt=\' ","r");
  pcVar3 = fgets(acStack_118,0x100,pFVar2);
  if (pcVar3 != (char *)0x0) {
    FUN_0051d4b0(uVar1,acStack_118);
  }
  pclose(pFVar2);
  DAT_008c6dec = FUN_0051c364(DAT_008c6dfc);
  FUN_004dbe84(DAT_008c6dec,1,0x3c,0xd2);
  FUN_004e7020(DAT_008c6dec,0xffffffff,0);
  FUN_004e4820(DAT_008c6dec,&DAT_00826054,0);
  thunk_FUN_0097a000(DAT_008c6dec,"wait_connect");
  DAT_008c6df8 = FUN_00513720(&LAB_00455a98,1000,0);
  uVar1 = FUN_004d992c(DAT_008c6dfc);
  FUN_004db8f0(uVar1,0x78,0x14);
  FUN_004dbe84(uVar1,5,0,0x50);
  FUN_004e6e70(uVar1,0,0);
  FUN_004e6ee0(uVar1,0,0);
  FUN_004e6d90(uVar1,0,0);
  pthread_create((pthread_t *)&DAT_008c6e6c,(pthread_attr_t *)0x0,FUN_00455860,(void *)0x0);
  return;
}

