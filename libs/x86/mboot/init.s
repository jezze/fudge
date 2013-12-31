.code32

.set INIT_MBOOT_HEADER_MAGIC,           0x1BADB002
.set INIT_MBOOT_HEADER_FLAGS,           0x00000001
.set INIT_MBOOT_CHECKSUM,               0x00000000 - (INIT_MBOOT_HEADER_MAGIC + INIT_MBOOT_HEADER_FLAGS)

.extern mboot_setup

.section .mboot

.int INIT_MBOOT_HEADER_MAGIC
.int INIT_MBOOT_HEADER_FLAGS
.int INIT_MBOOT_CHECKSUM

.section .text

.global init
init:
    cli
    pushl %eax
    pushl %ebx
    call mboot_setup
    hlt

.section .data

.section .bss

