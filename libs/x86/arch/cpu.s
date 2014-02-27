.code32

.global cpu_get_cr0
cpu_get_cr0:
    movl %cr0, %eax
    ret

.global cpu_get_cr2
cpu_get_cr2:
    movl %cr2, %eax
    ret

.global cpu_get_cr3
cpu_get_cr3:
    movl %cr3, %eax
    ret

.global cpu_get_cr4
cpu_get_cr4:
    movl %cr4, %eax
    ret

.global cpu_get_eflags
cpu_get_eflags:
    pushf
    popl %eax
    ret

.global cpu_get_gdt
cpu_get_gdt:
    sgdt (%eax)
    ret

.global cpu_get_idt
cpu_get_idt:
    sidt (%eax)
    ret

.global cpu_set_cr0
cpu_set_cr0:
    movl 4(%esp), %eax
    movl %eax, %cr0
    ret

.global cpu_set_cr2
cpu_set_cr2:
    movl 4(%esp), %eax
    movl %eax, %cr2
    ret

.global cpu_set_cr3
cpu_set_cr3:
    movl 4(%esp), %eax
    movl %eax, %cr3
    ret

.global cpu_set_cr4
cpu_set_cr4:
    movl 4(%esp), %eax
    movl %eax, %cr4
    ret

.global cpu_set_eflags
cpu_set_eflags:
    movl 4(%esp), %eax
    pushl %eax
    popf
    ret

.global cpu_set_gdt
cpu_set_gdt:
    movl 4(%esp), %eax
    lgdt (%eax)
    movw 12(%esp), %ax
    movw %ax, %ds
    movw %ax, %es
    movw %ax, %fs
    movw %ax, %gs
    movw %ax, %ss
    pushl 8(%esp)
    pushl (%esp)
    retf

.global cpu_set_idt
cpu_set_idt:
    movl 4(%esp), %eax
    lidt (%eax)
    ret

.global cpu_set_tss
cpu_set_tss:
    movw 4(%esp), %ax
    ltrw %ax
    ret

