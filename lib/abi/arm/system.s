.section .text

.global system_run
.type system_run, %function
system_run:
    push {r2, r3}
    mov r2, sp
    push {r12, lr}
    bl system_runv
    pop {r12, lr}
    add sp, sp, #8
    bx lr
