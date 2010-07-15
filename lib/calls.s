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

global call_halt
call_halt:
    mov eax, 0x50
    int 0x80
    ret

global call_reboot
call_reboot:
    mov eax, 0x51
    int 0x80
    ret

