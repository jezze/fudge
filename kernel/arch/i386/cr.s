global cr0_read
cr0_read:
    mov eax, cr0
    ret

global cr2_read
cr2_read:
    mov eax, cr2
    ret

global cr3_read
cr3_read:
    mov eax, cr3
    ret

global cr4_read
cr4_read:
    mov eax, cr4
    ret

global cr0_write
cr0_write:
    mov eax, [esp + 4]
    mov cr0, eax
    ret

global cr2_write
cr2_write:
    mov eax, [esp + 4]
    mov cr2, eax
    ret

global cr3_write
cr3_write:
    mov eax, [esp + 4]
    mov cr3, eax
    ret

global cr4_write
cr4_write:
    mov eax, [esp + 4]
    mov cr4, eax
    ret

