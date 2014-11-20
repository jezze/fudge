.code32

.extern arch_getsegment
.extern arch_generalfault
.extern arch_pagefault
.extern arch_syscall

.global arch_halt
arch_halt:
    hlt
    ret

.global arch_usermode
arch_usermode:
    movw 8(%esp), %ax
    movw %ax, %ds
    movw %ax, %es
    movw %ax, %fs
    movw %ax, %gs
    pushl %eax
    movl 20(%esp), %eax
    pushl %eax
    call cpu_geteflags
    orl $0x200, %eax
    pushl %eax
    movl 16(%esp), %eax
    pushl %eax
    movl 28(%esp), %eax
    pushl %eax
    iret

.global arch_isrgeneralfault
arch_isrgeneralfault:
    pusha
    movl %esp, %eax
    pushl %eax
    call arch_getsegment
    movw %ax, %ds
    movw %ax, %es
    movw %ax, %fs
    movw %ax, %gs
    call arch_generalfault
    movw %ax, %ds
    movw %ax, %es
    movw %ax, %fs
    movw %ax, %gs
    addl $4, %esp
    popa
    addl $4, %esp
    iret

.global arch_isrpagefault
arch_isrpagefault:
    pusha
    movl %esp, %eax
    pushl %eax
    call arch_getsegment
    movw %ax, %ds
    movw %ax, %es
    movw %ax, %fs
    movw %ax, %gs
    call arch_pagefault
    movw %ax, %ds
    movw %ax, %es
    movw %ax, %fs
    movw %ax, %gs
    addl $4, %esp
    popa
    addl $4, %esp
    iret

.global arch_isrsyscall
arch_isrsyscall:
    pusha
    movl %esp, %eax
    pushl %eax
    call arch_getsegment
    movw %ax, %ds
    movw %ax, %es
    movw %ax, %fs
    movw %ax, %gs
    call arch_syscall
    movw %ax, %ds
    movw %ax, %es
    movw %ax, %fs
    movw %ax, %gs
    addl $4, %esp
    popa
    iret

