
void FUN_00957600(void)

{
  undefined4 uVar1;
  undefined1 *puVar2;
  undefined4 uVar3;
  int unaff_s0;
  
  uVar1 = FUN_004d992c(*(undefined4 *)(unaff_s0 + 0x5b10));
  FUN_004db8f0(uVar1,0x168,0x3c);
  FUN_004e6d80(uVar1,0xffffffff,0);
  FUN_004e6d90(uVar1,0,0);
  FUN_004e6e70(uVar1,0,0);
  FUN_004e70b0(uVar1,0,0x10000);
  FUN_004d9a88(uVar1,0x10);
  FUN_004e6d90(uVar1,0x4c,0x20);
  FUN_004e6d80(uVar1,0xffaaaaaa,0x20);
  puVar2 = malloc(1);
  *puVar2 = 7;
  FUN_004d3c7c(uVar1,&DAT_0043d388,7,puVar2);
  FUN_004d3c7c(uVar1,&LAB_0043d330,0x21,puVar2);
  FUN_004d9960(uVar1,0x4000);
  uVar3 = FUN_0051c364(uVar1);
  thunk_FUN_0097a000(uVar3,&DAT_00681ea0);
  FUN_004dbe84(uVar3,7,0x2c,0);
  FUN_004e7020(uVar3,0xffffffff,0);
  FUN_004e4820(uVar3,&DAT_00826054,0);
  uVar1 = FUN_0051c364(uVar1);
  FUN_004e7020(uVar1,0xffffffff,0);
  FUN_004dbe84(uVar1,7,300,0);
  thunk_FUN_0097a000(uVar1,&DAT_0068ad70);
  FUN_004e7040(uVar1,&PTR_LAB_007bb120,0);
  thunk_FUN_00952500(*(undefined4 *)(unaff_s0 + 0x5b10));
  FUN_0046c770();
  return;
}

