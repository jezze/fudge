.section .data

b arch_reset
b arch_undefined
b arch_swi
b arch_undefined
b arch_undefined
b arch_undefined
b arch_irq
b arch_fiq

.section .text

.global init
init:
    mov r4, #0x10000
    cps #0x13
    mov sp, r4
    cps #0x17
    mov sp, r4
    cps #0x12
    mov sp, r4
    cps #0x1f
    mov sp, r4
    bl arch_setup

.global halt
halt:
    wfe
    b halt

