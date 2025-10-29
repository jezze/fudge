.code32

.section .text

.global buffer_copy
buffer_copy:
    push %edi
    push %esi
    movl 12(%esp), %edi
    movl 16(%esp), %esi
    movl 20(%esp), %ecx
    rep movsb
    pop %esi
    pop %edi
    ret

.global buffer_clear
buffer_clear:
    push %edi
    movl 8(%esp), %edi
    movl 12(%esp), %ecx
    movl $0, %eax
    rep stosb
    pop %edi
    ret

