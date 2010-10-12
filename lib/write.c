#include <lib/call.h>
#include <lib/string.h>
#include <lib/vfs.h>
#include <lib/write.h>

int write_char(char c)
{

    struct vfs_node *node = call_open("dev/stdout");

    if (!node)
        return 0;

    return vfs_write(node, 0, 1, &c);

}

int write_string(char *s)
{

    struct vfs_node *node = call_open("dev/stdout");

    if (!node)
        return 0;

    return vfs_write(node, 0, string_length(s), s);

}

void write_string_num(unsigned int n, unsigned int base)
{

    if (!n)
    {

        write_char('0');

        return;

    }

    char s[32] = {0};

    int i;

    for (i = 30; n && i; --i, n /= base)
        s[i] = "0123456789abcdef"[n % base];

    write_string(s + i + 1);

}

void write_string_dec(unsigned int n)
{

    write_string_num(n, 10);

}

void write_string_hex(unsigned int n)
{

    write_string_num(n, 16);

}

void write_string_bcd(unsigned char n)
{

    write_string_dec(n >> 4);
    write_string_dec(n & 0x0F);

}

