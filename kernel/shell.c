#include <lib/call.h>
#include <lib/elf.h>
#include <lib/memory.h>
#include <lib/stack.h>
#include <lib/file.h>
#include <lib/session.h>
#include <lib/string.h>
#include <lib/vfs.h>
#include <kernel/elf.h>
#include <kernel/kernel.h>
#include <kernel/shell.h>

char shellBuffer[SHELL_BUFFER_SIZE];
struct stack shellStack;

static void shell_clear()
{

    file_write_string(session_get_out(), "fudge:/$ ");
    stack_clear(&shellStack);

}

static void shell_execute_flat(unsigned int *address, int argc, char *argv[])
{

    void (*func)(int argc, char *argv[]) = (void (*)(int argc, char *argv[]))address;

    func(argc, argv);

}

static void shell_execute_elf(struct vfs_node *node, struct elf_header *header, int argc, char *argv[])
{

    struct elf_program_header *pHeader = (struct elf_program_header *)(0x280000 + header->programHeaderOffset);

    file_read(node, pHeader->offset, pHeader->memorySize, (void *)pHeader->virtualAddress);

    shell_execute_flat((unsigned int *)pHeader->virtualAddress, argc, argv);

}

static void shell_call(struct vfs_node *node, int argc, char *argv[])
{

    char *buffer = (char *)0x280000;

    file_read(node, 0, node->length, buffer);

    if (elf_check((struct elf_header *)buffer))
    {

        shell_execute_elf(node, (struct elf_header *)0x280000, argc, argv);

    }

    else
    {

        file_write_string(session_get_out(), "Unrecognized binary format. Continue? (y/n): ");

        char c = file_read_byte(session_get_in());

        file_write_string(session_get_out(), "\n");

        if (c == 'y')
        {

            shell_execute_flat((unsigned int *)0x280000, argc, argv);

        }

    }

}

static void shell_interpret(char *command)
{

    char *argv[32];
    int argc = string_split(argv, command, ' ');

    if (argc)
    {

        struct vfs_node *initrd = call_open("/initrd");
        struct vfs_node *node = file_find(initrd, argv[0]);

        if (node)
            shell_call(node, argc, argv);

        else
        {

            file_write_string(session_get_out(), argv[0]);
            file_write_string(session_get_out(), ": Command not found\n");

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

                file_write_byte(session_get_out(), '\b');
                file_write_byte(session_get_out(), ' ');
                file_write_byte(session_get_out(), '\b');

             }

            break;

        case '\n':

            stack_push(&shellStack, '\0');
            file_write_byte(session_get_out(), c);
            shell_interpret(shellBuffer);

            break;

        default:

            stack_push(&shellStack, c);
            file_write_byte(session_get_out(), c);

            break;

    }

}

static void shell_poll()
{

    for (;;)
    {

        char c = file_read_byte(session_get_in());

        shell_handle_input(c);

    }

}

void shell_init()
{

    stack_init(&shellStack, shellBuffer, SHELL_BUFFER_SIZE);

    file_write_string(session_get_out(), "Fudge\n");
    file_write_string(session_get_out(), "-----\n");
    file_write_string(session_get_out(), "Copyright (c) 2009 Jens Nyberg\n");
    file_write_string(session_get_out(), "Type 'cat help.txt' to read the help section.\n\n");

    shell_clear();
    shell_poll();

}

