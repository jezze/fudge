.section .text

.global cpu_getcontrol
cpu_getcontrol:
    mrc p15, 0, r0, c1, c0, 0
    bx lr

.global cpu_setcontrol
cpu_setcontrol:
    mcr p15, 0, r0, c1, c0, 0
    bx lr

.global cpu_setdomains
cpu_setdomains:
    mcr p15, 0, r0, c3, c0, 0
    bx lr

.global cpu_getttbr
cpu_getttbr:
    mrc p15, 0, r0, c2, c0, 0
    bx lr

.global cpu_setttbr
cpu_setttbr:
    mcr p15, 0, r0, c2, c0, 0
    mov r0, #0
    mcr p15, 0, r0, c8, c7, 0
    bx lr

.global cpu_flushtlb
cpu_flushtlb:
    mov r0, #0
    mcr p15, 0, r0, c8, c7, 0
    bx lr

.global cpu_getfsr
cpu_getfsr:
    mrc p15, 0, r0, c5, c0, 0
    bx lr

.global cpu_getfar
cpu_getfar:
    mrc p15, 0, r0, c6, c0, 0
    bx lr

.global cpu_setmodestack
cpu_setmodestack:
    mrs r2, cpsr
    bic r3, r2, #0x1F
    orr r3, r3, r0
    msr cpsr_c, r3
    mov sp, r1
    msr cpsr_c, r2
    bx lr

.global cpu_halt
cpu_halt:
    mov r0, #0
    mcr p15, 0, r0, c7, c0, 4
    b cpu_halt

.global cpu_leave
cpu_leave:
    mov sp, r0
    b isr_exit

