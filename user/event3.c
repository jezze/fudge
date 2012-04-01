#include <fudge.h>

void keyboard()
{

    file_write_format(FILE_STDOUT, 0, "You pressed a key it seems...\n");

    call_wait();

}

void mouse()
{

    file_write_format(FILE_STDOUT, 0, "You used the mouse it seems...\n");

    call_wait();

}

void main(int argc, char *argv[])
{

    call_attach(0x01, keyboard);
    call_attach(0x0C, mouse);
    call_wait();

}

