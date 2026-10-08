.code32

.section .text

# cstring_write_fmt(a, b, c, fmt, ...) calls cstring_write_fmtv(a, b, c, fmt, args), args pointing at the
# variadic arguments: on i386 they already lie on the stack after fmt.
.global cstring_write_fmt
cstring_write_fmt:
    leal 20(%esp), %eax
    pushl %eax
    pushl 20(%esp)
    pushl 20(%esp)
    pushl 20(%esp)
    pushl 20(%esp)
    call cstring_write_fmtv
    addl $20, %esp
    ret
