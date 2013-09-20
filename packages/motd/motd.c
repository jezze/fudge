#include <fudge.h>

void main()
{

    unsigned char buffer[FUDGE_BSIZE];
    unsigned int count, roff, woff = 0;

    woff += call_write(CALL_O0, woff, 23, "FUDGE OPERATING SYSTEM\n");
    woff += call_write(CALL_O0, woff, 12, "Build date: ");
    woff += call_write(CALL_O0, woff, 11, __DATE__);
    woff += call_write(CALL_O0, woff, 1, " ");
    woff += call_write(CALL_O0, woff, 8, __TIME__);
    woff += call_write(CALL_O0, woff, 2, "\n\n");

    if (!call_open(CALL_L0, CALL_DR, 14, "share/motd.txt"))
        return;

    for (roff = 0; (count = call_read(CALL_L0, roff, FUDGE_BSIZE, buffer)); roff += count)
        woff += call_write(CALL_O0, woff, count, buffer);

}

