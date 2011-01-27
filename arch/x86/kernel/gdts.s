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

global tss_flush
tss_flush:
    mov ax, 0x2B
    ltr ax
    ret

global usermode_init
usermode_init:
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
    push usermode_next
    iret

usermode_next:
    pop eax
    jmp 0xffffffff
