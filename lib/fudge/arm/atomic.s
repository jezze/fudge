.section .text

.global atomic_testandset
.type atomic_testandset, %function
atomic_testandset:
    swp r0, r0, [r1]
    bx lr
