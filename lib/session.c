#include <lib/call.h>
#include <lib/vfs.h>

struct vfs_node *session_get_location()
{

    struct vfs_node *location = call_open("/dev/location");

    char buffer[256];

    vfs_read(location, 0, 256, buffer);

    return call_open(buffer);

}

struct vfs_node *session_get_in()
{

    return call_open("/dev/kbd");

}

struct vfs_node *session_get_out()
{

    return call_open("/dev/tty");

}

