#include <call.h>
#include <file.h>
#include <string.h>
#include <vfs.h>

void main(int argc, char *argv[])
{

    char *hw = "Hello world!";

    struct vfs_node *node = call_open("/dev/vga_fb");
    file_write(node, 0, string_length(hw), hw);

}
