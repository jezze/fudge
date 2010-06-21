#include <lib/types.h>
#include <lib/cbuffer.h>
#include <lib/stack.h>
#include <lib/string.h>
#include <kernel/regs.h>
#include <kernel/screen.h>
#include <kernel/pit.h>
#include <kernel/kbd.h>
#include <kernel/vfs.h>
#include <kernel/shell.h>

char shellBuffer[SHELL_BUFFER_SIZE];
stack_t shellStack;

static void shell_clear()
{

    screen_puts("fudge:/$ ");
    stack_clear(&shellStack);

}

static void shell_command_call()
{

    vfs_node_t *node = vfs_find(fsRoot, "hello");

    if (!node)
        return;

    char buffer[2000];

    vfs_read(node, 0, 2000, buffer);
        
    void (*func)() = &buffer;

    func();

}

static void shell_command_cat()
{

    vfs_node_t *node = vfs_find(fsRoot, "about.txt");

    if (!node)
        return;

    char buffer[2000];

    uint32_t size = vfs_read(node, 0, 2000, buffer);
        
    uint32_t i;

    for (i = 0; i < size; i++)
        screen_putc(buffer[i]);

    screen_putc('\n');

}

static void shell_command_clear()
{

    screen_clear();

}

static void shell_command_ls()
{

    uint32_t i;
    vfs_node_t *node;

    for (i = 0; (node = vfs_walk(fsRoot, i)); i++)
    {

        if (node->flags == VFS_DIRECTORY)
            screen_puts("/");

        screen_puts(node->name);
        screen_puts("\n");

    }

}

static void shell_command_null()
{

    return;

}

static void shell_command_time()
{

    screen_puts("Timer: ");
    screen_puts_dec(pit_timer);
    screen_puts("\n");

}

static void shell_interpret(char *command)
{

    if (!string_compare(command, ""))
        shell_command_null();

    else if (!string_compare(command, "call"))
        shell_command_call();

    else if (!string_compare(command, "cat"))
        shell_command_cat();

    else if (!string_compare(command, "clear"))
        shell_command_clear();

    else if (!string_compare(command, "ls"))
        shell_command_ls();

    else if (!string_compare(command, "time"))
        shell_command_time();

    else
    {

        screen_puts(command);
        screen_puts(": Command not found\n");

    }

    shell_clear();

}

void shell_init()
{

    screen_puts("Fudge\n");
    screen_puts("Copyright (c) 2009 Jens Nyberg\n");
    screen_puts("Type 'help' for a list of commands.\n\n");

    shell_clear();

    shellStack = stack_create(shellBuffer, SHELL_BUFFER_SIZE);

    for (;;)
    {

        char c;

        if (!(c = cbuffer_read(&keyboard.cbuffer)))
            continue;

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

}

