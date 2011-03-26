#include <call.h>
#include <file.h>
#include <session.h>

void main(int argc, char *argv[])
{

    if (argc != 2)
    {

        file_write_string(session_get_out(), "You need to supply filename.\n");

        return;

    }

    struct vfs_node *node = file_find(session_get_cwd(), argv[1]);

    if (!node)
    {

        file_write_string(session_get_out(), "File does not exist.\n");

        return;

    }

    char buffer[5000];

    unsigned int size = file_read(node, 0, 5000, buffer);

    file_write(session_get_out(), 0, size, buffer);

}

