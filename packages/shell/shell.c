#include <fudge.h>
#include "lifo.h"

static void interpret(struct lifo_stack *stack)
{

    /* This is a temporary fix */
    if (memory_match(stack->buffer, "cd ", 3))
    {

        unsigned int ret;

        if (stack->head < 4)
            return;

        if (stack->buffer[3] == '/')
            ret = call_walk(CALL_DT, CALL_DR, stack->head - 5, stack->buffer + 4);
        else
            ret = call_walk(CALL_DT, CALL_DW, stack->head - 4, stack->buffer + 3);

        if (ret)
        {

            call_open(CALL_DT);

            if (call_walk(CALL_DW, CALL_DT, 0, 0))
                call_open(CALL_DW);

            call_close(CALL_DT);

        }

        return;

    }

    if (!call_walk(CALL_O1, CALL_O0, 0, 0))
        return;

    if (!call_walk(CALL_L0, CALL_DR, 9, "bin/slang"))
        return;

    if (!call_walk(CALL_L1, CALL_DR, 17, "system/pipe/clone"))
        return;

    call_open(CALL_O1);
    call_open(CALL_L0);
    call_open(CALL_L1);

    if (call_walk(CALL_L2, CALL_L1, 4, "../0") && call_walk(CALL_I1, CALL_L1, 4, "../1"))
    {

        call_open(CALL_L2);
        call_open(CALL_I1);
        call_write(CALL_L2, 0, stack->head, stack->buffer);
        call_spawn(CALL_L0);
        call_close(CALL_I1);
        call_close(CALL_L2);

    }

    call_close(CALL_L1);
    call_close(CALL_L0);
    call_close(CALL_O1);

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

    for (roff = 0; (count = call_read(CALL_I0, roff, FUDGE_BSIZE, buffer)); roff += count)
    {

        unsigned int i;

        for (i = 0; i < count; i++)
            handle(&input, buffer[i]);

    }

}

void main()
{

    call_write(CALL_O0, 0, 2, "$ ");
    poll();

}

