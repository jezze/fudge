.code32

.extern arch_quicksyscall

.global syse_interrupt
syse_interrupt:
    pusha
    call arch_quicksyscall
    popa
    sysexit

