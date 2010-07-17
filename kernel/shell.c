#include <lib/types.h>
#include <lib/cbuffer.h>
#include <lib/stack.h>
#include <lib/string.h>
#include <lib/vfs.h>
#include <kernel/kernel.h>
#include <kernel/isr.h>
#include <kernel/screen.h>
#include <kernel/kbd.h>
#include <kernel/shell.h>

char shellBuffer[SHELL_BUFFER_SIZE];
stack_t shellStack;

static void shell_clear()
{

    screen_puts("fudge:/$ ");
    stack_clear(&shellStack);

}

static void shell_call(vfs_node_t *node, int argc, char *argv[])
{

    char *buffer = (char *)0x200000;

    vfs_read(node, 0, 5000, buffer);
        
    void (*func)(int argc, char *argv[]) = (void (*)(int argc, char *argv[]))0x200000;

    func(argc, argv);

}

static int shell_get_arguments(char *argv[], char *command)
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

static void shell_interpret(char *command)
{

    char *argv[32];
    int argc = shell_get_arguments(argv, command);

    if (!string_compare(argv[0], ""))
    {

    }

    else if (!string_compare(argv[0], "clear"))
    {

        screen_clear();

    }

    else
    {

        vfs_node_t *node = vfs_find(fsRoot, argv[0]);

        if (node)
            shell_call(node, argc, argv);

        else
        {

            screen_puts(argv[0]);
            screen_puts(": Command not found\n");

        }

    }

    shell_clear();

}

static void shell_handle_input(char c)
{

    switch (c)
    {

        case '\t':

            break;

        case '\b':

            if (stack_pop(&shellStack))
            {

                screen_putc('\b');
                screen_putc(' ');
                screen_putc('\b');

             }

            break;

        case '\n':

            stack_push(&shellStack, '\0');
            screen_putc(c);
            shell_interpret(shellBuffer);

            break;

        default:

            stack_push(&shellStack, c);
            screen_putc(c);

            break;

    }

}

static void shell_poll()
{

    for (;;)
    {

        char c;

        if ((c = cbuffer_read(&keyboard.cbuffer)))
            shell_handle_input(c);

    }

}

void shell_init()
{

    shellStack = stack_create(shellBuffer, SHELL_BUFFER_SIZE);

    screen_puts("Fudge\n");
    screen_puts("-----\n");
    screen_puts("Copyright (c) 2009 Jens Nyberg\n");
    screen_puts("Type 'cat help.txt' to read the help section.\n\n");

    shell_clear();
    shell_poll();

}

