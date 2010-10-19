#include <call.h>
#include <file.h>
#include <string.h>
#include <vfs.h>

void main(int argc, char *argv[])
{

    struct vfs_node *node = call_open("dev/location");

    char buffer[256];

    unsigned int size = vfs_read(node, 0, 256, buffer);

    if (argc == 1)
    {

        file_write(buffer);
        file_write("\n");

        return;

    }

    if (argv[1][0] == '/')
        vfs_write(node, 0, string_length(argv[1]), argv[1]);
    else
        vfs_write(node, string_length(buffer), string_length(argv[1]), argv[1]);

}

