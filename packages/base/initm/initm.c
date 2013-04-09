#include <fudge.h>

void main()
{

    call_open(FUDGE_CWD, FUDGE_ROOT, 5, "home/");
    call_open(FUDGE_IN, FUDGE_ROOT, 17, "config/base.slang");
    call_open(3, FUDGE_ROOT, 9, "bin/slang");
    call_spawn(3);
    call_open(FUDGE_IN, FUDGE_ROOT, 17, "config/user.slang");
    call_open(3, FUDGE_ROOT, 9, "bin/slang");
    call_execute(3);

}

