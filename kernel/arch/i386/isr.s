extern isr_handler
extern syscall_handler

; Divide By Zero Exception
global isr0
isr0:
	cli
	push byte 0
	push byte 0
	jmp isr_common

; Debug Exception
global isr1
isr1:
	cli
	push byte 0
	push byte 1
	jmp isr_common

; Non Maskable Interrupt Exception
global isr2
isr2:
	cli
	push byte 0
	push byte 2
	jmp isr_common

; Int 3 Exception
global isr3
isr3:
	cli
	push byte 0
	push byte 3
	jmp isr_common

; INTO Exception
global isr4
isr4:
	cli
	push byte 0
	push byte 4
	jmp isr_common

; Out of Bounds Exception
global isr5
isr5:
	cli
	push byte 0
	push byte 5
	jmp isr_common

; Invalid Opcode Exception
global isr6
isr6:
	cli
	push byte 0
	push byte 6
	jmp isr_common

; Coprocessor Not Available Exception
global isr7
isr7:
	cli
	push byte 0
	push byte 7
	jmp isr_common

; Double Fault Exception
global isr8
isr8:
	cli
	push byte 8
	jmp isr_common

; Coprocessor Segment Overrun Exception
global isr9
isr9:
	cli
	push byte 0
	push byte 9
	jmp isr_common

; Bad TSS Exception
global isr10
isr10:
	cli
	push byte 10
	jmp isr_common

; Segment Not Present Exception
global isr11
isr11:
	cli
	push byte 11
	jmp isr_common

; Stack Fault Exception Exception
global isr12
isr12:
	cli
	push byte 12
	jmp isr_common

; General Protection Fault Exception
global isr13
isr13:
	cli
	push byte 13
	jmp isr_common

; Page Fault Exception
global isr14
isr14:
	cli
	push byte 14
	jmp isr_common

; Reserved Exception Exception
global isr15
isr15:
	cli
	push byte 0
	push byte 15
	jmp isr_common

; Floating Point Exception
global isr16
isr16:
	cli
	push byte 0
	push byte 16
	jmp isr_common

; Alignment Check Exception
global isr17
isr17:
	cli
	push byte 0
	push byte 17
	jmp isr_common

; Machine Check Exception
global isr18
isr18:
	cli
	push byte 0
	push byte 18
	jmp isr_common

; Reserved Exception
global isr19
isr19:
	cli
	push byte 0
	push byte 19
	jmp isr_common

; Reserved Exception
global isr20
isr20:
	cli
	push byte 0
	push byte 20
	jmp isr_common

; Reserved Exception
global isr21
isr21:
	cli
	push byte 0
	push byte 21
	jmp isr_common

; Reserved Exception
global isr22
isr22:
	cli
	push byte 0
	push byte 22
	jmp isr_common

; Reserved Exception
global isr23
isr23:
	cli
	push byte 0
	push byte 23
	jmp isr_common

; Reserved Exception
global isr24
isr24:
	cli
	push byte 0
	push byte 24
	jmp isr_common

; Reserved Exception
global isr25
isr25:
	cli
	push byte 0
	push byte 25
	jmp isr_common

; Reserved Exception
global isr26
isr26:
	cli
	push byte 0
	push byte 26
	jmp isr_common

; Reserved Exception
global isr27
isr27:
	cli
	push byte 0
	push byte 27
	jmp isr_common

; Reserved Exception
global isr28
isr28:
	cli
	push byte 0
	push byte 28
	jmp isr_common

; Reserved Exception
global isr29
isr29:
	cli
	push byte 0
	push byte 29
	jmp isr_common

; Reserved Exception
global isr30
isr30:
	cli
	push byte 0
	push byte 30
	jmp isr_common

; Reserved Exception
global isr31
isr31:
	cli
	push byte 0
	push byte 31
	jmp isr_common

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

