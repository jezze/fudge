global cpu_disable_interrupts
cpu_disable_interrupts:
    cli
    ret

global cpu_enable_fpu
cpu_enable_fpu:
    finit
    ret

global cpu_enable_interrupts
cpu_enable_interrupts:
    sti
    ret

global cpu_enable_tss
cpu_enable_tss:
    mov ax, 0x2B
    ltr ax
    ret

global cpu_enter_usermode
cpu_enter_usermode:
    cli
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

global cpu_get_cr0
cpu_get_cr0:
    mov eax, cr0
    ret

global cpu_get_cr2
cpu_get_cr2:
    mov eax, cr2
    ret

global cpu_get_cr3
cpu_get_cr3:
    mov eax, cr3
    ret

global cpu_get_cr4
cpu_get_cr4:
    mov eax, cr4
    ret

global cpu_get_eflags
cpu_get_eflags:
    pushf
    pop eax
    ret

global cpu_get_stack
cpu_get_stack:
    mov eax, esp
    ret

global cpu_halt
cpu_halt:
    hlt
    ret

global cpu_idle
cpu_idle:
    mov eax, [esp]
    sti
    jmp $

global cpu_set_cr0
cpu_set_cr0:
    mov eax, [esp + 4]
    mov cr0, eax
    ret

global cpu_set_cr2
cpu_set_cr2:
    mov eax, [esp + 4]
    mov cr2, eax
    ret

global cpu_set_cr3
cpu_set_cr3:
    mov eax, [esp + 4]
    mov cr3, eax
    ret

global cpu_set_cr4
cpu_set_cr4:
    mov eax, [esp + 4]
    mov cr4, eax
    ret

global cpu_set_eflags
cpu_set_eflags:
    mov eax, [esp + 4]
    push eax
    popf
    ret

global cpu_set_gdt
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

global cpu_set_idt
cpu_set_idt:
    mov eax, [esp + 4]
    lidt [eax]
    ret

global cpu_set_stack
cpu_set_stack:
    mov eax, [esp + 4]
    mov ecx, [esp] 
    mov esp, eax
    push ecx
    ret

