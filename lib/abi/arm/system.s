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

.global system_feed
.type system_feed, %function
system_feed:
    push {r4, lr}
    ldr r4, [sp, #8]
    add r12, sp, #12
    push {r4, r12}
    bl system_feedv
    add sp, sp, #8
    pop {r4, lr}
    bx lr
