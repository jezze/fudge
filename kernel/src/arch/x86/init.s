.intel_syntax noprefix

.set INIT_MBOOT_HEADER_MAGIC, 0x1BADB002
.set INIT_MBOOT_HEADER_FLAGS, 0x00000001
.set INIT_MBOOT_CHECKSUM, - (INIT_MBOOT_HEADER_MAGIC + INIT_MBOOT_HEADER_FLAGS)

.extern arch_setup

.section .mboot

.int INIT_MBOOT_HEADER_MAGIC
.int INIT_MBOOT_HEADER_FLAGS
.int INIT_MBOOT_CHECKSUM

.section .text

.global init
init:
    cli
    push eax
    push ebx
    call arch_setup
    hlt

