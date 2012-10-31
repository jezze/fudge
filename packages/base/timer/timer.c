#include <fudge.h>

void main()
{

    char buffer[0x1000];

    if (!call_open(3, 19, "/nodefs/pit_jiffies"))
        return;

    call_write(FILE_STDOUT, 0, call_read(3, 0, 0x1000, buffer), buffer);
    call_close(3);

}

