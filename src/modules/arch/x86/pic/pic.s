.code32

.extern pic_interrupt

.section .text

.global pic_routine00
pic_routine00:
    push $0
    push $0x00
    jmp pic_common

.global pic_routine01
pic_routine01:
    push $0
    push $0x01
    jmp pic_common

.global pic_routine02
pic_routine02:
    push $0
    push $0x02
    jmp pic_common

.global pic_routine03
pic_routine03:
    push $0
    push $0x03
    jmp pic_common

.global pic_routine04
pic_routine04:
    push $0
    push $0x04
    jmp pic_common

.global pic_routine05
pic_routine05:
    push $0
    push $0x05
    jmp pic_common

.global pic_routine06
pic_routine06:
    push $0
    push $0x06
    jmp pic_common

.global pic_routine07
pic_routine07:
    push $0
    push $0x07
    jmp pic_common

.global pic_routine08
pic_routine08:
    push $1
    push $0x08
    jmp pic_common

.global pic_routine09
pic_routine09:
    push $1
    push $0x09
    jmp pic_common

.global pic_routine0A
pic_routine0A:
    push $1
    push $0x0A
    jmp pic_common

.global pic_routine0B
pic_routine0B:
    push $1
    push $0x0B
    jmp pic_common

.global pic_routine0C
pic_routine0C:
    push $1
    push $0x0C
    jmp pic_common

.global pic_routine0D
pic_routine0D:
    push $1
    push $0x0D
    jmp pic_common

.global pic_routine0E
pic_routine0E:
    push $1
    push $0x0E
    jmp pic_common

.global pic_routine0F
pic_routine0F:
    push $1
    push $0x0F
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

