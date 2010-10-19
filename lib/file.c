#include <lib/call.h>
#include <lib/file.h>
#include <lib/string.h>
#include <lib/vfs.h>

char file_read_single(struct vfs_node *node)
{

    if (!node)
        return 0;

    char c;

    while (!(vfs_read(node, 0, 1, &c)));

    return c;

}

int file_write_single(struct vfs_node *node, char c)
{

    if (!node)
        return 0;

    return vfs_write(node, 0, 1, &c);

}

int file_write(struct vfs_node *node, char *buffer)
{

    if (!node)
        return 0;

    return vfs_write(node, 0, string_length(buffer), buffer);

}

void file_write_num(struct vfs_node *node, unsigned int num, unsigned int base)
{

    if (!num)
    {

        file_write_single(node, '0');

        return;

    }

    char buffer[32] = {0};

    int i;

    for (i = 30; num && i; --i, num /= base)
        buffer[i] = "0123456789abcdef"[num % base];

    file_write(node, buffer + i + 1);

}

void file_write_dec(struct vfs_node *node, unsigned int num)
{

    file_write_num(node, num, 10);

}

void file_write_hex(struct vfs_node *node, unsigned int num)
{

    file_write_num(node, num, 16);

}

void file_write_bcd(struct vfs_node *node, unsigned char num)
{

    file_write_dec(node, num >> 4);
    file_write_dec(node, num & 0x0F);

}

