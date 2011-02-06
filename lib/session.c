#include <lib/call.h>
#include <lib/file.h>
#include <lib/vfs.h>

struct vfs_node *session_get_location()
{

    struct vfs_node *sessionLocation = call_open("/dev/location");

    char buffer[256];
    file_read(sessionLocation, 0, 256, buffer);

    return call_open(buffer);

}

struct vfs_node *session_get_in()
{

    struct vfs_node *sessionIn = call_open("/dev/stdin");

    return sessionIn;

}

struct vfs_node *session_get_out()
{

    struct vfs_node *sessionOut = call_open("/dev/stdout");

    return sessionOut;

}

