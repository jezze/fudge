global call_puts
call_puts:
    push ebp
    mov ebp, esp
    mov eax, 0x01
    mov ecx, [ebp + 8]
    int 0x80
    pop ebp
    ret

global call_puts_dec
call_puts_dec:
    push ebp
    mov ebp, esp
    mov eax, 0x02
    mov ecx, [ebp + 8]
    int 0x80
    pop ebp
    ret

global call_puts_hex
call_puts_hex:
    push ebp
    mov ebp, esp
    mov eax, 0x03
    mov ecx, [ebp + 8]
    int 0x80
    pop ebp
    ret

global call_vfs_read
call_vfs_read:
    push ebp
    mov ebp, esp
    mov eax, 0x10
    mov ebx, [ebp + 8]
    mov edi, [ebp + 12]
    int 0x80
    pop ebp
    ret

global call_vfs_walk
call_vfs_walk:
    push ebp
    mov ebp, esp
    mov eax, 0x12
    mov ecx, [ebp + 8]
    int 0x80
    pop ebp
    ret

global call_vfs_find
call_vfs_find:
    push ebp
    mov ebp, esp
    mov eax, 0x13
    mov ecx, [ebp + 8]
    int 0x80
    pop ebp
    ret

