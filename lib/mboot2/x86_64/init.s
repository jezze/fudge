.code32

.set STACK_SIZE,                        0x4000
.set MBOOT_MAGIC,                       0xE85250D6
.set MBOOT_ARCH_I386,                   0
.set MBOOT_REQUIRED,                    0
.set MBOOT_OPTIONAL,                    1
.set MBOOT_TAG_END,                     0
.set MBOOT_TAG_CONSOLE,                 4
.set MBOOT_CONSOLE_EGATEXT,             2
.set BOOT_TABLES,                       4
.set BOOT_FLAGS,                        0x03
.set BOOT_LARGEFLAGS,                   0x83

.section .text.boot

.align 8
.global _start
_start:
    jmp mboot_entry

.align 8
mboot_start:
.int MBOOT_MAGIC
.int MBOOT_ARCH_I386
.int mboot_end - mboot_start
.int 0x100000000 - (MBOOT_MAGIC + MBOOT_ARCH_I386 + (mboot_end - mboot_start))

.align 8
.short MBOOT_TAG_CONSOLE
.short MBOOT_OPTIONAL
.int 12
.int MBOOT_CONSOLE_EGATEXT

.align 8
.short MBOOT_TAG_END
.short MBOOT_REQUIRED
.int 8
mboot_end:

.align 8
mboot_entry:
    movl $(stack + STACK_SIZE), %esp
    movl %eax, %ebp
    movl $boot_pdpt, %eax
    orl $BOOT_FLAGS, %eax
    movl %eax, boot_pml4
    xorl %ecx, %ecx

1:
    movl %ecx, %eax
    shll $12, %eax
    addl $boot_pd, %eax
    orl $BOOT_FLAGS, %eax
    movl %eax, boot_pdpt(, %ecx, 8)
    incl %ecx
    cmpl $BOOT_TABLES, %ecx
    jne 1b
    xorl %ecx, %ecx

2:
    movl %ecx, %eax
    shll $21, %eax
    orl $BOOT_LARGEFLAGS, %eax
    movl %eax, boot_pd(, %ecx, 8)
    incl %ecx
    cmpl $(BOOT_TABLES * 512), %ecx
    jne 2b
    movl %cr4, %eax
    orl $0x20, %eax
    movl %eax, %cr4
    movl $boot_pml4, %eax
    movl %eax, %cr3
    movl $0xC0000080, %ecx
    rdmsr
    orl $0x100, %eax
    wrmsr
    movl %cr0, %eax
    orl $0x80000000, %eax
    movl %eax, %cr0
    lgdt boot_gdtpointer
    ljmp $0x08, $mboot_entry64

.code64

mboot_entry64:
    movw $0x10, %ax
    movw %ax, %ds
    movw %ax, %es
    movw %ax, %fs
    movw %ax, %gs
    movw %ax, %ss
    movl $(stack + STACK_SIZE), %esp
    movl %ebx, %edi
    movl %ebp, %esi
    call mboot_setup

3:
    hlt
    jmp 3b

.align 8
boot_gdt:
.quad 0x0000000000000000
.quad 0x00AF9A000000FFFF
.quad 0x00CF92000000FFFF

boot_gdtpointer:
.short boot_gdtpointer - boot_gdt - 1
.quad boot_gdt

.section .bss

.align 0x1000
boot_pml4:
.skip 0x1000
boot_pdpt:
.skip 0x1000
boot_pd:
.skip (BOOT_TABLES * 0x1000)
stack:
.skip STACK_SIZE
