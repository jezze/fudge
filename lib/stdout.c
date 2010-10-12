#include <lib/call.h>
#include <lib/stdout.h>
#include <lib/string.h>
#include <lib/vfs.h>

int stdout_write_single(char c)
{

    struct vfs_node *node = call_open("dev/stdout");

    if (!node)
        return 0;

    return vfs_write(node, 0, 1, &c);

}

int stdout_write(char *s)
{

    struct vfs_node *node = call_open("dev/stdout");

    if (!node)
        return 0;

    return vfs_write(node, 0, string_length(s), s);

}

void stdout_write_num(unsigned int n, unsigned int base)
{

    if (!n)
    {

        stdout_write_single('0');

        return;

    }

    char s[32] = {0};

    int i;

    for (i = 30; n && i; --i, n /= base)
        s[i] = "0123456789abcdef"[n % base];

    stdout_write(s + i + 1);

}

void stdout_write_dec(unsigned int n)
{

    stdout_write_num(n, 10);

}

void stdout_write_hex(unsigned int n)
{

    stdout_write_num(n, 16);

}

void stdout_write_bcd(unsigned char n)
{

    stdout_write_dec(n >> 4);
    stdout_write_dec(n & 0x0F);

}

