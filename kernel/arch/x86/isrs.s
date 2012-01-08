extern isr_handle
extern syscall_handle
extern syscall_handle_quick

global isr00
isr00:
    cli
    push 0
    push 0x00
    jmp isr_common

global isr01
isr01:
    cli
    push 0
    push 0x01
    jmp isr_common

global isr02
isr02:
    cli
    push 0
    push 0x02
    jmp isr_common

global isr03
isr03:
    cli
    push 0
    push 0x03
    jmp isr_common

global isr04
isr04:
    cli
    push 0
    push 0x04
    jmp isr_common

global isr05
isr05:
    cli
    push 0
    push 0x05
    jmp isr_common

global isr06
isr06:
    cli
    push 0
    push 0x06
    jmp isr_common

global isr07
isr07:
    cli
    push 0
    push 0x07
    jmp isr_common

global isr08
isr08:
    cli
    push 0x08
    jmp isr_common

global isr09
isr09:
    cli
    push 0
    push 0x09
    jmp isr_common

global isr0A
isr0A:
    cli
    push 0x0A
    jmp isr_common

global isr0B
isr0B:
    cli
    push 0x0B
    jmp isr_common

global isr0C
isr0C:
    cli
    push 0x0C
    jmp isr_common

global isr0D
isr0D:
    cli
    push 0x0D
    jmp isr_common

global isr0E
isr0E:
    cli
    push 0x0E
    jmp isr_common

global isr0F
isr0F:
    cli
    push 0
    push 0x0F
    jmp isr_common

global isr10
isr10:
    cli
    push 0
    push 0x10
    jmp isr_common

global isr11
isr11:
    cli
    push 0
    push 0x11
    jmp isr_common

global isr12
isr12:
    cli
    push 0
    push 0x12
    jmp isr_common

global isr13
isr13:
    cli
    push 0
    push 0x13
    jmp isr_common

global isr14
isr14:
    cli
    push 0
    push 0x14
    jmp isr_common

global isr15
isr15:
    cli
    push 0
    push 0x15
    jmp isr_common

global isr16
isr16:
    cli
    push 0
    push 0x16
    jmp isr_common

global isr17
isr17:
    cli
    push 0
    push 0x17
    jmp isr_common

global isr18
isr18:
    cli
    push 0
    push 0x18
    jmp isr_common

global isr19
isr19:
    cli
    push 0
    push 0x19
    jmp isr_common

global isr1A
isr1A:
    cli
    push 0
    push 0x1A
    jmp isr_common

global isr1B
isr1B:
    cli
    push 0
    push 0x1B
    jmp isr_common

global isr1C
isr1C:
    cli
    push 0
    push 0x1C
    jmp isr_common

global isr1D
isr1D:
    cli
    push 0
    push 0x1D
    jmp isr_common

global isr1E
isr1E:
    cli
    push 0
    push 0x1E
    jmp isr_common

global isr1F
isr1F:
    cli
    push 0
    push 0x1F
    jmp isr_common

global isr_syscall
isr_syscall:
    cli
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
    call syscall_handle
    pop eax
    pop gs
    pop fs
    pop es
    pop ds
    popa
    iret

global isr_syscall_quick
isr_syscall_quick:
    cli
    push eax
    push ebp
    push ds
    push es
    push fs
    push gs
    mov ax, 0x10
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov edx, esp
    push edx
    call syscall_handle_quick
    pop edx
    pop gs
    pop fs
    pop es
    pop ds
    add esp, 8
    iret

isr_common:
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
    call isr_handle
    pop eax
    pop gs
    pop fs
    pop es
    pop ds
    popa
    add esp, 8
    iret

