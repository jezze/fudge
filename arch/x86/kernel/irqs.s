extern irq_handler

global irq00
irq00:
    cli
    push 0
    push 0x20
    jmp irq_common

global irq01
irq01:
    cli
    push 0
    push 0x21
    jmp irq_common

global irq02
irq02:
    cli
    push 0
    push 0x22
    jmp irq_common

global irq03
irq03:
    cli
    push 0
    push 0x23
    jmp irq_common

global irq04
irq04:
    cli
    push 0
    push 0x24
    jmp irq_common

global irq05
irq05:
    cli
    push 0
    push 0x25
    jmp irq_common

global irq06
irq06:
    cli
    push 0
    push 0x26
    jmp irq_common

global irq07
irq07:
    cli
    push 0
    push 0x27
    jmp irq_common

global irq08
irq08:
    cli
    push 0
    push 0x28
    jmp irq_common

global irq09
irq09:
    cli
    push 0
    push 0x29
    jmp irq_common

global irq0A
irq0A:
    cli
    push 0
    push 0x2A
    jmp irq_common

global irq0B
irq0B:
    cli
    push 0
    push 0x2B
    jmp irq_common

global irq0C
irq0C:
    cli
    push 0
    push 0x2C
    jmp irq_common

global irq0D
irq0D:
    cli
    push 0
    push 0x2D
    jmp irq_common

global irq0E
irq0E:
    cli
    push 0
    push 0x2E
    jmp irq_common

global irq0F
irq0F:
    cli
    push 0
    push 0x2F
    jmp irq_common

irq_common:
    pusha
    push ds
    push es
    push fs
    push gs
    mov ax, 0x10
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov eax, esp
    push eax
    call irq_handler
    pop eax
    pop gs
    pop fs
    pop es
    pop ds
    popa
    add esp, 8
    iret

