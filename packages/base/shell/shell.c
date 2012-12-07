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
    unsigned int o = 0;

    if (memory_match(path, "/", 1))
        return call_open(3, length, path);

    o += memory_write(buffer, 256, "/ramdisk/bin/", 13, 0);
    o += memory_write(buffer, 256, path, length, 13);

    return call_open(3, o, buffer);

}

static unsigned int setup_stream(unsigned int length, char *path, unsigned int index)
{

    char buffer[256];
    unsigned int count;

    if (memory_match(path, "/", 1))
        return call_open(index, length, path);

    call_open(4, 12, "/dev/tty/cwd");
    count = call_read(4, 0, 256, buffer);
    call_close(4);

    count += memory_write(buffer, 256, path, length, count);

    return call_open(index, count, buffer);

}

static void clear()
{

    char buffer[128];
    unsigned int count;

    call_open(3, 12, "/dev/tty/cwd");
    count = call_read(3, 0, 128, buffer);
    call_close(3);

    call_write(FILE_STDOUT, 0, count, buffer);
    call_write(FILE_STDOUT, 0, 2, "$ ");
    stack_clear();

    setup_stream(22, "/dev/nodefs/ps2_buffer", FILE_STDIN);
    setup_stream(17, "/dev/video/0/data", FILE_STDOUT);

}

#define STATE_NONE    0
#define STATE_COMMAND 1
#define STATE_STDIN   2
#define STATE_STDOUT  3
#define STATE_DATA    4

static void interpret_command(unsigned int length, char *command)
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
            {

                exec = setup_executable(i - start, command + start);

                if (!exec)
                    return;

            }

            if (state == STATE_STDIN)
            {

                unsigned int id = setup_stream(i - start, command + start, FILE_STDIN);

                if (!id)
                    return;

            }

            if (state == STATE_STDOUT)
            {

                unsigned int id = setup_stream(i - start, command + start, FILE_STDOUT);

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

        unsigned int id = setup_stream(length - start, command + start, FILE_STDIN);

        if (!id)
            return;

    }

    if (state == STATE_STDOUT)
    {

        unsigned int id = setup_stream(length - start, command + start, FILE_STDOUT);

        if (!id)
            return;

    }

    if (state == STATE_DATA)
        call_write(FILE_STDIN, 0, length - start, command + start);

    if (exec)
    {

        call_spawn(exec);
        call_close(exec);

    }

}

static void interpret(unsigned int length, char *command)
{

    unsigned int start = 0;
    unsigned int i;

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

static void poll()
{

    char buffer[32];
    unsigned int count;
    unsigned int i;

    for (;;)
    {

        count = call_read(FILE_STDIN, 0, 32, buffer);

        for (i = 0; i < count; i++)
            handle_input(buffer[i]);

    }

}

void main()
{

    clear();
    poll();

}

