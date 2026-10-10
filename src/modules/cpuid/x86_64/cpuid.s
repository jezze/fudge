.code64

.section .text

.global cpuid_exist
cpuid_exist:
    movl $1, %eax
    ret

.global cpuid_getdata
cpuid_getdata:
    pushq %rbx
    movl %edi, %eax
    cpuid
    movl %eax, 0(%rsi)
    movl %ebx, 4(%rsi)
    movl %ecx, 8(%rsi)
    movl %edx, 12(%rsi)
    popq %rbx
    ret

.global cpuid_getvendor
cpuid_getvendor:
    pushq %rbx
    movl $0x00, %eax
    cpuid
    movl %ebx, 0(%rdi)
    movl %edx, 4(%rdi)
    movl %ecx, 8(%rdi)
    popq %rbx
    ret

.global cpuid_getbrand
cpuid_getbrand:
    pushq %rbx
    movl $0x80000002, %eax
    cpuid
    movl %eax, 0(%rdi)
    movl %ebx, 4(%rdi)
    movl %ecx, 8(%rdi)
    movl %edx, 12(%rdi)
    movl $0x80000003, %eax
    cpuid
    movl %eax, 16(%rdi)
    movl %ebx, 20(%rdi)
    movl %ecx, 24(%rdi)
    movl %edx, 28(%rdi)
    movl $0x80000004, %eax
    cpuid
    movl %eax, 32(%rdi)
    movl %ebx, 36(%rdi)
    movl %ecx, 40(%rdi)
    movl %edx, 44(%rdi)
    popq %rbx
    ret
