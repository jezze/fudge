#include <fudge.h>
#include <data/lifo.h>

#define STATE_NONE    0
#define STATE_COMMAND 1
#define STATE_STDIN   2
#define STATE_STDOUT  3
#define STATE_DATA    4

static struct lifo_stack input;

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

static unsigned int setup_executable(unsigned int length, char *path)
{

    char buffer[FUDGE_BSIZE];
    unsigned int offset = 0;

    if (memory_match(path, "/", 1))
        return call_open(3, FUDGE_ROOT, length - 1, path + 1);

    offset += memory_write(buffer, FUDGE_BSIZE, "ramdisk/bin/", 12, offset);
    offset += memory_write(buffer, FUDGE_BSIZE, path, length, offset);

    return call_open(3, FUDGE_ROOT, offset, buffer);

}

static unsigned int setup_stream(unsigned int index, unsigned int length, char *path)
{

    if (memory_match(path, "/", 1))
        return call_open(index, FUDGE_ROOT, length - 1, path + 1);

    return call_open(index, FUDGE_CWD, length, path);

}

static void clear()
{

    lifo_stack_clear(&input);
    setup_stream(FUDGE_IN, 18, "/system/ps2_buffer");
    setup_stream(FUDGE_OUT, 18, "/system/vga_buffer");
    call_write(FUDGE_OUT, 0, 2, "$ ");

}

static void interpret_command(unsigned int length, char *command)
{

    unsigned int exec = 0;
    unsigned int start = 0;
    unsigned int state = STATE_COMMAND;
    unsigned int i;

    for (i = 0; i < length; i++)
    {

        if (command[i] == ' ')
        {

            if (state == STATE_COMMAND)
            {

                exec = setup_executable(i - start, command + start);

                if (!exec)
                    return;

            }

            if (state == STATE_STDIN)
            {

                unsigned int id = setup_stream(FUDGE_IN, i - start, command + start);

                if (!id)
                    return;

            }

            if (state == STATE_STDOUT)
            {

                unsigned int id = setup_stream(FUDGE_OUT, i - start, command + start);

                if (!id)
                    return;

            }

            state = STATE_NONE;

        }

        if (command[i] == '<')
        {

            state = STATE_STDIN;
            start = i + 1;

        }

        if (command[i] == '>')
        {

            state = STATE_STDOUT;
            start = i + 1;

        }

        if (command[i] == '-')
        {

            state = STATE_DATA;
            start = i + 1;

            break;

        }

    }

    if (state == STATE_COMMAND)
    {

        exec = setup_executable(length - start, command + start);

        if (!exec)
            return;

    }

    if (state == STATE_STDIN)
    {

        unsigned int id = setup_stream(FUDGE_IN, length - start, command + start);

        if (!id)
            return;

    }

    if (state == STATE_STDOUT)
    {

        unsigned int id = setup_stream(FUDGE_OUT, length - start, command + start);

        if (!id)
            return;

    }

    if (state == STATE_DATA)
        call_write(FUDGE_IN, 0, length - start, command + start);

    if (exec)
    {

        call_spawn(3);
        call_close(3);

    }

}

static void changedir(unsigned int length, char *command)
{

    unsigned int id;
    
    if (!length)
        return;

    if (command[length - 1] != '/')
        return;

    id = (command[0] == '/') ? call_open(3, FUDGE_ROOT, length - 1, command + 1) : call_open(3, FUDGE_CWD, length, command);

    if (!id)
        return;

    call_open(FUDGE_CWD, 3, 0, 0);

}

static void interpret(unsigned int length, char *command)
{

    unsigned int start = 0;
    unsigned int i;

    if (memory_match(command, "cd ", 3))
    {

        changedir(length - 3, command + 3);

        return;

    }

    for (i = 0; i < length; i++)
    {

        if (command[i] != '|')
            continue;

        interpret_command(i - start, command + start);

        start += i + 1;

    }

    interpret_command(i - start, command + start);

}

static void handle_input(char c)
{

    switch (c)
    {

        case '\0':
        case '\t':

            break;

        case '\b':

            if (!lifo_stack_pop(&input))
                break;

            call_write(FUDGE_OUT, 0, 3, "\b \b");

            break;

        case '\r':
        case '\n':

            call_write(FUDGE_OUT, 0, 1, &c);

            if (!lifo_stack_isempty(&input))
                interpret(input.head, input.buffer);

            clear();

            break;

        default:

            lifo_stack_push(&input, c);
            call_write(FUDGE_OUT, 0, 1, &c);

            break;

    }

}

static void poll()
{

    unsigned char buffer[FUDGE_BSIZE];
    unsigned int count;
    unsigned int i;
    unsigned int toggleAlt = 0;
    unsigned int toggleShift = 0;
    unsigned int toggleCtrl = 0;

    for (;;)
    {

        count = call_read(FUDGE_IN, 0, FUDGE_BSIZE, buffer);

        for (i = 0; i < count; i++)
        {

            unsigned int scancode = buffer[i];

            if (scancode == 0x38)
                toggleAlt = 1;

            if (scancode == 0xB8)
                toggleAlt = 0;

            if (scancode == 0x1D)
                toggleCtrl = 1;

            if (scancode == 0x9D)
                toggleCtrl = 0;

            if (scancode == 0x2A)
                toggleShift = 1;

            if (scancode == 0xAA)
                toggleShift = 0;

            if (scancode & 0x80)
            {

            }

            else
            {

                if (toggleCtrl)
                    scancode = 0;

                if (toggleAlt)
                    scancode = 0;

                if (toggleShift)
                    scancode += 128;

                handle_input(mapUS[scancode]);

            }

        }

    }

}

void main()
{

    clear();
    poll();

}

