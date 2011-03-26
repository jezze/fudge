#include <call.h>
#include <file.h>
#include <session.h>

void main(int argc, char *argv[])
{

    struct vfs_node *vgaFb = call_open("/dev/tty");

    char c = ' ';
    unsigned int i;

    for (i = 2000; i; i--)
        file_write(vgaFb, i - 1, 1, &c);

}

