.text

.type strncmp, %function
.align 4

.global strncmp
strncmp:
    cmp r2, #0
    moveq r0, #0
    moveq pc, lr
    sub r2, r2, #1
    add ip, r0, r2
1:
    ldrb r2, [r0], #1
    ldrb r3, [r1], #1
    cmp ip, r0
    cmpcs r2, #1
    cmpcs r2, r3
    beq 1b
    sub r0, r2, r3
    mov pc, lr

