#include <call.h>
#include <file.h>
#include <vfs.h>

void main(int argc, char *argv[])
{

    struct vfs_node *vgaCursor = call_open("dev/vga_fb_cursor");
    struct vfs_node *node = call_open("dev/vga_fb");

    unsigned offset;

    vfs_read(vgaCursor, 0, 1, &offset);
 
    char row[80];

    vfs_read(node, 80 * (offset / 80), 80, row);

    char c = ' ';
    unsigned int i;

    for (i = 0; i < 2000; i++)
        vfs_write(node, i, 1, &c);

    vfs_write(node, 0, 80, row);

    offset %= 80;

    vfs_write(vgaCursor, 0, 1, &offset);


}

