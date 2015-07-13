.code32

.extern arch_generalfault
.extern arch_pagefault
.extern arch_syscall

.global isr_generalfault
isr_generalfault:
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

.global isr_pagefault
isr_pagefault:
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

.global isr_syscall
isr_syscall:
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

