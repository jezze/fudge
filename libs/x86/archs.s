.intel_syntax noprefix

.extern arch_generalfault
.extern arch_pagefault
.extern arch_syscall

.global arch_halt
arch_halt:
    hlt
    ret

.global arch_usermode
arch_usermode:
    mov ax, [esp + 8]
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    push eax
    mov eax, [esp + 20]
    push eax
    pushf
    pop eax
    or eax, 0x200
    push eax
    mov eax, [esp + 16]
    push eax
    mov eax, [esp + 28]
    push eax
    iret

.global arch_isr_generalfault
arch_isr_generalfault:
    cli
    pusha
    mov eax, esp
    push eax
    mov ax, 0x10
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    call arch_generalfault
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    add esp, 4
    popa
    add esp, 4
    iret

.global arch_isr_pagefault
arch_isr_pagefault:
    cli
    pusha
    mov eax, esp
    push eax
    mov ax, 0x10
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    call arch_pagefault
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    add esp, 4
    popa
    add esp, 4
    iret

.global arch_isr_syscall
arch_isr_syscall:
    cli
    pusha
    mov eax, esp
    push eax
    mov ax, 0x10
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    call arch_syscall
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    add esp, 4
    popa
    iret

