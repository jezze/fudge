INIT_MBOOT_PAGE_ALIGN   equ 1 << 0
INIT_MBOOT_MEMORY_INFO  equ 1 << 1
INIT_MBOOT_HEADER_MAGIC equ 0x1BADB002
INIT_MBOOT_HEADER_FLAGS equ INIT_MBOOT_PAGE_ALIGN | INIT_MBOOT_MEMORY_INFO
INIT_MBOOT_CHECKSUM     equ - (INIT_MBOOT_HEADER_MAGIC + INIT_MBOOT_HEADER_FLAGS)
INIT_STACK_SIZE         equ 0x1000

extern arch_init

section .mboot
align 4

init_mboot:
    dd INIT_MBOOT_HEADER_MAGIC
    dd INIT_MBOOT_HEADER_FLAGS
    dd INIT_MBOOT_CHECKSUM

section .text
align 4

global init
init:
    cli
    mov esp, init_stack + INIT_STACK_SIZE
    push eax
    push ebx
    call arch_init
    hlt

section .bss
align 4

global init_stack
init_stack:
    resb INIT_STACK_SIZE
