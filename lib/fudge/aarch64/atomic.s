.section .text

.globl atomic_testandset
atomic_testandset:
    ldaxr w2, [x1]
    stlxr w3, w0, [x1]
    cbnz w3, atomic_testandset
    mov w0, w2
    ret

