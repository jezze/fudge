#include <lib/call.h>
#include <lib/file.h>
#include <lib/string.h>
#include <lib/vfs.h>

char file_read_byte(struct vfs_node *node)
{

    if (!node)
        return 0;

    char c;

    while (!(vfs_read(node, 0, 1, &c)));

    return c;

}

unsigned int file_write_bcd(struct vfs_node *node, unsigned char num)
{

    return file_write_dec(node, num >> 4) + file_write_dec(node, num & 0x0F);

}

unsigned int file_write_byte(struct vfs_node *node, char c)
{

    if (!node)
        return 0;

    return vfs_write(node, 0, 1, &c);

}

unsigned int file_write_dec(struct vfs_node *node, unsigned int num)
{

    return file_write_num(node, num, 10);

}

unsigned int file_write_hex(struct vfs_node *node, unsigned int num)
{

    return file_write_num(node, num, 16);

}

unsigned int file_write_num(struct vfs_node *node, unsigned int num, unsigned int base)
{

    if (!num)
        return file_write_byte(node, '0');

    char buffer[32] = {0};

    int i;

    for (i = 30; num && i; --i, num /= base)
        buffer[i] = "0123456789abcdef"[num % base];

    return file_write_string(node, buffer + i + 1);

}

unsigned int file_write_string(struct vfs_node *node, char *buffer)
{

    if (!node)
        return 0;

    return vfs_write(node, 0, string_length(buffer), buffer);

}

