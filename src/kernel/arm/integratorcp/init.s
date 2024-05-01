.section .text

.global init
init:
    mov sp, #0x2000
    bl arch_setup

.align 4
.global isr_swi
isr_swi:
    mov sp, #0x3000
    stm sp, {r0-r12, lr}
    bl arch_swi
    ldm sp, {r0-r12, pc}^

.align 4
.global isr_irq
isr_irq:
    mov sp, #0x3000
    sub lr, lr, #4
    stm sp, {r0-r12, lr}
    bl pic_irq
    ldm sp, {r0-r12, pc}^

.align 4
.global isr_fiq
isr_fiq:
    mov sp, #0x3000
    sub lr, lr, #4
    stm sp, {r0-r12, lr}
    mrs r0, spsr
    push {r0}
    bl pic_fiq
    pop {r0}
    msr spsr, r0
    ldm sp!, {r0-r12, pc}^

.align 4
.global test_call
test_call:
    mov fp, sp
    stm sp!, {r0-r12, lr}
    swi #0
    ldm sp!, {r0-r12, lr}
    mov sp, fp
    bx lr
