.intel_syntax noprefix

.global cpu_get_cr0
cpu_get_cr0:
    mov eax, cr0
    ret

.global cpu_get_cr2
cpu_get_cr2:
    mov eax, cr2
    ret

.global cpu_get_cr3
cpu_get_cr3:
    mov eax, cr3
    ret

.global cpu_get_cr4
cpu_get_cr4:
    mov eax, cr4
    ret

.global cpu_get_eflags
cpu_get_eflags:
    pushf
    pop eax
    ret

.global cpu_halt
cpu_halt:
    hlt
    ret

.global cpu_set_cr0
cpu_set_cr0:
    mov eax, [esp + 4]
    mov cr0, eax
    ret

.global cpu_set_cr2
cpu_set_cr2:
    mov eax, [esp + 4]
    mov cr2, eax
    ret

.global cpu_set_cr3
cpu_set_cr3:
    mov eax, [esp + 4]
    mov cr3, eax
    ret

.global cpu_set_cr4
cpu_set_cr4:
    mov eax, [esp + 4]
    mov cr4, eax
    ret

.global cpu_set_eflags
cpu_set_eflags:
    mov eax, [esp + 4]
    push eax
    popf
    ret

.global cpu_set_gdt
cpu_set_gdt:
    mov eax, [esp + 4]
    lgdt [eax]
    mov ax, 0x10
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax
    jmp 0x08:cpu_set_gdt_high

cpu_set_gdt_high:
    ret

.global cpu_set_idt
cpu_set_idt:
    mov eax, [esp + 4]
    lidt [eax]
    ret

.global cpu_set_tss
cpu_set_tss:
    mov ax, [esp + 4]
    ltr ax
    ret

.global cpu_quirk_pic
cpu_quirk_pic:
    mov al, 0xFF
    out 0xA1, al
    out 0x21, al

