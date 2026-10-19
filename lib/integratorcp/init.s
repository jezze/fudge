.set STACK_SIZE,                        0x4000

.section .text.boot

.global _start
_start:
    msr cpsr_c, #0xD3
    ldr sp, =(stack + STACK_SIZE)
    bl integratorcp_setup

halt:
    b halt

.comm stack, STACK_SIZE, 8

