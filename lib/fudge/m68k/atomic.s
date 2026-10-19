.section .text

.global atomic_testandset
atomic_testandset:
    move.l 8(%sp), %a0
    move.l (%a0), %d0
1:
    move.l 4(%sp), %d1
    cas.l %d0, %d1, (%a0)
    bne 1b
    rts

