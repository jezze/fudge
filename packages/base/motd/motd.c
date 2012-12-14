#include <fudge.h>

void main()
{

    call_write(FUDGE_OUT, 0, 23, "FUDGE OPERATING SYSTEM\n");
    call_write(FUDGE_OUT, 0, 12, "Build date: ");
    call_write(FUDGE_OUT, 0, 11, __DATE__);
    call_write(FUDGE_OUT, 0, 1, " ");
    call_write(FUDGE_OUT, 0, 8, __TIME__);
    call_write(FUDGE_OUT, 0, 2, "\n\n");
    call_write(FUDGE_OUT, 0, 43, "Write `help` for a short list if commands\n\n");

}

