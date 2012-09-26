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
    mov eax, 0x10
    mov ds, eax
    mov es, eax
    mov fs, eax
    mov gs, eax
    call isr_raise
    mov ds, eax
    mov es, eax
    mov fs, eax
    mov gs, eax
    add esp, 4
    popa
    add esp, 8
    iret

