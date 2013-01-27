.intel_syntax noprefix

.extern arch_pagefault
.extern arch_syscall

.global arch_disable_pic
arch_disable_pic:
    mov al, 0xff
    outb 0xA1, al
    outb 0x21, al
    ret

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

.global arch_isr_pagefault
arch_isr_pagefault:
    cli
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
    call arch_pagefault
    add esp, 4
    pop eax
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    popa
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

