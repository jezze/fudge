.intel_syntax noprefix

.global apic_routine00
apic_routine00:
    cli
    push 0
    push 0x00
    jmp apic_common

.global apic_routine01
apic_routine01:
    cli
    push 0
    push 0x01
    jmp apic_common

.global apic_routine02
apic_routine02:
    cli
    push 0
    push 0x02
    jmp apic_common

.global apic_routine03
apic_routine03:
    cli
    push 0
    push 0x03
    jmp apic_common

.global apic_routine04
apic_routine04:
    cli
    push 0
    push 0x04
    jmp apic_common

.global apic_routine05
apic_routine05:
    cli
    push 0
    push 0x05
    jmp apic_common

.global apic_routine06
apic_routine06:
    cli
    push 0
    push 0x06
    jmp apic_common

.global apic_routine07
apic_routine07:
    cli
    push 0
    push 0x07
    jmp apic_common

.global apic_routine08
apic_routine08:
    cli
    push 1
    push 0x08
    jmp apic_common

.global apic_routine09
apic_routine09:
    cli
    push 1
    push 0x09
    jmp apic_common

.global apic_routine0A
apic_routine0A:
    cli
    push 1
    push 0x0A
    jmp apic_common

.global apic_routine0B
apic_routine0B:
    cli
    push 1
    push 0x0B
    jmp apic_common

.global apic_routine0C
apic_routine0C:
    cli
    push 1
    push 0x0C
    jmp apic_common

.global apic_routine0D
apic_routine0D:
    cli
    push 1
    push 0x0D
    jmp apic_common

.global apic_routine0E
apic_routine0E:
    cli
    push 1
    push 0x0E
    jmp apic_common

.global apic_routine0F
apic_routine0F:
    cli
    push 1
    push 0x0F
    jmp apic_common

apic_common:
    pusha
    mov ax, ds
    push eax
    mov eax, esp
    push eax
    mov ax, 0x10
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    call apic_interrupt
    add esp, 4
    pop eax
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    popa
    add esp, 8
    iret

