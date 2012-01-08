.intel_syntax noprefix

.global call_open
call_open:
    mov eax, 0x00
    int 0x80
    ret

.global call_close
call_close:
    mov eax, 0x01
    int 0x80
    ret

.global call_read
call_read:
    mov eax, 0x02
    int 0x80
    ret

.global call_write
call_write:
    mov eax, 0x03
    int 0x80
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
    mov eax, 0x11
    int 0x80
    ret

.global call_wait
call_wait:
    mov eax, 0x12
    int 0x80
    ret

.global call_load
call_load:
    mov eax, 0x16
    int 0x80
    ret

.global call_unload
call_unload:
    mov eax, 0x17
    int 0x80
    ret

.global call_halt
call_halt:
    mov eax, 0x20
    int 0x80
    ret

.global call_reboot
call_reboot:
    mov eax, 0x21
    int 0x80
    ret

.global call_attach
call_attach:
    mov eax, 0x30
    int 0x80
    ret

.global call_detach
call_detach:
    mov eax, 0x31
    int 0x80
    ret

