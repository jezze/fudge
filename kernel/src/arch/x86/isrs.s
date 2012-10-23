.intel_syntax noprefix

.extern isr_raise

.global isr_undefined
isr_undefined:
    cli
    iret

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

