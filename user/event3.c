#include <fudge.h>

void keyboard()
{

    file_write(FILE_STDOUT, 0, 4, "Key\n");

    call_wait();

}

void mouse()
{

    file_write(FILE_STDOUT, 0, 6, "Mouse\n");

    call_wait();

}

void main(int argc, char *argv[])
{

    call_attach(0x01, keyboard);
    call_attach(0x0C, mouse);
    call_wait();

}

