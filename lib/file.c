#include <lib/call.h>
#include <lib/file.h>
#include <lib/memory.h>
#include <lib/string.h>
#include <lib/vfs.h>

void file_close(struct vfs_node *node)
{

    if (node && node->close)
        return node->close(node);

}

struct vfs_node *file_find(struct vfs_node *node, char *path)
{

    unsigned int index = string_index(path, '/', 0);
    unsigned int length = string_length(path);

    if (!index)
        return node;

    struct vfs_node *current;
    unsigned int i;

    for (i = 0; (current = file_walk(node, i)); i++)
    {

        unsigned int count = string_length(current->name);

        if (index > count)
            count = index;

        if (!memory_compare(path, current->name, count))
            return (index == length) ? current : file_find(current, path + index + 1);

    }

    return 0;

}

void file_open(struct vfs_node *node)
{

    if (node && node->open)
        return node->open(node);

}

unsigned int file_read(struct vfs_node *node, unsigned int offset, unsigned int count, void *buffer)
{

    return (node && node->read) ? node->read(node, offset, count, buffer) : 0;

}

unsigned int file_read_byte(struct vfs_node *node, char c)
{

    return file_read(node, 0, 1, &c);

}

struct vfs_node *file_walk(struct vfs_node *node, unsigned int index)
{

    return (node && node->walk) ? node->walk(node, index) : 0;

}

unsigned int file_write(struct vfs_node *node, unsigned int offset, unsigned int count, void *buffer)
{

    return (node && node->write) ? node->write(node, offset, count, buffer) : 0;

}

unsigned int file_write_bcd(struct vfs_node *node, unsigned char num)
{

    return file_write_dec(node, num >> 4) + file_write_dec(node, num & 0x0F);

}

unsigned int file_write_byte(struct vfs_node *node, char c)
{

    return file_write(node, 0, 1, &c);

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

    return file_write(node, 0, string_length(buffer), buffer);

}

