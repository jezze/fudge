#include <fudge.h>
#include <data/lifo.h>

static void interpret(unsigned int count, char *buffer)
{

    /* This is a temporary fix */
    if (memory_match(buffer, "cd ", 3))
    {

        if (count < 4)
            return;

        if (buffer[3] == '/')
            call_open(CALL_DW, CALL_DR, count - 5, buffer + 4);
        else
            call_open(CALL_DW, CALL_DW, count - 4, buffer + 3);

        return;

    }

    call_open(CALL_D1, CALL_DI, 0, 0);
    call_open(CALL_DI, CALL_DR, 6, "temp/0");
    call_open(CALL_DC, CALL_DR, 6, "temp/1");
    call_write(CALL_DI, 0, count, buffer);
    call_open(CALL_D0, CALL_DR, 9, "bin/slang");
    call_spawn(CALL_D0);
    call_close(CALL_D0);
    call_open(CALL_DI, CALL_D1, 0, 0);
    call_close(CALL_D1);

}

static void complete(struct lifo_stack *stack)
{

    char buffer[FUDGE_BSIZE];
    unsigned int lines = 0;
    unsigned int count;
    unsigned int offset;
    unsigned int i;

    call_open(CALL_D1, CALL_DI, 0, 0);
    call_open(CALL_D2, CALL_DO, 0, 0);
    call_open(CALL_D3, CALL_DW, 0, 0);
    call_open(CALL_DI, CALL_DR, 6, "temp/0");
    call_open(CALL_DO, CALL_DR, 6, "temp/1");

    for (offset = stack->head; offset; offset--)
    {

        if (stack->buffer[offset - 1] == ' ' || stack->buffer[offset - 1] == '>' || stack->buffer[offset - 1] == '<')
            break;

    }

    for (i = stack->head; i > offset; i--)
    {

        if (stack->buffer[i - 1] != '/')
            continue;

        if (stack->buffer[offset] == '/')
            call_open(CALL_DW, CALL_DR, stack->head - offset - 1, stack->buffer + offset + 1);
        else
            call_open(CALL_DW, CALL_DW, stack->head - offset, stack->buffer + offset);

        offset = i;

        break;

    }

    call_write(CALL_DI, 0, stack->head - offset, stack->buffer + offset);
    call_open(CALL_D0, CALL_DR, 12, "bin/complete");
    call_spawn(CALL_D0);
    call_close(CALL_D0);

    count = call_read(CALL_DO, 0, FUDGE_BSIZE, buffer);

    call_open(CALL_DI, CALL_D1, 0, 0);
    call_open(CALL_DO, CALL_D2, 0, 0);
    call_open(CALL_DW, CALL_D3, 0, 0);
    call_close(CALL_D1);
    call_close(CALL_D2);
    call_close(CALL_D3);

    if (!count)
        return;

    for (i = 0; i < count; i++)
    {

        if (buffer[i] == '\n')
            lines++;

    }

    if (!lines)
        return;

    if (lines > 1)
    {

        call_write(CALL_DO, 0, 1, "\n");
        call_write(CALL_DO, 0, count, buffer);
        call_write(CALL_DO, 0, 2, "$ ");
        call_write(CALL_DO, 0, stack->head, stack->buffer);

        return;

    }

    for (i = stack->head - offset; i < count - 1; i++)
    {

        lifo_stack_push(stack, buffer[i]);
        call_write(CALL_DO, 0, 1, &buffer[i]);

    }

}

static void handle(struct lifo_stack *stack, char c)
{

    switch (c)
    {

        case '\0':

            break;

        case '\t':

            complete(stack);

            break;

        case '\b':

            if (!lifo_stack_pop(stack))
                break;

            call_write(CALL_DO, 0, 3, "\b \b");

            break;

        case '\r':

            c = '\n';

        case '\n':

            lifo_stack_push(stack, c);
            call_write(CALL_DO, 0, 1, &c);
            interpret(stack->head, stack->buffer);
            lifo_stack_clear(stack);
            call_write(CALL_DO, 0, 2, "$ ");

            break;

        default:

            lifo_stack_push(stack, c);
            call_write(CALL_DO, 0, 1, &c);

            break;

    }

}

static void poll()
{

    struct lifo_stack input;
    unsigned char buffer[FUDGE_BSIZE];
    unsigned int count;
    unsigned int i;

    lifo_stack_clear(&input);

    for (;;)
    {

        count = call_read(CALL_DI, 0, FUDGE_BSIZE, buffer);

        for (i = 0; i < count; i++)
            handle(&input, buffer[i]);

    }

}

void main()
{

    call_write(CALL_DO, 0, 2, "$ ");
    poll();

}

