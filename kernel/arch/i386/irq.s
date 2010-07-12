extern irq_handler

global irq0
irq0:
	cli
	push byte 0
	push byte 32
	jmp irq_common

global irq1
irq1:
	cli
	push byte 0
	push byte 33
	jmp irq_common

global irq2
irq2:
	cli
	push byte 0
	push byte 34
	jmp irq_common

global irq3
irq3:
	cli
	push byte 0
	push byte 35
	jmp irq_common

global irq4
irq4:
	cli
	push byte 0
	push byte 36
	jmp irq_common

global irq5
irq5:
	cli
	push byte 0
	push byte 37
	jmp irq_common

global irq6
irq6:
	cli
	push byte 0
	push byte 38
	jmp irq_common

global irq7
irq7:
	cli
	push byte 0
	push byte 39
	jmp irq_common

global irq8
irq8:
	cli
	push byte 0
	push byte 40
	jmp irq_common

global irq9
irq9:
	cli
	push byte 0
	push byte 41
	jmp irq_common

global irq10
irq10:
	cli
	push byte 0
	push byte 42
	jmp irq_common

global irq11
irq11:
	cli
	push byte 0
	push byte 43
	jmp irq_common

global irq12
irq12:
	cli
	push byte 0
	push byte 44
	jmp irq_common

global irq13
irq13:
	cli
	push byte 0
	push byte 45
	jmp irq_common

global irq14
irq14:
	cli
	push byte 0
	push byte 46
	jmp irq_common

global irq15
irq15:
	cli
	push byte 0
	push byte 47
	jmp irq_common

irq_common:
	pusha
	push ds
	push es
	push fs
	push gs
	mov ax, 0x10
	mov ds, ax
	mov es, ax
	mov fs, ax
	mov gs, ax
    mov eax, esp
    push eax
    call irq_handler
    pop eax
	pop gs
	pop fs
	pop es
	pop ds
	popa
	add esp, 8
	iret

