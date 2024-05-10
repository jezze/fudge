.section .text

.global cpu_get_cpsr
cpu_get_cpsr:
    mrs r0, cpsr
    bx lr

.global cpu_set_cpsr
cpu_set_cpsr:
    msr cpsr, r0
    bx lr

.global cpu_disable_interrupts
cpu_disable_interrupts:
    mrs r0, cpsr
    orr r0,r0, #0x80
    msr cpsr_c, r0
    mov r0, #1
    bx lr

.global cpu_enable_interrupts
cpu_enable_interrupts:
    mrs r0, cpsr
    bic r0, r0, #0x80
    msr cpsr_c, r0
    bx lr

.align 4
.global cpu_leave
cpu_leave:
    mov sp, r0
    mov lr, r1
    bx lr

.global cpu_halt
cpu_halt:
    wfe
    b cpu_halt

.global ivt_enable
ivt_enable:
    cpsie i
    bx lr

