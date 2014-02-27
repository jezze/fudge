.code32

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
    pushf
    popl %eax
    orl $0x200, %eax
    pushl %eax
    movl 16(%esp), %eax
    pushl %eax
    movl 28(%esp), %eax
    pushl %eax
    iret

.global arch_isr_generalfault
arch_isr_generalfault:
    pusha
    movl %esp, %eax
    pushl %eax
    movw $0x10, %ax
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

.global arch_isr_pagefault
arch_isr_pagefault:
    pusha
    movl %esp, %eax
    pushl %eax
    movw $0x10, %ax
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

.global arch_isr_syscall
arch_isr_syscall:
    pusha
    movl %esp, %eax
    pushl %eax
    movw $0x10, %ax
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

