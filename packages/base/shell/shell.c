#include <fudge.h>

#define BUFFER_SIZE 256

static char buffer[BUFFER_SIZE];
static unsigned int bufferHead;

static void stack_push(char c)
{

    if (bufferHead < BUFFER_SIZE)
    {

        buffer[bufferHead] = c;
        bufferHead++;

    }

}

static char stack_pop()
{

    return (bufferHead > 0) ? buffer[--bufferHead] : 0;

}

static void stack_clear()
{

    bufferHead = 0;

}

static unsigned int setup_executable(unsigned int length, char *path)
{

    char buffer[256];

    if (memory_match(path, "/", 1))
        return call_open(3, length, path);

    memory_copy(buffer, "/ramdisk/bin/", 13);
    memory_copy(buffer + 13, path, length);

    return call_open(3, length + 13, buffer);

}

static unsigned int setup_stream(unsigned int length, char *path, unsigned int index)
{

    char buffer[256];
    unsigned int count;

    if (memory_match(path, "/", 1))
        return call_open(index, length, path);

    call_open(4, 8, "/tty/cwd");
    count = call_read(4, 0, 256 - length, buffer);
    call_close(4);

    memory_copy(buffer + count, path, length);

    return call_open(index, count + length, buffer);

}

static void clear()
{

    call_write(FILE_STDOUT, 0, 2, "$ ");
    stack_clear();

}

#define STATE_NONE    0
#define STATE_COMMAND 1
#define STATE_STDIN   2
#define STATE_STDOUT  3
#define STATE_DATA    4

static void interpret(unsigned int length, char *command)
{

    unsigned int exec = 0;
    unsigned int i;
    unsigned int start = 0;
    unsigned int state = STATE_COMMAND;

    for (i = 0; i < length; i++)
    {

        if (command[i] == ' ')
        {

            if (state == STATE_COMMAND)
                exec = setup_executable(i - start, command + start);

            if (state == STATE_STDIN)
                setup_stream(i - start, command + start, FILE_STDIN);

            if (state == STATE_STDOUT)
                setup_stream(i - start, command + start, FILE_STDOUT);

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
        exec = setup_executable(length - start, command + start);

    if (state == STATE_STDIN)
        setup_stream(length - start, command + start, FILE_STDIN);

    if (state == STATE_STDOUT)
        setup_stream(length - start, command + start, FILE_STDOUT);

    if (state == STATE_DATA)
        call_write(FILE_STDIN, 0, length - start, command + start);

    if (exec)
    {

        call_execute(exec);
        call_close(exec);

    }

    setup_stream(10, "/tty/stdin", FILE_STDIN);
    setup_stream(11, "/tty/stdout", FILE_STDOUT);

}

static void handle_input(char c)
{

    switch (c)
    {

        case '\0':
        case '\t':

            break;

        case '\b':

            if (!stack_pop())
                break;

            call_write(FILE_STDOUT, 0, 3, "\b \b");

            break;

        case '\r':
        case '\n':

            call_write(FILE_STDOUT, 0, 1, &c);

            if (bufferHead)
                interpret(bufferHead, buffer);

            clear();

            break;

        default:

            stack_push(c);
            call_write(FILE_STDOUT, 0, 1, &c);

            break;

    }

}

static void read_keyboard()
{

    char buffer[32];
    unsigned int count;
    unsigned int i;

    count = call_read(FILE_STDIN, 0, 32, buffer);

    for (i = 0; i < count; i++)
        handle_input(buffer[i]);

    call_idle();

}

void main()
{

    clear();

    call_attach(0x21, read_keyboard);
    call_idle();

}

