#include <fudge.h>
#include <data/lifo.h>

static char mapUS[256] =
{

       0,   27,  '1',  '2',  '3',  '4',  '5',  '6',  '7',  '8',  '9',  '0',  '-',  '+', '\b', '\t',
     'q',  'w',  'e',  'r',  't',  'y',  'u',  'i',  'o',  'p',  '[',  ']', '\n',    0,  'a',  's',
     'd',  'f',  'g',  'h',  'j',  'k',  'l',  ';', '\'',  '`',    0, '\\',  'z',  'x',  'c',  'v',
     'b',  'n',  'm',  ',',  '.',  '/',    0,    0,    0,  ' ',    0,    0,    0,    0,    0,    0,
       0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
       0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
       0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
       0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
       0,   27,  '!',  '@',  '#',  '$',  '%',  '^',  '&',  '*',  '(',  ')',  '_',  '=', '\b', '\t',
     'Q',  'W',  'E',  'R',  'T',  'Y',  'U',  'I',  'O',  'P',  '{',  '}', '\n',    0,  'A',  'S',
     'D',  'F',  'G',  'H',  'J',  'K',  'L',  ':', '\"',  '~',    0,  '|',  'Z',  'X',  'C',  'V',
     'B',  'N',  'M',  '<',  '>',  '?',    0,    0,    0,  ' ',    0,    0,    0,    0,    0,    0,
       0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
       0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
       0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
       0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0

};

static void interpret(unsigned int count, char *buffer)
{

    /* This is a temporary fix */
    if (memory_match(buffer, "cd ", 3))
    {

        if (count < 4)
            return;

        if (buffer[3] == '/')
            call_open(FUDGE_CWD, FUDGE_ROOT, count - 4, buffer + 4);
        else
            call_open(FUDGE_CWD, FUDGE_CWD, count - 3, buffer + 3);

        return;

    }

    call_open(4, FUDGE_IN, 0, 0);
    call_open(FUDGE_IN, FUDGE_ROOT, 6, "temp/0");
    call_open(3, FUDGE_ROOT, 17, "ramdisk/bin/slang");
    call_write(FUDGE_IN, 0, count, buffer);
    call_spawn(3);
    call_open(FUDGE_IN, 4, 0, 0);

}

static void complete(struct lifo_stack *stack)
{

    char buffer[FUDGE_BSIZE];
    unsigned int lines = 0;
    unsigned int count;
    unsigned int offset;
    unsigned int slash;
    unsigned int i;

    call_open(4, FUDGE_IN, 0, 0);
    call_open(5, FUDGE_OUT, 0, 0);
    call_open(6, FUDGE_CWD, 0, 0);
    call_open(FUDGE_IN, FUDGE_ROOT, 6, "temp/0");
    call_open(FUDGE_OUT, FUDGE_ROOT, 6, "temp/1");

    for (offset = stack->head; offset > 0; offset--)
    {

        if (stack->buffer[offset - 1] == ' ' || stack->buffer[offset - 1] == '>' || stack->buffer[offset - 1] == '<')
            break;

    }

    if (stack->head > 0)
    {

        for (slash = stack->head; slash > offset; slash--)
        {

            if (stack->buffer[slash - 1] != '/')
                continue;

            if (stack->buffer[offset] == '/')
                call_open(FUDGE_CWD, FUDGE_ROOT, stack->head - offset - 1, stack->buffer + offset + 1);
            else
                call_open(FUDGE_CWD, FUDGE_CWD, stack->head - offset, stack->buffer + offset);

            offset = slash;

            break;

        }

    }

    call_write(FUDGE_IN, 0, stack->head - offset, stack->buffer + offset);
    call_open(3, FUDGE_ROOT, 20, "ramdisk/bin/complete");
    call_spawn(3);

    count = call_read(FUDGE_OUT, 0, FUDGE_BSIZE, buffer);

    call_open(FUDGE_IN, 4, 0, 0);
    call_open(FUDGE_OUT, 5, 0, 0);
    call_open(FUDGE_CWD, 6, 0, 0);

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

        call_write(FUDGE_OUT, 0, 1, "\n");
        call_write(FUDGE_OUT, 0, count, buffer);
        call_write(FUDGE_OUT, 0, 2, "$ ");
        call_write(FUDGE_OUT, 0, stack->head, stack->buffer);

        return;

    }

    for (i = stack->head - offset; i < count - 1; i++)
    {

        lifo_stack_push(stack, buffer[i]);
        call_write(FUDGE_OUT, 0, 1, &buffer[i]);

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

            call_write(FUDGE_OUT, 0, 3, "\b \b");

            break;

        case '\n':

            lifo_stack_push(stack, c);
            call_write(FUDGE_OUT, 0, 1, &c);
            interpret(stack->head, stack->buffer);
            lifo_stack_clear(stack);
            call_write(FUDGE_OUT, 0, 2, "$ ");

            break;

        default:

            lifo_stack_push(stack, c);
            call_write(FUDGE_OUT, 0, 1, &c);

            break;

    }

}

static void poll()
{

    struct lifo_stack input;
    unsigned char buffer[FUDGE_BSIZE];
    unsigned int count;
    unsigned int i;
    unsigned int alt = 0;
    unsigned int shift = 0;
    unsigned int ctrl = 0;

    lifo_stack_clear(&input);

    for (;;)
    {

        count = call_read(FUDGE_IN, 0, FUDGE_BSIZE, buffer);

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

                handle(&input, mapUS[scancode]);

            }

        }

    }

}

void main()
{

    call_write(FUDGE_OUT, 0, 2, "$ ");
    poll();

}

