.section .text

.globl system_run
system_run:
    stp x29, x30, [sp, #-64]!
    stp x2, x3, [sp, #16]
    stp x4, x5, [sp, #32]
    stp x6, x7, [sp, #48]
    add x2, sp, #16
    bl system_runv
    ldp x29, x30, [sp], #64
    ret

.globl system_feed
system_feed:
    stp x29, x30, [sp, #-48]!
    str x5, [sp, #24]
    stp x6, x7, [sp, #32]
    add x5, sp, #24
    bl system_feedv
    ldp x29, x30, [sp], #48
    ret

