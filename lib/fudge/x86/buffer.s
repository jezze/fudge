.code32

.section .text

.global buffer_copy
buffer_copy:
    push %ecx
    push %esi
    push %edi
    movl 16(%esp), %edi
    movl 20(%esp), %esi
    movl 24(%esp), %ecx
    rep movsb
    pop %edi
    pop %esi
    pop %ecx
    ret

