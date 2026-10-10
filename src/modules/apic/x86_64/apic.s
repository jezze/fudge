.code64

.section .text

.align 16
.global apic_isr
apic_isr:
    .align 8
    pushq $0x00
    jmp apic_common
    .align 8
    pushq $0x01
    jmp apic_common
    .align 8
    pushq $0x02
    jmp apic_common
    .align 8
    pushq $0x03
    jmp apic_common
    .align 8
    pushq $0x04
    jmp apic_common
    .align 8
    pushq $0x05
    jmp apic_common
    .align 8
    pushq $0x06
    jmp apic_common
    .align 8
    pushq $0x07
    jmp apic_common
    .align 8
    pushq $0x08
    jmp apic_common
    .align 8
    pushq $0x09
    jmp apic_common
    .align 8
    pushq $0x0A
    jmp apic_common
    .align 8
    pushq $0x0B
    jmp apic_common
    .align 8
    pushq $0x0C
    jmp apic_common
    .align 8
    pushq $0x0D
    jmp apic_common
    .align 8
    pushq $0x0E
    jmp apic_common
    .align 8
    pushq $0x0F
    jmp apic_common

.align 16
.global apic_test
apic_test:
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
    movq %rsp, %rdi
    leaq 120(%rsp), %rsi
    call apic_interruptnone
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
    iretq

.align 16
.global apic_spurious
apic_spurious:
    iretq

.align 16
apic_common:
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
    movq %rsp, %rdi
    movq 120(%rsp), %rsi
    leaq 128(%rsp), %rdx
    subq $8, %rsp
    call apic_interrupt
    addq $8, %rsp
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
    addq $8, %rsp
    iretq
