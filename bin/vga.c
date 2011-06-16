#include <call.h>
#include <file.h>
#include <string.h>

void main(int argc, char *argv[])
{

    struct file_node *node = call_open("/dev/io");

    char c = 0x10;

    file_write(node, 0x3C0, 1, &c);
    file_read(node, 0x3C1, 1, &c);
    file_read(node, 0x3DA, 1, &c);

    c = 0x10;

    file_write(node, 0x3C0, 1, &c);
    
    c = 0x41;

    file_write(node, 0x3C0, 1, &c);

    for(;;);

}

