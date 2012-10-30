.intel_syntax noprefix

.extern isr_raise

.global isr_common
isr_common:
    pusha
    mov eax, esp
    push eax
    mov ax, 0x10
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    call isr_raise
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    add esp, 4
    popa
    add esp, 8
    iret

.global isr_undefined
isr_undefined:
    cli
    iret

.global isr_usermode
isr_usermode:
    mov ax, 0x23
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov eax, [esp + 8]
    push 0x23
    push eax
    pushf
    pop eax
    or eax, 0x200
    push eax
    push 0x1B
    mov eax, [esp + 20]
    push eax
    iret

