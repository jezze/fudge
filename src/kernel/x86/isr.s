.code32

.section .text

.global isr_zero
isr_zero:
    pusha
    movw %ss, %ax
    movw %ax, %ds
    call arch_zero
    movw %ax, %ds
    popa
    iret

.global isr_debug
isr_debug:
    pusha
    movw %ss, %ax
    movw %ax, %ds
    call arch_debug
    movw %ax, %ds
    popa
    iret

.global isr_nmi
isr_nmi:
    pusha
    movw %ss, %ax
    movw %ax, %ds
    call arch_nmi
    movw %ax, %ds
    popa
    iret

.global isr_breakpoint
isr_breakpoint:
    pusha
    movw %ss, %ax
    movw %ax, %ds
    call arch_breakpoint
    movw %ax, %ds
    popa
    iret

.global isr_overflow
isr_overflow:
    pusha
    movw %ss, %ax
    movw %ax, %ds
    call arch_overflow
    movw %ax, %ds
    popa
    iret

.global isr_bound
isr_bound:
    pusha
    movw %ss, %ax
    movw %ax, %ds
    call arch_bound
    movw %ax, %ds
    popa
    iret

.global isr_opcode
isr_opcode:
    pusha
    movw %ss, %ax
    movw %ax, %ds
    call arch_opcode
    movw %ax, %ds
    popa
    iret

.global isr_device
isr_device:
    pusha
    movw %ss, %ax
    movw %ax, %ds
    call arch_device
    movw %ax, %ds
    popa
    iret

.global isr_doublefault
isr_doublefault:
    pusha
    movw %ss, %ax
    movw %ax, %ds
    call arch_doublefault
    movw %ax, %ds
    popa
    addl $4, %esp
    iret

.global isr_tss
isr_tss:
    pusha
    movw %ss, %ax
    movw %ax, %ds
    call arch_tss
    movw %ax, %ds
    popa
    addl $4, %esp
    iret

.global isr_segment
isr_segment:
    pusha
    movw %ss, %ax
    movw %ax, %ds
    call arch_segment
    movw %ax, %ds
    popa
    addl $4, %esp
    iret

.global isr_stack
isr_stack:
    pusha
    movw %ss, %ax
    movw %ax, %ds
    call arch_stack
    movw %ax, %ds
    popa
    addl $4, %esp
    iret

.global isr_generalfault
isr_generalfault:
    pusha
    movw %ss, %ax
    movw %ax, %ds
    call arch_generalfault
    movw %ax, %ds
    popa
    addl $4, %esp
    iret

.global isr_pagefault
isr_pagefault:
    pusha
    movw %ss, %ax
    movw %ax, %ds
    call arch_pagefault
    movw %ax, %ds
    popa
    addl $4, %esp
    iret

.global isr_syscall
isr_syscall:
    pusha
    movw %ss, %ax
    movw %ax, %ds
    call arch_syscall
    movw %ax, %ds
    popa
    iret

