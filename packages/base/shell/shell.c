#include <fudge.h>

#define BUFFER_SIZE 256

static char buffer[BUFFER_SIZE];
static unsigned int bufferHead;

static void replace(char *str, unsigned int length)
{

    unsigned int i;

    for (i = 0; i < length; i++)
    {

        if (str[i] == ' ')
            str[i] = '\0';

    }

}

static unsigned int find(char *str, char x, unsigned int length)
{

    unsigned int i;

    for (i = 0; i < length; i++)
    {

        if (str[i] == x)
            return i;

    }

    return 0;

}

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

static unsigned int setup_executable(char *path)
{

    char buffer[256];
    unsigned int length;

    if (memory_compare(path, "/", 1))
        return call_open(3, 0, path);

    length = string_length(path);

    memory_copy(buffer, "/ramdisk/bin/", 13);
    memory_copy(buffer + 13, path, length + 1);

    return call_open(3, 0, buffer);

}

static unsigned int setup_stream(char *path, unsigned int index)
{

    char buffer[256];
    unsigned int length;
    unsigned int count;

    if (memory_compare(path, "/", 1))
        return call_open(index, 0, path);

    length = string_length(path);

    call_open(4, 0, "/module/tty/cwd");
    count = call_read(4, 0, 256 - length, buffer);
    call_close(4);

    memory_copy(buffer + count, path, length + 1);

    return call_open(index, 0, buffer);

}

static void clear()
{

    call_write(FILE_STDOUT, 0, 2, "$ ");
    stack_clear();

}

static void interpret(char *command)
{

    unsigned int length;
    unsigned int sin;
    unsigned int sout;
    unsigned int data;
    unsigned int exec;

    length = string_length(command);

    if (!length)
        return;

    sin = find(command, '<', length);
    sout = find(command, '>', length);
    data = find(command, '-', length);

    if (data && (data < sin || data < sout))
        return;

    if (data)
        replace(command, data);
    else
        replace(command, length);

    exec = setup_executable(command);

    if (!exec)
        return;

    if (sin)
        setup_stream(command + sin + 1, FILE_STDIN);

    if (sout)
        setup_stream(command + sout + 1, FILE_STDOUT);

    if (data)
        call_write(FILE_STDIN, 0, length - data, command + data + 1);

    call_execute(exec);
    call_close(exec);

    setup_stream("/module/tty/stdin", FILE_STDIN);
    setup_stream("/module/tty/stdout", FILE_STDOUT);

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

            stack_push('\0');
            call_write(FILE_STDOUT, 0, 1, &c);
            interpret(buffer);
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

    call_wait();

}

void main()
{

    setup_stream("/module/tty/stdin", FILE_STDIN);
    setup_stream("/module/tty/stdout", FILE_STDOUT);

    call_write(FILE_STDOUT, 0, 23, "Fudge operating system\n");
    call_write(FILE_STDOUT, 0, 53, "Write `echo <help.txt` for a short list if commands\n\n");

    clear();

    call_attach(0x21, read_keyboard);
    call_wait();

}

