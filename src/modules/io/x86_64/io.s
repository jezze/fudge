.code64

.section .text

.global io_inb
io_inb:
    movw %di, %dx
    xorl %eax, %eax
    inb %dx, %al
    ret

.global io_inw
io_inw:
    movw %di, %dx
    xorl %eax, %eax
    inw %dx, %ax
    ret

.global io_ind
io_ind:
    movw %di, %dx
    inl %dx, %eax
    ret

.global io_outb
io_outb:
    movw %di, %dx
    movl %esi, %eax
    outb %al, %dx
    ret

.global io_outw
io_outw:
    movw %di, %dx
    movl %esi, %eax
    outw %ax, %dx
    ret

.global io_outd
io_outd:
    movw %di, %dx
    movl %esi, %eax
    outl %eax, %dx
    ret
