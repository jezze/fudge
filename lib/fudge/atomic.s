.code32

.section .text

.global atomic_testandset
atomic_testandset:
    movl 4(%esp), %eax
    movl 8(%esp), %edx
    lock xchgl %eax, (%edx)
    ret

