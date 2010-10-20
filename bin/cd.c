#include <call.h>
#include <file.h>
#include <session.h>
#include <string.h>
#include <vfs.h>

void main(int argc, char *argv[])
{

    struct vfs_node *location = call_open("/dev/location");

    char buffer[256];

    unsigned int size = vfs_read(location, 0, 256, buffer);

    if (argc == 1)
    {

        file_write(session_get_out(), buffer);
        file_write(session_get_out(), "\n");

        return;

    }

    struct vfs_node *node = vfs_find(session_get_location(), argv[1]);

    if (!node)
    {

        file_write(session_get_out(), "Directory does not exist.\n");

        return;

    }

    if (!node->walk)
    {

        file_write(session_get_out(), "Not a directory.\n");

        return;

    }

    if (argv[1][0] == '/')
        vfs_write(location, 0, string_length(argv[1]), argv[1]);
    else
        vfs_write(location, string_length(buffer), string_length(argv[1]), argv[1]);

}

