global cpu_check_cpuid
cpu_check_cpuid:
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

global cpu_get_vendor
cpu_get_vendor:
    push ebp
    mov ebp, esp
    push eax
    push ebx
    push ecx
    push edx
    push esi
    push edi
    mov edi, [ebp + 8]
    mov eax, 0x00
    cpuid
    mov [edi + 0], ebx
    mov [edi + 4], edx
    mov [edi + 8], ecx
    pop edi
    pop esi
    pop edx
    pop ecx
    pop ebx
    pop eax
    pop ebp
    ret

global cpu_get_feature_ecx
cpu_get_feature_ecx:
    push ebp
    mov ebp, esp
    push ecx
    push edx
    mov eax, 0x01
    cpuid
    mov eax, ecx
    pop edx
    pop ecx
    pop ebp
    ret

global cpu_get_feature_edx
cpu_get_feature_edx:
    push ebp
    mov ebp, esp
    push ecx
    push edx
    mov eax, 0x01
    cpuid
    mov eax, edx
    pop edx
    pop ecx
    pop ebp
    ret

global cpu_get_brand
cpu_get_brand:
    push ebp
    mov ebp, esp
    push eax
    push ebx
    push ecx
    push edx
    push esi
    push edi
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
    pop edi
    pop esi
    pop edx
    pop ecx
    pop ebx
    pop eax
    pop ebp
    ret

