#include <fudge.h>

static void interpret(struct buffer *buffer)
{

    /* This is a temporary fix */
    if (memory_match(buffer->memory, "cd ", 3))
    {

        unsigned int ok;
        char temp[FUDGE_BSIZE];
        unsigned int count;

        if (buffer->head < 4)
            return;

        count = buffer_rcfifo(buffer, FUDGE_BSIZE, temp);

        if (temp[3] == '/')
            ok = call_walk(CALL_L0, CALL_DR, count - 5, temp + 4);
        else
            ok = call_walk(CALL_L0, CALL_DW, count - 4, temp + 3);

        if (ok)
            call_walk(CALL_DW, CALL_L0, 0, 0);

        return;

    }

    if (!call_walk(CALL_DP, CALL_DR, 9, "bin/slang"))
        return;

    if (!call_walk(CALL_L1, CALL_DR, 14, "system/pipe:7/"))
        return;

    if (!call_walk(CALL_L2, CALL_L1, 1, "0"))
        return;

    if (!call_walk(CALL_I1, CALL_L1, 1, "1"))
        return;

    if (!call_walk(CALL_O1, CALL_O0, 0, 0))
        return;

    call_open(CALL_L2);
    call_write(CALL_L2, 0, buffer->head, buffer->memory);
    call_close(CALL_L2);
    call_spawn(CALL_DP);

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
        if (!buffer_poplifo(buffer, 1))
            break;

        call_write(CALL_O0, 0, 3, "\b \b");

        break;

    case '\r':
        c = '\n';

    case '\n':
        buffer_pushlifo(buffer, 1, &c);
        call_write(CALL_O0, 0, 1, &c);
        interpret(buffer);
        buffer_clear(buffer);
        call_write(CALL_O0, 0, 2, "$ ");

        break;

    default:
        buffer_pushlifo(buffer, 1, &c);
        call_write(CALL_O0, 0, 1, &c);

        break;

    }

}

static void poll()
{

    unsigned char buffer[FUDGE_BSIZE];
    unsigned int count, roff;
    unsigned char inputbuffer[FUDGE_BSIZE];
    struct buffer input;

    buffer_init(&input, 1, FUDGE_BSIZE, inputbuffer);

    call_open(CALL_I0);

    for (roff = 0; (count = call_read(CALL_I0, roff, FUDGE_BSIZE, buffer)); roff += count)
    {

        unsigned int i;

        for (i = 0; i < count; i++)
            handle(&input, buffer[i]);

    }

    call_close(CALL_I0);

}

void main()
{

    call_open(CALL_O0);
    call_write(CALL_O0, 0, 2, "$ ");
    poll();
    call_close(CALL_O0);

}

