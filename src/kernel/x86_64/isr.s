.code64

.macro pushall
    cld
    pushq %rax
    pushq %rcx
    pushq %rdx
    pushq %rbx
    pushq %rbp
    pushq %rsi
    pushq %rdi
    pushq %r8
    pushq %r9
    pushq %r10
    pushq %r11
    pushq %r12
    pushq %r13
    pushq %r14
    pushq %r15
.endm

.macro popall
    popq %r15
    popq %r14
    popq %r13
    popq %r12
    popq %r11
    popq %r10
    popq %r9
    popq %r8
    popq %rdi
    popq %rsi
    popq %rbp
    popq %rbx
    popq %rdx
    popq %rcx
    popq %rax
.endm

.macro isr name, handler
.global \name
\name:
    pushall
    movq %rsp, %rdi
    leaq 120(%rsp), %rsi
    call \handler
    popall
    iretq
.endm

.macro isrerror name, handler
.global \name
\name:
    pushall
    movq %rsp, %rdi
    movq 120(%rsp), %rsi
    leaq 128(%rsp), %rdx
    subq $8, %rsp
    call \handler
    addq $8, %rsp
    popall
    addq $8, %rsp
    iretq
.endm

.section .text

isr isr_zero, arch_zero
isr isr_debug, arch_debug
isr isr_nmi, arch_nmi
isr isr_breakpoint, arch_breakpoint
isr isr_overflow, arch_overflow
isr isr_bound, arch_bound
isr isr_opcode, arch_opcode
isr isr_device, arch_device
isrerror isr_doublefault, arch_doublefault
isrerror isr_tss, arch_tss
isrerror isr_segment, arch_segment
isrerror isr_stack, arch_stack
isrerror isr_generalfault, arch_generalfault
isrerror isr_pagefault, arch_pagefault
isr isr_syscall, arch_syscall
