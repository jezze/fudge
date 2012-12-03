.intel_syntax noprefix

.global pic_routine00
pic_routine00:
    cli
    push 0
    push 0x00
    jmp pic_common

.global pic_routine01
pic_routine01:
    cli
    push 0
    push 0x01
    jmp pic_common

.global pic_routine02
pic_routine02:
    cli
    push 0
    push 0x02
    jmp pic_common

.global pic_routine03
pic_routine03:
    cli
    push 0
    push 0x03
    jmp pic_common

.global pic_routine04
pic_routine04:
    cli
    push 0
    push 0x04
    jmp pic_common

.global pic_routine05
pic_routine05:
    cli
    push 0
    push 0x05
    jmp pic_common

.global pic_routine06
pic_routine06:
    cli
    push 0
    push 0x06
    jmp pic_common

.global pic_routine07
pic_routine07:
    cli
    push 0
    push 0x07
    jmp pic_common

.global pic_routine08
pic_routine08:
    cli
    push 1
    push 0x08
    jmp pic_common

.global pic_routine09
pic_routine09:
    cli
    push 1
    push 0x09
    jmp pic_common

.global pic_routine0A
pic_routine0A:
    cli
    push 1
    push 0x0A
    jmp pic_common

.global pic_routine0B
pic_routine0B:
    cli
    push 1
    push 0x0B
    jmp pic_common

.global pic_routine0C
pic_routine0C:
    cli
    push 1
    push 0x0C
    jmp pic_common

.global pic_routine0D
pic_routine0D:
    cli
    push 1
    push 0x0D
    jmp pic_common

.global pic_routine0E
pic_routine0E:
    cli
    push 1
    push 0x0E
    jmp pic_common

.global pic_routine0F
pic_routine0F:
    cli
    push 1
    push 0x0F
    jmp pic_common

pic_common:
    pusha
    mov ax, ds
    push eax
    mov eax, esp
    push eax
    mov ax, 0x10
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    call pic_interrupt
    add esp, 4
    pop eax
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    popa
    add esp, 8
    iret

