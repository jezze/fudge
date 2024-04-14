.section .text

.global init
init:
    mov r1, #0x2000
    mov sp, r1
    bl arch_setup

.align 4
.global isr_swi
isr_swi:
    push {r0-r12, lr}
    bl arch_swi
    ldm sp!, {r0-r12, pc}^

.align 4
.global isr_irq
isr_irq:
    mov r0, #0x3000
    mov sp, r0
    sub lr, lr, #4
    push {r0-r12, lr}
    mrs r0, spsr
    push {r0}
    bl pic_irq
    pop {r0}
    msr spsr, r0
    ldm sp!, {r0-r12, pc}^

.align 4
.global isr_fiq
isr_fiq:
    mov r0, #0x3000
    mov sp, r0
    sub lr, lr, #4
    push {r0-r12, lr}
    mrs r0, spsr
    push {r0}
    bl pic_fiq
    pop {r0}
    msr spsr, r0
    ldm sp!, {r0-r12, pc}^

