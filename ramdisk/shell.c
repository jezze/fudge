#include <call.h>
#include <cbuffer.h>
#include <stack.h>
#include <string.h>
#include <vfs.h>

char consoleBuffer[256];
struct stack consoleStack;

static void console_clear()
{

    call_puts("fudge:/$ ");
    stack_clear(&consoleStack);

}

static void console_call(vfs_node_t *node, int argc, char *argv[])
{

    char *buffer = (char *)0x200000;

    vfs_read(node, 0, 5000, buffer);
        
    void (*func)(int argc, char *argv[]) = (void (*)(int argc, char *argv[]))0x200000;

    func(argc, argv);

}

static int console_get_arguments(char *argv[], char *command)
{

    int argc = 0;
    int length = string_length(command);
    char *base = command;

    int i = 0;

    for (i = 0; i < length; i++)
    {

        if (command[i] == ' ')
        {

            argv[argc] = base;
            argc++;

            command[i] = 0;
            base = &command[i + 1];

        }

    }

    argv[argc] = base;
    argc++;

    return argc;

}

static void console_interpret(char *command)
{

    char *argv[32];
    int argc = console_get_arguments(argv, command);

    if (!string_compare(argv[0], ""))
    {

    }

    else
    {

        vfs_node_t *node = call_vfs_find(argv[0]);

        if (node)
            console_call(node, argc, argv);

        else
        {

            call_puts(argv[0]);
            call_puts(": Command not found\n");

        }

    }

    console_clear();

}

static void console_handle_input(char c)
{

    switch (c)
    {

        case '\t':

            break;

        case '\b':

            if (stack_pop(&consoleStack))
            {

                call_putc('\b');
                call_putc(' ');
                call_putc('\b');

             }

            break;

        case '\n':

            stack_push(&consoleStack, '\0');
            call_putc(c);
            console_interpret(consoleBuffer);

            break;

        default:

            stack_push(&consoleStack, c);
            call_putc(c);

            break;

    }

}

static void console_poll()
{

    for (;;)
    {

        console_handle_input(call_getc());

    }

}

void main(int argc, char *argv[])
{

    stack_init(&consoleStack, consoleBuffer, 256);

    call_puts("Fudge\n");
    call_puts("-----\n");
    call_puts("Copyright (c) 2009 Jens Nyberg\n");
    call_puts("Type 'cat help.txt' to read the help section.\n\n");

    console_clear();
    console_poll();

}

