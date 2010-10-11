#include <call.h>
#include <vfs.h>

void main(int argc, char *argv[])
{

    if (argc != 2)
    {

        call_puts("You need to supply filename.\n");

        return;

    }

    struct vfs_node *node = call_vfs_open(argv[1]);

    if (!node)
    {

        call_puts("File does not exist.\n");

        return;

    }

    char buffer[2000];

    unsigned int size = vfs_read(node, 0, 2000, buffer);

    struct vfs_node *out = call_vfs_open("dev/stdout");

    vfs_write(out, 0, size, buffer);

}

