.section .text

.global debug_fmt
debug_fmt:
    stp x29, x30, [sp, #-48]!
    stp x4, x5, [sp, #16]
    stp x6, x7, [sp, #32]
    add x4, sp, #16
    bl debug_fmtv
    ldp x29, x30, [sp], #48
    ret

