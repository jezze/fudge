#include <abi.h>
#include <fudge.h>

static void interpret(struct buffer *buffer)
{

    char command[FUDGE_BSIZE];
    unsigned int count = buffer_rcfifo(buffer, FUDGE_BSIZE, command);

    /* This is a temporary fix */
    if (memory_match(command, "cd ", 3))
    {

        if (count < 4)
            return;

        command[count - 1] = '\0';

        if (file_walk(CALL_L1, command + 3))
        {

            file_duplicate(CALL_PW, CALL_L1);
            file_duplicate(CALL_CW, CALL_L1);

        }

        return;

    }

    if (!file_walk(CALL_CP, "/bin/slang"))
        return;

    if (!file_walk(CALL_L0, "/system/pipe/clone/"))
        return;

    file_walkfrom(CALL_L1, CALL_L0, "0");
    file_walkfrom(CALL_CI, CALL_L0, "1");
    file_walkfrom(CALL_CO, CALL_L0, "1");
    file_open(CALL_L1);
    file_writeall(CALL_L1, command, count);
    file_close(CALL_L1);
    call_spawn();
    file_open(CALL_L1);

    while ((count = file_read(CALL_L1, command, FUDGE_BSIZE)))
        file_writeall(CALL_PO, command, count);

    file_close(CALL_L1);

}

static void complete(struct buffer *buffer)
{

}

static void handle(struct buffer *buffer, unsigned char c)
{

    switch (c)
    {

    case '\0':
        break;

    case '\t':
        complete(buffer);

        break;

    case '\b':
    case 0x7F:
        if (!buffer_ecfifo(buffer, 1))
            break;

        file_writeall(CALL_PO, "\b \b", 3);

        break;

    case '\r':
        c = '\n';

    case '\n':
        file_writeall(CALL_PO, &c, 1);
        buffer_wcfifo(buffer, 1, &c);
        interpret(buffer);
        file_writeall(CALL_PO, "$ ", 2);

        break;

    default:
        buffer_wcfifo(buffer, 1, &c);
        file_writeall(CALL_PO, &c, 1);

        break;

    }

}

void main(void)
{

    unsigned char buffer[FUDGE_BSIZE];
    unsigned int count;
    unsigned char inputbuffer[FUDGE_BSIZE];
    struct buffer input;

    buffer_init(&input, FUDGE_BSIZE, inputbuffer);
    file_open(CALL_PI);
    file_open(CALL_PO);
    file_writeall(CALL_PO, "$ ", 2);

    while ((count = file_read(CALL_PI, buffer, FUDGE_BSIZE)))
    {

        unsigned int i;

        for (i = 0; i < count; i++)
            handle(&input, buffer[i]);

    }

    file_close(CALL_PO);
    file_close(CALL_PI);

}

