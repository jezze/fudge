INIT_MBOOT_HEADER_MAGIC equ 0x1BADB002
INIT_MBOOT_HEADER_FLAGS equ 0x00000001
INIT_MBOOT_CHECKSUM     equ - (INIT_MBOOT_HEADER_MAGIC + INIT_MBOOT_HEADER_FLAGS)
INIT_STACK_SIZE         equ 0x8000

extern arch_x86
extern arch_x86_init
extern kernel_init

section .mboot
align 4

dd INIT_MBOOT_HEADER_MAGIC
dd INIT_MBOOT_HEADER_FLAGS
dd INIT_MBOOT_CHECKSUM

section .text
align 4

global init
init:
    cli
    mov esp, stack + INIT_STACK_SIZE
    push esp
    push eax
    push ebx
    mov eax, arch_x86
    push eax
    call arch_x86_init
    call kernel_init
    hlt

section .bss
align 4

stack:
    resb INIT_STACK_SIZE
