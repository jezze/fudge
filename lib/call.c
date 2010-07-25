#include <lib/call.h>
#include <lib/string.h>
#include <lib/vfs.h>

int call_putc(char c)
{

    struct vfs_node *node = call_vfs_find("stdout");

    if (!node)
        return 0;

    return vfs_write(node, 0, 1, &c);

}

int call_puts(char *s)
{

    struct vfs_node *node = call_vfs_find("stdout");

    if (!node)
        return 0;

    return vfs_write(node, 0, string_length(s), s);

}

char call_getc()
{

    struct vfs_node *node = call_vfs_find("stdin");

    if (!node)
        return 0;

    char c;

    while (!(vfs_read(node, 0, 1, &c)));

    return c;

}

