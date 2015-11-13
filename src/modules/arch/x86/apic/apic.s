.code32

.extern apic_interrupt

.global apic_spurious
apic_spurious:
    jmp apic_common

apic_common:
    pusha
    movw %ss, %ax
    movw %ax, %ds
    movw %ax, %es
    movw %ax, %fs
    movw %ax, %gs
    call apic_interrupt
    movw %ax, %ds
    movw %ax, %es
    movw %ax, %fs
    movw %ax, %gs
    popa
    iret

