.code32

.extern apic_interrupt

.section .text

.global apic_test
apic_test:
    pusha
    movw %ss, %ax
    movw %ax, %ds
    call apic_interrupt
    movw %ax, %ds
    popa
    iret

.global apic_spurious
apic_spurious:
    iret

