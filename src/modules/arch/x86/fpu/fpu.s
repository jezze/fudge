.code32

.section .text

.global fpu_enable
fpu_enable:
    finit
    ret

