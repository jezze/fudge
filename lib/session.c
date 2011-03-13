#include <lib/call.h>
#include <lib/file.h>
#include <lib/vfs.h>

struct vfs_node *session_get_cwd()
{

    struct vfs_node *node = call_open("/dev/cwd");

    char buffer[256];
    file_read(node, 0, 256, buffer);

    return call_open(buffer);

}

struct vfs_node *session_get_in()
{

    return call_open("/dev/stdin");

}

struct vfs_node *session_get_out()
{

    return call_open("/dev/stdout");

}

