#include <lib/call.h>
#include <lib/file.h>
#include <lib/memory.h>
#include <lib/string.h>
#include <kernel/shell.h>

static char shellBuffer[SHELL_BUFFER_SIZE];
static unsigned int shellBufferHead;

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

    file_write_string(FILE_STDOUT, "fudge:/$ ");
    shell_stack_clear();

}

static void shell_call(int file, int argc, char *argv[])
{

    void *buffer = (void *)0x00300000;
    file_read(file, 0x100000, buffer);

    unsigned int address = call_map((unsigned int)buffer);

    void (*func)(int argc, char **argv) = (void (*)(int argc, char **argv))address;

    func(argc, argv);

}

static void shell_interpret(char *command)
{

    char *argv[32];
    unsigned int argc = string_split(argv, command, ' ');

    if (!argc)
        return;

    char path[256];

    string_copy(path, "/initrd/");
    string_concat(path, argv[0]);

    int file = file_open(path);

    if (file == -1)
    {

        file_write_string(FILE_STDOUT, argv[0]);
        file_write_string(FILE_STDOUT, ": Command not found\n");

        return;

    }

    shell_call(file, argc, argv);
    file_close(file);

}

static void shell_handle_input(char c)
{

    switch (c)
    {

        case '\t':

            break;

        case '\b':

            if (shell_stack_pop())
            {

                file_write_byte(FILE_STDOUT, '\b');
                file_write_byte(FILE_STDOUT, ' ');
                file_write_byte(FILE_STDOUT, '\b');

             }

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

void shell_init()
{

    shellBufferHead = 0;

    int sin = file_open("/dev/kbd");
    int sout = file_open("/dev/tty");
    int serr = file_open("/dev/serial");

    file_write_string(FILE_STDOUT, "Fudge\n\n");
    file_write_string(FILE_STDOUT, "Copyright (c) 2009 Jens Nyberg\n");
    file_write_string(FILE_STDOUT, "Type 'cat help.txt' to read the help section.\n\n");

    file_write_string(FILE_STDERR, "Fudge\n\n");
    file_write_string(FILE_STDERR, "Copyright (c) 2009 Jens Nyberg\n");
    file_write_string(FILE_STDERR, "Debug window.\n\n");

    shell_clear();
    shell_poll();

    file_close(sin);
    file_close(sout);
    file_close(serr);

}

