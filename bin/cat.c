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

    struct vfs_node *location = session_get_location();

    location = vfs_find(location, (argv[1]));

    if (!location)
    {

        file_write(session_get_out(), "File does not exist.\n");

        return;

    }

    char buffer[5000];

    unsigned int size = vfs_read(location, 0, 5000, buffer);

    struct vfs_node *out = call_open("/dev/tty");

    vfs_write(out, 0, size, buffer);

}

