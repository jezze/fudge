.intel_syntax noprefix

.global msr_get
msr_get:
    push ebp
    mov ebp, esp
    push eax
    push ecx
    push edx
    mov ecx, [ebp + 8]
    rdmsr
    mov [ebp + 12], eax
    mov [ebp + 16], edx
    pop edx
    pop ecx
    pop eax
    pop ebp
    ret

.global msr_set
msr_set:
    push ebp
    mov ebp, esp
    push eax
    push ecx
    push edx
    mov ecx, [ebp + 8]
    mov eax, [ebp + 12]
    mov edx, [ebp + 16]
    wrmsr
    pop edx
    pop ecx
    pop eax
    pop ebp
    ret

