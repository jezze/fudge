global cpu_read_cr0
cpu_read_cr0:
    mov eax, cr0
    ret

global cpu_read_cr2
cpu_read_cr2:
    mov eax, cr2
    ret

global cpu_read_cr3
cpu_read_cr3:
    mov eax, cr3
    ret

global cpu_read_cr4
cpu_read_cr4:
    mov eax, cr4
    ret

global cpu_write_cr0
cpu_write_cr0:
    mov eax, [esp + 4]
    mov cr0, eax
    ret

global cpu_write_cr2
cpu_write_cr2:
    mov eax, [esp + 4]
    mov cr2, eax
    ret

global cpu_write_cr3
cpu_write_cr3:
    mov eax, [esp + 4]
    mov cr3, eax
    ret

global cpu_write_cr4
cpu_write_cr4:
    mov eax, [esp + 4]
    mov cr4, eax
    ret

