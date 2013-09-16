#include <fudge.h>
#include <data/lifo.h>

static void open_temp(unsigned int index, unsigned int index2, unsigned int count, void *buffer)
{

    char buffer0[FUDGE_BSIZE];
    char buffer1[FUDGE_BSIZE];
    char num[32];
    unsigned int ccount, offset0 = 0, offset1 = 0;

    call_open(index2, CALL_DR, 17, "system/pipe/clone");
    ccount = call_read(index2, 0, 32, num);

    offset0 += memory_write(buffer0, FUDGE_BSIZE, "system/pipe/", 12, offset0);
    offset0 += memory_write(buffer0, FUDGE_BSIZE, num, ccount, offset0);
    offset0 += memory_write(buffer0, FUDGE_BSIZE, "/0", 2, offset0);
    offset1 += memory_write(buffer1, FUDGE_BSIZE, "system/pipe/", 12, offset1);
    offset1 += memory_write(buffer1, FUDGE_BSIZE, num, ccount, offset1);
    offset1 += memory_write(buffer1, FUDGE_BSIZE, "/1", 2, offset1);

    call_open(CALL_D0, CALL_DR, offset0, buffer0);
    call_write(CALL_D0, 0, count, buffer);
    call_close(CALL_D0);
    call_open(index, CALL_DR, offset1, buffer1);

}

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
    call_open(CALL_D2, CALL_DO, 0, 0);
    open_temp(CALL_DI, CALL_D8, count, buffer);
    call_open(CALL_D0, CALL_DR, 9, "bin/slang");
    call_spawn(CALL_D0);
    call_close(CALL_D0);
    call_close(CALL_D8);
    call_open(CALL_DI, CALL_D1, 0, 0);
    call_open(CALL_DO, CALL_D2, 0, 0);
    call_close(CALL_D1);
    call_close(CALL_D2);

}

static void complete(struct lifo_stack *stack)
{

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

