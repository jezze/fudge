#include <lib/types.h>
#include <lib/string.h>
#include <lib/vfs.h>
#include <lib/call.h>

int call_puts(char *s)
{

    vfs_node_t *node = call_vfs_find("stdout");

    if (!node)
        return 0;

    return vfs_write(node, 0, string_length(s), s);

}

char call_getc()
{

    vfs_node_t *node = call_vfs_find("stdin");

    if (!node)
        return 0;

    char buffer[32];

    vfs_read(node, 0, 1, buffer);

    return buffer[0];

}
