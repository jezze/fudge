#include <call.h>
#include <file.h>
#include <session.h>
#include <vfs.h>

void main(int argc, char *argv[])
{

    if (argc != 2)
    {

        file_write(session_get_out(), "You need to supply filename.\n");

        return;

    }

    struct vfs_node *node = vfs_find(session_get_location(), argv[1]);

    if (!node)
    {

        file_write(session_get_out(), "File does not exist.\n");

        return;

    }

    char buffer[5000];

    unsigned int size = vfs_read(node, 0, 5000, buffer);

    vfs_write(session_get_out(), 0, size, buffer);

}

