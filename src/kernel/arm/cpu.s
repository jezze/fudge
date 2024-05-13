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
    wfi
    b cpu_halt

.align 4
.global call_place
call_place:
    push {r0-r12, lr}
    mov r7, #0x0F
    swi #0
    pop {r0-r12, lr}
    bx lr

.align 4
.global call_despawn
call_despawn:
    push {r0-r12, lr}
    mov r7, #0x0D
    swi #0
    pop {r0-r12, lr}
    bx lr

