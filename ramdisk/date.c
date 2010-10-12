#include <call.h>
#include <stdout.h>
#include <vfs.h>

void main(int argc, char *argv[])
{

    struct vfs_node *node = call_open("dev/rtc");

    unsigned char buffer;

    stdout_write("20");
    vfs_read(node, 0, 1, &buffer);
    stdout_write_bcd(buffer);

    stdout_write("-");

    vfs_read(node, 1, 1, &buffer);
    stdout_write_bcd(buffer);

    stdout_write("-");

    vfs_read(node, 2, 1, &buffer);
    stdout_write_bcd(buffer);

    stdout_write(" ");

    vfs_read(node, 3, 1, &buffer);
    stdout_write_bcd(buffer);

    stdout_write(":");

    vfs_read(node, 4, 1, &buffer);
    stdout_write_bcd(buffer);

    stdout_write(":");

    vfs_read(node, 5, 1, &buffer);
    stdout_write_bcd(buffer);

    vfs_read(node, 3, 1, &buffer);

    if (buffer >> 4)
        stdout_write("PM\n");
    else
        stdout_write("AM\n");

}

