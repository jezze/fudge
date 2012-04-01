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

static void clear()
{

    file_write_format(FILE_STDOUT, 0, "fudge:/$ ");
    stack_clear();

}

static void interpret(char *command)
{

    char *argv[32];
    unsigned int argc = string_split(argv, command, ' ');

    if (!argc)
        return;

    char binary[32];

    string_write(binary, "/ramdisk/bin/%s", argv[0]);

    if (!call_execute(binary, argc, argv))
        file_write_format(FILE_STDOUT, 0, "%s: Invalid command\n", binary);

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

            file_write_format(FILE_STDOUT, 0, "\b \b");

            break;

        case '\r':
        case '\n':

            stack_push('\0');
            file_write_format(FILE_STDOUT, 0, "%c", c);
            interpret(buffer);
            clear();

            break;

        default:

            stack_push(c);
            file_write_format(FILE_STDOUT, 0, "%c", c);

            break;

    }

}

static void read_keyboard()
{

    char buffer[32];

    unsigned int num = file_read(FILE_STDIN, 0, 32, buffer);
    unsigned int i;

    for (i = 0; i < num; i++)
        handle_input(buffer[i]);

    call_wait();

}

void main(int argc, char *argv[])
{

    file_write_format(FILE_STDOUT, 0, "Fudge operating system\n");
    file_write_format(FILE_STDOUT, 0, "Write `cat help.txt` for a short list if commands\n\n");

    clear();

    call_attach(0x01, read_keyboard);
    call_wait();

}

