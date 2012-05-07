#include <fudge.h>

void keyboard()
{

    call_write(FILE_STDOUT, 0, 4, "Key\n");
    call_wait();

}

void mouse()
{

    call_write(FILE_STDOUT, 0, 6, "Mouse\n");
    call_wait();

}

void main()
{

    call_attach(0x21, keyboard);
    call_attach(0x2C, mouse);
    call_wait();

}

