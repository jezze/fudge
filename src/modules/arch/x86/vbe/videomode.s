.set VBE_STACK,                         0x7000
.set VBE_CODE,                          0x8000
.set VBE_GDT,                           0x9000

.intel_syntax noprefix

.global _modenum
_modenum:
.word 0

.global vbe_getinfo
vbe_getinfo:
    pushad
    mov eax, VBE_STACK
    mov [eax], esp
    mov eax, cr0
    and eax, ~0x80000000
    mov cr0, eax
    mov eax, VBE_GDT
    lgdt [eax]
    ljmp 0x8:(VBE_CODE + vbe_getinfo_16 - vbe_begin16)

.global vbe_getvideomode
vbe_getvideomode:
    pushad
    mov eax, VBE_STACK
    mov [eax], esp
    mov eax, cr0
    and eax, ~0x80000000
    mov cr0, eax
    mov eax, VBE_GDT
    lgdt [eax]
    ljmp 0x8:(VBE_CODE + vbe_getvideomode_16 - vbe_begin16)

.global vbe_setvideomode
vbe_setvideomode:
    pushad
    mov eax, VBE_STACK
    mov [eax], esp
    mov eax, cr0
    and eax, ~0x80000000
    mov cr0, eax
    mov eax, VBE_GDT
    lgdt [eax]
    ljmp 0x8:(VBE_CODE + vbe_setvideomode_16 - vbe_begin16)

.global vbe_begin16
vbe_begin16:

.code16

vbe_getinfo_16:
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
    ljmp 0x0:(VBE_CODE + vbe_getinfo_real - vbe_begin16)

vbe_getvideomode_16:
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
    ljmp 0x0:(VBE_CODE + vbe_getvideomode_real - vbe_begin16)

vbe_setvideomode_16:
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
    ljmp 0x0:(VBE_CODE + vbe_setvideomode_real - vbe_begin16)

vbe_getinfo_real:
    xor ax, ax
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax
    mov sp, 0xa000

    mov di, 0xc000
    mov eax, 0x32454256
    mov dword [di], eax
    mov ax, 0x4F00
    int 0x10

    mov eax, 0x1000
    lgdt [eax]
    mov eax, 0x2000
    lidt [eax]
    mov eax, cr0
    or eax, 1
    mov cr0, eax
    ljmp 0x8:(VBE_CODE + vbe_final32 - vbe_begin16)

vbe_getvideomode_real:
    xor ax, ax
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax
    mov sp, 0xa000

    mov ax, 0x4F01
    mov cx, _modenum
    mov di, 0xd000
    int 0x10

    mov eax, 0x1000
    lgdt [eax]
    mov eax, 0x2000
    lidt [eax]
    mov eax, cr0
    or eax, 1
    mov cr0, eax
    ljmp 0x8:(VBE_CODE + vbe_final32 - vbe_begin16)

vbe_setvideomode_real:
    xor ax, ax
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax
    mov sp, 0xa000

    mov ax, 0x4F02
    mov bx, _modenum
    mov di, 0
    int 0x10

    mov eax, 0x1000
    lgdt [eax]
    mov eax, 0x2000
    lidt [eax]
    mov eax, cr0
    or eax, 1
    mov cr0, eax
    ljmp 0x8:(VBE_CODE + vbe_final32 - vbe_begin16)

.code32

vbe_final32:
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

.global vbe_end16
vbe_end16:

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

