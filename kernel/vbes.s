extern vbe_set_controller

global sum
sum:
    push ebp
    mov ebp, esp
    mov eax, [ebp + 8]
    mov ecx, [ebp + 12]
    add ecx, eax
    push ecx
    mov eax, vbe_set_controller
    call eax
    pop ecx
    mov eax, ecx
    pop ebp
	ret

