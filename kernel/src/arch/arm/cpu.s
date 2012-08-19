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

