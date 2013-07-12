#include <fudge.h>

void main()
{

    char buffer[FUDGE_BSIZE];
    unsigned int ioffset;
    unsigned int ooffset;
    unsigned int count;

    ooffset = call_write(CALL_DO, 0, 23, "FUDGE OPERATING SYSTEM\n");
    ooffset += call_write(CALL_DO, ooffset, 12, "Build date: ");
    ooffset += call_write(CALL_DO, ooffset, 11, __DATE__);
    ooffset += call_write(CALL_DO, ooffset, 1, " ");
    ooffset += call_write(CALL_DO, ooffset, 8, __TIME__);
    ooffset += call_write(CALL_DO, ooffset, 2, "\n\n");

    if (!call_open(CALL_D0, CALL_DR, 14, "share/motd.txt"))
        return;

    for (ioffset = 0; (count = call_read(CALL_D0, ioffset, FUDGE_BSIZE, buffer)); ioffset += count)
        call_write(CALL_DO, ooffset + ioffset, count, buffer);

}

