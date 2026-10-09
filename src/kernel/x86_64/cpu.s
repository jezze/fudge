.code64

.section .text

.global cpu_getcr0
cpu_getcr0:
    movq %cr0, %rax
    ret

.global cpu_getcr2
cpu_getcr2:
    movq %cr2, %rax
    ret

.global cpu_getcr3
cpu_getcr3:
    movq %cr3, %rax
    ret

.global cpu_getcr4
cpu_getcr4:
    movq %cr4, %rax
    ret

.global cpu_getrflags
cpu_getrflags:
    pushfq
    popq %rax
    ret

.global cpu_setcr0
cpu_setcr0:
    movq %rdi, %cr0
    ret

.global cpu_setcr3
cpu_setcr3:
    movq %rdi, %cr3
    ret

.global cpu_setcr4
cpu_setcr4:
    movq %rdi, %cr4
    ret

.global cpu_setgdt
cpu_setgdt:
    lgdt (%rdi)
    movw %dx, %ds
    movw %dx, %es
    movw %dx, %fs
    movw %dx, %gs
    movw %dx, %ss
    popq %rax
    pushq %rsi
    pushq %rax
    lretq

.global cpu_setidt
cpu_setidt:
    lidt (%rdi)
    ret

.global cpu_settss
cpu_settss:
    ltr %di
    ret

.global cpu_halt
cpu_halt:
    hlt
    jmp cpu_halt

.global cpu_leave
cpu_leave:
    pushq 32(%rsi)
    pushq 24(%rsi)
    pushq 16(%rsi)
    pushq 8(%rsi)
    pushq 0(%rsi)
    movq %rdi, %rax
    movq 0(%rax), %r15
    movq 8(%rax), %r14
    movq 16(%rax), %r13
    movq 24(%rax), %r12
    movq 32(%rax), %r11
    movq 40(%rax), %r10
    movq 48(%rax), %r9
    movq 56(%rax), %r8
    movq 64(%rax), %rdi
    movq 72(%rax), %rsi
    movq 80(%rax), %rbp
    movq 88(%rax), %rbx
    movq 96(%rax), %rdx
    movq 104(%rax), %rcx
    movq 112(%rax), %rax
    iretq
