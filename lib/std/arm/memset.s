.text

.type memset, %function
.align 4

.global memset
memset:
    stmfd sp!, {r0}
    and r1, r1, #0x000000ff
    cmp r2, #0x00000004
    blt .Lmemset_lessthanfour
    ands r3, r0, #0x00000003
    beq .Lmemset_addraligned
    rsb r3, r3, #0x00000004
    sub r2, r2, r3
    cmp r3, #0x00000002
    strb r1, [r0], #0x0001
    strgeb r1, [r0], #0x0001
    strgtb r1, [r0], #0x0001
    cmp r2, #0x00000004
    blt .Lmemset_lessthanfour

.Lmemset_addraligned:
    orr r3, r1, r1, lsl #8
    orr r3, r3, r3, lsl #16
    cmp r2, #0x00000020
    blt .Lmemset_lessthan32
    stmfd sp!, {r4-r6}
    mov r4, r3
    mov r5, r3
    mov r6, r3

.Lmemset_loop16:
    stmia r0!, {r3-r6}
    sub r2, r2, #0x00000010
    cmp r2, #0x00000010
    bgt .Lmemset_loop16
    ldmfd sp!, {r4-r6}
    cmp r2, #0x00000004
    blt .Lmemset_lessthanfour

.Lmemset_lessthan32:

.Lmemset_loop4:
    str r3, [r0], #0x0004
    sub r2, r2, #0x0004
    cmp r2, #0x00000004
    bge .Lmemset_loop4

.Lmemset_lessthanfour:
    cmp r2, #0x00000000
    ldmeqfd sp!, {r0}
    moveq pc, lr
    cmp r2, #0x00000002
    strb r1, [r0], #0x0001
    strgeb r1, [r0], #0x0001
    strgtb r1, [r0], #0x0001
    ldmfd sp!, {r0}
    mov pc, lr

