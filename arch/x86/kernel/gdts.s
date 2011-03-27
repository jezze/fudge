extern kernel_init_usermode

global gdt_flush
gdt_flush:
    mov eax, [esp + 4]
    lgdt [eax]
    mov ax, 0x10
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax
    jmp 0x08:gdt_flush_high

gdt_flush_high:
    ret

global gdt_get_stack
gdt_get_stack:
    mov eax, esp
    ret

global gdt_set_stack
gdt_set_stack:
    mov eax, [esp + 4]
    mov esp, eax
    ret

global gdt_usermode_init
gdt_usermode_init:
    cli
    mov ax, 0x23
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov eax, esp
    push 0x23
    push eax
    pushf
    pop eax
    or eax, 0x200
    push eax
    push 0x1B
    push gdt_usermode_entry
    iret

gdt_usermode_entry:
    jmp kernel_init_usermode

