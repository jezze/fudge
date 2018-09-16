#include <fudge.h>
#include "file.h"
#include "echo.h"

static void interpret(unsigned int descriptor, char c, union echo_arg *a)
{

    char num[FUDGE_NSIZE];

    switch (c)
    {

    case '%':
        file_writeall(descriptor, "%", 1);

        break;

    case 'b':
        file_writeall(descriptor, a->b.data, a->b.length);

        break;

    case 's':
        file_writeall(descriptor, a->s, ascii_length(a->s));

        break;

    case 'u':
        file_writeall(descriptor, num, ascii_wvalue(num, FUDGE_NSIZE, a->u, 10));

        break;

    case 'h':
        file_writeall(descriptor, num, ascii_wvalue(num, FUDGE_NSIZE, a->u, 16));

        break;

    }

}

void echo(unsigned int descriptor, char *s, union echo_arg *a)
{

    while (*s)
    {

        switch (*s)
        {

        case '%':
            interpret(descriptor, *++s, a++);

            break;

        default:
            file_writeall(descriptor, s, 1);

            break;

        }

        s++;

    }

}

