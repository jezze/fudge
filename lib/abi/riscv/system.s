.section .text

.globl system_run
system_run:
    addi sp, sp, -64
    sd ra, 8(sp)
    sd a2, 16(sp)
    sd a3, 24(sp)
    sd a4, 32(sp)
    sd a5, 40(sp)
    sd a6, 48(sp)
    sd a7, 56(sp)
    addi a2, sp, 16
    call system_runv
    ld ra, 8(sp)
    addi sp, sp, 64
    ret
