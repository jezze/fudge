.code64

.section .text

.global atomic_testandset
atomic_testandset:
    movl %edi, %eax
    lock xchgl %eax, (%rsi)
    ret
