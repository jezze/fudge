.code32

.extern apic_interrupt
.extern apic_interruptnone

.section .text

.global apic_isr00
apic_isr00:
    pushl $0x00
    jmp apic_common

.global apic_isr01
apic_isr01:
    pushl $0x01
    jmp apic_common

.global apic_isr02
apic_isr02:
    pushl $0x02
    jmp apic_common

.global apic_isr03
apic_isr03:
    pushl $0x03
    jmp apic_common

.global apic_isr04
apic_isr04:
    pushl $0x04
    jmp apic_common

.global apic_isr05
apic_isr05:
    pushl $0x05
    jmp apic_common

.global apic_isr06
apic_isr06:
    pushl $0x06
    jmp apic_common

.global apic_isr07
apic_isr07:
    pushl $0x07
    jmp apic_common

.global apic_isr08
apic_isr08:
    pushl $0x08
    jmp apic_common

.global apic_isr09
apic_isr09:
    pushl $0x09
    jmp apic_common

.global apic_isr0A
apic_isr0A:
    pushl $0x0A
    jmp apic_common

.global apic_isr0B
apic_isr0B:
    pushl $0x0B
    jmp apic_common

.global apic_isr0C
apic_isr0C:
    pushl $0x0C
    jmp apic_common

.global apic_isr0D
apic_isr0D:
    pushl $0x0D
    jmp apic_common

.global apic_isr0E
apic_isr0E:
    pushl $0x0E
    jmp apic_common

.global apic_isr0F
apic_isr0F:
    pushl $0x0F
    jmp apic_common

.global apic_test
apic_test:
    pusha
    movw %ss, %ax
    movw %ax, %ds
    movw %ax, %es
    movw %ax, %fs
    movw %ax, %gs
    call apic_interruptnone
    movw %ax, %ds
    movw %ax, %es
    movw %ax, %fs
    movw %ax, %gs
    popa
    iret

.global apic_spurious
apic_spurious:
    iret

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
    addl $4, %esp
    iret

