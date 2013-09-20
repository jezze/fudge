#include <fudge.h>

void main()
{

    call_open(CALL_DW, CALL_DR, 5, "home/");
    call_open(CALL_I0, CALL_DR, 17, "config/init.slang");
    call_open(CALL_E0, CALL_DR, 9, "bin/slang");
    call_spawn(CALL_E0);
    call_close(CALL_E0);

}

