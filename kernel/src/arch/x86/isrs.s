.intel_syntax noprefix

.extern isr_handle

.global isr_common_cpu
isr_common_cpu:
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

