.intel_syntax noprefix

.extern isr_handle_cpu
.extern isr_save_state
.extern isr_load_state
.extern arch_get_context

.global isr_routine00
isr_routine00:
    cli
    push 0
    push 0x00
    jmp isr_common_cpu

.global isr_routine01
isr_routine01:
    cli
    push 0
    push 0x01
    jmp isr_common_cpu

.global isr_routine02
isr_routine02:
    cli
    push 0
    push 0x02
    jmp isr_common_cpu

.global isr_routine03
isr_routine03:
    cli
    push 0
    push 0x03
    jmp isr_common_cpu

.global isr_routine04
isr_routine04:
    cli
    push 0
    push 0x04
    jmp isr_common_cpu

.global isr_routine05
isr_routine05:
    cli
    push 0
    push 0x05
    jmp isr_common_cpu

.global isr_routine06
isr_routine06:
    cli
    push 0
    push 0x06
    jmp isr_common_cpu

.global isr_routine07
isr_routine07:
    cli
    push 0
    push 0x07
    jmp isr_common_cpu

.global isr_routine08
isr_routine08:
    cli
    push 0x08
    jmp isr_common_cpu

.global isr_routine09
isr_routine09:
    cli
    push 0
    push 0x09
    jmp isr_common_cpu

.global isr_routine0A
isr_routine0A:
    cli
    push 0x0A
    jmp isr_common_cpu

.global isr_routine0B
isr_routine0B:
    cli
    push 0x0B
    jmp isr_common_cpu

.global isr_routine0C
isr_routine0C:
    cli
    push 0x0C
    jmp isr_common_cpu

.global isr_routine0D
isr_routine0D:
    cli
    push 0x0D
    jmp isr_common_cpu

.global isr_routine0E
isr_routine0E:
    cli
    push 0x0E
    jmp isr_common_cpu

.global isr_routine0F
isr_routine0F:
    cli
    push 0
    push 0x0F
    jmp isr_common_cpu

.global isr_routine10
isr_routine10:
    cli
    push 0
    push 0x10
    jmp isr_common_cpu

.global isr_routine11
isr_routine11:
    cli
    push 0
    push 0x11
    jmp isr_common_cpu

.global isr_routine12
isr_routine12:
    cli
    push 0
    push 0x12
    jmp isr_common_cpu

.global isr_routine13
isr_routine13:
    cli
    push 0
    push 0x13
    jmp isr_common_cpu

.global isr_routine14
isr_routine14:
    cli
    push 0
    push 0x14
    jmp isr_common_cpu

.global isr_routine15
isr_routine15:
    cli
    push 0
    push 0x15
    jmp isr_common_cpu

.global isr_routine16
isr_routine16:
    cli
    push 0
    push 0x16
    jmp isr_common_cpu

.global isr_routine17
isr_routine17:
    cli
    push 0
    push 0x17
    jmp isr_common_cpu

.global isr_routine18
isr_routine18:
    cli
    push 0
    push 0x18
    jmp isr_common_cpu

.global isr_routine19
isr_routine19:
    cli
    push 0
    push 0x19
    jmp isr_common_cpu

.global isr_routine1A
isr_routine1A:
    cli
    push 0
    push 0x1A
    jmp isr_common_cpu

.global isr_routine1B
isr_routine1B:
    cli
    push 0
    push 0x1B
    jmp isr_common_cpu

.global isr_routine1C
isr_routine1C:
    cli
    push 0
    push 0x1C
    jmp isr_common_cpu

.global isr_routine1D
isr_routine1D:
    cli
    push 0
    push 0x1D
    jmp isr_common_cpu

.global isr_routine1E
isr_routine1E:
    cli
    push 0
    push 0x1E
    jmp isr_common_cpu

.global isr_routine1F
isr_routine1F:
    cli
    push 0
    push 0x1F
    jmp isr_common_cpu

.global isr_routine20
isr_routine20:
    cli
    push 0
    push 0x20
    jmp isr_common_cpu

.global isr_routine21
isr_routine21:
    cli
    push 0
    push 0x21
    jmp isr_common_cpu

.global isr_routine22
isr_routine22:
    cli
    push 0
    push 0x22
    jmp isr_common_cpu

.global isr_routine23
isr_routine23:
    cli
    push 0
    push 0x23
    jmp isr_common_cpu

.global isr_routine24
isr_routine24:
    cli
    push 0
    push 0x24
    jmp isr_common_cpu

.global isr_routine25
isr_routine25:
    cli
    push 0
    push 0x25
    jmp isr_common_cpu

.global isr_routine26
isr_routine26:
    cli
    push 0
    push 0x26
    jmp isr_common_cpu

.global isr_routine27
isr_routine27:
    cli
    push 0
    push 0x27
    jmp isr_common_cpu

.global isr_routine28
isr_routine28:
    cli
    push 1
    push 0x28
    jmp isr_common_cpu

.global isr_routine29
isr_routine29:
    cli
    push 1
    push 0x29
    jmp isr_common_cpu

.global isr_routine2A
isr_routine2A:
    cli
    push 1
    push 0x2A
    jmp isr_common_cpu

.global isr_routine2B
isr_routine2B:
    cli
    push 1
    push 0x2B
    jmp isr_common_cpu

.global isr_routine2C
isr_routine2C:
    cli
    push 1
    push 0x2C
    jmp isr_common_cpu

.global isr_routine2D
isr_routine2D:
    cli
    push 1
    push 0x2D
    jmp isr_common_cpu

.global isr_routine2E
isr_routine2E:
    cli
    push 1
    push 0x2E
    jmp isr_common_cpu

.global isr_routine2F
isr_routine2F:
    cli
    push 1
    push 0x2F
    jmp isr_common_cpu

.global isr_routine80
isr_routine80:
    cli
    push eax
    push 0x80
    jmp isr_common_cpu

isr_common_cpu:
    pusha
    mov eax, esp
    push eax
    mov ax, 0x10
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    call arch_get_context
    push eax
    call isr_save_state
    call isr_handle_cpu
    call isr_load_state
    add esp, 4
    mov ax, 0x23
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    add esp, 4
    popa
    add esp, 8
    iret

