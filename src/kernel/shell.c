#include <lib/types.h>
#include <lib/cbuffer.h>
#include <lib/stack.h>
#include <lib/string.h>
#include <kernel/screen.h>
#include <kernel/kbd.h>
#include <kernel/vfs.h>
#include <kernel/shell.h>

char shellBuffer[SHELL_BUFFER_SIZE];
stack_t shellStack;

void shell_clear()
{

    screen_puts(&screen, "fudge:/$ ");
    stack_clear(&shellStack);

}

void shell_command_cat(int argc, char *argv[])
{

    if (argc > 1)
    {

        vfs_node_t *fsnode = vfs_directory_find(fsRoot, argv[1]);

        if (!fsnode)
            return;

        char buffer[256];

        uint32_t j;

        for (j = 0; j < vfs_read(fsnode, 0, 256, buffer); j++)
            screen_putc(&screen, buffer[j]);

        screen_putc(&screen, '\n');

    }

}

void shell_command_clear(int argc, char *argv[])
{

    screen_clear(&screen);

}

void shell_command_help(int argc, char *argv[])
{

    screen_puts(&screen, "cat - Show content of a file\n");
    screen_puts(&screen, "clear - Clear screen\n");
    screen_puts(&screen, "help - Show this dialog\n");
    screen_puts(&screen, "ls - List files and directories\n");

}

void shell_command_ls(int argc, char *argv[])
{

    int i = 0;
    vfs_node_t *node = 0;

    while ((node = vfs_directory_read(fsRoot, i)))
    {

        if ((node->flags & VFS_DIRECTORY))
        {

            screen_puts(&screen, "/");
            screen_puts(&screen, node->name);
            screen_puts(&screen, "\n");

        }

        else
        {

            screen_puts(&screen, node->name);
            screen_puts(&screen, "\n");

        }

        i++;

    }

}

void shell_command_null(int argc, char *argv[])
{

    return;

}

void shell_interpret(char *command)
{

    int argc = 0;
    char *argv[32];

    int start = 0;
    int current = 0;

    int i;
    int j = 0;

    while (command[current] != '\0')
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

        current++;

    }

    if (string_compare(argv[0], "") == 0)
        shell_command_null(argc, argv);

    else if (string_compare(argv[0], "cat") == 0)
        shell_command_cat(argc, argv);

    else if (string_compare(argv[0], "clear") == 0)
        shell_command_clear(argc, argv);

    else if (string_compare(argv[0], "help") == 0)
        shell_command_help(argc, argv);

    else if (string_compare(argv[0], "ls") == 0)
        shell_command_ls(argc, argv);

    else
    {

        screen_puts(&screen, argv[0]);
        screen_puts(&screen, ": Command not found\n");

    }

    shell_clear();

}

void shell_init()
{

    screen_clear(&screen);

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

