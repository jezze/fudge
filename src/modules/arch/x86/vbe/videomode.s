.set VBE_STACK,                         0x7000
.set VBE_CODE,                          0x8000
.set VBE_GDT,                           0x9000

.intel_syntax noprefix

.global _modenum
_modenum:
.word 0

.code32

.global _get_info
_get_info:
    pushad
    mov eax, VBE_STACK
    mov [eax], esp
    mov eax, cr0
    and eax, ~0x80000000
    mov cr0, eax
    mov eax, VBE_GDT
    lgdt [eax]
    ljmp 0x8:(VBE_CODE + 1f - _get_info)

.code16

1:
    mov ax, 0x10
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax
    mov ax, (VBE_GDT + realmode_idt - realmode_gdt)
    lidt [eax]
    mov eax, cr0
    and eax, 0x7FFFFFFE
    mov cr0, eax
    ljmp 0x0:(VBE_CODE + 1f - _get_info)

1:
    xor ax, ax
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax
    mov sp, 0xa000

    # real mode. do stuff here.

    mov di, 0xc000
    mov eax, 0x32454256
    mov dword [di], eax
    mov ax, 0x4F00
    int 0x10
    cmp ax, 0x4F
    jne .XXerror
    mov ebx, 1
    jmp .XXfinish

.XXerror:
    xor ebx, ebx

.XXfinish:
    mov eax, 0x1000
    lgdt [eax]
    mov eax, 0x2000
    lidt [eax]
    mov eax, cr0
    or eax, 1
    mov cr0, eax
    ljmp 0x8:(VBE_CODE + 1f - _get_info)

.code32

1:
    mov ax, 0x10
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax
    mov eax, cr0
    or eax, 0x80000000
    mov cr0, eax
    mov eax, VBE_STACK
    mov esp, [eax]
    popad
    ret

.code32

.global _get_video_mode
_get_video_mode:
    pushad
    mov eax, VBE_STACK
    mov [eax], esp
    mov eax, cr0
    and eax, ~0x80000000
    mov cr0, eax
    mov eax, VBE_GDT
    lgdt [eax]
    ljmp 0x8:(VBE_CODE + 1f - _get_video_mode)

.code16

1:
    mov ax, 0x10
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax
    mov ax, (VBE_GDT + realmode_idt - realmode_gdt)
    lidt [eax]
    mov eax, cr0
    and eax, 0x7FFFFFFE
    mov cr0, eax
    ljmp 0x0:(VBE_CODE + 1f - _get_video_mode)

1:
    xor ax, ax
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax
    mov sp, 0xa000

    # real mode. do stuff here.

    mov ax, 0x4F01
    mov cx, _modenum
    mov di, 0xd000
    int 0x10
    cmp ax, 0x4F
    jne .Xerror
    mov ebx, 1
    jmp .Xfinish

.Xerror:
    xor ebx, ebx

.Xfinish:
    mov eax, 0x1000
    lgdt [eax]
    mov eax, 0x2000
    lidt [eax]
    mov eax, cr0
    or eax, 1
    mov cr0, eax
    ljmp 0x8:(VBE_CODE + 1f - _get_video_mode)

.code32

1:
    mov ax, 0x10
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax
    mov eax, cr0
    or eax, 0x80000000
    mov cr0, eax
    mov eax, VBE_STACK
    mov esp, [eax]
    popad
    ret

.code32

.global _set_video_mode
_set_video_mode:
    pushad
    mov eax, VBE_STACK
    mov [eax], esp
    mov eax, cr0
    and eax, ~0x80000000
    mov cr0, eax
    mov eax, VBE_GDT
    lgdt [eax]
    ljmp 0x8:(VBE_CODE + 1f - _set_video_mode)

.code16

1:
    mov ax, 0x10
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax
    mov ax, (VBE_GDT + realmode_idt - realmode_gdt)
    lidt [eax]
    mov eax, cr0
    and eax, 0x7FFFFFFE
    mov cr0, eax
    ljmp 0x0:(VBE_CODE + 1f - _set_video_mode)

1:
    xor ax, ax
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax
    mov sp, 0xa000

    # real mode. do stuff here.

    mov ax, 0x4F02
    mov bx, _modenum
    mov di, 0
    int 0x10
    cmp ax, 0x4F
    jne .Yerror
    mov ebx, 1
    jmp .Yfinish

.Yerror:
    xor ebx, ebx

.Yfinish:
    mov eax, 0x1000
    lgdt [eax]
    mov eax, 0x2000
    lidt [eax]
    mov eax, cr0
    or eax, 1
    mov cr0, eax
    ljmp 0x8:(VBE_CODE + 1f - _set_video_mode)

.code32

1:
    mov ax, 0x10
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax
    mov eax, cr0
    or eax, 0x80000000
    mov cr0, eax
    mov eax, VBE_STACK
    mov esp, [eax]
    popad
    ret

.code32

.global realmode_gdt
realmode_gdt:
.word (3 * 8) - 1
.long 0x9008
.word 0

.long 0x0
.long 0x0

.word 0xFFFF
.word 0x0
.byte 0x0
.byte 0x9A
.byte 0x8F
.byte 0x00

.word 0xFFFF
.word 0x0
.byte 0x0
.byte 0x92
.byte 0x8F
.byte 0x00

.global realmode_idt
realmode_idt:
.word 0x3FF
.long 0x0

