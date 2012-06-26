.intel_syntax noprefix

.global msr_get
msr_get:
    push ebp
    mov ebp, esp
    pusha
    mov ecx, [ebp + 8]
    mov edi, [ebp + 12]
    rdmsr
    mov [edi + 0], eax
    mov [edi + 4], edx
    popa
    pop ebp
    ret

.global msr_set
msr_set:
    push ebp
    mov ebp, esp
    pusha
    mov ecx, [ebp + 8]
    mov edi, [ebp + 12]
    mov eax, [edi + 0]
    mov edx, [edi + 4]
    wrmsr
    popa
    pop ebp
    ret

