#include <call.h>
#include <file.h>
#include <string.h>

void main(int argc, char *argv[])
{

    int fd = file_open("/dev/io");

    char c = 0x10;

    file_write(fd, 0x3C0, 1, &c);
    file_read(fd, 0x3C1, 1, &c);
    file_read(fd, 0x3DA, 1, &c);

    c = 0x10;

    file_write(fd, 0x3C0, 1, &c);
    
    c = 0x41;

    file_write(fd, 0x3C0, 1, &c);

    file_close(fd);

    for(;;);

}

