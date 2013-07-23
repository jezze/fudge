#include <arch.h>
#include <fudge.h>

void main()
{

    unsigned char buffer[FUDGE_BSIZE];
    unsigned int count;
    unsigned int offset;
    unsigned int last = 0;

    last += call_write(CALL_DO, last, 23, "FUDGE OPERATING SYSTEM\n");
    last += call_write(CALL_DO, last, 12, "Build date: ");
    last += call_write(CALL_DO, last, 11, __DATE__);
    last += call_write(CALL_DO, last, 1, " ");
    last += call_write(CALL_DO, last, 8, __TIME__);
    last += call_write(CALL_DO, last, 2, "\n\n");

    if (!call_open(CALL_D0, CALL_DR, 14, "share/motd.txt"))
        return;

    for (offset = 0; (count = call_read(CALL_D0, offset, FUDGE_BSIZE, buffer)); offset += count)
        call_write(CALL_DO, last + offset, count, buffer);

}

