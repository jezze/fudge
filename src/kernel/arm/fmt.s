.section .text

.global debug_fmt
.type debug_fmt, %function
debug_fmt:
    mov r12, sp
    push {r12, lr}
    bl debug_fmtv
    pop {r12, pc}
