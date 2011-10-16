#include <fudge.h>

#define SHELL_BUFFER_SIZE 256

char shellBuffer[SHELL_BUFFER_SIZE];
unsigned int shellBufferHead;

static void shell_stack_push(char c)
{

    if (shellBufferHead < SHELL_BUFFER_SIZE)
    {

        shellBuffer[shellBufferHead] = c;
        shellBufferHead++;

    }

}

static char shell_stack_pop()
{

    return (shellBufferHead > 0) ? shellBuffer[--shellBufferHead] : 0;

}

static void shell_stack_clear()
{

    shellBufferHead = 0;

}

static void shell_clear()
{

    file_write_format(FILE_STDOUT, "shell:/$ ");
    shell_stack_clear();

}

static void shell_interpret(char *command)
{

    char *argv[32];
    unsigned int argc = string_split(argv, command, ' ');

    if (!argc)
        return;

    char path[256];

    string_copy(path, "/");
    string_concat(path, argv[0]);

    if (!call_execute(path, argc, argv))
        file_write_format(FILE_STDOUT, "Could not execute program: %s\n", path);

}

static void shell_handle_input(char c)
{

    switch (c)
    {

        case '\t':

            break;

        case '\b':

            if (!shell_stack_pop())
                break;

            file_write_byte(FILE_STDOUT, '\b');
            file_write_byte(FILE_STDOUT, ' ');
            file_write_byte(FILE_STDOUT, '\b');

            break;

        case '\n':

            shell_stack_push('\0');
            file_write_byte(FILE_STDOUT, c);
            shell_interpret(shellBuffer);
            shell_clear();

            break;

        default:

            shell_stack_push(c);
            file_write_byte(FILE_STDOUT, c);

            break;

    }

}

static void shell_poll()
{

    char c;

    for (;;)
    {

        while (!file_read(FILE_STDIN, 1, &c));

        shell_handle_input(c);

    }

}

void main(int argc, char *argv[])
{

    shell_clear();
    shell_poll();

}

