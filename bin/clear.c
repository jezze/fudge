#include <call.h>
#include <file.h>
#include <session.h>
#include <vfs.h>

void main(int argc, char *argv[])
{

    struct vfs_node *vgaFb = call_open("/dev/vga_fb");
    struct vfs_node *vgaFbCursor = call_open("/dev/vga_fb_cursor");

    char c = ' ';
    unsigned int i;

    for (i = 0; i < 2000; i++)
        vfs_write(vgaFb, i, 1, &c);

    i = 0;

    vfs_write(vgaFbCursor, 0, 1, &i);

}

