#include <fudge.h>

void main()
{

    unsigned int offset = 0;

    offset += call_write(FUDGE_OUT, offset, 23, "FUDGE OPERATING SYSTEM\n");
    offset += call_write(FUDGE_OUT, offset, 12, "Build date: ");
    offset += call_write(FUDGE_OUT, offset, 11, __DATE__);
    offset += call_write(FUDGE_OUT, offset, 1, " ");
    offset += call_write(FUDGE_OUT, offset, 8, __TIME__);
    offset += call_write(FUDGE_OUT, offset, 2, "\n\n");
    offset += call_write(FUDGE_OUT, offset, 43, "Write `help` for a short list if commands\n\n");

}

