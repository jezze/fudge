#include <abi.h>
#include <fudge.h>

void main(void)
{

    unsigned char buffer[FUDGE_BSIZE];
    unsigned int count, bytes = 0, spaces = 0, lines = 0;

    file_open(FILE_PI);

    while ((count = file_read(FILE_PI, buffer, FUDGE_BSIZE)))
    {

        unsigned int i;

        for (i = 0; i < count; i++)
        {

            switch (buffer[i])
            {

            case '\n':
                lines++;

                break;

            case ' ':
                spaces++;

                break;

            }

            bytes++;

        }

    }

    file_close(FILE_PI);
    file_open(FILE_PO);
    file_writeall(FILE_PO, buffer, ascii_fromint(buffer, FUDGE_BSIZE, bytes, 10));
    file_writeall(FILE_PO, "\n", 1);
    file_writeall(FILE_PO, buffer, ascii_fromint(buffer, FUDGE_BSIZE, spaces, 10));
    file_writeall(FILE_PO, "\n", 1);
    file_writeall(FILE_PO, buffer, ascii_fromint(buffer, FUDGE_BSIZE, lines, 10));
    file_writeall(FILE_PO, "\n", 1);
    file_close(FILE_PO);

}

