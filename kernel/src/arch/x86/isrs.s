.intel_syntax noprefix

.global isr_routine
isr_routine:
    cli
    iret

.global isr_usermode
isr_usermode:
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

