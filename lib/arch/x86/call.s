.intel_syntax noprefix

.global call_open
call_open:
    push ebp
    mov ebp, esp
    push ebx
    push ecx
    push esi
    push edi
    mov eax, 0x00
    mov esi, [ebp + 8]
    mov edi, [ebp + 12]
    int 0x80
    pop edi
    pop esi
    pop ecx
    pop ebx
    mov esp, ebp
    pop ebp
    ret

.global call_close
call_close:
    push ebp
    mov ebp, esp
    push ebx
    push ecx
    push esi
    push edi
    mov eax, 0x01
    mov ebx, [ebp + 8]
    int 0x80
    pop edi
    pop esi
    pop ecx
    pop ebx
    mov esp, ebp
    pop ebp
    ret

.global call_read
call_read:
    push ebp
    mov ebp, esp
    push ebx
    push ecx
    push esi
    push edi
    mov eax, 0x02
    mov ebx, [ebp + 8]
    mov esi, [ebp + 12]
    mov ecx, [ebp + 16]
    int 0x80
    pop edi
    pop esi
    pop ecx
    pop ebx
    mov esp, ebp
    pop ebp
    ret

.global call_write
call_write:
    push ebp
    mov ebp, esp
    push ebx
    push ecx
    push esi
    push edi
    mov eax, 0x03
    mov ebx, [ebp + 8]
    mov esi, [ebp + 12]
    mov ecx, [ebp + 16]
    int 0x80
    pop edi
    pop esi
    pop ecx
    pop ebx
    mov esp, ebp
    pop ebp
    ret

.global call_execute
call_execute:
    push ebp
    mov ebp, esp
    push ebx
    push ecx
    push esi
    push edi
    mov eax, 0x10
    mov esi, [ebp + 8]
    mov ecx, [ebp + 12]
    mov ebx, [ebp + 16]
    int 0x80
    pop edi
    pop esi
    pop ecx
    pop ebx
    mov esp, ebp
    pop ebp
    ret

.global call_exit
call_exit:
    push ebp
    mov ebp, esp
    push ebx
    push ecx
    push esi
    push edi
    mov eax, 0x11
    int 0x80
    pop edi
    pop esi
    pop ecx
    pop ebx
    mov esp, ebp
    pop ebp
    ret

.global call_wait
call_wait:
    push ebp
    mov ebp, esp
    push ebx
    push ecx
    push esi
    push edi
    mov eax, 0x12
    int 0x80
    pop edi
    pop esi
    pop ecx
    pop ebx
    mov esp, ebp
    pop ebp
    ret

.global call_load
call_load:
    mov eax, 0x16
    int 0x81
    ret

.global call_unload
call_unload:
    push ebp
    mov ebp, esp
    push ebx
    push ecx
    push esi
    push edi
    mov eax, 0x17
    mov esi, [ebp + 8]
    int 0x80
    pop edi
    pop esi
    pop ecx
    pop ebx
    mov esp, ebp
    pop ebp
    ret

.global call_halt
call_halt:
    push ebp
    mov ebp, esp
    push ebx
    push ecx
    push esi
    push edi
    mov eax, 0x20
    int 0x80
    pop edi
    pop esi
    pop ecx
    pop ebx
    mov esp, ebp
    pop ebp
    ret

.global call_reboot
call_reboot:
    push ebp
    mov ebp, esp
    push ebx
    push ecx
    push esi
    push edi
    mov eax, 0x21
    int 0x80
    pop edi
    pop esi
    pop ecx
    pop ebx
    mov esp, ebp
    pop ebp
    ret

.global call_attach
call_attach:
    push ebp
    mov ebp, esp
    push ebx
    push ecx
    push esi
    push edi
    mov eax, 0x30
    mov ebx, [ebp + 8]
    mov ecx, [ebp + 12]
    int 0x80
    pop edi
    pop esi
    pop ecx
    pop ebx
    mov esp, ebp
    pop ebp
    ret

.global call_detach
call_detach:
    push ebp
    mov ebp, esp
    push ebx
    push ecx
    push esi
    push edi
    mov eax, 0x31
    mov ebx, [ebp + 8]
    int 0x80
    pop edi
    pop esi
    pop ecx
    pop ebx
    mov esp, ebp
    pop ebp
    ret

