.section .text

.globl channel_send_fmt
channel_send_fmt:
    addi sp, sp, -48
    sd ra, 8(sp)
    sd a4, 16(sp)
    sd a5, 24(sp)
    sd a6, 32(sp)
    sd a7, 40(sp)
    addi a4, sp, 16
    call channel_send_fmtv
    ld ra, 8(sp)
    addi sp, sp, 48
    ret
