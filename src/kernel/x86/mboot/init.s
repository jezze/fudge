.code32

.set STACK_SIZE,                        0x4000
.set MBOOT_MAGIC,                       0x1BADB002
.set MBOOT_FLAGS,                       0x00000007
.set MBOOT_CHECKSUM,                    0x00000000 - (MBOOT_MAGIC + MBOOT_FLAGS)
.set MBOOT_HEADER_BASE,                 0x00000000
.set MBOOT_LOAD_BASE,                   0x00000000
.set MBOOT_LOAD_END_BASE,               0x00000000
.set MBOOT_BSS_END_BASE,                0x00000000
.set MBOOT_ENTRY_BASE,                  0x00000000
.set MBOOT_FRAMEBUFFER_MODE,            0
.set MBOOT_FRAMEBUFFER_WIDTH,           1920
.set MBOOT_FRAMEBUFFER_HEIGHT,          1080
.set MBOOT_FRAMEBUFFER_DEPTH,           32

.section .text.boot

.int MBOOT_MAGIC
.int MBOOT_FLAGS
.int MBOOT_CHECKSUM
.int MBOOT_HEADER_BASE
.int MBOOT_LOAD_BASE
.int MBOOT_LOAD_END_BASE
.int MBOOT_BSS_END_BASE
.int MBOOT_ENTRY_BASE
.int MBOOT_FRAMEBUFFER_MODE
.int MBOOT_FRAMEBUFFER_WIDTH
.int MBOOT_FRAMEBUFFER_HEIGHT
.int MBOOT_FRAMEBUFFER_DEPTH

.section .text

.global _start
_start:
.global mboot_init
mboot_init:
    movl $(stack + STACK_SIZE), %esp
    pushl %eax
    pushl %ebx
    call mboot_setup
    hlt

.comm stack, STACK_SIZE

