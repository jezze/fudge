.code32

.extern pic_interrupt

.section .text

.global pic_routine00
pic_routine00:
    pushl $0
    pushl $0x00
    jmp pic_common

.global pic_routine01
pic_routine01:
    pushl $0
    pushl $0x01
    jmp pic_common

.global pic_routine02
pic_routine02:
    pushl $0
    pushl $0x02
    jmp pic_common

.global pic_routine03
pic_routine03:
    pushl $0
    pushl $0x03
    jmp pic_common

.global pic_routine04
pic_routine04:
    pushl $0
    pushl $0x04
    jmp pic_common

.global pic_routine05
pic_routine05:
    pushl $0
    pushl $0x05
    jmp pic_common

.global pic_routine06
pic_routine06:
    pushl $0
    pushl $0x06
    jmp pic_common

.global pic_routine07
pic_routine07:
    pushl $0
    pushl $0x07
    jmp pic_common

.global pic_routine08
pic_routine08:
    pushl $1
    pushl $0x08
    jmp pic_common

.global pic_routine09
pic_routine09:
    pushl $1
    pushl $0x09
    jmp pic_common

.global pic_routine0A
pic_routine0A:
    pushl $1
    pushl $0x0A
    jmp pic_common

.global pic_routine0B
pic_routine0B:
    pushl $1
    pushl $0x0B
    jmp pic_common

.global pic_routine0C
pic_routine0C:
    pushl $1
    pushl $0x0C
    jmp pic_common

.global pic_routine0D
pic_routine0D:
    pushl $1
    pushl $0x0D
    jmp pic_common

.global pic_routine0E
pic_routine0E:
    pushl $1
    pushl $0x0E
    jmp pic_common

.global pic_routine0F
pic_routine0F:
    pushl $1
    pushl $0x0F
    jmp pic_common

pic_common:
    pusha
    movw %ss, %ax
    movw %ax, %ds
    call pic_interrupt
    movw %ax, %ds
    popa
    addl $8, %esp
    iret

