.code32

.set STACK_SIZE,                        0x4000
.set MBOOT_MAGIC,                       0xE85250D6
.set MBOOT_ARCH_I386,                   0
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
.long MBOOT_MAGIC
.long MBOOT_ARCH_I386
.long mboot_end - mboot_start
.long -(MBOOT_MAGIC + MBOOT_ARCH_I386 + (mboot_end - mboot_start))
mboot_framebuffer_start:
.short MBOOT_TAG_FRAMEBUFFER
.short MBOOT_OPTIONAL
.long mboot_framebuffer_end - mboot_framebuffer_start
.long MBOOT_FRAMEBUFFER_WIDTH
.long MBOOT_FRAMEBUFFER_HEIGHT
.long MBOOT_FRAMEBUFFER_DEPTH
mboot_framebuffer_end:
.long MBOOT_TAG_END
mboot_end:
mboot_entry:
    movl $(stack + STACK_SIZE), %esp
    pushl %eax
    pushl %ebx
    call mboot_setup
    hlt

.comm stack, STACK_SIZE

