#include <fudge.h>

void main(int argc, char *argv[])
{

    int fd = file_open("/vga_fb");

    char *hw = "Hello world!";

    file_write_string(fd, hw);

    file_close(fd);

}

