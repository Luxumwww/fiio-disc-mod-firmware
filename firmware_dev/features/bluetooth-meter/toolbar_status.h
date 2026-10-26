/* Preserve the stock global status bar across direct screen restoration.
 * Native hide/show helpers operate on 0x8de408; never rebuild its widgets. */
typedef struct {LvObj *bar;unsigned hidden,mode;} DiscToolbarStatus;
static DiscToolbarStatus disc_toolbar_capture(void){
 DiscToolbarStatus saved={LV_GLOBAL(0x8de408),0,*(unsigned *)0x8e16d4};
 if(saved.bar&&lv_valid(saved.bar))saved.hidden=*(unsigned *)((char *)saved.bar+36)&1;
 else saved.bar=0;
 return saved;
}
static void disc_toolbar_restore(const DiscToolbarStatus *saved){
 LvObj *bar=LV_GLOBAL(0x8de408);
 if(!saved->bar||bar!=saved->bar||!lv_valid(bar)||saved->mode!=*(unsigned *)0x8e16d4)return;
 if((*(unsigned *)((char *)bar+36)&1)!=saved->hidden)
  FN(saved->hidden?0x479090:0x4790b0,void (*)(void))();
}
static DiscToolbarStatus eq_toolbar_status;
