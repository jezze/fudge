#include <call.h>
#include <file.h>
#include <string.h>

void main(int argc, char *argv[])
{

    int fd = file_open("/dev/vga_fb");

    char *hw = "Hello world!";

    file_write(fd, 0, string_length(hw), hw);

    file_close(fd);

}

