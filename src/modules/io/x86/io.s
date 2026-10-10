.code32

.section .text

.global io_inb
io_inb:
    movw 4(%esp), %dx
    xorl %eax, %eax
    inb %dx, %al
    ret

.global io_inw
io_inw:
    movw 4(%esp), %dx
    xorl %eax, %eax
    inw %dx, %ax
    ret

.global io_ind
io_ind:
    movw 4(%esp), %dx
    inl %dx, %eax
    ret

.global io_outb
io_outb:
    movw 4(%esp), %dx
    movb 8(%esp), %al
    outb %al, %dx
    ret

.global io_outw
io_outw:
    movw 4(%esp), %dx
    movw 8(%esp), %ax
    outw %ax, %dx
    ret

.global io_outd
io_outd:
    movw 4(%esp), %dx
    movl 8(%esp), %eax
    outl %eax, %dx
    ret

