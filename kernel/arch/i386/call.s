global call_send
call_send:
    push ebp
    mov ebp, esp
    mov eax, [ebp + 8]
    mov ecx, [ebp + 12]
    int 0x80
    pop ebp
    ret

