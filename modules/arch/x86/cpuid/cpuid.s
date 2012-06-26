.intel_syntax noprefix

.global cpuid_check_cpuid
cpuid_check_cpuid:
    pushfd
    pop eax
    mov ecx, eax
    xor eax, 0x200000
    push eax
    popfd
    pushfd
    pop eax
    xor eax, ecx
    shr eax, 21
    and eax, 1
    ret

.global cpuid_fill
cpuid_fill:
    push ebp
    mov ebp, esp
    pusha
    mov eax, [ebp + 8]
    cpuid
    mov edi, [ebp + 12]
    mov [edi + 0], eax
    mov [edi + 4], ebx
    mov [edi + 8], ecx
    mov [edi + 12], edx
    popa
    pop ebp
    ret

.global cpuid_get_vendor
cpuid_get_vendor:
    push ebp
    mov ebp, esp
    pusha
    mov edi, [ebp + 8]
    mov eax, 0x00
    cpuid
    mov [edi + 0], ebx
    mov [edi + 4], edx
    mov [edi + 8], ecx
    popa
    pop ebp
    ret

.global cpuid_get_brand
cpuid_get_brand:
    push ebp
    mov ebp, esp
    pusha
    mov edi, [ebp + 8]
    mov eax, 0x80000002
    cpuid
    mov [edi + 0], eax
    mov [edi + 4], ebx
    mov [edi + 8], ecx
    mov [edi + 12], edx
    mov eax, 0x80000003
    cpuid
    mov [edi + 16], eax
    mov [edi + 20], ebx
    mov [edi + 24], ecx
    mov [edi + 28], edx
    mov eax, 0x80000004
    cpuid
    mov [edi + 32], eax
    mov [edi + 36], ebx
    mov [edi + 40], ecx
    mov [edi + 44], edx
    popa
    pop ebp
    ret

