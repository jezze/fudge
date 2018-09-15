#include <abi.h>
#include <fudge.h>

void main(void)
{

    unsigned char buffer[FUDGE_BSIZE];
    unsigned int count, bytes = 0, words = 0, lines = 0;
    unsigned int whitespace = 1;

    file_open(FILE_PI);

    while ((count = file_read(FILE_PI, buffer, FUDGE_BSIZE)))
    {

        unsigned int i;

        for (i = 0; i < count; i++)
        {

            switch (buffer[i])
            {

            case '\n':
                whitespace = 1;
                lines++;

                break;

            case ' ':
                whitespace = 1;

                break;

            default:
                if (whitespace)
                    words++;

                whitespace = 0;

                break;

            }

            bytes++;

        }

    }

    file_close(FILE_PI);
    file_open(FILE_PO);
    file_writeall(FILE_PO, buffer, ascii_wvalue(buffer, FUDGE_BSIZE, lines, 10));
    file_writeall(FILE_PO, "\n", 1);
    file_writeall(FILE_PO, buffer, ascii_wvalue(buffer, FUDGE_BSIZE, words, 10));
    file_writeall(FILE_PO, "\n", 1);
    file_writeall(FILE_PO, buffer, ascii_wvalue(buffer, FUDGE_BSIZE, bytes, 10));
    file_writeall(FILE_PO, "\n", 1);
    file_close(FILE_PO);

}

