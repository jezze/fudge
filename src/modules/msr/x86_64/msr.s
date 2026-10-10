.code64

.section .text

.global msr_get
msr_get:
    movl %edi, %ecx
    rdmsr
    movl %eax, 0(%rsi)
    movl %edx, 4(%rsi)
    ret

.global msr_set
msr_set:
    movl %edi, %ecx
    movl 0(%rsi), %eax
    movl 4(%rsi), %edx
    wrmsr
    ret
