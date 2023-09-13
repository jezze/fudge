.set VBE_MODENUM,                       0x8000
.set VBE_STACK,                         0x9000
.set VBE_CODE,                          0xA000
.set VBE_INFO,                          0xC000
.set VBE_MODE,                          0xD000
.set VBE_EDID,                          0xE000

.global vbe_lowmemstart
vbe_lowmemstart:

.code16

switch_real:
    movw $0x10, %ax
    movw %ax, %ds
    movw %ax, %es
    movw %ax, %fs
    movw %ax, %gs
    movw %ax, %ss
    movl $(VBE_CODE + realmode_idt - vbe_lowmemstart), %eax
    lidt (%eax)
    movl %cr0, %eax
    andl $0x7FFFFFFE, %eax
    movl %eax, %cr0
    pushl $0x0
    pushl %edx
    lret

getinfo_real:
    xorw %ax, %ax
    movw %ax, %ds
    movw %ax, %es
    movw %ax, %fs
    movw %ax, %gs
    movw %ax, %ss
    movw $0xA000, %sp
    movw $0x4F00, %ax
    xorw %bx, %bx
    xorw %cx, %cx
    xorw %dx, %dx
    movw $VBE_INFO, %di
    int $0x10
    jmp leave_real

getvideomode_real:
    xorw %ax, %ax
    movw %ax, %ds
    movw %ax, %es
    movw %ax, %fs
    movw %ax, %gs
    movw %ax, %ss
    movw $0xA000, %sp
    movw $0x4F01, %ax
    xorw %bx, %bx
    movw (VBE_MODENUM), %cx
    xorw %dx, %dx
    movw $VBE_MODE, %di
    int $0x10
    jmp leave_real

setvideomode_real:
    xorw %ax, %ax
    movw %ax, %ds
    movw %ax, %es
    movw %ax, %fs
    movw %ax, %gs
    movw %ax, %ss
    movw $0xA000, %sp
    movw $0x4F02, %ax
    movw (VBE_MODENUM), %bx
    xorw %cx, %cx
    xorw %dx, %dx
    xorw %di, %di
    int $0x10
    jmp leave_real

getedid_real:
    xorw %ax, %ax
    movw %ax, %ds
    movw %ax, %es
    movw %ax, %fs
    movw %ax, %gs
    movw %ax, %ss
    movw $0xA000, %sp
    movw $0x4F02, %ax
    movw $0x0001, %bx
    xorw %cx, %cx
    xorw %dx, %dx
    movw $VBE_EDID, %di
    int $0x10
    jmp leave_real

leave_real:
    movl $0x1000, %eax
    lgdt (%eax)
    movl $0x2000, %eax
    lidt (%eax)
    movl %cr0, %eax
    orl $1, %eax
    movl %eax, %cr0
    ljmp $0x8, $(VBE_CODE + switch_32 - vbe_lowmemstart)

.code32

switch_32:
    movw $0x10, %ax
    movw %ax, %ds
    movw %ax, %es
    movw %ax, %fs
    movw %ax, %gs
    movw %ax, %ss
    movl %cr0, %eax
    orl $0x80000000, %eax
    movl %eax, %cr0
    movl $VBE_STACK, %eax
    movl (%eax), %esp
    popa
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

.global vbe_getinfo
vbe_getinfo:
    movl $VBE_INFO, %eax
    pusha
    movl $(VBE_CODE + getinfo_real - vbe_lowmemstart), %edx
    jmp switch_16

.global vbe_getvideomode
vbe_getvideomode:
    movl $VBE_MODE, %eax
    pusha
    movl $VBE_MODENUM, %eax
    movl 36(%esp), %ebx
    movl %ebx, (%eax)
    movl $(VBE_CODE + getvideomode_real - vbe_lowmemstart), %edx
    jmp switch_16

.global vbe_setvideomode
vbe_setvideomode:
    pusha
    movl $VBE_MODENUM, %eax
    movl 36(%esp), %ebx
    movl %ebx, (%eax)
    movl $(VBE_CODE + setvideomode_real - vbe_lowmemstart), %edx
    jmp switch_16

.global vbe_getedid
vbe_getedid:
    pusha
    movl $(VBE_CODE + getedid_real - vbe_lowmemstart), %edx
    jmp switch_16

switch_16:
    movl $VBE_STACK, %eax
    movl %esp, (%eax)
    movl %cr0, %eax
    andl $~0x80000000, %eax
    movl %eax, %cr0
    movl $(VBE_CODE + realmode_gdt - vbe_lowmemstart), %eax
    lgdt (%eax)
    pushl $0x8
    movl $(VBE_CODE + switch_real - vbe_lowmemstart), %eax
    pushl %eax
    lret

