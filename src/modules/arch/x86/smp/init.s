.code32

.section .text

.set SMP_GDTADDRESS,                    0x1000
.set SMP_IDTADDRESS,                    0x2000
.set SMP_INIT16ADDRESS,                 0x8000
.set SMP_INIT32ADDRESS,                 0x8200
.set SMP_STACKSIZE,                     0x8000
.set SMP_KCODE,                         0x08
.set SMP_KDATA,                         0x10

stackaddr:
.int 0

.global smp_prep
smp_prep:
    movl stackaddr, %eax
    movl 4(%esp), %ebx
    movl %ebx, (%eax)
    ret

setup:
    movl stackaddr, %eax
    movl $SMP_STACKSIZE, %esp
    lock xaddl %esp, (%eax)
    pushl %esp
    call smp_setupap

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
    movl $SMP_GDTADDRESS, %eax
    lgdt (%eax)
    movl $SMP_IDTADDRESS, %eax
    lidt (%eax)
    movl %cr0, %eax
    orl $1, %eax
    movl %eax, %cr0
    ljmp $SMP_KCODE, $SMP_INIT32ADDRESS

.global smp_end16
smp_end16:

.code32

.global smp_begin32
smp_begin32:

init32:
    movw $SMP_KDATA, %ax
    movw %ax, %ds
    movw %ax, %es
    movw %ax, %fs
    movw %ax, %gs
    movw %ax, %ss
    ljmp $SMP_KCODE, $setup

.global smp_end32
smp_end32:

