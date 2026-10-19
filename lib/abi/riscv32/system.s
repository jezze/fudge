.section .text

.globl system_run
system_run:
    addi sp, sp, -32
    sw ra, 4(sp)
    sw a2, 8(sp)
    sw a3, 12(sp)
    sw a4, 16(sp)
    sw a5, 20(sp)
    sw a6, 24(sp)
    sw a7, 28(sp)
    addi a2, sp, 8
    call system_runv
    lw ra, 4(sp)
    addi sp, sp, 32
    ret

.globl system_feed
system_feed:
    addi sp, sp, -16
    sw ra, 0(sp)
    sw a5, 4(sp)
    sw a6, 8(sp)
    sw a7, 12(sp)
    addi a5, sp, 4
    call system_feedv
    lw ra, 0(sp)
    addi sp, sp, 16
    ret

