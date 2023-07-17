.code32

.extern pic_interrupt

.section .text

.global pic_isr0
pic_isr0:
    .align 8
    pushl $0
    pushl $0x00
    jmp pic_common
    .align 8
    pushl $0
    pushl $0x01
    jmp pic_common
    .align 8
    pushl $0
    pushl $0x02
    jmp pic_common
    .align 8
    pushl $0
    pushl $0x03
    jmp pic_common
    .align 8
    pushl $0
    pushl $0x04
    jmp pic_common
    .align 8
    pushl $0
    pushl $0x05
    jmp pic_common
    .align 8
    pushl $0
    pushl $0x06
    jmp pic_common
    .align 8
    pushl $0
    pushl $0x07
    jmp pic_common

.global pic_isr1
pic_isr1:
    .align 8
    pushl $1
    pushl $0x08
    jmp pic_common
    .align 8
    pushl $1
    pushl $0x09
    jmp pic_common
    .align 8
    pushl $1
    pushl $0x0A
    jmp pic_common
    .align 8
    pushl $1
    pushl $0x0B
    jmp pic_common
    .align 8
    pushl $1
    pushl $0x0C
    jmp pic_common
    .align 8
    pushl $1
    pushl $0x0D
    jmp pic_common
    .align 8
    pushl $1
    pushl $0x0E
    jmp pic_common
    .align 8
    pushl $1
    pushl $0x0F
    jmp pic_common

pic_common:
    pusha
    movw %ss, %ax
    movw %ax, %ds
    movw %ax, %es
    movw %ax, %fs
    movw %ax, %gs
    call pic_interrupt
    movw %ax, %ds
    movw %ax, %es
    movw %ax, %fs
    movw %ax, %gs
    popa
    addl $8, %esp
    iret

