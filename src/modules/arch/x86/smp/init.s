.code32

.extern smp_setup

.section .text

.code32

init32:
    movw $0x10, %ax
    movw %ax, %ds
    movw %ax, %es
    movw %ax, %fs
    movw %ax, %gs
    movw %ax, %ss
    ljmp $0x08, $print32

print32:
    movl $0xB8000, %eax
    movl $0x4121, (%eax)
    jmp .
    call smp_setup

.global smp_begin
smp_begin:

.code16
init:
    cli
    xorw %ax, %ax
    movw %ax, %ds
    movw %ax, %es
    movw %ax, %fs
    movw %ax, %gs
    movw %ax, %ss

    movw $0x8800, %ax
    movw %ax, %sp

    movl $0x1000, %eax
    lgdt (%eax)

#    ljmp $0x08, $init32

    call print
    hlt

print:
    movw $0xB800, %ax
    movw %ax, %es
    xorw %bx, %bx
    movw $0x4020, %ax
    movw %ax, %es:(%bx)
    ret

.global smp_end
smp_end:
