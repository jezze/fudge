#include <fudge.h>
#include "lifo.h"

static void interpret(struct lifo_stack *stack)
{

    char buffer[4096];
    unsigned int count;

    /* This is a temporary fix */
    if (memory_match(stack->buffer, "cd ", 3))
    {

        unsigned int ok;

        if (stack->head < 4)
            return;

        if (stack->buffer[3] == '/')
            ok = call_walk(CALL_L0, CALL_DR, stack->head - 5, stack->buffer + 4);
        else
            ok = call_walk(CALL_L0, CALL_DW, stack->head - 4, stack->buffer + 3);

        if (ok)
            call_walk(CALL_DW, CALL_L0, 0, 0);

        return;

    }

    if (!call_walk(CALL_DP, CALL_DR, 9, "bin/slang"))
        return;

    if (!call_walk(CALL_L1, CALL_DR, 13, "system/pipe/0"))
        return;

    if (!call_walk(CALL_I1, CALL_DR, 13, "system/pipe/1"))
        return;

    if (!call_walk(CALL_O1, CALL_I1, 0, 0))
        return;

    call_open(CALL_L1);
    call_write(CALL_L1, 0, stack->head, stack->buffer);
    call_close(CALL_L1);
    call_spawn(CALL_DP);
    call_open(CALL_L1);
    count = call_read(CALL_L1, 0, 4096, buffer);
    call_close(CALL_L1);
    call_write(CALL_O0, 0, count, buffer);


}

static void complete(struct lifo_stack *stack)
{

}

static void handle(struct lifo_stack *stack, unsigned char c)
{

    switch (c)
    {

    case '\0':
        break;

    case '\t':
        complete(stack);

        break;

    case '\b':
    case 0x7F:
        if (!lifo_stack_pop(stack, 1))
            break;

        call_write(CALL_O0, 0, 3, "\b \b");

        break;

    case '\r':
        c = '\n';

    case '\n':
        lifo_stack_push(stack, 1, &c);
        call_write(CALL_O0, 0, 1, &c);
        interpret(stack);
        lifo_stack_clear(stack);
        call_write(CALL_O0, 0, 2, "$ ");

        break;

    default:
        lifo_stack_push(stack, 1, &c);
        call_write(CALL_O0, 0, 1, &c);

        break;

    }

}

static void poll()
{

    unsigned char buffer[FUDGE_BSIZE];
    unsigned int count, roff;
    unsigned char inputbuffer[FUDGE_BSIZE];
    struct lifo_stack input;

    lifo_stack_init(&input, FUDGE_BSIZE, inputbuffer);

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

