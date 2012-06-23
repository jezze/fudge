.intel_syntax noprefix

.global fpu_enable
fpu_enable:
    finit
    ret

