extern isr_handler

; Divide By Zero Exception
global isr00
isr00:
	cli
	push byte 0
	push byte 0
	jmp isr_common

; Debug Exception
global isr01
isr01:
	cli
	push byte 0
	push byte 1
	jmp isr_common

; Non Maskable Interrupt Exception
global isr02
isr02:
	cli
	push byte 0
	push byte 2
	jmp isr_common

; Int 3 Exception
global isr03
isr03:
	cli
	push byte 0
	push byte 3
	jmp isr_common

; INTO Exception
global isr04
isr04:
	cli
	push byte 0
	push byte 4
	jmp isr_common

; Out of Bounds Exception
global isr05
isr05:
	cli
	push byte 0
	push byte 5
	jmp isr_common

; Invalid Opcode Exception
global isr06
isr06:
	cli
	push byte 0
	push byte 6
	jmp isr_common

; Coprocessor Not Available Exception
global isr07
isr07:
	cli
	push byte 0
	push byte 7
	jmp isr_common

; Double Fault Exception
global isr08
isr08:
	cli
	push byte 8
	jmp isr_common

; Coprocessor Segment Overrun Exception
global isr09
isr09:
	cli
	push byte 0
	push byte 9
	jmp isr_common

; Bad TSS Exception
global isr0A
isr0A:
	cli
	push byte 10
	jmp isr_common

; Segment Not Present Exception
global isr0B
isr0B:
	cli
	push byte 11
	jmp isr_common

; Stack Fault Exception Exception
global isr0C
isr0C:
	cli
	push byte 12
	jmp isr_common

; General Protection Fault Exception
global isr0D
isr0D:
	cli
	push byte 13
	jmp isr_common

; Page Fault Exception
global isr0E
isr0E:
	cli
	push byte 14
	jmp isr_common

; Reserved Exception Exception
global isr0F
isr0F:
	cli
	push byte 0
	push byte 15
	jmp isr_common

; Floating Point Exception
global isr10
isr10:
	cli
	push byte 0
	push byte 16
	jmp isr_common

; Alignment Check Exception
global isr11
isr11:
	cli
	push byte 0
	push byte 17
	jmp isr_common

; Machine Check Exception
global isr12
isr12:
	cli
	push byte 0
	push byte 18
	jmp isr_common

; Reserved Exception
global isr13
isr13:
	cli
	push byte 0
	push byte 19
	jmp isr_common

; Reserved Exception
global isr14
isr14:
	cli
	push byte 0
	push byte 20
	jmp isr_common

; Reserved Exception
global isr15
isr15:
	cli
	push byte 0
	push byte 21
	jmp isr_common

; Reserved Exception
global isr16
isr16:
	cli
	push byte 0
	push byte 22
	jmp isr_common

; Reserved Exception
global isr17
isr17:
	cli
	push byte 0
	push byte 23
	jmp isr_common

; Reserved Exception
global isr18
isr18:
	cli
	push byte 0
	push byte 24
	jmp isr_common

; Reserved Exception
global isr19
isr19:
	cli
	push byte 0
	push byte 25
	jmp isr_common

; Reserved Exception
global isr1A
isr1A:
	cli
	push byte 0
	push byte 26
	jmp isr_common

; Reserved Exception
global isr1B
isr1B:
	cli
	push byte 0
	push byte 27
	jmp isr_common

; Reserved Exception
global isr1C
isr1C:
	cli
	push byte 0
	push byte 28
	jmp isr_common

; Reserved Exception
global isr1D
isr1D:
	cli
	push byte 0
	push byte 29
	jmp isr_common

; Reserved Exception
global isr1E
isr1E:
	cli
	push byte 0
	push byte 30
	jmp isr_common

; Reserved Exception
global isr1F
isr1F:
	cli
	push byte 0
	push byte 31
	jmp isr_common

isr_common:
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
	call isr_handler
	pop eax
	pop gs
	pop fs
	pop es
	pop ds
	popa
	add esp, 8
	iret

global isr_enable
isr_enable:
    sti
    ret

global isr_disable
isr_disable:
    cli
    ret

