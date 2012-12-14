.intel_syntax noprefix

.extern mmu_interrupt

.global mmu_routine
mmu_routine:
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
    call mmu_interrupt
    add esp, 4
    pop eax
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    popa
    iret

