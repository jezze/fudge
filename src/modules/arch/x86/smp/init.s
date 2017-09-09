.code32

.extern smp_setup

.section .text

.code16

.global smp_begin16
smp_begin16:

init:
    cli
    xorw %ax, %ax
    movw %ax, %ds
    movw %ax, %es
    movw %ax, %fs
    movw %ax, %gs
    movw %ax, %ss

    # print red thing
#    movw $0xB800, %ax
#    movw %ax, %es
#    xorw %bx, %bx
#    movw $0x4020, %ax
#    movw %ax, %es:(%bx)

    # setup stack
#    movw $0x8800, %ax
#    movw %ax, %sp
#    movw %ax, %bp

    # load gdt
    movl $0x1000, %eax
    lgdt (%eax)

    # protected mode
    movl %cr0, %eax
    orl $1, %eax
    movl %eax, %cr0

    # long jump
#    ljmp $0x08, $0x9000

    # long ret
#    pushl $0x08
#    pushl $0x9000
#    lret

    hlt

.global smp_end16
smp_end16:

.code32

.global smp_begin32
smp_begin32:

smp_init32:
#    movl $0xB8000, %eax
#    movl $0x4121, (%eax)

    movw $0x10, %ax
    movw %ax, %ds
    movw %ax, %es
    movw %ax, %fs
    movw %ax, %gs
    movw %ax, %ss

    movl $0xB8000, %eax
    movl $0x4121, (%eax)

    jmp .
    call smp_setup

.global smp_end32
smp_end32:

