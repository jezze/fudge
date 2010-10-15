#include <lib/call.h>
#include <lib/file.h>
#include <lib/string.h>
#include <lib/vfs.h>

char file_read_single()
{

    struct vfs_node *node = call_open("dev/stdin");

    if (!node)
        return 0;

    char c;

    while (!(vfs_read(node, 0, 1, &c)));

    return c;

}

int file_write_single(char c)
{

    struct vfs_node *node = call_open("dev/stdout");

    if (!node)
        return 0;

    return vfs_write(node, 0, 1, &c);

}

int file_write(char *s)
{

    struct vfs_node *node = call_open("dev/stdout");

    if (!node)
        return 0;

    return vfs_write(node, 0, string_length(s), s);

}

void file_write_num(unsigned int n, unsigned int base)
{

    if (!n)
    {

        file_write_single('0');

        return;

    }

    char s[32] = {0};

    int i;

    for (i = 30; n && i; --i, n /= base)
        s[i] = "0123456789abcdef"[n % base];

    file_write(s + i + 1);

}

void file_write_dec(unsigned int n)
{

    file_write_num(n, 10);

}

void file_write_hex(unsigned int n)
{

    file_write_num(n, 16);

}

void file_write_bcd(unsigned char n)
{

    file_write_dec(n >> 4);
    file_write_dec(n & 0x0F);

}

