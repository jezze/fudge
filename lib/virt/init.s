.set STACK_SIZE,                        0x4000

.section .text.boot

# OpenSBI enters in supervisor mode with a0 = hart id and a1 = device tree
.global _start
_start:
    csrw sie, zero
    la sp, stack
    li t0, STACK_SIZE
    add sp, sp, t0
    call virt_setup

halt:
    wfi
    j halt

.section .bss

.balign 16
stack:
    .space STACK_SIZE

