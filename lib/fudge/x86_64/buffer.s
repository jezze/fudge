.code64

.section .text

.global buffer_copy
buffer_copy:
    movl %edx, %ecx
    rep movsb
    ret

.global buffer_clear
buffer_clear:
    movl %esi, %ecx
    xorl %eax, %eax
    rep stosb
    ret
