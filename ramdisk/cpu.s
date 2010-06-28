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
    push edi
    push ebx
    push ecx
    push edx
    mov edi, [ebp + 8]
    mov eax, 0x0
    cpuid
    mov [edi + 0], ebx
    mov [edi + 4], edx
    mov [edi + 8], ecx
    pop edx
    pop ecx
    pop ebx
    pop edi
    pop ebp
    ret

