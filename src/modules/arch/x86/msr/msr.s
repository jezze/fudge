.code32

.section .text

.global msr_get
msr_get:
    pushl %ebp
    movl %esp, %ebp
    pusha
    movl 8(%ebp), %ecx
    movl 12(%ebp), %edi
    rdmsr
    movl %eax, 0(%edi)
    movl %edx, 4(%edi)
    popa
    popl %ebp
    ret

.global msr_set
msr_set:
    pushl %ebp
    movl %esp, %ebp
    pusha
    movl 8(%ebp), %ecx
    movl 12(%ebp), %edi
    movl 0(%edi), %eax
    movl 4(%edi), %edx
    wrmsr
    popa
    popl %ebp
    ret

