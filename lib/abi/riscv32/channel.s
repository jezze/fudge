.section .text

.globl channel_send_fmt
channel_send_fmt:
    addi sp, sp, -32
    sw ra, 0(sp)
    sw a4, 16(sp)
    sw a5, 20(sp)
    sw a6, 24(sp)
    sw a7, 28(sp)
    addi a4, sp, 16
    call channel_send_fmtv
    lw ra, 0(sp)
    addi sp, sp, 32
    ret

