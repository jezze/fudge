global call_puts
call_puts:
    mov eax, 0x01
    mov ecx, [esp + 4]
    int 0x80
    ret

global call_puts_dec
call_puts_dec:
    mov eax, 0x02
    mov ecx, [esp + 4]
    int 0x80
    ret

global call_puts_hex
call_puts_hex:
    mov eax, 0x03
    mov ecx, [esp + 4]
    int 0x80
    ret

global call_vfs_read
call_vfs_read:
    mov eax, 0x10
    mov ebx, [esp + 4]
    mov edi, [esp + 8]
    int 0x80
    ret

global call_vfs_walk
call_vfs_walk:
    mov eax, 0x12
    mov ecx, [esp + 4]
    int 0x80
    ret

global call_vfs_find
call_vfs_find:
    mov eax, 0x13
    mov ecx, [esp + 4]
    int 0x80
    ret

