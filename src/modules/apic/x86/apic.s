.code32

.section .text

.align 16
.global apic_isr
apic_isr:
    .align 8
    pushl $0x00
    jmp apic_common
    .align 8
    pushl $0x01
    jmp apic_common
    .align 8
    pushl $0x02
    jmp apic_common
    .align 8
    pushl $0x03
    jmp apic_common
    .align 8
    pushl $0x04
    jmp apic_common
    .align 8
    pushl $0x05
    jmp apic_common
    .align 8
    pushl $0x06
    jmp apic_common
    .align 8
    pushl $0x07
    jmp apic_common
    .align 8
    pushl $0x08
    jmp apic_common
    .align 8
    pushl $0x09
    jmp apic_common
    .align 8
    pushl $0x0A
    jmp apic_common
    .align 8
    pushl $0x0B
    jmp apic_common
    .align 8
    pushl $0x0C
    jmp apic_common
    .align 8
    pushl $0x0D
    jmp apic_common
    .align 8
    pushl $0x0E
    jmp apic_common
    .align 8
    pushl $0x0F
    jmp apic_common

.align 16
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

.align 16
.global apic_spurious
apic_spurious:
    iret

.align 16
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

