.section .text

.globl atomic_testandset
atomic_testandset:
    amoswap.w.aq a0, a0, (a1)
    ret
