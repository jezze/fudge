.code32

.extern arch_generalfault
.extern arch_pagefault
.extern arch_syscall

.global arch_isrgeneralfault
arch_isrgeneralfault:
    pusha
    movl %esp, %eax
    pushl %eax
    movw %ss, %ax
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
    movw %ss, %ax
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
    movw %ss, %ax
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

