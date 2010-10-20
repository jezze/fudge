#include <lib/call.h>
#include <lib/file.h>
#include <lib/vfs.h>

struct vfs_node *sessionIn;
struct vfs_node *sessionLocation;
struct vfs_node *sessionOut;

struct vfs_node *session_get_location()
{

    if (!sessionLocation)
        sessionLocation = call_open("/dev/location");

    char buffer[256];
    file_read(sessionLocation, 0, 256, buffer);

    return call_open(buffer);

}

struct vfs_node *session_get_in()
{

    if (!sessionIn)
        sessionIn = call_open("/dev/kbd");

    return sessionIn;

}

struct vfs_node *session_get_out()
{

    if (!sessionOut)
        sessionOut = call_open("/dev/tty");

    return sessionOut;

}

