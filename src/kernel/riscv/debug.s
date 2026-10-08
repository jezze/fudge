.section .text

.globl debug_fmt
debug_fmt:
    addi sp, sp, -48
    sd ra, 8(sp)
    sd a4, 16(sp)
    sd a5, 24(sp)
    sd a6, 32(sp)
    sd a7, 40(sp)
    addi a4, sp, 16
    call debug_fmtv
    ld ra, 8(sp)
    addi sp, sp, 48
    ret
