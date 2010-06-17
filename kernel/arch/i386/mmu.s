global mmu_flush
mmu_flush:
    push ebp
    mov ebp, esp
    mov eax, [ebp + 8]
    mov cr3, eax
    mov eax, cr0
    or eax, 0x80000000
    mov cr0, eax
    pop ebp
    ret

global mmu_read_cr0
mmu_read_cr0:
    mov eax, cr0
    ret

global mmu_read_cr2
mmu_read_cr2:
    mov eax, cr2
    ret

global mmu_read_cr3
mmu_read_cr3:
    mov eax, cr3
    ret

global mmu_write_cr0
mmu_write_cr0:
    push ebp
    mov ebp, esp
    mov eax, [ebp + 8]
    mov cr0, eax
    pop ebp
    ret

global mmu_write_cr3
mmu_write_cr3:
    push ebp
    mov ebp, esp
    mov eax, [ebp + 8]
    mov cr3, eax
    pop ebp
    ret

