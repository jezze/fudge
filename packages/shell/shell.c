#include <fudge.h>
#include <data/lifo.h>

static void open_pipe(unsigned int index, unsigned int index0, unsigned int index1)
{

    char buffer[FUDGE_BSIZE];
    char num[32];
    unsigned int offset;
    unsigned int count = call_read(index, 0, 32, num);

    offset = memory_write(buffer, FUDGE_BSIZE, "system/pipe/", 12, 0);
    offset += memory_write(buffer, FUDGE_BSIZE, num, count, offset);
    offset += memory_write(buffer, FUDGE_BSIZE, "/0", 2, offset);

    call_open(index0, CALL_DR, offset, buffer);

    offset = memory_write(buffer, FUDGE_BSIZE, "system/pipe/", 12, 0);
    offset += memory_write(buffer, FUDGE_BSIZE, num, count, offset);
    offset += memory_write(buffer, FUDGE_BSIZE, "/1", 2, offset);

    call_open(index1, CALL_DR, offset, buffer);

}

static void close_pipe(unsigned int index, unsigned int index0, unsigned int index1)
{

    call_close(index0);
    call_close(index1);

}

static void call_slang(unsigned int index)
{

    call_open(index, CALL_DR, 9, "bin/slang");
    call_spawn(index);
    call_close(index);

}

/*
static void call_complete(unsigned int index)
{

    call_open(index, CALL_DR, 12, "bin/complete");
    call_spawn(index);
    call_close(index);

}
*/

static void interpret(struct lifo_stack *stack)
{

    /* This is a temporary fix */
    if (memory_match(stack->buffer, "cd ", 3))
    {

        if (stack->head < 4)
            return;

        if (stack->buffer[3] == '/')
            call_open(CALL_DW, CALL_DR, stack->head - 5, stack->buffer + 4);
        else
            call_open(CALL_DW, CALL_DW, stack->head - 4, stack->buffer + 3);

        return;

    }

    call_open(CALL_L0, CALL_I0, 0, 0);
    call_open(CALL_L1, CALL_O0, 0, 0);
    call_open(CALL_L2, CALL_I1, 0, 0);
    call_open(CALL_P0, CALL_DR, 17, "system/pipe/clone");
    open_pipe(CALL_P0, CALL_L4, CALL_I0);
    call_write(CALL_L4, 0, stack->head, stack->buffer);
    call_slang(CALL_E0);
    close_pipe(CALL_P0, CALL_L4, CALL_I0);
    call_close(CALL_P0);
    call_open(CALL_I0, CALL_L0, 0, 0);
    call_open(CALL_O0, CALL_L1, 0, 0);
    call_open(CALL_I1, CALL_L2, 0, 0);

}

static void complete(struct lifo_stack *stack)
{

/*
    char buffer[FUDGE_BSIZE];
    unsigned int count;

    call_open(CALL_L0, CALL_I0, 0, 0);
    call_open(CALL_L1, CALL_O0, 0, 0);
    call_open(CALL_L2, CALL_I1, 0, 0);
    call_open(CALL_P0, CALL_DR, 17, "system/pipe/clone");
    open_pipe(CALL_P0, CALL_L4, CALL_I1);
    call_open(CALL_P2, CALL_DR, 17, "system/pipe/clone");
    open_pipe(CALL_P2, CALL_O0, CALL_L5);
    call_open(CALL_I0, CALL_DW, 0, 0);
    call_write(CALL_L4, 0, stack->head, stack->buffer);
    call_complete(CALL_E0);

    count = call_read(CALL_L5, 0, FUDGE_BSIZE, buffer);

    close_pipe(CALL_P0, CALL_L4, CALL_I1);
    call_close(CALL_P0);
    close_pipe(CALL_P2, CALL_O0, CALL_L5);
    call_close(CALL_P2);
    call_open(CALL_I0, CALL_L0, 0, 0);
    call_open(CALL_O0, CALL_L1, 0, 0);
    call_open(CALL_I1, CALL_L2, 0, 0);
    call_write(CALL_O0, 0, 1, "\n");
    call_write(CALL_O0, 0, count, buffer);
    call_write(CALL_O0, 0, 2, "$ ");
    call_write(CALL_O0, 0, stack->head, stack->buffer);
*/

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

            call_write(CALL_O0, 0, 3, "\b \b");

            break;

        case '\r':

            c = '\n';

        case '\n':

            lifo_stack_push(stack, c);
            call_write(CALL_O0, 0, 1, &c);
            interpret(stack);
            lifo_stack_clear(stack);
            call_write(CALL_O0, 0, 2, "$ ");

            break;

        default:

            lifo_stack_push(stack, c);
            call_write(CALL_O0, 0, 1, &c);

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

        count = call_read(CALL_I0, 0, FUDGE_BSIZE, buffer);

        for (i = 0; i < count; i++)
            handle(&input, buffer[i]);

    }

}

void main()
{

    call_write(CALL_O0, 0, 2, "$ ");
    poll();

}

