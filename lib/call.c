#include <lib/call.h>
#include <lib/string.h>
#include <lib/vfs.h>

char call_getc()
{

    struct vfs_node *node = call_vfs_find("dev/stdin");

    if (!node)
        return 0;

    char c;

    while (!(vfs_read(node, 0, 1, &c)));

    return c;

}

int call_putc(char c)
{

    struct vfs_node *node = call_vfs_find("dev/stdout");

    if (!node)
        return 0;

    return vfs_write(node, 0, 1, &c);

}

int call_puts(char *s)
{

    struct vfs_node *node = call_vfs_find("dev/stdout");

    if (!node)
        return 0;

    return vfs_write(node, 0, string_length(s), s);

}

void call_puts_num(unsigned int n, unsigned int base)
{

    if (!n)
    {

        call_putc('0');
        return;

    }

    char s[32] = {0};

    int i;

    for (i = 30; n && i; --i, n /= base)
        s[i] = "0123456789abcdef"[n % base];

    call_puts(s + i + 1);

}

void call_puts_dec(unsigned int n)
{

    call_puts_num(n, 10);

}

void call_puts_hex(unsigned int n)
{

    call_puts_num(n, 16);

}

void call_puts_bcd(unsigned char n)
{

    call_puts_dec(n >> 4);
    call_puts_dec(n & 0x0F);

}

