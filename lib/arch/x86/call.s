.intel_syntax noprefix

.global call_open
call_open:
    mov eax, 0x01
    int 0x80
    ret

.global call_close
call_close:
    mov eax, 0x02
    int 0x80
    ret

.global call_read
call_read:
    mov eax, 0x03
    int 0x80
    ret

.global call_write
call_write:
    mov eax, 0x04
    int 0x80
    ret

.global call_execute
call_execute:
    mov eax, 0x05
    int 0x80
    ret

.global call_exit
call_exit:
    mov eax, 0x06
    int 0x80
    ret

.global call_wait
call_wait:
    mov eax, 0x07
    int 0x80
    ret

.global call_load
call_load:
    mov eax, 0x08
    int 0x80
    ret

.global call_unload
call_unload:
    mov eax, 0x09
    int 0x80
    ret

.global call_attach
call_attach:
    mov eax, 0x0A
    int 0x80
    ret

.global call_detach
call_detach:
    mov eax, 0x0B
    int 0x80
    ret

