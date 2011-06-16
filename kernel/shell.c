#include <lib/call.h>
#include <lib/file.h>
#include <lib/memory.h>
#include <lib/session.h>
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

    file_write_string(session_get_out(), "fudge:/$ ");
    shell_stack_clear();

}

static void shell_call(struct file_node *node, int argc, char *argv[])
{

    void *buffer = (void *)0x00300000;
    file_read(node, 0, node->length, buffer);

    unsigned int address = call_map((unsigned int)buffer);

    void (*func)(int argc, char **argv) = (void (*)(int argc, char **argv))address;

    func(argc, argv);

}

static void change_stdout(char *file)
{

    struct file_node *fileNode = call_open(file);

    if (!fileNode)
        return;

    struct file_node *stdoutNode = call_open("/dev/stdout");
    stdoutNode->write = fileNode->write;

}

static void shell_interpret(char *command)
{

    char *argv[32];
    unsigned int argc = string_split(argv, command, ' ');

    if (argc)
    {

        unsigned int i;

        for (i = 0; i < argc; i++)
        {

            string_trim(argv[i], ' ');

            if (!string_compare(argv[i], ">"))
            {

                change_stdout(argv[i + 1]);
                argc = i;

            }

        }

        struct file_node *initrd = call_open("/bin");
        struct file_node *node = file_find(initrd, argv[0]);

        if (node)
        {

            shell_call(node, argc, argv);

        }

        else
        {

            file_write_string(session_get_out(), argv[0]);
            file_write_string(session_get_out(), ": Command not found\n");

        }

    }

    change_stdout("/dev/tty");

    shell_clear();

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

                file_write_byte(session_get_out(), '\b');
                file_write_byte(session_get_out(), ' ');
                file_write_byte(session_get_out(), '\b');

             }

            break;

        case '\n':

            shell_stack_push('\0');
            file_write_byte(session_get_out(), c);
            shell_interpret(shellBuffer);

            break;

        default:

            shell_stack_push(c);
            file_write_byte(session_get_out(), c);

            break;

    }

}

static void shell_poll()
{

    char c = 0;

    for (;;)
    {

        while (!file_read(session_get_in(), 0, 1, &c));

        shell_handle_input(c);

    }

}

void shell_init()
{

    shellBufferHead = 0;

    int sin = call_open2("/dev/stdin");
    int sout = call_open2("/dev/stdout");

    void *version[] = {"PRE-ALPHA"};
    file_write_string_format(session_get_out(), "Fudge (%s)\n\n", version);
    file_write_string(session_get_out(), "Copyright (c) 2009 Jens Nyberg\n");
    file_write_string(session_get_out(), "Type 'cat help.txt' to read the help section.\n\n");

    shell_clear();
    shell_poll();

    call_close(sin);
    call_close(sout);

}

