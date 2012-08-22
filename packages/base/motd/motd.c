#include <fudge.h>

void main()
{

    call_write(FILE_STDOUT, 0, 23, "FUDGE OPERATING SYSTEM\n");
    call_write(FILE_STDOUT, 0, 12, "Build date: ");
    call_write(FILE_STDOUT, 0, 11, __DATE__);
    call_write(FILE_STDOUT, 0, 1, " ");
    call_write(FILE_STDOUT, 0, 8, __TIME__);
    call_write(FILE_STDOUT, 0, 2, "\n\n");
    call_write(FILE_STDOUT, 0, 53, "Write `echo <help.txt` for a short list if commands\n\n");

}

