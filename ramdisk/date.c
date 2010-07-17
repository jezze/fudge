#include <cbuffer.h>
#include <stack.h>
#include <string.h>
#include <vfs.h>
#include <call.h>

char shellBuffer[256];
stack_t shellStack;
int run;

static void shell_clear()
{

    call_puts("date:/$ ");
    stack_clear(&shellStack);

}

static void shell_call(vfs_node_t *node, int argc, char *argv[])
{
/*
    char *buffer = (char *)0x200000;

    vfs_read(node, 0, 2000, buffer);
        
    void (*func)(int argc, char *argv[]) = (void (*)(int argc, char *argv[]))0x200000;

    func(argc, argv);
*/
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
/*
    char *argv[32];
    int argc = shell_get_arguments(argv, command);

    if (!string_compare(argv[0], ""))
    {

    }

    else
    {

        vfs_node_t *node = call_vfs_find(argv[0]);

        if (node)
            shell_call(node, argc, argv);

        else
        {

            call_puts(argv[0]);
            call_puts(": Command not found\n");

        }

    }

*/


}

static void shell_handle_input(char c)
{

    switch (c)
    {

        case '\t':

            run = 0;
            break;

        case '\b':

            if (stack_pop(&shellStack))
            {

                call_putc('\b');
                call_putc(' ');
                call_putc('\b');

             }

            break;

        case '\n':

            stack_push(&shellStack, '\0');
            call_putc(c);
//            shell_interpret(shellBuffer);
            call_puts(shellBuffer);
            shell_clear();

            break;

        default:

            stack_push(&shellStack, c);
            call_putc(c);

            break;

    }

}

static void shell_poll()
{

    run = 1;
    char c = 0;

    while (run)
    {

        if ((c = call_getc()))
            shell_handle_input(c);

    }

}


void main(int argc, char *argv[])
{

    shellStack = stack_create(shellBuffer, 256);

    shell_clear();
    shell_poll();

}

