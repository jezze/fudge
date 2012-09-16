.intel_syntax noprefix

.extern isr_common

.global apic_routine00
apic_routine00:
    cli
    push 0
    push 0x20
    jmp isr_common

.global apic_routine01
apic_routine01:
    cli
    push 0
    push 0x21
    jmp isr_common

.global apic_routine02
apic_routine02:
    cli
    push 0
    push 0x22
    jmp isr_common

.global apic_routine03
apic_routine03:
    cli
    push 0
    push 0x23
    jmp isr_common

.global apic_routine04
apic_routine04:
    cli
    push 0
    push 0x24
    jmp isr_common

.global apic_routine05
apic_routine05:
    cli
    push 0
    push 0x25
    jmp isr_common

.global apic_routine06
apic_routine06:
    cli
    push 0
    push 0x26
    jmp isr_common

.global apic_routine07
apic_routine07:
    cli
    push 0
    push 0x27
    jmp isr_common

.global apic_routine08
apic_routine08:
    cli
    push 1
    push 0x28
    jmp isr_common

.global apic_routine09
apic_routine09:
    cli
    push 1
    push 0x29
    jmp isr_common

.global apic_routine0A
apic_routine0A:
    cli
    push 1
    push 0x2A
    jmp isr_common

.global apic_routine0B
apic_routine0B:
    cli
    push 1
    push 0x2B
    jmp isr_common

.global apic_routine0C
apic_routine0C:
    cli
    push 1
    push 0x2C
    jmp isr_common

.global apic_routine0D
apic_routine0D:
    cli
    push 1
    push 0x2D
    jmp isr_common

.global apic_routine0E
apic_routine0E:
    cli
    push 1
    push 0x2E
    jmp isr_common

.global apic_routine0F
apic_routine0F:
    cli
    push 1
    push 0x2F
    jmp isr_common

