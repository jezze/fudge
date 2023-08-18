.code32

.section .text

.global cpu_getcr0
cpu_getcr0:
    movl %cr0, %eax
    ret

.global cpu_getcr2
cpu_getcr2:
    movl %cr2, %eax
    ret

.global cpu_getcr3
cpu_getcr3:
    movl %cr3, %eax
    ret

.global cpu_getcr4
cpu_getcr4:
    movl %cr4, %eax
    ret

.global cpu_geteflags
cpu_geteflags:
    pushf
    popl %eax
    ret

.global cpu_getgdt
cpu_getgdt:
    sgdt (%eax)
    ret

.global cpu_getidt
cpu_getidt:
    sidt (%eax)
    ret

.global cpu_setcr0
cpu_setcr0:
    movl 4(%esp), %eax
    movl %eax, %cr0
    ret

.global cpu_setcr2
cpu_setcr2:
    movl 4(%esp), %eax
    movl %eax, %cr2
    ret

.global cpu_setcr3
cpu_setcr3:
    movl 4(%esp), %eax
    movl %eax, %cr3
    ret

.global cpu_setcr4
cpu_setcr4:
    movl 4(%esp), %eax
    movl %eax, %cr4
    ret

.global cpu_seteflags
cpu_seteflags:
    movl 4(%esp), %eax
    pushl %eax
    popf
    ret

.global cpu_setgdt
cpu_setgdt:
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
    lret

.global cpu_setidt
cpu_setidt:
    movl 4(%esp), %eax
    lidt (%eax)
    ret

.global cpu_settss
cpu_settss:
    movw 4(%esp), %ax
    ltr %ax
    ret

.global cpu_halt
cpu_halt:
    hlt
    ret

.global cpu_leave
cpu_leave:
    addl $4, %esp
    movw 16(%esp), %ax
    movw %ax, %ds
    movw %ax, %es
    movw %ax, %fs
    movw %ax, %gs
    iret

