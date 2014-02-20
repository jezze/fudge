#include <fudge.h>
#include "lifo.h"

static void open_pipe(unsigned int index, unsigned int index0, unsigned int index1)
{

    call_open(index, CALL_DR, 17, "system/pipe/clone");
    call_open(index0, index, 4, "../0");
    call_open(index1, index, 4, "../1");

}

static void close_pipe(unsigned int index, unsigned int index0, unsigned int index1)
{

    call_close(index0);
    call_close(index1);
    call_close(index);

}

static void call_slang(unsigned int index)
{

    call_open(index, CALL_DR, 9, "bin/slang");
    call_spawn(index);
    call_close(index);

}

static void call_complete(unsigned int index)
{

    call_open(index, CALL_DR, 12, "bin/complete");
    call_spawn(index);
    call_close(index);

}

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
    open_pipe(CALL_P0, CALL_L1, CALL_I0);
    call_write(CALL_L1, 0, stack->head, stack->buffer);
    call_slang(CALL_E0);
    close_pipe(CALL_P0, CALL_L1, CALL_I0);
    call_open(CALL_I0, CALL_L0, 0, 0);

}

static int is_multi(unsigned int count, char *buffer)
{

    unsigned int i;

    for (i = 0; i < count - 1; i++)
    {

        if (buffer[i] == '\n')
            return 1;

    }

    return (buffer[count - 1] == '\n') ? 0 : 1;

}

static void complete(struct lifo_stack *stack)
{

    char buffer[FUDGE_BSIZE];
    unsigned int count;
    unsigned int offset = stack->head;
    unsigned int offset2 = stack->head;

    call_open(CALL_L0, CALL_I0, 0, 0);
    call_open(CALL_L1, CALL_O0, 0, 0);
    call_open(CALL_L2, CALL_I1, 0, 0);
    call_open(CALL_L3, CALL_DW, 0, 0);
    open_pipe(CALL_P0, CALL_L4, CALL_I1);
    open_pipe(CALL_P1, CALL_O0, CALL_L5);

    while (offset && stack->buffer[offset - 1] != ' ')
        offset--;

    if (stack->buffer[offset] == '/')
    {

        offset++;

        call_open(CALL_DW, CALL_DR, 0, 0);

    }

    while (offset2 > offset && stack->buffer[offset2 - 1] != '/')
        offset2--;

    if (offset2 != offset)
    {

        call_open(CALL_I0, CALL_DW, (stack->head - offset) - (stack->head - offset2), stack->buffer + offset);
        call_write(CALL_L4, 0, stack->head - offset2, stack->buffer + offset2);

    }

    else
    {

        call_open(CALL_I0, CALL_DW, 0, 0);
        call_write(CALL_L4, 0, stack->head - offset, stack->buffer + offset);

    }

    call_complete(CALL_E0);

    count = call_read(CALL_L5, 0, FUDGE_BSIZE, buffer);

    close_pipe(CALL_P0, CALL_L4, CALL_I1);
    close_pipe(CALL_P1, CALL_O0, CALL_L5);
    call_open(CALL_I0, CALL_L0, 0, 0);
    call_open(CALL_O0, CALL_L1, 0, 0);
    call_open(CALL_I1, CALL_L2, 0, 0);
    call_open(CALL_DW, CALL_L3, 0, 0);

    if (!count)
        return;

    if (is_multi(count, buffer))
    {

        call_write(CALL_O0, 0, 1, "\n");
        call_write(CALL_O0, 0, count, buffer);
        call_write(CALL_O0, 0, 2, "$ ");
        call_write(CALL_O0, 0, stack->head, stack->buffer);

    }

    else
    {

        unsigned int head = stack->head;
        unsigned int i;

        for (i = stack->head - offset2; i < count - 1; i++)
            lifo_stack_push(stack, buffer[i]);

        call_write(CALL_O0, 0, stack->head - head, stack->buffer + head);

    }

}

static void handle(struct lifo_stack *stack, unsigned int c, unsigned int size)
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
            call_write(CALL_O0, 0, size, &c);
            interpret(stack);
            lifo_stack_clear(stack);
            call_write(CALL_O0, 0, 2, "$ ");

            break;

        default:

            lifo_stack_push(stack, c);
            call_write(CALL_O0, 0, size, &c);

            break;

    }

}

static void poll()
{

    struct lifo_stack input;

    lifo_stack_clear(&input);

    for (;;)
    {

        char buffer[64];
        unsigned int count = call_read(CALL_I0, 0, 64, buffer);
        unsigned int size;
        unsigned int offset;

        for (offset = 0; offset < count && (size = utf8_size(buffer + offset)); offset += size)
        {

            unsigned int value;

            memory_copy(&value, buffer + offset, size);
            handle(&input, value, size);

        }

    }

}

void main()
{

    call_write(CALL_O0, 0, 2, "$ ");
    poll();

}

