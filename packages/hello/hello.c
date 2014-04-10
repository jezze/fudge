#include <fudge.h>

void main()
{

    call_open(CALL_O0);
    call_write(CALL_O0, 0, 13, "Hello world!\n");
    call_close(CALL_O0);

}

