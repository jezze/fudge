#include <fudge.h>

void main()
{

    call_open(CALL_I0, CALL_OPEN_READ | CALL_OPEN_WRITE);
    call_unload(CALL_I0);
    call_close(CALL_I0);

}

