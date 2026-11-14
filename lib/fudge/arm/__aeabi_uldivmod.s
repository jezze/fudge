.global __aeabi_uldivmod
.align 4

# n in r0:r1 and d in r2:r3; the helper takes the result's address in r0, n in r2:r3 and d on the stack
__aeabi_uldivmod:
    stmfd sp!, {r4, lr}
    sub sp, sp, #24
    str r2, [sp]
    str r3, [sp, #4]
    mov r2, r0
    mov r3, r1
    add r0, sp, #8
    bl __udivmoddi4s
    ldr r0, [sp, #8]
    ldr r1, [sp, #12]
    ldr r2, [sp, #16]
    ldr r3, [sp, #20]
    add sp, sp, #24
    ldmfd sp!, {r4, pc}

