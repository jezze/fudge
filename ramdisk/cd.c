#include <call.h>
#include <file.h>
#include <session.h>
#include <string.h>
#include <vfs.h>

void main(int argc, char *argv[])
{

    struct vfs_node *node = call_open("/dev/location");

    char buffer[256];

    unsigned int size = vfs_read(node, 0, 256, buffer);

    if (argc == 1)
    {

        file_write(session_get_out(), buffer);
        file_write(session_get_out(), "\n");

        return;

    }

    struct vfs_node *base;
    struct vfs_node *directory;

    if (argv[1][0] == '/')
    {

        base = call_open("/");
        directory = vfs_find(base, argv[1]);

        if (directory && directory->walk)
            vfs_write(node, 0, string_length(argv[1]), argv[1]);

    }

    else
    {

        base = call_open(buffer);
        directory = vfs_find(base, argv[1]);

        if (directory && directory->walk)
            vfs_write(node, string_length(buffer), string_length(argv[1]), argv[1]);

    }

}

