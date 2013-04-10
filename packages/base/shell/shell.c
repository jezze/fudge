#include <fudge.h>
#include <data/lifo.h>

static char map[256];

static void interpret(unsigned int count, char *buffer)
{

    /* This is a temporary fix */
    if (memory_match(buffer, "cd ", 3))
    {

        if (count < 4)
            return;

        if (buffer[3] == '/')
            call_open(CALL_DW, CALL_DR, count - 4, buffer + 4);
        else
            call_open(CALL_DW, CALL_DW, count - 3, buffer + 3);

        return;

    }

    call_open(CALL_D1, CALL_DI, 0, 0);
    call_open(CALL_DI, CALL_DR, 6, "temp/0");
    call_write(CALL_DI, 0, count, buffer);
    call_open(CALL_D0, CALL_DR, 9, "bin/slang");
    call_spawn(CALL_D0);
    call_open(CALL_DI, CALL_D1, 0, 0);

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

    count = call_read(CALL_DO, 0, FUDGE_BSIZE, buffer);

    call_open(CALL_DI, CALL_D1, 0, 0);
    call_open(CALL_DO, CALL_D2, 0, 0);
    call_open(CALL_DW, CALL_D3, 0, 0);

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
        case '\r':

            break;

        case '\t':

            complete(stack);

            break;

        case '\b':

            if (!lifo_stack_pop(stack))
                break;

            call_write(CALL_DO, 0, 3, "\b \b");

            break;

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
    unsigned int alt = 0;
    unsigned int shift = 0;
    unsigned int ctrl = 0;
    unsigned int i;

    lifo_stack_clear(&input);

    for (;;)
    {

        count = call_read(CALL_DI, 0, FUDGE_BSIZE, buffer);

        for (i = 0; i < count; i++)
        {

            unsigned int scancode = buffer[i];

            if (scancode & 0x80)
            {

                scancode &= ~0x80;

                if (scancode == 0x1D)
                    ctrl = 0;

                if (scancode == 0x2A)
                    shift = 0;

                if (scancode == 0x38)
                    alt = 0;

            }

            else
            {

                if (scancode == 0x1D)
                    ctrl = 1;

                if (scancode == 0x2A)
                    shift = 1;

                if (scancode == 0x38)
                    alt = 1;

                if (ctrl)
                    scancode = 0;

                if (alt)
                    scancode = 0;

                if (shift)
                    scancode += 128;

                handle(&input, map[scancode]);

            }

        }

    }

}

static void read_keymap(void *buffer)
{

    call_open(CALL_D1, CALL_DO, 0, 0);
    call_open(CALL_DO, CALL_DR, 6, "temp/0");
    call_open(CALL_D0, CALL_DR, 12, "bin/keymapus");
    call_spawn(CALL_D0);
    call_close(CALL_D0);
    call_read(CALL_DO, 0, 256, map);
    call_open(CALL_DO, CALL_D1, 0, 0);

}

void main()
{

    read_keymap(map);
    call_write(CALL_DO, 0, 2, "$ ");
    poll();

}

