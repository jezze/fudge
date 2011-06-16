#include <call.h>
#include <file.h>
#include <string.h>

void main(int argc, char *argv[])
{

    int fd = call_open2("/dev/io");

    char c = 0x10;

    file_write2(fd, 0x3C0, 1, &c);
    file_read2(fd, 0x3C1, 1, &c);
    file_read2(fd, 0x3DA, 1, &c);

    c = 0x10;

    file_write2(fd, 0x3C0, 1, &c);
    
    c = 0x41;

    file_write2(fd, 0x3C0, 1, &c);

    call_close(fd);

    for(;;);

}

