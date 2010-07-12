extern syscall_handler

global syscall_interrupt
syscall_interrupt:
    cli
    push edi
    push esi
    push edx
    push ecx
    push ebx
    push eax
    call syscall_handler
    add esp, 24
	iret

