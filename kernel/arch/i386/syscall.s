extern syscall_handler

global syscall
syscall:
    cli
    push edi
    push edx
    push ecx
    push ebx
    push eax
    call syscall_handler
    add esp, 20
	iret

