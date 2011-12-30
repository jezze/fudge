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

    file_write_format(FILE_STDOUT, "fudge:/$ ");
    stack_clear();

}

static void interpret(char *command)
{

    char *argv[32];
    unsigned int argc = string_split(argv, command, ' ');

    if (!argc)
        return;

    if (!call_execute(argv[0], argc, argv))
        file_write_format(FILE_STDOUT, "%s: Invalid command\n", argv[0]);

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

            file_write_byte(FILE_STDOUT, '\b');
            file_write_byte(FILE_STDOUT, ' ');
            file_write_byte(FILE_STDOUT, '\b');

            break;

        case '\r':
        case '\n':

            stack_push('\0');
            file_write_byte(FILE_STDOUT, c);
            interpret(buffer);
            clear();

            break;

        default:

            stack_push(c);
            file_write_byte(FILE_STDOUT, c);

            break;

    }

}

static void read_keyboard()
{

    char c;

    unsigned int num = file_read(FILE_STDIN, 1, &c);

    if (num)
        handle_input(c);

    call_wait();

}

static void poll()
{

    char c;

    for (;;)
    {

        while (!file_read(FILE_STDIN, 1, &c));

        handle_input(c);

    }

}

void main(int argc, char *argv[])
{

    file_write_format(FILE_STDOUT, "Fudge operating system\n");
    file_write_format(FILE_STDOUT, "Write `cat help.txt` for a short list if commands\n\n");

    clear();
    poll();

//    Sadly this is buggy. Need to fix.
//    call_attach(0x01, read_keyboard);
//    call_wait();

}

