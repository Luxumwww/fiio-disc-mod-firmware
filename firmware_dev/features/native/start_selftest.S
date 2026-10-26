.set noreorder
.text
.globl _start
.ent _start
_start:
  jal disc_native_selftest
  nop
  move $4,$2
  li $2,4001
  syscall
  nop
.end _start
