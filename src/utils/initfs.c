#include <abi.h>
#include <fudge.h>
#include <lib/file.h>

static struct args
{

    char *position;
    unsigned int count;

} args[32];

static unsigned int parse(unsigned int count, void *buffer)
{

    char *b = buffer;
    char *begin = buffer;
    unsigned int nargs = 0;
    unsigned int i;

    for (i = 0; i < count; i++)
    {

        if (b[i] != '\n' && b[i] != ' ')
            continue;

        b[i] = '\0';
        args[nargs].position = begin;
        args[nargs].count = b + i - begin;
        begin = b + i + 1;
        nargs++;

    }

    return nargs;

}

void main(void)
{

    unsigned char buffer[FUDGE_BSIZE];
    unsigned int count;
    unsigned int nargs;
    unsigned int i;

    call_open(CALL_PI);

    count = file_read(CALL_PI, buffer, FUDGE_BSIZE);
    nargs = parse(count, buffer);

    call_close(CALL_PI);

    for (i = 0; i < nargs; i += 4)
    {

        unsigned int channel = ascii_rvalue(args[i].position, args[i].count, 10);
        unsigned int mount = ascii_rvalue(args[i + 1].position, args[i + 1].count, 10);
        unsigned int backend = ascii_rvalue(args[i + 2].position, args[i + 2].count, 10);

        if (!call_walk(CALL_L0, CALL_PR, args[i + 3].count - 1, args[i + 3].position + 1))
            continue;

        call_open(CALL_L0);
        call_auth(CALL_L1, channel, backend);
        call_mount(mount, CALL_L0, CALL_L1);
        call_close(CALL_L0);

    }

}

