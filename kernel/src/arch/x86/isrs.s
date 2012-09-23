.intel_syntax noprefix

.extern isr_handle

.global isr_routine
isr_routine:
    cli
    push 0xFF
    push 0xFF
    jmp isr_common

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
    call isr_handle
    mov ds, eax
    mov es, eax
    mov fs, eax
    mov gs, eax
    add esp, 4
    popa
    add esp, 8
    iret

