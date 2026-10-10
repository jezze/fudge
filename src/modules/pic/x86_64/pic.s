.code64

.section .text

.align 16
.global pic_isr0
pic_isr0:
    .align 8
    pushq $0x00
    jmp pic_common0
    .align 8
    pushq $0x01
    jmp pic_common0
    .align 8
    pushq $0x02
    jmp pic_common0
    .align 8
    pushq $0x03
    jmp pic_common0
    .align 8
    pushq $0x04
    jmp pic_common0
    .align 8
    pushq $0x05
    jmp pic_common0
    .align 8
    pushq $0x06
    jmp pic_common0
    .align 8
    pushq $0x07
    jmp pic_common0

.align 16
pic_common0:
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
    call pic_interrupt0
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

.align 16
.global pic_isr1
pic_isr1:
    .align 8
    pushq $0x08
    jmp pic_common1
    .align 8
    pushq $0x09
    jmp pic_common1
    .align 8
    pushq $0x0A
    jmp pic_common1
    .align 8
    pushq $0x0B
    jmp pic_common1
    .align 8
    pushq $0x0C
    jmp pic_common1
    .align 8
    pushq $0x0D
    jmp pic_common1
    .align 8
    pushq $0x0E
    jmp pic_common1
    .align 8
    pushq $0x0F
    jmp pic_common1

.align 16
pic_common1:
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
    call pic_interrupt1
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
