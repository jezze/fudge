#include <lib/string.h>
#include <kernel/log.h>
#include <kernel/vfs.h>
#include <modules/log/log.h>

void init()
{

    char *log = log_get();

    if (!log)
        return;

    struct vfs_node *node = vfs_find("tty", "stdout");

    if (!node)
        return;

    node->write(node, string_length(log), log);

}

void destroy()
{

}

