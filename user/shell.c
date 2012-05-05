#include <fudge.h>

#define SHELL_BUFFER_SIZE 256

char buffer[SHELL_BUFFER_SIZE];
unsigned int bufferHead;

static void stack_push(char c)
{

    if (bufferHead < SHELL_BUFFER_SIZE)
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

static void interpret(char *command)
{

    char *argv[32];
    unsigned int argc = split(argv, command, ' ');

    if (!argc)
        return;

    char buffer[256];

    unsigned int fd = call_open("/module/tty/cwd");
    unsigned int count = call_read(fd, 0, 256, buffer);
    call_close(fd);

    unsigned int i;

    for (i = 1; i < argc; i++)
    {

        if (argv[i][0] == '/')
        {

            call_open(argv[i]);

        }

        else
        {

            memory_copy(buffer + count, argv[i], string_length(argv[i]) + 1);

            call_open(buffer);

        }

    }

    memory_copy(buffer, "/ramdisk/bin/", 13);
    memory_copy(buffer + 13, argv[0], string_length(argv[0]) + 1);

    call_execute(buffer, argc, argv);

    for (i = 1; i < argc; i++)
    {

        call_close(i + 2);

    }

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

void main(int argc, char *argv[])
{

    call_write(FILE_STDOUT, 0, 23, "Fudge operating system\n");
    call_write(FILE_STDOUT, 0, 51, "Write `cat help.txt` for a short list if commands\n\n");
    clear();
    call_attach(0x21, read_keyboard);
    call_wait();

}

