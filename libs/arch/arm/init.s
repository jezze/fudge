interrupt_vector_table:
    b arch_reset
    b arch_undefined
    b arch_syscall
    b arch_undefined
    b arch_undefined
    b arch_undefined
    b arch_irq
    b arch_fiq

.comm stack, 0x10000

.global init
init:
    ldr sp, =stack+0x10000
    bl arch_setup

.global halt
halt:
    wfi
    b halt

