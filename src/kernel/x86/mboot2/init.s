.code32

.set STACK_SIZE,                        0x4000
.set MBOOT_MAGIC,                       0xE85250D6
.set MBOOT_ARCH_I386,                   0
.set MBOOT_REQUIRED,                    0
.set MBOOT_OPTIONAL,                    1
.set MBOOT_TAG_END,                     0
.set MBOOT_TAG_FRAMEBUFFER,             5
.set MBOOT_FRAMEBUFFER_WIDTH,           1024
.set MBOOT_FRAMEBUFFER_HEIGHT,          768
.set MBOOT_FRAMEBUFFER_DEPTH,           32

.section .text.boot

.align 8
.global _start
_start:
    jmp mboot_entry

.align 8
mboot_start:
.int MBOOT_MAGIC
.int MBOOT_ARCH_I386
.int mboot_end - mboot_start
.int 0x100000000 - (MBOOT_MAGIC + MBOOT_ARCH_I386 + (mboot_end - mboot_start))
.short MBOOT_TAG_FRAMEBUFFER
.short MBOOT_REQUIRED
.int 24
.int MBOOT_FRAMEBUFFER_WIDTH
.int MBOOT_FRAMEBUFFER_HEIGHT
.int MBOOT_FRAMEBUFFER_DEPTH
.int 0
.short MBOOT_TAG_END
.short MBOOT_REQUIRED
.int 8
mboot_end:

.align 8
mboot_entry:
    movl $(stack + STACK_SIZE), %esp
    pushl %eax
    pushl %ebx
    call mboot_setup
    hlt

.comm stack, STACK_SIZE

