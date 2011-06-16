#include <call.h>
#include <file.h>
#include <string.h>

void main(int argc, char *argv[])
{

    int fd = call_open2("/dev/vga_fb");

    char *hw = "Hello world!";

    file_write2(fd, 0, string_length(hw), hw);

    call_close(fd);

}

