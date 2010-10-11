#include <call.h>
#include <vfs.h>

void main(int argc, char *argv[])
{

    struct vfs_node *node = call_open("dev/rtc");

    unsigned char buffer;

    call_puts("20");
    vfs_read(node, 0, 1, &buffer);
    call_puts_bcd(buffer);

    call_puts("-");

    vfs_read(node, 1, 1, &buffer);
    call_puts_bcd(buffer);

    call_puts("-");

    vfs_read(node, 2, 1, &buffer);
    call_puts_bcd(buffer);

    call_puts(" ");

    vfs_read(node, 3, 1, &buffer);
    call_puts_bcd(buffer);

    call_puts(":");

    vfs_read(node, 4, 1, &buffer);
    call_puts_bcd(buffer);

    call_puts(":");

    vfs_read(node, 5, 1, &buffer);
    call_puts_bcd(buffer);

    vfs_read(node, 3, 1, &buffer);

    if (buffer >> 4)
        call_puts("PM\n");
    else
        call_puts("AM\n");

}

