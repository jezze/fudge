.code32

.set INIT_MBOOT_HEADER_MAGIC,           0x1BADB002
.set INIT_MBOOT_HEADER_FLAGS,           0x00000007
.set INIT_MBOOT_CHECKSUM,               0x00000000 - (INIT_MBOOT_HEADER_MAGIC + INIT_MBOOT_HEADER_FLAGS)
.set INIT_MBOOT_HEADER_BASE,            0x00000000
.set INIT_MBOOT_LOAD_BASE,              0x00000000
.set INIT_MBOOT_LOAD_END_BASE,          0x00000000
.set INIT_MBOOT_BSS_END_BASE,           0x00000000
.set INIT_MBOOT_ENTRY_BASE,             0x00000000
.set INIT_MBOOT_MODE,                   0
.set INIT_MBOOT_WIDTH,                  1920
.set INIT_MBOOT_HEIGHT,                 1080
.set INIT_MBOOT_DEPTH,                  32

.section .text.boot

.int INIT_MBOOT_HEADER_MAGIC
.int INIT_MBOOT_HEADER_FLAGS
.int INIT_MBOOT_CHECKSUM
.int INIT_MBOOT_HEADER_BASE
.int INIT_MBOOT_LOAD_BASE
.int INIT_MBOOT_LOAD_END_BASE
.int INIT_MBOOT_BSS_END_BASE
.int INIT_MBOOT_ENTRY_BASE
.int INIT_MBOOT_MODE
.int INIT_MBOOT_WIDTH
.int INIT_MBOOT_HEIGHT
.int INIT_MBOOT_DEPTH

.section .text

.global _start
_start:
.global mboot_init
mboot_init:
    pushl %eax
    pushl %ebx
    call mboot_setup
    hlt

