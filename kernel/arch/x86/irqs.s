extern irq_handle

global irq00
irq00:
    cli
    push 0
    push 0x00
    jmp irq_common

global irq01
irq01:
    cli
    push 0
    push 0x01
    jmp irq_common

global irq02
irq02:
    cli
    push 0
    push 0x02
    jmp irq_common

global irq03
irq03:
    cli
    push 0
    push 0x03
    jmp irq_common

global irq04
irq04:
    cli
    push 0
    push 0x04
    jmp irq_common

global irq05
irq05:
    cli
    push 0
    push 0x05
    jmp irq_common

global irq06
irq06:
    cli
    push 0
    push 0x06
    jmp irq_common

global irq07
irq07:
    cli
    push 0
    push 0x07
    jmp irq_common

global irq08
irq08:
    cli
    push 1
    push 0x08
    jmp irq_common

global irq09
irq09:
    cli
    push 1
    push 0x09
    jmp irq_common

global irq0A
irq0A:
    cli
    push 1
    push 0x0A
    jmp irq_common

global irq0B
irq0B:
    cli
    push 1
    push 0x0B
    jmp irq_common

global irq0C
irq0C:
    cli
    push 1
    push 0x0C
    jmp irq_common

global irq0D
irq0D:
    cli
    push 1
    push 0x0D
    jmp irq_common

global irq0E
irq0E:
    cli
    push 1
    push 0x0E
    jmp irq_common

global irq0F
irq0F:
    cli
    push 1
    push 0x0F
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
    call irq_handle
    pop eax
    pop gs
    pop fs
    pop es
    pop ds
    popa
    add esp, 8
    iret

