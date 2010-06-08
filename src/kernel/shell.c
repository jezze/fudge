#include <lib/types.h>
#include <lib/cbuffer.h>
#include <lib/stack.h>
#include <lib/string.h>
#include <kernel/screen.h>
#include <kernel/pit.h>
#include <kernel/kbd.h>
#include <kernel/vfs.h>
#include <kernel/shell.h>

char shellBuffer[SHELL_BUFFER_SIZE];
stack_t shellStack;

static void shell_clear()
{

    screen_puts(&screen, "fudge:/$ ");
    stack_clear(&shellStack);

}

static void shell_command_call(uint32_t argc, char *argv[])
{

    if (argc > 1)
    {

        vfs_node_t *node = vfs_find(fsRoot, argv[1]);

        if (!node)
            return;

        char buffer[256];

        vfs_read(node, 0, 256, buffer);
        
        void (*func)() = &buffer;

        func();

    }

}

static void shell_command_cat(uint32_t argc, char *argv[])
{

    if (argc > 1)
    {

        vfs_node_t *node = vfs_find(fsRoot, argv[1]);

        if (!node)
            return;

        char buffer[2000];

        uint32_t size = vfs_read(node, 0, 2000, buffer);
        
        uint32_t i;

        for (i = 0; i < size; i++)
            screen_putc(&screen, buffer[i]);

        screen_putc(&screen, '\n');

    }

}

static void shell_command_clear(uint32_t argc, char *argv[])
{

    screen_clear(&screen);

}

static void shell_command_help(uint32_t argc, char *argv[])
{

    argv[0] = "cat";
    argv[1] = "help.txt";

    shell_command_cat(2, argv);

}

static void shell_command_ls(uint32_t argc, char *argv[])
{

    uint32_t i;
    vfs_node_t *node;

    for (i = 0; (node = vfs_walk(fsRoot, i)); i++)
    {

        if (node->flags == VFS_DIRECTORY)
            screen_puts(&screen, "/");

        screen_puts(&screen, node->name);
        screen_puts(&screen, "\n");

    }

}

static void shell_command_null(uint32_t argc, char *argv[])
{

    return;

}

static void shell_command_time(uint32_t argc, char *argv[])
{

    screen_puts(&screen, "Timer: ");
    screen_puts_dec(&screen, &pit_timer);
    screen_puts(&screen, "\n");

}

static void shell_interpret(char *command)
{

    uint32_t argc = 0;
    char *argv[32];

    uint32_t start = 0;
    uint32_t current;

    uint32_t i;
    uint32_t j = 0;

    for (current = 0; command[current] != '\0'; current++)
    {

        if (command[current] == ' ' || command[current] == '\n')
        {

            j = 0;

            for (i = start; i < current; i++)
            {

                argv[argc][j] = command[i];

                j++;

            }

            argv[argc][j] = '\0';
            argc++;

            start = current + 1;

        }

    }

    if (!string_compare(argv[0], ""))
        shell_command_null(argc, argv);

    else if (!string_compare(argv[0], "call"))
        shell_command_call(argc, argv);

    else if (!string_compare(argv[0], "cat"))
        shell_command_cat(argc, argv);

    else if (!string_compare(argv[0], "clear"))
        shell_command_clear(argc, argv);

    else if (!string_compare(argv[0], "help"))
        shell_command_help(argc, argv);

    else if (!string_compare(argv[0], "ls"))
        shell_command_ls(argc, argv);

    else if (!string_compare(argv[0], "time"))
        shell_command_time(argc, argv);

    else
    {

        screen_puts(&screen, argv[0]);
        screen_puts(&screen, ": Command not found\n");

    }

    shell_clear();

}

void shell_init()
{

    screen_puts(&screen, "Fudge\n");
    screen_puts(&screen, "Copyright (c) 2009 Jens Nyberg\n");
    screen_puts(&screen, "Type 'help' for a list of commands.\n\n");

    shell_clear();

    shellStack = stack_create(shellBuffer, SHELL_BUFFER_SIZE);

    while (1)
    {

        char c;

        if ((c = cbuffer_read(&keyboard.cbuffer)))
        {

            switch (c)
            {

                case '\t':

                    break;

                case '\b':

                    if (stack_pop(&shellStack))
                    {

                        screen_putc(&screen, '\b');
                        screen_putc(&screen, ' ');
                        screen_putc(&screen, '\b');

                    }

                    break;

                case '\n':

                    stack_push(&shellStack, c);
                    stack_push(&shellStack, '\0');
                    screen_putc(&screen, c);
                    shell_interpret(shellBuffer);

                    break;

                default:

                    stack_push(&shellStack, c);
                    screen_putc(&screen, c);

                    break;

            }

        }

    }

}

