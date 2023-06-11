.set VBE_MODENUM,                       0x6000
.set VBE_STACK,                         0x7000
.set VBE_CODE,                          0x9000
.set VBE_INFO,                          0xC000
.set VBE_MODE,                          0xD000
.set VBE_EDID,                          0xE000

.intel_syntax noprefix

.global vbe_getinfo
vbe_getinfo:
    mov eax, VBE_INFO
    pushad
    mov edx, VBE_CODE + (getinfo_real - vbe_lowmemstart)
    jmp switch_16

.global vbe_getvideomode
vbe_getvideomode:
    mov eax, VBE_MODE
    pushad
    mov eax, VBE_MODENUM
    mov ebx, [esp + 32 + 4]
    mov [eax], ebx
    mov edx, VBE_CODE + (getvideomode_real - vbe_lowmemstart)
    jmp switch_16

.global vbe_setvideomode
vbe_setvideomode:
    pushad
    mov eax, VBE_MODENUM
    mov ebx, [esp + 32 + 4]
    mov [eax], ebx
    mov edx, VBE_CODE + (setvideomode_real - vbe_lowmemstart)
    jmp switch_16

.global vbe_getedid
vbe_getedid:
    pushad
    mov edx, VBE_CODE + (getedid_real - vbe_lowmemstart)
    jmp switch_16

switch_16:
    mov eax, VBE_STACK
    mov [eax], esp
    mov eax, cr0
    and eax, ~0x80000000
    mov cr0, eax
    mov eax, VBE_CODE + (realmode_gdt - vbe_lowmemstart)
    lgdt [eax]
    push 0x8
    mov eax, VBE_CODE + (switch_real - vbe_lowmemstart)
    push eax
    lret

.global vbe_lowmemstart
vbe_lowmemstart:

.code16

switch_real:
    mov ax, 0x10
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax
    mov eax, VBE_CODE + (realmode_idt - vbe_lowmemstart)
    lidt [eax]
    mov eax, cr0
    and eax, 0x7FFFFFFE
    mov cr0, eax
    push 0x0
    push edx
    lret

getinfo_real:
    xor ax, ax
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax
    mov sp, 0xA000
    mov ax, 0x4F00
    xor bx, bx
    xor cx, cx
    xor dx, dx
    mov di, VBE_INFO
    int 0x10
    jmp leave_real

getvideomode_real:
    xor ax, ax
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax
    mov sp, 0xA000
    mov ax, 0x4F01
    xor bx, bx
    mov cx, [VBE_MODENUM]
    xor dx, dx
    mov di, VBE_MODE
    int 0x10
    jmp leave_real

setvideomode_real:
    xor ax, ax
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax
    mov sp, 0xA000
    mov ax, 0x4F02
    mov bx, [VBE_MODENUM]
    xor cx, cx
    xor dx, dx
    xor di, di
    int 0x10
    jmp leave_real

getedid_real:
    xor ax, ax
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax
    mov sp, 0xA000
    mov ax, 0x4F15
    mov bx, 0x0001
    xor cx, cx
    xor dx, dx
    mov di, VBE_EDID
    int 0x10
    jmp leave_real

leave_real:
    mov eax, 0x1000
    lgdt [eax]
    mov eax, 0x2000
    lidt [eax]
    mov eax, cr0
    or eax, 1
    mov cr0, eax
    ljmp 0x8:(VBE_CODE + switch_32 - vbe_lowmemstart)

.code32

switch_32:
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

.global realmode_gdt
realmode_gdt:
.word (3 * 8) - 1
.long (VBE_CODE + realmode_gdt - vbe_lowmemstart + 8)
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

.global vbe_lowmemend
vbe_lowmemend:

