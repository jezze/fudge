#include <fudge.h>

void main()
{

    call_open(CALL_DW, CALL_DR, 5, "home/");
    call_open(CALL_DI, CALL_DR, 17, "config/init.slang");
    call_open(CALL_D0, CALL_DR, 9, "bin/slang");
    call_spawn(CALL_D0);
    call_close(CALL_D0);

}

