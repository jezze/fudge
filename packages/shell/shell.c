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

    call_write(CALL_DI, 0, count, buffer);
    call_open(CALL_D0, CALL_DR, 9, "bin/slang");
    call_spawn(CALL_D0);
    call_close(CALL_D0);

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

