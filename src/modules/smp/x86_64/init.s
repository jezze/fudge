.code64

.set SMP_BASE16,                        0x8000
.set SMP_BASE32,                        0x8200
.set SMP_CODE32,                        0x08
.set SMP_DATA,                          0x10
.set SMP_CODE64,                        0x18
.set SMP_DIRECTORY,                     0x00A00000
.set SMP_STACKBASE,                     0x00602000

.section .text

setup:
    movl $1, %eax
    cpuid
    shrl $24, %ebx
    movl %ebx, %edi
    shll $13, %ebx
    movl $SMP_STACKBASE, %esp
    addl %ebx, %esp
    call smp_setupap

.code16

.global smp_begin16
smp_begin16:
    cli
    xorw %ax, %ax
    movw %ax, %ds
    movw %ax, %es
    movw %ax, %fs
    movw %ax, %gs
    movw %ax, %ss
    lgdtl SMP_BASE16 + (smp_gdtpointer - smp_begin16)
    movl %cr0, %eax
    orl $1, %eax
    movl %eax, %cr0
    ljmpl $SMP_CODE32, $SMP_BASE32

.align 8
smp_gdt:
.quad 0x0000000000000000
.quad 0x00CF9A000000FFFF
.quad 0x00CF92000000FFFF
.quad 0x00AF9A000000FFFF

smp_gdtpointer:
.short smp_gdtpointer - smp_gdt - 1
.int SMP_BASE16 + (smp_gdt - smp_begin16)
.global smp_end16
smp_end16:

.code32

.global smp_begin32
smp_begin32:
    movw $SMP_DATA, %ax
    movw %ax, %ds
    movw %ax, %es
    movw %ax, %fs
    movw %ax, %gs
    movw %ax, %ss
    movl %cr4, %eax
    orl $0x20, %eax
    movl %eax, %cr4
    movl $SMP_DIRECTORY, %eax
    movl %eax, %cr3
    movl $0xC0000080, %ecx
    rdmsr
    orl $0x100, %eax
    wrmsr
    movl %cr0, %eax
    orl $0x80000000, %eax
    movl %eax, %cr0
    ljmp $SMP_CODE64, $setup
.global smp_end32
smp_end32:
