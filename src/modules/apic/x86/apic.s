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
    .align 8
    pushl $0x10
    jmp apic_common
    .align 8
    pushl $0x11
    jmp apic_common
    .align 8
    pushl $0x12
    jmp apic_common
    .align 8
    pushl $0x13
    jmp apic_common
    .align 8
    pushl $0x14
    jmp apic_common
    .align 8
    pushl $0x15
    jmp apic_common
    .align 8
    pushl $0x16
    jmp apic_common
    .align 8
    pushl $0x17
    jmp apic_common
    .align 8
    pushl $0x18
    jmp apic_common
    .align 8
    pushl $0x19
    jmp apic_common
    .align 8
    pushl $0x1A
    jmp apic_common
    .align 8
    pushl $0x1B
    jmp apic_common
    .align 8
    pushl $0x1C
    jmp apic_common
    .align 8
    pushl $0x1D
    jmp apic_common
    .align 8
    pushl $0x1E
    jmp apic_common
    .align 8
    pushl $0x1F
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
    leal 32(%esp), %eax
    pushl %eax
    leal 4(%esp), %eax
    pushl %eax
    call apic_interruptnone
    addl $8, %esp
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
    leal 36(%esp), %eax
    pushl %eax
    pushl 36(%esp)
    leal 8(%esp), %eax
    pushl %eax
    call apic_interrupt
    addl $12, %esp
    movw %ax, %ds
    movw %ax, %es
    movw %ax, %fs
    movw %ax, %gs
    popa
    addl $4, %esp
    iret

