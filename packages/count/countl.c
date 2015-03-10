#include <abi.h>
#include <fudge.h>

void main()
{

    unsigned char buffer[FUDGE_BSIZE];
    unsigned int offset, count;
    unsigned int lines = 0;

    call_open(CALL_P0);

    for (offset = 0; (count = call_read(CALL_P0, offset, FUDGE_BSIZE, buffer)); offset += count)
    {

        unsigned int i;

        for (i = 0; i < count; i++)
        {

            if (buffer[i] == '\n')
                lines++;

        }

    }

    call_close(CALL_P0);
    call_open(CALL_PO);
    call_write(CALL_PO, 0, ascii_fromint(buffer, FUDGE_BSIZE, lines, 10), buffer);
    call_close(CALL_PO);

}

