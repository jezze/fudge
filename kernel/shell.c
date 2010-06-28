#include <lib/types.h>
#include <lib/cbuffer.h>
#include <lib/stack.h>
#include <lib/string.h>
#include <kernel/regs.h>
#include <kernel/screen.h>
#include <kernel/pit.h>
#include <kernel/kbd.h>
#include <kernel/vfs.h>
#include <kernel/rtc.h>
#include <kernel/shell.h>

char shellBuffer[SHELL_BUFFER_SIZE];
stack_t shellStack;

static void shell_clear()
{

    screen_puts("fudge:/$ ");
    stack_clear(&shellStack);

}

static void shell_command_call(int argc, char *argv[])
{

    if (argc != 2)
        return;

    vfs_node_t *node = vfs_find(fsRoot, argv[1]);

    if (!node)
        return;

    char *buffer = (char *)0x200000;

    vfs_read(node, 0, 2000, buffer);
        
    void (*func)() = buffer;

    func();

}

static void shell_command_cat(int argc, char *argv[])
{

    if (argc != 2)
        return;

    vfs_node_t *node = vfs_find(fsRoot, argv[1]);

    if (!node)
        return;

    char buffer[2000];

    unsigned int size = vfs_read(node, 0, 2000, buffer);
        
    unsigned int i;

    for (i = 0; i < size; i++)
        screen_putc(buffer[i]);

    screen_putc('\n');

}

static void shell_command_clear()
{

    screen_clear();

}

static void shell_command_null()
{

    return;

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
        shell_command_null();

    else if (!string_compare(argv[0], "call"))
        shell_command_call(argc, argv);

    else if (!string_compare(argv[0], "cat"))
        shell_command_cat(argc, argv);

    else if (!string_compare(argv[0], "clear"))
        shell_command_clear();

    else
    {

        screen_puts(argv[0]);
        screen_puts(": Command not found\n");

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
    screen_puts("Copyright (c) 2009 Jens Nyberg\n");
    screen_puts("Type 'help' for a list of commands.\n\n");

    shell_clear();
    shell_poll();

}

