.code32

.section .text

.extern smp_setup

.set SMP_GDTADDRESS,                    0x1000
.set SMP_IDTADDRESS,                    0x2000
.set SMP_INIT16ADDRESS,                 0x8000
.set SMP_INIT32ADDRESS,                 0x8100
.set SMP_STACKADDRESS,                  0x8800
.set SMP_CODESEL,                       0x08
.set SMP_DATASEL,                       0x10

.code16

.global smp_begin16
smp_begin16:

init16:
    cli
    xorw %ax, %ax
    movw %ax, %ds
    movw %ax, %es
    movw %ax, %fs
    movw %ax, %gs
    movw %ax, %ss
    movl $SMP_STACKADDRESS, %esp
    movl $SMP_STACKADDRESS, %ebp
    movl $SMP_GDTADDRESS, %eax
    lgdt (%eax)
    movl $SMP_IDTADDRESS, %eax
    lidt (%eax)
    movl %cr0, %eax
    orl $1, %eax
    movl %eax, %cr0
    ljmp $SMP_CODESEL, $SMP_INIT32ADDRESS

.global smp_end16
smp_end16:

.code32

.global smp_begin32
smp_begin32:

init32:
    movw $SMP_DATASEL, %ax
    movw %ax, %ds
    movw %ax, %es
    movw %ax, %fs
    movw %ax, %gs
    movw %ax, %ss
    lcall $SMP_CODESEL, $smp_setup

.global smp_end32
smp_end32:

