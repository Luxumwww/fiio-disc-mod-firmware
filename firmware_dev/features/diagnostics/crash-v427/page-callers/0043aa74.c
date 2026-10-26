
void FUN_0043aa74(int param_1)

{
  undefined4 uVar1;
  undefined4 uVar2;
  int iVar3;
  tm tStack_50;
  time_t tStack_24;
  
  puts("creat theme screen");
  DAT_008c5a84 = FUN_004d992c(0);
  FUN_004db8f0(DAT_008c5a84,0x168,0x168);
  uVar1 = DAT_008c5a84;
  FUN_004e6d40(DAT_008c5a84,0,0);
  FUN_004e6d50(uVar1,0,0);
  FUN_004e6d20(uVar1,0,0);
  FUN_004e6d30(uVar1,0,0);
  FUN_004e6e70(DAT_008c5a84,0,0);
  FUN_004e6d80(DAT_008c5a84,0xff000000,0);
  FUN_004dbe84(DAT_008c5a84,1,0,0);
  FUN_004e7090(DAT_008c5a84,0,0);
  FUN_004e6d60(DAT_008c5a84,5,0);
  FUN_004e04b8(DAT_008c5a84,0);
  FUN_004d3c7c(DAT_008c5a84,FUN_00439920,1,0);
  FUN_004d3c7c(DAT_008c5a84,FUN_00439920,2,0);
  FUN_004d3c7c(DAT_008c5a84,FUN_00439920,8,0);
  thunk_FUN_00952500(DAT_008c5a84);
  printf("time_mode=%d\r\n",param_1);
  uVar1 = DAT_008c5a84;
  if (param_1 == 0) {
    FUN_00512540(&DAT_008c5a58);
    FUN_00513294(&DAT_008c5a58,&PTR_LAB_007c2140);
    FUN_00513084(&DAT_008c5a58,0xff000000);
    FUN_00513090(&DAT_008c5a58,0xff);
    FUN_00513138(&DAT_008c5a58,0);
    FUN_005132d0(&DAT_008c5a58,0);
    FUN_0051327c(&DAT_008c5a58,0xff696969);
    FUN_00513054(&DAT_008c5a58,0);
    FUN_00513060(&DAT_008c5a58,0);
    FUN_0051303c(&DAT_008c5a58,0);
    FUN_00513048(&DAT_008c5a58,0);
    FUN_00512540(&DAT_008c5a50);
    FUN_00513294(&DAT_008c5a50,&PTR_LAB_007d3360);
    FUN_00513084(&DAT_008c5a50,0xff000000);
    FUN_00513090(&DAT_008c5a50,0xcc);
    FUN_0051327c(&DAT_008c5a50,0xffffffff);
    FUN_00513138(&DAT_008c5a50,0);
    FUN_005132d0(&DAT_008c5a50,5);
    uVar2 = FUN_0046f51c();
    iVar3 = FUN_00428900("/usr/project/config/ui/set_menu/others.json",uVar2);
    uVar2 = FUN_0051c364(uVar1);
    if (iVar3 == 0) {
      thunk_FUN_0097a000(uVar2,&DAT_0068b08c);
    }
    else {
      thunk_FUN_0097a000(uVar2,*(undefined4 *)(*(int *)(iVar3 + 8) + 8));
    }
    FUN_00428dd0(iVar3);
    FUN_004e4820(uVar2,&DAT_00826054,0);
    FUN_004e7020(uVar2,0xff02a5e5,0);
    FUN_004dbe84(uVar2,2,0,0x10);
    uVar2 = FUN_0051c364(uVar1);
    thunk_FUN_0097a000(uVar2,&DAT_0068df14);
    FUN_004e4820(uVar2,&DAT_008c5a50,0);
    FUN_004e7020(uVar2,0xffffffff,0);
    FUN_004dbe84(uVar2,2,0,0x94);
    iVar3 = FUN_004dc2c0(uVar1);
    DAT_008c5a7c = FUN_0051f42c(uVar1);
    FUN_0051f460(DAT_008c5a7c,
                 "00\n01\n02\n03\n04\n05\n06\n07\n08\n09\n10\n11\n12\n13\n14\n15\n16\n17\n18\n19\n20\n21\n22\n23"
                 ,0);
    iVar3 = (iVar3 + -0xd9) / 2;
    FUN_00520430(DAT_008c5a7c,3);
    FUN_004db9cc(DAT_008c5a7c,0x48);
    FUN_004dba4c(DAT_008c5a7c,200);
    FUN_004e4820(DAT_008c5a7c,&DAT_008c5a58,0);
    FUN_004e4820(DAT_008c5a7c,&DAT_008c5a50,0x40000);
    FUN_004e7080(DAT_008c5a7c,2,0);
    FUN_004dbe84(DAT_008c5a7c,1,iVar3,0x4e);
    FUN_004d3c7c(DAT_008c5a7c,FUN_00439510,0,0);
    FUN_004d9960(DAT_008c5a7c,0x4000);
    DAT_008c5a78 = FUN_0051f42c(uVar1);
    FUN_0051f460(DAT_008c5a78,
                 "00\n01\n02\n03\n04\n05\n06\n07\n08\n09\n10\n11\n12\n13\n14\n15\n16\n17\n18\n19\n20\n21\n22\n23\n24\n25\n26\n27\n28\n29\n30\n31\n32\n33\n34\n35\n36\n37\n38\n39\n40\n41\n42\n43\n44\n45\n46\n47\n48\n49\n50\n51\n52\n53\n54\n55\n56\n57\n58\n59"
                 ,0);
    FUN_00520430(DAT_008c5a78,3);
    FUN_004db9cc(DAT_008c5a78,0x48);
    FUN_004dba4c(DAT_008c5a78,200);
    FUN_004e4820(DAT_008c5a78,&DAT_008c5a58,0);
    FUN_004e4820(DAT_008c5a78,&DAT_008c5a50,0x40000);
    FUN_004e7080(DAT_008c5a78,2,0);
    FUN_004dbe84(DAT_008c5a78,1,iVar3 + 0x91,0x4e);
    FUN_004d3c7c(DAT_008c5a78,FUN_00439510,0,0);
    FUN_004d9960(DAT_008c5a78,0x4000);
    time(&tStack_24);
    localtime_r(&tStack_24,&tStack_50);
    if (DAT_008c5a7c != 0) {
      FUN_0051f728(DAT_008c5a7c,(undefined2)tStack_50.tm_hour,0);
    }
    if (DAT_008c5a78 != 0) {
      FUN_0051f728(DAT_008c5a78,(undefined2)tStack_50.tm_min,0);
    }
    FUN_0042ae68(uVar1,0x78,0x29,2,0,0x12a,0xff2d83ff,0xffffffff,1,&LAB_0043a3d0);
  }
  else if (param_1 == 1) {
    FUN_00439a64(DAT_008c5a84);
  }
  else {
    FUN_0043a74c();
  }
  puts("creat theme screen success");
  return;
}

