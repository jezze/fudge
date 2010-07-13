extern irq_handler

global irq00
irq00:
	cli
	push byte 0
	push byte 32
	jmp irq_common

global irq01
irq01:
	cli
	push byte 0
	push byte 33
	jmp irq_common

global irq02
irq02:
	cli
	push byte 0
	push byte 34
	jmp irq_common

global irq03
irq03:
	cli
	push byte 0
	push byte 35
	jmp irq_common

global irq04
irq04:
	cli
	push byte 0
	push byte 36
	jmp irq_common

global irq05
irq05:
	cli
	push byte 0
	push byte 37
	jmp irq_common

global irq06
irq06:
	cli
	push byte 0
	push byte 38
	jmp irq_common

global irq07
irq07:
	cli
	push byte 0
	push byte 39
	jmp irq_common

global irq08
irq08:
	cli
	push byte 0
	push byte 40
	jmp irq_common

global irq09
irq09:
	cli
	push byte 0
	push byte 41
	jmp irq_common

global irq0A
irq0A:
	cli
	push byte 0
	push byte 42
	jmp irq_common

global irq0B
irq0B:
	cli
	push byte 0
	push byte 43
	jmp irq_common

global irq0C
irq0C:
	cli
	push byte 0
	push byte 44
	jmp irq_common

global irq0D
irq0D:
	cli
	push byte 0
	push byte 45
	jmp irq_common

global irq0E
irq0E:
	cli
	push byte 0
	push byte 46
	jmp irq_common

global irq0F
irq0F:
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

