.code32

.section .text

.align 16
.global pic_isr0
pic_isr0:
    .align 8
    pushl $0x00
    jmp pic_common0
    .align 8
    pushl $0x01
    jmp pic_common0
    .align 8
    pushl $0x02
    jmp pic_common0
    .align 8
    pushl $0x03
    jmp pic_common0
    .align 8
    pushl $0x04
    jmp pic_common0
    .align 8
    pushl $0x05
    jmp pic_common0
    .align 8
    pushl $0x06
    jmp pic_common0
    .align 8
    pushl $0x07
    jmp pic_common0

.align 16
pic_common0:
    pusha
    movw %ss, %ax
    movw %ax, %ds
    movw %ax, %es
    movw %ax, %fs
    movw %ax, %gs
    call pic_interrupt0
    movw %ax, %ds
    movw %ax, %es
    movw %ax, %fs
    movw %ax, %gs
    popa
    addl $4, %esp
    iret

.align 16
.global pic_isr1
pic_isr1:
    .align 8
    pushl $0x08
    jmp pic_common1
    .align 8
    pushl $0x09
    jmp pic_common1
    .align 8
    pushl $0x0A
    jmp pic_common1
    .align 8
    pushl $0x0B
    jmp pic_common1
    .align 8
    pushl $0x0C
    jmp pic_common1
    .align 8
    pushl $0x0D
    jmp pic_common1
    .align 8
    pushl $0x0E
    jmp pic_common1
    .align 8
    pushl $0x0F
    jmp pic_common1

.align 16
pic_common1:
    pusha
    movw %ss, %ax
    movw %ax, %ds
    movw %ax, %es
    movw %ax, %fs
    movw %ax, %gs
    call pic_interrupt1
    movw %ax, %ds
    movw %ax, %es
    movw %ax, %fs
    movw %ax, %gs
    popa
    addl $4, %esp
    iret

