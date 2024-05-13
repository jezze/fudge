.section .text

.global init
init:
    mov sp, #0x2000
    bl arch_setup

.align 4
.global isr_swi
isr_swi:
    push {r0-r12, lr}
    sub sp, #8
    stmia sp, {sp, lr}^

    mov r0, sp
    tst sp, #4
    subeq sp, #4
    push {r0}

    bl arch_syscall

    pop {r0}
    mov sp, r0

    ldmia sp, {sp, lr}^
    add sp, #8
    pop {r0-r12, lr}

    movs pc, lr

.align 4
.global isr_irq
isr_irq:
    mov sp, #0x2000
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

