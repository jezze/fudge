.code32

.section .text

.extern smp_setup

.global smp_arne
smp_arne:
    call smp_setup
    hlt

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
    movl $0x8C00, %esp
    movl $0x8C00, %ebp
    movl $0x1000, %eax
    lgdt (%eax)
    movl %cr0, %eax
    orl $1, %eax
    movl %eax, %cr0
    ljmp $0x08, $0x9000

.global smp_end16
smp_end16:

.code32

.global smp_begin32
smp_begin32:

smp_init32:
    movw $0x10, %ax
    movw %ax, %ds
    movw %ax, %es
    movw %ax, %fs
    movw %ax, %gs
    movw %ax, %ss
    ljmp $0x08, $smp_arne

.global smp_end32
smp_end32:

