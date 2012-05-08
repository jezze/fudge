#include <fudge.h>

#define BUFFER_SIZE 256

char buffer[BUFFER_SIZE];
unsigned int bufferHead;

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

static unsigned int split(char *out[], char *in, char value)
{

    if (in[0] == '\0')
        return 0;

    unsigned int count = 1;
    out[0] = in;

    unsigned int i;

    for (i = 1; in[i] != '\0'; i++)
    {

        if (in[i - 1] == value)
        {

            in[i - 1] = '\0';
            out[count] = in + i;
            count++;

        }

    }

    return count;

}

static void clear()
{

    call_write(FILE_STDOUT, 0, 9, "fudge:/$ ");
    stack_clear();

}

static unsigned int setup_stream(char *path, unsigned int index)
{

    unsigned int length = string_length(path);

    if (!memory_compare(path, "<", 1))
        return call_write(FILE_STDIN, 0, length, path);

    if (memory_compare(path, "</", 2))
        return call_open(index, path + 1);

    char buffer[256];

    unsigned int id = call_open(FILE_NEW, "/module/tty/cwd");
    unsigned int count = call_read(id, 0, 256 - length, buffer);
    call_close(id);

    memory_copy(buffer + count, path + 1, length);

    return call_open(index, buffer);

}

static void interpret(char *command)
{

    char *argv[32];
    unsigned int argc = split(argv, command, ' ');

    if (!argc)
        return;

    if (argc > 1)
        setup_stream(argv[1], FILE_STDIN);

    char buffer[256];
    memory_copy(buffer, "/ramdisk/bin/", 13);
    memory_copy(buffer + 13, argv[0], string_length(argv[0]) + 1);

    unsigned int id = call_open(FILE_NEW, buffer);
    call_execute(id);
    call_close(id);

    setup_stream("</module/tty/stdin", FILE_STDIN);
    setup_stream("</module/tty/stdout", FILE_STDOUT);

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

    unsigned int num = call_read(FILE_STDIN, 0, 32, buffer);
    unsigned int i;

    for (i = 0; i < num; i++)
        handle_input(buffer[i]);

    call_wait();

}

void main()
{

    setup_stream("</module/tty/stdin", FILE_STDIN);
    setup_stream("</module/tty/stdout", FILE_STDOUT);

    call_write(FILE_STDOUT, 0, 23, "Fudge operating system\n");
    call_write(FILE_STDOUT, 0, 53, "Write `echo <help.txt` for a short list if commands\n\n");

    clear();

    call_attach(0x21, read_keyboard);
    call_wait();

}

